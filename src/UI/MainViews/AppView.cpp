//
// Created by Joseph Rockwell on 4/7/25.
//

#include "AppView.h"
#include "../PopupWindows/PianoRollWindow.h"
AppView::AppView() {
    playButton.setButtonText("Play MIDI");
    playButton.onClick = [this] { engine.play(); };

    stopButton.setButtonText("Stop MIDI");
    stopButton.onClick = [this] { engine.stop(); };

    addTrackButton.setButtonText("Add MIDI Track");
    addTrackButton.onClick = [this] {
        engine.addMidiTrack();
        trackCount = engine.getNumTracks();
        selectedTrackIndex = trackCount - 1;
    };

    addClipButton.setButtonText("Add MIDI Clip");
    addClipButton.onClick = [this] {
        engine.addMidiClipToTrack(trackCount - 1); // Actually adds a clip
        clipCount++; // Keep track of visual clips
        repaint(); // Trigger UI redraw
    };

    openPianoRollWindow.setButtonText("Open piano roll");
    openPianoRollWindow.onClick = [this] {
        openPianoRoll();
    };

    addAndMakeVisible(playButton);
    addAndMakeVisible(stopButton);
    addAndMakeVisible(addTrackButton);
    addAndMakeVisible(addClipButton);
    addAndMakeVisible(openPianoRollWindow);

    setSize(400, 300);
}

void AppView::resized() {
    auto area = getLocalBounds().reduced(20);
    addTrackButton.setBounds(area.removeFromTop(40).reduced(0, 5));
    addClipButton.setBounds(area.removeFromTop(40).reduced(0, 5));
    playButton.setBounds(area.removeFromTop(40).reduced(0, 5));
    stopButton.setBounds(area.removeFromTop(40).reduced(0, 5));
    openPianoRollWindow.setBounds(area.removeFromTop(40).reduced(0, 5));
}

void AppView::paint (juce::Graphics& g) {
    // Draw each MIDI clip as a colored rectangle
    for (int i = 0; i < clipCount; ++i) {
        g.setColour(juce::Colours::skyblue);
        g.fillRect(40, 150 + i * 30, 200, 20); // (x, y, width, height)
        g.setColour(juce::Colours::black);
        g.drawText("MIDI Clip " + juce::String(i + 1), 45, 150 + i * 30, 200, 20, juce::Justification::centredLeft);
    }
}

void AppView::openPianoRoll() {
    if (pianoRollWindow == nullptr) {
        pianoRollWindow = std::make_unique<PianoRollWindow>();
    } else {
        pianoRollWindow->setVisible(true);
    }
}

void AppView::closePianoRoll() {
    pianoRollWindow = nullptr;
}
