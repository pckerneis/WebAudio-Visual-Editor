/*
  ==============================================================================

    ConsoleComponent.h
    Created: 19 Apr 2018 2:13:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ConsoleComponent : public Component
{
public:
    ConsoleComponent();
    ~ConsoleComponent() { masterReference.clear(); }
    
    void resized() override;
    
    void print (const String& message);
    void println (const String& message);
    void clear();
    
    //==============================================================================
    class ConsoleEditor : public TextEditor
    {
    public:
        ConsoleEditor()
        {
            setPopupMenuEnabled (true);
        }
        
        ~ConsoleEditor() {}
        
        void addPopupMenuItems (PopupMenu &menuToAddTo, const MouseEvent *mouseClickEvent) override;
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleEditor)
    };
    
    //==============================================================================
    struct Message
    {
        ConsoleEditor *const display;
        String text;
    };
    
private:
    static void* printInternal (void* udata);
    static void* clearInternal (void* udata);
    
    WeakReference<ConsoleComponent>::Master masterReference;
    friend class WeakReference<ConsoleComponent>;
    
    ConsoleEditor display;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ConsoleComponent)
};
