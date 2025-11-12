#include "ChannelStrip.h"
#include "../TrackView/TrackHeaderComponent.h"
#include "MainComponent.h"

ChannelStrip::ChannelStrip(juce::Colour color)
    : stripColor(color)
{
    setOpaque (false);

    addAndMakeVisible (&instrumentButton);
    instrumentButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFADB5BD));
    instrumentButton.setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));

    instrumentButton.setButtonText ("Instrument");

    addAndMakeVisible (insertsLabel);
    insertsLabel.setText ("Inserts", juce::dontSendNotification);
    insertsLabel.setJustificationType (juce::Justification::centred);
    insertsLabel.setColour (juce::Label::textColourId, juce::Colour (0xFF212529));
    insertsLabel.setFont (juce::Font (12.0f).boldened());

    if (insertSlots.isEmpty())
    {
        for (int i = 0; i < 4; ++i)
        {
            const int slotIndex = i;

            // Main slot button
            auto* slot = new juce::TextButton();
            slot->setButtonText ("");  // will be set to plugin name later
            slot->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFDDE0E3));
            slot->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
            slot->onClick = [this, slotIndex]
            {
                if (onInsertSlotClicked)
                    onInsertSlotClicked (slotIndex);   // open editor or choose FX if empty
            };
            addAndMakeVisible (slot);
            insertSlots.add (slot);

            // Tiny ▼ menu button
            auto* menuBtn = new juce::TextButton();
            menuBtn->setButtonText (juce::String::fromUTF8 (u8"\u25BE"));
            menuBtn->setTooltip ("Change effect");
            menuBtn->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFC4C9CF));
            menuBtn->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
            menuBtn->onClick = [this, slotIndex]
            {
                if (onInsertSlotMenuRequested)
                    onInsertSlotMenuRequested (slotIndex);  // always show FX menu
            };
            addAndMakeVisible (menuBtn);
            insertSlotMenus.add (menuBtn);
        }

    }


    for (auto* b : { &muteButton, &soloButton, &recordButton })
    {
        addAndMakeVisible (*b);
        b->setColour (juce::TextButton::buttonColourId, juce::Colour (0xFFADB5BD));
        b->setColour (juce::TextButton::textColourOffId, juce::Colour (0xFF343A40));
    }
    muteButton.setButtonText ("M");
    soloButton.setButtonText ("S");
    recordButton.setButtonText ("R");

    // Toggle behavior
    muteButton.setClickingTogglesState (true);
    soloButton.setClickingTogglesState (true);
    recordButton.setClickingTogglesState (true);

    // Match TrackHeaderComponent button colors (Written by Claude Code)
    muteButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);
    muteButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    muteButton.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
    muteButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    soloButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::yellow);
    soloButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    soloButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    soloButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    recordButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::darkred);
    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    recordButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    recordButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);

    instrumentButton.onClick = [this]
    {
        if (onOpenInstrumentEditor)
            onOpenInstrumentEditor();
    };


    // Notify TrackHeaderComponent listeners (safely via SafePointers) (Junie)
    muteButton.onClick = [this] {
        const bool nowMuted = muteButton.getToggleState();
        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*> (comp))
                    l->onMuteToggled (nowMuted);
                else
                    listenerComponents.remove (i);
            }
            else
            {
                listenerComponents.remove (i);
            }
        }
        if (onRequestMuteChange)
            onRequestMuteChange (nowMuted);
    };

    soloButton.onClick = [this] {
        const bool nowSolo = soloButton.getToggleState();
        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*> (comp))
                    l->onSoloToggled (nowSolo);
                else
                    listenerComponents.remove (i);
            }
            else
            {
                listenerComponents.remove (i);
            }
        }
        if (onRequestSoloChange)
            onRequestSoloChange (nowSolo);
    };

    recordButton.onClick = [this] {
        const bool nowArmed = recordButton.getToggleState();
        for (int i = listenerComponents.size(); --i >= 0;)
        {
            if (auto* comp = listenerComponents[i].getComponent())
            {
                if (auto* l = dynamic_cast<TrackHeaderComponent::Listener*> (comp))
                    l->onRecordArmToggled (nowArmed);
                else
                    listenerComponents.remove (i);
            }
            else
            {
                listenerComponents.remove (i);
            }
        }
        if (onRequestArmChange)
            onRequestArmChange (nowArmed);
    };

    addAndMakeVisible (name);
    name.setText ("Track 1", juce::dontSendNotification);
    name.setJustificationType (juce::Justification::centred);
    name.setOpaque (false);
    // Match TrackHeaderComponent label color (Written by Claude Code)
    name.setColour (juce::Label::textColourId, juce::Colours::white.darker (0.1));
    // Make track name editable on double-click (Written by Claude Code)
    name.setEditable (false, true, false);
    name.setMouseCursor (juce::MouseCursor::IBeamCursor);
    name.addListener (this);

    addAndMakeVisible (fader);
    fader.setSliderStyle (juce::Slider::LinearVertical);
    fader.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    fader.setRange (0.0, 1.0, 0.001);
    fader.setSkewFactorFromMidPoint (0.5); // Logarithmic curve for natural volume control
    fader.setValue (0.75);
    fader.setLookAndFeel (&lnf);

    //addAndMakeVisible(meter);

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

