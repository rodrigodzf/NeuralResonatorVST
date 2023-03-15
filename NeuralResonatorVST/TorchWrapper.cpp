#include "TorchWrapper.h"
#include "HelperFunctions.h"
#include "ServerThreadIf.h"
#include <cstring>
#include <algorithm>

TorchWrapper::TorchWrapper(
    ProcessorIf *processorPtr,
    juce::AudioProcessorValueTreeState &vtsRef,
    const juce::String &fcModelPath,
    const juce::String &encoderModelPath
)
    : mVts(vtsRef)
    , mProcessorPtr(processorPtr)
{
    mCoefficients.resize(32 * 2 * 6);

    // initialize the tensors
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    mLastMaterialTensor = torch::full({1, 5}, 0.5f, options);
    mLastPositionTensor = torch::full({1, 2}, 0.5f, options);

    // load the models
    loadModel(encoderModelPath.toStdString(), ModelType::ShapeEncoder);
    loadModel(fcModelPath.toStdString(), ModelType::FC);

    // do the first prediction to initialize the coefficients
    // and to avoid a delay when the first shape is received
    valueTreeRedirected(mVts.state);

    // add the listener for future changes
    mVts.state.addListener(this);

}

TorchWrapper::~TorchWrapper()
{
    mQueueThread.stopThread(100);
}

TorchWrapperIf *TorchWrapper::getTorchWrapperIfPtr()
{
    return this;
}

void TorchWrapper::loadModel(
    const std::string &modelPath,
    const ModelType modelType,
    const std::string &deviceString
)
{
    //! We need to read the models on the contructor
    //! because the state will be updated immediately
    //! after the constructor is called (usually)
    //! doing it in a thread does not guarantee that
    //! the models will be loaded before the state

    auto device = torch::Device(deviceString);
    try
    {
        // Deserialize the ScriptModule from a file using
        if (modelType == ModelType::ShapeEncoder)
        {
            mShapeEncoderNetwork = torch::jit::load(modelPath, device);
            mShapeEncoderNetwork.eval();
        }
        else if (modelType == ModelType::FC)
        {
            mFCNetwork = torch::jit::load(modelPath, device);
            mFCNetwork.eval();
        }
        else { JLOG("Model type not recognized"); }
    }
    catch (const c10::Error &e)
    {
        JLOG(
            "Error loading model: " + modelPath + " " + std::string(e.what())
        );
        jassertfalse;
        return;
    }
    JLOG("Model: " + modelPath + " loaded successfully!");
}

void TorchWrapper::handleReceivedNewShape(const juce::Path shape)
{
    // convert the path to an image
    juce::Image image = HelperFunctions::shapeToImage(shape);

    // get the bitmap data and convert to a float tensor
    juce::Image::BitmapData bitmapData(
        image,
        juce::Image::BitmapData::readOnly
    );

    // get the bitmap data and convert to a float tensor
    std::vector<float> bitmapDataAsFloats;
    bitmapDataAsFloats.reserve(bitmapData.width * bitmapData.height);
    for (int y = 0; y < bitmapData.height; y++)
    {
        for (int x = 0; x < bitmapData.width; x++)
        {
            bitmapDataAsFloats.push_back(
                bitmapData.getPixelPointer(x, y)[0] / 255.0f
            );
        }
    }

    // create the tensor
    int batchSize = 1;
    int channels = 1;
    int height = bitmapData.height;
    int width = bitmapData.width;
    auto options = torch::TensorOptions().dtype(torch::kFloat);
    auto tensor = torch::from_blob(
        bitmapDataAsFloats.data(),
        {batchSize, channels, height, width},
        options
    );

    // for images we need to repeat the tensor to match the number of channels
    tensor = tensor.repeat({1, 3, 1, 1});

    // inference
    c10::InferenceMode guard;
    try
    {
        // Create a vector of inputs.
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(tensor);

        // Execute the model and turn its output into a tensor.
        mFeatureTensor = mShapeEncoderNetwork.forward(inputs).toTensor();
    }
    catch (const c10::Error &e)
    {
        JLOG("Error processing image: " + std::string(e.what()));
        jassertfalse;
    }

    if (!mFeaturesReady) { mFeaturesReady = true; }

    predictCoefficients();
    JLOG("Predicted shape features");
}

