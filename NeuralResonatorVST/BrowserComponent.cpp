#include "BrowserComponent.h"

BrowserComponent::BrowserComponent(
    ParameterSyncerIf *parameterSyncerIf,
    const juce::String &url
)
    : mParameterSyncerIfPtr(parameterSyncerIf)
{
}

BrowserComponent::~BrowserComponent() {}

bool BrowserComponent::pageAboutToLoad(const juce::String &newURL)
{
    juce::Logger::writeToLog("Page about to load " + newURL);
    if (newURL.startsWith(urlSchema))
    {
        if (mParameterSyncerIfPtr != nullptr)
        {
            auto message = juce::URL::removeEscapeChars(
                newURL.substring(urlSchema.length())
            );

            mParameterSyncerIfPtr->receivedMessage(juce::JSON::parse(message));
        }
        else
        {
            juce::Logger::writeToLog("No ParameterSyncerIfPtr");
            jassertfalse;
        }
        return false;
    }

    return true;
}

void BrowserComponent::pageFinishedLoading(const juce::String &url)
{
    juce::Logger::writeToLog("Page finished loading " + url);
}

ServerThreadIf *BrowserComponent::getServerThreadIfPtr()
{
    return this;
}

void BrowserComponent::sendMessage(const juce::String &message)
{
    // send message to all connections
    const auto jsonMessage = juce::JSON::toString(message, true);
    const auto url = "javascript:receiveMessageFromJuce(" + jsonMessage + ")";
    goToURL(url);
}