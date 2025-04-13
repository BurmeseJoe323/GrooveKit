//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"


NoteGridComponent::NoteGridComponent(GridStyleSheet &sheet) : styleSheet(sheet) {
    blackPitches = {1, 3, 6, 8, 10};

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

    // Draw the background first
    float line = 0;

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

    // TODO: add to TODO document
    // Currently assuming 4/4, should be made adjustable in the future.
    // Probably the best way to make this adjustable is to make the timeline have more control over where the
    // bar and note lines in the grid are drawn, and be able to change where those lines appear. Also
    // quantisation should be drawn into the grid

    // Draw bar lines
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

void NoteGridComponent::setupGrid(float pixelsPerBar, float compHeight, const int bars) {
    this->pixelsPerBar = pixelsPerBar;
    noteCompHeight = compHeight;
    setSize(pixelsPerBar * bars, compHeight * 128); //we have 128 slots for notes
}

void NoteGridComponent::setQuantisation(const int val) {
    if (val >= 0 && val < PRE::eQuantisationValueTotal) {
        currentQValue = PRE::quantisedDivisionValues[val];
    }
    // NOTE: again... should probably do something else here rather than jassertfalse
    else {
        jassertfalse;
    }
}

float NoteGridComponent::getNoteCompHeight() const {
    return noteCompHeight;
}

float NoteGridComponent::getPixelsPerBar() const {
    return pixelsPerBar;
}

