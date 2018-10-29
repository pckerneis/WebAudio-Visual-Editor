/*
  ==============================================================================

    ShapeButtons.h
    Created: 4 Nov 2017 2:01:20am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ShapeButtonGeneric : public ShapeButton
{
public:
    ShapeButtonGeneric (String name,
                        Colour normal = Colours::black.withMultipliedAlpha(0.4f),
                        Colour over = Colours::white.withMultipliedAlpha(0.7f),
                        Colour down = Colours::white.withMultipliedAlpha(0.9f))
    : ShapeButton (name, normal, over, down)
    {
        shouldUseOnColours (true);
        setOnColours (Colours::white.withMultipliedAlpha(0.6f),
                      Colours::white.withMultipliedAlpha(0.8f),
                      Colours::white.withMultipliedAlpha(0.9f));
    }
    
    virtual ~ShapeButtonGeneric() {}
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ShapeButtonGeneric)
};

//==============================================================================

class PlusButton : public ShapeButtonGeneric
{
public:
    PlusButton (String name) : ShapeButtonGeneric (name) { setShape (getShape(), false, true, false); }
    ~PlusButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addLineSegment (Line<float> (0.5f, 0.0f, 0.5f, 1.0f), 0.15f);
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), 0.15f);
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlusButton)
};

//==============================================================================

class MinusButton : public ShapeButtonGeneric
{
public:
    MinusButton (String name) : ShapeButtonGeneric (name) { setShape (getShape(), false, true, false); }
    ~MinusButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addLineSegment (Line<float> (0.0f, 0.5f, 1.0f, 0.5f), 0.15f);
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MinusButton)
};

//==============================================================================

class PlayButton : public ShapeButtonGeneric
{
public:
    PlayButton (String name) : ShapeButtonGeneric (name) { setShape (getShape(), false, true, false); }
    ~PlayButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addTriangle(0.0f, 0.0f,
                          0.0f, 1.0f,
                          1.0f, 0.5f);
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayButton)
};

//==============================================================================

class StopButton : public ShapeButtonGeneric
{
public:
    StopButton (String name) : ShapeButtonGeneric (name) { setShape (getShape(), false, true, false); }
    ~StopButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addRectangle (0.0f, 0.0f, 1.0f, 1.0f);
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(StopButton)
};

//==============================================================================

class BuildButton : public ShapeButtonGeneric
{
public:
    BuildButton (String name) : ShapeButtonGeneric (name) { setShape (getShape(), false, true, false); }
    ~BuildButton() {}
    
    static Path getShape()
    {
        Path buildShape;
        buildShape.startNewSubPath(0.0f, 0.35f);
        buildShape.lineTo (0.8f, 0.35f);
        buildShape.lineTo (1.0f, 0.0f);
        buildShape.lineTo (1.5f, 0.35f);
        buildShape.lineTo (1.0f, 0.35f);
        buildShape.lineTo (1.0f, 0.65f);
        buildShape.lineTo (1.5f, 0.65f);
        buildShape.lineTo (1.0f, 1.0f);
        buildShape.lineTo (0.8f, 0.65f);
        buildShape.lineTo (0.0f, 0.65f);
        buildShape.closeSubPath();
        buildShape = buildShape.createPathWithRoundedCorners(0.2f);
        buildShape.applyTransform(AffineTransform::rotation(-0.25 * MathConstants<float>::pi));
        
        return buildShape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BuildButton)
};

//==============================================================================

class PlayStopButton : public ShapeButtonGeneric
{
public:
    PlayStopButton (String name) : ShapeButtonGeneric (name)
    {
        setShape (PlayButton::getShape(), false, true, false);
        setClickingTogglesState (true);
    }
    
    ~PlayStopButton() {}
    
    void buttonStateChanged() override
    {
        if (getToggleState())
            setShape (StopButton::getShape(), false, true, false);
        else
            setShape (PlayButton::getShape(), false, true, false);
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PlayStopButton)
};

//==============================================================================

class CloseButton : public ShapeButtonGeneric
{
public:
    CloseButton (String name) : ShapeButtonGeneric (name)
    {
        setShape (getShape(), false, true, false);
    }
    
    ~CloseButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addLineSegment (Line<float> (0.0f, 0.0f, 1.0f, 1.0f), 0.15f);
        shape.addLineSegment (Line<float> (1.0f, 0.0f, 0.0f, 1.0f), 0.15f);
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CloseButton)
};

//==============================================================================

class PublishButton : public ShapeButtonGeneric
{
public:
    PublishButton (String name) : ShapeButtonGeneric (name)
    {
        setShape (getShape(), false, true, false);
    }
    
    ~PublishButton() {}
    
    static Path getShape()
    {
        Path shape;
        shape.addRectangle (juce::Rectangle<float> (0.0f, 0.0f, 1.0f, 0.1f));
        shape.addRectangle (juce::Rectangle<float> (0.4f, 0.45f, 0.2f, 0.55f));
        shape.addTriangle (0.5f, 0.1f,
                           0.2f, 0.45f,
                           0.8f, 0.45f);
        
        shape.applyTransform (AffineTransform::rotation (0.5 * MathConstants<float>::pi));
        
        return shape;
    }
    
private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PublishButton)
};
