#pragma once
#include "../MIDIEngine/MIDIEngine.h"
#include "EngineHelpers.h"
#include <juce_audio_devices/juce_audio_devices.h>
#include <tracktion_engine/tracktion_engine.h>

namespace te = tracktion::engine;

/**
 * @brief Audio playback and device management engine wrapping JUCE AudioDeviceManager.
 *
 * AudioEngine provides simplified control over Tracktion Engine's audio playback system
 * and exposes JUCE's AudioDeviceManager for device configuration. It manages:
 *  - Transport control (play/stop/record via Tracktion Engine)
 *  - Audio output device selection and configuration
 *  - Audio buffer size and sample rate settings
 *  - MIDI input device management (routing to tracks via Tracktion InputDevice system)
 *  - MIDIEngine ownership for MIDI clip operations
 *
 * Architecture:
 *  - Wraps Tracktion Engine's transport and device manager
 *  - Owns MIDIEngine for MIDI clip creation/management
 *  - Provides settings API for buffer size, sample rate, and MIDI device enable/disable
 *  - Used by AppEngine as primary audio/MIDI subsystem
 *
 * Usage:
 *  - Constructed with te::Edit and te::Engine references
 *  - Call initialiseDefaults() to set up audio with default 48kHz/512 buffer
 *  - Use play/stop for transport control
 *  - Use setOutputDeviceByName() to change audio devices
 *  - Use setupMidiInputDevices() to enable MIDI input routing
 */
class AudioEngine
{
public:
    //==============================================================================
    // Construction / Destruction

    /**
     * @brief Constructs the AudioEngine.
     *
     * @param editRef Reference to Tracktion Edit (document state)
     * @param engine Reference to Tracktion Engine (global audio system)
     */
    AudioEngine (te::Edit& editRef, te::Engine& engine);

    /** Destructor. */
    ~AudioEngine();

    //==============================================================================
    // Transport Control

    /**
     * @brief Starts audio playback from current transport position.
     */
    void play();

    /**
     * @brief Stops audio playback.
     */
    void stop();

    /**
     * @brief Returns whether audio is currently playing.
     *
     * @return true if playing, false if stopped
     */
    bool isPlaying() const;

    //==============================================================================
    // Audio Output Device Management

    /**
     * @brief Lists all available audio output devices.
     *
     * @return StringArray of device names
     */
    juce::StringArray listOutputDevices() const;

    /**
     * @brief Sets the audio output device by name.
     *
     * @param deviceName Name of the device to use
     * @return true if successful, false on error
     */
    bool setOutputDeviceByName (const juce::String& deviceName);

    /**
     * @brief Resets to the default audio output device.
     *
     * @return true if successful, false on error
     */
    bool setDefaultOutputDevice();

    /**
     * @brief Returns the name of the current audio output device.
     *
     * @return Device name, or empty string if none
     */
    juce::String getCurrentOutputDeviceName() const;

    /**
     * @brief Provides direct access to JUCE AudioDeviceManager.
     *
     * @return Reference to the AudioDeviceManager
     */
    juce::AudioDeviceManager& getAudioDeviceManager();

    /**
     * @brief Initializes audio with default settings.
     *
     * @param sampleRate Sample rate in Hz (default 48000.0)
     * @param bufferSize Buffer size in samples (default 512)
     */
    void initialiseDefaults (double sampleRate = 48000.0, int bufferSize = 512);

    //==============================================================================
    // Audio Configuration

    /**
     * @brief Returns available buffer sizes supported by the current audio device.
     *
     * @return Array of buffer sizes in samples
     */
    juce::Array<int> getAvailableBufferSizes() const;

    /**
     * @brief Returns available sample rates supported by the current audio device.
     *
     * @return Array of sample rates in Hz
     */
    juce::Array<double> getAvailableSampleRates() const;

    /**
     * @brief Returns the current audio buffer size.
     *
     * @return Buffer size in samples
     */
    int getCurrentBufferSize() const;

    /**
     * @brief Returns the current audio sample rate.
     *
     * @return Sample rate in Hz
     */
    double getCurrentSampleRate() const;

    /**
     * @brief Sets the audio buffer size.
     *
     * @param bufferSize Buffer size in samples
     * @return true if successful, false on error
     */
    bool setBufferSize (int bufferSize);

    /**
     * @brief Sets the audio sample rate.
     *
     * @param sampleRate Sample rate in Hz
     * @return true if successful, false on error
     */
    bool setSampleRate (double sampleRate);

    //==============================================================================
    // MIDI Input Device Management

    /**
     * @brief Sets up MIDI input devices using Tracktion's InputDevice system.
     *
     * Enables MIDI routing to tracks via Tracktion Engine's device management.
     *
     * @param edit Reference to Tracktion Edit
     */
    void setupMidiInputDevices(te::Edit& edit);

    /**
     * @brief Routes MIDI input to a specific track.
     *
     * @param edit Reference to Tracktion Edit
     * @param trackIndex Index of track to receive MIDI input
     */
    void routeMidiToTrack(te::Edit& edit, int trackIndex);

    /**
     * @brief Lists all available MIDI input devices.
     *
     * @return StringArray of MIDI device names
     */
    juce::StringArray listMidiInputDevices() const;

    /**
     * @brief Logs available MIDI devices to debug output.
     */
    void logAvailableMidiDevices() const;

    /**
     * @brief Returns list of currently enabled MIDI input devices.
     *
     * @return StringArray of enabled device names
     */
    juce::StringArray getEnabledMidiDevices() const;

    /**
     * @brief Checks if a MIDI input device is enabled.
     *
     * @param deviceName Name of the MIDI device
     * @return true if enabled, false otherwise
     */
    bool isMidiDeviceEnabled (const juce::String& deviceName) const;

    /**
     * @brief Enables or disables a MIDI input device.
     *
     * @param deviceName Name of the MIDI device
     * @param enabled true to enable, false to disable
     */
    void setMidiDeviceEnabled (const juce::String& deviceName, bool enabled);

    //==============================================================================
    // Recording Control

    /**
     * @brief Toggles recording state (uses EngineHelpers).
     *
     * @param edit Reference to Tracktion Edit
     */
    void toggleRecord(te::Edit& edit);

    /**
     * @brief Returns whether recording is active.
     *
     * @return true if recording, false otherwise
     */
    bool isRecording() const;

private:
    //==============================================================================
    // Member Variables

    te::Edit& edit; ///< Reference to Tracktion Edit (not owned)
    std::unique_ptr<MIDIEngine> midiEngine; ///< Owned MIDI clip engine
    te::Engine& engine; ///< Reference to Tracktion Engine (not owned)

    //==============================================================================
    // Internal Methods

    /**
     * @brief Returns reference to JUCE AudioDeviceManager.
     */
    juce::AudioDeviceManager& adm() const;

    /**
     * @brief Applies audio device setup configuration.
     *
     * @param setup AudioDeviceSetup configuration to apply
     * @return true if successful, false on error
     */
    bool applySetup (const juce::AudioDeviceManager::AudioDeviceSetup& setup);
};
