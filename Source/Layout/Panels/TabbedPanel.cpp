/*
  ==============================================================================

    TabbedPanel.cpp
    Created: 30 Apr 2018 10:34:41am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "TabbedPanel.h"

class TabbedPanel::TabbedBarMouseListener : public MouseListener
{
public:
    TabbedBarMouseListener (TabbedPanel& o) : owner (o) {}
    virtual ~TabbedBarMouseListener() {}
    
    void mouseMove (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseMove (e);
    }
    
    void mouseEnter (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseEnter (e);
    }
    
    void mouseExit (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseExit (e);
    }
    
    void mouseDown (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseDown (e);
    }
    
    void mouseUp (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseUp (e);
    }
    
    void mouseDrag (const MouseEvent &e) override
    {
        if (shouldTransmit (e))
            owner.mouseDrag (e);
    }
    
    bool shouldTransmit (const MouseEvent &e) const
    {
        return (e.eventComponent == tabbedButtonBar || e.originalComponent == tabbedButtonBar);
    }
    
private:
    friend class TabbedPanel;
    
    SafePointer<TabbedButtonBar> tabbedButtonBar;
    TabbedPanel& owner;
};

//==============================================================================
TabbedPanel::TabbedPanel (PanelManager* manager) : Panel (manager, false), tabbedComponent (*this)
{
    auto listener = new TabbedBarMouseListener (*this);
    mouseListener = listener;
    
    addAndMakeVisible (tabbedComponent);
    tabbedComponent.setTabBarDepth (26);
    
    auto& tabbedButtonBar = tabbedComponent.getTabbedButtonBar();
    
    tabbedButtonBar.addMouseListener (listener, true);
    tabbedButtonBar.setInterceptsMouseClicks (true, true);
    tabbedButtonBar.setMouseCursor (MouseCursor::DraggingHandCursor);
    
    listener->tabbedButtonBar = &tabbedButtonBar;
}

TabbedPanel::~TabbedPanel()
{
    masterReference.clear();
    
    for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
        if (auto panel = getTabbedPanel(i))
            panel->setParentTabbedPanel (nullptr);
}

void TabbedPanel::resized()
{
    tabbedComponent.setBounds (getLocalBounds());
}

void TabbedPanel::addPanelAsTab (Panel* panel, int insertIndex)
{
    if (!panel)
    {
        jassertfalse;
        return;
    }
    
    // Show panel if already there
    const auto indexForPanel = getTabIndexForPanel (panel);

    if (indexForPanel >= 0)
    {
        tabbedComponent.setCurrentTabIndex (indexForPanel);
        return;
    }
    
    // Or add it...
    Colour tabColour (PanelTabbedComponent::getTabBackgroundColour());
    
    tabbedComponent.addTab (panel->getPanelName(), tabColour, panel, false, insertIndex);
    panel->setParentTabbedPanel (this);
    
    if (insertIndex < 0)
        insertIndex = tabbedComponent.getNumTabs() - 1;
    
    // ...and show it
    tabbedComponent.setCurrentTabIndex (insertIndex);
    
    //resized();
    panel->resized();
}

void TabbedPanel::removePanel (Panel* p)
{
    for (int i = tabbedComponent.getNumTabs(); --i >= 0;)
    {
        if (tabbedComponent.getTabContentComponent(i) == p)
        {
            if (tabbedComponent.getCurrentTabIndex() == i)
                tabbedComponent.setCurrentTabIndex (i - 1);
            
            tabbedComponent.removeTab(i);
            
            p->setParentTabbedPanel (nullptr);
            
            break;
        }
    }
    
    if (auto tree = getParentTree())
        tree->checkTabbedPanelSize (this);
}

bool TabbedPanel::isPanelOpenedAsTab (Panel* panel) const
{
    return getTabIndexForPanel (panel) >= 0;
}

int TabbedPanel::getTabIndexForPanel (Panel* panel) const
{
    for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
        if (tabbedComponent.getTabContentComponent(i) == panel)
            return i;
    
    return -1;
}

void TabbedPanel::setCurrentTab (int index)
{
    tabbedComponent.setCurrentTabIndex (index);
}

Array<juce::Rectangle<int>> TabbedPanel::getTabButtonBounds() const
{
    Array<juce::Rectangle<int>> result;
    
    auto& bar = tabbedComponent.getTabbedButtonBar();
    auto barPos = bar.getPosition();
    
    for (int i = 0; i < tabbedComponent.getNumTabs(); ++i)
    if (auto button = bar.getTabButton (i))
    result.add (button->getBounds().translated (barPos.x, barPos.y));
    
    return result;
}

void TabbedPanel::panelNameChanged (Panel* p)
{
    const int index = getTabIndexForPanel (p);
    
    if (index >= 0)
    tabbedComponent.setTabName (index, p->getPanelName());
}

XmlElement* TabbedPanel::getAsXml()
{
    XmlElement* e = new XmlElement ("TabbedPanel");
    
    for (int i = 0; i < getNumTabs(); ++i)
    {
        if (auto p = getTabbedPanel (i))
        {
            auto tab = new XmlElement ("TAB");
            tab->setAttribute ("panelId", p->getPanelId());
            e->addChildElement (tab);
        }
    }
    
    e->setAttribute ("currentTab", tabbedComponent.getCurrentTabIndex());
    
    return e;
}

void TabbedPanel::restoreState (XmlElement* state)
{
    if (state == nullptr)
    return;
    
    forEachXmlChildElementWithTagName(*state, tab, "TAB")
    addPanelAsTab (getPanelManager()->findPanelWithId (tab->getIntAttribute ("panelId")));
    
    setCurrentTab (state->getIntAttribute ("currentTab"));
}

void TabbedPanel::panelWasClosed()
{
    for (int i = 0; i < getNumTabs(); ++i)
    if (auto p = getTabbedPanel (i))
    p->panelWasClosed();
}

void TabbedPanel::buttonClicked (Button* b)
{
    int index;
    bool showTab = false;
    bool closeTab = false;
    
    for (index = tabbedComponent.getNumTabs(); --index >= 0;)
    {
        if (tabbedComponent.getTabbedButtonBar().getTabButton(index)->getExtraComponent() == b)
        {
            closeTab = true;
            break;
        }
        else if (tabbedComponent.getTabbedButtonBar().getTabButton(index) == b)
        {
            showTab = true;
            break;
        }
    }
    
    if (showTab)        showTabButtonClicked (index);
    if (closeTab)       closeTabButtonClicked (index);
}

void TabbedPanel::closeTabButtonClicked (int indexToClose)
{
    if (auto comp = tabbedComponent.getTabContentComponent (indexToClose))
    {
        if (auto panel = dynamic_cast<Panel*> (comp))
        {
            if (! panel->tryToClosePanel()) // If action cancelled
                return;
        
            removePanel (panel);
            panel->panelWasClosed();
        }
    }
}

void TabbedPanel::showTabButtonClicked (int tabNumber)
{
    if (auto tab = tabbedComponent.getTabContentComponent (tabNumber))
    {
        tab->resized();
        
        if (auto panel = dynamic_cast<Panel*>(tab))
            panel->gainFocus();
    }
}


void TabbedPanel::mouseDownOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e)
{
    if (!button)
        return;
    
    if (auto panel = dynamic_cast<Panel*> (tabbedComponent.getTabContentComponent (button->getIndex())))
        if (auto panelManager = panel->getPanelManager())
            panelManager->startDragging (panel, e);
}

void TabbedPanel::mouseDragOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e)
{
    if (!button)
        return;
    
    if (e.mouseWasDraggedSinceMouseDown())
        if (auto panel = dynamic_cast<Panel*> (tabbedComponent.getTabContentComponent (button->getIndex())))
            if (auto panelManager = panel->getPanelManager())
                panelManager->dragPanel (panel, e);
}

void TabbedPanel::mouseUpOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e)
{
    if (!button)
        return;
    
    auto panel = dynamic_cast<Panel*> (tabbedComponent.getTabContentComponent (button->getIndex()));
    auto panelManager = panel ? panel->getPanelManager() : nullptr;
    
    if (panelManager)
        panelManager->stopDragging (panel, e);
}
//==============================================================================

TabbedPanel::TabbedPanelBarButton::TabbedPanelBarButton (const String &name, TabbedButtonBar &ownerBar, TabbedPanel &ownerTabbed, bool closeButtonEnabled) : TabBarButton (name, ownerBar), owner (ownerTabbed)
{
    if (closeButtonEnabled)
        createAndAddCloseButton (&ownerTabbed);
}

void TabbedPanel::TabbedPanelBarButton::createAndAddCloseButton (TabbedPanel *panel)
{
    closeButton = new CloseButton ("Close tab");
    closeButton->setBorderSize (BorderSize<int> (4, 6, 4, 6));
    
    setExtraComponent (closeButton, ExtraComponentPlacement::afterText);
    
    closeButton->setSize (20, 15);
    closeButton->addListener (panel);
}

void TabbedPanel::TabbedPanelBarButton::mouseDown (const MouseEvent& e)
{
    TabBarButton::mouseDown (e);
    owner.mouseDownOnTabButton (this, e);
}

void TabbedPanel::TabbedPanelBarButton::mouseDrag (const MouseEvent& e)
{
    TabBarButton::mouseDrag (e);
    owner.mouseDragOnTabButton (this, e);
}

void TabbedPanel::TabbedPanelBarButton::mouseUp (const MouseEvent& e)
{
    TabBarButton::mouseUp (e);
    owner.mouseUpOnTabButton (this, e);
}

//==============================================================================

TabbedPanel::PanelTabbedComponent::PanelTabbedComponent(TabbedPanel& o) : TabbedComponent (TabbedButtonBar::Orientation::TabsAtTop), owner (o)
{
}

TabBarButton* TabbedPanel::PanelTabbedComponent::createTabButton (const String& tabName, const int tabIndex) 
{
    TabbedPanelBarButton* button = new TabbedPanelBarButton (tabName, *tabs, owner, true);
    
    button->addListener (&owner);
    return button;
}

void TabbedPanel::PanelTabbedComponent::lookAndFeelChanged()
{
    const Colour c (getTabBackgroundColour());
    
    for (int i = 0; i < getNumTabs(); ++i)
        setTabBackgroundColour(i, c);
}

void TabbedPanel::PanelTabbedComponent::resized()
{
    TabbedComponent::resized();
}

Colour TabbedPanel::PanelTabbedComponent::getTabBackgroundColour()
{
    return LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId).withMultipliedBrightness(0.9);
}
