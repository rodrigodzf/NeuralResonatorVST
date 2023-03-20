#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include "HelperFunctions.h"
#include <kac_core.hpp>
class ShapeComponent : public juce::Component,
                       public juce::ValueTree::Listener
{
public:
    ShapeComponent(juce::AudioProcessorValueTreeState &VTS)
    {
        VTS.state.addListener(this);
        valueTreeRedirected(VTS.state);
        // setOpaque(true);
        // setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics &g) override
    {
        // colors should have ff in front for alpha
        g.fillAll(juce::Colour::fromString("ff0057b8"));
        g.setColour(juce::Colour::fromString("ffffcc00"));
        g.fillPath(mPath);
    }

    void valueTreeRedirected(juce::ValueTree &redirectedTree) override
    {
        JLOG("redirected");

        auto child = redirectedTree.getChildWithProperty("id", "vertices");

        if (auto *newValue = child.getPropertyPointer("value"))
        {
            JLOG("aaa---------------");
            auto size = newValue->size();

                mPath.clear();
                int res = this->getHeight();

                for (int i = 0; i < size; i += 2)
                {
                    JLOG("x: " + juce::String((*newValue)[i]));
                    auto x = float((*newValue)[i]);
                    auto y = float((*newValue)[i + 1]);

                    // the positions are in the range [-1, 1], so we need
                    // to scale them to the range [0, res] and flip the y
                    // axis
                    x = (x + 1) * 0.5 * res;
                    y = res - ((y + 1) * 0.5 * res);
                    if (i == 0) { mPath.startNewSubPath(x, y); }
                    else { mPath.lineTo(x, y); }
                }

                // close the subpath
                mPath.closeSubPath();
                this->repaint();
        }
    }

    void valueTreePropertyChanged(
        juce::ValueTree &changedTree,
        const juce::Identifier &changedProperty
    ) override
    {
        auto treeType = changedTree.getType().toString();

        if (treeType == "polygon")
        {
            JLOG("polygon changed");
            if (auto *flattenedVertices =
                    changedTree.getPropertyPointer(changedProperty))
            {
                auto size = flattenedVertices->size();

                mPath.clear();
                int res = this->getHeight();

                for (int i = 0; i < size; i += 2)
                {
                    auto x = float((*flattenedVertices)[i]);
                    auto y = float((*flattenedVertices)[i + 1]);

                    // the positions are in the range [-1, 1], so we need
                    // to scale them to the range [0, res] and flip the y
                    // axis
                    x = (x + 1) * 0.5 * res;
                    y = res - ((y + 1) * 0.5 * res);
                    if (i == 0) { mPath.startNewSubPath(x, y); }
                    else { mPath.lineTo(x, y); }
                }

                // close the subpath
                mPath.closeSubPath();
                this->repaint();
            }
        }
    }

private:
    juce::Path mPath;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeComponent)
};

class Panel : public juce::Component
{
public:
    Panel(juce::AudioProcessorValueTreeState &VTS)
        : mVTSRef(VTS)
        , mDensityAttachment(VTS, "density", mDensitySlider)
        , mStiffnessAttachment(VTS, "stiffness", mStiffnessSlider)
        , mPRatioAttachment(VTS, "pratio", mPRatioSlider)
        , mAlphaAttachment(VTS, "alpha", mAlphaSlider)
        , mBetaAttachment(VTS, "beta", mBetaSlider)
        , mNewShapeButton("New Shape")
    {
        // add the sliders
        addAndMakeVisible(mDensitySlider);
        addAndMakeVisible(mStiffnessSlider);
        addAndMakeVisible(mPRatioSlider);
        addAndMakeVisible(mAlphaSlider);
        addAndMakeVisible(mBetaSlider);

        // add the button
        mNewShapeButton.onClick = [this]()
        {
            auto polygonTree =
                mVTSRef.state.getOrCreateChildWithName("polygon", nullptr);
            auto polygon = kac_core::geometry::normalisePolygon(
                kac_core::geometry::generateConvexPolygon(10)
            );
            // kac_core::geometry::generateIrregularStar(float(shape["value"])
            // kac_core::geometry::generatePolygon(float(shape["value"])

            juce::Array<juce::var> vertices;

            for (int i = 0; i < polygon.size(); ++i)
            {
                vertices.add(juce::var((polygon[i].x * 2.0f) - 1.));
                vertices.add(juce::var((polygon[i].y * 2.0f) - 1.));
            }

            polygonTree.setProperty("value", vertices, nullptr);
        };
        addAndMakeVisible(mNewShapeButton);

        // setOpaque(true);
        // setInterceptsMouseClicks(false, false);
    }

    void paint(juce::Graphics &g) override
    {
        // g.fillAll(juce::Colours::lightgrey);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        auto sliderHeight = area.getHeight() / 6;
        mDensitySlider.setBounds(area.removeFromTop(sliderHeight));
        mStiffnessSlider.setBounds(area.removeFromTop(sliderHeight));
        mPRatioSlider.setBounds(area.removeFromTop(sliderHeight));
        mAlphaSlider.setBounds(area.removeFromTop(sliderHeight));
        mBetaSlider.setBounds(area.removeFromTop(sliderHeight));
        mNewShapeButton.setBounds(area.removeFromTop(sliderHeight));
    }

private:
    juce::AudioProcessorValueTreeState &mVTSRef;
    juce::Slider mDensitySlider, mStiffnessSlider, mPRatioSlider,
        mAlphaSlider, mBetaSlider;

    juce::AudioProcessorValueTreeState::SliderAttachment mDensityAttachment,
        mStiffnessAttachment, mPRatioAttachment, mAlphaAttachment,
        mBetaAttachment;

    juce::TextButton mNewShapeButton;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Panel)
};