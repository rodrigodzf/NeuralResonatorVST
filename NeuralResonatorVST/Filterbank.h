#pragma once

#include "TwoPoleInterpolated.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class Filterbank
{
public:
    Filterbank();
    ~Filterbank();

    Filterbank(int numParallel, int numBiquads);
    void setup(int numParallel, int numBiquads);

    void cleanup();
    /**
     * @brief  Set the coefficients of the filterbank
     * @note   The coefficients are expected to be in the following order:
     * b0, b1, b2, a0, a1, a2
     * @param  coeffs: The coefficients
     * @retval None
     */
    void setCoefficients(const std::vector<float>& coeffs);

    /**
     * @brief  Process a block of samples through the filterbank
     * @param  samples: The input samples
     * @param  numSamples: The number of samples
     * @retval None
     */
    void processBuffer(juce::AudioBuffer<float>& buffer);

    void setInterpolationDelta(unsigned int delta);

private:
    std::vector<std::vector<TwoPoleInterpolated>> mIIRFilters;

    int mNumParallel;
    int mNumBiquads;
    int mStride;
    unsigned int mInterpolationDelta;

    juce::SpinLock mProcessLock;

private:
    JUCE_LEAK_DETECTOR(Filterbank)
};
