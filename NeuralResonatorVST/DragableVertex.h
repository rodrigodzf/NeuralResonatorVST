#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

class DragableVertex : public juce::Component
{
public:
    DragableVertex(const juce::Point<float> &initialPos);

    void paint(juce::Graphics &g) override;
    void resized() override;
    void moved() override;
    void mouseUp(const juce::MouseEvent &e) override;
    void mouseDown(const juce::MouseEvent &e) override;
    void mouseDrag(const juce::MouseEvent &e) override;

public:
    juce::Point<float> mRelativePos;
private:
    juce::Point<float> mOldPos;

    juce::ComponentBoundsConstrainer constrainer;
    juce::ComponentDragger mDragger;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DragableVertex)
};