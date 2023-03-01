#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <vector>

class TorchWrapperIf
{
public:
    virtual void receivedNewShape(juce::Path &shape) = 0;
    virtual void receivedNewMaterial(const std::vector<float> &material) = 0;
    virtual void receivedNewPosition(const std::vector<float> &position) = 0;
};