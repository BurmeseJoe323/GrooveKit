//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"

#define RETURN_IF_EDITING_DISABLED if(styleSheet.disableEditing) { return; }

NoteGridComponent::NoteGridComponent(GridStyleSheet &sheet) : styleSheet(sheet) {
    blackPitches = {1, 3, 6, 8, 10};

    addChildComponent(&selectorBox);
    addKeyListener(this);
    setWantsKeyboardFocus(true);
    currentQValue = PRE::quantisedDivisionValues[PRE::eQuantisationValue1_32];
    lastNoteLength = PRE::quantisedDivisionValues[PRE::eQuantisationValue1_4];
    firstDrag = false;
    firstCall = false;
    lastTrigger = -1;
    ticksPerTimeSignature = PRE::defaultResolution * 4; //4/4 assume
}

NoteGridComponent::~NoteGridComponent() {
    // Destroys all children MIDI note components
    for (int i = 0; i < noteComps.size(); i++) {
        removeChildComponent(noteComps[i]);
        delete noteComps[i];
    }
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
        g.drawLine(0, floor(line), getWidth(), floor(line));
    }

    // NOTE: Currently assuming 4/4, should be made adjustable in the future

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
    for (auto component: noteComps) {
        if (component->coordinatesDiffer) {
            noteCompPositionMoved(component, false);
        }
        // convert from model representation into component representation (translation and scale)

        const float xPos = (component->getModel().getStartTime() / ((float) ticksPerTimeSignature)) * pixelsPerBar;
        const float yPos = (getHeight() - (component->getModel().getNote() * noteCompHeight)) - noteCompHeight;

        float len = (component->getModel().getNoteLength() / ((float) ticksPerTimeSignature)) * pixelsPerBar;

        component->setBounds(xPos, yPos, len, noteCompHeight);
    }
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

void NoteGridComponent::noteCompSelected(NoteComponent *noteComponent, const juce::MouseEvent &e) {
    RETURN_IF_EDITING_DISABLED

    int dragMove = 0;
    for (auto component: noteComps) {
        if (component->isMultiDrag) {
            dragMove++;
        }
    }
    //    std::cout << "Drag: " << dragMove << "\n";

    for (auto component: noteComps) {
        if (component == noteComponent) {
            component->setState(NoteComponent::eSelected);
            component->toFront(true);
        }
        /*
         This complicated if statement exists because if the user is dragging multiple notes around
         we don't want to clear the selection. We only want so switch the selected note when
         the user selects another note
         */
        else if (component->getState() == NoteComponent::eSelected && !e.mods.isShiftDown() && !dragMove) {
            component->setState(NoteComponent::eNone);
        }
    }
    // Need to reset the multidrag
    for (auto component: noteComps) {
        if (component->isMultiDrag) {
            component->isMultiDrag = false;
        }
    }
    sendEdit();
}

void NoteGridComponent::noteCompPositionMoved(NoteComponent *comp, bool callResize) {
    RETURN_IF_EDITING_DISABLED

    if (!firstDrag) {
        firstDrag = true;
        // we want to move all the components...
        for (auto n: noteComps) {
            if (n != comp && n->getState() == NoteComponent::eSelected) {
                noteCompPositionMoved(n, false);
            }
        }
        firstDrag = false;
    }

    //could do with refactoring this code here..
    int xPos = (comp->getX() / ((float) pixelsPerBar)) * ticksPerTimeSignature;
    int note = 127 - (comp->getY() / noteCompHeight);
    if (note > 127) {
        note = 127;
    } else if (note < 0) {
        note = 0;
    }

    if (xPos <= 0) {
        xPos = 0;
    }

    const int len = (comp->getWidth() / ((float) pixelsPerBar)) * ticksPerTimeSignature;
    NoteModel nm = comp->getModel();
    nm.setNote(note);
    nm.setStartTime(xPos);
    nm.setNoteLength(len);
    nm.quantiseModel(currentQValue, true, true);
    nm.sendChange = sendChange;

    // TODO: could make this toggleable behaviour
    lastNoteLength = nm.getNoteLength();

    comp->startY = -1;
    comp->startX = -1;
    comp->setValues(nm);
    if (callResize) {
        resized();
    }
    sendEdit();
}

