#pragma once
#include "MorphLookAndFeel.h"
#include "MorphSynthPlugin.h"
#include <juce_gui_extra/juce_gui_extra.h>
#include <tracktion_engine/tracktion_engine.h>

// ==============================================================================
// MorphSynthView.h
// ------------------------------------------------------------------------------
// Editor component for the Morph Synth plugin.
//
// Responsibilities:
//  - Presents controls (selectors, knobs, labels) and lays them out.
//  - Binds UI widgets to te::AutomatableParameter via lightweight attachments.
//  - Applies a custom LookAndFeel for consistent styling.
//
// Notes:
//  - NO logic changes in this pass (comments & structure only).
// ==============================================================================

namespace te = tracktion::engine;

/**
 * @brief UI component for the Morph Synth instrument.
 *
 * This view owns JUCE controls (sliders, combo boxes) and synchronises them
 * with Tracktion Engine parameters using small attachment helpers defined
 * inside the class. Layout is computed in resized().
 */
class MorphSynthView : public juce::Component
{
public:
    //==============================================================================
    // Lifecycle
    //------------------------------------------------------------------------------

    /** Construct the view and bind to a MorphSynthPlugin. */
    explicit MorphSynthView (MorphSynthPlugin& p);

    /** Ensure LookAndFeel is reset on teardown. */
    ~MorphSynthView() override;

    //==============================================================================
    // juce::Component
    //------------------------------------------------------------------------------

    /** Lay out controls. (Adaptive sizing handled in the .cpp implementation.) */
    void resized() override;

private:
    //==============================================================================
    // Attachments: Slider <-> AutomatableParameter
    //------------------------------------------------------------------------------
    /**
     * @brief Simple two-way binding between a Slider and an AutomatableParameter.
     *
     * - Listens to param and updates the slider without feedback loops.
     * - Listens to slider and writes to the param with sendNotificationSync.
     */
    class SliderAttachment  : private te::AutomatableParameter::Listener,
                              private juce::Slider::Listener
    {
    public:
        SliderAttachment (te::AutomatableParameter& paramIn, juce::Slider& sliderIn)
            : param (paramIn), slider (sliderIn)
        {
            slider.addListener (this);
            param.addListener (this);
            // Initial sync from param -> slider
            const auto v = param.getCurrentValue();
            slider.setValue (v, juce::dontSendNotification);
        }

        ~SliderAttachment() override
        {
            param.removeListener (this);
            slider.removeListener (this);
        }

    private:
        // te::AutomatableParameter::Listener
        void parameterChanged (te::AutomatableParameter& /*p*/, float newValue) override
        {
            const double current = slider.getValue();
            if (std::abs (current - (double) newValue) > 1.0e-6)
                slider.setValue (newValue, juce::dontSendNotification);
        }
        void curveHasChanged (te::AutomatableParameter&) override {}

        // juce::Slider::Listener
        void sliderValueChanged (juce::Slider* s) override
        {
            if (s == &slider)
            {
                // Your TE branch usually has setParameter; if not, use setCurrentValue
                param.setParameter ((float) slider.getValue(), juce::sendNotificationSync);
                // param.setCurrentValue ((float) slider.getValue());
            }
        }

        te::AutomatableParameter& param;
        juce::Slider&             slider;
    };

    //==============================================================================
    // Attachments: ComboBox <-> AutomatableParameter
    //------------------------------------------------------------------------------
    /**
     * @brief Two-way binding for discrete/choice parameters.
     *
     * Keeps the combo box and parameter index in sync, avoiding feedback loops.
     */
    class ChoiceAttachment : private te::AutomatableParameter::Listener,
                             private juce::ComboBox::Listener
    {
    public:
        ChoiceAttachment (te::AutomatableParameter& paramIn, juce::ComboBox& boxIn)
            : param (paramIn), box (boxIn)
        {
            box.addListener (this);
            param.addListener (this);
            box.setSelectedItemIndex ((int) std::round (param.getCurrentValue()),
                                      juce::dontSendNotification);
        }

        ~ChoiceAttachment() override
        {
            param.removeListener (this);
            box.removeListener (this);
        }

    private:
        // te::AutomatableParameter::Listener
        void parameterChanged (te::AutomatableParameter& /*p*/, float newValue) override
        {
            const int idx = (int) std::round (newValue);
            if (box.getSelectedItemIndex() != idx)
                box.setSelectedItemIndex (idx, juce::dontSendNotification);
        }
        void curveHasChanged (te::AutomatableParameter&) override {}

        // juce::ComboBox::Listener
        void comboBoxChanged (juce::ComboBox* cb) override
        {
            if (cb == &box)
                param.setParameter ((float) box.getSelectedItemIndex(), juce::sendNotificationSync);
                // param.setCurrentValue ((float) box.getSelectedItemIndex());
        }

        te::AutomatableParameter& param;
        juce::ComboBox&           box;
    };

    //==============================================================================
    // UI widgets
    //------------------------------------------------------------------------------

    MorphSynthPlugin& plugin;  ///< Reference to the owning plugin

    // --- selectors
    juce::ComboBox oscASelect, oscBSelect, filterSelect;
    juce::Label    oscALabel,  oscBLabel,  filterLabel;

    // --- tone & amp
    juce::Slider morph, pulseWidth, cutoff, resonance, gain;
    juce::Slider aA, dA, sA, rA;
    juce::Label  morphLabel, pulseLabel, cutoffLabel, resoLabel, gainLabel;
    juce::Label  aLabel, dLabel, sLabel, rLabel;

    // --- pitch
    juce::Slider semi, fine, glide;      juce::Label semiLabel, fineLabel, glideLabel;

    // --- filter env + amount
    juce::Slider aF, dF, sF, rF, fAmt;   juce::Label aFLabel, dFLabel, sFLabel, rFLabel, fAmtLabel;

    // --- LFO
    juce::ComboBox lfoTarget;            juce::Label lfoTargetLabel;
    juce::Slider   lfoRate, lfoDepth;    juce::Label lfoRateLabel, lfoDepthLabel;

    //==============================================================================
    // Attachments (UI <-> parameters)
    //------------------------------------------------------------------------------

    std::unique_ptr<SliderAttachment>  morphA, pwA, cutoffA, resoA, gainA;
    std::unique_ptr<SliderAttachment>  aAA, dAA, sAA, rAA;
    std::unique_ptr<ChoiceAttachment>  oscAAtt, oscBAtt, filterAtt;
    std::unique_ptr<SliderAttachment>  semiA, fineA, glideA;
    std::unique_ptr<SliderAttachment>  aFA, dFA, sFA, rFA, fAmtA;
    std::unique_ptr<ChoiceAttachment>  lfoTargetA;
    std::unique_ptr<SliderAttachment>  lfoRateA, lfoDepthA;

    //==============================================================================
    // UI helpers
    //------------------------------------------------------------------------------

    /** Apply standard rotary style to a knob. */
    static void setupKnob (juce::Slider& s);

    /** Apply standard label style and set text. */
    static void setupLabel (juce::Label& l, const juce::String& text);

    //==============================================================================
    // Look & Feel
    //------------------------------------------------------------------------------

    std::unique_ptr<MorphLookAndFeel> lnf;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MorphSynthView)
};