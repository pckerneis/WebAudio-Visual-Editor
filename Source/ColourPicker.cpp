/*
  ==============================================================================

    ColourPicker.cpp
    Created: 27 Oct 2017 9:31:16am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "ColourPicker.h"

ShadeButton::ShadeButton (String name, Colour colour) : Button (name), currentColour (colour), outlineColour (colour.withMultipliedBrightness (1.5f)) {}

void ShadeButton::paintButton (Graphics &g, bool isMouseOverButton, bool isButtonDown)
{
    const auto r = getLocalBounds();
    
    g.setColour (currentColour);
    g.fillRect (r);
    
    g.setColour (WaveLookAndFeel::getDefaultLookAndFeel().findColour (TextEditor::outlineColourId));
    g.drawRect (r);
}

void ShadeButton::setColour (Colour colour)
{
    if (currentColour != colour)
    {
        currentColour = colour;
        repaint();
    }
}

//==============================================================================

ColourPickerComponent::ColourPickerComponent() : PopupMenu::CustomComponent (true)
{
    for (auto c : getAllColours())
    {
        auto b = new ShadeButton (c.toString(), c);
        shades.add (b);
        addAndMakeVisible (b);
        b->addListener (this);
    }
}

void ColourPickerComponent::getIdealSize (int &idealWidth, int &idealHeight)
{
    const int numColour = getAllColours().size();
    
    const int numCol = 5;
    const int numRow = numColour / numCol;
    
    const int w = 20;
    
    idealWidth = numCol * w;
    idealHeight = numRow * w;
}

void ColourPickerComponent::resized()
{
    resizeShades();
}

void ColourPickerComponent::resizeShades()
{
    const int numPerRow = 5;
    const double shadeWidth = getWidth() / numPerRow;
    
    for (int i = 0; i < shades.size(); ++i)
    {
        const int x = (i % numPerRow) * shadeWidth;
        const int y = int(i / numPerRow) * shadeWidth;
        const juce::Rectangle<int> bounds (x, y, shadeWidth, shadeWidth);
        shades.getUnchecked(i)->setBounds (bounds);
    }
}

Array<Colour> ColourPickerComponent::getAllColours()
{
    Array<Colour> colors;
    colors.add (Colour(Colours::steelblue));
    colors.add (Colour(Colours::royalblue));
    colors.add (Colour(Colours::dodgerblue));
    colors.add (Colour(Colours::darkturquoise));
    colors.add (Colour(Colours::skyblue));
    
    colors.add (Colour(Colours::darkgrey));
    colors.add (Colour(Colours::mediumpurple));
    colors.add (Colour(Colours::blueviolet));
    colors.add (Colour(Colours::rebeccapurple));
    colors.add (Colour(Colours::indigo));
    
    colors.add (Colour(Colours::deeppink));
    colors.add (Colour(Colours::palevioletred));
    colors.add (Colour(Colours::plum));
    colors.add (Colour(Colours::violet));
    colors.add (Colour(Colours::salmon));
    
    colors.add (Colour(Colours::brown));
    colors.add (Colour(Colours::chocolate));
    colors.add (Colour(Colours::indianred));
    colors.add (Colour(Colours::orangered));
    colors.add (Colour(Colours::red));
    
    colors.add (Colour(Colours::seagreen));
    colors.add (Colour(Colours::forestgreen));
    colors.add (Colour(Colours::goldenrod));
    colors.add (Colour(Colours::orange));
    colors.add (Colour(Colours::peru));
    
    return colors;
}

void ColourPickerComponent::buttonClicked (Button* b)
{
    if (auto cb = dynamic_cast<ShadeButton*> (b))
        if (listener != nullptr)
            listener->colourPicked (cb->getColour());
}

//==============================================================================

ColourPickerButton::ColourPickerButton (Colour defaultColour) : button ("colour", defaultColour)
{
    addAndMakeVisible (button);
    button.addListener (this);
}

void ColourPickerButton::buttonClicked (Button* b)
{
    if (b == &button)
    {
        auto colourPicker = new ColourPickerComponent();
        colourPicker->setListener (this);
        
        PopupMenu menu;
        menu.addCustomItem (1, colourPicker);
        menu.show();
    }
}

//==============================================================================

ColourPropertyComponent::ColourPropertyComponent (String propertyName, Colour defaultColour) : PropertyComponent (propertyName), button (defaultColour)
{
    addAndMakeVisible (button);
    setName (propertyName);
    setPreferredHeight (22);
}
