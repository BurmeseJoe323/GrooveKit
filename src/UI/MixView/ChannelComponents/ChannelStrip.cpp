#include "ChannelStrip.h"
#include "../TrackView/TrackHeaderComponent.h"
#include "MainComponent.h"

/**
 * @brief Constructs a ChannelStrip with fully styled UI components.
 *
 * Sets up:
 *   - Instrument button
 *   - Inserts label + insert slot buttons + ▼ menus
 *   - Mute / Solo / Record buttons
 *   - Editable track name
 *   - Volume fader with custom LookAndFeel (FaderComponent)
 *   - Pan knob
 */
ChannelStrip::ChannelStrip (juce::Colour color)
    : stripColor (color)
{
    setOpaque (false);

    //==========================================================================
    // Instrument button
    addAndMakeVisible (&instrumentButton);
    instrumentButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFADB5BD));
    instrumentButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
    instrumentButton.setButtonText ("Instrument");

    //==========================================================================
    // Inserts label
    addAndMakeVisible (insertsLabel);
    insertsLabel.setText ("Inserts", juce::dontSendNotification);
    insertsLabel.setJustificationType (juce::Justification::centred);
    insertsLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF212529));
    {
        juce::Font f (juce::FontOptions (12.0f));
        f.setBold (true);
        insertsLabel.setFont (f);
    }

    //==========================================================================
    // FX insert slots (4)
    if (insertSlots.isEmpty())
    {
        for (int i = 0; i < 4; ++i)
        {
            const int slotIndex = i;

            // Main slot button (plugin name)
            auto* slot = new juce::TextButton();
            slot->setButtonText ("");
            slot->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFDDE0E3));
            slot->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
            slot->onClick = [this, slotIndex]
            {
                if (onInsertSlotClicked)
                    onInsertSlotClicked (slotIndex);
            };
            addAndMakeVisible (slot);
            insertSlots.add (slot);

            // ▼ menu button for choosing/removing FX
            auto* menuBtn = new juce::TextButton();
            menuBtn->setButtonText (juce::String::fromUTF8 (u8"\u25BE"));
            menuBtn->setTooltip ("Change effect");
            menuBtn->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFC4C9CF));
            menuBtn->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
            menuBtn->onClick = [this, slotIndex]
            {
                if (onInsertSlotMenuRequested)
                    onInsertSlotMenuRequested (slotIndex);
            };
            addAndMakeVisible (menuBtn);
            insertSlotMenus.add (menuBtn);
        }
    }

    //==========================================================================
    // Mute / Solo / Record buttons (toggle states)
    for (auto* b : { &muteButton, &soloButton, &recordButton })
    {
        addAndMakeVisible (*b);
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFADB5BD));
        b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
    }

    muteButton.setButtonText ("M");
    soloButton.setButtonText ("S");
    recordButton.setButtonText ("R");

    muteButton.setClickingTogglesState   (true);
    soloButton.setClickingTogglesState   (true);
    recordButton.setClickingTogglesState (true);

    // Match TrackHeaderComponent colors
    muteButton  .setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);
    soloButton  .setColour (juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    recordButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::darkred);

    //==========================================================================
    // Instrument button callback (open plugin editor)
    instrumentButton.onClick = [this]
    {
        if (onOpenInstrumentEditor)
            onOpenInstrumentEditor();
    };

    //==========================================================================
    // Mute / Solo / Record propagate changes to TrackHeader listeners
    muteButton.onClick = [this]
    {
        const bool nowMuted = muteButton.getToggleState();

        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*>(comp))
                    l->onMuteToggled (nowMuted);
                else
                    listenerComponents.remove (i);
            }
            else
                listenerComponents.remove (i);
        }

        if (onRequestMuteChange)
            onRequestMuteChange (nowMuted);
    };

    soloButton.onClick = [this]
    {
        const bool nowSolo = soloButton.getToggleState();

        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*>(comp))
                    l->onSoloToggled (nowSolo);
                else
                    listenerComponents.remove (i);
            }
            else
                listenerComponents.remove (i);
        }

        if (onRequestSoloChange)
            onRequestSoloChange (nowSolo);
    };

    recordButton.onClick = [this]
    {
        const bool nowArmed = recordButton.getToggleState();

        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*>(comp))
                    l->onRecordArmToggled (nowArmed);
                else
                    listenerComponents.remove (i);
            }
            else
                listenerComponents.remove (i);
        }

        if (onRequestArmChange)
            onRequestArmChange (nowArmed);
    };

    //==========================================================================
    // Track name label
    addAndMakeVisible (name);
    name.setText ("Track 1", juce::dontSendNotification);
    name.setJustificationType (juce::Justification::centred);
    name.setOpaque (false);
    name.setColour (juce::Label::textColourId, juce::Colours::white.darker (0.1f));
    name.setEditable (false, true, false);
    name.setMouseCursor (juce::MouseCursor::IBeamCursor);
    name.addListener (this);

    //==========================================================================
    // Volume fader
    addAndMakeVisible (fader);
    fader.setSliderStyle (juce::Slider::LinearVertical);
    fader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    fader.setRange (0.0, 1.0, 0.001);
    fader.setSkewFactorFromMidPoint (0.5);
    fader.setValue (0.75);
    fader.setLookAndFeel (&lnf);

    //==========================================================================
    // Pan knob
    addAndMakeVisible (pan);
    pan.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange (-1.0, 1.0, 0.001);
    pan.setValue (0.0);
}

