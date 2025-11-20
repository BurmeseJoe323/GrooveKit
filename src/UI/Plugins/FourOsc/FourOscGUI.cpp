/**
    FourOSC editor UI implementation (see FourOscGUI.h)

    Notes:
    - We only write to the engine on user gesture (slider drag / combo change).
      Normal timer polling keeps sliders in sync with engine values.
    - A small helper maps a few well-known 4OSC parameter names to ValueTree
      keys so the Tracktion UI remains consistent if the plugin uses VT.
*/

#include "FourOscGUI.h"
#include "FourOscLookAndFeel.h"

namespace
{
    // Keep one L&F alive for all knobs (lifetime >= sliders)
    FourOscLookAndFeel gFourOscLNF;
}

namespace te = tracktion::engine;

//==============================================================================
// local helpers (used below)

// Try any of several token substrings
static te::AutomatableParameter* findParamAny (te::Plugin& plug,
                                               std::initializer_list<const char*> tokens)
{
    for (auto* p : plug.getAutomatableParameters())
    {
        if (!p) continue;
        const auto name = p->getParameterName();
        for (auto* t : tokens)
            if (name.containsIgnoreCase (t))
                return p;
    }
    return nullptr;
}

// Map a handful of 4OSC names to VT keys (only used when user edits)
static std::optional<juce::Identifier> vtKeyFor4Osc (const juce::String& pname)
{
    if      (pname.equalsIgnoreCase ("Amp Attack"))       return "ampAttack";
    else if (pname.equalsIgnoreCase ("Amp Decay"))        return "ampDecay";
    else if (pname.equalsIgnoreCase ("Amp Sustain"))      return "ampSustain";
    else if (pname.equalsIgnoreCase ("Amp Release"))      return "ampRelease";
    else if (pname.equalsIgnoreCase ("Filter Resonance")) return "filterResonance";
    else if (pname.equalsIgnoreCase ("Filter Freq"))      return "filterFreq";

    return std::nullopt;
}

//==============================================================================
// Param lookups (public helpers)

te::AutomatableParameter* findParam (te::Plugin& plug, const juce::String& token)
{
    for (auto* p : plug.getAutomatableParameters())
        if (p && p->getParameterName().containsIgnoreCase (token))
            return p;
    return nullptr;
}

te::AutomatableParameter* findParamExactOnly (te::Plugin& plug,
                                              const juce::StringArray& candidates)
{
    for (auto* p : plug.getAutomatableParameters())
    {
        if (!p) continue;
        const auto n = p->getParameterName().trim();
        for (auto& c : candidates)
            if (n.equalsIgnoreCase (c.trim()))
                return p;
    }
    return nullptr;
}

te::AutomatableParameter* findParamExact (te::Plugin& plug,
                                          const juce::StringArray& candidates)
{
    // 1) exact
    for (auto* p : plug.getAutomatableParameters())
    {
        if (!p) continue;
        const auto n = p->getParameterName().trim();
        for (auto& c : candidates)
            if (n.equalsIgnoreCase (c.trim()))
                return p;
    }
    // 2) fallback: contains
    for (auto* p : plug.getAutomatableParameters())
    {
        if (!p) continue;
        const auto n = p->getParameterName();
        for (auto& c : candidates)
            if (n.containsIgnoreCase (c))
                return p;
    }
    return nullptr;
}

//==============================================================================
// ParamPanelBase
ParamPanelBase::~ParamPanelBase()
{
    stopPolling();
}

