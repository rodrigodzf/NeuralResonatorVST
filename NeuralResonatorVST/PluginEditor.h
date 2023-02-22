#pragma once

#include "PluginProcessor.h"
#include "ConsoleLogger.h"
#include "BrowserComponent.h"
#include "ServerThread.h"

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
    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    AudioPluginAudioProcessor& processorRef;

    ConsoleLogger mLogger;
    BrowserComponent mBrowser;
    ServerThread mServerThread;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPluginAudioProcessorEditor)
};
