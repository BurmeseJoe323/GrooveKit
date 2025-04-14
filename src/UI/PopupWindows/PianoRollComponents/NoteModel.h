//
// Created by Joseph Rockwell on 4/13/25.
//

#ifndef NOTEMODEL_H
#define NOTEMODEL_H

#include <juce_gui_basics/juce_gui_basics.h>
#include "PConstants.h"

class NoteModel {
public:
    struct Flags {
        Flags() {
            state = 0;
            isGenerative = 0;
        }

        unsigned int state: 2;
        unsigned int isGenerative: 2;
    };

    NoteModel();
    NoteModel(u8 n, u8 v, st_int st, st_int nl, Flags flags);
    NoteModel(const NoteModel& other);

    void quantiseModel (int qValue, bool qStartTime, bool qNoteLegnth);

    bool compare (const NoteModel & other, bool compareUIDs = true);

    // Getters and setters
    // Setters will trigger notes as a side effect
    void setNote(u8 note);
    void setVelocity(u8 velocity);
    void setStartTime(st_int time);
    void setNoteLength(st_int length);

    u8 getNote() {return note;};
    u8 getVelocity() {return velocity;};
    st_int getStartTime() {return startTime;}
    st_int getNoteLength() {return noteLength;}

    Flags flags;

    std::function<void(int note, int velocity)> sendChange;
    void trigger();
    void trigger(const u8 note, const u8 velocity);

// #ifndef LIB_VERSION
//     int64_t        uniqueId;
// #endif

private:
    u8 note;
    u8 velocity;
    st_int startTime;
    st_int noteLength;
};

// Sequence for piano roll editor
class PRESequence {
public:
    std::vector<NoteModel> events;
    int tsLow;
    int tsHight;

    int lowNote;
    int highNote;

    // Debugging method
    void print();
};



#endif //NOTEMODEL_H