ChannelStrip::~ChannelStrip()
{
    fader.setLookAndFeel (nullptr);
    DBG ("[ChannelStrip] dtor");
}

void ChannelStrip::bindToTrack (te::AudioTrack& track)
{
    boundTrack = &track;

    // Bind to the track's Volume & Pan plugin
    boundVnp = track.getVolumePlugin();
    const double pos  = boundVnp ? boundVnp->getSliderPos() : 0.0;
    const double gain = te::volumeFaderPositionToGain (pos);

    // Set fader without re-triggering callback
    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    // Fader → VolumeAndPanPlugin
    fader.onValueChange = [this]
    {
        if (! boundVnp || ignoreSliderCallback)
            return;

        const double gain = (double) fader.getValue();
        const double pos  = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };

    // Show instrument + inserts for regular tracks
    instrumentButton.setVisible (true);
    insertsLabel.setVisible (true);
    for (auto* s : insertSlots)
        s->setVisible (true);

    // ---- Pan binding ----
    const float panValue = boundVnp ? boundVnp->getPan() : 0.0f;
    ignoreSliderCallback = true;
    pan.setValue (panValue, juce::dontSendNotification);
    ignoreSliderCallback = false;

    pan.onValueChange = [this]
    {
        if (! boundVnp || ignoreSliderCallback)
            return;

        const float panValue = (float) pan.getValue();
        boundVnp->setPan (panValue);
    };
}

void ChannelStrip::bindToMaster (te::Edit& edit)
{
    boundTrack = nullptr;

    // Bind to master Volume & Pan plugin
    boundVnp = edit.getMasterVolumePlugin();
    const double pos  = boundVnp ? boundVnp->getSliderPos() : 0.0;
    const double gain = te::volumeFaderPositionToGain (pos);

    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    // Fader → master VolumeAndPanPlugin
    fader.onValueChange = [this]
    {
        if (! boundVnp || ignoreSliderCallback)
            return;

        const double gain = (double) fader.getValue();
        const double pos  = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };

    // Master strip doesn’t have an instrument button
    instrumentButton.setVisible (false);

    // ---- Pan binding ----
    const float panValue = boundVnp ? boundVnp->getPan() : 0.0f;
    ignoreSliderCallback = true;
    pan.setValue (panValue, juce::dontSendNotification);
    ignoreSliderCallback = false;

    pan.onValueChange = [this]
    {
        if (! boundVnp || ignoreSliderCallback)
            return;

        const float panValue = (float) pan.getValue();
        boundVnp->setPan (panValue);
    };
}

