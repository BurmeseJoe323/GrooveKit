#include "TrackHeaderComponent.h"

TrackHeaderComponent::TrackHeaderComponent() {
  // add everything to view
  addAndMakeVisible(addClip);
  addAndMakeVisible(deleteTrackButton);
  addAndMakeVisible(pianoRollButton);
  addAndMakeVisible(trackNameLabel);
  addAndMakeVisible(muteTrackButton);
  addAndMakeVisible(soloTrackButton);

  addClip.onClick = [this]() {
    listeners.call([](Listener& l) { l.onAddClipClicked(); });
  };

  deleteTrackButton.onClick = [this]() {
    listeners.call([](Listener& l) { l.onDeleteTrackClicked(); });
  };

  pianoRollButton.onClick = [this]() {
    listeners.call([](Listener& l) { l.onPianoRollClicked(); });
  };

  muteTrackButton.setClickingTogglesState(true);
  muteTrackButton.onClick = [this]() {
    const bool nowMuted = muteTrackButton.getToggleState();
    updateMuteButtonVisuals();
    listeners.call([&](Listener& l) { l.onMuteToggled(nowMuted); });
  };

  soloTrackButton.setClickingTogglesState(true);
  soloTrackButton.onClick = [this]{
    const bool nowSolo = soloTrackButton.getToggleState();
    updateSoloButtonVisuals();
    listeners.call([&](Listener& l){ l.onSoloToggled(nowSolo); });
  };

  soloTrackButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow.withAlpha(0.9f));
  soloTrackButton.setColour(juce::TextButton::buttonColourId,  juce::Colours::lightgrey);
  soloTrackButton.setColour(juce::TextButton::textColourOnId,  juce::Colours::black);
  soloTrackButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

  // setup trackNameLabel
  trackNameLabel.setFont(juce::Font(15.0f));
  trackNameLabel.setColour(juce::Label::textColourId, juce::Colours::black);
  trackNameLabel.setJustificationType(juce::Justification::centred);
  trackNameLabel.setText("Track", juce::dontSendNotification);
};

TrackHeaderComponent::~TrackHeaderComponent() = default;

void TrackHeaderComponent::setMuted(bool shouldBeMuted) {
  muteTrackButton.setToggleState(shouldBeMuted, juce::dontSendNotification);
  updateMuteButtonVisuals();
}
bool TrackHeaderComponent::isMuted() const {
  return muteTrackButton.getToggleState();
}
void TrackHeaderComponent::setTrackName(juce::String name) {
  trackNameLabel.setText(std::move(name), juce::dontSendNotification);
}

void TrackHeaderComponent::updateMuteButtonVisuals() {
  const bool on = muteTrackButton.getToggleState();

  muteTrackButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red.withAlpha(0.8f));
  muteTrackButton.setColour(juce::TextButton::buttonColourId,  juce::Colours::lightgrey);

  muteTrackButton.setColour(juce::TextButton::textColourOnId,  juce::Colours::white);
  muteTrackButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

  muteTrackButton.setTooltip(on ? "Muted" : "Unmuted");
  muteTrackButton.repaint();
}

void TrackHeaderComponent::setSolo(bool s){
  soloTrackButton.setToggleState(s, juce::dontSendNotification);
  updateSoloButtonVisuals();
}
bool TrackHeaderComponent::isSolo() const { return soloTrackButton.getToggleState(); }

void TrackHeaderComponent::updateSoloButtonVisuals(){
  soloTrackButton.setTooltip(soloTrackButton.getToggleState() ? "Soloed" : "Not soloed");
}

void TrackHeaderComponent::setDimmed(bool dim){
  setAlpha(dim ? 0.6f : 1.0f);
}

void TrackHeaderComponent::paint(juce::Graphics &g) {
  g.fillAll(juce::Colours::beige);
};

void TrackHeaderComponent::resized() {
  // this will be the main layout
  // set as vertical layout
  juce::FlexBox mainFB;
  mainFB.flexDirection = juce::FlexBox::Direction::column;
  mainFB.alignItems = juce::FlexBox::AlignItems::stretch;

  auto area = getLocalBounds().reduced(5);

  // Row 1: Track name
  mainFB.items.add(juce::FlexItem(trackNameLabel)
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

  buttonRowFB.items.add(juce::FlexItem(muteTrackButton)
        .withWidth(50)
        .withHeight(30));

  buttonRowFB.items.add(juce::FlexItem(soloTrackButton)
        .withWidth(50)
        .withHeight(30));

  buttonRowFB.items.add(juce::FlexItem(deleteTrackButton)
        .withWidth(50)
        .withHeight(30));

  buttonRowFB.items.add(juce::FlexItem(pianoRollButton)
        .withWidth(50)
        .withHeight(30));

  // Add the button row as a FlexItem to the main layout
  mainFB.items.add(juce::FlexItem(buttonRowFB)
      .withHeight(30)); // Fixed height for the button row

  // Perform layout
  mainFB.performLayout(area);
}