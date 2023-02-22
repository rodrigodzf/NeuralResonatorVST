#include "PluginProcessor.h"
#include "PluginEditor.h"

using namespace juce;

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , mBrowser("http://localhost:3000") {
    juce::ignoreUnused(processorRef);

    // Set up the logger
    juce::Logger::setCurrentLogger(&mLogger);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // addAndMakeVisible(mBrowser);
    setResizable(true, true);
    setSize(400, 300);

    // Draw a shape
    mServerThread.setOnNewShapeCallback([this](const juce::Path &path)
    {
        int res = 64;
        DrawablePath shape;
        shape.setPath(path);
        shape.setFill(Colours::white);
        shape.setStrokeFill(Colours::white);
        Image img = Image(Image::RGB, res, res, true);
        Graphics bufferGraphics(img);
        shape.draw(bufferGraphics, 1.0f);
        PNGImageFormat png;

        Logger::writeToLog("Writing image to file");
        FileOutputStream stream(
            File::getCurrentWorkingDirectory().getChildFile("star.png")
        );

        // overwrite the file if it already exists
        if (stream.openedOk())
        {
            stream.setPosition(0);
            stream.truncate();
        }
        png.writeImageToStream(
            img, stream
        );
    });

}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor() {
    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g) {
    // (Our component is opaque, so we must completely fill the background
    // with a solid colour)
    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId));
    // mShape.draw(g, 1.0f);
}

void AudioPluginAudioProcessorEditor::resized() {
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mBrowser.setBounds(getBounds());
}
