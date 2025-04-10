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
            createTrackHeader();
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

        int y = area.getY();
        for (int i = 0; i < trackHeaders.size(); ++i)
        {
            auto& header = trackHeaders[i];
            header.label->setBounds(20, y, 200, 30);
            header.addClipButton->setBounds(230, y, 30, 30);
            header.deleteButton->setBounds(270, y, 30, 30);
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

    struct TrackHeader
    {
        std::unique_ptr<juce::Label> label;
        std::unique_ptr<juce::TextButton> addClipButton;
        std::unique_ptr<juce::TextButton> deleteButton;
    };

    std::vector<TrackHeader> trackHeaders;

    void createTrackHeader()
    {
        int trackIndex = static_cast<int>(trackHeaders.size());
        TrackHeader header;

        header.label = std::make_unique<juce::Label>();
        header.label->setText("Track " + juce::String(trackIndex + 1), juce::dontSendNotification);
        header.label->setFont(juce::Font(16.0f));
        header.label->setColour(juce::Label::textColourId, juce::Colours::white);
        addAndMakeVisible(header.label.get());

        header.addClipButton = std::make_unique<juce::TextButton>("+");
        addAndMakeVisible(header.addClipButton.get());

        header.deleteButton = std::make_unique<juce::TextButton>("-");
        addAndMakeVisible(header.deleteButton.get());

        trackHeaders.push_back(std::move(header));
        updateTrackHeaderLabels();
    }

    void updateTrackHeaderLabels()
    {
        for (int i = 0; i < trackHeaders.size(); ++i)
        {
            trackHeaders[i].label->setText("Track " + juce::String(i + 1), juce::dontSendNotification);

            trackHeaders[i].addClipButton->onClick = [this, i] {
                engine.addMidiClipToTrack(i);
            };

            trackHeaders[i].deleteButton->onClick = [this, i] {
                engine.deleteMidiTrack(i);
                removeChildComponent(trackHeaders[i].label.get());
                removeChildComponent(trackHeaders[i].addClipButton.get());
                removeChildComponent(trackHeaders[i].deleteButton.get());
                trackHeaders.erase(trackHeaders.begin() + i);
                updateTrackHeaderLabels();
                resized();
                repaint();
            };
        }
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TestComponent)
};