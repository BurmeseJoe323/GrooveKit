#include "TrackEditView.h"
#include "../../AppEngine/AppEngine.h"
#include "PopupWindows/OutputDevice/OutputDeviceWindow.h"

// Helper for styling the menu buttons
void styleMenuButton (juce::TextButton& button)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colour (0x00000000));
    button.setColour (juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

TrackEditView::TrackEditView (AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine> (&engine, [] (AppEngine*) {});

#if JUCE_MAC
    juce::MenuBarModel::setMacMainMenu (this);
#else
    menuBar = std::make_unique<juce::MenuBarComponent> (this);
    addAndMakeVisible (menuBar.get());
#endif

    trackList = std::make_unique<TrackListComponent> (appEngine);

    trackList->setPixelsPerSecond (pixelsPerSecond);
    trackList->setViewStart (viewStart);

    viewport.setScrollBarsShown (true, false); // vertical only
    viewport.setViewedComponent (trackList.get(), false);

    setupButtons();
    addAndMakeVisible (viewport);
}

TrackEditView::~TrackEditView() = default;

void TrackEditView::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xFF343A40)); // Dark background for track area

    const auto topBarBounds = getLocalBounds().removeFromTop (40);
    g.setColour (juce::Colour (0xFF212529)); // Even darker for top bar
    g.fillRect (topBarBounds);
    g.setColour (juce::Colours::black.withAlpha (0.2f));
    g.drawHorizontalLine (topBarBounds.getBottom(), 0.0f, static_cast<float> (getWidth()));
}

void TrackEditView::resized()
{
    auto r = getLocalBounds();
    const auto topBar = r.removeFromTop (40);
    viewport.setBounds (r);

    auto topBarContent = topBar.reduced (10, 0);

// --- Menu ---
#if !JUCE_MAC
    if (menuBar)
        menuBar->setBounds (topBarContent.removeFromLeft (200));
#endif

    // --- Right side: Switch ---
    const auto switchArea = topBarContent.removeFromRight (50);
    switchButton.setBounds (switchArea);

    // --- Center: Transport ---
    auto centerArea = topBarContent;
    bpmLabel.setBounds (centerArea.removeFromLeft (80));
    clickLabel.setBounds (centerArea.removeFromLeft (50));

    constexpr int buttonSize = 20;
    constexpr int buttonGap = 10;
    constexpr int transportWidth = (buttonSize * 3) + (buttonGap * 2);
    auto transportBounds = centerArea.withSizeKeepingCentre (transportWidth, buttonSize);

    stopButton.setBounds (transportBounds.removeFromLeft (buttonSize));
    transportBounds.removeFromLeft (buttonGap);
    playButton.setBounds (transportBounds.removeFromLeft (buttonSize));
    transportBounds.removeFromLeft (buttonGap);
    recordButton.setBounds (transportBounds.removeFromLeft (buttonSize));
}

void TrackEditView::setupButtons()
{
    // --- Center Controls ---
    addAndMakeVisible (bpmLabel);
    bpmLabel.setText ("BPM 120", juce::dontSendNotification);
    bpmLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    bpmLabel.setJustificationType (juce::Justification::centred);

    addAndMakeVisible (clickLabel);
    clickLabel.setText ("Click", juce::dontSendNotification);
    clickLabel.setColour (juce::Label::textColourId, juce::Colours::lightgrey);
    clickLabel.setJustificationType (juce::Justification::centred);

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
    switchButton.onClick = [this] { if (onOpenMix) onOpenMix(); };
}

juce::StringArray TrackEditView::getMenuBarNames()
{
    return { "File", "View", "Track", "Help" };
}

juce::PopupMenu TrackEditView::getMenuForIndex (const int topLevelMenuIndex, const juce::String&)
{
    juce::PopupMenu menu;
    enum MenuIDs {
        NewTrack = 1001,
        OpenMixer = 1002,
        ShowOutputSettings = 1003,
        OpenEdit = 2001,
        SaveEdit = 2002,
        SaveEditAs = 2003
    };

    if (topLevelMenuIndex == 0) // File
    {
        menu.addItem (OpenEdit, "Open Edit...");
        menu.addSeparator();
        menu.addItem (SaveEdit, "Save Edit...");
        menu.addItem (SaveEditAs, "Save Edit As...");
        menu.addSeparator();
        menu.addItem (ShowOutputSettings, "Output Device Settings...");
    }
    else if (topLevelMenuIndex == 1) // View
    {
        menu.addItem (OpenMixer, "Mix View");
    }
    else if (topLevelMenuIndex == 2) // Track
    {
        menu.addItem (NewTrack, "New Track...");
    }
    return menu;
}

void TrackEditView::menuItemSelected (const int menuItemID, int)
{
    enum MenuIDs {
        NewTrack = 1001,
        OpenMixer = 1002,
        ShowOutputSettings = 1003,
        OpenEdit = 2001,
        SaveEdit = 2002,
        SaveEditAs = 2003
    };

    switch (menuItemID)
    {
        case NewTrack:
            showNewTrackMenu();
            break;
        case OpenMixer:
            if (onOpenMix)
                onOpenMix();
            break;
        case ShowOutputSettings:
            showOutputDeviceSettings(); // TODO : fix positioning
            break;
        case OpenEdit:
            DBG ("File -> Open Edit... selected");
            break;
        case SaveEdit:
            DBG ("File -> Save Edit... selected");
            break;
        case SaveEditAs:
            DBG ("File -> Save Edit As... selected");
            break;
        default:
            break;
    }
}

void TrackEditView::showNewTrackMenu() const
{
    juce::PopupMenu m;
    m.addItem (1, "Instrument (FourOsc)");
    m.addItem (2, "Drum (Sampler)");

    m.showMenuAsync (juce::PopupMenu::Options(), [this] (int choice) {
        if (!trackList || choice == 0)
            return;

        int index = -1;
        if (choice == 1)
            index = appEngine->addInstrumentTrack();
        else if (choice == 2)
            index = appEngine->addDrumTrack();

        if (index >= 0)
        {
            trackList->addNewTrack (index);
            trackList->setPixelsPerSecond (pixelsPerSecond);
            trackList->setViewStart (viewStart);
        }
    });
}

void TrackEditView::showOutputDeviceSettings()
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