// Handle user editing the track name label (Written by Claude Code)
void ChannelStrip::labelTextChanged (juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &name && onRequestNameChange)
    {
        const juce::String newName = name.getText();
        onRequestNameChange (trackIndex, newName);
    }
}

// Programmatic state sync without re-firing listeners
bool ChannelStrip::isMuted() const { return muteButton.getToggleState(); }
void ChannelStrip::setMuted (const bool isMuted)
{
    muteButton.setToggleState (isMuted, juce::dontSendNotification);
}

bool ChannelStrip::isSolo() const { return soloButton.getToggleState(); }
void ChannelStrip::setSolo (const bool isSolo)
{
    soloButton.setToggleState (isSolo, juce::dontSendNotification);
}

bool ChannelStrip::isArmed() const { return recordButton.getToggleState(); }
void ChannelStrip::setArmed (const bool isArmed)
{
    recordButton.setToggleState (isArmed, juce::dontSendNotification);
}

void ChannelStrip::bindToTrack (te::AudioTrack& track)
{
    boundTrack = &track;

    boundVnp = track.getVolumePlugin();
    const double pos = boundVnp ? boundVnp->getSliderPos() : 0.0;
    const double gain = te::volumeFaderPositionToGain (pos);

    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    fader.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const double gain = (double) fader.getValue();
        const double pos = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };

    instrumentButton.setVisible (true);
    insertsLabel.setVisible (true);
    for (auto* s : insertSlots) s->setVisible (true);

    // Pan binding (Written by Claude Code)
    const float panValue = boundVnp ? boundVnp->getPan() : 0.0f;
    ignoreSliderCallback = true;
    pan.setValue (panValue, juce::dontSendNotification);
    ignoreSliderCallback = false;

    pan.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const float panValue = (float) pan.getValue();
        boundVnp->setPan (panValue);
    };
}

void ChannelStrip::bindToMaster (te::Edit& edit)
{
    boundTrack = nullptr;

    boundVnp = edit.getMasterVolumePlugin();
    const double pos = boundVnp->getSliderPos();
    const double gain = te::volumeFaderPositionToGain (pos);

    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    fader.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const double gain = (double) fader.getValue();
        const double pos = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };
    instrumentButton.setVisible (false);

    // Pan binding (Written by Claude Code)
    const float panValue = boundVnp ? boundVnp->getPan() : 0.0f;
    ignoreSliderCallback = true;
    pan.setValue (panValue, juce::dontSendNotification);
    ignoreSliderCallback = false;

    pan.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const float panValue = (float) pan.getValue();
        boundVnp->setPan (panValue);
    };
}

void ChannelStrip::bindToVolume (te::VolumeAndPanPlugin& vnp)
{
    boundVnp = &vnp;
    const double pos = boundVnp->getSliderPos();
    const double gain = te::volumeFaderPositionToGain (pos);

    ignoreSliderCallback = true;
    fader.setValue (gain, juce::dontSendNotification);
    ignoreSliderCallback = false;

    fader.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const double gain = (double) fader.getValue();
        const double pos = te::gainToVolumeFaderPosition (gain);
        boundVnp->setSliderPos (pos);
    };

    // Pan binding (Written by Claude Code)
    const float panValue = boundVnp ? boundVnp->getPan() : 0.0f;
    ignoreSliderCallback = true;
    pan.setValue (panValue, juce::dontSendNotification);
    ignoreSliderCallback = false;

    pan.onValueChange = [this] {
        if (!boundVnp || ignoreSliderCallback)
            return;
        const float panValue = (float) pan.getValue();
        boundVnp->setPan (panValue);
    };
}