void NoteGridComponent::noteCompLengthChanged(NoteComponent *original, int diff) {
    RETURN_IF_EDITING_DISABLED

    for (auto n: noteComps) {
        if (n->getState() == NoteComponent::eSelected || n == original) {
            if (n->startWidth == -1) {
                n->startWidth = n->getWidth();
                n->coordinatesDiffer = true;
            }

            const int newWidth = n->startWidth - diff;
            // TODO: this seems arbitary..
            if (newWidth > 20) {
                n->setSize(newWidth, n->getHeight());
            }
        }
    }
    sendEdit();
}

void NoteGridComponent::noteCompDragging(NoteComponent *original, const juce::MouseEvent &event) {
    RETURN_IF_EDITING_DISABLED

    for (auto note: noteComps) {
        if (note->getState() == NoteComponent::eSelected && note != original) {
            const int movedX = event.getDistanceFromDragStartX(); // (event.getx - original->startX);
            const int movedY = event.getDistanceFromDragStartY(); //(original->getY() - original->startY);

            if (note->startY == -1) {
                note->startX = note->getX();
                note->startY = note->getY();
            }

            /*
            std::cout << "Started at: " << n->startX << " - " << n->startY << "\n";
            std::cout << n->getBounds().toString() << "\n";
             */

            const int newX = note->startX + movedX;
            const int newY = note->startY + movedY;
            const int xDif = abs(newX - note->startX);
            const int yDif = abs(newY - note->startY);
            if (xDif > 2 || yDif > 2) {
                //ingnore a small amount of jitter.
                note->setTopLeftPosition(newX, newY);
                note->isMultiDrag = true;
            }

            /*
            std::cout << "Moved: " << movedX << " : " << movedY << " -- " << n->getX() << " : " << n->getY() <<  "\n" ;
            std::cout << n->getBounds().toString() << "\n \n" ;
             */
        }
    }


    /*
     This enables the notes to be triggered while dragging.
     */
    int note = 127 - (original->getY() / noteCompHeight);
    if (note > 127) { note = 127; } else if (note < 0) { note = 0; }
    if (note != lastTrigger) {
        original->getModel().trigger(note, 100);
        lastTrigger = note;
    }
}

void NoteGridComponent::setPositions() {
    //unused..
}

void NoteGridComponent::mouseDown(const juce::MouseEvent &) {
    RETURN_IF_EDITING_DISABLED

    for (NoteComponent *component: noteComps) {
        component->setState(NoteComponent::eNone);
    }
    sendEdit();
}

void NoteGridComponent::mouseDrag(const juce::MouseEvent &e) {
    RETURN_IF_EDITING_DISABLED

    if (!selectorBox.isVisible()) {
        selectorBox.setVisible(true);
        selectorBox.toFront(false);

        selectorBox.setTopLeftPosition(e.getPosition());
        selectorBox.startX = e.getPosition().x;
        selectorBox.startY = e.getPosition().y;
    } else {
        int xDir = e.getPosition().x - selectorBox.startX;
        int yDir = e.getPosition().y - selectorBox.startY;

        // Work out which way to draw the selection box
        if (xDir < 0 && yDir < 0) {
            // Top left
            selectorBox.setTopLeftPosition(e.getPosition().x, e.getPosition().y);
            selectorBox.setSize(selectorBox.startX - e.getPosition().getX(),
                                selectorBox.startY - e.getPosition().getY());
        } else if (xDir > 0 && yDir < 0) {
            // Top right
            selectorBox.setTopLeftPosition(selectorBox.startX, e.getPosition().y);
            selectorBox.setSize(e.getPosition().getX() - selectorBox.startX,
                                selectorBox.startY - e.getPosition().getY());
        } else if (xDir < 0 && yDir > 0) {
            // Bottom left
            selectorBox.setTopLeftPosition(e.getPosition().x, selectorBox.startY);
            selectorBox.setSize(selectorBox.startX - e.getPosition().getX(),
                                e.getPosition().getY() - selectorBox.startY);
        } else {
            // Bottom right
            selectorBox.setSize(e.getPosition().getX() - selectorBox.getX(),
                                e.getPosition().getY() - selectorBox.getY());
        }
    }
}

