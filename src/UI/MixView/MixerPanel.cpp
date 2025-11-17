#include "MixerPanel.h"
namespace te = tracktion::engine;

MixerPanel::MixerPanel (AppEngine& engine)
    : appEngine (engine)
{
    // Setup viewport for horizontal scrolling of track strips
    addAndMakeVisible (tracksViewport);
    tracksViewport.setViewedComponent (&tracksContainer, false);
    tracksViewport.setScrollBarsShown (false, true);  // vertical: no, horizontal: yes

    refreshTracks();

    appEngine.onArmedTrackChanged = [this]
    {
        refreshArmStates();
    };
}

MixerPanel::~MixerPanel()
{
    removeAllChildren();
    trackStrips.clear (true);
    masterStrip.reset();
    DBG ("[MixerPanel] dtor");
}

void MixerPanel::refreshTracks()
{
    // Clear only the container's children, not the viewport
    tracksContainer.removeAllChildren();
    trackStrips.clear (true);

    // Remove and reset master strip
    if (masterStrip)
        removeChildComponent (masterStrip.get());
    masterStrip.reset();

    auto& edit = appEngine.getEdit();
    auto audioTracks = te::getAudioTracks (edit);

    DBG ("Mixer sees " << te::getAudioTracks (appEngine.getEdit()).size() << " tracks");
    for (auto* t : te::getAudioTracks (appEngine.getEdit()))
        DBG ("  " << t->getName());

    for (int i = 0; i < audioTracks.size(); ++i)
    {
        auto* t = audioTracks[i];
        auto* strip = new ChannelStrip();
        strip->setTrackName (t->getName());
        strip->bindToTrack (*t);

        // Reuse existing TrackComponent controller via AppEngine registry (Junie)
        if (auto* listener = appEngine.getTrackListener (i))
            strip->addListener (listener);

        // Always update engine state if listeners aren't present (e.g., Track view not active)
        strip->onRequestMuteChange = [this, idx = i] (bool mute) { appEngine.setTrackMuted (idx, mute); };
        strip->onRequestSoloChange = [this, idx = i] (bool solo) { appEngine.setTrackSoloed (idx, solo); };
        strip->onRequestArmChange = [this, idx = i](bool armed) {
            const int currentSelected = appEngine.getArmedTrackIndex();
            const int newSelected = armed ? idx : -1;
            if (currentSelected != newSelected)
                appEngine.setArmedTrack (newSelected);
        };

        // Initialize UI state from engine
        strip->setMuted (appEngine.isTrackMuted (i));
        strip->setSolo  (appEngine.isTrackSoloed (i));
        strip->setArmed (appEngine.getArmedTrackIndex () == i);

        tracksContainer.addAndMakeVisible (strip);  // Add to scrollable container
        trackStrips.add (strip);
    }

    if (!audioTracks.isEmpty())
    {
        masterStrip = std::make_unique<ChannelStrip>();
        masterStrip->setTrackName ("Master");
        masterStrip->bindToMaster (edit);
        addAndMakeVisible (*masterStrip);
    }

    resized();
    repaint();
}

void MixerPanel::refreshArmStates()
{
    const int selectedTrack = appEngine.getArmedTrackIndex();
    for (int i = 0; i < trackStrips.size(); ++i)
    {
        if (auto* strip = trackStrips[i])
            strip->setArmed (i == selectedTrack);
    }
}

void MixerPanel::resized()
{
    auto r = getLocalBounds().reduced (innerMargin);

    // Master strip stays fixed on right
    if (masterStrip)
    {
        auto masterArea = r.removeFromRight (stripW);
        masterStrip->setBounds (masterArea);
        r.removeFromRight (gap);  // Add gap between viewport and master
    }

    // Viewport fills remaining space on left
    tracksViewport.setBounds (r);

    // Calculate total width needed for all track strips
    const int totalTracksWidth = trackStrips.size() * (stripW + gap);
    tracksContainer.setBounds (0, 0, totalTracksWidth, r.getHeight());

    // Position strips inside container
    int x = 0;
    const int h = tracksContainer.getHeight()-gap; // small padding
    for (auto* s : trackStrips)
    {
        s->setBounds ({ x, 0, stripW, h });
        x += stripW + gap;
    }
}