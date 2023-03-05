#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // init browser
#if BROWSER_DEV_SERVER
#pragma message("Using dev server for UI")
    juce::String url = "http://" + processorRef.mConfigMap["host"] + ":" +
                       processorRef.mConfigMap["port"];
#else
#pragma message("Using local file for UI")
    juce::String url = "file://" + processorRef.mIndexFile.getFullPathName();
#endif
    setOpaque(true);
    mBrowserPtr.reset(new BrowserComponent());
    addAndMakeVisible(mBrowserPtr.get());
    mBrowserPtr->goToURL(url);
    setSize(400, 400);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background
    // with a solid colour)
    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId));
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    if (mBrowserPtr) { mBrowserPtr->setBounds(getBounds()); }
}