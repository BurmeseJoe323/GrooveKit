/// JUNIE
#include "ChannelStrip.h"
#include "../TrackView/TrackHeaderComponent.h"
#include "MainComponent.h"

ChannelStrip::ChannelStrip()
{
    setOpaque (false);

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

    muteButton.setColour (juce::TextButton::textColourOffId, juce::Colours::red);
    muteButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFF6C757D));
    muteButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF6C757D));
    muteButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::red);

    soloButton.setColour (juce::TextButton::textColourOffId, juce::Colours::yellow);
    soloButton.setColour (juce::TextButton::textColourOnId, juce::Colour (0xFF6C757D));
    soloButton.setColour (juce::TextButton::buttonColourId, juce::Colour (0xFF6C757D));
    soloButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::yellow);

    recordButton.setColour (juce::TextButton::textColourOffId, juce::Colours::white);
    recordButton.setColour (juce::TextButton::textColourOnId, juce::Colours::black);
    recordButton.setColour (juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    recordButton.setColour (juce::TextButton::buttonOnColourId, juce::Colours::darkred);

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
    name.setColour (juce::Label::textColourId, juce::Colour (0xFF343A40));

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

void ChannelStrip::paint (juce::Graphics& g)
{
    auto r = getLocalBounds().toFloat();

    g.setColour (juce::Colour (0xFFCED4DA));
    g.fillRoundedRectangle (r, 10.0f);
}

void ChannelStrip::resized()
{
    auto r = getLocalBounds().reduced (7); // inner padding

    // top controls stack
    auto top = r.removeFromTop (110);
    auto btnRowH = 24; // matches your rounded rects
    muteButton.setBounds (top.removeFromTop (btnRowH));
    top.removeFromTop (4);
    soloButton.setBounds (top.removeFromTop (btnRowH));
    top.removeFromTop (4);
    recordButton.setBounds (top.removeFromTop (btnRowH));

    const int nameH = 24;
    const int nameGap = 6;
    auto nameArea = r.removeFromBottom (nameH + nameGap);
    name.setBounds (nameArea.removeFromBottom (nameH));

    // bottom area: fader + meter
    auto bottom = r.removeFromBottom (r.getHeight()); // whatever remains

    // meter
    // const int meterW = juce::jmax(12, getWidth() / 9);
    // auto meterArea = bottom.removeFromRight(meterW);
    // meter.setBounds(meterArea.reduced(1, 6));

    bottom.removeFromRight (6);

    // pan knob
    auto panArea = bottom.removeFromTop (48);
    pan.setBounds (panArea.withSizeKeepingCentre (36, 36));

    // fader
    fader.setBounds (bottom.reduced (2, 8));
}