void NoteGridComponent::mouseUp(const juce::MouseEvent &) {
    RETURN_IF_EDITING_DISABLED

    if (selectorBox.isVisible()) {
        for (NoteComponent *component: noteComps) {
            if (component->getBounds().intersects(selectorBox.getBounds())) {
                component->setState(NoteComponent::eState::eSelected);
            } else {
                component->setState(NoteComponent::eState::eNone);
            }
        }
        selectorBox.setVisible(false);
        selectorBox.toFront(false);
        selectorBox.setSize(1, 1);
    }

    sendEdit();
}

void NoteGridComponent::mouseDoubleClick(const juce::MouseEvent &e) {
    RETURN_IF_EDITING_DISABLED

    const int xPos = (e.getMouseDownX() / ((float) pixelsPerBar)) * ticksPerTimeSignature;
    const int yIn = ((float) e.getMouseDownY() / noteCompHeight);
    const int note = 127 - yIn;
    jassert(note >= 0 && note <= 127);

    /*
     * Set up lambdas. Essentialy each note component (child) sends messages back
     * to parent (this) through a series of lambda callbacks
     */

    NoteComponent *newNote = new NoteComponent(styleSheet);
    newNote->onNoteSelect = [this](NoteComponent *n, const juce::MouseEvent &e) {
        this->noteCompSelected(n, e);
    };
    newNote->onPositionMoved = [this](NoteComponent *n) {
        this->noteCompPositionMoved(n);
    };
    newNote->onLengthChange = [this](NoteComponent *n, int diff) {
        this->noteCompLengthChanged(n, diff);
    };
    newNote->onDragging = [this](NoteComponent *n, const juce::MouseEvent &e) {
        this->noteCompDragging(n, e);
    };
    addAndMakeVisible(newNote);

    const int defaultVelocity = 100;

    NoteModel nModel((u8) note, defaultVelocity, (st_int) xPos, lastNoteLength, {});
    nModel.quantiseModel(currentQValue, true, true);
    nModel.sendChange = sendChange;
    nModel.trigger();
    newNote->setValues(nModel);

    noteComps.push_back(newNote);

    resized();
    repaint();
    sendEdit();
}

bool NoteGridComponent::keyPressed(const juce::KeyPress &key, Component *originatingComponent) {
    // #ifndef LIB_VERSION
    //     LOG_KEY_PRESS(key.getKeyCode(), 1, key.getModifiers().getRawFlags());
    // #endif

    if (styleSheet.disableEditing) {
        return true;
    }
    if (key == juce::KeyPress::backspaceKey) {
        //
        deleteAllSelected();
        sendEdit();
        return true;
    } else if (key == juce::KeyPress::upKey || key == juce::KeyPress::downKey) {
        bool didMove = false;
        for (auto nComp: noteComps) {
            if (nComp->getState() == NoteComponent::eSelected) {
                NoteModel nModel = nComp->getModel();

                (key == juce::KeyPress::upKey)
                    ? nModel.setNote(nModel.getNote() + 1)
                    : nModel.setNote(nModel.getNote() - 1);

                nModel.sendChange = sendChange;
                nComp->setValues(nModel);
                didMove = true;
            }
        }
        if (didMove) {
            sendEdit();
            resized();
            return true;
        }
    } else if (key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey) {
        bool didMove = false;
        const int nudgeAmount = currentQValue;
        for (auto noteComponent: noteComps) {
            if (noteComponent->getState() == NoteComponent::eSelected) {
                NoteModel noteModel = noteComponent->getModel();

                (key == juce::KeyPress::rightKey)
                    ? noteModel.setStartTime(noteModel.getStartTime() + nudgeAmount)
                    : noteModel.setStartTime(noteModel.getStartTime() - nudgeAmount);

                noteModel.sendChange = sendChange;
                noteComponent->setValues(noteModel);
                didMove = true;
            }
        }
        if (didMove) {
            sendEdit();
            resized();
            return true;
        }
    }
    return false;
}

