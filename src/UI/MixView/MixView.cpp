#include <juce_gui_basics/juce_gui_basics.h>
#include "MixView.h"
#include "../TransportBar/TransportBar.h"
#include "../MenuBar/GrooveKitMenuBar.h"

//==============================================================================
// Construction / Destruction

MixView::MixView(AppEngine& engine, TransportBar& transport, GrooveKitMenuBar& menu)
    : appEngine(engine), transportBar(&transport), menuBar(&menu)
{
    setOpaque(true);

    // Add menu bar and transport bar
    addAndMakeVisible(menuBar);
    addAndMakeVisible(transportBar);

    mixerPanel = std::make_unique<MixerPanel>(appEngine);
    addAndMakeVisible(*mixerPanel);

    // Set up menu bar callbacks to refresh mixer when tracks are created
    menuBar->onNewInstrumentTrack = [this] {
        appEngine.addInstrumentTrack();
        refreshMixer();
    };
    menuBar->onNewDrumTrack = [this] {
        appEngine.addDrumTrack();
        refreshMixer();
    };

    // Enable keyboard focus for MIDI playback
    setWantsKeyboardFocus(true);
}

MixView::~MixView()
{
    mixerPanel.reset();
}

//==============================================================================
// Component Overrides

void MixView::paint (juce::Graphics& g)
{
    // Match TrackListComponent background color
    g.fillAll(juce::Colour(0xFF343A40));
}

void MixView::resized()
{
    auto bounds = getLocalBounds();

    // Position menu bar at top on non-Mac platforms
    #if !JUCE_MAC
    constexpr int menuHeight = 24;
    menuBar->setBounds(bounds.removeFromTop(menuHeight));
    #endif

    // Position transport bar below menu (or at top on Mac)
    constexpr int transportHeight = 40;
    transportBar->setBounds(bounds.removeFromTop(transportHeight));

    // Mixer panel fills remaining space with margin
    bounds.reduce(outerMargin, outerMargin);
    mixerPanel->setBounds(bounds);
}

//==============================================================================
// Keyboard and Mouse Handlers

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