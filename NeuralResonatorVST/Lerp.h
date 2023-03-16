#pragma once

#include <juce_core/juce_core.h>

template <typename T>
class Lerp
{
	public:
		Lerp();
		~Lerp();

		Lerp(unsigned int delta);
		void setup(unsigned int delta);

		void setTarget(T target);
		void setValue(T value);
		void setDelta(unsigned int delta);

		T process(bool bound=true);

		T getTarget() { return m_target; }
		T getValue() { return m_val; }
		T getDelta() { return m_delta; }

		bool isFinished() { return m_counter == 0; }

	private:
		T m_val;
		T m_increment;
		int m_counter;
		unsigned int m_delta;
		T m_target;

		void calcIncrement(T target, unsigned int delta);
		T boundValue();
};

template <typename T>
inline Lerp<T>::Lerp()
{
	setup(0);
}

template <typename T>
inline Lerp<T>::Lerp(unsigned int delta)
{
	setup(delta);
}

template <typename T>
inline void Lerp<T>::setup(unsigned int delta)
{
	m_val = 0;
	m_increment = 0;
	m_counter = 0;
	m_delta = delta;
	m_target = 0;
}

template <typename T>
inline void Lerp<T>::setTarget(T target)
{
	m_target = target;
	calcIncrement(m_target, m_delta);
}

template <typename T>
inline void Lerp<T>::setValue(T value)
{
	m_val = value;
	m_counter = 0;
	m_increment = 0;
}

template <typename T>
inline void Lerp<T>::setDelta(unsigned int delta)
{
	m_delta = delta;
	calcIncrement(m_target, m_delta);
}

template <typename T>
inline void Lerp<T>::calcIncrement(T target, unsigned int delta)
{
	if(delta == 0)
		m_increment = 0.0;
	else
		m_increment = (target - m_val) / static_cast<T>(delta);

	if(m_increment == 0.0)
	{
		m_counter = 0;
		m_val = target;
	}
	else
	{
		m_counter = delta;
	}
}

template <typename T>
inline T Lerp<T>::process(bool bound)
{
	if (m_counter > 0)
	{
		m_val += m_increment;
		m_counter--;
	}
	if (bound)
		m_val = boundValue();
	return m_val;
}

template <typename T>
inline T Lerp<T>::boundValue()
{
	if (m_increment > 0 && m_val > m_target)
		m_val = m_target;
	else if (m_increment < 0 && m_val < m_target)
		m_val = m_target;
	return m_val;
}

template <typename T>
inline Lerp<T>::~Lerp()
{
	juce::Logger::writeToLog("Lerp::~Lerp");
}
