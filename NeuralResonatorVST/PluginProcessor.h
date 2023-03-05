#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include "ServerThread.h"
#include "TorchWrapper.h"
#include "ProcessorIf.h"
#include "Filterbank.h"
#include "ParameterSyncer.h"
//==============================================================================
class AudioPluginAudioProcessor : public juce::AudioProcessor,
                                  public ProcessorIf
{
public:
    //==============================================================================
    AudioPluginAudioProcessor();
    ~AudioPluginAudioProcessor() override;

    //==============================================================================
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

public:
    void coefficentsChanged(const std::vector<float>& coeffs) override;
    void handleCoefficentsChanged(const std::vector<float>& coeffs);
    QueueThread mQueueThread{"plugin_processor"};

    std::map<juce::String, juce::String> mConfigMap;
    juce::File mIndexFile;

    std::unique_ptr<ServerThread> mServerThreadPtr;
    std::unique_ptr<TorchWrapper> mTorchWrapperPtr;
    std::unique_ptr<ParameterSyncer> mParameterSyncerPtr;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
private:
    std::unique_ptr<juce::FileLogger> mFileLoggerPtr;
    juce::AudioProcessorValueTreeState mParameters;
    Filterbank mFilterbank;
private:
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioPluginAudioProcessor)
};
