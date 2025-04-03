#include "TrackView.h"

TrackView::TrackView()
{
    setSize(800, 600);
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
    // layout child components here
}
