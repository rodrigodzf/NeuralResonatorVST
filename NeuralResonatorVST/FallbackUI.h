#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HelperFunctions.h"
#include <geometry/generate_polygon.hpp>
#include <geometry/morphisms.hpp>
#include <geometry/lines.hpp>
#include <memory>
#include <vector>
#include "DragableVertex.h"

class ExciterVertex : public DragableVertex
{
public:
    ExciterVertex(const juce::Point<float>& initialPos)
        : DragableVertex(initialPos)
    {
    }

    void paint(juce::Graphics& g) override
    {
        g.setColour(juce::Colours::black.withAlpha(
            isMouseOverOrDragging() ? 0.9f : 0.5f
        ));
        g.fillEllipse(getLocalBounds().reduced(3).toFloat());

        g.setColour(juce::Colours::black);
        g.drawEllipse(getLocalBounds().reduced(3).toFloat(), 2.0f);
    }

    void mouseUp(const juce::MouseEvent& e) override
    {
        JLOG("MOUSE UP");
        // send a message to the parent component
        getParentComponent()->postCommandMessage(1);
    }
};

class ShapeComponent : public juce::Component,
                       public juce::ValueTree::Listener
{
public:
    ShapeComponent(juce::AudioProcessorValueTreeState& VTS);

    ~ShapeComponent();

    void handleCommandMessage(int commandId) override;

    void resized() override;

    void paint(juce::Graphics& g) override;

    void valueTreeRedirected(juce::ValueTree& redirectedTree) override;

    void valueTreePropertyChanged(
        juce::ValueTree& changedTree,
        const juce::Identifier& changedProperty
    ) override;

private:
    juce::Path mPath;
    juce::SpinLock mMutex;
    juce::AudioProcessorValueTreeState& mVts;
    juce::OwnedArray<DragableVertex> mVertices;
    std::unique_ptr<ExciterVertex> mExciter;

    juce::AffineTransform mMainTransform;
    juce::AffineTransform mScreenTransform;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeComponent)
};

class Panel : public juce::Component
{
public:
    Panel(juce::AudioProcessorValueTreeState& VTS);
    void resized() override;

private:
    juce::AudioProcessorValueTreeState& mVTSRef;
    juce::Slider mDensitySlider, mStiffnessSlider, mPRatioSlider,
        mAlphaSlider, mBetaSlider;

    juce::Label mDensityLabel, mStiffnessLabel, mPRatioLabel, mAlphaLabel,
        mBetaLabel;

    juce::AudioProcessorValueTreeState::SliderAttachment mDensityAttachment,
        mStiffnessAttachment, mPRatioAttachment, mAlphaAttachment,
        mBetaAttachment;

    juce::Slider mNumVerticesSlider;
    juce::TextButton mNewShapeButton;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Panel)
};