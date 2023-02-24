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

    // Set callbacks
    mServerThread.setOnNewShapeCallback(
        [this](const juce::Path& path){ this->onNewShape(path); }
    );
    mServerThread.setOnNewMaterialCallback(
        [this](const std::vector<float>& material){ this->onNewMaterial(material); }
    );

    // Initialize the torch wrapper
    mTorchWrapperPtr = std::make_unique<TorchWrapper>(processorRef);
    mTorchWrapperPtr->loadModel(
        "/home/diaz/projects/torchplugins/pretrained/pretrained/encoder.pt",
        TorchWrapper::ModelType::ShapeEncoder
    );
    mTorchWrapperPtr->loadModel(
        "/home/diaz/projects/torchplugins/pretrained/pretrained/fc.pt",
        TorchWrapper::ModelType::FC
    );
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
    // draw the shape to an image
    int res = 64;
    juce::DrawablePath shape;
    shape.setPath(path);
    shape.setFill(juce::Colours::white);
    shape.setStrokeFill(juce::Colours::white);
    juce::Image image = juce::Image(juce::Image::SingleChannel, res, res, true);
    juce::Graphics bufferGraphics(image);
    shape.draw(bufferGraphics, 1.0f);
    
    // send the shape data to the torch wrapper
    mTorchWrapperPtr->getShapeFeatures(image);
#if 0
    juce::PNGImageFormat png;

    juce::Logger::writeToLog("Writing image to file");
    juce::FileOutputStream stream(
        juce::File::getCurrentWorkingDirectory().getChildFile("shape.png"));

    // overwrite the file if it already exists
    if (stream.openedOk())
    {
        stream.setPosition(0);
        stream.truncate();
    }
    png.writeImageToStream(image, stream);
#endif
}

void AudioPluginAudioProcessorEditor::onNewMaterial(
    const std::vector<float>& material)
{
    // send the material data to the torch wrapper
    mTorchWrapperPtr->predictCoefficients(material);
}