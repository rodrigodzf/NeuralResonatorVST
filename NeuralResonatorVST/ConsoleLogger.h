#pragma once

#include <juce_core/juce_core.h>

class ConsoleLogger : public juce::Logger
{
    void logMessage(const juce::String& message) override
    {
        std::cout << message << std::endl;

#if JUCE_WINDOWS
        Logger::outputDebugString(message);
#endif
    }
};
