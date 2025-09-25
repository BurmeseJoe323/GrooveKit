#include "TrackHeaderComponent.h"

TrackHeaderComponent::TrackHeaderComponent()
{
    addAndMakeVisible (settingsButton);
    addAndMakeVisible (trackNameLabel);
    addAndMakeVisible (muteTrackButton);
    addAndMakeVisible (soloTrackButton);
    addAndMakeVisible (trackTypeLabel);

    settingsButton.onClick = [this]() {
        listeners.call ([] (Listener& l) { l.onSettingsClicked(); });
    };

    muteTrackButton.setClickingTogglesState (true);
    muteTrackButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);
    muteTrackButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    muteTrackButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    muteTrackButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    muteTrackButton.onClick = [this]() {
        const bool nowMuted = muteTrackButton.getToggleState();
        listeners.call ([&] (Listener& l) { l.onMuteToggled (nowMuted); });
    };

    soloTrackButton.setClickingTogglesState (true);
    soloTrackButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloTrackButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    soloTrackButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    soloTrackButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    soloTrackButton.onClick = [this] {
        const bool nowSolo = soloTrackButton.getToggleState();
        listeners.call ([&] (Listener& l) { l.onSoloToggled (nowSolo); });
    };

    trackNameLabel.setFont (juce::Font (15.0f));
    trackNameLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    trackNameLabel.setJustificationType (juce::Justification::centred);
    trackNameLabel.setText ("Track", juce::dontSendNotification);

    trackTypeLabel.setFont (juce::Font (12.0f));
    trackTypeLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    trackTypeLabel.setJustificationType (juce::Justification::centred);

    setTrackType (TrackType::Instrument);
}

TrackHeaderComponent::~TrackHeaderComponent() = default;

void TrackHeaderComponent::setTrackName (juce::String name)
{
    trackNameLabel.setText (std::move (name), juce::dontSendNotification);
}

void TrackHeaderComponent::setTrackType (const TrackType type)
{
    if (type == TrackType::Drum)
    {
        setTrackName ("Drums");
        trackTypeLabel.setText ("Drum Track", juce::dontSendNotification);
    }
    else
    {
        trackTypeLabel.setText ("Instrument", juce::dontSendNotification);
    }
}

bool TrackHeaderComponent::isMuted() const { return muteTrackButton.getToggleState(); }
void TrackHeaderComponent::setMuted (const bool shouldBeMuted)
{
    muteTrackButton.setToggleState (shouldBeMuted, juce::dontSendNotification);
}

bool TrackHeaderComponent::isSolo() const { return soloTrackButton.getToggleState(); }
void TrackHeaderComponent::setSolo (const bool shouldBeSolo)
{
    soloTrackButton.setToggleState (shouldBeSolo, juce::dontSendNotification);
}

void TrackHeaderComponent::setDimmed (const bool dim)
{
    setAlpha (dim ? 0.6f : 1.0f);
}

void TrackHeaderComponent::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);

    g.setColour (juce::Colour (0xFF495057));

    // Rounded background
    const float radius = 10.0f;
    g.fillRoundedRectangle (bounds, radius);

    // Border
    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.drawRoundedRectangle (bounds, radius, 1.5f);
}

void TrackHeaderComponent::resized()
{
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    fb.alignItems = juce::FlexBox::AlignItems::stretch;

    const auto bounds = getLocalBounds().reduced (5);
    constexpr int buttonHeight = 25;
    const auto margin = juce::FlexItem::Margin (2, 0, 2, 0);

    fb.items.add (juce::FlexItem (trackTypeLabel).withHeight (15.0f));
    fb.items.add (juce::FlexItem (trackNameLabel).withFlex (1.0f));
    fb.items.add (juce::FlexItem (settingsButton).withHeight (buttonHeight).withMargin (margin));
    fb.items.add (juce::FlexItem (muteTrackButton).withHeight (buttonHeight).withMargin (margin));
    fb.items.add (juce::FlexItem (soloTrackButton).withHeight (buttonHeight).withMargin (margin));

    fb.performLayout (bounds);
}