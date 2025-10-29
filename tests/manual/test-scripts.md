# Manual Test Scripts for GrooveKit

This document contains manual test procedures for features that are difficult to automate.

## BPM Control Testing

### Test 1: Valid BPM Input
**Steps:**
1. Launch GrooveKit application
2. Locate the BPM value field in the top bar
3. Click the BPM field to edit
4. Enter "138.5"
5. Press Enter or click away

**Expected Result:**
- Label displays "138.50"
- Playback tempo adjusts to 138.5 BPM

### Test 2: BPM Lower Bound Constraint
**Steps:**
1. Click BPM field
2. Enter "5" (below minimum of 20)
3. Press Enter

**Expected Result:**
- Label automatically updates to "20.00"
- BPM is constrained to minimum value

### Test 3: BPM Upper Bound Constraint
**Steps:**
1. Click BPM field
2. Enter "500" (above maximum of 250)
3. Press Enter

**Expected Result:**
- Label automatically updates to "250.00"
- BPM is constrained to maximum value

### Test 4: Invalid Input Rejection
**Steps:**
1. Click BPM field
2. Enter "abc" (non-numeric)
3. Press Enter

**Expected Result:**
- Label reverts to previous valid BPM value
- No change to tempo

### Test 5: Decimal Rounding
**Steps:**
1. Click BPM field
2. Enter "120.456789"
3. Press Enter

**Expected Result:**
- Label displays "120.46" (rounded to 2 decimal places)
- BPM set to exactly 120.46

---

## Drum Pad MIDI Mapping

### Test 1: Pad Triggering
**Steps:**
1. Create a drum track
2. Load samples into pads 1-16
3. Click each pad sequentially

**Expected Result:**
- Each pad triggers its assigned sample
- MIDI notes 36-51 (C1-D#2) are sent to sampler

### Test 2: External MIDI Input
**Steps:**
1. Connect MIDI keyboard
2. Create drum track with loaded samples
3. Play MIDI notes 36-51 on keyboard

**Expected Result:**
- Pads trigger corresponding to MIDI notes
- Pad 1 = Note 36 (C1), Pad 16 = Note 51 (D#2)

---

## Track Management

###Test 1: Add Drum Track
**Steps:**
1. Click "Add Track" or equivalent
2. Select "Drum Track" option

**Expected Result:**
- New track created with sampler plugin
- Track marked as drum type
- 16 drum pads available

### Test 2: Add Instrument Track
**Steps:**
1. Click "Add Track"
2. Select "Instrument Track"

**Expected Result:**
- New track created with FourOsc synth
- Track marked as instrument type
- Piano roll available for editing

### Test 3: Solo/Mute Behavior
**Steps:**
1. Create 3 tracks
2. Solo track 2
3. Verify only track 2 plays
4. Un-solo track 2
5. Mute track 1
6. Verify track 1 is silent

**Expected Result:**
- Solo correctly isolates single track
- Mute silences track without affecting others

---

## Save/Load Persistence

### Test 1: BPM Persistence
**Steps:**
1. Set BPM to 140
2. Save project
3. Close application
4. Reopen project

**Expected Result:**
- BPM loads as 140

### Test 2: Track Type Persistence
**Steps:**
1. Create 2 drum tracks and 1 instrument track
2. Save project
3. Close and reopen

**Expected Result:**
- Tracks load with correct types
- Drum tracks have sampler
- Instrument track has FourOsc

### Test 3: Drum Sample Persistence
**Steps:**
1. Load samples into drum pads
2. Save project
3. Close and reopen

**Expected Result:**
- Samples reload correctly on pads
- Trigger correctly after load

---

## Notes

- These tests should be run before releases
- Document any failures with screenshots
- Update tests as features change
