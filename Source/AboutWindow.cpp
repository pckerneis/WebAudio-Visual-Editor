/*
  ==============================================================================

    AboutWindow.cpp
    Created: 9 Oct 2018 12:57:26pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "AboutWindow.h"

AboutScreen::AboutScreen()
{
    setSize (windowWidth, windowHeight);
}

#include "EmbeddedFonts.h"
void AboutScreen::paint (Graphics& g)
{
    const auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.fillAll (bg);
    
    g.setColour (bg);
    g.setOpacity (0.23f);
    const auto bgImage = ImageCache::getFromMemory (BinaryData::aboutbg_png, BinaryData::aboutbg_pngSize);
    g.drawImageAt (bgImage, 0, 0, false);
    
    const auto r = getLocalBounds().reduced (12);

	SharedResourcePointer<EmbeddedFonts> fonts;

    g.setFont (fonts->getMontserrat());
    g.setFont (26.0f);
    g.setColour (bg.contrasting());
    
    g.drawFittedText ("WebAudio Visual Editor", r, Justification::centred, 1, 1.0);
    
    g.setFont (12.0f);
    
    const String authorString (CharPointer_UTF8 ("\xc2\xa9 2018 Pierre-Cl\xc3\xa9ment Kerne\xc3\xafs"));
    g.drawFittedText (authorString, r, Justification::bottomLeft, 1, 1.0);
    
    auto version = JUCEApplicationBase::getInstance()->getApplicationVersion();
    g.drawFittedText (version, r, Justification::topLeft, 1, 1.0);
}

void AboutScreen::mouseDown (const MouseEvent&)
{
    close();
}

ScopedPointer<AboutScreen> aboutScreen;

void AboutScreen::show()
{
    if (aboutScreen != nullptr)
    {
        aboutScreen->toFront (true);
        return;
    }
    
    aboutScreen = new AboutScreen();
    
    int flags = ComponentPeer::windowIsTemporary | ComponentPeer::windowHasDropShadow;
    
    aboutScreen->setOpaque (true);
    aboutScreen->addToDesktop (flags);
    aboutScreen->setCentreRelative (0.5f, 0.5f);
    aboutScreen->setVisible (true);
}

void AboutScreen::close()
{
    aboutScreen = nullptr;
}
