#include "ParameterSyncer.h"
#include "JuceHeader.h"
#include "generate_polygon.hpp"
ParameterSyncer::ParameterSyncer(juce::AudioProcessorValueTreeState& vtsRef)
    : juce::ValueTreeSynchroniser(vtsRef.state), mVTSRef(vtsRef)
{
}

ParameterSyncer::~ParameterSyncer() {}

ParameterSyncerIf* ParameterSyncer::getParameterSyncerIfPtr()
{
    return this;
}

void ParameterSyncer::setServerThreadIf(ServerThreadIf* serverThreadIfPtr)
{
    if (serverThreadIfPtr != nullptr)
    {
        mServerThreadIfPtr = serverThreadIfPtr;
    }
}

void ParameterSyncer::stateChanged(
    const void* encodedChange,
    size_t encodedChangeSize
)
{
    // encode as a base64 string
    auto changesAsBase64 =
        juce::Base64::toBase64(encodedChange, encodedChangeSize);

    // juce::Logger::writeToLog("ParameterSyncer::stateChanged: " +
    //  mVTSRef.state.toXmlString());

    // juce::Logger::writeToLog(
    // "ParameterSyncer::stateChanged: sending to "
    // "server");
    mServerThreadIfPtr->sendMessage(changesAsBase64);
}

void ParameterSyncer::receivedParameterChange(const juce::var& parameter)
{
    MessageManager::callAsync(
        [this, parameter]()
        {
            // update the state tree
            auto parameterID = parameter["id"].toString();
            auto newValue = parameter["value"];

            // juce::Logger::writeToLog(
            // "ParameterSyncer::receivedParameterChange: " + parameterID +
            // " " + newValue.toString());

            mVTSRef.getParameterAsValue(parameterID) = newValue;
            // mVTSRef.state.setPropertyExcludingListener(this, parameterID,
            // newValue, nullptr);
        }
    );
}

void ParameterSyncer::receivedShapeChange(const juce::var& shape)
{
    MessageManager::callAsync(
        [this]()
        {
            auto polygonTree = mVTSRef.state.getOrCreateChildWithName("polygon", nullptr);
            auto polygon =
                kac_core::geometry::PolygonGenerator::generateConvexPolygon(10
                );

            juce::Array<juce::var> vertices;

            for (int i = 0; i < polygon.size(); ++i)
            {
                vertices.add(juce::var(polygon[i].x * 2.0f));
                vertices.add(juce::var(polygon[i].y * 2.0f));
            }

            polygonTree.setProperty("value", vertices, nullptr);
        }
    );
}

void ParameterSyncer::onOpen()
{
    // send the full state tree to the server
    // TODO: probably we will better wait for the cliet to request the
    // full state tree
    sendFullSyncCallback();
}

void ParameterSyncer::onClose()
{
    // do nothing
}