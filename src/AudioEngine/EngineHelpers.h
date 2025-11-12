#pragma once

#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * Helper functions adapted from Tracktion Engine examples
 * Located in: ~/tracktion_engine/examples/common/Utilities.h
 *
 * These are battle-tested patterns for common engine operations.
 */
namespace EngineHelpers
{
    /**
     * Toggles recording state on the edit's transport.
     * If recording: stops and keeps recordings (creates clips)
     * If not recording: starts recording
     */
    inline void toggleRecord(te::Edit& edit)
    {
        auto& transport = edit.getTransport();

        if (transport.isRecording())
            transport.stop(false, false);  // false = keep recordings (create clips)
        else
            transport.record(false);       // Start recording
    }

    /**
     * Toggles play/pause state on the edit's transport.
     * @param returnToStart If true, playback restarts from beginning when starting
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

    /**
     * Arms or disarms a track for recording.
     * This enables/disables recording on all input devices targeting this track.
     *
     * @param track The track to arm/disarm
     * @param arm True to arm, false to disarm
     * @param position Input device position (default 0)
     */
    inline void armTrack(te::AudioTrack& track, bool arm, int position = 0)
    {
        auto& edit = track.edit;
        for (auto* instance : edit.getAllInputDevices())
            if (te::isOnTargetTrack(*instance, track, position))
                instance->setRecordingEnabled(track.itemID, arm);
    }

    /**
     * Checks if a track is armed for recording.
     *
     * @param track The track to check
     * @param position Input device position (default 0)
     * @return True if the track is armed
     */
    inline bool isTrackArmed(te::AudioTrack& track, int position = 0)
    {
        auto& edit = track.edit;
        for (auto* instance : edit.getAllInputDevices())
            if (te::isOnTargetTrack(*instance, track, position))
                return instance->isRecordingEnabled(track.itemID);

        return false;
    }

    /**
     * Gets or creates an audio track at a specific index.
     * Ensures the edit has at least (index + 1) tracks.
     *
     * @param edit The edit to modify
     * @param index The track index (0-based)
     * @return Pointer to the track at that index, or nullptr if failed
     */
    inline te::AudioTrack* getOrInsertAudioTrackAt(te::Edit& edit, int index)
    {
        edit.ensureNumberOfAudioTracks(index + 1);
        return te::getAudioTracks(edit)[index];
    }
}
