#pragma once

#include "QueueThread.h"
#include "PluginProcessor.h"

#include <torch/script.h>
#include <torch/torch.h>

class TorchWrapper
{

public:
    TorchWrapper(AudioPluginAudioProcessor& processorRef);
    ~TorchWrapper();

    void loadModel(
        const std::string& modelPath,
        const std::string& deviceString = "cpu"
    );
    
    void getShapeFeatures(const std::string& msg);
    void handleGetShapeFeatures(const std::string& msg);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& mProcessorRef;

    QueueThread mQueueThread{ "torch_wrapper" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TorchWrapper)
};


