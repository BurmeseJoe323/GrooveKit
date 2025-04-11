//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"


NoteGridComponent::NoteGridComponent() {
    blackPitches = {1, 3, 6, 8, 10};
    setSize(getParentWidth() * 0.85, getParentHeight() * 1.2);

    // addChildComponent(&selectorBox);
    // addKeyListener(this);
    // setWantsKeyboardFocus(true);
    // currentQValue = PRE::quantisedDivisionValues[PRE::eQuantisationValue1_32];
    // lastNoteLength = PRE::quantisedDivisionValues[PRE::eQuantisationValue1_4];
    // firstDrag = false;
    // firstCall = false;
    // lastTrigger = -1;
    // ticksPerTimeSignature = PRE::defaultResolution * 4; //4/4 assume
}

NoteGridComponent::~NoteGridComponent() {
    // Destroys all children MIDI note components
    // for (int i = 0; i < noteComps.size(); i++) {
    //     removeChildComponent(noteComps[i]);
    //     delete noteComps[i];
    // }
}

void NoteGridComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey);

    const float noteCompHeight = getHeight() / 128.0;
    // Roughly controls the "width" of each cell
    const float pixelsPerBar = 300; // TODO: get this value from parent or other component
    const int totalBars = (getWidth() / noteCompHeight) + 1;


    // Draw the background first
    float line = 0; //noteCompHeight;

    for (int i = 127; i >= 0; i--) {
        const int pitch = i % 12;
        g.setColour(blackPitches.contains(pitch)
                        ? juce::Colours::darkgrey.withAlpha(0.5f)
                        : juce::Colours::lightgrey.darker().withAlpha(0.5f));

        g.fillRect(0, juce::detail::floorAsInt(line), getWidth(), juce::detail::floorAsInt(noteCompHeight));

        line += noteCompHeight;
        g.setColour(juce::Colours::black);
        g.drawLine(0, floor(line),getWidth(), floor(line));
    }

    // Draw bar lines
    // NOTE: currently assuming 4/4, should be made adjustable in the future
    const float increment = pixelsPerBar / 16;
    line = 0;
    g.setColour(juce::Colours::black);
    for (int i = 0; line < getWidth(); i++) {
        float lineThickness = 1.0;
            // Bar marker
        if (i % 16 == 0) {
            lineThickness = 3.0;
        } else if (i % 4 == 0) {
            // Quarter-note div
            lineThickness = 2.0;
        }
        g.drawLine(line, 0, line, getHeight(), lineThickness);

        line += increment;
    }
}

void NoteGridComponent::resized() {
    // for (auto component: noteComps) {
    //     if (component->coordiantesDiffer) {
    //         noteCompPositionMoved(component, false);
    //     }
    //     // convert from model representation into component representation (translation and scale)
    //
    //     const float xPos = (component->getModel().getStartTime() / ((float) ticksPerTimeSignature)) * pixelsPerBar;
    //     const float yPos = (getHeight() - (component->getModel().getNote() * noteCompHeight)) - noteCompHeight;
    //
    //     float len = (component->getModel().getNoteLegnth() / ((float) ticksPerTimeSignature)) * pixelsPerBar;
    //
    //     component->setBounds(xPos, yPos, len, noteCompHeight);
    // }
}
