#include <juce_gui_basics/juce_gui_basics.h>

#include "MainComponent.h"
#include "AppEngine.h"


class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name)
        : juce::DocumentWindow(name,
                                juce::Desktop::getInstance().getDefaultLookAndFeel()
                                    .findColour(juce::ResizableWindow::backgroundColourId),
                                juce::DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
        juce::Logger::outputDebugString("== APP STARTED ==");
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }

};

class GrooveKitApplication : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "GrooveKit"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }

    void initialise(const juce::String&) override
    {
        mainWindow.reset(new MainWindow(getApplicationName()));
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(GrooveKitApplication)
