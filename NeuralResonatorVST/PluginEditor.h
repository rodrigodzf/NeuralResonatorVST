#pragma once

#include "PluginProcessor.h"
#include "ConsoleLogger.h"
#include "BrowserComponent.h"
#include "ServerThread.h"
#include "TorchWrapper.h"
#include <juce_gui_basics/juce_gui_basics.h>
//==============================================================================
class AudioPluginAudioProcessorEditor 
    : public juce::AudioProcessorEditor
{
public:
    explicit AudioPluginAudioProcessorEditor (AudioPluginAudioProcessor&);
    ~AudioPluginAudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:

    // Callbacks for ServerThread
    void onNewShape(const juce::Path &path);
    void onNewMaterial(const std::vector<float> &material);
    void onNewPosition(const std::vector<float> &position);

private:
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    ConsoleLogger mLogger;
    std::unique_ptr<ServerThread> mServerThreadPtr;
    std::unique_ptr<BrowserComponent> mBrowserPtr;
    std::unique_ptr<TorchWrapper> mTorchWrapperPtr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