void ChannelStrip::setInsertSlotName (int slotIndex, const juce::String& text)
{
    if (juce::isPositiveAndBelow (slotIndex, insertSlots.size()))
        insertSlots[slotIndex]->setButtonText (text);
}

void ChannelStrip::paint (juce::Graphics& g)
{
    // Match TrackComponent styling with custom color (Written by Claude Code)
    auto bounds = getLocalBounds().toFloat().reduced (2.0f);

    // Use custom color darkened like TrackComponent does
    g.setColour (stripColor.darker (0.4f));

    // Rounded background
    const float radius = 10.0f;
    g.fillRoundedRectangle (bounds, radius);

    // Border
    g.setColour (juce::Colours::white.withAlpha (0.20f));
    g.drawRoundedRectangle (bounds, radius, 1.5f);
}

void ChannelStrip::resized()
{
    // Match TrackHeaderComponent padding and dimensions (Written by Claude Code)
    auto r = getLocalBounds().reduced (5); // inner padding (matches TrackHeaderComponent)

    // --- Track name at bottom ---
    const int nameH = 24;
    name.setBounds (r.removeFromBottom (nameH));

    // --- Metrics ---
    const int bigBtnH = 24;   // Instrument, M, S, R
    const int gapS    = 4;
    const int gapM    = 6;
    const int labelH  = 16;

    const int slotH    = 18;
    const int slotGap  = 2;
    const int menuW    = 18;   // width of ▼ button
    const int menuGap  = 2;    // spacing between main slot and ▼
    const int slotH   = 18;
    const int slotGap = 2;
    // top controls stack
    auto top = r.removeFromTop (110);
    constexpr int btnRowH = 25; // matches TrackHeaderComponent button height
    muteButton.setBounds (top.removeFromTop (btnRowH));
    top.removeFromTop (2); // matches TrackHeaderComponent margin
    soloButton.setBounds (top.removeFromTop (btnRowH));
    top.removeFromTop (2);
    recordButton.setBounds (top.removeFromTop (btnRowH));

    constexpr int nameH = 25; // matches TrackHeaderComponent label height
    const int nameGap = 6;
    auto nameArea = r.removeFromBottom (nameH + nameGap);
    name.setBounds (nameArea.removeFromBottom (nameH));

    const int numSlots = insertSlots.size(); // typically 4

    // compute exact needed height for the whole top stack:
    const int topH =
        /* Instrument */          bigBtnH +
        gapS +
        /* M */                   bigBtnH +
        gapS +
        /* S */                   bigBtnH +
        gapS +
        /* R */                   bigBtnH +
        gapM +
        /* INSERTS label */       labelH +
        /* slots */               (numSlots > 0 ? (numSlots * (slotH + slotGap)) - slotGap : 0);

    auto top = r.removeFromTop (topH);

    // === Top controls ===
    instrumentButton.setBounds (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);

    muteButton.setBounds   (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);
    soloButton.setBounds   (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapS);
    recordButton.setBounds (top.removeFromTop (bigBtnH));
    top.removeFromTop (gapM);

    // === INSERTS label ===
    insertsLabel.setBounds (top.removeFromTop (labelH));

    // === INSERT SLOTS WITH ▼ MENUS ===
    for (int i = 0; i < numSlots; ++i)
    {
        auto row = top.removeFromTop (slotH);

        // Reserve right-side space for ▼
        auto menuArea = row.removeFromRight (menuW);
        menuArea = menuArea.reduced (0, 1); // small vertical inset
        row.removeFromRight (menuGap);

        // Main button
        insertSlots[i]->setBounds (row);

        // ▼ button
        if (i < insertSlotMenus.size() && insertSlotMenus[i] != nullptr)
            insertSlotMenus[i]->setBounds (menuArea);

        if (i < numSlots - 1)
            top.removeFromTop (slotGap);
    }

    // === Pan + Fader ===
    auto body = r;
    body.removeFromRight (6);

    auto panArea = body.removeFromTop (48);
    pan.setBounds (panArea.withSizeKeepingCentre (36, 36));

    fader.setBounds (body.reduced (2, 8));
    // pan knob
    auto panArea = bottom.removeFromTop (48);
    pan.setBounds (panArea.withSizeKeepingCentre (50, 50));

    // fader
    fader.setBounds (bottom.reduced (2, 8));
}