void ChannelStrip::bindToVolume (te::VolumeAndPanPlugin& vnp)
{
    boundVnp = &vnp;
    const double pos  = boundVnp->getSliderPos();
    const double gain = te::volumeFaderPositionToGain (pos);

    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    fader.onValueChange = [this]
    {
        if (! boundVnp || ignoreSliderCallback)
            return;

        const double gain = (double) fader.getValue();
        const double pos  = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };
}

/**
 * @brief Called when user finishes editing the track name.
 * Forwards to MixView via onRequestNameChange.
 */
void ChannelStrip::labelTextChanged (juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &name && onRequestNameChange)
    {
        const juce::String newName = name.getText();
        onRequestNameChange (trackIndex, newName);
    }
}

//==============================================================================
// State getters/setters without re-triggering callbacks
bool ChannelStrip::isMuted()  const { return muteButton.getToggleState(); }
bool ChannelStrip::isSolo()   const { return soloButton.getToggleState(); }
bool ChannelStrip::isArmed()  const { return recordButton.getToggleState(); }

void ChannelStrip::setMuted (bool b)   { muteButton.setToggleState   (b, juce::dontSendNotification); }
void ChannelStrip::setSolo  (bool b)   { soloButton.setToggleState   (b, juce::dontSendNotification); }
void ChannelStrip::setArmed (bool b)   { recordButton.setToggleState (b, juce::dontSendNotification); }

void ChannelStrip::setInsertSlotName (int slotIndex, const juce::String& text)
{
    if (! juce::isPositiveAndBelow (slotIndex, insertSlots.size()))
        return;

    if (auto* button = insertSlots[slotIndex])
        button->setButtonText (text);
}

void ChannelStrip::setInstrumentButtonText (const juce::String& text)
{
    instrumentButton.setButtonText (text.isEmpty() ? "Instrument" : text);
}

//==============================================================================
// Painting & Layout
void ChannelStrip::paint (juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);

    g.setColour (stripColor.darker (0.4f));
    g.fillRoundedRectangle (bounds, 10.0f);

    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.drawRoundedRectangle (bounds, 10.0f, 1.5f);
}

void ChannelStrip::resized()
{
    auto r = getLocalBounds().reduced (5);

    const int bigBtnH  = 24;
    const int gapS     = 4;
    const int gapM     = 6;
    const int labelH   = 16;
    const int slotH    = 18;
    const int slotGap  = 2;
    const int menuW    = 18;
    const int menuGap  = 2;

    const int numSlots = insertSlots.size();
    const int slotsTotalH =
        numSlots > 0 ? (numSlots * slotH + (numSlots - 1) * slotGap) : 0;

    const int topH =
        bigBtnH + gapS + labelH + slotsTotalH + gapM + (3 * bigBtnH);

    auto top = r.removeFromTop (topH);

    // Track name
    constexpr int nameH = 25;
    const int nameGap   = 6;
    auto nameArea = r.removeFromBottom (nameH + nameGap);
    name.setBounds (nameArea.removeFromBottom (nameH));

    // Instrument button
    instrumentButton.setBounds (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);

    // Insert label
    insertsLabel.setBounds (top.removeFromTop (labelH));

    // Insert slots + ▼
    for (int i = 0; i < numSlots; ++i)
    {
        auto row = top.removeFromTop (slotH);
        auto menuArea = row.removeFromRight (menuW).reduced (0, 1);
        row.removeFromRight (menuGap);

        insertSlots[i]->setBounds (row);

        if (i < insertSlotMenus.size())
            if (auto* m = insertSlotMenus[i])
                m->setBounds (menuArea);

        if (i < numSlots - 1)
            top.removeFromTop (slotGap);
    }

    top.removeFromTop (gapM);

    // Mute / Solo / Record
    muteButton.setBounds   (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);
    soloButton.setBounds   (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);
    recordButton.setBounds (top.removeFromTop (bigBtnH));

    // Pan + Fader area
    auto bottom = r;

    auto panArea = bottom.removeFromTop (48);
    pan.setBounds (panArea.withSizeKeepingCentre (50, 50));

    fader.setBounds (bottom.reduced (2, 8));
}