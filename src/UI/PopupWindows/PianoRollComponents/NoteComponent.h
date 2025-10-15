//
// Created by Joseph Rockwell on 4/13/25.
//

#ifndef NOTECOMPONENT_H
#define NOTECOMPONENT_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "PConstants.h"
# include "GridStyleSheet.h"

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion;

class NoteComponent : public juce::Component, public juce::ComponentDragger {
    public:
    enum eState {
        eNone,
        eSelected,
    };

    struct MultiEditState {
        int startWidth; // Used when resizing the noteComponents' length
        bool coordinatesDiffer;
        // Sometimes the size of this component gets changed externally (for example on multi-resizing) set this flag to be true and at
        juce::Rectangle<int> startingBounds;
    };

    NoteComponent(GridStyleSheet styleSheet);
    ~NoteComponent() override = default;

    void paint (juce::Graphics & g) override;
    void resized () override;
    void setCustomColour (juce::Colour c);

    void setModel(te::MidiNote *model);

    te::MidiNote *getModel();

    void setState (eState state);
    eState getState ();

    void mouseEnter (const juce::MouseEvent&) override;
    void mouseExit  (const juce::MouseEvent&) override;
    void mouseDown  (const juce::MouseEvent&) override;
    void mouseUp    (const juce::MouseEvent&) override;
    void mouseDrag  (const juce::MouseEvent&) override;
    void mouseMove  (const juce::MouseEvent&) override;

    std::function<void(NoteComponent *, const juce::MouseEvent &)> onNoteSelect;
    std::function<void(NoteComponent *)> onPositionMoved;
    // Send the drag event to all selected components
    std::function<void(NoteComponent *, const juce::MouseEvent &)> onDragging;
    // Send the note length change event to all selected components
    std::function<void(NoteComponent *, const juce::MouseEvent &)> onEdgeDragging;
    // Sends the difference as this is relative for all components
    std::function<void(NoteComponent *, int)> onLengthChange;

    int minWidth = 10;
     // Used when resizing the noteComponents length
    int startWidth;
    int startX, startY;
    // Sometimes the size of this component gets changed externally (for example on multi-resizing)
    // set this flag to be true and at some point the internal model will get updated also
    bool coordinatesDiffer;
    bool isMultiDrag;

private:
    GridStyleSheet &styleSheet;
    juce::ResizableEdgeComponent edgeResizer;

    bool mouseOver, useCustomColour, resizeEnabled, velocityEnabled;
    int startVelocity;

    juce::Colour customColour;

    te::MidiNote *model;
    juce::MouseCursor normal;
    eState state;
};

#endif //NOTECOMPONENT_H
