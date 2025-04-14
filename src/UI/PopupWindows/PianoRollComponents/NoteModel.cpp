//
// Created by Joseph Rockwell on 4/13/25.
//

#include "NoteModel.h"

NoteModel::NoteModel() {
    note = 0;
    velocity = 0;
    startTime = 0;
    noteLength = 0;
    // #ifndef LIB_VERSION
    //     uniqueId = StaticCounter::count();
    // #endif
}

NoteModel::NoteModel(u8 n, u8 v, st_int st, st_int nl, Flags f) {
    note = n;
    velocity = v;
    startTime = st;
    noteLength = nl;
    flags = f;
// #ifndef LIB_VERSION
//     uniqueId = StaticCounter::count();
// #endif
}

NoteModel::NoteModel(const NoteModel &other) {
    note = other.note;
    velocity = other.velocity;
    startTime = other.startTime;
    noteLength = other.noteLength;
    flags = other.flags;
// #ifndef LIB_VERSION
//     uniqueId = other.uniqueId;
// #endif
    sendChange = other.sendChange;
}


/*
 Quantising to say 1/8 notes would require an input value of 240 (or 0.5 * the default resolution)
 */
void NoteModel::quantiseModel(int qValue, bool qStartTime, bool qNoteLegnth) {
    auto quantiseValue = [&](int qDiv, int valueToQuantise) -> int {
        const int simpleQLow = (valueToQuantise / qDiv) * qDiv; //use lossey int division
        const int simpleQHigh = ((valueToQuantise / qDiv) + 1) * qDiv;

        if (valueToQuantise - simpleQLow < simpleQHigh - valueToQuantise) {
            //lower boundry
            return simpleQLow;
        }
        return simpleQHigh;
    };

    if (qStartTime) {
        startTime = quantiseValue(qValue, startTime);
    }
    if (qNoteLegnth) {
        noteLength = quantiseValue(qValue, noteLength);
        if (noteLength == 0) {
            noteLength = qValue; //
        }
    }
}

bool NoteModel::compare(const NoteModel &other, bool compareUIDs) {
#define RETURN_IF(A) if(A){return false;}

    RETURN_IF(note != other.note)
    RETURN_IF(velocity != other.velocity)
    RETURN_IF(startTime != other.startTime)
    RETURN_IF(noteLength != other.noteLength)
    RETURN_IF(flags.state != other.flags.state)
    RETURN_IF(flags.isGenerative != other.flags.isGenerative)

// #ifndef LIB_VERSION
//     if (compareUIDs) {
//         RETURN_IF(uniqueId != other.uniqueId)
//     }
// #endif
    return true;
}


void NoteModel::setNote(u8 note) {
    this->note = note;
    trigger();
// #ifndef LIB_VERSION
//     LOG_NOTE_EDITED_PR(note, velocity, startTime, noteLength);
// #endif
}

void NoteModel::setVelocity(u8 velocity) {
    this->velocity = velocity;
    trigger();
// #ifndef LIB_VERSION
//     LOG_NOTE_EDITED_PR(note, velocity, startTime, noteLength);
// #endif
}

void NoteModel::setStartTime(st_int time) {
    startTime = time;
}

void NoteModel::setNoteLength(st_int length) {
    noteLength = length;
}

void NoteModel::trigger() {
    if (sendChange != nullptr) {
        sendChange(note, velocity);
    }
}

void NoteModel::trigger(const u8 n, const u8 vel) {
    if (sendChange != nullptr) {
        sendChange(n, vel);
    }
}

void PRESequence::print() {
    for (auto nm: events) {
        std::cout << (int) nm.getNote() << " - " << (int) nm.getVelocity() << " : " << nm.getStartTime() << " - " << nm.
                getNoteLength() << "\n";
    }
}
