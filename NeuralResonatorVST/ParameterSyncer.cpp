#include "ParameterSyncer.h"
#include "JuceHeader.h"

ParameterSyncer::ParameterSyncer(juce::AudioProcessorValueTreeState& vtsRef)
    : juce::ValueTreeSynchroniser(vtsRef.state), mVTSRef(vtsRef)
{
    // Sync the state tree with the server.
    // after connecting to the server, the server will send the full state
    // tree sendFullSyncCallback();

    //  mVTSRef.state);
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

void ParameterSyncer::stateChanged(const void* encodedChange,
                                   size_t encodedChangeSize)
{
    // encode as a base64 string
    auto changesAsBase64 =
        juce::Base64::toBase64(encodedChange, encodedChangeSize);

    juce::Logger::writeToLog("ParameterSyncer::stateChanged: " +
                             mVTSRef.state.toXmlString());

    // if (mShouldSendToServer)  // !WARNING! this will not stop all messages
                              // !from being sent to the server (only some)
    {
        juce::Logger::writeToLog(
            "ParameterSyncer::stateChanged: sending to "
            "server");
        mServerThreadIfPtr->sendMessage(changesAsBase64);
    }
    // mShouldSendToServer = true;
}

void ParameterSyncer::receivedParameterChange(const juce::var& parameter)
{
    MessageManager::callAsync(
    [this, parameter]()
    {
    // update the state tree
    auto parameterID = parameter["id"].toString();
    auto newValue = parameter["value"];

    juce::Logger::writeToLog("ParameterSyncer::receivedParameterChange: " +
                             parameterID + " " + newValue.toString());

    // mShouldSendToServer = false;
    mVTSRef.getParameterAsValue(parameterID) = newValue;
    // mVTSRef.state.setPropertyExcludingListener(this, parameterID, newValue, nullptr);
    });
    // mVTSRef.getParameterAsValue(parameterID).setValue(newValue);
}
// https://forum.juce.com/t/updating-dynamicobject-property-in-valuetree-listeners-not-triggered/49628/5
void ParameterSyncer::receivedShapeChange(const juce::var& shape)
{
    MessageManager::callAsync(
        [this, shape]()
        {
            // mShouldSendToServer = false;
            auto polygonTree = mVTSRef.state.getChildWithName("polygon");
            juce::Logger::writeToLog(
                "ParameterSyncer::receivedShapeChange:polygon: ");
            // polygonTree.toXmlString());

            if (auto positions = shape.getProperty("shape", var()).getArray())
            {
                auto numPositions = positions->size();
                polygonTree.getNumChildren();

                // check these are the same
                jassert(numPositions == polygonTree.getNumChildren());

                for (int i = 0; i < numPositions; i++)
                {
                    auto vertex = (*positions)[i];

                    // get child with index i
                    auto vertexTree = polygonTree.getChild(i);

                    //! WARNING! Calling
                    //! value.getValueSource().sendChangeMessage(true) will
                    //! generate callbacks to Value::Listener::valueChanged,
                    //! not ValueTree::Listener::valueTreePropertyChanged
                    auto valueX =
                        vertexTree.getPropertyAsValue("x", nullptr, true);
                    // valueX.getValue().getDynamicObject()->setProperty(
                    // "x", juce::var(0.1f));
                    // valueX.getValueSource().sendChangeMessage(true);

                    auto valueY =
                        vertexTree.getPropertyAsValue("y", nullptr, true);
                    // valueY.getValue().getDynamicObject()->setProperty(
                    // "y", juce::var(0.1f));
                    // valueY.getValueSource().sendChangeMessage(true);
                    // auto valuey = vertexTree.getPropertyAsValue("y",
                    // nullptr);
                    valueX = vertex["x"];
                    valueY = vertex["y"];

                    // copy our vertex data into the vertex tree
                    // vertexTree.setProperty("x", vertex["x"], nullptr);
                    // vertexTree.setProperty("y", vertex["y"], nullptr);
                }
            }
        });

    // juce::Logger::writeToLog(
    //     "ParameterSyncer::receivedShapeChange: " +
    //     mVTSRef.state.toXmlString());

    // juce::Path path;
    // int res = 64;
    // parsedJson.getProperty("shape", {}).toString();
    // if (auto positions =
    //         parsedJson.getProperty("shape", var()).getArray())
    // {
    //     // get number of positions
    //     auto numPositions = positions->size();

    //     for (int i = 0; i < numPositions; i++)
    //     {
    //         auto position = (*positions)[i];
    //         auto x = float(position.getProperty("x", var()));
    //         auto y = float(position.getProperty("y", var()));

    //         // the positions are in the range [-1, 1], so we need
    //         to
    //         // scale them to the range [0, res]
    //         // and flip the y axis
    //         x = (x + 1) * 0.5 * res;
    //         y = res - ((y + 1) * 0.5 * res);

    //         // start a new subpath if this is the first position
    //         if (i == 0) { path.startNewSubPath(x, y); }
    //         else { path.lineTo(x, y); }
    //     }

    //     // close the subpath
    //     path.closeSubPath();

    // }
    // });
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