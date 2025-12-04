// JUNIE
#include "NoteGridComponent.h"
#include "AppEngine.h"
namespace t = tracktion;
namespace te = tracktion::engine;

//==============================================================================
// Construction / Destruction
//==============================================================================

NoteGridComponent::NoteGridComponent (GridStyleSheet& sheet, AppEngine& engine, te::MidiClip* clip)
  : styleSheet(sheet), appEngine(engine), clipModel(clip)
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
    auto* currentClip = clip;
    if (currentClip == nullptr)
    {
        return;
    }

    // Detect if this clip belongs to a drum track (Written by Claude Code)
    // Used for highlighting drum sampler note range (MIDI 36-51)
    if (currentClip)
    {
        if (auto* clipTrack = dynamic_cast<te::AudioTrack*> (currentClip->getTrack()))
        {
            auto& edit = appEngine.getEdit();
            auto tracks = te::getAudioTracks (edit);
            for (int i = 0; i < tracks.size(); ++i)
            {
                if (tracks[i] == clipTrack)
                {
                    isDrumTrack = appEngine.isDrumTrack (i);
                    break;
                }
            }
        }
    }

    // Add all existing notes from clip
    for (te::MidiNote* note : currentClip->getSequence().getNotes())
        addNewNoteComponent (note);
}

NoteGridComponent::~NoteGridComponent()
{
    // Detach all note components; unique_ptr will clean up
    for (auto& nc : noteComps)
        if (auto* raw = nc.get())
            removeChildComponent (raw);
    noteComps.clear();
}

//==============================================================================
// Component Overrides
//==============================================================================

void NoteGridComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::darkgrey);

    // Draw the background first
    float line = 0;

    // For drum tracks, only draw 16 rows (MIDI 36-51)
    // For instrument tracks, draw all 128 rows (MIDI 0-127)
    const int startNote = isDrumTrack ? 51 : 127;
    const int endNote = isDrumTrack ? 36 : 0;

    for (int i = startNote; i >= endNote; i--)
    {
        const int pitch = i % 12;

        // Determine base color for this note row (Written by Claude Code)
        juce::Colour baseColor = blackPitches.contains (pitch)
            ? juce::Colours::darkgrey.withAlpha (0.5f)
            : juce::Colours::lightgrey.darker().withAlpha (0.5f);

        g.setColour (baseColor);
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
    for (auto& uptr : noteComps)
    {
        auto* component = uptr.get();
        if (component == nullptr)
            continue;

        if (component->getModel() == nullptr)
            continue;

        if (component->coordinatesDiffer)
            noteCompPositionMoved (component, false);

        // Convert model-side information to component coordinates
        const float xPos = beatsToX (static_cast<float> (component->getModel()->getStartBeat().inBeats()));
        const float yPos = pitchToY (static_cast<float> (component->getModel()->getNoteNumber()));
        const float len = beatsToX (static_cast<float> (component->getModel()->getLengthBeats().inBeats()));

        component->setBounds (xPos, yPos, len, noteCompHeight);
    }
}

//==============================================================================
// Setup and Configuration
//==============================================================================

void NoteGridComponent::setupGrid (float pixelsPerBar, float compHeight, const int bars)
{
    this->pixelsPerBar = pixelsPerBar;
    noteCompHeight = compHeight;
    // For drum tracks, only 16 rows (MIDI 36-51); for instruments, all 128 rows
    const int numRows = isDrumTrack ? 16 : 128;
    setSize (pixelsPerBar * bars, compHeight * numRows);
}

void NoteGridComponent::setQuantisation (float newVal)
{
    currentQValue = newVal;
}

//==============================================================================
// NoteComponent Event Handlers
//==============================================================================

