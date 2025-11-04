// ==============================================================================
// MorphSynthView.cpp
// ------------------------------------------------------------------------------
// Implementation of the Morph Synth editor component.
//
// Notes:
//  - This pass adds comments/section banners only. No logic or symbols changed.
// ==============================================================================

#include "MorphSynthView.h"
#include "MorphLookAndFeel.h"

//==============================================================================
// Local helpers
//==============================================================================

/** Populate a ComboBox with a 1-based item list. */
static void addChoices (juce::ComboBox& c, const juce::StringArray& items)
{
    c.clear (juce::dontSendNotification);
    for (int i = 0; i < items.size(); ++i)
        c.addItem (items[i], i + 1);
}

//==============================================================================
// Styling helpers (static)
//==============================================================================

void MorphSynthView::setupKnob (juce::Slider& s)
{
    s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    s.setNumDecimalPlacesToDisplay (2);
    s.setRotaryParameters (juce::degreesToRadians (120.0f),
                           juce::degreesToRadians (420.0f),
                           true);
}

void MorphSynthView::setupLabel (juce::Label& l, const juce::String& text)
{
    l.setText (text, juce::dontSendNotification);
    l.setJustificationType (juce::Justification::centred);
    l.setColour (juce::Label::textColourId, juce::Colours::white.withAlpha (0.9f));
    l.setInterceptsMouseClicks (false, false);
}

//==============================================================================
// Construction / destruction
//==============================================================================

