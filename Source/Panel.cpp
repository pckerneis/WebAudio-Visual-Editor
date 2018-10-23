/*
  ==============================================================================

    Panel.cpp
    Created: 31 Aug 2017 8:42:59pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "Panel.h"

#include "Project.h"
Panel::Panel (PanelManager* manager, bool wantsMouseListener) : border (*this), mouseListener (*this)
{
    addAndMakeVisible (&border);
    border.setInterceptsMouseClicks (false, false);
    
    if (wantsMouseListener)
        addMouseListener (&mouseListener, true);
    
    setMouseCursor (MouseCursor::StandardCursorType::DraggingHandCursor);
    
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    commandManager.registerAllCommandsForTarget (this);
    
    setPanelManager (manager);
}

Panel::~Panel()
{
    if (panelManager != nullptr)
        panelManager->removePanel (this);
    
    masterReference.clear();
}

void Panel::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
    
    if (getLocalBounds().getWidth() < 20)
        return;
    
    if (panelName != String() && !getLocalBounds().isEmpty() && ! isInTab())
    {
        const int headerHeight = 24;
        const int textLeftMargin = 6;
        const int textTopMargin = 0;
        
        g.setColour (LookAndFeel::getDefaultLookAndFeel().findColour (TextButton::textColourOnId));
        g.setFont (Font (14.0f));
        
        g.drawText (panelName, textLeftMargin, textTopMargin, getWidth() - textLeftMargin, headerHeight, Justification::centredLeft, true);
        
        g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).withMultipliedBrightness (0.8f));
        g.fillRect (0, headerHeight, getWidth(), 2);
    }
}

void Panel::resized()
{
    border.setBounds (getLocalBounds());
}

void Panel::mouseDown (const MouseEvent& e)
{
    if (panelManager)
        panelManager->startDragging (this, e);
}

void Panel::mouseDrag (const MouseEvent& e)
{
    if (!panelManager)
        return;
    
    if (e.getDistanceFromDragStart() > 5)
        panelManager->dragPanel (this, e);
}

void Panel::mouseUp (const MouseEvent& e)
{
    if (panelManager)
        panelManager->stopDragging (this, e);
}

void Panel::gainFocus()
{
    if (panelManager != nullptr)
        panelManager->setFocus (this);
}

#include "PanelWindow.h"
void Panel::reveal()
{
    if (isInTab())
        showInTabs();
            
    if (auto window = getParentPanelWindow())
        window->toFront (false);
    else
        panelManager->createAndAddPanelWindow (this);
    
    gainFocus();
}

void Panel::setPanelManager (PanelManager* pm)
{
    if (panelManager != nullptr)
    {
        panelManager->removePanel (this);
        panelManager = nullptr;
    }
    
    if (pm != nullptr)
    {
        panelManager = pm;
        pm->addPanel (this);
        
        border.toFront(false);
    }
}

void Panel::setFocus (bool shouldHaveFocus)
{
    if (shouldHaveFocus != hasFocus)
    {
        if (getChildren().getLast() != &border)
            sendPanelBorderToFront();
        
        hasFocus = shouldHaveFocus;
        border.repaint();
        
        if (hasFocus)
            hasGainedFocus();
        else
            hasLostFocus();
    }
}

#include "TabbedPanel.h"

void Panel::setParentTabbedPanel (TabbedPanel* panel)
{
    parentTabbedPanel = panel;
}

TabbedPanel* Panel::getParentTabbedPanel() const
{
    return parentTabbedPanel;
}

void Panel::sendPanelToFront()
{
    if (isInTab())
        showInTabs();
    
    if (auto window = getParentPanelWindow())
        window->toFront (false);
}

#include "PanelTree.h"
void Panel::showInTabs()
{
    if (auto tree = getParentTree())
        tree->showPanelInTabs (this);
}


void Panel::setPanelName (String name)
{
    panelName = name;
    
    if (parentTabbedPanel)
        parentTabbedPanel->panelNameChanged (this);
    
    if (getHeaderHeight() > 0)
        repaint();
}

PanelWindow* Panel::getParentPanelWindow() const
{
    if (auto parentTree = getParentTree())
        return parentTree->findParentComponentOfClass<PanelWindow>();
    
    return nullptr;
}

PanelTree* Panel::getParentTree() const
{
    if (isInTab())
        return getParentTabbedPanel()->findParentComponentOfClass<PanelTree>();
    
    return findParentComponentOfClass<PanelTree>();
}

XmlElement* Panel::getAsXml()
{
    auto e = new XmlElement (getPanelName().isEmpty() ? "Panel" : getPanelName().removeCharacters(" "));
    e->setAttribute ("panelId", getPanelId());
    return e;
}

void Panel::restoreState (XmlElement* e)
{
    if (e == nullptr)
        return;
    
    setPanelId (e->getIntAttribute ("panelId"));
}
//==============================================================================

void Panel::FocusBorder::paint (Graphics& g)
{
    if (panel.hasPanelFocus())
    {
        Colour colour;
        
        if (auto lf = dynamic_cast<LookAndFeel_V4*>(&LookAndFeel::getDefaultLookAndFeel()))
        {
            typedef LookAndFeel_V4::ColourScheme::UIColour UIColour;
            colour = lf->getCurrentColourScheme().getUIColour(UIColour::highlightedFill);
        }
        else
        {
            colour = Colours::steelblue;
        }
        
        g.setColour (colour);
        g.drawRect (getLocalBounds(), 1);
    }
}

//==============================================================================

void Panel::PanelMouseListener::mouseDown (const MouseEvent& e)
{
    if (!panel.hasPanelFocus())
        panel.gainFocus();
}

//==============================================================================
PanelManager::PanelManager (Project& p) : project (p)
{}

PanelManager::~PanelManager()
{
    masterReference.clear();
}

void PanelManager::addPanel (Panel* panel)
{
    panels.addIfNotAlreadyThere (panel);
    panel->panelId = ++previousPanelId;
}

void PanelManager::removePanel (Panel* panel)
{
    panels.removeFirstMatchingValue (panel);
}

void PanelManager::setFocus (Panel* panelToGiveFocusTo)
{
    for (auto panel : panels)
        panel->setFocus (panel.get() == panelToGiveFocusTo);
    
    if (panelToGiveFocusTo != nullptr)
        lastFocus = panelToGiveFocusTo;
}

void PanelManager::enableFocus()
{
    setFocus (lastFocus);
}

void PanelManager::disableFocus()
{
    setFocus (nullptr);
}

PanelWindow* PanelManager::createAndAddPanelWindow (Panel* content, juce::Rectangle<int> bounds)
{
    if (content == nullptr)
        return nullptr;
    
    auto w = new PanelWindow (content, *this, project);
    panelWindows.add (w);
    
    if (bounds.isEmpty())
    {
        const auto pref = content->getPanelPreferences();
        w->centreWithSize (pref.defaultWidth, pref.defaultHeight);
    }
    else
        w->setBounds (bounds);
    
    return w;
}

void PanelManager::addPanelWindow (PanelWindow* w)
{
    panelWindows.add (w);
}

void PanelManager::removePanelWindow (PanelWindow* w)
{
    if (panelWindows.size() == 1)
        project.tryToClose();
    else
        panelWindows.removeObject (w);
}

void PanelManager::closeAllWindows()
{
    quitFullScreenMode();
    panelWindows.clear();
}

void PanelManager::quitFullScreenMode()
{
    for (auto w : panelWindows)
        w->setFullScreen (false);
}

void PanelManager::startDragging (Panel* panel, const MouseEvent& e)
{
    if (!panel)
        return;
    
    const bool isTabbedPanel = dynamic_cast<TabbedPanel*>(panel) != nullptr;
    
    for (auto window : panelWindows)
        window->panelTree.prepareDropOverlay (isTabbedPanel);
    
    const auto b = panel->getLocalBounds();
    screenshot.image = panel->createComponentSnapshot (b);
    screenshot.image.multiplyAllAlphas (0.5f);
    screenshot.setSize (b.getWidth(), b.getHeight());
}

void PanelManager::dragPanel (Panel* panel, const MouseEvent& e)
{
    dropZone = nullptr;
    
    bool mouseIsOnDesktop = true;
    
    for (auto window : panelWindows) // Should check by Z order
    {
        auto& tree = window->panelTree;
        
        window->panelTree.setDropOverlayVisible (true);
        
        auto event = e.getEventRelativeTo (&tree.getRootItem());
        
        if (window->getBounds().contains (event.getScreenPosition()))
        {
            mouseIsOnDesktop = false;
            
            if (!dropZone)
                dropZone = tree.lookForDropAreaAt (event.getPosition());
        }
    }
    
    if (mouseIsOnDesktop)
    {
        if (!screenshot.isOnDesktop())
        {
            screenshot.addToDesktop(0);
            screenshot.setVisible (true);
        }
        
        screenshot.setCentrePosition (e.getScreenPosition());
    }
    else
        screenshot.removeFromDesktop();
}

void PanelManager::stopDragging (Panel* panel, const MouseEvent& e)
{
    const bool createNewWindow = screenshot.isOnDesktop();
    auto sourceTree = panel->getParentTree();
    
    for (auto w : panelWindows)
        w->panelTree.setDropOverlayVisible (false);
    
    if (createNewWindow)
    {
        sourceTree->findAndRemovePanel (panel);
        
        createAndAddPanelWindow (panel, screenshot.getBounds());
        
        screenshot.removeFromDesktop();
    }
    else if (dropZone)
    {
        auto destTree = dropZone->tree;
        
        if (destTree == sourceTree)
            sourceTree->moveContainedPanel (panel, dropZone);
        else
        {
            sourceTree->findAndRemovePanel (panel);
            destTree->insertPanel (panel, dropZone);
        }
    }
}

String PanelManager::getWindowsTitle() const
{
    return project.getName() + " - WebAudio Visual Editor";
}

void PanelManager::refreshWindowsTitle()
{
    for (auto w : panelWindows)
        w->setName (getWindowsTitle());
}

Panel* PanelManager::findPanelWithId (int idToLookFor)
{
    for (auto p : panels)
        if (p->panelId == idToLookFor)
            return p;
    
    return nullptr;
}

//==============================================================================

void PanelManager::ComponentScreenshot::paint (Graphics& g)
{
    g.drawImage (image, getLocalBounds().toFloat());
}
