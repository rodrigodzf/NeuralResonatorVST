#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "JuceHeader.h"

//==============================================================================
AudioPluginAudioProcessorEditor::AudioPluginAudioProcessorEditor(
    AudioPluginAudioProcessor& p)
    : AudioProcessorEditor(&p), processorRef(p)
{
    // Set up the logger
    juce::Logger::setCurrentLogger(&mLogger);

    // load the config file
    juce::File configFile =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::userApplicationDataDirectory)
            .getChildFile("config.json");

    std::cout << "Config file: " << configFile.getFullPathName() << std::endl;
    
    // if config file doesn't exist, create it
    if (!configFile.existsAsFile())
    {
        juce::Logger::writeToLog("Config file doesn't exist, creating it");
        juce::FileOutputStream stream(configFile);
        stream << "{\n"
               << "    \"encoder_path\": \"encoder.pt\",\n"
               << "    \"fc_path\": \"fc.pt\",\n"
               << "    \"host\": \"localhost\",\n"
               << "    \"port\": 3000\n"
               << "}";
        
        // close the stream
        stream.flush();
    }

    // load the config file
    juce::var config = juce::JSON::parse(configFile);

    // get the encoder path
    juce::String encoderPath = config.getProperty("encoder_path", {}).toString();
    juce::String fcPath = config.getProperty("fc_path", {}).toString();
    juce::String host = config.getProperty("host", {}).toString();
    int port = config.getProperty("port", {}).toString().getIntValue();

    // Check that the files exist
    if (!juce::File(encoderPath).existsAsFile())
    {
        juce::Logger::writeToLog("Encoder file doesn't exist");
        jassertfalse;
    }
    if (!juce::File(fcPath).existsAsFile())
    {
        juce::Logger::writeToLog("FC file doesn't exist");
        jassertfalse;
    }

    // Save or load the index.html file
    auto indexFile = saveLoadIndexFile();

    // init browser
#if BROWSER_DEV_SERVER
#pragma message("Using dev server")
    juce::String url = "http://" + host + ":" + juce::String(port);
#else
#pragma message("Using local file")
    juce::String url = "file://" + indexFile.getFullPathName();
#endif
    mBrowserPtr = std::make_unique<BrowserComponent>(url);
    addAndMakeVisible(mBrowserPtr.get());
    setResizable(true, true);
    setSize(800, 800);

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
}

void AudioPluginAudioProcessorEditor::resized()
{
    // This is generally where you'll want to lay out the positions of any
    // subcomponents in your editor..
    if (mBrowserPtr)
    {
        mBrowserPtr->setBounds(getBounds());
    }
}

void AudioPluginAudioProcessorEditor::onNewShape(const juce::Path& path)
{
    // draw the shape to an image
    int res = 64;
    juce::DrawablePath shape;
    shape.setPath(path);
    shape.setFill(juce::Colours::white);
    shape.setStrokeFill(juce::Colours::white);
    juce::Image image =
        juce::Image(juce::Image::SingleChannel, res, res, true);
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
    mTorchWrapperPtr->updateMaterial(material);
}

void AudioPluginAudioProcessorEditor::onNewPosition(
    const std::vector<float>& position)
{
    // send the position data to the torch wrapper
    mTorchWrapperPtr->updatePosition(position);
}

juce::File AudioPluginAudioProcessorEditor::saveLoadIndexFile()
{
    // create or load the index file
    juce::File indexFile = juce::File::getSpecialLocation(
        juce::File::SpecialLocationType::userApplicationDataDirectory
    ).getChildFile("index.html");

    juce::Logger::writeToLog("Index file path: " + indexFile.getFullPathName());

    // if the index file doesn't exist, create it
    if (!indexFile.existsAsFile())
    {
        // Create a file output stream to write the binary data to
        juce::FileOutputStream stream(
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userApplicationDataDirectory
            ).getChildFile("index.html")
        );

        // write the binary data to the file
        stream.write(BinaryData::index_html, BinaryData::index_htmlSize);
        stream.flush();
    }
    else
    {
        // if the index file exists, check that it's the same as the one in the
        // binary data
        juce::FileInputStream stream(indexFile);
        juce::MemoryBlock block;
        stream.readIntoMemoryBlock(block);
        if (block.getSize() != BinaryData::index_htmlSize)
        {
            juce::Logger::writeToLog("Index file is different, overwriting it");
            // if the size is different, overwrite the file
            juce::FileOutputStream stream(
                juce::File::getSpecialLocation(
                    juce::File::SpecialLocationType::userApplicationDataDirectory
                ).getChildFile("index.html")
            );

            // write the binary data to the file
            stream.write(BinaryData::index_html, BinaryData::index_htmlSize);
            stream.flush();
        }

        // check that the file exists (this should always be true)
        if (!indexFile.existsAsFile())
        {
            juce::Logger::writeToLog("Index file doesn't exist");
            jassertfalse;
        }
    }

    return indexFile;
}