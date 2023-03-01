#pragma once

template <typename T>
class TwoPole
{
public:
    TwoPole() {}

    void set_coefficients(T b0, T b1, T b2, T a1, T a2)
    {
        m_a1 = a1;
        m_a2 = a2;
        m_b0 = b0;
        m_b1 = b1;
        m_b2 = b2;
    }

    std::vector<T> get_coefficients()
    {
        std::vector<T> coefficients = {m_b0, m_b1, m_b2, m_a1, m_a2};
        return coefficients;
    }

    T process(T x)
    {
        T y = m_b0 * x + m_s0;
        m_s0 = m_b1 * x - m_a1 * y + m_s1;
        m_s1 = m_b2 * x - m_a2 * y;
        return y;
    }

    std::string to_string()
    {
        std::stringstream ss;
        ss << "a1: " << m_a1 << " a2: " << m_a2 << " b0: " << m_b0 << " b1: " << m_b1 << " b2: " << m_b2;
        return ss.str();
    }
private:
    T m_a1, m_a2, m_b0, m_b1, m_b2 = 0.0f;
    T m_s0, m_s1 = 0.0f;
};