void TorchWrapper::predictCoefficients()
{
    JLOG("Predicting coefficients");
    if (!mFeaturesReady)
    {
        JLOG("Features not ready");
        return;
    }

    if (!mLastMaterialTensor.numel())
    {
        JLOG("Material tensor not initialized");
        return;
    }

    if (!mLastPositionTensor.numel())
    {
        JLOG("Position tensor not initialized");
        return;
    }

    // Before inference, we need to concatenate the feature tensor (1x1000)
    // with position tensor (1x2), and the material tensor (1x5) along the 1st
    // dimension.

    auto tensor = torch::cat(
        {mFeatureTensor, mLastPositionTensor, mLastMaterialTensor},
        1
    );

    // std::cout << tensor.sizes() << std::endl;
#if 1
    // inference
    c10::InferenceMode guard;
    try
    {
        // Create a vector of inputs.
        std::vector<torch::jit::IValue> inputs;
        inputs.push_back(tensor);

        // Execute the model and turn its output into a tensor.
        auto coefficientTensor = mFCNetwork.forward(inputs).toTensor();

        std::memcpy(
            mCoefficients.data(),
            coefficientTensor.data_ptr(),
            coefficientTensor.numel() * sizeof(float)
        );
    }
    catch (const c10::Error &e)
    {
        JLOG("Error processing image: " + std::string(e.what()));
        jassertfalse;
    }
    mProcessorPtr->coefficentsChanged(mCoefficients);
#endif
}

void TorchWrapper::setServerThreadIf(ServerThreadIf *serverThreadIf)
{
    if (serverThreadIf != nullptr) { mServerThreadIf = serverThreadIf; }
}

bool TorchWrapper::startThread()
{
    return mQueueThread.startThread();
}

void TorchWrapper::valueTreePropertyChanged(
    juce::ValueTree &changedTree,
    const juce::Identifier &changedProperty
)
{
    // get the type of the tree that changed
    auto treeType = changedTree.getType().toString();

    if (treeType == "PARAM")
    {
        auto parameterID = changedTree.getProperty("id").toString();

        JLOG("TorchWrapper::Parameter changed: " + parameterID);

        if (auto *newValue = changedTree.getPropertyPointer(changedProperty))
        {
            mQueueThread.getIoService().post(
                [this, parameterID, newValue]
                {
                    if (parameterID == "density")
                    {
                        mLastMaterialTensor[0][0] = float(*newValue);
                    }
                    else if (parameterID == "stiffness")
                    {
                        mLastMaterialTensor[0][1] = float(*newValue);
                    }
                    else if (parameterID == "pratio")
                    {
                        mLastMaterialTensor[0][2] = float(*newValue);
                    }
                    else if (parameterID == "alpha")
                    {
                        mLastMaterialTensor[0][3] = float(*newValue);
                    }
                    else if (parameterID == "beta")
                    {
                        mLastMaterialTensor[0][4] = float(*newValue);
                    }
                    // the ui lives in the coordinate space where the origin
                    // is in the centre of the screen, and the range is
                    // approximately -1 to 1 in both x and y directions. with
                    // the y axis pointing up. the neural network lives in the
                    // coordinate space where the origin is in the TOP LEFT
                    // corner and the range is 0 to 1 in both x and y
                    // directions.
                    // we need to convert the values from the ui to the values
                    // that the neural network expects.
                    else if (parameterID == "xpos")
                    {
                        mLastPositionTensor[0][0] =
                            (float(*newValue) + 1.0f) * 0.5f;
                    }
                    else if (parameterID == "ypos")
                    {
                        // the y axis is flipped, so we need to invert the
                        // value
                        mLastPositionTensor[0][1] =
                            1.0f - ((float(*newValue) + 1.0f) * 0.5f);
                    }

                    this->predictCoefficients();
                }
            );
        }
    }
    else if (treeType == "polygon")
    {
        JLOG("TorchWrapper::Polygon changed");

        if (auto *flattenedVertices =
                changedTree.getPropertyPointer(changedProperty))
        {
            mQueueThread.getIoService().post(
                [this, flattenedVertices]
                {
                    auto size = flattenedVertices->size();

                    juce::Path path;
                    int res = 64;

                    for (int i = 0; i < size; i += 2)
                    {
                        auto x = float((*flattenedVertices)[i]);
                        auto y = float((*flattenedVertices)[i + 1]);

                        // the positions are in the range [-1, 1], so we need
                        // to scale them to the range [0, res] and flip the y
                        // axis
                        x = (x + 1) * 0.5 * res;
                        y = res - ((y + 1) * 0.5 * res);
                        if (i == 0) { path.startNewSubPath(x, y); }
                        else { path.lineTo(x, y); }
                    }

                    // close the subpath
                    path.closeSubPath();

                    // JLOG(
                    //     "TorchWrapper::polygon changed: " + path.toString()
                    // );

                    // handle the path
                    this->handleReceivedNewShape(path);
                }
            );
        }
    }
    else
    {
        JLOG("TorchWrapper::Unknown tree type changed: " + treeType);

        //         else if (parameterID == "vertices")
        // {
        //     JLOG("Vertices changed");
        // }
    }
}

