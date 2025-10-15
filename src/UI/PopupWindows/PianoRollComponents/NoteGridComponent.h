//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef NOTEGRIDCOMPONENT_H
#define NOTEGRIDCOMPONENT_H

#include "../../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>

#include "GridStyleSheet.h"
#include "NoteComponent.h"
#include "PConstants.h"

namespace te = tracktion;

class SelectionBox : public juce::Component
{
public:
    void paint (juce::Graphics& g)
    {
        juce::Colour c = juce::Colours::white;
        c = c.withAlpha ((float) 0.5);
        g.fillAll (c);
    }

    int startX, startY;
};

/**
 * Grid where notes are drawn.
 */
class NoteGridComponent : public juce::Component, public juce::KeyListener
{
public:
    // Public struct for defining time signatures
    struct TimeSignature
    {
        unsigned int beatsPerBar;
        unsigned int beatValue;
    };
    NoteGridComponent (GridStyleSheet& sheet, AppEngine& engine, int trackIndex);
    ~NoteGridComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    void noteCompSelected (NoteComponent*, const juce::MouseEvent&);
    void noteCompPositionMoved (NoteComponent*, bool callResize = true);
    void noteCompLengthChanged (NoteComponent*, int diff);
    void noteCompDragging (NoteComponent*, const juce::MouseEvent&);
    void noteEdgeDragging (NoteComponent*, const juce::MouseEvent&);
    void setPositions();
    void setTimeSignature (unsigned int beatsPerBar, unsigned int beatValue);

    /* optional
    void mouseEnter (const MouseEvent&);
    void mouseExit  (const MouseEvent&);
    void mouseMove  (const MouseEvent&);
    */

    void mouseDown (const juce::MouseEvent&) override;
    void mouseDrag (const juce::MouseEvent&) override;
    void mouseUp (const juce::MouseEvent&) override;
    void mouseDoubleClick (const juce::MouseEvent&) override;

    void setupGrid (float pixelsPerBar, float compHeight, const int bars);
    void setQuantisation (float newVal);

    bool keyPressed (const juce::KeyPress& key, Component*) override;
    void deleteAllSelected();

    // From here you could convert this into MIDI or any other custom musical encoding.
    te::MidiList& getSequence();

    float getNoteCompHeight() const;
    float getPixelsPerBar() const;

    juce::Array<te::MidiNote*> getSelectedModels();

    std::function<void (int note, int velocity)> sendChange;
    std::function<void()> onEdit;

private:
    void sendEdit();

    NoteComponent* addNewNoteComponent(te::MidiNote*);

    // The appEngine is where all MIDI information is stored. The trackIndex is used to locate the correct track inside
    // the engine
    AppEngine& appEngine;
    int trackIndex;

    GridStyleSheet& styleSheet;
    SelectionBox selectorBox;
    std::vector<NoteComponent*> noteComps;

    std::set<int> blackPitches = { 1, 3, 6, 8, 10 };
    float noteCompHeight;
    float pixelsPerBar;
    TimeSignature timeSignature;
    st_int ticksPerTimeSignature;
    float currentQValue;
    // st_int lastNoteLength;
    bool firstDrag;
    bool firstCall;
    int lastTrigger;

    float beatsToX (float beats);
    float pitchToY (float pitch);
    float xToBeats (float x);
    int yToPitch (float y);
};

#endif //NOTEGRIDCOMPONENT_H