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
    setSize(800, 800);

#if 0

    // Set callbacks
    mServerThreadPtr = std::make_unique<ServerThread>();
    mServerThreadPtr->setOnNewShapeCallback([this](const juce::Path& path)
                                            { this->onNewShape(path); });
    mServerThreadPtr->setOnNewMaterialCallback(
        [this](const std::vector<float>& material)
        { this->onNewMaterial(material); });
    mServerThreadPtr->setOnNewPositionCallback(
        [this](const std::vector<float>& position)
        { this->onNewPosition(position); });

    // Initialize the torch wrapper
    mTorchWrapperPtr = std::make_unique<TorchWrapper>(processorRef);
    mTorchWrapperPtr->loadModel(
        encoderPath.toStdString(),
        TorchWrapper::ModelType::ShapeEncoder);
    mTorchWrapperPtr->loadModel(
        fcPath.toStdString(),
        TorchWrapper::ModelType::FC);
#endif
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