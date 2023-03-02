#include "Filterbank.h"

Filterbank::Filterbank(int numParallel, int numBiquads)
{
    mNumParallel = numParallel;
    mNumBiquads = numBiquads;
    mStride = mNumBiquads * 3;

    // Set up the IIR filters
    mIIRFilters.resize(mNumParallel);
    for (int i = 0; i < mNumParallel; i++)
    {
        mIIRFilters[i].resize(mNumBiquads);
    }

    for (int i = 0; i < mNumParallel; i++)
    {
        for (int j = 0; j < mNumBiquads; j++)
        {
            mIIRFilters[i][j].set_coefficients(0.0, 0.0, 0.0, 0.0, 0.0);
        }
    }
}

Filterbank::~Filterbank() {}

void Filterbank::setCoefficients(const std::vector<float>& coeffs)
{
    const juce::SpinLock::ScopedLockType lock(mProcessLock);

    for (int i = 0; i < mNumParallel; i++)
    {
        for (int j = 0; j < mNumBiquads; j++)
        {
            int idx = i * mNumBiquads * mStride + j * mStride;
            mIIRFilters[i][j].set_coefficients(
                coeffs[idx], coeffs[idx + 1], coeffs[idx + 2],
                coeffs[idx + 4], coeffs[idx + 5]);
        }
    }
}

void Filterbank::processBuffer(juce::AudioBuffer<float>& buffer)
{
    const juce::SpinLock::ScopedLockType lock(mProcessLock);

    for (int sampleIdx = 0; sampleIdx < buffer.getNumSamples(); sampleIdx++)
    {
        for (int channel = 0; channel < buffer.getNumChannels(); channel++)
        {
            double out = 0.0;
            // for each filter
            for (int i = 0; i < mNumParallel; i++)
            {
                double y = buffer.getSample(channel, sampleIdx);
                for (int j = 0; j < mNumBiquads; j++)
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