// This plugin is only used for testing purposes.

#include "ConsolePlugin.h"


int main(int argc, char* argv[])
{
    int sampleRate = 44100;
    int blockSize = 32;
    AudioPluginAudioProcessor processor;

    // Debug all parameters
    for (auto param : processor.getParameters())
    {
        JLOG("Parameter: " + param->getName(1024) + " = " + std::to_string(param->getValue()));
    }

    processor.prepareToPlay(sampleRate, blockSize);

    // Create a stereo buffer with 512 samples
    juce::AudioBuffer<float> buffer(2, blockSize);
    
    // Fill with a single impulse
    buffer.clear();
    buffer.setSample(0, 0, 1.0f);
    buffer.setSample(1, 0, 1.0f);

    // Process the buffer
    juce::MidiBuffer midi;
    processor.processBlock(buffer, midi);

    // Check the output is not empty
    for (int i = 0; i < buffer.getNumSamples(); i++)
    {
        for (int j = 0; j < buffer.getNumChannels(); j++)
        {
            JLOG("Sample: " + std::to_string(buffer.getSample(j, i)));
        }
    }


    // Release resources
    processor.releaseResources();
    return 0;
}