MorphSynthView::MorphSynthView (MorphSynthPlugin& p)
    : plugin (p)
{
    // Look & Feel ---------------------------------------------------------------
    lnf = std::make_unique<MorphLookAndFeel>();
    setLookAndFeel (lnf.get());

    if (lnf)
    {
        // Tone & filter
        lnf->styleKnob (morph);
        lnf->styleKnob (pulseWidth);
        lnf->styleKnob (cutoff);
        lnf->styleKnob (resonance);
        lnf->styleKnob (gain);

        // Selectors
        lnf->styleCombo (oscASelect);
        lnf->styleCombo (oscBSelect);
        lnf->styleCombo (filterSelect);

        // Pitch
        lnf->styleKnob (semi);  lnf->styleKnob (fine);  lnf->styleKnob (glide);

        // Filter env + amount
        lnf->styleKnob (aF);    lnf->styleKnob (dF);    lnf->styleKnob (sF);  lnf->styleKnob (rF);  lnf->styleKnob (fAmt);

        // LFO
        lnf->styleKnob (lfoRate); lnf->styleKnob (lfoDepth);
        lnf->styleCombo (lfoTarget);
    }

    // Selectors ----------------------------------------------------------------
    addAndMakeVisible (oscASelect);
    addAndMakeVisible (oscBSelect);
    addAndMakeVisible (filterSelect);
    addChoices (oscASelect,  { "Sine", "Tri", "Saw", "Pulse" });
    addChoices (oscBSelect,  { "Sine", "Tri", "Saw", "Pulse" });
    addChoices (filterSelect,{ "LP", "BP", "HP" });

    // Knobs --------------------------------------------------------------------
    for (auto* s : { &morph, &pulseWidth, &cutoff, &resonance, &gain, &aA, &dA, &sA, &rA })
    {
        setupKnob (*s);
        addAndMakeVisible (*s);
    }

    // Ranges (match plugin parameter ranges) -----------------------------------
    morph     .setRange (0.0, 1.0);
    pulseWidth.setRange (0.05, 0.95);
    cutoff    .setRange (20.0, 20000.0);
    resonance .setRange (0.1, 1.2);
    gain      .setRange (-24.0, 6.0);

    aA.setRange (0.001, 5.0); dA.setRange (0.001, 5.0);
    sA.setRange (0.0,   1.0); rA.setRange (0.001, 5.0);

    // Attachments: assert param existence, then bind ---------------------------
    jassert (plugin.morph && plugin.pulseWidth && plugin.cutoff && plugin.resonance && plugin.gain);
    jassert (plugin.aA && plugin.dA && plugin.sA && plugin.rA);
    jassert (plugin.oscAType && plugin.oscBType && plugin.filterType);

    morphA  = std::make_unique<SliderAttachment> (*plugin.morph,      morph);
    pwA     = std::make_unique<SliderAttachment> (*plugin.pulseWidth,  pulseWidth);
    cutoffA = std::make_unique<SliderAttachment> (*plugin.cutoff,      cutoff);
    resoA   = std::make_unique<SliderAttachment> (*plugin.resonance,   resonance);
    gainA   = std::make_unique<SliderAttachment> (*plugin.gain,        gain);

    aAA     = std::make_unique<SliderAttachment> (*plugin.aA, aA);
    dAA     = std::make_unique<SliderAttachment> (*plugin.dA, dA);
    sAA     = std::make_unique<SliderAttachment> (*plugin.sA, sA);
    rAA     = std::make_unique<SliderAttachment> (*plugin.rA, rA);

    oscAAtt   = std::make_unique<ChoiceAttachment> (*plugin.oscAType,   oscASelect);
    oscBAtt   = std::make_unique<ChoiceAttachment> (*plugin.oscBType,   oscBSelect);
    filterAtt = std::make_unique<ChoiceAttachment> (*plugin.filterType, filterSelect);

    // Labels -------------------------------------------------------------------
    addAndMakeVisible (oscALabel);   setupLabel (oscALabel,   "Osc A");
    addAndMakeVisible (oscBLabel);   setupLabel (oscBLabel,   "Osc B");
    addAndMakeVisible (filterLabel); setupLabel (filterLabel, "Filter");

    addAndMakeVisible (morphLabel);  setupLabel (morphLabel,  "Morph");
    addAndMakeVisible (pulseLabel);  setupLabel (pulseLabel,  "Pulse");
    addAndMakeVisible (cutoffLabel); setupLabel (cutoffLabel, "Cutoff");
    addAndMakeVisible (resoLabel);   setupLabel (resoLabel,   "Resonance");
    addAndMakeVisible (gainLabel);   setupLabel (gainLabel,   "Output");

    addAndMakeVisible (aLabel);      setupLabel (aLabel,      "Attack");
    addAndMakeVisible (dLabel);      setupLabel (dLabel,      "Decay");
    addAndMakeVisible (sLabel);      setupLabel (sLabel,      "Sustain");
    addAndMakeVisible (rLabel);      setupLabel (rLabel,      "Release");

    // Pitch --------------------------------------------------------------------
    for (auto* s : { &semi, &fine, &glide }) { setupKnob (*s); addAndMakeVisible (*s); }
    addAndMakeVisible (semiLabel);  setupLabel (semiLabel,  "Semi");
    addAndMakeVisible (fineLabel);  setupLabel (fineLabel,  "Fine");
    addAndMakeVisible (glideLabel); setupLabel (glideLabel, "Glide");

    // Filter env + amount ------------------------------------------------------
    for (auto* s : { &aF, &dF, &sF, &rF, &fAmt }) { setupKnob (*s); addAndMakeVisible (*s); }
    addAndMakeVisible (aFLabel);   setupLabel (aFLabel,   "Filter Attack");
    addAndMakeVisible (dFLabel);   setupLabel (dFLabel,   "Filter Decay");
    addAndMakeVisible (sFLabel);   setupLabel (sFLabel,   "Filter Sustain");
    addAndMakeVisible (rFLabel);   setupLabel (rFLabel,   "Filter Release");
    addAndMakeVisible (fAmtLabel); setupLabel (fAmtLabel, "Envelope Amount");

    // LFO ----------------------------------------------------------------------
    addAndMakeVisible (lfoTarget); addChoices (lfoTarget, { "Off", "Morph", "Pulse", "Cutoff", "Pitch" });
    addAndMakeVisible (lfoTargetLabel); setupLabel (lfoTargetLabel, "LFO Target");
    for (auto* s : { &lfoRate, &lfoDepth }) { setupKnob (*s); addAndMakeVisible (*s); }
    addAndMakeVisible (lfoRateLabel);  setupLabel (lfoRateLabel,  "LFO Rate");
    addAndMakeVisible (lfoDepthLabel); setupLabel (lfoDepthLabel, "LFO Depth");

    // Additional ranges --------------------------------------------------------
    semi.setRange (-24, 24, 1);
    fine.setRange (-100, 100, 1);
    glide.setRange (0, 500, 1);

    aF.setRange (0.001, 5.0); dF.setRange (0.001, 5.0);
    sF.setRange (0.0,   1.0); rF.setRange (0.001, 5.0);
    fAmt.setRange (-1.0, 1.0, 0.01);

    lfoRate.setRange (0.05, 20.0, 0.01);
    lfoDepth.setRange (0.0, 1.0, 0.001);

    // Attachments (pitch, filter env, LFO) -------------------------------------
    semiA   = std::make_unique<SliderAttachment>  (*plugin.semi,   semi);
    fineA   = std::make_unique<SliderAttachment>  (*plugin.fine,   fine);
    glideA  = std::make_unique<SliderAttachment>  (*plugin.glide,  glide);

    aFA     = std::make_unique<SliderAttachment>  (*plugin.aF,     aF);
    dFA     = std::make_unique<SliderAttachment>  (*plugin.dF,     dF);
    sFA     = std::make_unique<SliderAttachment>  (*plugin.sF,     sF);
    rFA     = std::make_unique<SliderAttachment>  (*plugin.rF,     rF);
    fAmtA   = std::make_unique<SliderAttachment>  (*plugin.fEnvAmt, fAmt);

    lfoTargetA = std::make_unique<ChoiceAttachment> (*plugin.lfoTarget, lfoTarget);
    lfoRateA   = std::make_unique<SliderAttachment>  (*plugin.lfoRate,   lfoRate);
    lfoDepthA  = std::make_unique<SliderAttachment>  (*plugin.lfoDepth,  lfoDepth);
}

