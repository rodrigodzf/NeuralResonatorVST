#pragma once

#include "ParameterSyncerIf.h"
#include "ServerThreadIf.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_audio_processors/juce_audio_processors.h>
class ParameterSyncer : public juce::ValueTreeSynchroniser,
                        public ParameterSyncerIf
{
public:
    ParameterSyncer(juce::AudioProcessorValueTreeState &vtsRef);
    ~ParameterSyncer();

    ParameterSyncerIf* getParameterSyncerIfPtr() override;
    void setServerThreadIf(ServerThreadIf *serverThreadIfPtr);
protected:
    void stateChanged(const void* encodedChange,
                      size_t encodedChangeSize) override;
    
    /**
     * @brief  Received a message from the server.
     * @note   
     * @param  message: 
     * @retval None
     */
    void receivedMessage(const juce::var& message) override;

    void receivedParameterChange(const juce::var& parameter);
    void receivedShapeChange(const juce::var& shape);

    void onOpen() override;
    void onClose() override;

private:
    /*
     * Reference to the state tree that is being synchronised.
     */
    juce::AudioProcessorValueTreeState& mVTSRef;

    /*
     * Pointer to the server thread.
     */
    ServerThreadIf* mServerThreadIfPtr;

    bool mShouldSendToServer = true;
};
