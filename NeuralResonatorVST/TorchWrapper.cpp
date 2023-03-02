#include "TorchWrapper.h"
#include "HelperFunctions.h"
#include "ServerThreadIf.h"
#include <cstring>
#include <algorithm>

TorchWrapper::TorchWrapper(ProcessorIf *processorPtr,
                           juce::AudioProcessorValueTreeState &vtsRef)
    : mProcessorPtr(processorPtr), mVts(vtsRef)
{
    mCoefficients.resize(32 * 2 * 6);

    // initialize the tensors
    auto options = torch::TensorOptions().dtype(torch::kFloat32);
    mLastMaterialTensor = torch::full({1, 5}, 0.5f, options);
    mLastPositionTensor = torch::full({1, 2}, 0.5f, options);

    // initialize the attachments
    if (auto *parameter = mVts.getParameter("density"))
    {
        densityAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer)
            { parameterUpdate("density", 0, newValue, shouldSendToServer); },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("stiffness"))
    {
        stiffnessAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer) {
                parameterUpdate("stiffness", 1, newValue, shouldSendToServer);
            },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("pratio"))
    {
        poissonsRatioAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer)
            { parameterUpdate("pratio", 2, newValue, shouldSendToServer); },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("alpha"))
    {
        alphaAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer)
            { parameterUpdate("alpha", 3, newValue, shouldSendToServer); },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("beta"))
    {
        betaAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer)
            { parameterUpdate("beta", 4, newValue, shouldSendToServer); },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("xpos"))
    {
        xposAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer) {
                positionParameterUpdate("xpos", 0, newValue,
                                        shouldSendToServer);
            },
            mVts.undoManager));
    }

    if (auto *parameter = mVts.getParameter("ypos"))
    {
        yposAttachment.reset(new RemoteParameterAttachment(
            *parameter,
            [this](float newValue, bool shouldSendToServer) {
                positionParameterUpdate("ypos", 1, newValue,
                                        shouldSendToServer);
            },
            mVts.undoManager));
    }
}

TorchWrapper::~TorchWrapper()
{
    mQueueThread.stopThread(100);
}

TorchWrapperIf *TorchWrapper::getTorchWrapperIfPtr()
{
    return this;
}

void TorchWrapper::loadModel(const std::string &modelPath,
                             const ModelType modelType,
                             const std::string &deviceString)
{
    // TODO: check that the file exists

    mQueueThread.getIoService().post(
        [this, modelPath, modelType, deviceString]()
        {
            auto device = torch::Device(deviceString);
            try
            {
                // Deserialize the ScriptModule from a file using
                // torch::jit::load()

                if (modelType == ModelType::ShapeEncoder)
                {
                    mShapeEncoderNetwork =
                        torch::jit::load(modelPath, device);
                    mShapeEncoderNetwork.eval();
                }
                else if (modelType == ModelType::FC)
                {
                    mFCNetwork = torch::jit::load(modelPath, device);
                    mFCNetwork.eval();
                }
                else
                {
                    juce::Logger::writeToLog("Model type not recognized");
                }
            }
            catch (const c10::Error &e)
            {
                juce::Logger::writeToLog("Error loading model: " + modelPath +
                                         " " + std::string(e.what()));
                jassertfalse;
                return;
            }
            juce::Logger::writeToLog("Model loaded successfully");
        });
}

void TorchWrapper::receivedNewShape(juce::Path &shape)
{
    mQueueThread.getIoService().post(
        [this, shape]() { this->handleReceivedNewShape(shape); });
}

void TorchWrapper::handleReceivedNewShape(const juce::Path shape)
{
    // convert the path to an image
    juce::Image image = HelperFunctions::shapeToImage(shape);

    // get the bitmap data and convert to a float tensor
    juce::Image::BitmapData bitmapData(image,
                                       juce::Image::BitmapData::readOnly);

    // get the bitmap data and convert to a float tensor
    std::vector<float> bitmapDataAsFloats;
    bitmapDataAsFloats.reserve(bitmapData.width * bitmapData.height);
    for (int y = 0; y < bitmapData.height; y++)
    {
        for (int x = 0; x < bitmapData.width; x++)
        {
            bitmapDataAsFloats.push_back(bitmapData.getPixelPointer(x, y)[0] /
                                         255.0f);
        }
    }

    // create the tensor
    int batchSize = 1;
    int channels = 1;
    int height = bitmapData.height;
    int width = bitmapData.width;
    auto options = torch::TensorOptions().dtype(torch::kFloat);
    auto tensor =
        torch::from_blob(bitmapDataAsFloats.data(),
                         {batchSize, channels, height, width}, options);

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
        juce::Logger::writeToLog("Error processing image: " +
                                 std::string(e.what()));
        jassertfalse;
    }

    if (!mFeaturesReady) { mFeaturesReady = true; }

    predictCoefficients();
    DBG("Predicted shape features");
}

