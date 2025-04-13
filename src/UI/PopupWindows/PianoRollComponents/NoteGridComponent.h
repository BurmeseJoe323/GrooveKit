//
// Created by Joseph Rockwell on 4/8/25.
//

#ifndef NOTEGRIDCOMPONENT_H
#define NOTEGRIDCOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>

#include "GridStyleSheet.h"
#include "PConstants.h"

class NoteGridComponent : public juce::Component {
public:
    NoteGridComponent(GridStyleSheet &sheet);
    ~NoteGridComponent() override;

    void paint (juce::Graphics & g) override;
    void resized () override;

    // void noteCompSelected (PNoteComponent *, const MouseEvent&);
    // void noteCompPositionMoved (PNoteComponent *, bool callResize = true);
    // void noteCompLengthChanged (PNoteComponent *, int diff);
    // void noteCompDragging (PNoteComponent*, const MouseEvent&);
    // void setPositions ();

    /* optional
    void mouseEnter (const MouseEvent&);
    void mouseExit  (const MouseEvent&);
    void mouseMove  (const MouseEvent&);
    */
    // void mouseDown  (const MouseEvent&) override;
    // void mouseDrag  (const MouseEvent&) override;
    // void mouseUp    (const MouseEvent&) override;
    // void mouseDoubleClick (const MouseEvent&) override;

    void setupGrid (float pixelsPerBar, float compHeight, const int bars);
    void setQuantisation (const int val);

    // bool keyPressed (const KeyPress& key, Component* originatingComponent) override;
    // void deleteAllSelected ();
    //
    // // From here you could convert this into MIDI or any other custom musical encoding.
    // PRESequence getSequence ();
    // void loadSequence (PRESequence sq);

    float getNoteCompHeight() const;
    float getPixelsPerBar() const;

    // std::vector<NoteModel *> getSelectedModels ();
    //
    // std::function<void(int note,int velocity)> sendChange;
    // std::function<void()> onEdit;
private:
    // void sendEdit ();

    GridStyleSheet &styleSheet;
    // SelectionBox selectorBox;
    // std::vector<PNoteComponent *> noteComps;

    juce::Array<int> blackPitches;

    float noteCompHeight;
    float pixelsPerBar;
    st_int ticksPerTimeSignature;
    st_int currentQValue;
    st_int lastNoteLength;
    // bool firstDrag;
    // bool firstCall;
    // int lastTrigger;
};



#endif //NOTEGRIDCOMPONENT_H
