/*
  ==============================================================================

    PanelWindow.cpp
    Created: 30 Apr 2018 5:07:59pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/Layout/Windows/PanelWindow.h"

#include "../Panels/Panel.h"

#include "../Source/Project/Project.h"
PanelWindow::PanelWindow (Panel* contentPanel, PanelManager& manager, Project& proj)
: DocumentWindow (manager.getWindowsTitle(),
				  WaveLookAndFeel::getWindowBackgroundColour(), DocumentWindow::allButtons),
panelTree (manager),
panelManager (manager),
project (proj),
contentHolder (*this, panelTree)
{
    setTitleBarHeight (18);
    setResizable(true, false);
    setContentNonOwned (&panelTree, false);
    
    setVisible (true);
    
    if (contentPanel)
    {
        auto node = panelTree.getRootItem().addChildNode();
        node->setComponent (contentPanel);
    }
    
    setUsingNativeTitleBar (true);

#ifndef JUCE_MAC
	setMenuBar(project.getProjectManager().getMenuBarModel());
#endif
}

PanelWindow::PanelWindow (PanelManager& manager, Project& proj) :
DocumentWindow (manager.getWindowsTitle(), WaveLookAndFeel::getWindowBackgroundColour(), DocumentWindow::allButtons),
panelTree (manager),
panelManager (manager),
project (proj),
contentHolder (*this, panelTree)
{
    setTitleBarHeight (18);
    setResizable(true, false);
    setContentNonOwned (&contentHolder, false);
    
    setVisible (true);
    
    setUsingNativeTitleBar (true);

#ifndef JUCE_MAC
	setMenuBar (project.getProjectManager().getMenuBarModel());
#endif
}

PanelWindow::~PanelWindow()
{
}

XmlElement* PanelWindow::getStateAsXml()
{
    XmlElement* state = new XmlElement ("PanelWindow");
    state->setAttribute ("state", getWindowStateAsString());
    state->addChildElement (panelTree.getStateAsXml());
    return state;
}

void PanelWindow::restoreFromXml (XmlElement* state)
{
    if (state == nullptr)
        return;
    
    restoreWindowStateFromString (state->getStringAttribute ("state"));
    panelTree.restoreFromXml (state->getChildElement (0));
}

ApplicationCommandTarget* PanelWindow::getNextCommandTarget()
{
    return &project.getCommandTarget();
}

void PanelWindow::maximiseButtonPressed()
{
    auto display = Desktop::getInstance().getDisplays().getDisplayContaining (getScreenBounds().getCentre());
    auto userArea = display.userArea;
    
    if (userArea != getBounds())
    {
        previousBounds = getBounds();
        setBounds (userArea);
    }
    else
    {
        if (previousBounds == userArea)
        previousBounds = previousBounds.reduced (30);
        
        setBounds (previousBounds);
    }
}

void PanelWindow::activeWindowStatusChanged()
{
    DocumentWindow::activeWindowStatusChanged();
    
    if (isActiveWindow())
        project.getProjectManager().setActiveProject (&project);
}

#include "../Panels/PanelTree.h"
void PanelWindow::closeButtonPressed()
{
    if (project.windowContainsAllStaticPanels (this))
    {
        project.tryToClose();
        return;
    }
    
    if (panelTree.tryToCloseAllPanels())
        panelManager.removePanelWindow (this);
}

void PanelWindow::closeWithoutWarning()
{
    panelManager.removePanelWindow (this);
}

//==============================================================================

PanelWindow::PanelWindowContentHolder::PanelWindowContentHolder (PanelWindow& o, PanelTree& tree) : owner (o), panelTree (tree)
{
    addAndMakeVisible (panelTree);
}

void PanelWindow::PanelWindowContentHolder::resized()
{
    panelTree.setBounds (getLocalBounds().reduced (5));
}
    
