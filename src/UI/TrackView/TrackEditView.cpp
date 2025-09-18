#include "TrackEditView.h"
#include "../../AppEngine/AppEngine.h"
#include "PopupWindows/OutputDevice/OutputDeviceWindow.h"

/**
 * A class that defines the contents and behaviour of the main menu bar.
 */
class TrackEditView::MainMenuModel final : public juce::MenuBarModel
{
public:
    explicit MainMenuModel (TrackEditView* view) : trackEditView (view) {}

    // Define unique IDs for our menu items
    enum MenuIDs {
        NewTrack = 1001,
        OpenMixer = 1002,
        ShowOutputSettings = 1003
    };

    juce::StringArray getMenuBarNames() override
    {
        return { "File", "View", "Track", "Help" };
    }

    juce::PopupMenu getMenuForIndex (int topLevelMenuIndex, const juce::String& /*menuName*/) override
    {
        juce::PopupMenu menu;

        if (topLevelMenuIndex == 0) // File
        {
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

    void menuItemSelected (const int menuItemID, int /*topLevelMenuIndex*/) override
    {
        switch (menuItemID)
        {
            case NewTrack:
                trackEditView->showNewTrackMenu();
                break;
            case OpenMixer:
                if (trackEditView->onOpenMix)
                    trackEditView->onOpenMix();
                break;
            case ShowOutputSettings:
                trackEditView->showOutputDeviceSettings();
                break;
            default:
                break;
        }
    }

private:
    TrackEditView* trackEditView; // A pointer back to the main component
};

// Helper for styling the menu buttons
void styleMenuButton (juce::TextButton& button)
{
    button.setColour (juce::TextButton::buttonColourId, juce::Colours::transparentBlack);
    button.setColour (juce::TextButton::textColourOffId, juce::Colours::lightgrey);
    button.setColour (juce::TextButton::textColourOnId, juce::Colours::white);
}

TrackEditView::TrackEditView (AppEngine& engine)
{
    appEngine = std::shared_ptr<AppEngine> (&engine, [] (AppEngine*) {});

    menuModel = std::make_unique<MainMenuModel> (this);
    menuBar = std::make_unique<juce::MenuBarComponent> (menuModel.get());
    addAndMakeVisible (menuBar.get());

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
    auto topBar = r.removeFromTop (40);
    viewport.setBounds (r);

    auto topBarContent = topBar.reduced (10, 0);

    // --- Menu ---
    menuBar->setBounds (topBarContent.removeFromLeft (200));

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
}

void TrackEditView::showNewTrackMenu()
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

    auto screenBounds = menuBar->getScreenBounds();
    juce::CallOutBox::launchAsynchronously (std::unique_ptr<Component> (content), screenBounds, nullptr);
}
