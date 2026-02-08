#pragma once
#include "../MIDIEngine/MIDIEngine.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Manages audio playback, device configuration, and MIDI input routing.
 *
 * AudioEngine wraps Tracktion's device manager and transport control, providing
 * a simplified interface for:
 *  - Audio output device enumeration and selection
 *  - Transport control (play, stop, recording)
 *  - MIDI input device management and routing
 *  - Default audio configuration (48kHz sample rate, 512 buffer size)
 *
 * This class is owned by AppEngine and coordinates with MIDIEngine for clip management.
 */
class AudioEngine
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the AudioEngine.
     *
     * @param editRef Reference to the Tracktion Edit to control.
     * @param engine Reference to the Tracktion Engine instance.
     */
    AudioEngine (te::Edit& editRef, te::Engine& engine);

    /** Destructor. */
    ~AudioEngine();

    //==============================================================================
    // Transport Control

    /**
     * @brief Starts playback from the current position or loop start.
     *
     * If looping is enabled, playback starts from the loop range start position.
     * Otherwise, playback continues from the current playhead position.
     */
    void play();

    /**
     * @brief Stops playback and silences all active notes.
     *
     * This stops the transport and sends note-off messages to all MorphSynth
     * plugins to prevent stuck notes.
     */
    void stop();

    /**
     * @brief Returns whether the transport is currently playing.
     *
     * @return True if playing, false otherwise.
     */
    bool isPlaying() const;

    //==============================================================================
    // Audio Device Management

    /**
     * @brief Lists all available audio output devices.
     *
     * @return Array of output device names.
     */
    juce::StringArray listOutputDevices() const;

    /**
     * @brief Sets the audio output device by name.
     *
     * @param deviceName The name of the output device to use.
     * @return True if the device was successfully set, false otherwise.
     */
    bool setOutputDeviceByName (const juce::String& deviceName);

    /**
     * @brief Resets to the system's default audio output device.
     *
     * @return True if successful, false otherwise.
     */
    bool setDefaultOutputDevice();

    /**
     * @brief Returns the name of the currently active output device.
     *
     * @return The current output device name, or empty string if none.
     */
    juce::String getCurrentOutputDeviceName() const;

    /**
     * @brief Provides access to the underlying JUCE AudioDeviceManager.
     *
     * @return Reference to the AudioDeviceManager.
     */
    juce::AudioDeviceManager& getAudioDeviceManager();

    /**
     * @brief Initializes the audio device manager with default settings.
     *
     * Sets up CoreAudio (macOS) with the specified sample rate and buffer size.
     * Logs available MIDI input devices on startup.
     *
     * @param sampleRate Target sample rate (default: 48000 Hz).
     * @param bufferSize Target buffer size (default: 512 samples).
     */
    void initialiseDefaults (double sampleRate = 48000.0, int bufferSize = 512);

    /**
     * @brief Returns available buffer sizes supported by the current audio device.
     *
     * @return Array of buffer sizes in samples.
     */
    juce::Array<int> getAvailableBufferSizes() const;

    /**
     * @brief Returns available sample rates supported by the current audio device.
     *
     * @return Array of sample rates in Hz.
     */
    juce::Array<double> getAvailableSampleRates() const;

    /**
     * @brief Returns the current audio buffer size.
     *
     * @return Buffer size in samples.
     */
    int getCurrentBufferSize() const;

    /**
     * @brief Returns the current audio sample rate.
     *
     * @return Sample rate in Hz.
     */
    double getCurrentSampleRate() const;

    /**
     * @brief Sets the audio buffer size.
     *
     * @param bufferSize Buffer size in samples.
     * @return true if successful, false on error.
     */
    bool setBufferSize (int bufferSize);

    /**
     * @brief Sets the audio sample rate.
     *
     * @param sampleRate Sample rate in Hz.
     * @return true if successful, false on error.
     */
    bool setSampleRate (double sampleRate);

    //==============================================================================
    // MIDI Input Device Management

    /**
     * @brief Enables all MIDI input devices for the specified edit.
     *
     * Sets all physical MIDI input devices to enabled and configures them with
     * automatic monitor mode (monitors when stopped, silent during playback/recording).
     * Also ensures the transport context is allocated for recording readiness.
     *
     * This should be called once during edit initialization.
     *
     * @param edit The edit to configure MIDI devices for.
     */
    void setupMidiInputDevices(te::Edit& edit);

    /**
     * @brief Enables verbose MIDI event logging for debugging.
     *
     * When enabled, logs all incoming MIDI messages (note on/off, CC, etc.) with
     * timestamps to help diagnose recording issues. Call this before starting recording
     * if you need to debug MIDI event capture.
     *
     * @param enable True to enable logging, false to disable.
     */
    void setMidiEventLoggingEnabled(bool enable);

    /**
     * @brief Routes all MIDI input devices to a specific track and pre-arms it.
     *
     * This sets the target track for all physical MIDI inputs and enables recording
     * on those inputs, preparing the track to capture MIDI when recording starts.
     * Follows Tracktion's MidiRecordingDemo pattern of pre-arming at track selection time.
     *
     * Calls restartPlayback() to rebuild the audio graph with the new routing.
     *
     * @param edit The edit containing the track.
     * @param trackIndex The 0-based index of the track to route MIDI to.
     */
    void routeMidiToTrack(te::Edit& edit, int trackIndex);

    /**
     * @brief Lists all available MIDI input devices.
     *
     * Tracktion automatically manages MIDI device enabling, so this is for display purposes only.
     *
     * @return Array of MIDI input device names.
     */
    juce::StringArray listMidiInputDevices() const;

    /**
     * @brief Logs all available MIDI input devices to the console.
     *
     * Useful for debugging MIDI connectivity issues.
     */
    void logAvailableMidiDevices() const;