void NoteGridComponent::deleteAllSelected() {
    std::vector<NoteComponent *> itemsToKeep;
    for (int i = 0; i < noteComps.size(); i++) {
        if (noteComps[i]->getState() == NoteComponent::eSelected) {
            removeChildComponent(noteComps[i]);
            delete noteComps[i];
        } else {
            itemsToKeep.push_back(noteComps[i]);
        }
    }
    noteComps = itemsToKeep;
}

PRESequence NoteGridComponent::getSequence() {
    int leftToSort = (int) noteComps.size();

    std::vector<NoteComponent *> componentsCopy = noteComps;

    /*
     inline lambda function to find the lowest startTime
     */
    auto findLowest = [&]() -> int {
        int lowestIndex = 0;
        for (int i = 0; i < componentsCopy.size(); i++) {
            if (componentsCopy[i]->getModel().getStartTime() < componentsCopy[lowestIndex]->getModel().getStartTime()) {
                lowestIndex = i;
            }
        }
        return lowestIndex;
    };


    PRESequence sequence;
    while (leftToSort) {
        const int index = findLowest();
        auto m = componentsCopy[index]->getModel();
        m.flags.state = componentsCopy[index]->getState();
        sequence.events.push_back(m);
        //        seq.events[seq.events.size()-1]->flags =1  //we also want the selected flags..

        componentsCopy[index] = nullptr;
        componentsCopy.erase(componentsCopy.begin() + index);
        leftToSort--;
    }
    sequence.print();
    return sequence;
}

void NoteGridComponent::loadSequence(PRESequence sequence) {
    for (int i = 0; i < noteComps.size(); i++) {
        removeChildComponent(noteComps[i]);
        delete noteComps[i];
    }
    noteComps.clear();

    noteComps.reserve(sequence.events.size());

    for (auto event: sequence.events) {
        NoteComponent *newNote = new NoteComponent(styleSheet);
        newNote->onNoteSelect = [this](NoteComponent *n, const juce::MouseEvent &e) {
            this->noteCompSelected(n, e);
        };
        newNote->onPositionMoved = [this](NoteComponent *n) {
            this->noteCompPositionMoved(n);
        };
        newNote->onLengthChange = [this](NoteComponent *n, int diff) {
            this->noteCompLengthChanged(n, diff);
        };
        newNote->onDragging = [this](NoteComponent *n, const juce::MouseEvent &e) {
            this->noteCompDragging(n, e);
        };
        addAndMakeVisible(newNote);
        NoteModel nModel(event);
        nModel.sendChange = sendChange;
        //        nModel.quantiseModel(PRE::defaultResolution / 8, true, true);
        newNote->setValues(nModel);

        noteComps.push_back(newNote);
    }
    resized();
    repaint();
}

std::vector<NoteModel *> NoteGridComponent::getSelectedModels() {
    std::vector<NoteModel *> noteModels;
    for (auto comp: noteComps) {
        if (comp->getState()) {
            noteModels.push_back(comp->getModelPtr());
        }
    }
    return noteModels;
}

void NoteGridComponent::sendEdit() {
    if (this->onEdit != nullptr) {
        this->onEdit();
    }
}

float NoteGridComponent::getNoteCompHeight() const {
    return noteCompHeight;
}

float NoteGridComponent::getPixelsPerBar() const {
    return pixelsPerBar;
}