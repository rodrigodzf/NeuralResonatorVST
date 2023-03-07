#include <random>
#include <time.h>
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
    , mFilterbank(32, 2)
    , mParameters(
          *this,    // processor to connect to
          nullptr,  // undo manager
          juce::Identifier("NeuralResonatorVSTParams"),  // identifier
          createParameterLayout()                        // parameter layout
      )
{
    // Set up the logger
    mFileLoggerPtr.reset(juce::FileLogger::createDefaultAppLogger(
        "NeuralResonatorVST",
        "log.txt",
        "NeuralResonatorVST log file"
    ));
    juce::Logger::setCurrentLogger(mFileLoggerPtr.get());
    juce::Logger::writeToLog("AudioPluginAudioProcessor constructor");

    // Get config file and index file path
    mConfigMap = HelperFunctions::getConfig();
    mIndexFile = HelperFunctions::saveLoadIndexFile();

    // Create and append the polygon valueTree to the parameter tree
    createAndAppendValueTree();

    juce::Logger::writeToLog(
        juce::JSON::toString(HelperFunctions::convertToVar(mParameters.state))
    );

    // initialize the torch wrapper
    juce::Logger::writeToLog("Initializing torch wrapper");
    mTorchWrapperPtr.reset(new TorchWrapper(this, mParameters));
    mTorchWrapperPtr->loadModel(
        mConfigMap["encoder_path"].toStdString(),
        TorchWrapper::ModelType::ShapeEncoder
    );
    mTorchWrapperPtr->loadModel(
        mConfigMap["fc_path"].toStdString(),
        TorchWrapper::ModelType::FC
    );

    // Initialize the parameter syncer
    juce::Logger::writeToLog("Initializing parameter syncer");
    mParameterSyncerPtr.reset(new ParameterSyncer(mParameters));

    // Initialize the server thread
    juce::Logger::writeToLog("Initializing server thread");
    mServerThreadPtr.reset(
        new ServerThread(mParameterSyncerPtr->getParameterSyncerIfPtr())
    );

    // Pass the server thread to the parameter syncer
    mParameterSyncerPtr->setServerThreadIf(
        mServerThreadPtr->getServerThreadIfPtr()
    );

    // Start the threads in order (from the bottom up)
    mQueueThread.startThread();
    mTorchWrapperPtr->startThread();
    mServerThreadPtr->startThread();
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    juce::Logger::writeToLog("AudioPluginAudioProcessor destructor");

    // Stop the threads in reverse order (from the top down)
    mServerThreadPtr.reset();
    mServerThreadPtr = nullptr;

    mParameterSyncerPtr.reset();
    mParameterSyncerPtr = nullptr;

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
    juce::Logger::writeToLog("prepareToPlay");
    // Use this method as the place to do any pre-playback
    // initialisation that you need..
}

void AudioPluginAudioProcessor::releaseResources()
{
    juce::Logger::writeToLog("releaseResources");
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
    juce::ignoreUnused(midiMessages);

    // juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

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
    juce::Logger::writeToLog("getStateInformation");
    auto state = mParameters.copyState();
    MemoryOutputStream stream(destData, false);
    state.writeToStream(stream);
}

void AudioPluginAudioProcessor::setStateInformation(
    const void* data,
    int sizeInBytes
)
{
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the
    // getStateInformation() call.
    juce::Logger::writeToLog("setStateInformation");
    MemoryInputStream stream(data, static_cast<size_t>(sizeInBytes), false);
    mParameters.replaceState(ValueTree::readFromStream(stream));
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
    juce::Logger::writeToLog("Coefficents changed");
    // juce::Logger::writeToLog("Number of coefficients: " +
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
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
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
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "alpha",                         // parameterID
        "Alpha",                         // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "beta",                          // parameterID
        "Beta",                          // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "xpos",                          // parameterID
        "X Position",                    // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        "ypos",                          // parameterID
        "Y Position",                    // parameter name
        juce::NormalisableRange<float>(  // range
            0.0f,
            1.0f,
            0.01f
        ),    // min, max, interval
        0.5f  // default value
    ));

    return layout;
}

void AudioPluginAudioProcessor::createAndAppendValueTree()
{
    // generate 10 evenly spaced points on a circle with radius 1
    // auto polygon = HelperFunctions::createCircle(10, 1.0f);
    std::default_random_engine random_engine =
        std::default_random_engine(time(NULL));
    auto polygon =
        kac_core::geometry::generateConvexPolygon(10, random_engine);

    // juce::Logger::writeToLog(
    //     "Number of vertices: " + std::to_string(polygon.size())
    // );
    // for (auto& vertex : polygon)
    // {
    //     juce::Logger::writeToLog(
    //         "Vertex: " + std::to_string(vertex.x) + ", " +
    //         std::to_string(vertex.y)
    //     );
    // }

    juce::ValueTree verticesTree("polygon");
    verticesTree.setProperty("id", "vertices", nullptr);

    juce::Array<juce::var> vertices;

    for (int i = 0; i < polygon.size(); ++i)
    {
        vertices.add(juce::var(polygon[i].x * 2.0f));
        vertices.add(juce::var(polygon[i].y * 2.0f));
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
