#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>
#include <array>

#include "DrumPadComponent.h"
#include "DrumSamplerEngine.h"
#include "SampleLibraryComponent.h"

/**
 * @brief Main UI component for the Drum Sampler view.
 *
 * This component contains:
 *   - A title header
 *   - Volume control
 *   - ADSR envelope sliders
 *   - A sample library browser (left pane)
 *   - A 4x4 grid of DrumPadComponents
 *   - Per-pad "Load" buttons for importing samples
 *
 * Responsibilities:
 *   - Reflect the current DrumSamplerEngine state
 *   - Forward UI interactions to the engine (loading samples, triggering pads, etc.)
 *   - Manage layout of all UI elements (pads, sliders, labels, library)
 *
 * This class **does not** perform audio processing; it delegates to DrumSamplerEngine.
 */
class DrumSamplerComponent : public juce::Component,
                             public juce::DragAndDropContainer
{
public:
    /**
     * @brief Creates a DrumSamplerComponent bound to a DrumSamplerEngine.
     *
     * @param engineRef Reference to the engine implementation backing this UI.
     */
    explicit DrumSamplerComponent (DrumSamplerEngine& engineRef)
        : engine (engineRef)
    {
        //==========================================================================
        // Title label
        addAndMakeVisible (titleLabel);
        titleLabel.setText ("Drum Sampler", juce::dontSendNotification);
        titleLabel.setJustificationType (juce::Justification::centred);

        {
            juce::Font f (juce::FontOptions (18.0f));
            f.setBold (true);
            titleLabel.setFont (f);
        }

        //==========================================================================
        // Volume
        addAndMakeVisible (volumeLabel);
        volumeLabel.setText ("Volume", juce::dontSendNotification);

        addAndMakeVisible (volume);
        volume.setRange (0.0, 1.0, 0.001);
        volume.setValue (0.8);
        volume.onValueChange = [this]
        {
            engine.setVolume ((float) volume.getValue());
        };

        //==========================================================================
        // ADSR
        addAndMakeVisible (adsrLabel);
        adsrLabel.setText ("ADSR", juce::dontSendNotification);

        setupSlider (attack,  "A", 0.001, 2.0, 0.01, 0.01f);
        setupSlider (decay,   "D", 0.001, 2.0, 0.01, 0.10f);
        setupSlider (sustain, "S", 0.0,   1.0, 0.001, 0.80f);
        setupSlider (release, "R", 0.001, 3.0, 0.01, 0.20f);

        auto updateADSR = [this]
        {
            engine.setADSR ((float) attack.getValue(),
                            (float) decay.getValue(),
                            (float) sustain.getValue(),
                            (float) release.getValue());
        };

        attack .onValueChange = updateADSR;
        decay  .onValueChange = updateADSR;
        sustain.onValueChange = updateADSR;
        release.onValueChange = updateADSR;

        //==========================================================================
        // Sample library panel
        sampleLibrary = std::make_unique<SampleLibraryComponent>();
        addAndMakeVisible (*sampleLibrary);

        //==========================================================================
        // Create 16 drum pads
        for (int i = 0; i < 16; ++i)
        {
            pads.add (std::make_unique<DrumPadComponent>(
                i,
                // On sample drop
                [this, i](const juce::File& f)
                {
                    engine.loadSampleIntoSlot (i, f);
                    pads[i]->setTitle (f.getFileNameWithoutExtension());
                    if (sampleLibrary)
                        sampleLibrary->addFile (f);
                },
                // On trigger
                [this, i](float v)
                {
                    engine.triggerSlot (i, v);
                }
            ));

            addAndMakeVisible (pads.getLast());
        }

        //==========================================================================
        // Per-pad file load buttons
        for (int i = 0; i < 16; ++i)
        {
            auto& btn = *loadButtons.add (new juce::TextButton ("Load"));
            addAndMakeVisible (btn);

            btn.onClick = [this, i]
            {
                auto chooser = std::make_shared<juce::FileChooser>(
                    "Choose a sample",
                    juce::File{},
                    "*.wav;*.aif;*.aiff;*.flac;*.mp3");

                chooser->launchAsync (
                    juce::FileBrowserComponent::openMode
                        | juce::FileBrowserComponent::canSelectFiles,
                    [this, i, chooser](const juce::FileChooser& fc)
                    {
                        auto f = fc.getResult();
                        if (f.existsAsFile())
                        {
                            engine.loadSampleIntoSlot (i, f);
                            pads[i]->setTitle (f.getFileNameWithoutExtension());
                            if (sampleLibrary)
                                sampleLibrary->addFile (f);
                        }
                    });
            };
        }

        //==========================================================================
        // Initialize pad labels from the engine
        for (int i = 0; i < 16; ++i)
            pads[i]->setTitle (engine.getSlotName (i));
    }

    //==============================================================================
    /** Lays out all UI components. */
    void resized() override
    {
        auto r = getLocalBounds().reduced (0);

        // Header
        auto header = r.removeFromTop (36);
        titleLabel.setBounds (header);

        // Volume + ADSR row
        auto controls = r.removeFromTop (70);

        auto volArea = controls.removeFromLeft (220);
        volumeLabel.setBounds (volArea.removeFromTop (20));
        volume.setBounds (volArea.reduced (0, 4));

        auto adsrArea = controls;
        adsrLabel.setBounds (adsrArea.removeFromTop (20));

        auto each = adsrArea;
        int w = each.getWidth() / 4;

        attack .setBounds (each.removeFromLeft (w).reduced (4));
        decay  .setBounds (each.removeFromLeft (w).reduced (4));
        sustain.setBounds (each.removeFromLeft (w).reduced (4));
        release.setBounds (each.removeFromLeft (w).reduced (4));

        r.removeFromTop (8);

        // Sample library on left
        auto left = r.removeFromLeft (juce::jmax (180, r.getWidth() / 5));
        sampleLibrary->setBounds (left);

        // 4x4 pad grid
        juce::Grid grid;
        grid.autoRows    = juce::Grid::TrackInfo (juce::Grid::Fr (1));
        grid.autoColumns = juce::Grid::TrackInfo (juce::Grid::Fr (1));
        grid.rowGap      = juce::Grid::Px (8);
        grid.columnGap   = juce::Grid::Px (8);

        std::array<juce::Component*, 16> padPtrs {};
        for (int i = 0; i < 16; ++i)
            padPtrs[i] = pads[i];

        grid.templateColumns =
        {
            juce::Grid::TrackInfo (juce::Grid::Fr (1)),
            juce::Grid::TrackInfo (juce::Grid::Fr (1)),
            juce::Grid::TrackInfo (juce::Grid::Fr (1)),
            juce::Grid::TrackInfo (juce::Grid::Fr (1))
        };

        grid.templateRows = grid.templateColumns;

        juce::Array<juce::GridItem> items;
        for (auto* p : padPtrs)
            items.add (juce::GridItem (*p));

        grid.items = std::move (items);

        auto right = r;
        auto padsArea = right.removeFromTop (right.proportionOfHeight (0.72f));
        auto side = juce::jmin (padsArea.getWidth(), padsArea.getHeight());

        padsArea = juce::Rectangle<int> (padsArea.getX(), padsArea.getY(), side, side)
                       .withCentre (padsArea.getCentre());

        grid.performLayout (padsArea);
    }

private:
    //==============================================================================
    DrumSamplerEngine& engine;

    juce::Label  titleLabel;
    juce::Label  volumeLabel;
    juce::Label  adsrLabel;

    juce::Slider volume;
    juce::Slider attack, decay, sustain, release;

    std::unique_ptr<SampleLibraryComponent> sampleLibrary;

    juce::OwnedArray<DrumPadComponent> pads;
    juce::OwnedArray<juce::TextButton> loadButtons;

    /**
     * @brief Helper to configure an ADSR slider.
     */
    static void setupSlider (juce::Slider& s, const juce::String& name,
                             double min, double max, double step, double init)
    {
        s.setSliderStyle (juce::Slider::LinearBar);
        s.setTextBoxStyle (juce::Slider::TextBoxRight, false, 60, 20);
        s.setRange (min, max, step);
        s.setDoubleClickReturnValue (true, init);
        s.setName (name);
        s.setValue (init, juce::dontSendNotification);
        s.setNumDecimalPlacesToDisplay (2);
    }
};