void NoteGridComponent::noteCompSelected (NoteComponent* noteComponent, const juce::MouseEvent& e)
{
    const bool additive = e.mods.isShiftDown();

    for (auto& up : noteComps)
        if (auto* c = up.get())
            c->isMultiDrag = false;

    if (!additive)
        for (auto& up : noteComps)
            if (auto* c = up.get())
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
    for (auto& up : noteComps)
    {
        auto* n = up.get();
        if (n == nullptr || n == original || n->getState() != NoteComponent::eSelected)
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
    // const float deltaBeats         = newLengthBeats - startBeatsOrig;

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
    // auto* clip = appEngine.getMidiClipFromTrack (trackIndex);
    if (!clip) { DBG("Error: NoteGridComponent has no clip set."); return; }

    auto* um = clip->getUndoManager();

    // preserve note position on length changed
    t::BeatPosition beatStart = n->getModel()->getBeatPosition();
    float beatLength = xToBeats (newWidth);
    beatLength = std::round (beatLength / currentQValue) * currentQValue; // snap x
    t::BeatDuration newDur = t::BeatDuration::fromBeats (beatLength);
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
        for (auto& up : noteComps)
        {
            auto* n = up.get();
            if (n != nullptr && n != comp && n->getState() == NoteComponent::eSelected)
                noteCompPositionMoved (n, false);
        }
        firstDrag = false;
    }

    const int note = juce::jlimit(0, 127, yToPitch((float) comp->getY()));

    float beatStart = xToBeats((float) comp->getX());
    beatStart = std::round(beatStart / currentQValue) * currentQValue; // snap X
    beatStart = std::max(0.0f, beatStart);

    // preserve existing length on move
    te::MidiNote* nm = comp->getModel();
    const float beatLength = (float) nm->getLengthBeats().inBeats();

    if (!clip) { DBG("Error: NoteGridComponent has no clip set."); return; }
    auto* um = clip->getUndoManager();

    nm->setNoteNumber(note, um);
    nm->setStartAndLength(t::BeatPosition::fromBeats(beatStart),
                          t::BeatDuration::fromBeats(beatLength),
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

//==============================================================================
// Mouse Event Handling
//==============================================================================

void NoteGridComponent::mouseDown (const juce::MouseEvent&)
{
    for (auto& up : noteComps)
        if (auto* component = up.get())
            component->setState (NoteComponent::eNone);
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
        for (auto& up : noteComps)
        {
            if (auto* component = up.get())
            {
                if (component->getBounds().intersects (selectorBox.getBounds()))
                    component->setState (NoteComponent::eState::eSelected);
                else
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

    auto* currentClip = clip;
    if (!currentClip) { DBG("Error: NoteGridComponent has no clip set."); return; }

    auto& seq = currentClip->getSequence();
    auto* um  = currentClip->getUndoManager();

    auto* newModel = seq.addNote(
        pitch,
        t::BeatPosition::fromBeats(beatStartQ),   // << use the quantised start
        t::BeatDuration::fromBeats(beatLength),
        100,
        0,
        um);

    auto* newNote = addNewNoteComponent(newModel);

    for (auto& up : noteComps)
        if (auto* c = up.get())
            if (c != newNote) c->setState (NoteComponent::eNone);

    newNote->setState (NoteComponent::eSelected);
    newNote->toFront (true);

    resized();
    repaint();
    sendEdit();
}

//==============================================================================
// Keyboard Event Handling
//==============================================================================

bool NoteGridComponent::keyPressed (const juce::KeyPress& key, Component*)
{
    // #ifndef LIB_VERSION
    //     LOG_KEY_PRESS(key.getKeyCode(), 1, key.getModifiers().getRawFlags());
    // #endif

    auto* um = clip ? clip->getUndoManager() : nullptr;
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
        for (auto& up : noteComps)
        {
            auto* nComp = up.get();
            if (nComp == nullptr)
                continue;
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
        for (auto& up : noteComps)
        {
            auto* noteComponent = up.get();
            if (noteComponent == nullptr)
                continue;
            if (noteComponent->getState() == NoteComponent::eSelected)
            {
                te::MidiNote* noteModel = noteComponent->getModel();

                // Moving MIDI note on timeline right or left (up and down)
                (key == juce::KeyPress::rightKey)
                    ? noteModel->setStartAndLength (
                        t::BeatPosition::fromBeats (noteModel->getStartBeat().inBeats() + nudgeAmount),
                        noteModel->getLengthBeats(),
                        um)
                    : noteModel->setStartAndLength (
                        t::BeatPosition::fromBeats (noteModel->getStartBeat().inBeats() - nudgeAmount),
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

//==============================================================================
// Note Management
//==============================================================================

void NoteGridComponent::deleteAllSelected()
{
    if (clip == nullptr)
    {
        DBG ("Error: NoteGridComponent has no clip set.");
        return;
    }
    auto& seq = clip->getSequence();
    auto* um = clip->getUndoManager();

    for (auto it = noteComps.begin(); it != noteComps.end(); )
    {
        NoteComponent* noteComp = it->get();
        if (noteComp != nullptr && noteComp->getState() == NoteComponent::eSelected)
        {
            if (auto* model = noteComp->getModel())
                seq.removeNote (*model, um);
            removeChildComponent (noteComp);
            it = noteComps.erase(it); // unique_ptr deletes the component
        }
        else
        {
            ++it;
        }
    }
}

//==============================================================================
// Data Access
//==============================================================================

// TODO: do we need this function?
te::MidiList& NoteGridComponent::getSequence()
{
    if (clip == nullptr)
    {
        DBG ("Error: NoteGridComponent has no clip set.");
        static te::MidiList dummy; // fallback to avoid UB; operations should guard against this
        return dummy;
    }
    return clip->getSequence();
}

void NoteGridComponent::setClip (te::MidiClip* newClip)
{
    // Detach existing note components; unique_ptr will clean up on clear
    for (auto& up : noteComps)
        if (auto* nc = up.get())
            removeChildComponent (nc);
    noteComps.clear();

    clip = newClip;

    // Update drum track detection when clip changes (Written by Claude Code)
    isDrumTrack = false;  // Reset to false
    if (clip != nullptr)
    {
        if (auto* clipTrack = dynamic_cast<te::AudioTrack*> (clip->getTrack()))
        {
            auto& edit = appEngine.getEdit();
            auto tracks = te::getAudioTracks (edit);
            for (int i = 0; i < tracks.size(); ++i)
            {
                if (tracks[i] == clipTrack)
                {
                    isDrumTrack = appEngine.isDrumTrack (i);
                    break;
                }
            }
        }

        for (te::MidiNote* note : clip->getSequence().getNotes())
            addNewNoteComponent (note);
    }

    resized();
    repaint();
}

juce::Array<te::MidiNote*> NoteGridComponent::getSelectedModels()
{
    juce::Array<te::MidiNote*> noteModels;
    for (auto& up : noteComps)
    {
        if (auto* comp = up.get())
        {
            if (comp->getState() == NoteComponent::eSelected)
                noteModels.add (comp->getModel());
        }
    }
    return noteModels;
}

//==============================================================================
// Internal Methods
//==============================================================================

void NoteGridComponent::sendEdit()
{
    if (this->onEdit != nullptr)
    {
        this->onEdit();
    }
}

NoteComponent* NoteGridComponent::addNewNoteComponent (te::MidiNote* model)
{
    // Create and configure note component.
    auto newNote = std::make_unique<NoteComponent> (styleSheet);

    // Set up lambdas. Essentially each note component (child) sends messages
    // back to parent (this) through a series of lambda callbacks.
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

    // Add to UI and store ownership
    addAndMakeVisible (newNote.get());
    auto* raw = newNote.get();
    noteComps.push_back (std::move(newNote));
    return raw;
}

//==============================================================================
// Coordinate Conversion Utilities
//==============================================================================

float NoteGridComponent::beatsToX (float beats)
{
    const float floatTicks = static_cast<float> (ticksPerTimeSignature);
    return beats * PRE::defaultResolution / floatTicks * pixelsPerBar;
}

float NoteGridComponent::pitchToY (float pitch)
{
    // For drum tracks, offset pitch by 36 (C1) since we only show MIDI 36-51
    const float adjustedPitch = isDrumTrack ? (pitch - 36.0f) : pitch;
    const float gridHeight = static_cast<float> (getHeight());
    return gridHeight - adjustedPitch * noteCompHeight - noteCompHeight;
}

float NoteGridComponent::xToBeats (float x)
{
    const float floatTicks = static_cast<float> (ticksPerTimeSignature);
    return x / PRE::defaultResolution * floatTicks / pixelsPerBar;
}

int NoteGridComponent::yToPitch (float y)
{
    const int row = (int) std::floor(y / noteCompHeight);
    // For drum tracks, we only show 16 rows (MIDI 36-51), so offset the result
    if (isDrumTrack)
    {
        const int pitch = 51 - row;  // Top row is 51 (D#2), bottom row is 36 (C1)
        return juce::jlimit(36, 51, pitch);
    }
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
