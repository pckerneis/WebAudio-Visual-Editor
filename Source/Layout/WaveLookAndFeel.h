/*
  ==============================================================================

    WaveLookAndFeel.h
    Created: 8 Sep 2017 11:14:54am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

static LookAndFeel_V4::ColourScheme getBrightColourSchemeWithBase (Colour base)
{
    const Colour bg             (base.withMultipliedSaturation(0.1f).withMultipliedBrightness(2.2f));
    const Colour lighter        (bg.withMultipliedBrightness(1.2f));
    const Colour darker         (bg.withMultipliedBrightness(0.8f));
    const Colour evenDarker     (darker.withMultipliedBrightness(0.8f));
    
    const Colour textHighlight  (bg.contrasting());
    const Colour textBase       (textHighlight.withMultipliedBrightness(0.9f));
    
    const Colour elemHighlight  (base);
    const Colour elemBase       (base.withMultipliedBrightness(1.2f).withMultipliedSaturation(0.5f));
    
    return {
        bg,                 // windowBackground
        darker,             // widgetBackground
        darker,             // menuBackground
        lighter,            // outline
        textBase,           // defaultText
        elemBase,           // defaultFill
        textHighlight,      // highlightedText
        elemHighlight,      // highlightedFill
        textHighlight       // menuText
    };
}

static LookAndFeel_V4::ColourScheme getDarkColourSchemeWithBase (Colour base)
{
    const Colour bg             (base.withMultipliedSaturation(0.25f).withMultipliedBrightness(0.4f));
    const Colour lighter        (bg.withMultipliedBrightness(1.2f));
    const Colour darker         (bg.withMultipliedBrightness(0.8f));
    const Colour evenDarker     (darker.withMultipliedBrightness(0.8f));
    
    const Colour textHighlight  (bg.contrasting());
    const Colour textBase       (textHighlight.withMultipliedBrightness(0.92f));
    
    const Colour elemHighlight  (base);
    const Colour elemBase       (base.withMultipliedBrightness(0.8f).withMultipliedSaturation(0.5f));
    
    return {
        bg,                 // windowBackground
        darker,             // widgetBackground
        darker,             // menuBackground
        lighter,            // outline
        textBase,           // defaultText
        elemBase,           // defaultFill
        textHighlight,      // highlightedText
        elemHighlight,      // highlightedFill
        textHighlight       // menuText
    };
}

#include "../Application/AppSettings.h"
class WaveLookAndFeel    : public LookAndFeel_V4
{
public:
    WaveLookAndFeel()
    {
        auto mainColour = AppSettings::getCurrentMainColour();
        
        if (AppSettings::isCurrentLookBright())
            setColourScheme (getBrightColourSchemeWithBase (mainColour));
        else
            setColourScheme (getDarkColourSchemeWithBase (mainColour));
        
        setUsingNativeAlertWindows(true);
    }
    
    void setMainColour (Colour c)
    {
        if (AppSettings::isCurrentLookBright())
            setColourScheme (getBrightColourSchemeWithBase (c));
        else
            setColourScheme (getDarkColourSchemeWithBase (c));
    }
    
    Colour getUIColour (LookAndFeel_V4::ColourScheme::UIColour uiColour)
    {
        return getCurrentColourScheme().getUIColour(uiColour);
    }
    
    juce::Rectangle<int> getPropertyComponentContentPosition (PropertyComponent& component) override
    {
        const auto textW = jlimit(75, 200, component.proportionOfWidth(0.5f));
        return { textW, 0, component.getWidth() - textW, component.getHeight() - 1 };
    }
    
    void drawRotarySlider (Graphics& g, int x, int y, int width, int height, float sliderPos,
                      const float rotaryStartAngle, const float rotaryEndAngle, Slider& slider) override
    {
        const auto outline = slider.findColour (Slider::rotarySliderOutlineColourId);
        const auto fill    = slider.findColour (Slider::rotarySliderFillColourId);
        
        const auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (8.0f);
        
        auto radius = jmin (bounds.getWidth(), bounds.getHeight()) / 2.0f;
        const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
        auto lineW = jmin (5.0f, radius * 0.3f);
        auto arcRadius = radius - lineW * 0.5f;
        
        Path backgroundArc;
        backgroundArc.addCentredArc (bounds.getCentreX(),
                                     bounds.getCentreY(),
                                     arcRadius,
                                     arcRadius,
                                     0.0f,
                                     rotaryStartAngle,
                                     rotaryEndAngle,
                                     true);
        
        g.setColour (outline);
        g.strokePath (backgroundArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
        
        if (slider.isEnabled())
        {
            Path valueArc;
            valueArc.addCentredArc (bounds.getCentreX(),
                                    bounds.getCentreY(),
                                    arcRadius,
                                    arcRadius,
                                    0.0f,
                                    rotaryStartAngle,
                                    toAngle,
                                    true);
            
            g.setColour (fill);
            g.strokePath (valueArc, PathStrokeType (lineW, PathStrokeType::curved, PathStrokeType::rounded));
        }
        
        const auto thumbWidth = lineW * 1.5f;
        const Point<float> thumbPoint (bounds.getCentreX() + arcRadius * std::cos (toAngle - float_Pi * 0.5f),
                                       bounds.getCentreY() + arcRadius * std::sin (toAngle - float_Pi * 0.5f));
        
        g.setColour (slider.findColour (Slider::thumbColourId));
        g.fillEllipse (juce::Rectangle<float> (thumbWidth, thumbWidth).withCentre (thumbPoint));
    }
    
    void drawDocumentWindowTitleBar (DocumentWindow& window, Graphics& g,
                                     int w, int h, int titleSpaceX, int titleSpaceW,
                                     const Image* icon, bool drawTitleTextOnLeft) override
    {
        if (w * h == 0)
            return;
        
        const bool isActive = window.isActiveWindow();
        
        g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::widgetBackground));
        
        const int cornerRadius = 4;
        g.fillRoundedRectangle (0, 0, w, h,
                                cornerRadius);
        
        g.fillRect (g.getClipBounds().removeFromBottom (cornerRadius));
        
        Font font (h * 0.7f, Font::plain);
        g.setFont (font);
        
        int textW = font.getStringWidth (window.getName());
        int iconW = 0;
        int iconH = 0;
        
        if (icon != nullptr)
        {
            iconH = (int) font.getHeight();
            iconW = icon->getWidth() * iconH / icon->getHeight() + 4;
        }
        
        textW = jmin (titleSpaceW, textW + iconW);
        int textX = drawTitleTextOnLeft ? titleSpaceX
        : jmax (titleSpaceX, (w - textW) / 2);
        
        if (textX + textW > titleSpaceX + titleSpaceW)
            textX = titleSpaceX + titleSpaceW - textW;
        
        if (icon != nullptr)
        {
            g.setOpacity (isActive ? 1.0f : 0.6f);
            g.drawImageWithin (*icon, textX, (h - iconH) / 2, iconW, iconH,
                               RectanglePlacement::centred, false);
            textX += iconW;
            textW -= iconW;
        }
        
        if (window.isColourSpecified (DocumentWindow::textColourId) || isColourSpecified (DocumentWindow::textColourId))
            g.setColour (window.findColour (DocumentWindow::textColourId));
        else
            g.setColour (getCurrentColourScheme().getUIColour (ColourScheme::defaultText));
        
        g.drawText (window.getName(), textX, -2, textW, h, Justification::centredLeft, true);
    }
    
    class WaveLookAndFeel_DocumentWindowButton   : public Button
    {
    public:
        WaveLookAndFeel_DocumentWindowButton (const String& name, Colour c, const Path& normal, const Path& toggled)
        : Button (name), colour (c), normalShape (normal), toggledShape (toggled)
        {
        }
        
        void paintButton (Graphics& g, bool isMouseOverButton, bool isButtonDown) override
        {
            const float d = 12.0;
            const auto r = getLocalBounds().toFloat().withSizeKeepingCentre(d, d);
            
            bool isActive = false;
            
            if (auto w = findParentComponentOfClass<DocumentWindow>())
                isActive = w->isActiveWindow();
            
            const auto background = isActive ? colour : Colours::lightgrey;
            
            g.setColour (background);
            g.fillEllipse (r);
            
            g.setColour (background.darker());
            g.drawEllipse (r, 1.);
            
            if (isMouseOverButton)
            {
                auto& p = getToggleState() ? toggledShape : normalShape;
                
                auto reducedRect = Justification (Justification::centred)
                .appliedToRectangle (juce::Rectangle<int> (getHeight(), getHeight()), getLocalBounds())
                .toFloat()
                .reduced (getHeight() * 0.32f);
                
                g.setColour (colour.darker (0.9f));
                g.fillPath (p, p.getTransformToScaleToFit (reducedRect, true));
            }
        }
        
    private:
        Colour colour;
        Path normalShape, toggledShape;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WaveLookAndFeel_DocumentWindowButton)
    };
    
    Button* createDocumentWindowButton (int buttonType) override
    {
        Path shape;
        const float crossThickness = 0.15f;
        
        if (buttonType == DocumentWindow::closeButton)
        {
            shape.addLineSegment ({ 0.0f, 0.0f, 1.0f, 1.0f }, crossThickness);
            shape.addLineSegment ({ 1.0f, 0.0f, 0.0f, 1.0f }, crossThickness);
            
            return new WaveLookAndFeel_DocumentWindowButton ("close", Colour (0xff9A131D), shape, shape);
        }
        
        if (buttonType == DocumentWindow::minimiseButton)
        {
            shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);
            
            return new WaveLookAndFeel_DocumentWindowButton ("minimise", Colour (0xffaa8811), shape, shape);
        }
        
        if (buttonType == DocumentWindow::maximiseButton)
        {
            shape.addLineSegment ({ 0.5f, 0.0f, 0.5f, 1.0f }, crossThickness);
            shape.addLineSegment ({ 0.0f, 0.5f, 1.0f, 0.5f }, crossThickness);
            
            Path fullscreenShape;
            fullscreenShape.startNewSubPath (45.0f, 100.0f);
            fullscreenShape.lineTo (0.0f, 100.0f);
            fullscreenShape.lineTo (0.0f, 0.0f);
            fullscreenShape.lineTo (100.0f, 0.0f);
            fullscreenShape.lineTo (100.0f, 45.0f);
            fullscreenShape.addRectangle (45.0f, 45.0f, 100.0f, 100.0f);
            PathStrokeType (30.0f).createStrokedPath (fullscreenShape, fullscreenShape);
            
            return new WaveLookAndFeel_DocumentWindowButton ("maximise", Colour (0xff0A830A), shape, fullscreenShape);
        }
        
        jassertfalse;
        return nullptr;
    }
    void fillResizableWindowBackground (Graphics& g, int w, int h,
                                        const BorderSize<int>& /*border*/, ResizableWindow& window) override
    {
#if JUCE_MAC
        const float cornerRadius = 6.0;
#else
		const float cornerRadius = 0.0;
#endif

        
        Path p;
        
        p.addRoundedRectangle (0, 0,
                               w, h,
                               cornerRadius, cornerRadius,
                               false, false,
                               true, true);
        
        g.setColour (Desktop::getInstance().getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId).darker());
        g.fillPath (p);
    }
    
    void drawResizableFrame (Graphics& g, int w, int h, const BorderSize<int>& border) override
    {
        if (! border.isEmpty())
        {
            const float cornerRadius = 5.0f;
            
            const juce::Rectangle<int> fullSize (0, 0, w, h);
            const juce::Rectangle<int> centreArea (border.subtractedFrom (fullSize));
            
            g.saveState();
            
            g.excludeClipRegion (centreArea);
            
            g.setColour (Colour (0x50000000));
            g.drawRoundedRectangle (fullSize.toFloat(), cornerRadius, 1.0f);
         
            /*
            g.setColour (Colour (0x19000000));
            g.drawRect (centreArea.expanded (1, 1));
            */
        
            g.restoreState();
        }
    }
    
    void positionComboBoxText (ComboBox& box, Label& label) override
    {
        label.setBounds (1, 1,
                         box.getWidth() - 20,
                         box.getHeight() - 2);
        
        label.setFont (getComboBoxFont (box));
    }
    
    void drawComboBox (Graphics& g, int width, int height, bool, int, int, int, int, ComboBox& box) override
    {
        const auto cornerSize = box.findParentComponentOfClass<ChoicePropertyComponent>() != nullptr ? 0.0f : 3.0f;
        const juce::Rectangle<int> boxBounds (0, 0, width, height);
        
        g.setColour (box.findColour (ComboBox::backgroundColourId));
        g.fillRoundedRectangle (boxBounds.toFloat(), cornerSize);
        
        g.setColour (box.findColour (ComboBox::outlineColourId));
        g.drawRoundedRectangle (boxBounds.toFloat().reduced (0.5f, 0.5f), cornerSize, 1.0f);
        
		juce::Rectangle<int> arrowZone (width - 20, 0, 14, height);
        Path path;
        path.startNewSubPath (arrowZone.getX() + 3.0f, arrowZone.getCentreY() - 2.0f);
        path.lineTo (static_cast<float> (arrowZone.getCentreX()), arrowZone.getCentreY() + 3.0f);
        path.lineTo (arrowZone.getRight() - 3.0f, arrowZone.getCentreY() - 2.0f);
        
        g.setColour (box.findColour (ComboBox::arrowColourId).withAlpha ((box.isEnabled() ? 0.9f : 0.2f)));
        g.strokePath (path, PathStrokeType (2.0f));
    }
    
	static Colour getWindowBackgroundColour()
	{
#if JUCE_MAC
		return Colours::transparentBlack;
#else
		return  Colours::black;
#endif
	}
};

