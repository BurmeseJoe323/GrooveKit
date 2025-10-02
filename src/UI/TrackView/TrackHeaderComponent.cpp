#include "TrackHeaderComponent.h"

TrackHeaderComponent::TrackHeaderComponent()
{
    // addAndMakeVisible(addClip);
    // addAndMakeVisible(deleteTrackButton);
    // addAndMakeVisible(pianoRollButton);
    addAndMakeVisible (settingsButton);
    addAndMakeVisible (trackNameLabel);
    addAndMakeVisible (muteTrackButton);
    addAndMakeVisible (soloTrackButton);
    addAndMakeVisible (trackTypeLabel);
    // addAndMakeVisible (drumSamplerButton);

    // addClip.onClick = [this]() {
    //     listeners.call ([] (Listener& l) { l.onAddClipClicked(); });
    // };
    //
    // deleteTrackButton.onClick = [this]() {
    //     listeners.call ([] (Listener& l) { l.onDeleteTrackClicked(); });
    // };
    //
    // pianoRollButton.onClick = [this]() {
    //     listeners.call ([] (Listener& l) { l.onPianoRollClicked(); });
    // };
    //
    // drumSamplerButton.onClick = [this]() {
    //     listeners.call ([] (Listener& l) { l.onDrumSamplerClicked(); });
    // };

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
        // updateMuteButtonVisuals();
        listeners.call ([&] (Listener& l) { l.onMuteToggled (nowMuted); });
    };

    soloTrackButton.setClickingTogglesState (true);
    soloTrackButton.onClick = [this] {
        const bool nowSolo = soloTrackButton.getToggleState();
        // updateSoloButtonVisuals();
        listeners.call ([&] (Listener& l) { l.onSoloToggled (nowSolo); });
    };

    soloTrackButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloTrackButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    soloTrackButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    soloTrackButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    // setup trackNameLabel
    trackNameLabel.setFont (juce::Font (15.0f));
    trackNameLabel.setColour (juce::Label::textColourId, juce::Colours::black);
    trackNameLabel.setJustificationType (juce::Justification::centred);
    trackNameLabel.setText ("Track", juce::dontSendNotification);

    // setup trackTypeLabel
    trackTypeLabel.setFont (juce::Font (12.0f));
    trackTypeLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    trackTypeLabel.setJustificationType (juce::Justification::centred);

    setTrackType (TrackType::Instrument);
};

TrackHeaderComponent::~TrackHeaderComponent() = default;

void TrackHeaderComponent::setTrackType (TrackType type)
{
    trackType = type;
    const bool isDrum = (trackType == TrackType::Drum);
    // drumSamplerButton.setVisible (isDrum);
    if (isDrum)
    {
        setTrackName ("Drums");
        trackTypeLabel.setText ("Drum Track", juce::dontSendNotification);
    }
    else
    {
        trackTypeLabel.setText ("Instrument", juce::dontSendNotification);
    }
}

void TrackHeaderComponent::setMuted (bool shouldBeMuted)
{
    muteTrackButton.setToggleState (shouldBeMuted, juce::dontSendNotification);
    // updateMuteButtonVisuals();
}
bool TrackHeaderComponent::isMuted() const
{
    return muteTrackButton.getToggleState();
}
void TrackHeaderComponent::setTrackName (juce::String name)
{
    trackNameLabel.setText (std::move (name), juce::dontSendNotification);
}

// void TrackHeaderComponent::updateMuteButtonVisuals()
// {
//     const bool on = muteTrackButton.getToggleState();
//
//     muteTrackButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red.withAlpha (0.8f));
//     muteTrackButton.setColour (juce::TextButton::buttonColourId, juce::Colours::lightgrey);
//
//     muteTrackButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
//     muteTrackButton.setColour (juce::TextButton::textColourOffId, juce::Colours::black);
//
//     muteTrackButton.setTooltip (on ? "Muted" : "Unmuted");
//     muteTrackButton.repaint();
// }

void TrackHeaderComponent::setSolo (bool s)
{
    soloTrackButton.setToggleState (s, juce::dontSendNotification);
    // updateSoloButtonVisuals();
}
bool TrackHeaderComponent::isSolo() const { return soloTrackButton.getToggleState(); }

// void TrackHeaderComponent::updateSoloButtonVisuals()
// {
//     soloTrackButton.setTooltip (soloTrackButton.getToggleState() ? "Soloed" : "Not soloed");
// }

void TrackHeaderComponent::setDimmed (bool dim)
{
    setAlpha (dim ? 0.6f : 1.0f);
}

void TrackHeaderComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::dimgrey);
};

void TrackHeaderComponent::resized()
{
    // Vertical layout
    // juce::FlexBox mainFB;
    // mainFB.flexDirection = juce::FlexBox::Direction::column;
    // mainFB.alignItems = juce::FlexBox::AlignItems::stretch;
    juce::FlexBox fb;
    fb.flexDirection = juce::FlexBox::Direction::column;
    fb.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    fb.alignItems = juce::FlexBox::AlignItems::stretch;

    auto bounds = getLocalBounds().reduced (5);
    const int buttonHeight = 25;
    const auto margin = juce::FlexItem::Margin (2, 0, 2, 0);

    // // Row 1: Track name
    // mainFB.items.add (juce::FlexItem (trackNameLabel)
    //         .withWidth (50)
    //         .withHeight (30)
    //         .withAlignSelf (juce::FlexItem::AlignSelf::flexStart));
    //
    // // Row 2: Buttons
    // juce::FlexBox buttonRowFB;
    // buttonRowFB.flexDirection = juce::FlexBox::Direction::row;
    // buttonRowFB.justifyContent = juce::FlexBox::JustifyContent::flexEnd;
    //
    // buttonRowFB.items.add (juce::FlexItem (addClip).withWidth (50).withHeight (30));
    // buttonRowFB.items.add (juce::FlexItem (muteTrackButton).withWidth (50).withHeight (30));
    // buttonRowFB.items.add (juce::FlexItem (soloTrackButton).withWidth (50).withHeight (30));
    // buttonRowFB.items.add (juce::FlexItem (deleteTrackButton).withWidth (50).withHeight (30));
    //
    // juce::FlexBox buttonRowFB2;
    // buttonRowFB2.flexDirection = juce::FlexBox::Direction::row;
    // buttonRowFB2.justifyContent = juce::FlexBox::JustifyContent::flexEnd;
    //
    // buttonRowFB2.items.add (juce::FlexItem (pianoRollButton).withWidth (70).withHeight (30));
    //
    // if (drumSamplerButton.isVisible())
    //     buttonRowFB2.items.add (juce::FlexItem (drumSamplerButton).withWidth (70).withHeight (30));
    //
    // mainFB.items.add (juce::FlexItem (buttonRowFB).withHeight (30));
    // mainFB.items.add (juce::FlexItem (buttonRowFB2).withHeight (30));
    // mainFB.performLayout (area);

    fb.items.add (juce::FlexItem (trackTypeLabel).withHeight (15.0f));
    fb.items.add (juce::FlexItem (trackNameLabel).withFlex (1.0f));
    fb.items.add (juce::FlexItem (settingsButton).withHeight (buttonHeight).withMargin (margin));
    fb.items.add (juce::FlexItem (muteTrackButton).withHeight (buttonHeight).withMargin (margin));
    fb.items.add (juce::FlexItem (soloTrackButton).withHeight (buttonHeight).withMargin (margin));

    fb.performLayout (bounds);
}