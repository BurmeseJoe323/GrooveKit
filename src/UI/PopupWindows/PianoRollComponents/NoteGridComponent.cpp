//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"
#include "AppEngine.h"

#define RETURN_IF_EDITING_DISABLED if(styleSheet.disableEditing) { return; }

// TODO: clean up and refactor this class to work with Tracktion instead of custom components
NoteGridComponent::NoteGridComponent(GridStyleSheet &sheet, std::shared_ptr<AppEngine> engine, int trackIndex) : styleSheet(sheet), appEngine(engine), trackIndex(trackIndex) {

    addChildComponent(&selectorBox);

    // NOTE: key presses don't work in this current implementation. In the more polished piano roll, they
    // should work
    // addKeyListener(this);
    // setWantsKeyboardFocus(true);
    currentQValue = PRE::quantisedDivisionValues[PRE::eQuantisationValue1_32];
    firstDrag = false;
    firstCall = false;
    lastTrigger = -1;
    pixelsPerBar = 0;
    noteCompHeight = 0;

    // TODO: We want the time signature to be set according to the DAW itself
    timeSignature.beatsPerBar = 4;
    timeSignature.beatValue = 4;
    // Set ticks according to time signature's beatValue
    ticksPerTimeSignature = PRE::defaultResolution * timeSignature.beatsPerBar;

    // TODO: refactor to not use NoteComponent
    // Components for each note will likely impact performance. We will probably want to draw directly
    // on the grid instead, and also figure out a way to select notes and drag them
    const te::MidiList &seq = appEngine->getMidiClipFromTrack(trackIndex);
    for (te::MidiNote *note : seq.getNotes()) {
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
        newNote->setModel(note);
        addAndMakeVisible(newNote);
        noteComps.push_back(newNote);
    }
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

    // TODO: Currently assuming 4/4, should be made adjustable in the future
    // Draw bar lines
    const float increment = pixelsPerBar / 16;
    line = 0;
    g.setColour(juce::Colours::black);
    for (int i = 0; line < static_cast<float>(getWidth()); i++) {
        float lineThickness = 1.0;
        // Bar marker
        if (i % 16 == 0) {
            lineThickness = 3.0;
        } else if (i % 4 == 0) {
            // Quarter-note div
            lineThickness = 2.0;
        }
        g.drawLine(line, 0, line, static_cast<float>(getHeight()), lineThickness);

        line += increment;
    }

    // TODO: Draw all notes in this clip directly, instead of using components
    // const te::MidiList &seq = appEngine->getMidiClipFromTrack(trackIndex);
    // for (te::MidiNote* note : seq.getNotes()) {
    //     // NOTE: calculations for the time-position of a note require using PRE::defaultResolution right now, which
    //     // is hard-coded to 480. We may want to change this in the future
    //     const float xPos = beatsToX(static_cast<float>(note->getStartBeat().inBeats()));
    //     const float yPos = pitchToY(static_cast<float>(note->getNoteNumber()));
    //     const float len = beatsToX(static_cast<float>(note->getLengthBeats().inBeats()));
    //
    //     // Set color
    //     juce::Colour colourToUse = juce::Colour(252, 97, 92);
    //     // if (useCustomColour) {
    //     //     colourToUse = customColour;
    //     // } else {
    //     //     colourToUse = juce::Colour(252, 97, 92);
    //     // }
    //     //
    //     // if (state == eSelected || mouseOver) {
    //     //     colourToUse = colourToUse.brighter(0.8);
    //     // }
    //     g.setColour(colourToUse);
    //
    //     // Draw middle box
    //     g.fillRect(xPos, yPos, len, noteCompHeight);
    // }
}

