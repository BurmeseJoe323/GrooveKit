#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class FaderComponent : public juce::LookAndFeel_V4
{
public:
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                          float sliderPos, float, float,
                          const juce::Slider::SliderStyle, juce::Slider& s) override
    {
        auto r = juce::Rectangle<float>((float) x, (float) y, (float) width, (float) height);

        auto railW = juce::jmin(6.0f, r.getWidth() * 0.12f);
        auto railX = r.getCentreX() - railW * 0.5f;
        auto rail = juce::Rectangle<float>(railX, r.getY()+8.0f, railW, r.getHeight()-16.0f);

        // Darker rail for visibility against new dark background (Written by Claude Code)
        g.setColour(juce::Colour(0xFF343A40));
        g.fillRect(rail);

        const float handleH = juce::jlimit(28.0f, 60.0f, r.getHeight() * 0.08f);
        const float handleW = juce::jlimit(18.0f, 28.0f, r.getWidth()  * 0.35f);

        auto cx = r.getCentreX();
        auto hy = sliderPos - handleH * 0.5f;
        juce::Rectangle<float> handle(cx - handleW * 0.5f, hy, handleW, handleH);

        // Lighter handle for better visibility (Written by Claude Code)
        g.setColour(juce::Colour(0xFFADB5BD));
        g.fillRect(handle);

        // subtle inner border
        g.setColour(juce::Colours::black.withAlpha(0.6f));
        g.drawRect(handle, 1.0f);
    }
};
