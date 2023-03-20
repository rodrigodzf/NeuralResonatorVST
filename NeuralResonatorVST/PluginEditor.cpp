#include "HelperFunctions.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p
)
    : AudioProcessorEditor(&p)
    , processorRef(p)
#if SIMPLE_UI
    , mShapeComponent(p.mParameters)
    , mPanel(p.mParameters)
#endif
{
#ifndef SIMPLE_UI
    // init browser
#if BROWSER_DEV_SERVER
#pragma message("Using dev server for UI")
    juce::String url = "http://localhost:3000";
#else
#pragma message("Using local file for UI")
    juce::String url = "file://" + processorRef.mIndexFile.getFullPathName();
#endif
    // Initialize the parameter syncer
    JLOG("Initializing parameter syncer");
    mParameterSyncerPtr.reset(new ParameterSyncer(processorRef.mParameters));

    // Initialize the server thread
    JLOG("Initializing server thread");
    mServerThreadPtr.reset(
        new ServerThread(mParameterSyncerPtr->getParameterSyncerIfPtr())
    );
    mServerThreadPtr->startThread();

    // Pass the server thread to the parameter syncer
    mParameterSyncerPtr->setServerThreadIf(
        mServerThreadPtr->getServerThreadIfPtr()
    );

    setOpaque(true);
    mBrowserPtr.reset(new BrowserComponent());
    addAndMakeVisible(mBrowserPtr.get());
    mBrowserPtr->goToURL(url);
#else
    addAndMakeVisible(mShapeComponent);
    addAndMakeVisible(mPanel);
#endif
    setSize(800, 400);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    // Stop the threads in reverse order (from the top down)
#ifndef SIMPLE_UI
    mServerThreadPtr.reset();
    mServerThreadPtr = nullptr;

    mParameterSyncerPtr.reset();
    mParameterSyncerPtr = nullptr;
#endif
    mBrowserPtr.reset();
    mBrowserPtr = nullptr;

}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background
    // with a solid colour)
    g.fillAll(
        getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId)
    );
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
#if SIMPLE_UI
    auto area = getLocalBounds();
    mShapeComponent.setBounds(area.removeFromLeft(500));
    mPanel.setBounds(area.removeFromRight(300));

#else
    if (mBrowserPtr) { mBrowserPtr->setBounds(getBounds()); }
#endif
}