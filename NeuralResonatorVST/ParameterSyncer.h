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
     * @brief  receivedParameterChange
     * @note   called when a parameter change is received from the server,
     * from the server thread
     * @retval None
     */
    void receivedParameterChange(const juce::var& parameter) override;
    void receivedNewShape(const juce::var& shape) override;
    void receivedShapeUpdate(const juce::var& shape) override;

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
