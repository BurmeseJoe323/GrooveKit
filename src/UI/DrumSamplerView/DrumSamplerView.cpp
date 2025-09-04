#include "DrumSamplerView.h"
#include <juce_data_structures/juce_data_structures.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <BinaryData.h>   // from juce_add_binary_data (GrooveKitResources)

using namespace juce;

static String getBinaryDataAsString(const void* data, int size)
{
    return String::fromUTF8(static_cast<const char*>(data), size);
}

DrumSamplerView::DrumSamplerView(AudioEngine& a, MIDIEngine& m)
    : audio(a), midi(m)
{
    // 1) Load XML & JSON from BinaryData
    auto layoutXmlStr = getBinaryDataAsString(BinaryData::drum_sampler_layout_xml,
                                              BinaryData::drum_sampler_layout_xmlSize);
    auto styleJsonStr = getBinaryDataAsString(BinaryData::drum_sampler_styles_json,
                                              BinaryData::drum_sampler_styles_jsonSize);

    std::unique_ptr<XmlElement> layoutXml = XmlDocument::parse(layoutXmlStr);
    jassert(layoutXml != nullptr);

    layoutTree = ValueTree::fromXml(*layoutXml);
    styleJson  = JSON::parse(styleJsonStr);
    layoutTree.setProperty("style", styleJson, nullptr);

    // 2) Build the GUI using JIVE's public API
    jive::Interpreter interpreter;
    root = interpreter.interpret(layoutTree);            // returns std::unique_ptr<jive::GuiItem>
    addAndMakeVisible(root->getComponent().get());             // show underlying JUCE Component

    // 3) Wire events and parameters
    wirePads();
    wireControls();
}

void DrumSamplerView::wirePads()
{
    int note = 36; // C1
    auto linkPad = [this](const String& id, int midiNote)
    {
        if (auto* comp = root->getComponent()->findChildWithID(id))
            if (auto* btn = dynamic_cast<Button*>(comp))
                btn->onClick = [this, btn, midiNote]
                {
                    // midi.triggerNoteOn(midiNote, 100); // TODO: hook up your engine
                    btn->setToggleState(true, dontSendNotification);
                    Timer::callAfterDelay(90, [btn]{ btn->setToggleState(false, dontSendNotification); });
                };
    };

    for (int r = 0; r < 4; ++r)
        for (int c = 0; c < 4; ++c)
            linkPad("pad" + String(r) + String(c), note++);
}

void DrumSamplerView::wireControls()
{
    // Sample loader
    if (auto* comp = root->getComponent()->findChildWithID("loadSampleBtn"))
        if (auto* btn = dynamic_cast<Button*>(comp))
            btn->onClick = [this]
            {
                // audio.showSampleOpenDialogForSelectedPad(); // TODO
            };

    // APVTS attachments (replace with your real attachment wrapper & storage)
    auto attach = [this](const String& widgetId, const String& paramID)
    {
        if (auto* comp = root->getComponent()->findChildWithID(widgetId))
            if (auto* slider = dynamic_cast<Slider*>(comp))
            {
                // attachments.emplace_back(std::make_unique<AudioProcessorValueTreeState::SliderAttachment>(apvts, paramID, *slider));
            }
    };

    attach("attack",        "drum.attack");
    attach("decay",         "drum.decay");
    attach("sustain",       "drum.sustain");
    attach("release",       "drum.release");
    attach("tune",          "drum.tune");
    attach("filterCutoff",  "drum.cutoff");
    attach("volume",        "drum.volume");
}

void DrumSamplerView::resized()
{
    if (root) root->getComponent()->setBounds(getLocalBounds());
}
