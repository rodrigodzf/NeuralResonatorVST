#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

class RemoteParameterAttachment
{
public:
    RemoteParameterAttachment(
        juce::RangedAudioParameter& parameter,  // parameter
        std::function<void(float, bool)>
            parameterChangedCallback,  // parameterChangedCallback
        juce::UndoManager* um          // undoManager
        )
        : attachment(
              parameter,                         // parameter
              [this](float f) { setValue(f); },  // parameterChangedCallback
              um                                 // undoManager
          )
        , mParameterChangedCallback(parameterChangedCallback)
    {
    }
    ~RemoteParameterAttachment() {}

    // sets the value of the GUI element
    void setValue(float newValue)
    {
        // juce::Logger::writeToLog("RemoteParameterAttachment::setValue");

        mParameterChangedCallback(newValue, shouldSendToServer);
    }

    void remoteBeginGesture()
    {
        juce::Logger::writeToLog(
            "RemoteParameterAttachment::remoteBeginGesture");
        attachment.beginGesture();
    }

    void remoteEndGesture()
    {
        juce::Logger::writeToLog(
            "RemoteParameterAttachment::remoteEndGesture");
        attachment.endGesture();
    }

    void remoteValueChanged(float newValue)
    {
        // juce::Logger::writeToLog(
        // "RemoteParameterAttachment::remoteValueChanged");
        shouldSendToServer = false;
        attachment.setValueAsPartOfGesture(newValue);
        shouldSendToServer = true;
    }

    void sendInitialUpdate()
    {
        shouldSendToServer = true;
        attachment.sendInitialUpdate();
    }
public:
    bool shouldSendToServer = true;
    std::function<void(float, bool)> mParameterChangedCallback;

private:
    juce::ParameterAttachment attachment;
};
