//
// Created by Joseph Rockwell on 4/11/25.
//

#ifndef GRIDSTYLESHEET_H
#define GRIDSTYLESHEET_H


// NOTE: this is primarily for development of the piano roll. In the final project, we likely  want
// to use an application-wide style sheet
class GridStyleSheet {
public:
    /*
     * We use a friend class to ensure a one-way only relationship.
     * The control panel can set member variables easily.
     * Saves adding loads of getters/setters
     */
    friend class GridControlPanel;

    GridStyleSheet();

    bool getDrawMIDINum();

    bool getDrawMIDINoteStr();

    bool getDrawVelocity();

private:
    bool drawMIDINum, drawMIDINoteStr, drawVelocity;
};


#endif //GRIDSTYLESHEET_H
