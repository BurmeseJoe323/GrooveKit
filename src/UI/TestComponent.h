#include "../AppEngine/AppEngine.h"
#include <vector>

class TestComponent : public juce::Component
{
public:
    TestComponent()
    {
        playButton.setButtonText("Play MIDI");
        playButton.onClick = [this] { engine.play(); };

        stopButton.setButtonText("Stop MIDI");
        stopButton.onClick = [this] { engine.stop(); };

        addTrackButton.setButtonText("Add MIDI Track");
        addTrackButton.onClick = [this] { engine.addMidiTrack(); };

        addClipButton.setButtonText("Add MIDI Clip");
        addClipButton.onClick = [this] {
            engine.addMidiClipToTrack(0);   // Actually adds a clip
            clipCount++;            // Keep track of visual clips
            repaint();              // Trigger UI redraw
        };

        addAndMakeVisible(playButton);
        addAndMakeVisible(stopButton);
        addAndMakeVisible(addTrackButton);
        addAndMakeVisible(addClipButton);

        setSize(400, 300);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        playButton.setBounds(area.removeFromTop(40).reduced(0, 5));
        stopButton.setBounds(area.removeFromTop(40).reduced(0, 5));
        addTrackButton.setBounds(area.removeFromTop(40).reduced(0, 5));
        addClipButton.setBounds(area.removeFromTop(40).reduced(0, 5));
    }

    void paint(juce::Graphics& g) override
    {
        // Draw each MIDI clip as a colored rectangle
        for (int i = 0; i < clipCount; ++i)
        {
            g.setColour(juce::Colours::skyblue);
            g.fillRect(40, 150 + i * 30, 200, 20); // (x, y, width, height)
            g.setColour(juce::Colours::black);
            g.drawText("MIDI Clip " + juce::String(i + 1), 45, 150 + i * 30, 200, 20, juce::Justification::centredLeft);
        }
    }

private:
    AppEngine engine;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton addTrackButton;
    juce::TextButton addClipButton;

    int clipCount = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestComponent)
};