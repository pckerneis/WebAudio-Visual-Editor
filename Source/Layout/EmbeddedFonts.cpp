/*
  ==============================================================================

    EmbeddedFonts.cpp
    Created: 29 Oct 2018 5:42:59pm
    Author:  Pierre

  ==============================================================================
*/

#include "EmbeddedFonts.h"

EmbeddedFonts::EmbeddedFonts()
{
	montserrat = Font(Typeface::createSystemTypefaceFor(BinaryData::MontserratLight_ttf,
		BinaryData::MontserratLight_ttfSize));
}

Font EmbeddedFonts::getMontserrat() const 
{ 
	return montserrat;
}

Array<Font> EmbeddedFonts::getAllFonts() const
{
	Array<Font> r;
	r.add(montserrat);
	return r;
}