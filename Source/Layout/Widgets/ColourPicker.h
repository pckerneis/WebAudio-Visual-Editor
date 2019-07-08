/*
  ==============================================================================

    ColourPicker.h
    Created: 27 Oct 2017 9:31:16am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

//==============================================================================
#include "../Source/Layout/WaveLookAndFeel.h"

class ShadeButton : public Button
{
public:
    ShadeButton (String name, Colour colour);
    ~ShadeButton() {}
    
    void paintButton (Graphics &g, bool isMouseOverButton, bool isButtonDown) override;
    
    void setColour (Colour colour);
    Colour getColour() const { return currentColour; }
    
private:
    Colour currentColour;
    Colour outlineColour;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ShadeButton)
};

//==============================================================================
/** \brief A popup menu custom component with selectable colour shades. */
class ColourPickerComponent : public PopupMenu::CustomComponent, Button::Listener
{
public:
    ColourPickerComponent();
    ~ColourPickerComponent() {}
    
    void getIdealSize (int &idealWidth, int &idealHeight) override;
    
    void resized() override;
    void resizeShades();
    
    static Array<Colour> getAllColours();
    
    void buttonClicked (Button* b) override;
    
    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener() { masterReference.clear(); }
        
        virtual void colourPicked (Colour c) = 0;
        
    private:
        WeakReference<Listener>::Master masterReference;
        friend class WeakReference<Listener>;
    };
    //==============================================================================

    void setListener (Listener* l) { listener = l; }
    
private:
    OwnedArray<ShadeButton> shades;
    
    WeakReference<Listener> listener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourPickerComponent)
};


//==============================================================================
/** \brief A coloured button that shows the ColourPickerComponent. */
class ColourPickerButton : public Component, public Button::Listener, public ColourPickerComponent::Listener
{
public:
    ColourPickerButton (Colour defaultColour);
    ~ColourPickerButton() {}
    
    void resized() override { button.setBounds(getLocalBounds()); }
    
    void buttonClicked (Button* b) override;
    
    void colourPicked (Colour c) override
    {
        button.setColour (c);
        
        for (auto l : listeners)
            l->colourPicked (this, c);
    }
    
    void setChosenColour (Colour col) { button.setColour (col); }
    Colour getChosenColour() const { return button.getColour(); }
    
    class Listener
    {
    public:
        virtual ~Listener() {}
        
        virtual void colourPicked (ColourPickerButton* b, Colour c) = 0;
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
private:    
    Array<Listener*> listeners;
    
    ShadeButton button;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourPickerButton)
};

//==============================================================================
/** \brief A PropertyComponent based on the ColourPickerButton. */
class ColourPropertyComponent : public PropertyComponent
{
public:
    ColourPropertyComponent (String propertyName, Colour defaultColour = Colour (Colours::grey));
    
    void refresh() override {}
    
    void setChosenColour(Colour col) { button.setChosenColour (col); }
    Colour getChosenColour() const { return button.getChosenColour(); }
    
    //==============================================================================
    void addListener (ColourPickerButton::Listener* l) { button.addListener (l); }
    void removeListener (ColourPickerButton::Listener* l) { button.removeListener (l); }
private:
    ColourPickerButton button;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ColourPropertyComponent)
};
