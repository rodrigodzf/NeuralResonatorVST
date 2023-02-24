#include "TorchWrapper.h"
#include <cstring>

TorchWrapper::TorchWrapper(AudioPluginAudioProcessor &processorRef)
    : mProcessorRef(processorRef)
{
    mCoefficients.resize(32 * 2 * 6);
    mQueueThread.startThread();
}

TorchWrapper::~TorchWrapper()
{
    mQueueThread.stopThread(50);
}

void TorchWrapper::loadModel(const std::string &modelPath,
                             const ModelType modelType,
                             const std::string &deviceString)
{
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
                juce::Logger::writeToLog("Error loading model: " +
                                         std::string(e.what()));
                jassertfalse;
                return;
            }
            juce::Logger::writeToLog("Model loaded successfully");
        });
}

void TorchWrapper::getShapeFeatures(const juce::Image &image)
{
    mQueueThread.getIoService().post(
        [this, image]() { this->handleGetShapeFeatures(image); });
}

void TorchWrapper::handleGetShapeFeatures(const juce::Image &image)
{
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

    if (!mFeaturesReady)
    {
        mFeaturesReady = true;
    }
    DBG("Predicted shape features");
}

void TorchWrapper::predictCoefficients(
    const std::vector<float> &material)
{
    mQueueThread.getIoService().post(
        [this, material]() { this->handlePredictCoefficients(material); });
}

void TorchWrapper::handlePredictCoefficients(
    /*const*/ std::vector<float> material)
{
    if (!mFeaturesReady)
    {
        juce::Logger::writeToLog("Features not ready");
        return;
    }
#if 0
    // print the material
    for (int i = 0; i < material.size(); i++)
    {
        std::cout << material[i] << " ";
    }
    std::cout << std::endl;
#endif

    // create the tensor
    int batchSize = 1;
    int channels = 1;
    int height = 1;
    int width = material.size();
    auto options = torch::TensorOptions().dtype(torch::kFloat);
    auto tensor = torch::from_blob(
        material.data(), {batchSize, channels, height, width}, options);

    // We need to concatenate the feature tensor with the material tensor
    // along the 1st dimension (the feature tensor is 1x1000)
    std::cout << tensor.sizes() << std::endl;
    std::cout << mFeatureTensor.sizes() << std::endl;

    // expand the feature tensor to match the material tensor
    auto features = mFeatureTensor.view({1, 1, 1, 1000});
    tensor = torch::cat({features, tensor}, 3);
    std::cout << tensor.sizes() << std::endl;

#if 0
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
    mProcessorRef.coefficentsChanged(mCoefficients);
#endif

}