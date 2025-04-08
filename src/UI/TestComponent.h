#pragma once

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
        addTrackButton.onClick = [this] {
            juce::Logger::outputDebugString("Add MIDI Track clicked");
            engine.addMidiTrack();
            trackCount++;
            createTrackHeader(trackCount);
            resized();
            repaint();
        };

        addAndMakeVisible(playButton);
        addAndMakeVisible(stopButton);
        addAndMakeVisible(addTrackButton);

        setSize(500, 400);
    }

    void resized() override
    {
        auto area = getLocalBounds().reduced(20);
        addTrackButton.setBounds(area.removeFromTop(40).reduced(0, 5));
        playButton.setBounds(area.removeFromTop(40).reduced(0, 5));
        stopButton.setBounds(area.removeFromTop(40).reduced(0, 5));

        // Layout track headers
        int y = area.getY();
        for (auto& [trackLabel, plusButton] : trackHeaders)
        {
            trackLabel->setBounds(20, y, 200, 30);
            plusButton->setBounds(230, y, 30, 30);
            y += 40;
        }
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
    }

private:
    AppEngine engine;
    juce::TextButton playButton, stopButton, addTrackButton;

    int trackCount = 0;

    // Track header: pair of label and plus button
    std::vector<std::pair<std::unique_ptr<juce::Label>, std::unique_ptr<juce::TextButton>>> trackHeaders;

    void createTrackHeader(int trackIndex)
    {
        auto label = std::make_unique<juce::Label>("TrackLabel", "Track " + juce::String(trackIndex + 1));
        label->setFont(juce::Font(16.0f));
        label->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(label.get());

        auto plusButton = std::make_unique<juce::TextButton>("+");
        plusButton->onClick = [this, trackIndex] {
            engine.addMidiClipToTrack(trackIndex);
        };
        addAndMakeVisible(plusButton.get());

        trackHeaders.emplace_back(std::move(label), std::move(plusButton));
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestComponent)
};
