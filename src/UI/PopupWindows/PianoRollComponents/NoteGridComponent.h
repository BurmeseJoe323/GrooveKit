//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef NOTEGRIDCOMPONENT_H
#define NOTEGRIDCOMPONENT_H

#include "../../../AppEngine/AppEngine.h"
#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include <memory>

#include "GridStyleSheet.h"
#include "NoteComponent.h"
#include "PConstants.h"

namespace te = tracktion::engine;

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
    NoteGridComponent (GridStyleSheet& sheet, AppEngine& engine, te::MidiClip* clip);
    ~NoteGridComponent() override;

    // Change the clip being edited/displayed
    void setClip (te::MidiClip* newClip);

    void paint (juce::Graphics& g) override;
    void resized() override;

    void noteCompSelected (NoteComponent*, const juce::MouseEvent&);
    void noteCompPositionMoved (NoteComponent*, bool callResize = true);
    void noteCompLengthChanged (NoteComponent*);
    void noteCompDragging (NoteComponent*, const juce::MouseEvent&);
    void noteEdgeDragging (NoteComponent*, const juce::MouseEvent&);
    void setPositions();
    void setTimeSignature (unsigned int beatsPerBar, unsigned int beatValue);
    unsigned int getBeatsPerBar() const { return timeSignature.beatsPerBar; }

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

    void setActiveClip (te::MidiClip* clip);
    te::MidiClip* getActiveClip();
    const te::MidiClip* getActiveClip() const;

    juce::Array<te::MidiNote*> getSelectedModels();

    std::function<void (int note, int velocity)> sendChange;
    std::function<void()> onEdit;

private:
    void sendEdit();

    NoteComponent* addNewNoteComponent(te::MidiNote*);

    // The appEngine provides access to engine services; this component now targets a specific MIDI clip
    AppEngine& appEngine;
    te::MidiClip* clip = nullptr; // not owned

    GridStyleSheet& styleSheet;
    SelectionBox selectorBox;
    std::vector<std::unique_ptr<NoteComponent>> noteComps;

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

    te::MidiClip* clipModel = nullptr;
    const te::MidiClip* resolveClip() const;

    float beatsToX (float beats);
    float pitchToY (float pitch);
    float xToBeats (float x);
    int yToPitch (float y);
};

#endif //NOTEGRIDCOMPONENT_H