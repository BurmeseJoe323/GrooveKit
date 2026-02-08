// ==============================================================================
// MorphLookAndFeel.h
// ------------------------------------------------------------------------------
// A compact custom LookAndFeel for the Morph Synth UI.
//
// - Styles rotary sliders with a value ring and pointer.
// - Styles combo boxes with rounded rectangles and a custom arrow.
// - Provides small helpers to set up knobs/combos consistently.
//
// Ownership: The editor creates one instance and sets it via setLookAndFeel.
// Usage    : Keep one L&F instance alive as long as any components need it.
// ==============================================================================

#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief Custom LookAndFeel for Morph Synth.
 *
 * Responsibilities:
 *  - Draw rotary sliders with a rotated arc (gap at bottom) and a pointer.
 *  - Draw combo boxes with a dark rounded background and a chevron arrow.
 *  - Provide small convenience functions for consistent component setup.
 */
class MorphLookAndFeel : public juce::LookAndFeel_V4
{
public:
    //==============================================================================
    /** Creates a LookAndFeel with Morph's color palette and defaults. */
    MorphLookAndFeel()
    {
        using C = juce::Colour;

        textColour    = juce::Colours::white.withAlpha (0.92f);
        knobBase      = C::fromRGB (33, 36, 44);
        knobEdge      = C::fromRGB (18, 20, 25);
        ringColour    = C::fromRGB (70, 85, 110);
        pointerColour = C::fromRGB (240, 245, 255);

        setColour (juce::Slider::textBoxTextColourId,     textColour);
        setColour (juce::Slider::textBoxOutlineColourId,  juce::Colours::transparentBlack);
        setColour (juce::Label::textColourId,             textColour);
        setColour (juce::ComboBox::textColourId,          textColour);
    }

    /** Virtual destructor. */
    ~MorphLookAndFeel() override = default;

    //==============================================================================
    // Public helpers (non-virtual)
    //------------------------------------------------------------------------------

    /**
     * @brief Apply Morph’s rotary style to a slider.
     *
     * - Rotary drag
     * - Text box below, fixed width/height
     */
    void styleKnob (juce::Slider& s) const
    {
        s.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        s.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 60, 18);
    }

    /**
     * @brief Apply Morph’s style to a combo box.
     *
     * Sets background/outline/text/arrow colours for a cohesive look.
     */
    void styleCombo (juce::ComboBox& c) const
    {
        c.setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xff2a2e36));
        c.setColour (juce::ComboBox::outlineColourId,    juce::Colours::transparentBlack);
        c.setColour (juce::ComboBox::textColourId,       textColour);
        c.setColour (juce::ComboBox::arrowColourId,      textColour);
    }

    //==============================================================================
    // LookAndFeel overrides (drawing & layout)
    //------------------------------------------------------------------------------

    /**
     * @brief Draw a rotary slider with an outer track, value arc, face, and pointer.
     *
     * Angle sweep: 135° → 405° for the pointer.
     * The arc (ring) is rotated by +90° so the gap sits at the bottom.
     */
    void drawRotarySlider (juce::Graphics& g, int x, int y, int w, int h,
                           float sliderPosProportional,
                           float /*startAngle*/, float /*endAngle*/,
                           juce::Slider& /*s*/) override
    {
        // Keep drawing bounds square
        const int d  = std::min (w, h);
        const int cx = x + (w - d) / 2;
        const int cy = y + (h - d) / 2;

        auto bounds = juce::Rectangle<float> ((float) cx, (float) cy, (float) d, (float) d).reduced (6.0f);
        const auto centre = bounds.getCentre();
        const float radius = std::min (bounds.getWidth(), bounds.getHeight()) * 0.5f;

        // Pointer sweep (visual)
        const float start = juce::degreesToRadians (135.0f);
        const float end   = juce::degreesToRadians (405.0f);
        const float angle = juce::jmap (sliderPosProportional, 0.0f, 1.0f, start, end);

        // Ring sweep rotated by +90° so the gap is at the bottom
        const float ringStart = start + juce::MathConstants<float>::halfPi;
        const float ringEnd   = end   + juce::MathConstants<float>::halfPi;
        const float ringAngle = juce::jmap (sliderPosProportional, 0.0f, 1.0f, ringStart, ringEnd);

        // Background ring
        {
            juce::Path track;
            track.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, ringStart, ringEnd, true);
            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.strokePath (track, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Value ring
        {
            juce::Path valuePath;
            valuePath.addCentredArc (centre.x, centre.y, radius, radius, 0.0f, ringStart, ringAngle, true);
            g.setColour (juce::Colours::white.withAlpha (0.9f));
            g.strokePath (valuePath, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));
        }

        // Knob face
        g.setColour (juce::Colour (0xff21252b));
        g.fillEllipse (bounds);
        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawEllipse (bounds, 1.5f);

        // Pointer
        const float pointerLen = radius * 0.55f;
        const juce::Point<float> tip  { centre.x + std::cos (angle) * pointerLen,
                                        centre.y + std::sin (angle) * pointerLen };
        g.setColour (juce::Colours::white);
        g.drawLine (juce::Line<float> (centre, tip), 3.0f);
        g.fillEllipse (tip.x - 3.0f, tip.y - 3.0f, 6.0f, 6.0f);
    }

    /**
     * @brief Draw a rounded, dark combo box with a chevron arrow.
     */
    void drawComboBox (juce::Graphics& g, int w, int h, bool /*isButtonDown*/,
                       int, int, int, int, juce::ComboBox&) override
    {
        auto r = juce::Rectangle<float> (0, 0, (float) w, (float) h).reduced (1.0f);
        g.setColour (juce::Colour (0xff1e2127));  g.fillRoundedRectangle (r, 6.0f);
        g.setColour (juce::Colour (0x44ffffff));  g.drawRoundedRectangle (r, 6.0f, 1.0f);

        // Down arrow
        juce::Path p; const float cx = (float) w - 12.0f, cy = h * 0.5f;
        p.addTriangle (cx - 4, cy - 2, cx + 4, cy - 2, cx, cy + 3);
        g.setColour (textColour);
        g.fillPath (p);
    }

    /**
     * @brief Position the editable label inside a combo box.
     */
    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (box.getLocalBounds().reduced (8, 0));
        label.setJustificationType (juce::Justification::centredLeft);
        label.setColour (juce::Label::textColourId, textColour);
    }

    /**
     * @brief Font used for combo box text.
     */
    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (juce::FontOptions (14.0f));
    }

private:
    //==============================================================================
    // Palette
    juce::Colour textColour {}, knobBase {}, knobEdge {}, ringColour {}, pointerColour {};
};