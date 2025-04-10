//
// Created by ikera on 4/9/2025.
//

#include "EditComponent.h"

#include <tracktion_engine/audio_files/formats/tracktion_FFmpegEncoderAudioFormat.h>

EditComponent::EditComponent() {

}

// this is the destructor
EditComponent::~EditComponent() = default;

void EditComponent::paint(juce::Graphics &g) {
    g.fillAll (juce::Colours::maroon);
}

void EditComponent::resized() {

    const int trackHeight = 50, trackGap = 2;
    // const int headerWidth = editViewState.showHeaders ? 150 : 0;
    // const int footerWidth = editViewState.showFooters ? 150 : 0;

    const int headerWidth = 150;
    const int footerWidth = 150;

    // playhead.setBounds (getLocalBounds().withTrimmedLeft (headerWidth).withTrimmedRight (footerWidth));

    // int y = roundToInt (editViewState.viewY.get());

    int y = 100;
    for (int i = 0; i < jmin (headers.size(), tracks.size()); i++)
    {
        auto h = headers[i];
        auto t = tracks[i];
        // auto f = footers[i];

        h->setBounds (0, y, headerWidth, trackHeight);
        t->setBounds (headerWidth, y, getWidth() - headerWidth - footerWidth, trackHeight);
        // f->setBounds (getWidth() - footerWidth, y, footerWidth, trackHeight);

        y += trackHeight + trackGap;
    }

    for (auto t : tracks)
        t->resized();
}


void EditComponent::buildTracks() {
    tracks.clear();
    headers.clear();
    // footers.clear();

    for (auto t : tracks)
    {
        TrackComponent* c = nullptr;

        c = new TrackComponent();

        // if (t->isMasterTrack())
        // {
        //     if (editViewState.showMasterTrack)
        //         c = new TrackComponent (editViewState, t);
        // }
        // else if (t->isTempoTrack())
        // {
        //     if (editViewState.showGlobalTrack)
        //         c = new TrackComponent (editViewState, t);
        // }
        // else if (t->isMarkerTrack())
        // {
        //     if (editViewState.showMarkerTrack)
        //         c = new TrackComponent (editViewState, t);
        // }
        // else if (t->isChordTrack())
        // {
        //     if (editViewState.showChordTrack)
        //         c = new TrackComponent (editViewState, t);
        // }
        // else if (t->isArrangerTrack())
        // {
        //     if (editViewState.showArrangerTrack)
        //         c = new TrackComponent (editViewState, t);
        // }
        // else
        // {
        //     c = new TrackComponent (editViewState, t);
        // }

        if (c != nullptr)
        {
            tracks.add (c);
            addAndMakeVisible (c);

            auto h = new TrackHeader();
            headers.add (h);
            addAndMakeVisible (h);

            // auto f = new TrackFooterComponent (editViewState, t);
            // footers.add (f);
            // addAndMakeVisible (f);
        }
    }
    // playhead.toFront (false);
    resized();
}


