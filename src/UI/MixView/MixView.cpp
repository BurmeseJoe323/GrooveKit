#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "MixView.h"

MixView::MixView(AppEngine& engine)
    : appEngine(engine)
{
    setOpaque(true);

    addAndMakeVisible(backButton);
    backButton.setButtonText("Track View");
    backButton.onClick = [this]{ if (onBack) onBack(); };

    mixerPanel = std::make_unique<MixerPanel>(appEngine);
    addAndMakeVisible(*mixerPanel);

    // Enable keyboard focus for MIDI playback (Written by Claude Code)
    setWantsKeyboardFocus(true);
}

MixView::~MixView() {
    mixerPanel.reset();
}


void MixView::paint (juce::Graphics& g) {
    g.fillAll(juce::Colour(0xFF495057));
}

void MixView::resized()
{
    auto bounds = getLocalBounds().reduced(outerMargin);

    auto topRow = bounds.removeFromTop(topBarHeight);

    const int btnW = 140, btnH = 36;
    backButton.setBounds(topRow.removeFromRight(btnW).withY(topRow.getY() + (topBarHeight - btnH) / 2)
                                        .withHeight(btnH).withWidth(btnW));

    mixerPanel->setBounds(bounds);
}

// Keyboard event handlers for MIDI playback (Written by Claude Code)
bool MixView::keyPressed(const juce::KeyPress& key_press)
{
    // The note keys are being handled by keyStateChanged, so we'll just say that the event is consumed
    if (appEngine.getMidiListener().getNoteKeys().contains(key_press.getKeyCode()))
        return true;

    if (key_press == juce::KeyPress::spaceKey)
    {
        // Spacebar toggles transport
        if (appEngine.isPlaying())
        {
            appEngine.stop();
        }
        else
        {
            appEngine.play();
        }
        return true;
    }

    // Let MidiListener handle octave changes (Z/X keys)
    if (appEngine.getMidiListener().handleKeyPress(key_press))
        return true;

    // This is the top level of our application, so if the key press has not been consumed,
    // it is not an implemented key command in GrooveKit
    return true;
}

bool MixView::keyStateChanged(bool isKeyDown)
{
    return appEngine.getMidiListener().handleKeyStateChanged(isKeyDown);
}

void MixView::parentHierarchyChanged()
{
    juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<MixView>(this)]
        {
            if (safe != nullptr && safe->isShowing())
                safe->grabKeyboardFocus();
        });
}

void MixView::mouseDown(const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    juce::Component::mouseDown(e);
}