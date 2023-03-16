#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HelperFunctions.h"
#include <kac_core.hpp>
//==============================================================================
AudioPluginAudioProcessor::AudioPluginAudioProcessor()
    : AudioProcessor(
          BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
              .withInput("Input", juce::AudioChannelSet::stereo(), true)
#endif
              .withOutput("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
    , mParameters(
          *this,    // processor to connect to
          nullptr,  // undo manager
          juce::Identifier("NeuralResonatorVSTParams"),  // identifier
          createParameterLayout()                        // parameter layout
      )
    , mFilterbank(32, 2)
{
    // Set up the logger
    mFileLoggerPtr.reset(juce::FileLogger::createDefaultAppLogger(
        "NeuralResonatorVST",
        "log.txt",
        "NeuralResonatorVST log file"
    ));
    juce::Logger::setCurrentLogger(mFileLoggerPtr.get());
    JLOG("AudioPluginAudioProcessor constructor");

    // Create and append the polygon valueTree to the parameter tree
    createAndAppendValueTree();

    // location of the index.html file inside the plugin bundle
    mIndexFile =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("index.html")
            .getFullPathName();
    
    // check that the index file exists
    if (!juce::File(mIndexFile).existsAsFile()) {
        JLOG("index.html not found");
    }

    // location of the pretrained models inside the plugin bundle 
    auto encoderPath =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("encoder.pt")
            .getFullPathName();

    auto fcPath =
        juce::File::getSpecialLocation(
            juce::File::SpecialLocationType::currentApplicationFile)
            .getChildFile("Contents")
            .getChildFile("Resources")
            .getChildFile("model_wrap.pt")
            .getFullPathName();
    
    // check that the models exist
    if (!juce::File(encoderPath).existsAsFile()) {
        JLOG("encoder.pt not found");
    }
    if (!juce::File(fcPath).existsAsFile()) {
        JLOG("model_wrap.pt not found");
    }

    // initialize the torch wrapper
    JLOG("Initializing torch wrapper");
    mTorchWrapperPtr.reset(
        new TorchWrapper(this, mParameters, fcPath, encoderPath)
    );

    // Start the threads in order (from the bottom up)
    mQueueThread.startThread();
    mTorchWrapperPtr->startThread();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    JLOG("AudioPluginAudioProcessor destructor");

    mTorchWrapperPtr.reset();
    mTorchWrapperPtr = nullptr;

    mQueueThread.stopThread(100);

    juce::Logger::setCurrentLogger(nullptr);
}

//==============================================================================
const juce::String AudioPluginAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool AudioPluginAudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool AudioPluginAudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double AudioPluginAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int AudioPluginAudioProcessor::getNumPrograms()
{
    return 1;  // NB: some hosts don't cope very well if you tell them there
               // are 0 programs, so this should be at least 1, even if you're
               // not really implementing programs.
}

int AudioPluginAudioProcessor::getCurrentProgram()
{
    return 0;
}

void AudioPluginAudioProcessor::setCurrentProgram(int index)
{
    juce::ignoreUnused(index);
}

const juce::String AudioPluginAudioProcessor::getProgramName(int index)
{
    juce::ignoreUnused(index);
    return {};
}

void AudioPluginAudioProcessor::changeProgramName(
    int index,
    const juce::String& newName
)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(
    double sampleRate,
    int samplesPerBlock
)
{
    JLOG("prepareToPlay");
    // Use this method as the place to do any pre-playback
    // initialisation that you need..

    // Set interpolation delta for filterbank (0.05 seconds)
    float interpolationDeltaSeconds = 0.05;
    unsigned int interpolationDelta = (unsigned int)(interpolationDeltaSeconds * sampleRate);
    mFilterbank.setInterpolationDelta(interpolationDelta);
}

void AudioPluginAudioProcessor::releaseResources()
{
    JLOG("releaseResources");
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

bool AudioPluginAudioProcessor::isBusesLayoutSupported(
    const BusesLayout& layouts
) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused(layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

        // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}

void AudioPluginAudioProcessor::processBlock(
    juce::AudioBuffer<float>& buffer,
    juce::MidiBuffer& midiMessages
)
{
    // juce::ignoreUnused(midiMessages);

    // juce::ScopedNoDenormals noDenormals;
    // auto totalNumInputChannels = getTotalNumInputChannels();
    // auto totalNumOutputChannels = getTotalNumOutputChannels();

    // In case we have more outputs than inputs, this code clears any output
    // channels that didn't contain input data, (because these aren't
    // guaranteed to be empty - they may contain garbage).
    // This is here to avoid people getting screaming feedback
    // when they first compile a plugin, but obviously you don't need to keep
    // this code if your algorithm always overwrites all the output channels.
    // for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    // buffer.clear(i, 0, buffer.getNumSamples());

    // This is the place where you'd normally do the guts of your plugin's
    // audio processing...
    // Make sure to reset the state if your inner loop is processing
    // the samples and the outer loop is handling the channels.
    // Alternatively, you can process the samples with the channels
    // interleaved by keeping the same state.
    // https://forum.juce.com/t/1-most-common-programming-mistake-that-we-see-on-the-forum/26013

    // if we receive any midi message and the
    // buffer is empty, then create a buffer 
    // with an impulse
    if (midiMessages.getNumEvents() > 0)
    {
        // get midi on
        for (const auto &midi : midiMessages)
        {
            if (midi.getMessage().isNoteOn())
            {
                // JLOG("Midi note on");
                // check if the buffer is empty
                buffer.clear();
                buffer.setSample(0, 0, 1.0f);
                buffer.setSample(1, 0, 1.0f);
                break;
            }
        }
    }

    // Process samples
    mFilterbank.processBuffer(buffer);
}

//==============================================================================
bool AudioPluginAudioProcessor::hasEditor() const
{
    return true;  // (change this to false if you choose to not supply an
                  // editor)
}

juce::AudioProcessorEditor* AudioPluginAudioProcessor::createEditor()
{
    JLOG("AudioPluginAudioProcessor::createEditor");
    return new AudioPluginAudioProcessorEditor(*this);
}

//==============================================================================
void AudioPluginAudioProcessor::getStateInformation(
    juce::MemoryBlock& destData
)
{
    // You should use this method to store your parameters in the memory
    // block. You could do that either as raw data, or use the XML or
    // ValueTree classes as intermediaries to make it easy to save and load
    // complex data.
    JLOG("AudioPluginAudioProcessor::getStateInformation");
    juce::MemoryOutputStream stream(destData, false);
    auto str = juce::JSON::toString(HelperFunctions::convertToVar(mParameters.state));
    // JLOG("AudioPluginAudioProcessor::getStateInformation: " + str);
    stream.writeString(str);
}

void AudioPluginAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes
)
{
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the
    // getStateInformation() call.
    //! Warning: this seems to delete the callbacks if not handled properly
    JLOG("AudioPluginAudioProcessor::setStateInformation");
    juce::MemoryInputStream stream(data, size_t (sizeInBytes), true);
    auto jsonAsStr = stream.readEntireStreamAsString();
    // JLOG(jsonAsStr);
    auto json = juce::JSON::parse(jsonAsStr);
    mParameters.replaceState(HelperFunctions::convertToValueTree(json));    
}

void AudioPluginAudioProcessor::coefficentsChanged(
    const std::vector<float>& coeffs
)
{
    mQueueThread.getIoService().post(
        [this, coeffs]() { this->handleCoefficentsChanged(coeffs); }
    );
}

void AudioPluginAudioProcessor::handleCoefficentsChanged(
    const std::vector<float>& coefficients
)
{
    JLOG("Coefficents changed");
    // JLOG("Number of coefficients: " +
    //                          std::to_string(coefficients.size()));
    mFilterbank.setCoefficients(coefficients);
}

juce::AudioProcessorValueTreeState::ParameterLayout
    AudioPluginAudioProcessor::createParameterLayout()
{
    // Set up the parameters
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "density",                       // parameterID
        "Density",                       // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            2.0f,
            0.01f
        ),    // min, max, interval
        1.0f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "stiffness",                     // parameterID
        "Stiffness",                     // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "pratio",                        // parameterID
        "Poisson Ratio",                 // parameter name
        juce::NormalisableRange<float>(  // range
            -0.5f,
            1.5f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "alpha",                         // parameterID
        "Alpha",                         // parameter name
        juce::NormalisableRange<float>(  // range
           -0.5f,
            1.5f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "beta",                          // parameterID
        "Beta",                          // parameter name
        juce::NormalisableRange<float>(  // range
            -0.5f,
            1.5f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "xpos",                          // parameterID
        "X Position",                    // parameter name
        juce::NormalisableRange<float>(  // range
            -1.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.0f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ypos",                          // parameterID
        "Y Position",                    // parameter name
        juce::NormalisableRange<float>(  // range
            -1.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.0f  // default value
    ));

    return layout;
}

void AudioPluginAudioProcessor::createAndAppendValueTree()
{
    // generate 10 evenly spaced points on a circle with radius 1
    // auto polygon = HelperFunctions::createCircle(10, 1.0f);
    auto polygon = kac_core::geometry::normalisePolygon(
        kac_core::geometry::generateConvexPolygon(10)
    );

    // JLOG(
    //     "Number of vertices: " + std::to_string(polygon.size())
    // );
    // for (auto& vertex : polygon)
    // {
    //     JLOG(
    //         "Vertex: " + std::to_string(vertex.x) + ", " +
    //         std::to_string(vertex.y)
    //     );
    // }

    juce::ValueTree verticesTree("polygon");
    verticesTree.setProperty("id", "vertices", nullptr);

    juce::Array<juce::var> vertices;

    for (int i = 0; i < polygon.size(); ++i)
    {
        vertices.add(juce::var((polygon[i].x * 2.0f) - 1.));
        vertices.add(juce::var((polygon[i].y * 2.0f) - 1.));
    }

    verticesTree.setProperty("value", vertices, nullptr);

    mParameters.state.appendChild(verticesTree, nullptr);
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
