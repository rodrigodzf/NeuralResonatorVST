#pragma once

#include <juce_core/juce_core.h>

class ServerThreadIf
{
public:
    virtual void sendMessage(const juce::String &message) = 0;
    virtual ServerThreadIf* getServerThreadIfPtr() = 0;
};
