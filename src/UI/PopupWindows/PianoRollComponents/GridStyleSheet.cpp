//
// Created by Joseph Rockwell on 4/11/25.
//

#include "GridStyleSheet.h"

GridStyleSheet::GridStyleSheet() {
    drawMIDINum = false;
    drawMIDINoteStr = false;
    drawVelocity = false;
    disableEditing = false;
}

bool GridStyleSheet::getDrawMIDINum() {
    return drawMIDINum;
}

bool GridStyleSheet::getDrawMIDINoteStr() {
    return drawMIDINoteStr;
}

bool GridStyleSheet::getDrawVelocity() {
    return drawVelocity;
}
