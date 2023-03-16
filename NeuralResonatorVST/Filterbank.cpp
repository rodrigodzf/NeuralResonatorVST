#include "Filterbank.h"
Filterbank::Filterbank()
{
    mNumParallel = 0;
    mNumBiquads = 0;
    mStride = 0;
}

Filterbank::Filterbank(int numParallel, int numBiquads)
{
	setup(numParallel, numBiquads);
}

void Filterbank::setup(int numParallel, int numBiquads)
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
            mIIRFilters[i][j].setCoefficientValues(0.0, 0.0, 0.0, 0.0, 0.0);
        }
    }
}

Filterbank::~Filterbank()
{
    cleanup();
}

void Filterbank::cleanup()
{
	juce::Logger::writeToLog("Filterbank::cleanup()");
	for (int i = 0; i < mNumParallel; i++)
	{
		for (int j = 0; j < mNumBiquads; j++)
		{
			mIIRFilters[i][j].cleanup();
		}
	}
}

void Filterbank::setCoefficients(const std::vector<float>& coeffs)
{
    const juce::SpinLock::ScopedLockType lock(mProcessLock);
    for (int i = 0; i < mNumParallel; i++)
    {
        for (int j = 0; j < mNumBiquads; j++)
        {
            int idx = i * mNumBiquads * mStride + j * mStride;
	    mIIRFilters[i][j].set_coefficients(
		    coeffs[idx],
		    coeffs[idx + 1],
		    coeffs[idx + 2],
		    coeffs[idx + 4],
		    coeffs[idx + 5]
	    );
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
void Filterbank::setInterpolationDelta(unsigned int delta)
{
    mInterpolationDelta = delta;
    for (int i = 0; i < mNumParallel; i++)
    {
	for (int j = 0; j < mNumBiquads; j++)
	{
	    mIIRFilters[i][j].setDelta(mInterpolationDelta);
	}
    }
}
