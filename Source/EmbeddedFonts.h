/*
  ==============================================================================

    EmbeddedFonts.h
    Created: 29 Oct 2018 5:42:59pm
    Author:  Pierre

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class EmbeddedFonts
{
public:
	EmbeddedFonts();

	Font getMontserrat() const;
	Array<Font> getAllFonts() const;

private:
	Font montserrat;
};