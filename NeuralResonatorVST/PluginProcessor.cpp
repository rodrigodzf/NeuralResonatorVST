#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "HelperFunctions.h"

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
{
    // Set up the logger
    mFileLoggerPtr.reset(juce::FileLogger::createDefaultAppLogger(
        "NeuralResonatorVST", "log.txt", "NeuralResonatorVST log file"));
    juce::Logger::setCurrentLogger(mFileLoggerPtr.get());
    juce::Logger::writeToLog("AudioPluginAudioProcessor constructor");

    // Get config file and index file path
    mConfigMap = HelperFunctions::getConfig();
    mIndexFile = HelperFunctions::saveLoadIndexFile();

    // Set up the IIR filters
    mIIRFilters.resize(nParallelFilters);
    for (int i = 0; i < nParallelFilters; i++)
    {
        mIIRFilters[i].resize(nBiquadFilters);
    }

    for (int i = 0; i < nParallelFilters; i++)
    {
        for (int j = 0; j < nBiquadFilters; j++)
        {
            mIIRFilters[i][j].set_coefficients(0.0, 0.0, 0.0, 0.0, 0.0);
        }
    }

    // Start the threads in order (from the bottom up)
    // Start this thread
    mQueueThread.startThread();

    // Start Torch wrapper thread
    mTorchWrapperPtr.reset(new TorchWrapper(this));
    mTorchWrapperPtr->loadModel(mConfigMap["encoder_path"].toStdString(),
                                TorchWrapper::ModelType::ShapeEncoder);
    mTorchWrapperPtr->loadModel(mConfigMap["fc_path"].toStdString(),
                                TorchWrapper::ModelType::FC);

    // Start WS server thread
    mServerThreadPtr.reset(
        new ServerThread(mTorchWrapperPtr->getTorchWrapperIfPtr()));
}

AudioPluginAudioProcessor::~AudioPluginAudioProcessor()
{
    juce::Logger::writeToLog("AudioPluginAudioProcessor destructor");

    // Stop the threads in reverse order (from the top down)
    mServerThreadPtr.reset();
    mServerThreadPtr = nullptr;

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

void AudioPluginAudioProcessor::changeProgramName(int index,
                                                  const juce::String& newName)
{
    juce::ignoreUnused(index, newName);
}

//==============================================================================
void AudioPluginAudioProcessor::prepareToPlay(double sampleRate,
                                              int samplesPerBlock)
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
    const BusesLayout& layouts) const
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

void AudioPluginAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                             juce::MidiBuffer& midiMessages)
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
    for (int sampleIdx = 0; sampleIdx < buffer.getNumSamples(); sampleIdx++)
    {
        for (int channel = 0; channel < totalNumOutputChannels; channel++)
        {
            double out = 0.0;
            // for each filter
            for (int i = 0; i < mIIRFilters.size(); i++)
            {
                double y = buffer.getSample(channel, sampleIdx);
                for (int j = 0; j < mIIRFilters[i].size(); j++)
                {
                    y = mIIRFilters[i][j].process(y);
                }

                // add to output
                out += y;
            }

            buffer.setSample(channel, sampleIdx, static_cast<float>(out));
        }
    }
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
    juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory
    // block. You could do that either as raw data, or use the XML or
    // ValueTree classes as intermediaries to make it easy to save and load
    // complex data.
    juce::ignoreUnused(destData);
}

void AudioPluginAudioProcessor::setStateInformation(const void* data,
                                                    int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory
    // block, whose contents will have been created by the
    // getStateInformation() call.
    juce::ignoreUnused(data, sizeInBytes);
}

void AudioPluginAudioProcessor::coefficentsChanged(
    const std::vector<float>& coeffs)
{
    mQueueThread.getIoService().post(
        [this, coeffs]() { this->handleCoefficentsChanged(coeffs); });
}

void AudioPluginAudioProcessor::handleCoefficentsChanged(
    const std::vector<float>& coefficients)
{
    juce::Logger::writeToLog("Coefficents changed");
    // juce::Logger::writeToLog("Number of coefficients: " +
    //                          std::to_string(coefficients.size()));
    int n_parallel = mIIRFilters.size();
    int n_biquads = mIIRFilters[0].size();
    int stride = n_biquads * 3;
    for (int i = 0; i < n_parallel; i++)
    {
        for (int j = 0; j < n_biquads; j++)
        {
            mIIRFilters[i][j].set_coefficients(
                coefficients[i * n_biquads * stride + j * stride + 0],
                coefficients[i * n_biquads * stride + j * stride + 1],
                coefficients[i * n_biquads * stride + j * stride + 2],
                coefficients[i * n_biquads * stride + j * stride + 4],
                coefficients[i * n_biquads * stride + j * stride + 5]);

            // print coefficients
            // juce::Logger::writeToLog("The filter coefficients are: " +
            //  mIIRFilters[i][j].to_string());
        }
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPluginAudioProcessor();
}
