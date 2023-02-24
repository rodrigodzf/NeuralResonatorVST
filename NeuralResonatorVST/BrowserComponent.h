#pragma once

#include <juce_gui_extra/juce_gui_extra.h>

class BrowserComponent : public juce::WebBrowserComponent
{
public:
    BrowserComponent(const juce::String &url = "http://localhost:3000");
    ~BrowserComponent();

private:
    bool pageAboutToLoad(const juce::String &newURL) override;
    void pageFinishedLoading(const juce::String &url) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
