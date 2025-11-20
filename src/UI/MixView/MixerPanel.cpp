#include "MixerPanel.h"
namespace te = tracktion::engine;

//==============================================================================
// Construction / Destruction

MixerPanel::MixerPanel (AppEngine& engine)
    : appEngine (engine)
{
    // Setup viewport for horizontal scrolling of track strips
    addAndMakeVisible (tracksViewport);
    tracksViewport.setViewedComponent (&tracksContainer, false);
    tracksViewport.setScrollBarsShown (false, true); // vertical: no, horizontal: yes

    refreshTracks();

    appEngine.onArmedTrackChanged = [this] {
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

//==============================================================================
// Public API

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

    // Color palette matching TrackListComponent
    static const juce::Array<juce::Colour> trackColors {
        juce::Colour (0xffff6b6b),  // Red
        juce::Colour (0xfff06595),  // Pink
        juce::Colour (0xffcc5de8),  // Purple
        juce::Colour (0xff845ef7),  // Deep Purple
        juce::Colour (0xff5c7cfa),  // Indigo
        juce::Colour (0xff339af0),  // Blue
        juce::Colour (0xff22b8cf),  // Cyan
        juce::Colour (0xff20c997),  // Teal
        juce::Colour (0xff51cf66),  // Green
        juce::Colour (0xfffcc419)   // Yellow
    };

    for (int i = 0; i < audioTracks.size(); ++i)
    {
        auto* t = audioTracks[i];

        const auto color = trackColors[i % trackColors.size()]; // Color based on index
        auto* strip = new ChannelStrip(color); // Pass color to constructor
        strip->setTrackIndex (i); // Track index for renaming
        strip->setTrackName (t->getName());
        strip->bindToTrack (*t);

        // Reuse existing TrackComponent controller via AppEngine registry
        if (auto* listener = appEngine.getTrackListener (i))
            strip->addListener (listener);

        strip->onRequestMuteChange = [this, idx = i] (bool mute) {
            appEngine.setTrackMuted (idx, mute);
        };
        strip->onRequestSoloChange = [this, idx = i] (bool solo) {
            appEngine.setTrackSoloed (idx, solo);
        };
        strip->onRequestArmChange = [this, idx = i] (bool armed) {
            const int currentSelected = appEngine.getArmedTrackIndex();
            const int newSelected     = armed ? idx : -1;
            if (currentSelected != newSelected)
                appEngine.setArmedTrack (newSelected);
        };
        // Handle track name changes
        strip->onRequestNameChange = [this] (int trackIndex, const juce::String& newName) {
            appEngine.setTrackName (trackIndex, newName);
        };

        strip->onOpenInstrumentEditor = [this, i] {
            appEngine.openInstrumentEditor (i);
        };

        // FX slots – click to open editor or choose FX if empty
        strip->onInsertSlotClicked = [this, i, strip] (int slotIndex)
        {
            appEngine.onFxInsertSlotClicked (
                i,
                slotIndex,
                [strip, slotIndex] (const juce::String& pluginName)
                {
                    strip->setInsertSlotName (slotIndex, pluginName);
                });
        };

        strip->onInsertSlotMenuRequested = [this, i, strip] (int slotIndex)
        {
            appEngine.showFxInsertMenu (
                i,
                slotIndex,
                [strip, slotIndex] (const juce::String& pluginName)
                {
                    strip->setInsertSlotName (slotIndex, pluginName);
                });
        };

        // --- Initialise UI state from engine ---
        strip->setMuted (appEngine.isTrackMuted (i));
        strip->setSolo (appEngine.isTrackSoloed (i));
        strip->setArmed (appEngine.getArmedTrackIndex() == i);

        strip->setInstrumentButtonText (
            appEngine.getInstrumentLabelForTrack (i));

        const int numSlots = strip->getNumInsertSlots();
        for (int slot = 0; slot < numSlots; ++slot)
        {
            const auto label = appEngine.getInsertSlotLabel (i, slot);
            strip->setInsertSlotName (slot, label);
        }

        addAndMakeVisible (strip);
        tracksContainer.addAndMakeVisible (strip); // Add to scrollable container
        trackStrips.add (strip);
    }

    if (! audioTracks.isEmpty())
    {
        masterStrip = std::make_unique<ChannelStrip>(juce::Colours::dimgrey);
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

//==============================================================================
// Component Overrides

void MixerPanel::resized()
{
    auto r = getLocalBounds().reduced (innerMargin);

    // Master strip stays fixed on right
    if (masterStrip)
    {
        auto masterArea = r.removeFromRight (stripW);
        masterStrip->setBounds (masterArea);
        r.removeFromRight (gap); // Add gap between viewport and master
    }

    // Viewport fills remaining space on left
    tracksViewport.setBounds (r);

    // Calculate total width needed for all track strips
    const int totalTracksWidth = trackStrips.size() * (stripW + gap);
    tracksContainer.setBounds (0, 0, totalTracksWidth, r.getHeight());

    // Position strips inside container
    int x = 0;
    const int h = tracksContainer.getHeight() - gap; // small padding
    for (auto* s : trackStrips)
    {
        s->setBounds ({ x, 0, stripW, h });
        x += stripW + gap;
    }
}