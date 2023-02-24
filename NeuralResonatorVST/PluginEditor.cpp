#include "PluginProcessor.h"
#include "PluginEditor.h"


//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
    , mBrowser("http://localhost:3000")
{
    juce::ignoreUnused(processorRef);

    // Set up the logger
    juce::Logger::setCurrentLogger(&mLogger);

    // Make sure that before the constructor has finished, you've set the
    // editor's size to whatever you need it to be.
    // addAndMakeVisible(mBrowser);
    setResizable(true, true);
    setSize(400, 300);

    // Draw a shape
    mServerThread.setOnNewShapeCallback(
        [this](const juce::Path& path){ this->onNewShape(path); }
    );

    // Initialize the torch wrapper
    mTorchWrapperPtr = std::make_unique<TorchWrapper>(processorRef);
    mTorchWrapperPtr->loadModel("/home/diaz/projects/torchplugins/pretrained/pretrained/model_wrap.pt");
}

AudioPluginAudioProcessorEditor::~AudioPluginAudioProcessorEditor()
{
    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
void AudioPluginAudioProcessorEditor::paint(juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background
    // with a solid colour)
    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId));
    // mShape.draw(g, 1.0f);
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    mBrowser.setBounds(getBounds());
}

void AudioPluginAudioProcessorEditor::onNewShape(const juce::Path& path)
{
    int res = 64;
    juce::DrawablePath shape;
    shape.setPath(path);
    shape.setFill(juce::Colours::white);
    shape.setStrokeFill(juce::Colours::white);
    juce::Image img = juce::Image(juce::Image::SingleChannel, res, res, true);
    juce::Graphics bufferGraphics(img);
    shape.draw(bufferGraphics, 1.0f);

    // get the bitmap data and convert to a float tensor
    juce::Image::BitmapData bitmapData(img, juce::Image::BitmapData::readOnly);

    // get the bitmap data and convert to a float tensor
    std::vector<float> shapeData;
    shapeData.reserve(res * res);
    for (int y = 0; y < res; y++)
    {
        for (int x = 0; x < res; x++)
        {
            shapeData.push_back(
                bitmapData.getPixelPointer(x, y)[0] / 255.0f
            );
        }
    }
#if 0
    juce::PNGImageFormat png;

    juce::Logger::writeToLog("Writing image to file");
    juce::FileOutputStream stream(
        juce::File::getCurrentWorkingDirectory().getChildFile("star.png"));

    // overwrite the file if it already exists
    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
    }
    png.writeImageToStream(img, stream);
#endif
    // send message to wrapper
    mTorchWrapperPtr->predict("test");
}