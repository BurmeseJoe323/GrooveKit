#include "AppEngine.h"
#include "MainComponent.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <melatonin_inspector/melatonin_inspector.h>
#include <memory>

class GrooveKitApplication final : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "GrooveKit"; }
    const juce::String getApplicationVersion() override    { return "1.0.0"; }

    void initialise (const juce::String&) override
    {
        mainWindow = std::make_unique<MainWindow>(getApplicationName());
    }

    void shutdown() override
    {
        mainWindow = nullptr;
    }

    class MainWindow final : public juce::DocumentWindow
    {
    public:
        explicit MainWindow (const juce::String& name)
            : juce::DocumentWindow (name,
                  juce::Desktop::getInstance().getDefaultLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId),
                  juce::DocumentWindow::allButtons)
        {
            setUsingNativeTitleBar (true);
            setContentOwned (new MainComponent(), true);
            setResizable (true, true);
            centreWithSize (getWidth(), getHeight());
            Component::setVisible (true);
            inspector.setVisible (true);
            juce::Logger::outputDebugString ("== APP STARTED ==");
        }

        void closeButtonPressed() override
        {
            juce::JUCEApplication::getInstance()->systemRequestedQuit();
        }

    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainWindow)
        melatonin::Inspector inspector { *this, false };
    };

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION (GrooveKitApplication)
