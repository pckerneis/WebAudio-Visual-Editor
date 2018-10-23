/*
  ==============================================================================

    WebAudioNavigation.cpp
    Created: 29 Aug 2018 9:31:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioNavigation.h"

#include "WebAudioInspectableElement.h"

class WebAudioNavigation::ItemComponent : public Component
{
public:
    ItemComponent (NavigationItem* navItem, WebAudioInspectableElement* e, String icon, NavigationPanel* navPanel) : iconText (icon), item (navItem), element (e), navigationPanel (navPanel)
    {}
    
    void paint (Graphics &g) override
    {
        auto c = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting().withAlpha (0.1f);
        
        if (item->isSelected())
            g.fillAll (c);
        
        auto r = getLocalBounds();
        const auto iconArea = r.removeFromLeft (getHeight());
        const auto textArea = r.reduced (4, 0);
        
        // Icon
        g.setColour (element->getElementColour().withBrightness (0.8f).withAlpha (1.0f));
        g.fillRoundedRectangle (iconArea.reduced (3).toFloat(), 2.);
        
        g.setColour (LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId));
        g.setFont (Font::fromString (Font::getDefaultMonospacedFontName()));
        g.drawFittedText (iconText, iconArea, Justification::centred, 1);
        
        // Label
        g.setFont (14.0f);
        
        if (! element->isWebAudioElementEnabled())
            g.setFont (g.getCurrentFont().italicised());
        
        auto col = LookAndFeel::getDefaultLookAndFeel().findColour (TextButton::textColourOnId);
        g.setColour (element->isWebAudioElementEnabled() ? col : col.withAlpha (0.5f));
        g.drawText (element->getElementName(), textArea, Justification::centredLeft, true);
    }
    
    void mouseUp (const MouseEvent& e) override
    {
        if (e.mouseWasClicked())
            item->itemClicked (e.getEventRelativeTo (navigationPanel));
    }
    
private:
    String iconText;
    NavigationItem* item;
    WebAudioInspectableElement* element;
    NavigationPanel* navigationPanel;
};

//==============================================================================

#include "Project.h"

WebAudioNavigation::WebAudioNavigation (Project& p) : NavigationPanel (&p.getPanelManager()), project (p)
{}

#include "WebAudioGraphElementTypeNames.h"
Component* WebAudioNavigation::createComponentFor (NavigationItem* item, NavigationPanel::Navigable* navigable)
{
    if (navigable == nullptr)
        return nullptr;
    
    auto webAudioElement = dynamic_cast<WebAudioInspectableElement*> (navigable);
    
    if (webAudioElement == nullptr)
        return nullptr;
    
    item->setVisible (false);
    
    auto info = navigable->getNavigationInfo();
    const String type (info["type"]);
    
    if (type == GraphElementType::dynamicRouteType)         return new ItemComponent (item, webAudioElement, "d", this);
    else if (type == GraphElementType::audioContextType)    return new ItemComponent (item, webAudioElement, "c", this);
    else if (type == GraphElementType::messageType)         return new ItemComponent (item, webAudioElement, "m", this);
    else if (type == GraphElementType::audioDataType)       return new ItemComponent (item, webAudioElement, "a", this);
    else if (type == GraphElementType::scriptType)          return new ItemComponent (item, webAudioElement, "s", this);
    else if (type == GraphElementType::audioNodeType || type == GraphElementType::audioDestinationNodeType)
        return new ItemComponent (item, webAudioElement, "n", this);
    
    item->setVisible (true);
    
    return nullptr;
}
