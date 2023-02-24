#include "BrowserComponent.h"

BrowserComponent::BrowserComponent(const juce::String &url)
{
    goToURL(url);
}

BrowserComponent::~BrowserComponent() {}

bool BrowserComponent::pageAboutToLoad(const juce::String &newURL)
{
    juce::Logger::writeToLog("Page about to load " + newURL);
    return true;
}

void BrowserComponent::pageFinishedLoading(const juce::String &url)
{
    juce::Logger::writeToLog("Page finished loading " + url);
}
