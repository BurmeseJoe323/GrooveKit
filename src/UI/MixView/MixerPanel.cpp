#include "MixerPanel.h"
namespace te = tracktion::engine;

MixerPanel::MixerPanel (AppEngine& engine)
    : appEngine (engine)
{
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
    removeAllChildren();
    trackStrips.clear (true);
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

        addAndMakeVisible (strip);
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

    if (masterStrip)
    {
        auto masterArea = r.removeFromRight (stripW);
        masterStrip->setBounds (masterArea);
    }

    int x = r.getX();
    const int y = r.getY();
    const int h = r.getHeight();

    for (auto* s : trackStrips)
    {
        s->setBounds ({ x, y, stripW, h });
        x += stripW + gap;
    }
}