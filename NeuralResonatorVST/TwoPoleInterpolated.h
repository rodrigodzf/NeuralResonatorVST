#pragma once
#include "TwoPole.h"
#include "ParallelLerp.h"

//#define NO_INTERPOLATION

class TwoPoleInterpolated : public TwoPole<double>
{
	public:
		TwoPoleInterpolated() : m_interpolator(N_COEFFICIENTS_TWO_POLE, 0) {};
		~TwoPoleInterpolated();

		TwoPoleInterpolated(unsigned int delta);
		void setup(unsigned int delta);

		double process(double x);

		void setDelta(unsigned int delta);

		void setCoefficientValues(double b0, double b1, double b2, double a1, double a2);
		void setCoefficientTargets(double b0, double b1, double b2, double a1, double a2);

		void set_coefficients(double b0, double b1, double b2, double a1, double a2);

		void cleanup();
	private:
		ParallelLerp<double> m_interpolator;
		bool m_interpolationFinished = false;
};

inline TwoPoleInterpolated::TwoPoleInterpolated(unsigned int delta)
{
	setup(delta);
}

inline TwoPoleInterpolated::~TwoPoleInterpolated()
{
	cleanup();
}

inline void TwoPoleInterpolated::setup(unsigned int delta)
{
	//m_interpolator.setup(N_COEFFICIENTS_TWO_POLE, delta);
	setDelta(delta);
}

inline void TwoPoleInterpolated::setDelta(unsigned int delta)
{
	m_interpolator.setDelta(delta);
}

inline void TwoPoleInterpolated::setCoefficientValues(double b0, double b1, double b2, double a1, double a2)
{
	m_interpolator.setValue(0, b0);
	m_interpolator.setValue(1, b1);
	m_interpolator.setValue(2, b2);
	m_interpolator.setValue(3, a1);
	m_interpolator.setValue(4, a2);

	TwoPole<double>::set_coefficients(b0, b1, b2, a1, a2);
}

inline void TwoPoleInterpolated::setCoefficientTargets(double b0, double b1, double b2, double a1, double a2)
{
	m_interpolator.setTarget(0, b0);
	m_interpolator.setTarget(1, b1);
	m_interpolator.setTarget(2, b2);
	m_interpolator.setTarget(3, a1);
	m_interpolator.setTarget(4, a2);
}

inline double TwoPoleInterpolated::process(double x)
{
#ifndef NO_INTERPOLATION
	if(!m_interpolator.isFinished() || m_interpolator.getDelta() == 0)
	{
		m_interpolator.process();
		TwoPole<double>::set_coefficients(
			m_interpolator.getValue(0),
			m_interpolator.getValue(1),
			m_interpolator.getValue(2),
			m_interpolator.getValue(3),
			m_interpolator.getValue(4)
		);
	}
#endif
	return TwoPole<double>::process(x);
}

inline void TwoPoleInterpolated::cleanup()
{
	juce::Logger::writeToLog("TwoPoleInterpolated::cleanup()");
	m_interpolator.cleanup();
}

inline void TwoPoleInterpolated::set_coefficients(double b0, double b1, double b2, double a1, double a2)
{
#ifdef NO_INTERPOLATION
	setCoefficientValues(b0, b1, b2, a1, a2);
#else
	setCoefficientTargets(b0, b1, b2, a1, a2);
#endif
}
