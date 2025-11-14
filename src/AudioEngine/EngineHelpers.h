#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Helper functions for common Tracktion Engine operations.
 *
 * This namespace contains utility functions adapted from Tracktion Engine's official examples
 * (located in ~/tracktion_engine/examples/common/Utilities.h). These functions encode
 * battle-tested patterns for transport control, track arming, and recording management.
 *
 * Important notes:
 *  - These helpers ensure correct parameter usage for Tracktion API calls (e.g., proper
 *    flags for transport.stop() to keep/discard recordings).
 *  - Use these instead of direct Tracktion API calls when performing common operations
 *    to avoid subtle bugs related to recording state management.
 *  - All functions are inline and header-only for convenience.
 */
namespace EngineHelpers
{
    //==============================================================================
    // Transport Control

    /**
     * @brief Toggles the recording state on the edit's transport.
     *
     * If currently recording, stops recording and creates clips from recorded data.
     * If not recording, starts recording on all armed tracks.
     *
     * NOTE: Using stop(false, false) to KEEP recordings and create clips.
     * The official Tracktion examples use stop(true, false) which discards recordings,
     * but that doesn't work for us. Testing with keep=false to see if clips are created.
     *
     * @param edit The edit whose transport to control.
     */
    inline void toggleRecord(te::Edit& edit)
    {
        auto& transport = edit.getTransport();

        if (transport.isRecording())
            transport.stop(false, false);  // false = KEEP recordings, create clips
        else
            transport.record(false);       // Start recording
    }

    /**
     * @brief Toggles the play/pause state on the edit's transport.
     *
     * If currently playing, stops playback. If not playing, starts playback from
     * either the current position or the beginning, depending on the returnToStart parameter.
     *
     * @param edit The edit whose transport to control.
     * @param rts If ReturnToStart::yes, playback restarts from beginning when starting.
     *            If ReturnToStart::no, playback continues from current position.
     */
    enum class ReturnToStart { no, yes };

    inline void togglePlay(te::Edit& edit, ReturnToStart rts = ReturnToStart::no)
    {
        auto& transport = edit.getTransport();

        if (transport.isPlaying())
            transport.stop(false, false);
        else
        {
            if (rts == ReturnToStart::yes)
                transport.playFromStart(true);
            else
                transport.play(false);
        }
    }

    //==============================================================================
    // Track Arming

    /**
     * @brief Arms or disarms a track for recording.
     *
     * This enables or disables recording on all input devices that are currently
     * routed to the specified track. When armed, the track will capture MIDI or audio
     * input when the transport enters recording mode.
     *
     * @param track The track to arm or disarm.
     * @param arm True to arm the track for recording, false to disarm.
     * @param position Input device position (default 0 for primary routing).
     */
    inline void armTrack(te::AudioTrack& track, bool arm, int position = 0)
    {
        auto& edit = track.edit;
        for (auto* instance : edit.getAllInputDevices())
            if (te::isOnTargetTrack(*instance, track, position))
                instance->setRecordingEnabled(track.itemID, arm);
    }

    /**
     * @brief Checks if a track is armed for recording.
     *
     * Returns true if any input device routed to this track has recording enabled.
     *
     * @param track The track to check.
     * @param position Input device position (default 0 for primary routing).
     * @return True if the track is armed for recording, false otherwise.
     */
    inline bool isTrackArmed(te::AudioTrack& track, int position = 0)
    {
        auto& edit = track.edit;
        for (auto* instance : edit.getAllInputDevices())
            if (te::isOnTargetTrack(*instance, track, position))
                return instance->isRecordingEnabled(track.itemID);

        return false;
    }

    //==============================================================================
    // Track Utilities

    /**
     * @brief Gets or creates an audio track at a specific index.
     *
     * Ensures the edit has at least (index + 1) audio tracks, creating new tracks
     * if necessary to reach the desired index.
     *
     * @param edit The edit to modify.
     * @param index The track index (0-based).
     * @return Pointer to the track at the specified index, or nullptr if creation failed.
     */
    inline te::AudioTrack* getOrInsertAudioTrackAt(te::Edit& edit, int index)
    {
        edit.ensureNumberOfAudioTracks(index + 1);
        return te::getAudioTracks(edit)[index];
    }
}
