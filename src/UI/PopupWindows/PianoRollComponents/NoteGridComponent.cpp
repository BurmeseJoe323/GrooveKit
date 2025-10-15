//
// Created by Joseph Rockwell on 4/8/25.
//

#include "NoteGridComponent.h"
#include "AppEngine.h"


NoteGridComponent::NoteGridComponent (GridStyleSheet& sheet, AppEngine& engine, int trackIndex) : styleSheet (sheet), appEngine (engine), trackIndex (trackIndex)
{
    addChildComponent (&selectorBox);

    addKeyListener (this);
    setWantsKeyboardFocus (true);
    currentQValue = 1.f; // Assume quantisation to quarter-note beats
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

    // TODO: refactor to not use NoteComponent?
    // Components for each note will likely impact performance. We will probably want to draw directly
    // on the grid instead, and also figure out a way to select notes and drag them
    const auto clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (clip == nullptr)
    {
        return;
    }

    // Add all existing notes from clip
    for (te::MidiNote* note : clip->getSequence().getNotes())
        addNewNoteComponent (note);
}

NoteGridComponent::~NoteGridComponent()
{
    // Destroys all children MIDI note components
    for (int i = 0; i < noteComps.size(); i++)
    {
        removeChildComponent (noteComps[i]);
        delete noteComps[i];
    }
}

void NoteGridComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    // Draw the background first
    float line = 0;

    for (int i = 127; i >= 0; i--)
    {
        const int pitch = i % 12;
        g.setColour (blackPitches.contains (pitch)
                         ? juce::Colours::darkgrey.withAlpha (0.5f)
                         : juce::Colours::lightgrey.darker().withAlpha (0.5f));

        g.fillRect (0, juce::detail::floorAsInt (line), getWidth(), juce::detail::floorAsInt (noteCompHeight));

        line += noteCompHeight;
        g.setColour (juce::Colours::black);
        g.drawLine (0, floor (line), getWidth(), floor (line));
    }

    // TODO: Currently assuming 4/4, should be made adjustable in the future
    // Draw bar lines
    const float increment = pixelsPerBar / 16;
    line = 0;
    g.setColour (juce::Colours::black);
    for (int i = 0; line < static_cast<float> (getWidth()); i++)
    {
        float lineThickness = 1.0;
        // Bar marker
        if (i % 16 == 0)
        {
            lineThickness = 3.0;
        }
        else if (i % 4 == 0)
        {
            // Quarter-note div
            lineThickness = 2.0;
        }
        g.drawLine (line, 0, line, static_cast<float> (getHeight()), lineThickness);

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

void NoteGridComponent::resized()
{
    for (auto component : noteComps)
    {
        if (component->coordinatesDiffer)
        {
            noteCompPositionMoved (component, false);
        }
        // Convert model-side information to component coordinates
        const float xPos = beatsToX (static_cast<float> (component->getModel()->getStartBeat().inBeats()));
        const float yPos = pitchToY (static_cast<float> (component->getModel()->getNoteNumber()));
        const float len = beatsToX (static_cast<float> (component->getModel()->getLengthBeats().inBeats()));

        component->setBounds (xPos, yPos, len, noteCompHeight);
    }
}

void NoteGridComponent::setupGrid (float pixelsPerBar, float compHeight, const int bars)
{
    this->pixelsPerBar = pixelsPerBar;
    noteCompHeight = compHeight;
    setSize (pixelsPerBar * bars, compHeight * 128); //we have 128 slots for notes
}

void NoteGridComponent::setQuantisation (float newVal)
{
    currentQValue = newVal;
}

void NoteGridComponent::noteCompSelected (NoteComponent* noteComponent, const juce::MouseEvent& e)
{
    const bool additive = e.mods.isShiftDown();

    for (auto* c : noteComps)
        c->isMultiDrag = false;

    if (!additive)
        for (auto* c : noteComps)
            if (c != noteComponent)
                c->setState (NoteComponent::eNone);

    noteComponent->setState (NoteComponent::eSelected);
    noteComponent->toFront (true);
    sendEdit();
}


void NoteGridComponent::noteCompDragging (NoteComponent* original, const juce::MouseEvent& e)
{
    const float q = currentQValue;

    // helpers
    auto pxToBeats = [this](int xPx) -> float { return xToBeats((float) xPx); };
    auto beatsToPx = [this](float beats) -> int { return (int) std::round(beatsToX(beats)); };

    auto snapY = [this](int yPx) -> int {
        int pitch = juce::jlimit(0, 127, yToPitch((float) yPx));
        const int ySnapped = (int) std::round(pitchToY((float) pitch));
        const int yMax     = juce::jmax(0, getHeight() - (int) noteCompHeight);
        return juce::jlimit(0, yMax, ySnapped);
    };

    const int dx  = e.getDistanceFromDragStartX();
    const int dy  = e.getDistanceFromDragStartY();
    const int mdx = e.getMouseDownX(); // where inside the note you grabbed it

    // latch anchors
    if (original->startY == -1) { original->startX = original->getX(); original->startY = original->getY(); }

    // --- stable, jitter-free horizontal snap ---
    // beat where the cursor was on mouse-down, relative to the note's left
    const float startBeatsOrig     = pxToBeats(original->startX);
    const float grabOffsetBeats    = pxToBeats(original->startX + mdx) - startBeatsOrig;

    // beat where the cursor is now
    const float cursorBeatsNow     = pxToBeats(original->startX + dx + mdx);

    // proposed new left edge in beats (cursor minus fixed grab offset), then quantise
    float newStartBeats            = cursorBeatsNow - grabOffsetBeats;
    newStartBeats                  = std::round(newStartBeats / q) * q;   // use std::floor for left-align feel

    // convert to pixels
    const int origLeftSnappedPx    = juce::jmax(0, beatsToPx(newStartBeats));
    const int origTopSnappedPx     = snapY(original->startY + dy);

    // move original
    original->setTopLeftPosition(origLeftSnappedPx, origTopSnappedPx);

    // delta to apply to any other selected notes (in beats, not pixels)
    const float deltaBeats         = newStartBeats - startBeatsOrig;

    // move the rest by the exact same beat delta (prevents drift/jitter)
    for (auto* n : noteComps)
    {
        if (n == original || n->getState() != NoteComponent::eSelected)
            continue;

        n->isMultiDrag = true;
        if (n->startY == -1) { n->startX = n->getX(); n->startY = n->getY(); }

        const float nStartBeats = pxToBeats(n->startX);
        const int   newLeftPx   = juce::jmax(0, beatsToPx(nStartBeats + deltaBeats));
        const int   newTopPx    = snapY(n->startY + dy);

        n->setTopLeftPosition(newLeftPx, newTopPx);
    }
}

void NoteGridComponent::noteEdgeDragging (NoteComponent* original, const juce::MouseEvent& e)
{
    const float q = currentQValue;

    // helpers
    auto pxToBeats = [this](int xPx) -> float { return xToBeats((float) xPx); };
    auto beatsToPx = [this](float beats) -> int { return (int) std::round(beatsToX(beats)); };

    const int dx  = e.getDistanceFromDragStartX();

    // TODO: might not need this
    const int mdx = e.getMouseDownX(); // where inside the note you grabbed it

    // latch anchors
    if (original->startWidth == -1) { original->startWidth = original->getWidth(); }

    // --- stable, jitter-free horizontal snap ---
    // beat where the cursor was on mouse-down, relative to the note's left
    const float startBeatsOrig     = pxToBeats(original->startWidth);
    const float grabOffsetBeats    = pxToBeats(original->startWidth + mdx) - startBeatsOrig;

    // beat where the cursor is now
    const float cursorBeatsNow     = pxToBeats(original->startWidth + dx + mdx);

    // proposed new right edge in beats (cursor minus fixed grab offset), then quantise
    float newLengthBeats            = cursorBeatsNow - grabOffsetBeats;
    // newStartBeats                  = std::round(newStartBeats / q) * q;   // use std::floor for left-align feel

    // convert to pixels
    const int origRightEdgePx    = juce::jmax(0, beatsToPx(newLengthBeats));

    // set length of original using setSize
    // original->setTopLeftPosition(origLeftSnappedPx, origTopSnappedPx);
    original->setSize (origRightEdgePx, original->getHeight());

    // delta to apply to any other selected notes (in beats, not pixels)
    const float deltaBeats         = newLengthBeats - startBeatsOrig;

    // resize other selected notes by the exact same beat delta (prevents drift/jitter)
    // for (auto* n : noteComps)
    // {
    //     if (n == original || n->getState() != NoteComponent::eSelected)
    //         continue;
    //
    //     n->isMultiDrag = true;
    //     if (n->startWidth == -1) { n->startWidth = n->getWidth(); }
    //
    //     const float nStartBeats = pxToBeats(n->startX);
    //     const int   newRightEdgePx   = juce::jmax(0, beatsToPx(nStartBeats + deltaBeats));
    //
    //     n->setSize(newRightEdgePx, n->getHeight());
    // }
}

void NoteGridComponent::noteCompLengthChanged (NoteComponent* original)
{
    // TODO: we only want to iterate over each note component IF we are changing multiple note lengths simultaneously
    // There might be a better way to do this (queue changed notes, etc.)

    // for (auto n : noteComps)
    // {
    //     if (n->getState() == NoteComponent::eSelected || n == original)
    //     {
    auto n = original;
    if (n->startWidth == -1)
    {
        n->startWidth = n->getWidth();
        n->coordinatesDiffer = true;
    }

    // TODO: change the minimum value in this jmax call
    // The minimum value we want for a note length is the amount of pixels that corresponds to a 1/32 note
    const int newWidth = juce::jmax (20, n->getWidth());
    n->setSize (newWidth, n->getHeight());
    auto* clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (!clip)
    {
        DBG ("Error: MIDI clip at " << trackIndex << " not found.");
        return;
    }
    auto* um = clip->getUndoManager();

    // preserve note position on length changed
    te::BeatPosition beatStart = n->getModel()->getBeatPosition();
    float beatLength = xToBeats (newWidth);
    beatLength = std::round (beatLength / currentQValue) * currentQValue; // snap x
    te::BeatDuration newDur = te::BeatDuration::fromBeats (beatLength);
    n->getModel()->setStartAndLength (beatStart, newDur, um);
    //     }
    // }
    sendEdit();
    resized();
}

void NoteGridComponent::noteCompPositionMoved (NoteComponent* comp, bool callResize)
{
    // TODO: is this supposed to be a lock?
    if (!firstDrag)
    {
        firstDrag = true;
        for (auto n : noteComps)
            if (n != comp && n->getState() == NoteComponent::eSelected)
                noteCompPositionMoved (n, false);
        firstDrag = false;
    }

    const int note = juce::jlimit(0, 127, yToPitch((float) comp->getY()));

    float beatStart = xToBeats((float) comp->getX());
    beatStart = std::round(beatStart / currentQValue) * currentQValue; // snap X
    beatStart = std::max(0.0f, beatStart);

    // preserve existing length on move
    te::MidiNote* nm = comp->getModel();
    const float beatLength = (float) nm->getLengthBeats().inBeats();

    auto* clip = appEngine.getMidiClipFromTrack(trackIndex);
    if (!clip) { DBG("Error: MIDI clip at " << trackIndex << " not found."); return; }
    auto* um = clip->getUndoManager();

    nm->setNoteNumber(note, um);
    nm->setStartAndLength(te::BeatPosition::fromBeats(beatStart),
                          te::BeatDuration::fromBeats(beatLength),
                          um);

    comp->startY = comp->startX = -1;
    comp->setModel(nm);
    if (callResize) resized();
    sendEdit();
}


void NoteGridComponent::setPositions()
{
    //unused..
}

void NoteGridComponent::setTimeSignature (unsigned int beatsPerBar, unsigned int beatValue)
{
    // Check if the beat value is valid (for our sake, must be between 1 and 16 inclusively, and must be a power of 2)
    if (beatValue > 16 || beatValue < 1 || (beatValue & beatValue - 1) != 0)
    {
        DBG ("Invalid beat value passed");
        return;
    }
    timeSignature.beatsPerBar = beatsPerBar;
    timeSignature.beatValue = beatValue;
}

void NoteGridComponent::mouseDown (const juce::MouseEvent&)
{
    for (NoteComponent* component : noteComps)
    {
        component->setState (NoteComponent::eNone);
    }
    sendEdit();
    grabKeyboardFocus();
}

void NoteGridComponent::mouseDrag (const juce::MouseEvent& e)
{
    if (!selectorBox.isVisible())
    {
        selectorBox.setVisible (true);
        selectorBox.toFront (false);

        selectorBox.setTopLeftPosition (e.getPosition());
        selectorBox.startX = e.getPosition().x;
        selectorBox.startY = e.getPosition().y;
    }
    else
    {
        int xDir = e.getPosition().x - selectorBox.startX;
        int yDir = e.getPosition().y - selectorBox.startY;

        // Work out which way to draw the selection box
        if (xDir < 0 && yDir < 0)
        {
            // Top left
            selectorBox.setTopLeftPosition (e.getPosition().x, e.getPosition().y);
            selectorBox.setSize (selectorBox.startX - e.getPosition().getX(),
                selectorBox.startY - e.getPosition().getY());
        }
        else if (xDir > 0 && yDir < 0)
        {
            // Top right
            selectorBox.setTopLeftPosition (selectorBox.startX, e.getPosition().y);
            selectorBox.setSize (e.getPosition().getX() - selectorBox.startX,
                selectorBox.startY - e.getPosition().getY());
        }
        else if (xDir < 0 && yDir > 0)
        {
            // Bottom left
            selectorBox.setTopLeftPosition (e.getPosition().x, selectorBox.startY);
            selectorBox.setSize (selectorBox.startX - e.getPosition().getX(),
                e.getPosition().getY() - selectorBox.startY);
        }
        else
        {
            // Bottom right
            selectorBox.setSize (e.getPosition().getX() - selectorBox.getX(),
                e.getPosition().getY() - selectorBox.getY());
        }
    }
}

void NoteGridComponent::mouseUp (const juce::MouseEvent&)
{
    if (selectorBox.isVisible())
    {
        for (NoteComponent* component : noteComps)
        {
            if (component->getBounds().intersects (selectorBox.getBounds()))
            {
                component->setState (NoteComponent::eState::eSelected);
            }
            else
            {
                component->setState (NoteComponent::eState::eNone);
            }
        }
        selectorBox.setVisible (false);
        selectorBox.toFront (false);
        selectorBox.setSize (1, 1);
    }

    sendEdit();
}

void NoteGridComponent::mouseDoubleClick (const juce::MouseEvent& e)
{
    const float q = currentQValue;
    const float beatStartRaw = xToBeats((float) e.getMouseDownX());
    const float beatStartQ   = std::floor(beatStartRaw / q) * q;
    const float beatLength   = q;

    int pitch = yToPitch ((float) e.getMouseDownY());
    pitch = juce::jlimit (0, 127, pitch);

    auto clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (!clip) { DBG("Error: MIDI clip at " << trackIndex << " not found."); return; }

    auto& seq = clip->getSequence();
    auto* um  = clip->getUndoManager();

    auto* newModel = seq.addNote(
        pitch,
        te::BeatPosition::fromBeats(beatStartQ),   // << use the quantised start
        te::BeatDuration::fromBeats(beatLength),
        100,
        0,
        um);

    auto newNote = addNewNoteComponent(newModel);

    for (auto* c : noteComps)
        if (c != newNote) c->setState (NoteComponent::eNone);

    newNote->setState (NoteComponent::eSelected);
    newNote->toFront (true);

    noteComps.push_back(newNote);

    resized();
    repaint();
    sendEdit();
}

bool NoteGridComponent::keyPressed (const juce::KeyPress& key, Component*)
{
    // #ifndef LIB_VERSION
    //     LOG_KEY_PRESS(key.getKeyCode(), 1, key.getModifiers().getRawFlags());
    // #endif

    auto clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (clip == nullptr)
    {
        DBG("Error: midi clip at track index " << trackIndex << " not found.");
        return true;
    }

    auto* um = clip->getUndoManager();
    // Delete all selected midi notes
    if (key == juce::KeyPress::backspaceKey)
    {
        deleteAllSelected();
        sendEdit();
        return true;
    }
    else if (key == juce::KeyPress::upKey || key == juce::KeyPress::downKey)
    {
        bool didMove = false;
        for (auto nComp : noteComps)
        {
            if (nComp->getState() == NoteComponent::eSelected)
            {
                te::MidiNote* nModel = nComp->getModel();

                (key == juce::KeyPress::upKey)
                    ? nModel->setNoteNumber (nModel->getNoteNumber() + 1, um)
                    : nModel->setNoteNumber (nModel->getNoteNumber() - 1, um);

                nComp->setModel (nModel);
                didMove = true;
            }
        }
        if (didMove)
        {
            sendEdit(); // TODO : take out later
            resized();
            return true;
        }
    }
    else if (key == juce::KeyPress::leftKey || key == juce::KeyPress::rightKey)
    {
        bool didMove = false;
        const float nudgeAmount = currentQValue;
        for (auto noteComponent : noteComps)
        {
            if (noteComponent->getState() == NoteComponent::eSelected)
            {
                te::MidiNote* noteModel = noteComponent->getModel();

                // Moving MIDI note on timeline right or left (up and down)
                (key == juce::KeyPress::rightKey)
                    ? noteModel->setStartAndLength (
                        te::BeatPosition::fromBeats (noteModel->getStartBeat().inBeats() + nudgeAmount),
                        noteModel->getLengthBeats(),
                        um)
                    : noteModel->setStartAndLength (
                        te::BeatPosition::fromBeats (noteModel->getStartBeat().inBeats() - nudgeAmount),
                        noteModel->getLengthBeats(),
                        um);

                noteComponent->setModel (noteModel);
                didMove = true;
            }
        }
        if (didMove)
        {
            sendEdit();
            resized();
            return true;
        }
    }
    return false;
}

void NoteGridComponent::deleteAllSelected()
{
    std::vector<NoteComponent*> itemsToKeep;
    auto clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (clip == nullptr)
    {
        DBG ("Error: MIDI clip at track " << trackIndex << "not found.");
        return;
    }
    auto& seq = clip->getSequence();
    auto* um = appEngine.getMidiClipFromTrack (trackIndex)->getUndoManager();
    for (int i = 0; i < noteComps.size(); i++)
    {
        if (noteComps[i]->getState() == NoteComponent::eSelected)
        {
            seq.removeNote (*noteComps[i]->getModel(), um);
            removeChildComponent (noteComps[i]);
            delete noteComps[i];
        }
        else
        {
            itemsToKeep.push_back (noteComps[i]);
        }
    }
    noteComps = itemsToKeep;
}

// TODO: do we need this function?
te::MidiList& NoteGridComponent::getSequence()
{
    auto clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (clip == nullptr)
    {
        DBG ("Error: MIDI clip not found at track " << trackIndex);
        throw std::format ("Error: MIDI clip not found at track {}", trackIndex);
    }
    return clip->getSequence();
}

juce::Array<te::MidiNote*> NoteGridComponent::getSelectedModels()
{
    juce::Array<te::MidiNote*> noteModels;
    for (auto comp : noteComps)
    {
        if (comp->getState() == NoteComponent::eSelected)
        {
            noteModels.add (comp->getModel());
        }
    }
    return noteModels;
}

void NoteGridComponent::sendEdit()
{
    if (this->onEdit != nullptr)
    {
        this->onEdit();
    }
}

NoteComponent *NoteGridComponent::addNewNoteComponent (te::MidiNote* model)
{
    /*
     * Set up lambdas. Essentially each note component (child) sends messages back
     * to parent (this) through a series of lambda callbacks
     */
    auto newNote = new NoteComponent (styleSheet);
    newNote->onNoteSelect = [this] (NoteComponent* n, const juce::MouseEvent& e) {
        this->noteCompSelected (n, e);
    };
    newNote->onPositionMoved = [this] (NoteComponent* n) {
        this->noteCompPositionMoved (n);
    };
    newNote->onLengthChange = [this] (NoteComponent* n) {
        this->noteCompLengthChanged (n);
    };
    newNote->onDragging = [this] (NoteComponent* n, const juce::MouseEvent& e) {
        this->noteCompDragging (n, e);
    };
    newNote->onEdgeDragging = [this] (NoteComponent* n, const juce::MouseEvent& e) {
        this->noteEdgeDragging (n, e);
    };
    newNote->setModel (model);
    addAndMakeVisible (newNote);
    noteComps.push_back (newNote);
    return newNote;
}

float NoteGridComponent::beatsToX (float beats)
{
    const float floatTicks = static_cast<float> (ticksPerTimeSignature);
    return beats * PRE::defaultResolution / floatTicks * pixelsPerBar;
}

float NoteGridComponent::pitchToY (float pitch)
{
    const float gridHeight = static_cast<float> (getHeight());
    return gridHeight - pitch * noteCompHeight - noteCompHeight;
}

float NoteGridComponent::xToBeats (float x)
{
    const float floatTicks = static_cast<float> (ticksPerTimeSignature);
    return x / PRE::defaultResolution * floatTicks / pixelsPerBar;
}

int NoteGridComponent::yToPitch (float y)
{
    const int row = (int) std::floor(y / noteCompHeight);
    return juce::jlimit(0, 127, 127 - row);
}

float NoteGridComponent::getNoteCompHeight() const
{
    return noteCompHeight;
}

float NoteGridComponent::getPixelsPerBar() const
{
    return pixelsPerBar;
}