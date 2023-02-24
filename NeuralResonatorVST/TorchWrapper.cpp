#include "TorchWrapper.h"
#include "model.h"

TorchWrapper::TorchWrapper(
    AudioPluginAudioProcessor& processorRef
) : mProcessorRef(processorRef)
{
    mQueueThread.startThread();
}

TorchWrapper::~TorchWrapper()
{
    mQueueThread.stopThread(200);
}

void TorchWrapper::loadModel(
    const std::string& modelPath,
    const std::string& deviceString
)
{
    mQueueThread.getIoService().post([this, modelPath, deviceString]()
    {
        auto& model = Model::getInstance();
        model.loadModel(modelPath, deviceString);
    });
}

void TorchWrapper::getShapeFeatures(const std::vector<float> &shape)
{
    mQueueThread.getIoService().post([this, shape]()
    {
        this->handleGetShapeFeatures(shape);
    });
}

void TorchWrapper::handleGetShapeFeatures(const std::vector<float> &shape)
{
    auto tensor = Model::getInstance().toTensor(
        shape.data(),
        batch_size,
        channels,
        height,
        width
    );

    std::cout << "handleGetShapeFeatures: " << msg << std::endl;
    // mProcessorRef.coefficentsChanged(msg);
}