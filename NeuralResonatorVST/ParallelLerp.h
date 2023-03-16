#pragma once
#include <vector>

#include "Lerp.h"
#include <juce_core/juce_core.h>

template <typename T>
class ParallelLerp
{
	public:
		ParallelLerp() {};
		~ParallelLerp();

		ParallelLerp(unsigned int nInterp, unsigned int delta);

		void cleanup();
		void setup(unsigned int nInterp, unsigned int delta);

		void setup(const T* values, unsigned int nValues, unsigned int delta);
		template<typename A>
		void setup(std::vector<T,A> const& values, unsigned int delta);
		template<size_t N>
		void setup(T const (&values)[N], unsigned int delta);


		bool setValues(const T* values, unsigned int nValues);
		template<typename A>
		bool setValues(std::vector<T,A> const& values);
		template<size_t N>
		bool setValues(T const (&values)[N]);

		bool setValue(unsigned int index, T value);

		bool setTargets(const T* targets, unsigned int nValues);
		template<typename A>
		bool setTargets(std::vector<T,A> const& targets);
		template<size_t N>
		bool setTargets(T const (&targets)[N]);

		bool setTarget(unsigned int index, T target);

		void setDelta(unsigned int delta);
		unsigned int getDelta();

		bool isFinished();

		const T* process();
		std::vector<T> getValues();
		const T* getValuesPtr();
		unsigned int getNValues();
		unsigned int getNInterpolators();

		T getTarget(unsigned int index);
		T getValue(unsigned int index);

		Lerp<T>* getInterpolator(unsigned int index);

	private:
		std::vector<Lerp<T>*> m_interpolators;
		std::vector<T> m_values;
};

template<typename T>
inline ParallelLerp<T>::ParallelLerp(unsigned int nInterp, unsigned int delta)
{
	setup(nInterp, delta);
}

template<typename T>
inline void ParallelLerp<T>::cleanup()
{
	juce::Logger::writeToLog("ParallelLerp::cleanup()");
	for (unsigned int i = 0; i < m_interpolators.size(); i++)
	{
		delete m_interpolators[i];
	}
	//m_values.clear();
}

template<typename T>
inline ParallelLerp<T>::~ParallelLerp()
{
	cleanup();
}

template<typename T>
inline void ParallelLerp<T>::setup(unsigned int nInterp, unsigned int delta)
{
	for (unsigned int i = 0; i < nInterp; i++)
	{
		m_interpolators.push_back(new Lerp<T>(delta));
		m_values.push_back(T());
	}
}

template<typename T>
inline void ParallelLerp<T>::setup(const T* values, unsigned int nValues, unsigned int delta)
{
	for (unsigned int i = 0; i < nValues; i++)
	{
		m_interpolators.push_back(new Lerp<T>(delta));
		m_values.push_back(values[i]);
		m_interpolators[i]->setValue(m_values[i]);
	}
}

template<typename T>
template<typename A>
inline void ParallelLerp<T>::setup(std::vector<T,A> const& values, unsigned int delta)
{
	setup(values.data(), values.size(), delta);
}

template<typename T>
template<size_t N>
inline void ParallelLerp<T>::setup(T const (&values)[N], unsigned int delta)
{
	setup(values, N*sizeof(T), delta);
}

template<typename T>
inline bool ParallelLerp<T>::setValues(const T* values, unsigned int nValues)
{
	if(nValues != m_interpolators.size())
		return false;
	for (unsigned int i = 0; i < nValues; i++)
	{
		m_interpolators[i]->setValues(values[i]);
		m_values[i] = values[i];
	}
	return true;
}

template<typename T>
template<typename A>
inline bool ParallelLerp<T>::setValues(std::vector<T,A> const& values)
{
	return setValues(values.data(), values.size());
}

template<typename T>
template<size_t N>
inline bool ParallelLerp<T>::setValues(T const (&values)[N])
{
	return setValues(values, N*sizeof(T));
}

template<typename T>
inline bool ParallelLerp<T>::setTargets(const T* targets, unsigned int nTargets)
{
	if(nTargets != m_interpolators.size())
		return false;

	for (unsigned int i = 0; i < nTargets; i++)
	{
		m_interpolators[i]->setTarget(targets[i]);
	}
	return true;
}

template<typename T>
template<typename A>
inline bool ParallelLerp<T>::setTargets(std::vector<T,A> const& targets)
{
	return setTargets(targets.data(), targets.size());
}

template<typename T>
template<size_t N>
inline bool ParallelLerp<T>::setTargets(T const (&targets)[N])
{
	return setTargets(targets, N*sizeof(T));
}

template<typename T>
inline void ParallelLerp<T>::setDelta(unsigned int delta)
{
	juce::Logger::writeToLog("ParallelLerp::setDelta " + std::to_string(delta));
	for (unsigned int i = 0; i < m_interpolators.size(); i++)
	{
		m_interpolators[i]->setDelta(delta);
	}
}

template<typename T>
inline bool ParallelLerp<T>::isFinished()
{
	for (unsigned int i = 0; i < m_interpolators.size(); i++)
	{
		if(!m_interpolators[i]->isFinished())
			return false;
	}
	return true;
}

template<typename T>
inline const T* ParallelLerp<T>::process()
{
	for (unsigned int i = 0; i < m_interpolators.size(); i++)
	{
		m_values[i] = m_interpolators[i]->process();
	}
	return m_values.data();
}

template<typename T>
inline std::vector<T> ParallelLerp<T>::getValues()
{
	return m_values;
}

template<typename T>
inline const T* ParallelLerp<T>::getValuesPtr()
{
	return m_values.data();
}

template<typename T>
inline unsigned int ParallelLerp<T>::getNValues()
{
	return m_values.size();
}

template<typename T>
inline Lerp<T>* ParallelLerp<T>::getInterpolator(unsigned int index)
{
	if(index >= m_interpolators.size())
		return nullptr;
	return m_interpolators[index];
}

template<typename T>
inline T ParallelLerp<T>::getTarget(unsigned int index)
{
	if(index >= m_interpolators.size())
		return T();
	return m_interpolators[index]->getTarget();
}

template<typename T>
inline bool ParallelLerp<T>::setValue(unsigned int index, T value)
{
	if(index >= m_interpolators.size())
		return false;
	m_interpolators[index]->setValue(value);
	m_values[index] = value;
	return true;
}

template<typename T>
inline bool ParallelLerp<T>::setTarget(unsigned int index, T target)
{
	if(index >= m_interpolators.size())
	{
		return false;
	}
	m_interpolators[index]->setTarget(target);
	return true;
}

template<typename T>
inline T ParallelLerp<T>::getValue(unsigned int index)
{
	if(index >= m_interpolators.size())
		return T();
	return m_values[index];
}

template<typename T>
inline unsigned int ParallelLerp<T>::getNInterpolators()
{
	return m_interpolators.size();
}

template<typename T>
inline unsigned int ParallelLerp<T>::getDelta()
{
	if(m_interpolators.size() == 0)
		return 0;
	return m_interpolators[0]->getDelta();
}
