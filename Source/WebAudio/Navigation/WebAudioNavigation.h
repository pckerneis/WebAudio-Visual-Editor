/*
  ==============================================================================

    WebAudioNavigation.h
    Created: 29 Aug 2018 9:31:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/Panels/NavigationPanel.h"

class WebAudioInspectableElement;
class Project;

class WebAudioNavigation : public NavigationPanel
{
public:
    WebAudioNavigation (Project& p);
    
    Component* createComponentFor (NavigationItem* item, NavigationPanel::Navigable* navigable) override;
    
private:
    class ItemComponent;
    
    Project& project;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioNavigation)
};
