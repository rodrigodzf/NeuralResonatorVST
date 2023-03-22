#include "FallbackUI.h"
#include "HelperFunctions.h"

ShapeComponent::ShapeComponent(juce::AudioProcessorValueTreeState &VTS)
    : mVts(VTS)
{
    // build the transform to go from [-1, 1] to [0, 1] with y going down
    mMainTransform =
        juce::AffineTransform::scale(0.5f, -0.5f).translated(0.5f, 0.5f);

    setOpaque(true);
    setInterceptsMouseClicks(false, true);
    valueTreeRedirected(VTS.state);
    mVts.state.addListener(this);

    // add a single vertex for the excitation
    auto xpos = mVts.state.getChildWithProperty("id", "xpos")
                    .getPropertyPointer("value");
    auto ypos = mVts.state.getChildWithProperty("id", "ypos")
                    .getPropertyPointer("value");

    auto exciterPos = juce::Point<float>(float(*xpos), float(*ypos))
                          .transformedBy(mMainTransform);

    mExciter = std::make_unique<ExciterVertex>(exciterPos);
    addAndMakeVisible(mExciter.get());
}

ShapeComponent::~ShapeComponent()
{
    mVts.state.removeListener(this);
    mPath.clear();
}

void ShapeComponent::handleCommandMessage(int commandId)
{
    auto invertedPos =
        mExciter->mRelativePos.transformedBy(mMainTransform.inverted());
    // update the tree
    if (commandId == 0)
    {
        auto polygonTree = mVts.state.getChildWithName("polygon");

        juce::Array<juce::var> vertices;

        for (auto *vertex : mVertices)
        {
            vertices.add(juce::var(invertedPos.x));
            vertices.add(juce::var(invertedPos.y));
        }

        polygonTree.setProperty("value", vertices, nullptr);
    }
    else if (commandId == 1)
    {
        // update the tree
        auto xpos = mVts.state.getChildWithProperty("id", "xpos");
        auto ypos = mVts.state.getChildWithProperty("id", "ypos");

        xpos.setProperty("value", juce::var(invertedPos.x), nullptr);
        ypos.setProperty("value", juce::var(invertedPos.y), nullptr);
    }
}

void ShapeComponent::resized()
{
    auto margin = getWidth() - getHeight();

    mScreenTransform = juce::AffineTransform()
                           .scaled(getHeight(), getHeight())
                           .translated(static_cast<float>(margin) / 2.0f, 0);

    mExciter->setCentrePosition(
        mExciter->mRelativePos.transformedBy(mScreenTransform).roundToInt()
    );
}

void ShapeComponent::paint(juce::Graphics &g)
{
    const juce::SpinLock::ScopedLockType lock(mMutex);

    // colors should have ff in front for alpha
    g.fillAll(juce::Colour::fromString("fffe746e"));
    g.setColour(juce::Colour::fromString("ff2f7df6"));

    for (auto *vertex : mVertices)
        vertex->setCentrePosition(
            vertex->mRelativePos.transformedBy(mScreenTransform).roundToInt()
        );

    // copy the path and apply a transform
    // we need to do this because paint might be run multiple times and
    // the shape will only get bigger otherwise
    auto path = mPath;
    path.applyTransform(mScreenTransform);
    g.fillPath(path);
}

void ShapeComponent::valueTreeRedirected(juce::ValueTree &redirectedTree)
{
    // JLOG("polygon valueTreeRedirected");
    const juce::SpinLock::ScopedLockType lock(mMutex);

    auto child = redirectedTree.getChildWithProperty("id", "vertices");

    if (auto *newValue = child.getPropertyPointer("value"))
    {
        auto size = newValue->size();

        mPath.clear();
        mVertices.clear();

        for (int i = 0; i < size; i += 2)
        {
            auto x = float((*newValue)[i]);
            auto y = float((*newValue)[i + 1]);

            // the positions are in the range [-1, 1], so we need
            // to scale them to the range [0, res] and flip the y
            // axis

            auto pos = juce::Point<float>(x, y).transformedBy(mMainTransform);
            if (i == 0) { mPath.startNewSubPath(pos); }
            else { mPath.lineTo(pos); }

            addAndMakeVisible(mVertices.add(new DragableVertex(pos)));
        }

        // close the subpath
        mPath.closeSubPath();
        repaint();
    }
}

void ShapeComponent::valueTreePropertyChanged(
    juce::ValueTree &changedTree,
    const juce::Identifier &changedProperty
)
{
    const juce::SpinLock::ScopedLockType lock(mMutex);

    auto treeType = changedTree.getType().toString();

    if (treeType == "polygon")
    {
        // JLOG("ShapeComponent::valueTreePropertyChanged polygon changed");
        if (auto *flattenedVertices =
                changedTree.getPropertyPointer(changedProperty))
        {
            auto size = flattenedVertices->size();

            mPath.clear();
            mVertices.clear();

            for (int i = 0; i < size; i += 2)
            {
                auto x = float((*flattenedVertices)[i]);
                auto y = float((*flattenedVertices)[i + 1]);

                // the positions are in the range [-1, 1], so we need
                // to scale them to the range [0, res] and flip the y
                // axis
                auto pos =
                    juce::Point<float>(x, y).transformedBy(mMainTransform);
                if (i == 0) { mPath.startNewSubPath(pos); }
                else { mPath.lineTo(pos); }

                addAndMakeVisible(mVertices.add(new DragableVertex(pos)));
            }

            // close the subpath
            mPath.closeSubPath();
            repaint();
        }
    }
}

