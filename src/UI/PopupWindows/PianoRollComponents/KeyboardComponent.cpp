//
// Created by Joseph Rockwell on 4/8/25.
//

#include "KeyboardComponent.h"

KeyboardComponent::KeyboardComponent() {
    blackPitches = {1, 3, 6, 8, 10};
    setSize(50, 15);
}

void KeyboardComponent::paint (juce::Graphics & g) {
    const float noteCompHeight = getHeight() / 128.0;
}
