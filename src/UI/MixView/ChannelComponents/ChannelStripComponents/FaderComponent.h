#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief Custom LookAndFeel for vertical volume faders.
 *
 * Only overrides drawLinearSlider and keeps all logic inside ChannelStrip
 * unchanged. Used to give the mixer fader a DAW-style rail + handle.
 */
class FaderComponent : public juce::LookAndFeel_V4
{
public:
    /**
     * @brief Draws a simple DAW-style vertical fader rail and handle.
     *
     * @param sliderPos The Y position of the fader handle (already scaled)
     * @param s         Reference to the owning slider
     */
    void drawLinearSlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPos,
                           float, float,
                           const juce::Slider::SliderStyle,
                           juce::Slider& s) override
    {
        juce::ignoreUnused (s);

        auto r = juce::Rectangle<float> ((float) x, (float) y,
                                         (float) width, (float) height);

        // Rail
        auto railW = juce::jmin (6.0f, r.getWidth() * 0.12f);
        auto railX = r.getCentreX() - railW * 0.5f;
        auto rail  = juce::Rectangle<float> (railX,
                                             r.getY() + 8.0f,
                                             railW,
                                             r.getHeight() - 16.0f);

        g.setColour (juce::Colour (0xFF343A40));
        g.fillRect (rail);

        // Handle
        const float handleH = juce::jlimit (28.0f, 60.0f, r.getHeight() * 0.08f);
        const float handleW = juce::jlimit (18.0f, 28.0f, r.getWidth()  * 0.35f);

        auto cx = r.getCentreX();
        auto hy = sliderPos - handleH * 0.5f;
        juce::Rectangle<float> handle (cx - handleW * 0.5f,
                                       hy,
                                       handleW,
                                       handleH);

        g.setColour (juce::Colour (0xFFADB5BD));
        g.fillRect (handle);

        g.setColour (juce::Colours::black.withAlpha (0.6f));
        g.drawRect (handle, 1.0f);
    }
};

