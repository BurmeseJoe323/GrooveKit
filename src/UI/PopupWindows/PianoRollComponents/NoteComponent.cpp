//
// Created by Joseph Rockwell on 4/13/25.
//

#include "NoteComponent.h"

//==============================================================================
// Lifecycle
//==============================================================================

NoteComponent::NoteComponent (GridStyleSheet styleSheet)
    : styleSheet (styleSheet),
      edgeResizer (this,
          nullptr,
          juce::ResizableEdgeComponent::Edge::rightEdge)
{
    mouseOver = useCustomColour = velocityEnabled = resizeEnabled = false;
    addAndMakeVisible (edgeResizer);
    setMouseCursor (normal);
    startWidth = startX = startY = -1;
    coordinatesDiffer = false;
    isMultiDrag = false;
    state = eNone;
    startVelocity = 0;

    edgeResizer.addMouseListener (this, false);
    // setInterceptsMouseClicks (true, false);

    setCustomColour (juce::Colours::green);
}

//==============================================================================
// Component Overrides
//==============================================================================

void NoteComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    // Set color
    juce::Colour colourToUse = juce::Colour (252, 97, 92);
    // if (useCustomColour) {
    //     colourToUse = customColour;
    // } else {
    //     colourToUse = juce::Colour(252, 97, 92);
    // }

    if (state == eSelected || mouseOver)
    {
        colourToUse = colourToUse.brighter (0.8);
    }
    g.setColour (colourToUse);

    // Draw middle box
    g.fillRect (1, 1, getWidth() - 2, getHeight() - 2);

    // if (styleSheet.getDrawVelocity() && getWidth() > 10) {
    //     g.setColour(colourToUse.brighter());
    //     const int lineMax = getWidth() - 5;
    //     g.drawLine(5, getHeight() * 0.5 - 2, lineMax * (model.getVelocity() / 127.0), getHeight() * 0.5 - 2, 4);
    // }

    // juce::String toDraw;
    // if (styleSheet.getDrawMIDINoteStr()) {
    //     toDraw += juce::String(PRE::pitches_names[model.getNote() % 12]) + juce::String(model.getNote() / 12) +
    //             juce::String(" ");
    // }
    // if (styleSheet.getDrawMIDINum()) {
    //     toDraw += juce::String(model.getNote());
    // }

    // g.setColour(juce::Colours::white);
    // g.drawText(juce::String(toDraw), 3, 3, getWidth() - 6, getHeight() - 6, juce::Justification::centred);
}

void NoteComponent::resized ()
{
    edgeResizer.setBounds (getWidth() - 10, 0, 10, getHeight());
}

//==============================================================================
// Appearance Configuration
//==============================================================================

void NoteComponent::setCustomColour (juce::Colour c)
{
    customColour = c;
    useCustomColour = true;
}

//==============================================================================
// Model Binding
//==============================================================================

void NoteComponent::setModel (te::MidiNote* model)
{
    this->model = model;
    repaint();
}

te::MidiNote* NoteComponent::getModel ()
{
    return model;
}

//==============================================================================
// Selection State Management
//==============================================================================

void NoteComponent::setState (eState s)
{
    state = s;
    repaint();
}

NoteComponent::eState NoteComponent::getState ()
{
    return state;
}

//==============================================================================
// Mouse Event Handlers
//==============================================================================

void NoteComponent::mouseEnter (const juce::MouseEvent&)
{
    mouseOver = true;
    repaint();
}

void NoteComponent::mouseExit (const juce::MouseEvent&)
{
    mouseOver = false;
    setMouseCursor (juce::MouseCursor::NormalCursor);
    repaint();
}

void NoteComponent::mouseDown (const juce::MouseEvent& e)
{
    startX = getX();
    startY = getY();

    const int handleW = juce::jmin (10, getWidth() / 2);

    if (e.mods.isShiftDown())
    {
        velocityEnabled = true;
        startVelocity = model->getVelocity();
    }
    else if (e.originalComponent == &edgeResizer)
    {
        resizeEnabled = true;
        startWidth = getWidth();
    }
    else
    {
        setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    }
}

void NoteComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (resizeEnabled)
    {
        // TODO: this is only useful if we want to change the lengths of multiple selected notes at once
        // Decide if we want this functionality or not
        // if (onEdgeDragging)
        //     onEdgeDragging(this, e);
        return;
    }

    if (velocityEnabled)
    {
        const int velocityDiff = (int) std::round (e.getDistanceFromDragStartY() * -0.5);
        int newVelocity = juce::jlimit (1, 127, startVelocity + velocityDiff);
        if (model)
            model->setVelocity (newVelocity, nullptr);
        repaint();
        return;
    }

    setMouseCursor (juce::MouseCursor::DraggingHandCursor);
    if (onDragging)
        onDragging (this, e);
}


void NoteComponent::mouseUp (const juce::MouseEvent& e)
{
    if (e.originalComponent == &edgeResizer)
    {
        if (onLengthChange)
        {
           onLengthChange(this);
        }
        setState (eState::eSelected);
        return;
    }
    if (onPositionMoved)
        onPositionMoved (this);
    if (onNoteSelect)
        onNoteSelect (this, e);
    startWidth = startX = startY = -1;

    mouseOver = false;
    resizeEnabled = false;
    velocityEnabled = false;
    repaint();
    isMultiDrag = false;
}

void NoteComponent::mouseMove (const juce::MouseEvent& e)
{
    const int handleW = juce::jmin (10, getWidth() / 2);
    if (handleW > 0 && e.getMouseDownX() >= getWidth() - handleW)
    {
        setMouseCursor (juce::MouseCursor::RightEdgeResizeCursor);
    }
    else
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
    }
}