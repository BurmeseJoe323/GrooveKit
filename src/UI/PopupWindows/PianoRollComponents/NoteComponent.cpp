//
// Created by Joseph Rockwell on 4/13/25.
//

#include "NoteComponent.h"

#define CHECK_EDIT if(styleSheet.disableEditing) {return;}

NoteComponent::NoteComponent(GridStyleSheet styleSheet) : styleSheet(styleSheet),
                                                          edgeResizer(this, nullptr,
                                                                      juce::ResizableEdgeComponent::Edge::rightEdge) {
    mouseOver = useCustomColour = velocityEnabled = resizeEnabled = false;
    addAndMakeVisible(edgeResizer);
    setMouseCursor(normal);
    startWidth = startX = startY = -1;
    coordinatesDiffer = false;
    isMultiDrag = false;
    state = eNone;
    startVelocity = 0;

    setCustomColour(juce::Colours::green);
}

void NoteComponent::paint(juce::Graphics &g) {
    g.fillAll(juce::Colours::darkgrey);

    // Set color
    juce::Colour colourToUse;
    if (useCustomColour && model.flags.isGenerative) {
        colourToUse = customColour;
    } else {
        colourToUse = juce::Colour(252, 97, 92);
    }

    if (state == eSelected || mouseOver) {
        colourToUse = colourToUse.brighter(0.8);
    }
    g.setColour(colourToUse);

    // Draw middle box
    g.fillRect(1, 1, getWidth() - 2, getHeight() - 2);

    // if (styleSheet.getDrawVelocity() && getWidth() > 10) {
    //     g.setColour(colourToUse.brighter());
    //     const int lineMax = getWidth() - 5;
    //     g.drawLine(5, getHeight() * 0.5 - 2, lineMax * (model.getVelocity() / 127.0), getHeight() * 0.5 - 2, 4);
    // }

    // juce::String toDraw;
    // if (styleSheet.getDrawMIDINoteStr()) {
    //     toDraw += juce::String(PRE::pitches_names[model.getNote() % 12]) + juce::String(model.getNote() / 12) +
    //             juce::String(" ");
    // }
    // if (styleSheet.getDrawMIDINum()) {
    //     toDraw += juce::String(model.getNote());
    // }

    // g.setColour(juce::Colours::white);
    // g.drawText(juce::String(toDraw), 3, 3, getWidth() - 6, getHeight() - 6, juce::Justification::centred);
}

void NoteComponent::resized() {
    edgeResizer.setBounds(getWidth() - 10, getHeight(), 10, getHeight());
}

void NoteComponent::setCustomColour(juce::Colour c) {
    customColour = c;
    useCustomColour = true;
}

void NoteComponent::setValues(NoteModel m) {
    if (m.getNote() == 255) { m.setNote(0); } //unsigned overflow
    if (m.getNote() > 127) { m.setNote(127); }

    //cast to int as noteLen is unsigned. allows us to check for 0
    if (((int) m.getStartTime()) < 0) { m.setStartTime(0); }

    model = m;
    repaint();
}

NoteModel NoteComponent::getModel() {
    return model;
}

NoteModel *NoteComponent::getModelPtr() {
    return &model;
}

void NoteComponent::setState(eState s) {
    state = s;
    repaint();
}

NoteComponent::eState NoteComponent::getState() {
    return state;
}

void NoteComponent::mouseEnter(const juce::MouseEvent &) {
    CHECK_EDIT
    mouseOver = true;
    repaint();
}

void NoteComponent::mouseExit(const juce::MouseEvent &) {
    CHECK_EDIT
    mouseOver = false;
    setMouseCursor(juce::MouseCursor::NormalCursor);
    repaint();
}

void NoteComponent::mouseDown(const juce::MouseEvent &e) {
    CHECK_EDIT

    if (e.mods.isShiftDown()) {
        velocityEnabled = true;
        startVelocity = model.getVelocity();
    } else if (getWidth() - e.getMouseDownX() < 10) {
        resizeEnabled = true;
        startWidth = getWidth();
    } else {
        startDraggingComponent(this, e);
    }
    if (!resizeEnabled) {
    }
}

void NoteComponent::mouseUp(const juce::MouseEvent &e) {
    CHECK_EDIT
    if (onPositionMoved != nullptr) {
        onPositionMoved(this);
    }
    if (onNoteSelect != nullptr) {
        onNoteSelect(this, e);
    }
    startWidth = startX = startY - 1;
    mouseOver = false;
    resizeEnabled = false;
    velocityEnabled = false;
    repaint();
    isMultiDrag = false;
}

void NoteComponent::mouseDrag(const juce::MouseEvent &e) {
    CHECK_EDIT
    //velocityEnabled
    if (resizeEnabled) {
        if (onLengthChange != nullptr) {
            onLengthChange(this, startWidth - e.getPosition().getX());
        }
    } else if (velocityEnabled) {
        int velocityDiff = e.getDistanceFromDragStartY() * -0.5;
        int newVelocity = startVelocity + velocityDiff;
        if (newVelocity < 1) {
            newVelocity = 1;
        } else if (newVelocity > 127) {
            newVelocity = 127;
        }
        model.setVelocity(newVelocity);
        repaint();
        //        std::cout << velocityDiff << "\n";
    } else {
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        dragComponent(this, e, nullptr);

        if (onDragging != nullptr) {
            //&& isMultiDrag
            onDragging(this, e);
        }
    }
}

void NoteComponent::mouseMove(const juce::MouseEvent &e) {
    CHECK_EDIT
    if (getWidth() - e.getMouseDownX() < 10) {
        setMouseCursor(juce::MouseCursor::RightEdgeResizeCursor);
    } else {
        setMouseCursor(juce::MouseCursor::NormalCursor);
    }
}
