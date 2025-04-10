//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"


NoteGridComponent::NoteGridComponent() {
    blackPitches = {1, 3, 6, 8, 10};
    setSize(getParentWidth() * 0.85, getParentHeight() * 1.2);
}

NoteGridComponent::~NoteGridComponent() {

}

void NoteGridComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey);

    const float noteCompHeight = getHeight() / 128.0;
    const int totalBars = (getWidth() / noteCompHeight) + 1;


    // Draw the background first
    {
        float line = 0;//noteCompHeight;

        for (int i = 127; i >= 0; i--) {
            const int pitch = i % 12;
            g.setColour(blackPitches.contains(pitch) ?
                        juce::Colours::darkgrey.withAlpha(0.5f) :
                        juce::Colours::lightgrey.darker().withAlpha(0.5f));

            g.fillRect(0, (int)line, getWidth(), (int)noteCompHeight);
            //            g.setColour(Colours::white);
            //            g.drawText(String(i), 5, line, 40, noteCompHeight, Justification::left);

            line += noteCompHeight;
            g.setColour(juce::Colours::black);
            g.drawLine(0, line, getWidth(), line);
        }
    }
}

void NoteGridComponent::resized() {
}
