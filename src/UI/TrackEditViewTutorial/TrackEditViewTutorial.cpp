#include "TrackEditViewTutorial.h"

TrackEditViewTutorial::TrackEditViewTutorial (DatabaseManager& dbManager) : db (dbManager)
{
    addAndMakeVisible (endTutorial);
    endTutorial.onClick = [&] {
        db.addCompletedTutorial ("TrackViewTutorial", "User1");
    };
}

TrackEditViewTutorial::~TrackEditViewTutorial() = default;

void TrackEditViewTutorial::paint (Graphics& g)
{
    g.fillAll (Colours::black);
    g.setColour (Colours::white);
    g.setFont (20.0f);
    g.drawText ("Hello, Welcome to the TrackView Tutorial!", getLocalBounds(), Justification::centred, true);
}

void TrackEditViewTutorial::resized()
{
    FlexBox box;
    box.flexDirection = FlexBox::Direction::row;
    box.justifyContent = FlexBox::JustifyContent::spaceBetween;
    box.alignItems = FlexBox::AlignItems::flexEnd;

    box.items.addArray ({
        FlexItem (endTutorial)
            .withFlex (1.0f, 1.0f)
            .withMinWidth (50.0f)
            .withMinHeight (30.0f)
            .withMargin ({ 5.0f, 10.0f, 5.0f, 10.0f }),
    });

    box.performLayout (getLocalBounds().reduced (10)); // Add overall padding
}
