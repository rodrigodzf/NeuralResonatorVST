#include "BrowserComponent.h"
#include "HelperFunctions.h"

BrowserComponent::BrowserComponent(const juce::String &url)
{}

BrowserComponent::~BrowserComponent() {}

bool BrowserComponent::pageAboutToLoad(const juce::String &newURL)
{
    JLOG("BrowserComponent::pageAboutToLoad " + newURL);
    return true;
}

void BrowserComponent::pageFinishedLoading(const juce::String &url)
{
    JLOG("BrowserComponent::pageFinishedLoading " + url);
}
