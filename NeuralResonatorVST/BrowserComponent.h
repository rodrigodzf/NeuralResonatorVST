#pragma once

#include <juce_gui_extra/juce_gui_extra.h>
#include "ParameterSyncerIf.h"
#include "ServerThreadIf.h"

class BrowserComponent : public juce::WebBrowserComponent,
                         public ServerThreadIf
{
public:
    BrowserComponent(
        ParameterSyncerIf *parameterSyncerIf,
        const juce::String &url = "http://localhost:3000"
    );
    ~BrowserComponent();

    void sendMessage(const juce::String &message) override;
    ServerThreadIf *getServerThreadIfPtr() override;

private:
    const juce::String urlSchema = "juce://";
    ParameterSyncerIf *mParameterSyncerIfPtr;

    bool pageAboutToLoad(const juce::String &newURL) override;
    void pageFinishedLoading(const juce::String &url) override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BrowserComponent)
};
