//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef PCONSTANTS_H
#define PCONSTANTS_H

#ifndef u8
typedef unsigned char u8;
#endif
#ifndef st_int
typedef unsigned int st_int;
#endif

namespace PRE {
    // Piano Roll Editor namespace
    /*
     Although this could easily be updated 480 works nicely for MIDI timing and is common in MIDI file formats.
     480/16 = 30 hemidemisemiquaver 1/64
     */
    static const int defaultResolution = 480; // per quarter note

    static const char *pitches_names[] = {
        "C",
        "C#",
        "D",
        "D#",
        "E",
        "F",
        "F#",
        "G",
        "G#",
        "A",
        "A#",
        "B",
    };
}
#endif //PCONSTANTS_H