void TorchWrapper::valueTreeChildAdded(
    juce::ValueTree &parentTree,
    juce::ValueTree &childWhichHasBeenAdded
)
{
}

void TorchWrapper::valueTreeChildRemoved(
    juce::ValueTree &parentTree,
    juce::ValueTree &childWhichHasBeenRemoved,
    int indexFromWhichChildWasRemoved
)
{
}

void TorchWrapper::valueTreeChildOrderChanged(
    juce::ValueTree &parentTreeWhoseChildrenHaveMoved,
    int oldIndex,
    int newIndex
)
{
}

void TorchWrapper::valueTreeParentChanged(
    juce::ValueTree &treeWhoseParentHasChanged
)
{
}

void TorchWrapper::valueTreeRedirected(juce::ValueTree &redirectedTree)
{
    JLOG(
        "TorchWrapper::valueTreeRedirected " +
        redirectedTree.getType().toString()
    );

    auto treeType = redirectedTree.getType().toString();

    // for each child of the tree, get the id and value and set the tensor
    // accordingly
    for (int i = 0; i < redirectedTree.getNumChildren(); i++)
    {
        auto child = redirectedTree.getChild(i);
        auto parameterID = child.getProperty("id").toString();

        if (auto *newValue = child.getPropertyPointer("value"))
        {
            if (parameterID == "density")
            {
                mLastMaterialTensor[0][0] = float(*newValue);
            }
            else if (parameterID == "stiffness")
            {
                mLastMaterialTensor[0][1] = float(*newValue);
            }
            else if (parameterID == "pratio")
            {
                mLastMaterialTensor[0][2] = float(*newValue);
            }
            else if (parameterID == "alpha")
            {
                mLastMaterialTensor[0][3] = float(*newValue);
            }
            else if (parameterID == "beta")
            {
                mLastMaterialTensor[0][4] = float(*newValue);
            }
            else if (parameterID == "xpos")
            {
                mLastPositionTensor[0][0] = (float(*newValue) + 1.0f) * 0.5f;
            }
            else if (parameterID == "ypos")
            {
                mLastPositionTensor[0][1] =
                    1.0f - ((float(*newValue) + 1.0f) * 0.5f);
            }
            else if (parameterID == "vertices")
            {
                auto size = newValue->size();

                juce::Path path;
                int res = 64;

                for (int i = 0; i < size; i += 2)
                {
                    auto x = float((*newValue)[i]);
                    auto y = float((*newValue)[i + 1]);

                    // the positions are in the range [-1, 1], so we need
                    // to scale them to the range [0, res] and flip the y
                    // axis
                    x = (x + 1) * 0.5 * res;
                    y = res - ((y + 1) * 0.5 * res);
                    if (i == 0) { path.startNewSubPath(x, y); }
                    else { path.lineTo(x, y); }
                }

                // close the subpath
                path.closeSubPath();

                // handle the path
                this->handleReceivedNewShape(path);
            }
        }
    }
}