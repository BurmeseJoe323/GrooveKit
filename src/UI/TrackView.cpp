#include "TrackView.h"

TrackView::TrackView() {
    setSize(800, 600);
    for (auto* button : buttons) {
        addAndMakeVisible(button);
    }
}

TrackView::~TrackView() = default;

void TrackView::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(20.0f);
    g.drawText("TrackView", getLocalBounds(), juce::Justification::centred, true);
}

void TrackView::resized()
{
    // This is the Flex Box object for Buttons
    FlexBox fb;
    fb.flexWrap = juce::FlexBox::Wrap::wrap;
    fb.justifyContent = juce::FlexBox::JustifyContent::center;
    fb.alignContent = juce::FlexBox::AlignContent::center;
    for (auto *b :buttons) {
        fb.items.add (juce::FlexItem(*b).withMinWidth(50.0f).withMinHeight(50.0f));
    }

    fb.performLayout(getLocalBounds());
    // layout child components here
}

