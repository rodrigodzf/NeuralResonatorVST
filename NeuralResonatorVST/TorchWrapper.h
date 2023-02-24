#pragma once

#include "QueueThread.h"
#include "PluginProcessor.h"

#include <juce_gui_basics/juce_gui_basics.h>

#include <torch/script.h>
#include <torch/torch.h>

class TorchWrapper
{
public:
    enum class ModelType
    {
        ShapeEncoder,
        FC
    };

    TorchWrapper(AudioPluginAudioProcessor& processorRef);
    ~TorchWrapper();

    void loadModel(const std::string& modelPath, const ModelType modelType,
                   const std::string& deviceString = "cpu");

    void getShapeFeatures(const juce::Image& image);
    void handleGetShapeFeatures(const juce::Image& image);

    void predictCoefficients(const std::vector<float>& material);
    void handlePredictCoefficients(/*const*/ std::vector<float> material);

private:
    torch::jit::Module mShapeEncoderNetwork;
    torch::jit::Module mFCNetwork;

    // intermediate tensor for features
    torch::Tensor mFeatureTensor;
    std::vector<float> mCoefficients;

    // flags
    bool mFeaturesReady = false;

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& mProcessorRef;

    QueueThread mQueueThread{"torch_wrapper"};

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TorchWrapper)
};
