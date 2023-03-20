#pragma once

#include <map>

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <juce_data_structures/juce_data_structures.h>

#define JLOG(msg)                                          \
    JUCE_BLOCK_WITH_FORCED_SEMICOLON(                      \
        juce::Time current = juce::Time::getCurrentTime(); \
        juce::String timestamp = juce::String::formatted(  \
            "%02d:%02d:%02d.%03d",                         \
            current.getHours(),                            \
            current.getMinutes(),                          \
            current.getSeconds(),                          \
            current.getMilliseconds()                      \
        );                                                 \
        juce::Logger::writeToLog(timestamp + " " + msg);   \
    )

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
        const juce::String& subDirectoryName = "NeuralResonatorVST",
        const juce::String& configFileName = "config.json"
    )
    {
        // load the config file
        juce::File configFile =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userApplicationDataDirectory
            )
                .getChildFile(subDirectoryName)
                .getChildFile(configFileName);

        juce::Logger::writeToLog(
            "Config file path: " + configFile.getFullPathName()
        );

        // Create parent directory if it doesn't exist
        if (!configFile.exists())
        {
            configFile.getParentDirectory().createDirectory();
        }

        // if config file doesn't exist, create it
        if (!configFile.existsAsFile())
        {
            juce::Logger::writeToLog("Config file doesn't exist, creating it"
            );
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

    static juce::String findResourcePath(const juce::String& path)
    {
        // locate the resource path inside the bundle
        juce::String resourcePath =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::currentApplicationFile
            )
                .getChildFile("Contents")
                .getChildFile("Resources")
                .getChildFile(path)
                .getFullPathName();

        // if the resource path doesn't exist, use the current directory
        if (!juce::File(resourcePath).existsAsFile())
        {
            resourcePath =
                juce::File::getSpecialLocation(
                    juce::File::SpecialLocationType::currentExecutableFile
                )
                    .getSiblingFile(path)
                    .getFullPathName();
        }

        // if the resource path doesn't exist exit
        if (!juce::File(resourcePath).existsAsFile())
        {
            JLOG("Resource file: " + path + " doesn't exist");
            jassertfalse;
        }

        return resourcePath;
    }

#if 0
    /**
     * @brief Save the index.html file to the user's home directory
     */
    static juce::File saveLoadIndexFile(
        const juce::String& subDirectoryName = "NeuralResonatorVST",
        const juce::String& indexFileName = "index.html"
    )
    {
        // create or load the index file
        juce::File indexFile =
            juce::File::getSpecialLocation(
                juce::File::SpecialLocationType::userApplicationDataDirectory
            )
                .getChildFile(subDirectoryName)
                .getChildFile(indexFileName);

        juce::Logger::writeToLog(
            "Index file path: " + indexFile.getFullPathName()
        );

        // Create parent directory if it doesn't exist
        if (!indexFile.exists())
        {
            indexFile.getParentDirectory().createDirectory();
        }

        // Create a file output stream to write the binary data to
        // overwrite the file if it already exists
        juce::FileOutputStream stream(indexFile);
        if (stream.openedOk())
        {
            stream.setPosition(0);
            stream.truncate();
            // write the binary data to the file
            stream.write(BinaryData::index_html, BinaryData::index_htmlSize);
            stream.flush();
        }

        return indexFile;
    }
#endif
    static juce::Image shapeToImage(
        const juce::Path& path,
        const int width = 64,
        const int height = 64
    )
    {
#if 1

        juce::Image image(juce::Image::SingleChannel, width, height, true);
        juce::Graphics g(image);
        g.setColour(juce::Colours::white);
        g.fillPath(path);
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

    static std::vector<juce::Point<float>>
        createCircle(unsigned int nVertices = 10, float radius = 1.0f)
    {
        // generate 10 evenly spaced points on a circle with radius 0.5
        std::vector<juce::Point<float>> polygon;
        for (int i = 0; i < 10; i++)
        {
            float angle = 2 * juce::MathConstants<float>::pi * i / 10;
            polygon.push_back(juce::Point<float>(
                radius * std::cos(angle),
                radius * std::sin(angle)
            ));
        }
        return polygon;
    }

    static juce::var convertToVar(const juce::ValueTree& tree)
    {
        juce::var root(new juce::DynamicObject());

        auto type = tree.getType();
        root.getDynamicObject()->setProperty("type", type.toString());

        for (int i = 0; i < tree.getNumProperties(); ++i)
        {
            const auto propName = tree.getPropertyName(i);
            const juce::var value = tree.getProperty(propName);

            root.getDynamicObject()->setProperty(propName, value);
        }

        juce::Array<juce::var> children;
        for (int i = 0; i < tree.getNumChildren(); ++i)
        {
            const auto& child = tree.getChild(i);
            children.add(convertToVar(child));
        }
        if (children.size() > 0)
        {
            root.getDynamicObject()->setProperty("children", children);
        }
        return root;
    }

    static juce::ValueTree convertToValueTree(const juce::var& var)
    {
        auto* obj = var.getDynamicObject();
        if (obj == nullptr)
        {
            juce::Logger::writeToLog(
                "ValueTreeToVar::convertToValueTree: var has no dynamic "
                "object"
            );
            jassertfalse;
            return {};
        }

        auto type = obj->getProperty("type").toString();
        auto root = juce::ValueTree(type);

        for (int i = 0; i < obj->getProperties().size(); ++i)
        {
            const auto& propName = obj->getProperties().getName(i);
            const auto& value = obj->getProperty(propName);

            if (propName == juce::Identifier("children")) continue;
            root.setProperty(propName, value, nullptr);
        }

        const auto& children = obj->getProperty("children");
        for (int i = 0; i < children.size(); ++i)
        {
            const auto& child = children[i];
            root.addChild(convertToValueTree(child), -1, nullptr);
        }

        return root;
    }
};

// Tests
#if 0

    auto state = HelperFunctions::convertToVar(mParameters.state);

    juce::Logger::writeToLog(juce::JSON::toString(state));
    // write to 

    auto newState = HelperFunctions::convertToValueTree(state);

    // check that the new state is the same as the old state
    juce::MemoryOutputStream stream(2048);
    mParameters.state.writeToStream(stream);

    juce::MemoryOutputStream stream2(2048);
    newState.writeToStream(stream2);

    stream.flush();
    stream2.flush();

    auto isEqual = stream.toString() == stream2.toString();

    if (isEqual)
    {
        juce::Logger::writeToLog("The two states are equal");
    }
    else
    {
        juce::Logger::writeToLog("The two states are not equal");
    }

    

    auto newState2 = HelperFunctions::convertToVar(newState);

    juce::Logger::writeToLog(juce::JSON::toString(newState2));

    // write as xml
    juce::Logger::writeToLog(mParameters.state.toXmlString());

#endif