void TorchWrapper::receivedNewMaterial(const std::vector<float> &material)
{
    // We need to post this to the queue thread because this function is
    // called from the main thread
    // we pass a copy of the material vector to the lambda function
    // TODO: check if we can only make one copy
    // !This must be called from the MessagerManager thread, because we want
    // !to avoid feedback update of the parameters
    MessageManager::callAsync(
        [this, material]()
        {
            juce::Logger::writeToLog("Updating material");

            densityAttachment->remoteValueChanged(material[0]);
            stiffnessAttachment->remoteValueChanged(material[1]);
            poissonsRatioAttachment->remoteValueChanged(material[2]);
            alphaAttachment->remoteValueChanged(material[3]);
            betaAttachment->remoteValueChanged(material[4]);
        });
}

void TorchWrapper::receivedNewPosition(const std::vector<float> &position)
{
    // We need to post this to the queue thread because this function is
    // called from the main thread
    // we pass a copy of the position vector to the lambda function
    // TODO: check if we can only make one copy
    // !This must be called from the MessagerManager thread, because we want
    // !to avoid feedback update of the parameters
    MessageManager::callAsync(
        [this, position]()
        {
            juce::Logger::writeToLog("Updating position");

            xposAttachment->remoteValueChanged(position[0]);
            yposAttachment->remoteValueChanged(position[1]);
        });
}

void TorchWrapper::predictCoefficients()
{
    DBG("Predicting coefficients");
    if (!mFeaturesReady)
    {
        juce::Logger::writeToLog("Features not ready");
        return;
    }

    if (!mLastMaterialTensor.numel())
    {
        juce::Logger::writeToLog("Material tensor not initialized");
        return;
    }

    if (!mLastPositionTensor.numel())
    {
        juce::Logger::writeToLog("Position tensor not initialized");
        return;
    }

    // Before inference, we need to concatenate the feature tensor (1x1000)
    // with position tensor (1x2), and the material tensor (1x5) along the 1st
    // dimension.

    auto tensor = torch::cat(
        {mFeatureTensor, mLastPositionTensor, mLastMaterialTensor}, 1);

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

        std::memcpy(mCoefficients.data(), coefficientTensor.data_ptr(),
                    coefficientTensor.numel() * sizeof(float));
    }
    catch (const c10::Error &e)
    {
        juce::Logger::writeToLog("Error processing image: " +
                                 std::string(e.what()));
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

void TorchWrapper::parameterUpdate(const juce::String &parameterID, int idx,
                                   float value, bool shouldSendToServer)
{
    //! Here we are in the messenger thread or we MUST ensure that we are in
    //! the messenger thread

    // ensure we are in the messenger thread
    jassert(MessageManager::getInstance()->isThisTheMessageThread());

    juce::Logger::writeToLog(
        "Parameter update: " + parameterID +
        " should send to server: " + (shouldSendToServer ? "true" : "false"));

    // change the value in the tensor if different
    // if (mLastMaterialTensor[0][idx].item<float>() == value) { return; }

    mLastMaterialTensor[0][idx] = value;

    // create message
    juce::var json(new DynamicObject());

    json.getDynamicObject()->setProperty(parameterID, juce::var(value));

    // this should be done in a separate thread
    mQueueThread.getIoService().post([this] { this->predictCoefficients(); });

    // send the value to the ui
    if (shouldSendToServer)
    {
        juce::Logger::writeToLog("Sending message to UI");
        mServerThreadIf->sendMessage(JSON::toString(json));
    }
}

void TorchWrapper::positionParameterUpdate(const juce::String &parameterID,
                                           int idx, float value,
                                           bool shouldSendToServer)
{
    //! Here we are in the messenger thread or we MUST ensure that we are in
    //! the messenger thread

    // ensure we are in the messenger thread
    jassert(MessageManager::getInstance()->isThisTheMessageThread());

    // change the value in the tensor if different
    if (mLastPositionTensor[0][idx].item<float>() == value) { return; }

    mLastPositionTensor[0][idx] = value;

    // create message
    juce::var json(new DynamicObject());

    json.getDynamicObject()->setProperty(parameterID, juce::var(value));

    // this should be done in a separate thread
    mQueueThread.getIoService().post([this] { this->predictCoefficients(); });

    // send the value to the ui
    if (shouldSendToServer)
    {
        juce::Logger::writeToLog("Sending message to UI");
        mServerThreadIf->sendMessage(JSON::toString(json));
    }
}

void TorchWrapper::onOpen()
{
    //! This is called from the server thread
    //! We need to post this to the MessageManager thread
    MessageManager::callAsync(
        [this]()
        {
            juce::Logger::writeToLog("Sending initial values to UI");
            densityAttachment->sendInitialUpdate();
        });
}

void TorchWrapper::onClose()
{
    //! This is called from the server thread
    //! We need to post this to the MessageManager thread
    MessageManager::callAsync(
        [this]() { juce::Logger::writeToLog("Closing connection to UI"); });
}