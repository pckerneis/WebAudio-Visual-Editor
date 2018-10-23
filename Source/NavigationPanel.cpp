/*
  ==============================================================================

    NavigationPanel.cpp
    Created: 24 Jul 2017 11:22:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "NavigationPanel.h"

NavigationItem::NavigationItem (NavigationPanel& np, NavigationPanel::Navigable* navigableItem) :  navigationPanel (np), navigable (navigableItem)
{
    navigableItem->setNavigationPanel (&np);
}

String NavigationItem::getUniqueName() const
{
    if (navigable == nullptr)
        return "";
    
    return navigable->getNavigableUuid().toString();
}

bool NavigationItem::mightContainSubItems()
{
    if (navigable == nullptr)
        return false;
    
    return ! navigable->getSubNavigables().isEmpty();
}

void NavigationItem::paintItem (Graphics& g, int width, int height)
{
    if (! isVisible || navigable == nullptr)
        return;
    
    auto info = navigable->getNavigationInfo();
    
    auto col = LookAndFeel::getDefaultLookAndFeel().findColour (TextButton::textColourOnId);
    Colour sel = isSelected() ? col : col.withAlpha(0.7f);
    g.setColour(sel);
    g.setFont (14.0f);
    g.drawText (info["name"],
                4, 0, width - 4, height,
                Justification::centredLeft, true);
}

void NavigationItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen && getNumSubItems() == 0)
        refreshSubItems();
    else
        clearSubItems();
}

void NavigationItem::itemClicked (const MouseEvent &e)
{
    if (navigable != nullptr)
        navigable->reveal (e.mods);
    
    repaintItem();
}

void NavigationItem::itemDoubleClicked (const MouseEvent &e)
{
    navigable->open();
}

void NavigationItem::refreshSubItems()
{
    const auto subNavigables = navigable->getSubNavigables();
    
    clearSubItems();
    
    int i = 0;
    
    for (auto nav : subNavigables)
        addSubItem (new NavigationItem (navigationPanel, nav), i++);
}

void NavigationItem::updateSelectionState ()
{
    if (navigable == nullptr)
        return;
    
    StringPairArray info = navigable->getNavigationInfo();
    String selString = info.getValue("selected", "NULL");
    
    if (selString != String ("NULL"))
        setSelected (selString == "TRUE", false);
}

//==============================================================================

NavigationPanel::Navigable::~Navigable()
{
    masterReference.clear();
}

//==============================================================================

NavigationPanel::NavigationPanel (PanelManager* manager) : Panel (manager)
{
    addAndMakeVisible (navigationTree);
    
    navigationTree.setDefaultOpenness (true);
    navigationTree.setIndentSize (12);
    navigationTree.setRootItemVisible (false);
    navigationTree.setMultiSelectEnabled (true);
    
    setWantsKeyboardFocus (true);
    
    setPanelName ("Navigation");
}

NavigationPanel::~NavigationPanel()
{
    navigationTree.setRootItem (nullptr);
    
    masterReference.clear();
}

void NavigationPanel::refresh (bool async)
{
    if (async)
    {
        startTimer (20);
        return;
    }
    
    rootItem->refreshSubItems();
    repaint();
    recursiveSelectionUpdate (rootItem);
}

void NavigationPanel::resized()
{
    Panel::resized();
    
    navigationTree.setBounds (getLocalBounds().withTrimmedTop (getHeaderHeight()));
}

void NavigationPanel::selectionChanged()
{
    recursiveSelectionUpdate (rootItem);
}

void NavigationPanel::setRootNavigable (Navigable* root)
{
    rootNavigable = root;
    navigationTree.setRootItem (rootItem = new NavigationItem (*this, rootNavigable));
    refresh();
}

void NavigationPanel::recursiveSelectionUpdate (NavigationItem* item)
{
    if (item != nullptr)
    {
        item->updateSelectionState();
    
        for (int i = item->getNumSubItems(); --i >= 0;)
            recursiveSelectionUpdate (dynamic_cast<NavigationItem*>(item->getSubItem(i)));
    }
}

XmlElement* NavigationPanel::getAsXml()
{
    auto e = new XmlElement ("NavigationPanel");
    e->setAttribute ("panelId", getPanelId());
    
    if (auto openness = navigationTree.getOpennessState (true))
    e->addChildElement (openness);
    
    return e;
}

void NavigationPanel::restoreLayout (XmlElement* e)
{
    if (auto openness = e->getChildElement (0))
    navigationTree.restoreOpennessState (*openness, true);
}

Component* NavigationPanel::createComponentFor (NavigationItem* item, Navigable* navigable)
{
    return new NavigationItem::ItemMouseTarget (*item);
}

void NavigationPanel::timerCallback()
{
    refresh (false);
    stopTimer();
}
