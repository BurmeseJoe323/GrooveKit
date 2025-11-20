#pragma once
/**
    FourOSC editor UI

    - Re-usable ParamPanelBase handles safe polling of AutomatableParameters
      and user gesture tracking.
    - OscStrip renders one OSC row (OSC1..OSC4).
    - FilterPanel renders the filter & its envelope.
    - AmpPanel renders the amp ADSR.
    - FourOscView composes the panels and tabbed OSC pages.
    - FourOscWindow wraps the view in a DocumentWindow.
*/

#include <juce_gui_basics/juce_gui_basics.h>
#include <tracktion_engine/tracktion_engine.h>
#include <atomic>
#include <vector>

namespace te = tracktion::engine;

//==============================================================================
// Small rotary knob (Slider wrapper) with common JUCE settings
class Knob : public juce::Slider
{
public:
    Knob()
    {
        setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        setTextBoxStyle (juce::Slider::TextBoxBelow, false, 48, 18);
        setRange (0.0, 1.0, 0.0);
        setNumDecimalPlacesToDisplay (3);

        // 300° sweep (120° -> 420°) matching our LookAndFeel
        setRotaryParameters (juce::degreesToRadians (120.0f),
                             juce::degreesToRadians (420.0f),
                             true);
    }
};

//==============================================================================
// Parameter lookup helpers
te::AutomatableParameter* findParam          (te::Plugin& plug, const juce::String& token);
te::AutomatableParameter* findParamExact     (te::Plugin& plug, const juce::StringArray& candidates);
te::AutomatableParameter* findParamExactOnly (te::Plugin& plug, const juce::StringArray& candidates);

//==============================================================================
// Base panel: binds sliders to AutomatableParameters and polls them safely.
class ParamPanelBase : public juce::Component, private juce::Timer
{
public:
    explicit ParamPanelBase (te::Plugin& p) : plugin (p) {}
    ~ParamPanelBase() override;

    virtual void detachFromPlugin();

protected:
    struct Binding { te::AutomatableParameter* param{}; juce::Slider* slider{}; };

    // lifecycle
    void startPolling (int ms) { startTimer (ms); }
    void stopPolling()         { stopTimer(); }

    // binding
    void bind (juce::Slider& slider, te::AutomatableParameter* param);

    // layout helper (knob + centred label stacked vertically)
    void layoutKnob (juce::Slider& knob, juce::Label& label,
                     juce::Rectangle<int> cell, const juce::String& text);

    // per-panel tick after polling has synchronised all sliders
    virtual void panelTick() {}

    // Timer
    void timerCallback() override;

    // data
    std::vector<Binding>   bindings;
    te::Plugin&            plugin;
    std::atomic<int>       activeEdits { 0 };           // mouse drags in progress
    std::atomic<int64_t>   lastEditMs  { 0 };           // last user edit time
    std::atomic<bool>      initialising { true };       // suppress one-shot on open
};

//==============================================================================
// One oscillator row (OSC 1..4)
class OscStrip : public ParamPanelBase
{
public:
    OscStrip (te::Plugin& plug, int oscIndex);  // oscIndex is 1..4
    ~OscStrip() override;

    void resized() override;

    void detachFromPlugin() override;

private:
    void panelTick() override;

    int         osc = 1;
    juce::Label title { {}, "OSC" };

    // Waveform UI: the real 4OSC exposes a discrete waveChoice
    juce::ComboBox waveSelector;

    // Controls
    Knob wave, pw, tune, fine, unison, detune, spread, level, pan;
    juce::Label waveL, pwL, tuneL, fineL, unisonL, detuneL, spreadL, levelL, panL;

    // Direct access for waveform choice
    te::FourOscPlugin* fourOsc = nullptr;
};

//==============================================================================
// Filter controls + Filter envelope
class FilterPanel : public ParamPanelBase
{
public:
    explicit FilterPanel (te::Plugin& plug);
    ~FilterPanel() override;

    void resized() override;

private:
    juce::Label title { {}, "Filter" };

    // main filter
    Knob cutoff, resonance, envAmt, keyTrack, velocity;
    juce::Label cutoffL, resonanceL, envAmtL, keyTrackL, velocityL;

    // filter envelope (ADSR)
    Knob fA, fD, fS, fR;
    juce::Label fAL, fDL, fSL, fRL;
};

//==============================================================================
// Amp envelope (ADSR)
class AmpPanel : public ParamPanelBase
{
public:
    explicit AmpPanel (te::Plugin& plug);
    ~AmpPanel() override;

    void resized() override;

private:
    juce::Label title { {}, "Amp" };

    Knob a, d, s, r;
    juce::Label aL, dL, sL, rL;
};

//==============================================================================
// Overall view: two OSC pages + Filter + Amp
class FourOscView : public juce::Component
{
public:
    explicit FourOscView (te::Plugin& plug);
    void resized() override;
    void detachAllPanels();

private:
    te::Plugin& plugin;

    juce::TabbedComponent tabs { juce::TabbedButtonBar::TabsAtTop };
    juce::Component osc12Page, osc34Page;

    OscStrip   osc1, osc2, osc3, osc4;
    FilterPanel filter;
    AmpPanel    amp;
};

//==============================================================================
// Simple window wrapper
class FourOscWindow : public juce::DocumentWindow
{
public:
    explicit FourOscWindow (te::Plugin& plugin);

    void closeButtonPressed() override { setVisible (false); }

    void detachAllPanels();

private:
    FourOscView view;
};


