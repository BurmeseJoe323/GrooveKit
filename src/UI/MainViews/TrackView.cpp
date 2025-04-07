#include "TrackView.h"

TrackView::TrackView() {
    setSize(800, 600);

    menu.setColour(Label::backgroundColourId, juce::Colours::white);
    track.setColour(Label::backgroundColourId, juce::Colours::blue);
    menu.setColour(Label::textColourId, juce::Colours::red);
    track.setColour(Label::textColourId, juce::Colours::red);
    addAndMakeVisible(menu);
    addAndMakeVisible(track);
    addAndMakeVisible(trackComponent);


    // for (auto* button : buttons) {
    //     addAndMakeVisible(button);
    // }
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
    auto area = getLocalBounds();
    auto menuHeight = 20;
    menu.setBounds(area.removeFromTop(menuHeight));

    auto trackHeight = 50;
    track.setBounds(area.removeFromTop(trackHeight));

    auto trackComponentHeight = 100;
    trackComponent.setBounds(area.removeFromTop(trackComponentHeight));

    // This is the Flex Box object for Buttons
    // FlexBox fb;
    // fb.flexWrap = juce::FlexBox::Wrap::wrap;
    // fb.justifyContent = juce::FlexBox::JustifyContent::center;
    // fb.alignContent = juce::FlexBox::AlignContent::center;
    // for (auto *b :buttons) {
    //     fb.items.add (juce::FlexItem(*b).withMinWidth(50.0f).withMinHeight(50.0f));
    // }

    // fb.performLayout(getLocalBounds());
    // layout child components here
}

