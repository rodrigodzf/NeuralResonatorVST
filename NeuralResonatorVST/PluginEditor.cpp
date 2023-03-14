#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p
)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // init browser
#if BROWSER_DEV_SERVER
#pragma message("Using dev server for UI")
    juce::String url = "http://localhost:3000";
#else
#pragma message("Using local file for UI")
    juce::String url = "file://" + processorRef.mIndexFile.getFullPathName();
#endif

    // Initialize the parameter syncer
    juce::Logger::writeToLog("Initializing parameter syncer");
    mParameterSyncerPtr.reset(new ParameterSyncer(processorRef.mParameters));

    // Initialize the server thread
    juce::Logger::writeToLog("Initializing server thread");
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
    setSize(800, 400);
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() 
{
    // Stop the threads in reverse order (from the top down)
    mServerThreadPtr.reset();
    mServerThreadPtr = nullptr;

    mParameterSyncerPtr.reset();
    mParameterSyncerPtr = nullptr;

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
    if (mBrowserPtr) { mBrowserPtr->setBounds(getBounds()); }
}