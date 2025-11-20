//
// Created by Joseph Rockwell on 4/11/25.
//

#include "GridStyleSheet.h"

//==============================================================================
// Lifecycle
//==============================================================================

GridStyleSheet::GridStyleSheet() {
    drawMIDINum = false;
    drawMIDINoteStr = false;
    drawVelocity = false;
}

//==============================================================================
// Display Option Accessors
//==============================================================================

bool GridStyleSheet::getDrawMIDINum() {
    return drawMIDINum;
}

bool GridStyleSheet::getDrawMIDINoteStr() {
    return drawMIDINoteStr;
}

bool GridStyleSheet::getDrawVelocity() {
    return drawVelocity;
}
