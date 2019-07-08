/*
  ==============================================================================

    PreferencesWindow.h
    Created: 13 Dec 2017 7:23:17pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

// Definition in PreferencesWindow.cpp
class ListPropertyComponent;
class TextButtonPropertyComponent;
class IncDecPropertyComponent;
class MsSliderPropertyComponent;
class FontSizePropertyComponent;
class PortPropertyComponent;
class TextColourPropertyComponent;
class FontChoicePropertyComponent;

//==============================================================================
#include "../Source/Layout/Widgets/ColourPicker.h"
class AppPreferencesPanel : public Component, public LookAndFeelUpdater::Listener
{
public:
    AppPreferencesPanel();
    ~AppPreferencesPanel() {}
    
    void resized() override;
    
private:
    enum PageName
    {
        lookPage
    };
    
    void addPreferenceTab (PageName pageName);
    
    //==============================================================================
    // Definition in PreferencesWindow.cpp
    class PreferencesPage;
    
    Component* createComponentForPage (const PageName pageName);
    void lookAndFeelChanged() override;
    
private:
    TabbedComponent tabbedComp;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AppPreferencesPanel)
};

//==============================================================================
/** \brief The app preferences window. */
class PreferencesWindow    : public DocumentWindow
{
public:
    /** \brief Constructor */
    PreferencesWindow ();
    
    void closeButtonPressed() override;
    
    AppPreferencesPanel* getContent() { return content; }
    
    static void show();
    static void close();
    
private:
    SafePointer<AppPreferencesPanel> content;
    
    const int windowWidth = 400;
    const int windowHeight = 428;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesWindow)
};
