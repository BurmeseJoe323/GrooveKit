add_library(app_ui)
target_sources(app_ui
        PRIVATE
        MainComponent.cpp MainComponent.h
        MainViews/WelcomeView.cpp MainViews/WelcomeView.h
        TrackView/TrackEditView.cpp TrackView/TrackEditView.h
        TrackView/TrackComponent.cpp TrackView/TrackComponent.h
        TrackView/TrackHeaderComponent.cpp TrackView/TrackHeaderComponent.h
        TrackView/TrackClip.cpp TrackView/TrackClip.h
        TrackView/TrackListComponent.cpp TrackView/TrackListComponent.h
        TrackView/PlayheadComponent.cpp TrackView/PlayheadComponent.h
        PopupWindows/PianoRollWindow.cpp PopupWindows/PianoRollWindow.h
        PopupWindows/PianoRollComponents/PianoRollEditor.cpp PopupWindows/PianoRollComponents/PianoRollEditor.h
        PopupWindows/PianoRollComponents/KeyboardComponent.cpp PopupWindows/PianoRollComponents/KeyboardComponent.h
        PopupWindows/PianoRollComponents/NoteGridComponent.cpp PopupWindows/PianoRollComponents/NoteGridComponent.h
        PopupWindows/PianoRollComponents/TimelineComponent.cpp PopupWindows/PianoRollComponents/TimelineComponent.h
        PopupWindows/PianoRollComponents/GridControlPanel.cpp PopupWindows/PianoRollComponents/GridControlPanel.h
        PopupWindows/PianoRollComponents/GridStyleSheet.cpp PopupWindows/PianoRollComponents/GridStyleSheet.h
        PopupWindows/PianoRollComponents/NoteComponent.cpp PopupWindows/PianoRollComponents/NoteComponent.h
        PopupWindows/PianoRollComponents/NoteModel.cpp PopupWindows/PianoRollComponents/NoteModel.h
        PopupWindows/PianoRollComponents/PianoRollMainComponent.cpp PopupWindows/PianoRollComponents/PianoRollMainComponent.h
        TrackEditViewTutorial/TrackEditViewTutorial.cpp TrackEditViewTutorial/TrackEditViewTutorial.h
        TutorialManager/TutorialManagerComponent.cpp TutorialManager/TutorialManagerComponent.h
        InstrumentTutorial/InstrumentTutorial.cpp InstrumentTutorial/InstrumentTutorial.h
        # Resources
        ${CMAKE_SOURCE_DIR}/Resources/exampleGUISetup.cpp ${CMAKE_SOURCE_DIR}/Resources/exampleGUISetup.h
)
target_include_directories(app_ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(app_ui PUBLIC juce::juce_gui_basics app_engine)