void ParamPanelBase::layoutKnob (juce::Slider& knob, juce::Label& label,
                                 juce::Rectangle<int> cell, const juce::String& text)
{
    constexpr int labelH = 16;
    auto knobArea  = cell.withTrimmedBottom (labelH);
    auto labelArea = cell.removeFromBottom (labelH);

    knob.setBounds (knobArea);
    label.setText (text, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
    label.setBounds (labelArea);
}

void ParamPanelBase::bind (juce::Slider& s, te::AutomatableParameter* p)
{
    bindings.push_back ({ p, &s });

    if (p != nullptr)
        s.setValue (p->getCurrentValue(), juce::dontSendNotification);

    // gesture bookkeeping
    s.onDragStart = [this] { activeEdits.fetch_add (1); };
    s.onDragEnd   = [this] {
        activeEdits.fetch_sub (1);
        lastEditMs = (int64_t) juce::Time::getMillisecondCounter();
    };

    // write-through only on user change
    s.onValueChange = [this, &s, p]
    {
        if (!p || !s.isMouseButtonDown())
            return;

        const float v = (float) s.getValue();
        p->setParameter (v, juce::sendNotification);

        if (auto key = vtKeyFor4Osc (p->getParameterName()))
            if (plugin.state.hasProperty (*key))
                plugin.state.setProperty (*key, v, nullptr);

        lastEditMs = (int64_t) juce::Time::getMillisecondCounter();
    };
}

void ParamPanelBase::timerCallback()
{
    // Don't fight the user; defer for a short time after an edit
    if (activeEdits.load() > 0)
        return;

    const auto nowMs = (int64_t) juce::Time::getMillisecondCounter();
    if (nowMs - lastEditMs.load() < 120)
        return;

    // Pull current param values into the sliders
    for (auto& b : bindings)
    {
        if (!b.param || !b.slider) continue;
        if (b.slider->isMouseButtonDown() || b.slider->isCurrentlyModal()) continue;

        const float pv = b.param->getCurrentValue();
        if (std::abs ((float) b.slider->getValue() - pv) > 1e-6f)
            b.slider->setValue (pv, juce::dontSendNotification);
    }

    panelTick();
}

// Call when switching edits or replacing plugins
void ParamPanelBase::detachFromPlugin()
{
    stopPolling();
}


//==============================================================================
// OscStrip

OscStrip::OscStrip (te::Plugin& plug, int oscIndex)
    : ParamPanelBase (plug), osc (oscIndex)
{
    // Optional direct FourOsc access (for wave selection)
    if (auto* f = dynamic_cast<te::FourOscPlugin*>(&plugin))
    {
        fourOsc = f;

        // Wave choices
        waveSelector.addItem ("Sine",     1);
        waveSelector.addItem ("Square",   2);
        waveSelector.addItem ("Saw Up",   3);
        waveSelector.addItem ("Saw Down", 4);

        const int current = fourOsc->oscParams[osc - 1]->waveShapeValue.get();
        waveSelector.setSelectedId (current + 1, juce::dontSendNotification);

        waveSelector.onChange = [this]
        {
            if (!fourOsc || initialising.load()) return;
            const int choiceIndex = waveSelector.getSelectedId() - 1;   // 0..3
            fourOsc->oscParams[osc - 1]->waveShapeValue = choiceIndex;
        };

        waveSelector.setLookAndFeel (&gFourOscLNF);
        waveSelector.setJustificationType (juce::Justification::centredLeft);
        waveSelector.setColour (juce::ComboBox::textColourId, juce::Colours::white.withAlpha (0.9f));
        waveSelector.setColour (juce::ComboBox::backgroundColourId, juce::Colours::transparentBlack);
        waveSelector.setColour (juce::ComboBox::outlineColourId, juce::Colours::transparentBlack);

        addAndMakeVisible (waveSelector);
    }

    addAndMakeVisible (title);
    title.setText ("OSC " + juce::String (osc), juce::dontSendNotification);
    title.setJustificationType (juce::Justification::centredLeft);

    // Controls
    for (auto* k : { &wave, &pw, &tune, &fine, &unison, &detune, &spread, &level, &pan })
    {
        k->setLookAndFeel (&gFourOscLNF);
        addAndMakeVisible (*k);
    }
    for (auto* l : { &pwL, &tuneL, &fineL, &unisonL, &detuneL, &spreadL, &levelL, &panL })
        addAndMakeVisible (*l);

    // Param bindings (exact names used by FourOSC)
    const juce::String n = juce::String (osc);
    bind (pw,     findParamExactOnly (plug, { "Pulse Width " + n }));
    bind (tune,   findParamExactOnly (plug, { "Tune " + n }));
    bind (fine,   findParamExactOnly (plug, { "Fine Tune " + n }));
    bind (detune, findParamExactOnly (plug, { "Detune " + n }));
    bind (spread, findParamExactOnly (plug, { "Spread " + n }));
    bind (level,  findParamExactOnly (plug, { "Level " + n }));
    bind (pan,    findParamExactOnly (plug, { "Pan " + n }));

    startPolling (30);

    initialising.store (false);
}

OscStrip::~OscStrip()
{
    waveSelector.setLookAndFeel (nullptr);
    for (auto* k : { &wave, &pw, &tune, &fine, &unison, &detune, &spread, &level, &pan })
        k->setLookAndFeel (nullptr);
}

void OscStrip::panelTick()
{
    auto* fo = fourOsc;
    if (fo == nullptr)
    {
        stopPolling();
        return;
    }

    // Clamp osc index safely (we have exactly 4 oscs)
    if (osc < 1 || osc > 4)
        return;

    const int oscIdx = osc - 1;

    auto* params = fo->oscParams[oscIdx];
    if (params == nullptr)
        return;

    const int targetId = params->waveShapeValue.get() + 1;

    if (waveSelector.isPopupActive())
        return;

    if (waveSelector.getSelectedId() != targetId)
        waveSelector.setSelectedId (targetId, juce::dontSendNotification);
}


void OscStrip::detachFromPlugin()
{
    stopPolling();   // ✅ use the wrapper, not stopTimer()
    fourOsc = nullptr;
}


void OscStrip::resized()
{
    auto area = getLocalBounds().reduced (8);

    // Title row
    title.setBounds (area.removeFromTop (18));
    area.removeFromTop (6);

    // Control row
    auto line  = area.removeFromTop (140);
    const int cols  = 9;
    const int cellW = juce::jmax (1, line.getWidth() / cols);

    auto nextCell = [&]() -> juce::Rectangle<int>
    {
        return line.removeFromLeft (cellW).reduced (0);
    };

    // Wave selector / label
    {
        auto c0 = nextCell();

        if (fourOsc != nullptr)
        {
            auto comboBounds = c0.removeFromTop (24);
            waveSelector.setBounds (comboBounds);

            waveL.setText ("Wave", juce::dontSendNotification);
            waveL.setJustificationType (juce::Justification::centred);
            waveL.setBounds (c0.removeFromTop (16));

            wave.setVisible (false);
        }
        else
        {
            wave.setVisible (true);
            layoutKnob (wave, waveL, c0, "Wave");
        }
    }

    layoutKnob (pw,     pwL,     nextCell(), "Pulse Width");
    layoutKnob (tune,   tuneL,   nextCell(), "Tune");
    layoutKnob (fine,   fineL,   nextCell(), "Fine");
    layoutKnob (unison, unisonL, nextCell(), "Unison");
    layoutKnob (detune, detuneL, nextCell(), "Detune");
    layoutKnob (spread, spreadL, nextCell(), "Spread");
    layoutKnob (level,  levelL,  nextCell(), "Level");
    layoutKnob (pan,    panL,    nextCell(), "Pan");
}

//==============================================================================
// FilterPanel

FilterPanel::FilterPanel (te::Plugin& plug) : ParamPanelBase (plug)
{
    addAndMakeVisible (title);
    title.setJustificationType (juce::Justification::centredLeft);

    for (auto* k : { &cutoff, &resonance, &envAmt, &keyTrack, &velocity, &fA, &fD, &fS, &fR })
    {
        k->setLookAndFeel (&gFourOscLNF);
        addAndMakeVisible (*k);
    }

    for (auto* l : { &cutoffL, &resonanceL, &envAmtL, &keyTrackL, &velocityL, &fAL, &fDL, &fSL, &fRL })
        addAndMakeVisible (*l);

    // bind filter & its ADSR
    bind (cutoff,    findParamAny (plug, { "Filter Freq", "Cutoff" }));
    bind (resonance, findParamAny (plug, { "Filter Resonance", "Resonance" }));
    bind (envAmt,    findParamAny (plug, { "Env Amt", "Filter Amount", "Filter Env" }));
    bind (keyTrack,  findParamAny (plug, { "Key Trk", "Filter Key" }));
    bind (velocity,  findParamAny (plug, { "Velocity", "Filter Velocity" }));

    bind (fA, findParamAny (plug, { "Filter Attack"  }));
    bind (fD, findParamAny (plug, { "Filter Decay"   }));
    bind (fS, findParamAny (plug, { "Filter Sustain" }));
    bind (fR, findParamAny (plug, { "Filter Release" }));

    startPolling (30);
}

FilterPanel::~FilterPanel()
{
    for (auto* k : { &cutoff, &resonance, &envAmt, &keyTrack, &velocity, &fA, &fD, &fS, &fR })
        k->setLookAndFeel (nullptr);
}

void FilterPanel::resized()
{
    auto r   = getLocalBounds().reduced (12);
    auto row = [&r](int h){ auto x = r.removeFromTop (h); r.removeFromTop (10); return x; };

    title.setBounds (row (22));

    {
        // main filter row
        auto line  = row (140);
        const int cols = 5;
        const int cellW = line.getWidth() / cols;
        auto next = [&]() { return line.removeFromLeft (cellW).reduced (8); };

        layoutKnob (cutoff,    cutoffL,    next(), "Freq");
        layoutKnob (resonance, resonanceL, next(), "Res");
        layoutKnob (envAmt,    envAmtL,    next(), "Env Amt");
        layoutKnob (keyTrack,  keyTrackL,  next(), "Key Trk");
        layoutKnob (velocity,  velocityL,  next(), "Velocity");
    }

    {
        // filter ADSR row
        auto line  = row (140);
        const int cols = 4;
        const int cellW = line.getWidth() / cols;
        auto next = [&]() { return line.removeFromLeft (cellW).reduced (8); };

        layoutKnob (fA, fAL, next(), "A");
        layoutKnob (fD, fDL, next(), "D");
        layoutKnob (fS, fSL, next(), "S");
        layoutKnob (fR, fRL, next(), "R");
    }
}

//==============================================================================
// AmpPanel

AmpPanel::AmpPanel (te::Plugin& plug) : ParamPanelBase (plug)
{
    addAndMakeVisible (title);
    title.setJustificationType (juce::Justification::centredLeft);

    for (auto* k : { &a, &d, &s, &r })
    {
        k->setLookAndFeel (&gFourOscLNF);
        addAndMakeVisible (*k);
    }
    for (auto* l : { &aL, &dL, &sL, &rL })
        addAndMakeVisible (*l);

    bind (a, findParam (plug, "Amp Attack"));
    bind (d, findParam (plug, "Amp Decay"));
    bind (s, findParam (plug, "Amp Sustain"));
    bind (r, findParam (plug, "Amp Release"));

    startPolling (30);
}

AmpPanel::~AmpPanel()
{
    for (auto* k : { &a, &d, &s, &r })
        k->setLookAndFeel (nullptr);
}

void AmpPanel::resized()
{
    auto area = getLocalBounds().reduced (15);
    title.setBounds (area.removeFromTop (18));
    area.removeFromTop (6);

    auto row = area.removeFromTop (140);
    const int cols = 4;
    const int cellW = row.getWidth() / cols;
    auto next = [&]() { return row.removeFromLeft (cellW).reduced (10); };

    layoutKnob (a, aL, next(), "A");
    layoutKnob (d, dL, next(), "D");
    layoutKnob (s, sL, next(), "S");
    layoutKnob (r, rL, next(), "R");
}

//==============================================================================
// FourOscView

FourOscView::FourOscView (te::Plugin& plug)
    : plugin (plug),
      osc1 (plug, 1), osc2 (plug, 2), osc3 (plug, 3), osc4 (plug, 4),
      filter (plug), amp (plug)
{
    addAndMakeVisible (tabs);

    osc12Page.addAndMakeVisible (osc1);
    osc12Page.addAndMakeVisible (osc2);

    osc34Page.addAndMakeVisible (osc3);
    osc34Page.addAndMakeVisible (osc4);

    tabs.addTab ("OSC 1 & 2", juce::Colours::transparentBlack, &osc12Page, false);
    tabs.addTab ("OSC 3 & 4", juce::Colours::transparentBlack, &osc34Page, false);

    addAndMakeVisible (filter);
    addAndMakeVisible (amp);
}

void FourOscView::resized()
{
    auto r = getLocalBounds().reduced (8);

    // Tabs / OSC pages
    auto top = r.removeFromTop (220);
    tabs.setBounds (top);

    auto layHalves = [] (juce::Component& a, juce::Component& b, juce::Rectangle<int> area)
    {
        auto left = area.removeFromLeft (area.getWidth() / 2).reduced (6);
        a.setBounds (left);
        b.setBounds (area.reduced (6));
    };
    layHalves (osc1, osc2, tabs.getTabContentComponent (0)->getLocalBounds().withSizeKeepingCentre (top.getWidth(), 180));
    layHalves (osc3, osc4, tabs.getTabContentComponent (1)->getLocalBounds().withSizeKeepingCentre (top.getWidth(), 180));

    // Filter (2 rows) + Amp (1 row)
    const int titleH = 22, gap = 10, rowH = 140;
    const int filterH = titleH + gap + rowH + gap + rowH;
    filter.setBounds (r.removeFromTop (filterH));

    const int ampH = titleH + gap + rowH;
    amp.setBounds (r.removeFromTop (ampH));
}

void FourOscView::detachAllPanels()
{
    // Stop polling and clear pointers on all panels
    osc1.detachFromPlugin();
    osc2.detachFromPlugin();
    osc3.detachFromPlugin();
    osc4.detachFromPlugin();
    filter.detachFromPlugin();
    amp.detachFromPlugin();
}


//==============================================================================
// FourOscWindow

FourOscWindow::FourOscWindow (te::Plugin& plug)
    : DocumentWindow ("Instrument", juce::Colours::black, DocumentWindow::closeButton),
      view (plug)
{
    setUsingNativeTitleBar (true);
    setContentOwned (&view, false);

    setName (plug.getName());

    setResizable (true, true);
    setResizeLimits (700, 480, 2000, 1200);
    setSize (1200, 800);
    centreWithSize (getWidth(), getHeight());
}

void FourOscWindow::detachAllPanels()
{
    view.detachAllPanels();
}
