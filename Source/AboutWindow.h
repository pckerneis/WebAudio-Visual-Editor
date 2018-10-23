/*
  ==============================================================================

    AboutWindow.h
    Created: 9 Oct 2018 12:57:26pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WaveLookAndFeel.h"

/** \brief The about window content and static methods to add an intance to the desktop. */
class AboutScreen    : public Component, public LookAndFeelUpdater::Listener
{
public:
    AboutScreen();
    virtual ~AboutScreen() {}
    
    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent&) override;
    
    static void show();
    static void close();
    
private:
    const int windowWidth = 350;
    const int windowHeight = 450;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AboutScreen)
};
