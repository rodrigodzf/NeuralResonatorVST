#include "DragableVertex.h"
#include "HelperFunctions.h"

DragableVertex::DragableVertex(const juce::Point<float> &initialPos)
    : mRelativePos(initialPos)
{
    // setInterceptsMouseClicks(false, false);
    // set the size of the vertex
    setSize(20, 20);
    setRepaintsOnMouseActivity(true);
}

void DragableVertex::paint(juce::Graphics &g)
{
    auto color = juce::Colour::fromString("ffeefa5a");
    g.setColour(
        color.withAlpha(isMouseOverOrDragging() ? 0.9f : 0.5f)
    );
    g.fillEllipse(getLocalBounds().reduced(3).toFloat());

    g.setColour(color);
    g.drawEllipse(getLocalBounds().reduced(3).toFloat(), 2.0f);
}

void DragableVertex::resized()
{
    // Just set the limits of our constrainer so that we don't drag
    // ourselves off the screen
    constrainer.setMinimumOnscreenAmounts(
        getHeight(),
        getWidth(),
        getHeight(),
        getWidth()
    );
}

void DragableVertex::moved()
{
    if (isMouseButtonDown())
    {
        auto margin = getParentWidth() - getParentHeight();
        auto transform =
            juce::AffineTransform()
                .scaled(getParentHeight(), getParentHeight())
                .translated(static_cast<float>(margin) / 2.0f, 0);
        auto centre = getBounds().getCentre().toFloat();
        mRelativePos = centre.transformedBy(transform.inverted());
    }
}

void DragableVertex::mouseUp(const juce::MouseEvent &e)
{
    // if the mouse hasn't moved, dont do anything
    if (mOldPos != mRelativePos)
    {
        // JLOG("MOUSE UP IN DRAGABLE VERTEX");
        // send a message to the parent component
        getParentComponent()->postCommandMessage(0);
    }
}

void DragableVertex::mouseDown(const juce::MouseEvent &e)
{
    mOldPos = mRelativePos;
    mDragger.startDraggingComponent(this, e);
}

void DragableVertex::mouseDrag(const juce::MouseEvent &e)
{
    mDragger.dragComponent(this, e, &constrainer);
}
