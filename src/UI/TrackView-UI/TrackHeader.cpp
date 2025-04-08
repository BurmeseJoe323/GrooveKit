//
// Created by ikera on 4/8/2025.
//

#include "TrackHeader.h"

#include <tracktion_engine/utilities/tracktion_Identifiers.h>

TrackHeader::TrackHeader() {
  // add everything to view
  addAndMakeVisible(addClip);
  addAndMakeVisible(trackNameLabel);
  // setup trackNameLabel
  trackNameLabel.setFont(juce::Font(15.0f));
  trackNameLabel.setColour(juce::Label::textColourId, juce::Colours::black);
  trackNameLabel.setJustificationType(juce::Justification::centred);
  trackNameLabel.setText("Track", juce::dontSendNotification);
};

TrackHeader::~TrackHeader() = default;

void TrackHeader::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::beige);
};

void TrackHeader::resized() {
  // this will be the main layout
  // set as vertical layout
  juce::FlexBox mainFb;
  mainFb.flexDirection = juce::FlexBox::Direction::column;
  mainFb.alignItems = juce::FlexBox::AlignItems::stretch;

  auto area = getLocalBounds().reduced(5);

  // Row 1: Track name
  mainFb.items.add(juce::FlexItem(trackNameLabel)
    .withWidth(50)
    .withHeight(30)
    .withAlignSelf(juce::FlexItem::AlignSelf::flexStart));

  // Row 2: Add clip button
  juce::FlexBox buttonRowFB;
  buttonRowFB.flexDirection = juce::FlexBox::Direction::row;
  buttonRowFB.justifyContent = juce::FlexBox::JustifyContent::flexEnd;

  buttonRowFB.items.add(juce::FlexItem(addClip)
        .withWidth(50)
        .withHeight(30));

  // Add the button row as a FlexItem to the main layout
  mainFb.items.add(juce::FlexItem(buttonRowFB)
      .withHeight(30)); // Fixed height for the button row

  // Perform layout
  mainFb.performLayout(area);
}