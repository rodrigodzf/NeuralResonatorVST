#pragma once

#include "Lerp.h"
#include "ParallelLerp.h"
#include "TwoPole.h"

class TwoPoleInterpolator
{
	public:
		TwoPoleInterpolator();
		~TwoPoleInterpolator();

		TwoPoleInterpolator(TwoPole<double>* filter, unsigned int delta);
		void setup(TwoPole<double>* filter, unsigned int delta);

		void process();

		void setDelta(unsigned int delta);
		void setTargets(const double* targets, unsigned int nValues);
		void setTargets(std::vector<double> const& targets);
		void setTargets(double b0, double b1, double b2, double a1, double a2);

	private:
		TwoPole<double>* m_filter;
		ParallelLerp<double>* m_interpolator;
};

TwoPoleInterpolator::TwoPoleInterpolator()
{
	m_filter = NULL;
	m_interpolator = NULL;
}

TwoPoleInterpolator::TwoPoleInterpolator(TwoPole<double>* filter, unsigned int delta)
{
	setup(filter, delta);
}

void TwoPoleInterpolator::setup(TwoPole<double>* filter, unsigned int delta)
{
	m_filter = filter;
	m_interpolator = new ParallelLerp<double>(m_filter->get_n_coefficients(), delta);
	m_interpolator->setup(m_filter->get_coefficients_vector(), delta);
}

void TwoPoleInterpolator::process()
{
	if (!m_interpolator->isFinished())
	{
		m_interpolator->process();
		m_filter->set_coefficients(m_interpolator->getValuesPtr(), m_interpolator->getNValues());
	}
}

void TwoPoleInterpolator::setDelta(unsigned int delta)
{
	m_interpolator->setDelta(delta);
}

void TwoPoleInterpolator::setTargets(const double* targets, unsigned int nValues)
{
	m_interpolator->setTargets(targets, nValues);
}

void TwoPoleInterpolator::setTargets(std::vector<double> const& targets)
{
	m_interpolator->setTargets(targets);
}

void TwoPoleInterpolator::setTargets(double b0, double b1, double b2, double a1, double a2)
{
	m_interpolator->setTarget(a1, 0);
	m_interpolator->setTarget(a2, 1);
	m_interpolator->setTarget(b0, 2);
	m_interpolator->setTarget(b1, 3);
	m_interpolator->setTarget(b2, 4);
}