private:
    //==============================================================================
    // Internal Methods

    /**
     * @brief Provides access to the JUCE AudioDeviceManager.
     *
     * @return Reference to the device manager from the Tracktion engine.
     */
    juce::AudioDeviceManager& adm() const;

    /**
     * @brief Applies a new audio device setup configuration.
     *
     * @param setup The audio device setup to apply.
     * @return True if successful, false if an error occurred.
     */
    bool applySetup (const juce::AudioDeviceManager::AudioDeviceSetup& setup);

    //==============================================================================
    // Internal Classes

    /**
     * @brief MIDI event logger for debugging recording issues.
     *
     * Attached to the engine's MIDI device manager to intercept and log all
     * incoming MIDI messages during recording sessions.
     */
    class MidiEventLogger : public juce::MidiInputCallback
    {
    public:
        MidiEventLogger() = default;

        void handleIncomingMidiMessage(juce::MidiInput* source,
                                       const juce::MidiMessage& message) override
        {
            juce::ignoreUnused (source);
            if (!enabled)
                return;

            juce::String desc;

            if (message.isNoteOn())
                desc = "NOTE ON:  Note=" + juce::String(message.getNoteNumber()) +
                       " Velocity=" + juce::String(message.getVelocity());
            else if (message.isNoteOff())
                desc = "NOTE OFF: Note=" + juce::String(message.getNoteNumber()) +
                       " Velocity=" + juce::String(message.getVelocity());
            else if (message.isController())
                desc = "CC:       Controller=" + juce::String(message.getControllerNumber()) +
                       " Value=" + juce::String(message.getControllerValue());
            else if (message.isPitchWheel())
                desc = "PITCH BEND: " + juce::String(message.getPitchWheelValue());
            else
                desc = message.getDescription();

            juce::Logger::writeToLog("[MIDI EVENT] " + desc +
                                    " | Time=" + juce::String(message.getTimeStamp(), 3) + "s" +
                                    " | Channel=" + juce::String(message.getChannel()));
        }

        void setEnabled(bool shouldEnable) { enabled = shouldEnable; }

    private:
        std::atomic<bool> enabled{false};
    };

    //==============================================================================
    // Member Variables

    te::Edit& edit;                            ///< Reference to the Tracktion Edit (not owned).
    std::unique_ptr<MIDIEngine> midiEngine;    ///< MIDI clip management engine.
    te::Engine& engine;                        ///< Reference to the Tracktion Engine (not owned).
    MidiEventLogger midiEventLogger;           ///< MIDI event logger for debugging.
};