MorphSynthView::~MorphSynthView()
{
    setLookAndFeel (nullptr);
}

//==============================================================================
// Layout
//==============================================================================

void MorphSynthView::resized()
{
    auto r = getLocalBounds().reduced (10);

    // --- layout constants (tuned for clarity & non-overlap)
    const int selectorRowH = 30;  // tall enough for combo boxes
    const int rowGap       = 8;   // vertical gap between rows
    const int labelH       = 18;  // label height under each knob
    const int knobPad      = 6;   // padding inside each knob cell

    // We have 6 "knob rows" (Tone, Amp ADSR, Pitch, Filter ADSR, LFO, Output)
    const int knobRowCount = 6;

    // Compute a row height that includes knob + label and keeps things readable
    const int remainingH   = juce::jmax (0, r.getHeight() - selectorRowH - rowGap - (rowGap * (knobRowCount - 1)));
    const int knobRowH     = juce::jlimit (120, 190, remainingH / knobRowCount);

    // Helper to place a knob and keep its label within the same row cell
    auto placeKnob = [labelH, knobPad] (juce::Component& knob, juce::Label& label, juce::Rectangle<int> cell)
    {
        auto k = cell.reduced (knobPad);

        // Reserve space for the label at the bottom of the cell
        const int kHeight = juce::jmax (40, k.getHeight() - labelH - 4);
        auto kArea  = k.removeFromTop (kHeight);
        knob.setBounds (kArea);

        // Label anchored inside the same cell to avoid spilling into next row
        label.setBounds (k.getX(), kArea.getBottom() + 2, k.getWidth(), labelH);
    };

    // Row 0: selector labels + boxes ------------------------------------------
    {
        auto row = r.removeFromTop (selectorRowH);
        auto w = row.getWidth();

        auto c1 = row.removeFromLeft (w / 3);
        auto c2 = row.removeFromLeft ((w - w/3) / 2);
        auto c3 = row;

        oscALabel .setBounds (c1.removeFromLeft (60));
        oscASelect.setBounds (c1.reduced (4));

        oscBLabel .setBounds (c2.removeFromLeft (60));
        oscBSelect.setBounds (c2.reduced (4));

        filterLabel .setBounds (c3.removeFromLeft (60));
        filterSelect.setBounds (c3.reduced (4));
    }

    r.removeFromTop (rowGap);

    // Row 1: Morph / Pulse / Cutoff / Reso ------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        const int w = row.getWidth();
        auto k1 = row.removeFromLeft (w / 4);
        auto k2 = row.removeFromLeft (w / 4);
        auto k3 = row.removeFromLeft (w / 4);
        auto k4 = row;

        placeKnob (morph,      morphLabel,   k1);
        placeKnob (pulseWidth, pulseLabel,   k2);
        placeKnob (cutoff,     cutoffLabel,  k3);
        placeKnob (resonance,  resoLabel,    k4);
    }

    r.removeFromTop (rowGap);

    // Row 2: Amp ADSR ----------------------------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        const int w = row.getWidth();
        auto a = row.removeFromLeft (w / 4);
        auto d = row.removeFromLeft (w / 4);
        auto s = row.removeFromLeft (w / 4);
        auto rr= row;

        placeKnob (aA, aLabel, a);
        placeKnob (dA, dLabel, d);
        placeKnob (sA, sLabel, s);
        placeKnob (rA, rLabel, rr);
    }

    r.removeFromTop (rowGap);

    // Row 3: Pitch (Semi / Fine / Glide) --------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        const int w = row.getWidth();
        auto ps1 = row.removeFromLeft (w / 3);
        auto ps2 = row.removeFromLeft (w / 3);
        auto ps3 = row;

        placeKnob (semi,  semiLabel,  ps1);
        placeKnob (fine,  fineLabel,  ps2);
        placeKnob (glide, glideLabel, ps3);
    }

    r.removeFromTop (rowGap);

    // Row 4: Filter ADSR + Env Amt --------------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        const int w = row.getWidth();
        auto fa = row.removeFromLeft (w / 5);
        auto fd = row.removeFromLeft (w / 5);
        auto fs = row.removeFromLeft (w / 5);
        auto fr = row.removeFromLeft (w / 5);
        auto fE = row;

        placeKnob (aF,   aFLabel,   fa);
        placeKnob (dF,   dFLabel,   fd);
        placeKnob (sF,   sFLabel,   fs);
        placeKnob (rF,   rFLabel,   fr);
        placeKnob (fAmt, fAmtLabel, fE);
    }

    r.removeFromTop (rowGap);

    // Row 5: LFO (Target / Rate / Depth) --------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        const int w = row.getWidth();

        // Left third: label + combo; keep both inside the row
        auto left = row.removeFromLeft (w / 3);
        {
            auto l = left.reduced (4);
            auto title = l.removeFromTop (labelH);
            lfoTargetLabel.setBounds (title);
            lfoTarget.setBounds (l); // remaining area under the label
        }

        // Middle third: Rate
        auto mid = row.removeFromLeft (w / 3);
        placeKnob (lfoRate,  lfoRateLabel,  mid);

        // Right third: Depth
        auto right = row;
        placeKnob (lfoDepth, lfoDepthLabel, right);
    }

    r.removeFromTop (rowGap);

    // Row 6: Output (Gain) -----------------------------------------------------
    {
        auto row = r.removeFromTop (knobRowH);
        placeKnob (gain, gainLabel, row);
    }
}