//==============================================================================

class LookAndFeelUpdater
{
public:
    LookAndFeelUpdater()
    {
        LookAndFeel::setDefaultLookAndFeel (&lookAndFeel);
    }
    
    ~LookAndFeelUpdater() {}
    
    WaveLookAndFeel& getWaveLookAndFeel() { return lookAndFeel; }
    
    //==============================================================================
    static void lookAndFeelChanged()
    {
        SharedResourcePointer<LookAndFeelUpdater> updater;
        updater->notify();
    }
    
    static WaveLookAndFeel& getLookAndFeel()
    {
        SharedResourcePointer<LookAndFeelUpdater> updater;
        return updater->getWaveLookAndFeel();
    }
    
    //==============================================================================
    class Listener
    {
    public:
        Listener()
        {
            SharedResourcePointer<LookAndFeelUpdater> updater;
            updater->addListener (this);
        }
        
        virtual ~Listener()
        {
            SharedResourcePointer<LookAndFeelUpdater> updater;
            updater->removeListener (this);
        }
        
        void updateLookAndFeel()
        {
            if (auto thisComp = dynamic_cast<Component*> (this))
            {
                thisComp->setLookAndFeel (nullptr);
                thisComp->setLookAndFeel (&LookAndFeel::getDefaultLookAndFeel());
            }
        }
    };
    
    //==============================================================================
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    void notify()
    {
        for (auto l : listeners)
            l->updateLookAndFeel();
    }
    
private:
    Array<Listener*> listeners;
    
    WaveLookAndFeel lookAndFeel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LookAndFeelUpdater)
};

