add_library(app_ui
        DrumSamplerView/DrumSamplerView.h
        DrumSamplerView/DrumSamplerLauncher.cpp
        DrumSamplerView/DrumSamplerLauncher.h
        DrumSamplerView/DrumSamplerEngine.h
        DrumSamplerView/DrumPadComponent.h
        DrumSamplerView/DrumSamplerComponent.h
        DrumSamplerView/SampleLibraryComponent.h
        PopupWindows/OutputDevice/OutputDeviceWindow.cpp
        PopupWindows/OutputDevice/OutputDeviceWindow.h
        MixView/MixView.cpp
        MixView/MixView.h
        MixView/MixerPanel.h
        MixView/ChannelComponents/ChannelStripComponents/FaderComponent.h
        MixView/ChannelComponents/ChannelStrip.h
        MixView/ChannelComponents/ChannelStrip.cpp
        MixView/MixerPanel.cpp
        Plugins/FourOsc/FourOscGUI.h
        Plugins/FourOsc/FourOscGUI.cpp
        TrackView/TimelineComponent.h
        TrackView/TimelineComponent.cpp
        Plugins/Synthesizer/MorphSynthView.h
        Plugins/Synthesizer/MorphSynthView.cpp
        Plugins/Synthesizer/MorphSynthWindow.h
        Plugins/Synthesizer/MorphLookAndFeel.h
)

target_sources(app_ui
        PRIVATE
        MainComponent.cpp MainComponent.h
        TransportBar/TransportBar.cpp TransportBar/TransportBar.h
        MenuBar/GrooveKitMenuBar.cpp MenuBar/GrooveKitMenuBar.h
        Settings/SettingsDialog.cpp Settings/SettingsDialog.h
        Settings/AudioSettingsPanel.cpp Settings/AudioSettingsPanel.h
        Settings/MidiSettingsPanel.cpp Settings/MidiSettingsPanel.h
        TrackView/TrackEditView.cpp TrackView/TrackEditView.h
        TrackView/TrackComponent.cpp TrackView/TrackComponent.h
        TrackView/TrackHeaderComponent.cpp TrackView/TrackHeaderComponent.h
        TrackView/TrackClip.cpp TrackView/TrackClip.h
        TrackView/GhostClipComponent.cpp TrackView/GhostClipComponent.h
        TrackView/TrackListComponent.cpp TrackView/TrackListComponent.h
        TrackView/PlayheadComponent.cpp TrackView/PlayheadComponent.h
        TrackView/LoopRangeComponent.cpp TrackView/LoopRangeComponent.h
        PopupWindows/PianoRollComponents/PianoRollEditor.cpp PopupWindows/PianoRollComponents/PianoRollEditor.h
        PopupWindows/PianoRollComponents/KeyboardComponent.cpp PopupWindows/PianoRollComponents/KeyboardComponent.h
        PopupWindows/PianoRollComponents/NoteGridComponent.cpp PopupWindows/PianoRollComponents/NoteGridComponent.h
        PopupWindows/PianoRollComponents/TimelineComponent.cpp PopupWindows/PianoRollComponents/TimelineComponent.h
        PopupWindows/PianoRollComponents/GridControlPanel.cpp PopupWindows/PianoRollComponents/GridControlPanel.h
        PopupWindows/PianoRollComponents/GridStyleSheet.cpp PopupWindows/PianoRollComponents/GridStyleSheet.h
        PopupWindows/PianoRollComponents/NoteComponent.cpp PopupWindows/PianoRollComponents/NoteComponent.h
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/UI/DrumSamplerView/DefaultSampleLibrary.cpp
        # Resources
        ${CMAKE_SOURCE_DIR}/Resources/exampleGUISetup.cpp ${CMAKE_SOURCE_DIR}/Resources/exampleGUISetup.h
        MixView/MixView.cpp
        MixView/MixerPanel.cpp
        MixView/ChannelComponents/ChannelStrip.cpp
        PopupWindows/OutputDevice/OutputDeviceWindow.cpp
)

target_include_directories(app_ui
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(app_ui
        PUBLIC
            juce::juce_gui_basics
            app_engine
            drum_sampler_engine
            tracktion_engine
        PRIVATE
            GrooveKitResources
)
