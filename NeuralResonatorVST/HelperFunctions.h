#pragma once

#include <map>

#include "JuceHeader.h"

class HelperFunctions
{
public:
    /**
     * @brief  Get the config map from the config file
     * @note
     * @param  &configFileName:
     * @retval std::map<juce::String, juce::String>
     */
    static std::map<juce::String, juce::String> getConfig(
        const juce::String &subDirectoryName = "NeuralResonatorVST",
        const juce::String &configFileName = "config.json")
    {
        // load the config file
        juce::File configFile =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userApplicationDataDirectory)
                .getChildFile(subDirectoryName)
                .getChildFile(configFileName);

        juce::Logger::writeToLog("Config file path: " +
                                 configFile.getFullPathName());

        // Create parent directory if it doesn't exist
        if (!configFile.exists())
        {
            configFile.getParentDirectory().createDirectory();
        }

        // if config file doesn't exist, create it
        if (!configFile.existsAsFile())
        {
            juce::Logger::writeToLog(
                "Config file doesn't exist, creating it");
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
        auto encoderPath = config.getProperty("encoder_path", {}).toString();
        auto fcPath = config.getProperty("fc_path", {}).toString();
        auto host = config.getProperty("host", {}).toString();
        auto port = config.getProperty("port", {}).toString();

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

        std::map<juce::String, juce::String> configMap = {
            {"encoder_path", encoderPath},
            {"fc_path", fcPath},
            {"host", host},
            {"port", juce::String(port)}};

        return configMap;
    }

    /**
     * @brief Save the index.html file to the user's home directory
     */
    static juce::File saveLoadIndexFile(
        const juce::String &subDirectoryName = "NeuralResonatorVST",
        const juce::String &indexFileName = "index.html")
    {
        // create or load the index file
        juce::File indexFile =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userApplicationDataDirectory)
                .getChildFile(subDirectoryName)
                .getChildFile(indexFileName);

        juce::Logger::writeToLog("Index file path: " +
                                 indexFile.getFullPathName());

        // Create parent directory if it doesn't exist
        if (!indexFile.exists())
        {
            indexFile.getParentDirectory().createDirectory();
        }

        // if the index file doesn't exist, create it
        if (!indexFile.existsAsFile())
        {
            // Create a file output stream to write the binary data to
            juce::FileOutputStream stream(indexFile);

            // write the binary data to the file
            stream.write(BinaryData::index_html, BinaryData::index_htmlSize);
            stream.flush();
        }
        else
        {
            // if the index file exists, check that it's the same as the one
            // in the binary data
            juce::FileInputStream stream(indexFile);
            juce::MemoryBlock block;
            stream.readIntoMemoryBlock(block);
            if (block.getSize() != BinaryData::index_htmlSize)
            {
                juce::Logger::writeToLog(
                    "Index file is different, overwriting it");
                // if the size is different, overwrite the file
                juce::FileOutputStream stream(
                    juce::File::getSpecialLocation(
                        juce::File::SpecialLocationType::
                            userApplicationDataDirectory)
                        .getChildFile("index.html"));

                // write the binary data to the file
                stream.write(BinaryData::index_html,
                             BinaryData::index_htmlSize);
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

    static juce::Image shapeToImage(const juce::Path &path,
                                    const int width = 64,
                                    const int height = 64)
    {
#if 0

        juce::Image image(juce::Image::SingleChannel, width, height, true);
        juce::Graphics g(image);
        g.setColour(juce::Colours::white);
        g.fillPath(shape);
        return image;
#else
        // draw the shape to an image
        juce::DrawablePath shape;
        shape.setPath(path);
        shape.setFill(juce::Colours::white);
        shape.setStrokeFill(juce::Colours::white);
        juce::Image image =
            juce::Image(juce::Image::SingleChannel, width, height, true);
        juce::Graphics bufferGraphics(image);
        shape.draw(bufferGraphics, 1.0f);
#endif
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
        return image;

    }

    static std::vector<juce::Point<float>> createCircle(
        unsigned int nVertices = 10, float radius = 1.0f)
    {
        // generate 10 evenly spaced points on a circle with radius 0.5
        std::vector<juce::Point<float>> polygon;
        for (int i = 0; i < 10; i++)
        {
            float angle = 2 * juce::MathConstants<float>::pi * i / 10;
            polygon.push_back(juce::Point<float>(radius * std::cos(angle),
                                                 radius * std::sin(angle)));
        }
        return polygon;
    }

private:
    juce::String mEncoderPath;
    juce::String mFCPath;
    juce::String mHost;
    int mPort;
};