///////////////////////////////////
///////////////////////////////////
///////////////////////////////////

Panel::Panel(juce::AudioProcessorValueTreeState &VTS)
    : mVTSRef(VTS)
    , mDensityAttachment(VTS, "density", mDensitySlider)
    , mStiffnessAttachment(VTS, "stiffness", mStiffnessSlider)
    , mPRatioAttachment(VTS, "pratio", mPRatioSlider)
    , mAlphaAttachment(VTS, "alpha", mAlphaSlider)
    , mBetaAttachment(VTS, "beta", mBetaSlider)
    , mNumVerticesSlider("Number of Vertices")
    , mNewShapeButton("New Shape")
{
    // add the sliders with labels
    addAndMakeVisible(mDensitySlider);

    addAndMakeVisible(mDensityLabel);
    mDensityLabel.setText("Density", juce::dontSendNotification);
    mDensityLabel.attachToComponent(&mDensitySlider, true);

    addAndMakeVisible(mStiffnessSlider);

    addAndMakeVisible(mStiffnessLabel);
    mStiffnessLabel.setText("Stiffness", juce::dontSendNotification);
    mStiffnessLabel.attachToComponent(&mStiffnessSlider, true);

    addAndMakeVisible(mPRatioSlider);

    addAndMakeVisible(mPRatioLabel);
    mPRatioLabel.setText("Poisson Ratio", juce::dontSendNotification);
    mPRatioLabel.attachToComponent(&mPRatioSlider, true);

    addAndMakeVisible(mAlphaSlider);

    addAndMakeVisible(mAlphaLabel);
    mAlphaLabel.setText("Alpha", juce::dontSendNotification);
    mAlphaLabel.attachToComponent(&mAlphaSlider, true);

    addAndMakeVisible(mBetaSlider);

    addAndMakeVisible(mBetaLabel);
    mBetaLabel.setText("Beta", juce::dontSendNotification);
    mBetaLabel.attachToComponent(&mBetaSlider, true);

    mNumVerticesSlider.setSliderStyle(juce::Slider::SliderStyle::IncDecButtons
    );
    mNumVerticesSlider.setIncDecButtonsMode(
        juce::Slider::IncDecButtonMode::incDecButtonsDraggable_Vertical
    );

    mNumVerticesSlider.setRange(3, 50, 1);
    mNumVerticesSlider.setValue(10);
    // the number of vertices slider accepts only integers
    mNumVerticesSlider.setNumDecimalPlacesToDisplay(0);

    mNumVerticesSlider.setTextBoxStyle(
        juce::Slider::TextEntryBoxPosition::TextBoxBelow,
        false,
        120,
        20
    );

    addAndMakeVisible(mNumVerticesSlider);

    // add the button
    mNewShapeButton.onClick = [this]()
    {
        auto polygonTree =
            mVTSRef.state.getOrCreateChildWithName("polygon", nullptr);
        auto polygon = kac_core::geometry::normalisePolygon(
            kac_core::geometry::generateConvexPolygon(
                static_cast<unsigned long>(mNumVerticesSlider.getValue())
            )
        );

        juce::Array<juce::var> vertices;

        for (int i = 0; i < polygon.size(); ++i)
        {
            vertices.add(juce::var((polygon[i].x * 2.0f) - 1.));
            vertices.add(juce::var((polygon[i].y * 2.0f) - 1.));
        }

        polygonTree.setProperty("value", vertices, nullptr);
    };
    addAndMakeVisible(mNewShapeButton);
}

void Panel::resized()
{
    auto area = getLocalBounds();
    auto sliderHeight = area.getHeight() / 6;

    // leave some space on the left for the label
    int labelWidth = 50;
    // set the bounds of the sliders
    mDensitySlider.setBounds(
        labelWidth,
        0 * sliderHeight,
        area.getWidth() - labelWidth,
        sliderHeight
    );
    mStiffnessSlider.setBounds(
        labelWidth,
        1 * sliderHeight,
        area.getWidth() - labelWidth,
        sliderHeight
    );
    mPRatioSlider.setBounds(
        labelWidth,
        2 * sliderHeight,
        area.getWidth() - labelWidth,
        sliderHeight
    );
    mAlphaSlider.setBounds(
        labelWidth,
        3 * sliderHeight,
        area.getWidth() - labelWidth,
        sliderHeight
    );
    mBetaSlider.setBounds(
        labelWidth,
        4 * sliderHeight,
        area.getWidth() - labelWidth,
        sliderHeight
    );

    // mStiffnessSlider.setBounds(area.removeFromTop(sliderHeight).reduced(10));
    // mPRatioSlider.setBounds(area.removeFromTop(sliderHeight).reduced(10));
    // mAlphaSlider.setBounds(area.removeFromTop(sliderHeight).reduced(10));
    // mBetaSlider.setBounds(area.removeFromTop(sliderHeight).reduced(10));

    // the new shape button and the number of vertices slider are
    // on the same row
    mNumVerticesSlider
        .setBounds(0, 5 * sliderHeight, area.getWidth() / 2, sliderHeight);
    mNewShapeButton.setBounds(
        area.getWidth() / 2,
        5 * sliderHeight,
        area.getWidth() / 2,
        sliderHeight
    );
}
