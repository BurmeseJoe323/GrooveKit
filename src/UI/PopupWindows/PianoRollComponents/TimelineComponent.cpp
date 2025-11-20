//
// Created by Joseph Rockwell on 4/10/25.
//

#include "TimelineComponent.h"

//==============================================================================
// Lifecycle
//==============================================================================

TimelineComponent::TimelineComponent()
{
    barsToDraw = 4;
    pixelsPerBar = 500;
    setBounds (getParentWidth() * 0.15, 0, getParentWidth() * 0.85, getParentHeight() * 0.15);
}

//==============================================================================
// Configuration
//==============================================================================

void TimelineComponent::setup (const int barsToDraw, const int pixelsPerBar)
{
    // Throws an assert false if the passed in values are invalid
    // TODO: this should probably do something else, maybe have default values?
    if (pixelsPerBar <= 0 || barsToDraw <= 0)
    {
        jassertfalse;
        return;
    }

    this->barsToDraw = barsToDraw;
    this->pixelsPerBar = pixelsPerBar;
    setSize (pixelsPerBar * barsToDraw, getHeight()); //height is set externally.
}

//==============================================================================
// Component Overrides
//==============================================================================

void TimelineComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    // NOTE: assume 4/4
    const int marks = barsToDraw * 4;
    const float increment = getWidth() / (float) (marks);
    float yPos = 0;

    g.setColour (juce::Colours::white);

    for (int i = 0; i < marks; i++)
    {
        if (i % 4 == 0)
        {
            const juce::String txt (i / 4 + 1);
            g.drawText (txt, yPos + 5, 3, 30, 20, juce::Justification::left);
            g.drawLine (yPos, 0, yPos, getHeight());
        }
        else if (i % 2 == 0)
        {
            g.drawLine (yPos, getHeight() * 0.66, yPos, getHeight());
        }
        else
        {
            g.drawLine (yPos, getHeight() * 0.33, yPos, getHeight());
        }

        yPos += increment;
    }
}

void TimelineComponent::resized()
{
}
