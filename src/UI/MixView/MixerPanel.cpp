#include "MixerPanel.h"

MixerPanel::MixerPanel (AppEngine& engine)
    : appEngine (engine)
{
    refreshTracks();
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

    DBG ("Mixer sees " << tracktion::getAudioTracks (appEngine.getEdit()).size() << " tracks");
    for (auto* t : tracktion::getAudioTracks (appEngine.getEdit()))
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

        // Initialize UI state from engine
        strip->setMuted (appEngine.isTrackMuted (i));
        strip->setSolo  (appEngine.isTrackSoloed (i));

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