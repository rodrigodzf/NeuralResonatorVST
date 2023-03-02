#pragma once

#include "TwoPole.h"
#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <vector>

class Filterbank
{
public:
    Filterbank(int numParallel, int numBiquads);
    ~Filterbank();

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

private:
    std::vector<std::vector<TwoPole<double>>> mIIRFilters;

    int mNumParallel;
    int mNumBiquads;
    int mStride;

    juce::SpinLock mProcessLock;

private:
    JUCE_LEAK_DETECTOR(Filterbank)
};
