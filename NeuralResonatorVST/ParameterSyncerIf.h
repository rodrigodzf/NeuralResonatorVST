#pragma once

#include <juce_core/juce_core.h>

class ParameterSyncerIf
{
public:
    virtual ParameterSyncerIf* getParameterSyncerIfPtr() = 0;
    virtual void receivedParameterChange(const juce::var& parameter) = 0;
    virtual void receivedNewShape(const juce::var& shape) = 0;
    virtual void receivedShapeUpdate(const juce::var& shape) = 0;
    virtual void onOpen() = 0;
    virtual void onClose() = 0;
};