void NoteGridComponent::resized() {
    for (auto component: noteComps) {
        if (component->coordinatesDiffer) {
            noteCompPositionMoved(component, false);
        }
        // Convert model-side information to component coordinates
        const float xPos = beatsToX(static_cast<float>(component->getModel()->getStartBeat().inBeats()));
        const float yPos = pitchToY(static_cast<float>(component->getModel()->getNoteNumber()));
        const float len = beatsToX(static_cast<float>(component->getModel()->getLengthBeats().inBeats()));

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
    int note = yToPitch(comp->getY());
    if (note > 127) {
        note = 127;
    } else if (note < 0) {
        note = 0;
    }

    float beatStart = xToBeats(static_cast<float>(comp->getX()));
    if (beatStart < 0) {
        beatStart = 0;
    }

    const float beatLength = xToBeats(static_cast<float>(comp->getWidth()));
    te::MidiNote *nm = comp->getModel();
    nm->setNoteNumber(note, nullptr);
    nm->setStartAndLength(te::BeatPosition::fromBeats(beatStart),
                          te::BeatDuration::fromBeats(beatLength), nullptr);
    // TODO: figure out how Tracktion quantization works and apply here
    // nm.quantiseModel(currentQValue, true, true);
    // nm.sendChange = sendChange;

    comp->startY = -1;
    comp->startX = -1;
    comp->setModel(nm);
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
}

void NoteGridComponent::setPositions() {
    //unused..
}

void NoteGridComponent::setTimeSignature(unsigned int beatsPerBar, unsigned int beatValue) {
    // Check if the beat value is valid (for our sake, must be between 1 and 16 inclusively, and must be a power of 2)
    if ( beatValue > 16 || beatValue < 1|| (beatValue & beatValue - 1) != 0 ) {
        DBG("Invalid beat value passed");
        return;
    }
    timeSignature.beatsPerBar = beatsPerBar;
    timeSignature.beatValue = beatValue;
}

void NoteGridComponent::mouseDown(const juce::MouseEvent &) {
    RETURN_IF_EDITING_DISABLED

    for (NoteComponent *component: noteComps) {
        component->setState(NoteComponent::eNone);
    }
    sendEdit();
    grabKeyboardFocus();
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

    /*
     * Set up lambdas. Essentially each note component (child) sends messages back
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


    // NoteModel nModel((u8) note, defaultVelocity, (st_int) xPos, lastNoteLength);
    // nModel.quantiseModel(currentQValue, true, true);
    // nModel.sendChange = sendChange;
    // nModel.trigger();

    const float beatStart = xToBeats(static_cast<float>(e.getMouseDownX()));
    const float beatLength = xToBeats(static_cast<float>(newNote->getWidth()));
    int pitch = yToPitch(static_cast<float>(e.getMouseDownY()));
    if (pitch < 0) { pitch = 0; } else if (pitch > 127) { pitch = 127; }

    te::MidiList &seq = appEngine->getMidiClipFromTrack(trackIndex);
    // TODO: add new note to sequence here, then add note to component
    // auto newModel = seq.addNote(pitch, te::BeatPosition::fromBeats(beatStart), te::BeatDuration::fromBeats(beatLength),
    //                             100, 0, nullptr);
    //
    // newNote->setModel(newModel);

    noteComps.push_back(newNote);

    resized();
    repaint();
    sendEdit();
}

// bool NoteGridComponent::keyPressed(const juce::KeyPress &key, Component *originatingComponent) {
//     // #ifndef LIB_VERSION
//     //     LOG_KEY_PRESS(key.getKeyCode(), 1, key.getModifiers().getRawFlags());
//     // #endif
//
//     if (styleSheet.disableEditing) {
//         return true;
//     }
//     if (key == juce::KeyPress::backspaceKey) {
//         //
//         deleteAllSelected();
//         sendEdit();
//         return true;
//     } else if (key == juce::KeyPress::upKey || key == juce::KeyPress::downKey) {
//         bool didMove = false;
//         for (auto nComp: noteComps) {
//             if (nComp->getState() == NoteComponent::eSelected) {
//                 NoteModel nModel = nComp->getModel();
//
//                 (key == juce::KeyPress::upKey)
//                     ? nModel.setNote(nModel.getNote() + 1)
//                     : nModel.setNote(nModel.getNote() - 1);
//
//                 nModel.sendChange = sendChange;
//                 nComp->setValues(nModel);
//                 didMove = true;
//             }
//         }
//         if (didMove) {
//             sendEdit();
//             resized();
//             return true;
//         }
//     } else if (key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey) {
//         bool didMove = false;
//         const int nudgeAmount = currentQValue;
//         for (auto noteComponent: noteComps) {
//             if (noteComponent->getState() == NoteComponent::eSelected) {
//                 NoteModel noteModel = noteComponent->getModel();
//
//                 (key == juce::KeyPress::rightKey)
//                     ? noteModel.setStartTime(noteModel.getStartTime() + nudgeAmount)
//                     : noteModel.setStartTime(noteModel.getStartTime() - nudgeAmount);
//
//                 noteModel.sendChange = sendChange;
//                 noteComponent->setValues(noteModel);
//                 didMove = true;
//             }
//         }
//         if (didMove) {
//             sendEdit();
//             resized();
//             return true;
//         }
//     }
//     return false;
// }

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

const te::MidiList &NoteGridComponent::getSequence() {
    // int leftToSort = (int) noteComps.size();
    //
    // std::vector<NoteComponent *> componentsCopy = noteComps;
    //
    // /*
    //  inline lambda function to find the lowest startTime
    //  */
    // auto findLowest = [&]() -> int {
    //     int lowestIndex = 0;
    //     for (int i = 0; i < componentsCopy.size(); i++) {
    //         if (componentsCopy[i]->getModel().getStartTime() < componentsCopy[lowestIndex]->getModel().getStartTime()) {
    //             lowestIndex = i;
    //         }
    //     }
    //     return lowestIndex;
    // };
    //
    //
    // PRESequence sequence;
    // while (leftToSort) {
    //     const int index = findLowest();
    //     auto m = componentsCopy[index]->getModel();
    //     m.flags.state = componentsCopy[index]->getState();
    //     sequence.events.push_back(m);
    //     //        seq.events[seq.events.size()-1]->flags =1  //we also want the selected flags..
    //
    //     componentsCopy[index] = nullptr;
    //     componentsCopy.erase(componentsCopy.begin() + index);
    //     leftToSort--;
    // }
    // sequence.print();
    return appEngine->getMidiClipFromTrack(trackIndex);
}

// void NoteGridComponent::loadSequence() {
//      const te::MidiList &sequence = appEngine->getMidiClipFromTrack(trackIndex);
//     for (int i = 0; i < noteComps.size(); i++) {
//         removeChildComponent(noteComps[i]);
//         delete noteComps[i];
//     }
//     noteComps.clear();
//
//     noteComps.reserve(sequence.size());
//
//     for (auto event: sequence) {
//         NoteComponent *newNote = new NoteComponent(styleSheet);
//         newNote->onNoteSelect = [this](NoteComponent *n, const juce::MouseEvent &e) {
//             this->noteCompSelected(n, e);
//         };
//         newNote->onPositionMoved = [this](NoteComponent *n) {
//             this->noteCompPositionMoved(n);
//         };
//         newNote->onLengthChange = [this](NoteComponent *n, int diff) {
//             this->noteCompLengthChanged(n, diff);
//         };
//         newNote->onDragging = [this](NoteComponent *n, const juce::MouseEvent &e) {
//             this->noteCompDragging(n, e);
//         };
//         addAndMakeVisible(newNote);
//         NoteModel nModel(event->getNoteNumber(), event->getVelocity(), event->getStartBeat().inTicks(), event->getLengthInTicks(), {});
//         nModel.sendChange = sendChange;
//         //        nModel.quantiseModel(PRE::defaultResolution / 8, true, true);
//         newNote->setValues(nModel);
//
//         noteComps.push_back(newNote);
//     }
//     resized();
//     repaint();
// }

juce::Array<te::MidiNote *> NoteGridComponent::getSelectedModels() {
    juce::Array<te::MidiNote *> noteModels;
    for (auto comp: noteComps) {
        if (comp->getState()) {
            noteModels.add(comp->getModel());
        }
    }
    return noteModels;
}

void NoteGridComponent::sendEdit() {
    if (this->onEdit != nullptr) {
        this->onEdit();
    }
}

float NoteGridComponent::beatsToX(float beats) {
    const float floatTicks = static_cast<float>(ticksPerTimeSignature);
    return beats * PRE::defaultResolution / floatTicks * pixelsPerBar;
}

float NoteGridComponent::pitchToY(float pitch) {
    const float gridHeight = static_cast<float>(getHeight());
    return gridHeight - pitch * noteCompHeight - noteCompHeight;
}

float NoteGridComponent::xToBeats(float x) {
    const float floatTicks = static_cast<float>(ticksPerTimeSignature);
    return x / PRE::defaultResolution * floatTicks / pixelsPerBar;
}

int NoteGridComponent::yToPitch(float y) {
    return 127 - y / noteCompHeight;
}

float NoteGridComponent::getNoteCompHeight() const {
    return noteCompHeight;
}

float NoteGridComponent::getPixelsPerBar() const {
    return pixelsPerBar;
}