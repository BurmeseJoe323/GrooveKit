// Note: Junie (JetBrains AI) contributed code to this file on 2025-09-24.
#include "TrackEditView.h"
#include "MidiListener.h"
#include "../../AppEngine/AppEngine.h"
#include "../../AppEngine/ValidationUtils.h"
#include "PopupWindows/OutputDevice/OutputDeviceWindow.h"
#include "PopupWindows/MidiInputDevice/MidiInputDeviceWindow.h"
#include <regex>

// Helper for styling the menu buttons
void styleMenuButton (juce::TextButton& button)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colour (0x00000000));
    button.setColour (juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

TrackEditView::TrackEditView (AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine> (&engine,
        [](AppEngine*) {
        });

    #if JUCE_MAC
    // Use native macOS global menu bar
    juce::MenuBarModel::setMacMainMenu (this);
    #else
    menuBar = std::make_unique<juce::MenuBarComponent> (this);
    addAndMakeVisible (menuBar.get());
    #endif

    trackList = std::make_unique<TrackListComponent> (appEngine);

    trackList->setPixelsPerBeat (pixelsPerBeat);
    trackList->setViewStartBeat (viewStartBeat);

    viewport.setScrollBarsShown (true, true);
    viewport.setViewedComponent (trackList.get(), false);

    trackList->rebuildFromEngine();

    appEngine->onEditLoaded = [this] {
        trackList = std::make_unique<TrackListComponent> (appEngine);
        trackList->setPixelsPerBeat (pixelsPerBeat);
        trackList->setViewStartBeat (viewStartBeat);

        viewport.setViewedComponent (trackList.get(), false);
        trackList->rebuildFromEngine();

        hidePianoRoll();

        bpmEditField.setText (juce::String (appEngine->getBpm()), juce::NotificationType::dontSendNotification);

        repaint();
    };

    setupButtons();

    // Initialize and hide the piano roll editor
    pianoRoll = std::make_unique<PianoRollEditor> (*appEngine, -1);
    addAndMakeVisible (pianoRoll.get());
    pianoRoll->setVisible (false);
    pianoRoll->onClose = [this] { hidePianoRoll(); };

    // Split the view vertically
    verticalLayout.setItemLayout (0, -0.45, -0.85, -0.6); // Track list takes 70%
    verticalLayout.setItemLayout (1, 5, 5, 5); // 5-pixel splitter
    verticalLayout.setItemLayout (2, -0.15, -0.55, -0.4); // Piano roll takes 30%

    // Create and add resizer bar (index 1 in components array)
    resizerBar = std::make_unique<PianoRollResizerBar> (&verticalLayout, 1, false);
    addAndMakeVisible (resizerBar.get());

    addAndMakeVisible (viewport);

    setWantsKeyboardFocus (true);
}

TrackEditView::~TrackEditView ()
{
    #if JUCE_MAC
    // Clear the native macOS menu bar to avoid assertions during shutdown
    juce::MenuBarModel::setMacMainMenu (nullptr);
    #endif
}

void TrackEditView::paint (juce::Graphics& g)
{
    const auto topBarBounds = getLocalBounds().removeFromTop (40);
    g.setColour (juce::Colour (0xFF212529)); // Even darker for top bar
    g.fillRect (topBarBounds);
    g.setColour (juce::Colours::black.withAlpha (0.2f));
    g.drawHorizontalLine (topBarBounds.getBottom(), 0.0f, static_cast<float> (getWidth()));
}

void TrackEditView::resized ()
{
    auto r = getLocalBounds();
    const auto topBar = r.removeFromTop (40);
    viewport.setBounds (r);

    auto topBarContent = topBar.reduced (10, 0);

    // --- Menu ---
    if (menuBar)
        menuBar->setBounds (topBarContent.removeFromLeft (200));

    // --- Right side: Switch ---
    const auto switchArea = topBarContent.removeFromRight (50);
    switchButton.setBounds (switchArea);

    // --- Center: Transport ---
    auto centerArea = topBarContent;
    bpmLabel.setBounds (centerArea.removeFromLeft (50));
    auto valueArea = centerArea.removeFromLeft (50);
    int deltaHeight = valueArea.getHeight() / 8;
    valueArea.removeFromBottom (deltaHeight);
    valueArea.removeFromTop (deltaHeight);
    bpmEditField.setBounds (valueArea);

    constexpr int buttonSize = 20;
    constexpr int buttonGap = 10;
    constexpr int transportWidth = (buttonSize * 3) + (buttonGap * 2);
    auto transportBounds = centerArea.withSizeKeepingCentre (transportWidth, buttonSize);
    stopButton.setBounds (transportBounds.removeFromLeft (buttonSize));
    transportBounds.removeFromLeft (buttonGap);
    playButton.setBounds (transportBounds.removeFromLeft (buttonSize));
    transportBounds.removeFromLeft (buttonGap);
    recordButton.setBounds (transportBounds.removeFromLeft (buttonSize));

    // Content area below top bar
    // If the piano roll is hidden, just fill with the viewport and hide the resizer
    if (!pianoRoll || !pianoRoll->isVisible())
    {
        viewport.setBounds (r);
        if (resizerBar)
            resizerBar->setVisible (false);
        return;
    }

    // Piano roll is visible: use the stretchable layout to split vertically
    if (resizerBar)
        resizerBar->setVisible (true);

    juce::Component* comps[] = { &viewport, resizerBar.get(), pianoRoll.get() };
    verticalLayout.layOutComponents (comps, (int) std::size (comps), r.getX(), r.getY(), r.getWidth(), r.getHeight(), true, true);

    // Ensure the resizer and piano roll are on top of the viewport
    resizerBar->toFront (false);
    pianoRoll->toFront (false);
}

bool TrackEditView::keyPressed (const juce::KeyPress& key_press)
{
    // The note keys are being handled by keyStateChanged, so we'll just say that the event is consumed
    if (appEngine->getMidiListener().getNoteKeys().contains (key_press.getKeyCode()))
        return true;

    if (key_press == juce::KeyPress::spaceKey)
    {
        // Spacebar toggles transport
        if (appEngine->isPlaying())
        {
            appEngine->stop();
        }
        else
        {
            appEngine->play();
        }
        return true;
    }

    // Let MidiListener handle octave changes (Z/X keys)
    if (appEngine->getMidiListener().handleKeyPress(key_press))
        return true;

    // This is the top level of our application, so if the key press has not been consumed,
    // it is not an implemented key command in GrooveKit
    return true;
}

bool TrackEditView::keyStateChanged (bool isKeyDown)
{
    return appEngine->getMidiListener().handleKeyStateChanged(isKeyDown);
}

void TrackEditView::parentHierarchyChanged()
{
    juce::MessageManager::callAsync(
        [safe = juce::Component::SafePointer<TrackEditView>(this)]
        {
            if (safe != nullptr && safe->isShowing())
                safe->grabKeyboardFocus();
        });
}

void TrackEditView::mouseDown (const juce::MouseEvent& e)
{
    grabKeyboardFocus();
    juce::Component::mouseDown(e);
}

void TrackEditView::setupButtons ()
{
    // --- Left Controls ---
    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM:", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    bpmLabel.setJustificationType (juce::Justification::right);

    addAndMakeVisible (bpmEditField);
    bpmEditField.setText ("120", juce::dontSendNotification);
    bpmEditField.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    bpmEditField.setColour (juce::Label::outlineColourId, juce::Colours::lightgrey.brighter (0.5f));
    bpmEditField.setColour (juce::Label::backgroundColourId, juce::Colours::darkgrey.darker ());
    bpmEditField.setJustificationType (juce::Justification::centred);
    bpmEditField.setEditable (true);
    bpmEditField.addListener (this);
    bpmEditField.setMouseCursor (juce::MouseCursor::IBeamCursor);

    // --- Transport Buttons ---
    {
        juce::Path stopShape;
        stopShape.addRectangle (0.0f, 0.0f, 1.0f, 1.0f);
        stopButton.setShape (stopShape, true, true, false);
        stopButton.setColours (juce::Colours::lightgrey, juce::Colours::white, juce::Colours::darkgrey);
        stopButton.onClick = [this] { appEngine->stop(); };
        addAndMakeVisible (stopButton);

        juce::Path playShape;
        playShape.addTriangle (0.0f, 0.0f, 1.0f, 0.5f, 0.0f, 1.0f);
        playButton.setShape (playShape, true, true, false);
        playButton.setColours (juce::Colours::lightgrey, juce::Colours::white, juce::Colours::darkgrey);
        playButton.onClick = [this] { appEngine->play(); };
        addAndMakeVisible (playButton);

        juce::Path recordShape;
        recordShape.addEllipse (0.0f, 0.0f, 1.0f, 1.0f);
        recordButton.setShape (recordShape, true, true, false);
        recordButton.setColours (juce::Colours::red, juce::Colours::lightcoral, juce::Colours::maroon);
        addAndMakeVisible (recordButton);
    }

    // --- Right Switch ---
    addAndMakeVisible (switchButton);
    styleMenuButton (switchButton);
    switchButton.onClick = [this] {
        if (onOpenMix)
            onOpenMix();
    };
}

juce::StringArray TrackEditView::getMenuBarNames ()
{
    return { "File", "View", "Track", "Help" };
}

juce::PopupMenu TrackEditView::getMenuForIndex (const int topLevelMenuIndex, const juce::String&)
{
    juce::PopupMenu menu;
    enum MenuIDs
    {
        OpenMixer = 1002,
        ShowOutputSettings = 1003,
        ShowMidiInputSettings = 1004,
        NewEdit = 2001,
        OpenEdit = 2002,
        SaveEdit = 2003,
        SaveEditAs = 2004,
        NewInstrumentTrack = 3001,
        NewDrumTrack = 3002
    };

    if (topLevelMenuIndex == 0) // File
    {
        menu.addItem (NewEdit, "New Edit");
        menu.addItem (OpenEdit, "Open Edit...");
        menu.addSeparator();
        menu.addItem (SaveEdit, "Save Edit");
        menu.addItem (SaveEditAs, "Save Edit As...");
        menu.addSeparator();
        menu.addItem (ShowOutputSettings, "Output Device Settings...");
        menu.addItem (ShowMidiInputSettings, "MIDI Input Device Settings...");
    }
    else if (topLevelMenuIndex == 1) // View
    {
        menu.addItem (OpenMixer, "Mix View");
    }
    else if (topLevelMenuIndex == 2) // Track
    {
        menu.addItem (NewInstrumentTrack, "New Instrument Track");
        menu.addItem (NewDrumTrack, "New Drum Track");
    }
    return menu;
}

void TrackEditView::menuItemSelected (const int menuItemID, int)
{
    enum MenuIDs
    {
        OpenMixer = 1002,
        ShowOutputSettings = 1003,
        ShowMidiInputSettings = 1004,
        NewEdit = 2001,
        OpenEdit = 2002,
        SaveEdit = 2003,
        SaveEditAs = 2004,
        NewInstrumentTrack = 3001,
        NewDrumTrack = 3002
    };

    switch (menuItemID)
    {
        case NewInstrumentTrack:
        case NewDrumTrack:
        {
            if (!trackList)
                return;
            const int index = (menuItemID == NewInstrumentTrack) ? appEngine->addInstrumentTrack() : appEngine->addDrumTrack();
            trackList->addNewTrack (index);
            trackList->setPixelsPerBeat (pixelsPerBeat);
            trackList->setViewStartBeat (viewStartBeat);
            break;
        }
        case OpenMixer:
            if (onOpenMix)
                onOpenMix();
            break;
        case ShowOutputSettings:
            showOutputDeviceSettings(); // TODO : fix positioning
            break;
        case ShowMidiInputSettings:
            showMidiInputDeviceSettings();
            break;
        case NewEdit:
            showNewEditMenu();
            break;
        case OpenEdit:
            showOpenEditMenu();
            break;
        case SaveEdit:
            appEngine->saveEdit();
            break;
        case SaveEditAs:
            appEngine->saveEditAsAsync();
            break;
        default:
            break;
    }
}

void TrackEditView::showOutputDeviceSettings () const
{
    auto* content = new OutputDeviceWindow (*appEngine);

    content->setSize (360, 140);

    juce::Rectangle<int> screenBounds;
    #if JUCE_MAC
    screenBounds = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    screenBounds = screenBounds.withHeight (25); // Approx height of mac menu bar
    #else
    if (menuBar)
        screenBounds = menuBar->getScreenBounds();
    #endif
    juce::CallOutBox::launchAsynchronously (std::unique_ptr<Component> (content), screenBounds, nullptr);
}

void TrackEditView::showMidiInputDeviceSettings () const
{
    auto* content = new MidiInputDeviceWindow (*appEngine);

    content->setSize (360, 140);

    juce::Rectangle<int> screenBounds;
    #if JUCE_MAC
    screenBounds = juce::Desktop::getInstance().getDisplays().getPrimaryDisplay()->userArea;
    screenBounds = screenBounds.withHeight (25); // Approx height of mac menu bar
    #else
    if (menuBar)
        screenBounds = menuBar->getScreenBounds();
    #endif
    juce::CallOutBox::launchAsynchronously (std::unique_ptr<Component> (content), screenBounds, nullptr);
}

void TrackEditView::showNewEditMenu () const
{
    if (appEngine->isDirty())
    {
        const auto opts = juce::MessageBoxOptions()
            .withIconType (juce::MessageBoxIconType::WarningIcon)
            .withTitle ("Save changes?")
            .withMessage ("You have unsaved changes.")
            .withButton ("Save")
            .withButton ("Discard")
            .withButton ("Cancel");

        juce::AlertWindow::showAsync (opts,
            [this](const int r) {
                if (r == 1)
                {
                    // Save
                    const bool hasPath =
                        appEngine->getCurrentEditFile().getFullPathName().isNotEmpty();
                    if (hasPath)
                    {
                        if (appEngine->saveEdit())
                            appEngine->newUntitledEdit();
                    }
                    else
                    {
                        appEngine->saveEditAsAsync ([this](const bool ok) {
                            if (ok)
                                appEngine->newUntitledEdit();
                        });
                    }
                }
                else if (r == 2)
                {
                    // Discard
                    appEngine->newUntitledEdit();
                }
            });
    }
    else
    {
        appEngine->newUntitledEdit();
    }
}

void TrackEditView::showOpenEditMenu () const
{
    if (!appEngine->isDirty())
    {
        appEngine->openEditAsync();
        return;
    }

    const auto opts = juce::MessageBoxOptions()
        .withIconType (juce::MessageBoxIconType::WarningIcon)
        .withTitle ("Save changes?")
        .withMessage ("You have unsaved changes.")
        .withButton ("Save")
        .withButton ("Discard")
        .withButton ("Cancel");

    juce::AlertWindow::showAsync (opts,
        [this](const int result) {
            if (result == 1) // Save
            {
                if (appEngine->getCurrentEditFile().getFullPathName().isNotEmpty())
                {
                    if (appEngine->saveEdit())
                        appEngine->openEditAsync();
                }
                else
                {
                    appEngine->saveEditAsAsync ([this](const bool ok) {
                        if (ok)
                            appEngine->openEditAsync();
                    });
                }
            }
            else if (result == 2) // Discard
            {
                appEngine->openEditAsync();
            }
        });
}

void TrackEditView::showPianoRoll (te::MidiClip* clip)
{
    if (pianoRoll == nullptr)
    {
        pianoRoll = std::make_unique<PianoRollEditor> (*appEngine, clip);
        addAndMakeVisible (pianoRoll.get());
    }
    else
    {
        pianoRoll->setClip (clip);
    }

    pianoRollClip = clip;
    pianoRoll->setVisible (true);
    resized();
}

void TrackEditView::hidePianoRoll ()
{
    pianoRollVisible = false;
    if (pianoRoll) pianoRoll->setVisible(false);
    pianoRollClip = nullptr;
    resized();
}

int TrackEditView::getPianoRollIndex () const
{
    if (pianoRollClip == nullptr || !appEngine)
        return -1;

    const int n = appEngine->getNumTracks();
    for (int i = 0; i < n; ++i)
    {
        auto clips = appEngine->getMidiClipsFromTrack (i);
        for (auto* mc : clips)
            if (mc == pianoRollClip)
                return i;
    }
    return -1;
}

void TrackEditView::labelTextChanged (juce::Label* labelThatHasChanged)
{
    if (labelThatHasChanged == &bpmEditField)
    {
        std::string text = labelThatHasChanged->getText().toStdString();

        // Validate numeric input
        if (!ValidationUtils::isValidNumeric(text))
        {
            labelThatHasChanged->setText (juce::String (appEngine->getBpm()), juce::NotificationType::dontSendNotification);
            return;
        }

        // Convert to double and apply constraints/rounding
        double bpmValue = std::stod(text);
        bpmValue = ValidationUtils::constrainAndRoundBpm(bpmValue);

        // Update label with constrained and rounded value
        labelThatHasChanged->setText(juce::String(bpmValue, 2), juce::NotificationType::dontSendNotification);

        // Update AppEngine with the constrained and rounded value
        appEngine->setBpm(bpmValue);
    }
}

void TrackEditView::PianoRollResizerBar::hasBeenMoved ()
{
    // DBG("X: " << this->getX() << " Y: " << this->getY());
    resized();
}

void TrackEditView::PianoRollResizerBar::mouseDrag (const juce::MouseEvent& event)
{
    // DBG("X: " << this->getX() << " Y: " << this->getY());
    // this->setTopLeftPosition (this->getX(), event.getPosition().getY());
    hasBeenMoved();
}

TrackEditView::PianoRollResizerBar::PianoRollResizerBar (juce::StretchableLayoutManager* layoutToUse, int itemIndexInLayout, bool isBarVertical)
    : StretchableLayoutResizerBar (layoutToUse, itemIndexInLayout, isBarVertical)
{
}

TrackEditView::PianoRollResizerBar::~PianoRollResizerBar ()
= default;

