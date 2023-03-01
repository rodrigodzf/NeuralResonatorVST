#pragma once

#include <vector>

class ProcessorIf
{
public:
    virtual void coefficentsChanged(const std::vector<float>& coeffs) = 0;
};