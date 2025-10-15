#pragma once
/**
    FourOscLookAndFeel
    -------------------
    Custom LookAndFeel for FourOSC-style controls.

    - Rotary slider: dark body, subtle rim, value ring with a gap at the bottom,
      and a simple needle pointer. Designed to be used with a 300° sweep
      (start=120°, end=420°) as configured on your Knob sliders.

    - Combo box: rounded dark box with a minimal down arrow; the text itself is
      drawn by JUCE’s internal label (via positionComboBoxText / getComboBoxFont).
*/

#include <juce_gui_basics/juce_gui_basics.h>

class FourOscLookAndFeel : public juce::LookAndFeel_V4
{
public:
    FourOscLookAndFeel()
    {
        using C = juce::Colour;

        // Palette
        knobBase      = C::fromRGB (38,  42,  49);   // knob face
        knobEdge      = C::fromRGB (22,  25,  30);   // outer rim / shadow
        ringColour    = C::fromRGB (80,  92, 110);   // track ring
        pointerColour = C::fromRGB (220, 230, 240);  // pointer + value arc
        textColour    = juce::Colours::white.withAlpha (0.90f);

        // Slider text box styling (used when TextBoxBelow is enabled)
        setColour (juce::Slider::textBoxTextColourId,     textColour);
        setColour (juce::Slider::textBoxOutlineColourId,  juce::Colours::transparentBlack);
    }

    //==========================================================================
    // Rotary knob drawing
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPosProportional, float rotaryStartAngle,
                           float rotaryEndAngle, juce::Slider&) override
    {
        const auto bounds   = juce::Rectangle<float> (x, y, w, h).reduced (4.0f);
        const float diameter = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto  centre   = bounds.getCentre();
        const float r        = diameter * 0.5f;

        // --- Body & rim ---
        g.setColour (knobEdge);
        g.fillEllipse (bounds.withSizeKeepingCentre (diameter, diameter));
        g.setColour (knobBase);
        g.fillEllipse (bounds.withSizeKeepingCentre (diameter - 3.0f, diameter - 3.0f));

        // --- Value ring (gap at bottom) ---
        const float sweep      = rotaryEndAngle - rotaryStartAngle;
        const float trackStart = rotaryStartAngle + juce::MathConstants<float>::halfPi; // +90° => opening downwards
        const float trackEnd   = trackStart + sweep;

        const float ringWidth = juce::jlimit (1.5f, 3.0f, r * 0.13f);
        const float rr        = r - 6.0f;

        // background track
        juce::Path bg;
        bg.addCentredArc (centre.x, centre.y, rr, rr, 0.0f, trackStart, trackEnd, true);
        g.setColour (ringColour.withAlpha (0.35f));
        g.strokePath (bg, juce::PathStrokeType (ringWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // foreground (value) arc
        juce::Path fg;
        fg.addCentredArc (centre.x, centre.y, rr, rr, 0.0f,
                          trackStart, trackStart + sweep * sliderPosProportional, true);
        g.setColour (pointerColour.withAlpha (0.90f));
        g.strokePath (fg, juce::PathStrokeType (ringWidth, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

        // --- Needle pointer ---
        const float angle     = rotaryStartAngle + sliderPosProportional * sweep;
        const float pointerR  = r * 0.62f;                // pointer length
        const auto  tip       = centre + juce::Point<float> (std::cos (angle), std::sin (angle)) * pointerR;

        juce::Path needle;
        needle.startNewSubPath (centre);
        needle.lineTo (tip);

        g.setColour (pointerColour);
        g.strokePath (needle, juce::PathStrokeType (2.5f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        g.fillEllipse (tip.x - 2.0f, tip.y - 2.0f, 4.0f, 4.0f);   // tip cap
        g.fillEllipse (centre.x - 3.0f, centre.y - 3.0f, 6.0f, 6.0f); // hub
    }

    //==========================================================================
    // Minimal combo box chrome (text drawn by JUCE’s label)
    void drawComboBox (juce::Graphics& g, int w, int h, bool,
                       int, int, int, int, juce::ComboBox&) override
    {
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.0f);

        g.setColour (juce::Colour (0xff24272b));  // fill
        g.fillRoundedRectangle (r, 6.0f);

        g.setColour (juce::Colour (0x44ffffff));  // outline
        g.drawRoundedRectangle (r, 6.0f, 1.0f);

        // down-arrow
        juce::Path p;
        const float cx = (float) w - 12.0f, cy = h * 0.5f;
        p.addTriangle (cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 3);
        g.setColour (juce::Colours::white.withAlpha (0.85f));
        g.fillPath (p);
    }

    // Place JUCE’s internal text label inside the box
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (box.getLocalBounds().reduced (8, 0));
        label.setJustificationType (juce::Justification::centredLeft);
        label.setColour (juce::Label::textColourId, textColour);
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (14.0f);
    }

private:
    // palette
    juce::Colour knobBase{}, knobEdge{}, ringColour{}, pointerColour{}, textColour{};
};