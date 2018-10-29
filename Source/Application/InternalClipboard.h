/*
  ==============================================================================

    InternalClipboard.h
    Created: 13 Sep 2017 10:32:22pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class InternalClipboard
{
public:
    InternalClipboard() {}
    ~InternalClipboard() {}
    
    String getClipboard()
    {
        return clipboard;
    }
    
    void setClipboard (String s)
    {
        if (clipboard != s)
        {
            clipboard = s;
            numConsecutivePaste = 0;
            // dbg
            SystemClipboard::copyTextToClipboard (s);
        }
    }
    
    int getAndIncNumConsecutivePaste() { return ++numConsecutivePaste; }
    
private:
    String clipboard;
    int numConsecutivePaste = 0;
};
