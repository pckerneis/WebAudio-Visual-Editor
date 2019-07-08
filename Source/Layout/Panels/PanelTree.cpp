/*
  ==============================================================================

    PanelTree.cpp
    Created: 22 Apr 2018 11:17:02am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "PanelTree.h"

PanelNode::PanelNode (bool isVertical, PanelTree* tree) : layout (*this, isVertical), panelTree (tree)
{
    //if (isVertical)
        layout.setKeepsProportionsWhenResized (true);
}

void PanelNode::resized()
{
    layout.layOutComponents (getLocalBounds(), true);
    
    for (auto c : children)
        c->resized();
    
    if (correspondingComp)
        correspondingComp->setBounds (getLocalBounds());
}

#include "Panel.h"
bool PanelNode::tryToCloseAllPanels()
{
    if (auto panel = dynamic_cast<Panel*>(correspondingComp.get()))
        return panel->tryToClosePanel();
    else
    {
        for (auto c : children)
        {
            bool canceled = ! c->tryToCloseAllPanels();
            
            if (canceled)
                return false;
        }
    }
    
    return true;
}

bool PanelNode::closeAllPanels()
{
    if (auto panel = dynamic_cast<Panel*>(correspondingComp.get()))
        panel->panelWasClosed();
    
    for (auto c : children)
        c->closeAllPanels();
    
    return true;
}

PanelNode* PanelNode::addChildNode (float defaultSize, float minSize, float maxSize)
{
    const bool defaultSizeSpecified = (defaultSize != -1);
    
    if (!defaultSizeSpecified)
        defaultSize = -1. / ((float)children.size() + 1.);
    
    auto node = new PanelNode (!layout.isVerticalLayout(), panelTree);
    layout.addItem (node, minSize, maxSize, defaultSize, true, -1, !defaultSizeSpecified);
    children.add (node);
    return node;
}

PanelNode* PanelNode::removeChildNode (PanelNode* n)
{
    removeChildComponent (n);
    layout.removeItem (n);
    children.removeObject (n);
    
    if (correspondingComp == n)
        correspondingComp = nullptr;
    
    // If unique child with comp, take this comp and delete child
    if (children.size() == 1 && children[0]->correspondingComp)
    {
        auto c = children[0];
        
        correspondingComp = c->correspondingComp;
        
        layout.removeItem (c);
        children.removeObject (c);
        
        addAndMakeVisible (correspondingComp);
        
        return this;
    }
    
    // If unique child with stack, put the stack in parent and remove this
    if (children.size() == 1 && !children[0]->correspondingComp)
    {
        auto c = children[0];
        
        auto parent = getParentNode();
        
        // If we're at the root, we need to set a new root holding this node
        if (!parent)
        {
            parent = new PanelNode (!layout.isVerticalLayout(), panelTree);
            parent->insertNode(panelTree->releaseRoot(), -1);
            panelTree->setRoot (parent);
        }
        
        if (auto parent = getParentNode())
        {
            const int indexToInsert = parent->getIndexOfChildNode (this);
            
            for (int i = c->children.size(); --i >= 0;)
            {
                auto grandChild = c->children.getUnchecked(i);
                parent->insertNode (grandChild, indexToInsert);
                c->layout.removeItem (grandChild);
                c->children.removeObject(grandChild, false);
            }
            
            removeChildNode (c);
            return parent->removeChildNode (this);
        }
    }
    
    // Delete this node from parent
    if (children.size() == 0 && correspondingComp == nullptr)
    {
        if (auto parent = getParentNode())
            return parent->removeChildNode (this);
        else if (panelTree)
            panelTree->closeParentWindow();
    }
    
    return nullptr;
}

void PanelNode::setComponent (Component* c)
{
    if (correspondingComp)
        removeChildComponent (correspondingComp);
    
    if (c == nullptr)
        return;
    
    addAndMakeVisible (c);
    correspondingComp = c;
    
    resized();
}

void PanelNode::insertNode (PanelNode* newChild, int index)
{
    layout.addItem (newChild, 100, -1., -1., true, -1, false);
    children.add (newChild);
    
    if (index > 0)
        moveNode (children.size() - 1, index);
    
    refreshPanelPreferences();
}

void PanelNode::insertComponent (Component* c, int index)
{
    if (correspondingComp == nullptr && children.size() == 0)
    {
        setComponent (c);
        return;
    }
    else if (children.size() == 0)
    {
        removeChildComponent (correspondingComp);
        auto newNode = addChildNode();
        newNode->setComponent (correspondingComp);
        correspondingComp = nullptr;
    }
    
    auto newNode = addChildNode ();
    newNode->setComponent (c);
    
    if (index >= 0 && index != children.size() - 1)
        moveNode (children.size() - 1, index);
    
    refreshPanelPreferences();
}

void PanelNode::moveNode (int oldIndex, int newIndex)
{
    if (oldIndex == newIndex)
        return;
    
    children.move (oldIndex, newIndex);
    layout.move (oldIndex * 2, newIndex * 2);
    
}

void PanelNode::moveNode (PanelNode* n, int newIndex)
{
    const auto oldIndex = getIndexOfChildNode (n);
    
    moveNode (oldIndex, newIndex);
}

int PanelNode::getIndexOfChildNode (PanelNode* node) const
{
    for (int i = 0; i < children.size(); ++i)
        if (children.getUnchecked(i) == node)
            return i;
    
    return -1;
}

#include "TabbedPanel.h"
void PanelNode::getDropZones (OwnedArray<DropZone>& result, bool hideTabs, juce::Rectangle<int> boundsInRoot)
{
    const auto parentTree = getParentTree();
    const bool vertical = layout.isVerticalLayout();
    const int w = 6;
    
    if (boundsInRoot.isEmpty())
        boundsInRoot = getLocalBounds();
    
    if (vertical)   // means this node is inside a horizontal layout
    {
        result.add (new DropZone (parentTree, this, 0, boundsInRoot.removeFromTop (w), DropZoneSide::SideTop));
        result.add (new DropZone (parentTree, this, children.size() + 1, boundsInRoot.removeFromBottom (w), DropZoneSide::SideBottom));
        
        if (auto parentNode = getParentNode())
        {
            const int indexForThis = parentNode->getIndexOfChildNode (this);
            
            if (indexForThis != 0)
                result.add (new DropZone (parentTree, parentNode, indexForThis, boundsInRoot.removeFromLeft (w), DropZoneSide::SideLeft));
            
            if (indexForThis != parentNode->children.size() - 1)
                result.add (new DropZone (parentTree, parentNode, indexForThis + 1, boundsInRoot.removeFromRight (w), DropZoneSide::SideRight));
        }
    }
    else
    {
        result.add (new DropZone (parentTree, this, 0, boundsInRoot.removeFromLeft (w), DropZoneSide::SideLeft));
        result.add (new DropZone (parentTree, this, children.size() + 1, boundsInRoot.removeFromRight (w), DropZoneSide::SideRight));
        
        if (auto parentNode = getParentNode())
        {
            const int indexForThis = parentNode->getIndexOfChildNode (this);
            
            if (indexForThis != 0)
                result.add (new DropZone (parentTree, parentNode, indexForThis, boundsInRoot.removeFromTop (w), DropZoneSide::SideTop));
            
            if (indexForThis != parentNode->children.size() - 1)
                result.add (new DropZone (parentTree, parentNode, indexForThis + 1, boundsInRoot.removeFromBottom (w), DropZoneSide::SideBottom));
            
        }
    }
    
    // Add a tab drop zone if terminal node
    if (correspondingComp && !hideTabs)
    {
        auto tabZone = boundsInRoot.removeFromTop(20);
        
        // If this is a tabbed panel, we want the tab button bounds
        if (auto tabbed = dynamic_cast<TabbedPanel*>(correspondingComp.get()))
        {
            auto tabButtons = tabbed->getTabButtonBounds();
            result.add (new DropZone (parentTree, this, 0, tabZone.withWidth (w), DropZoneSide::TabBar));
            
            for (int i = 0; i < tabButtons.size(); ++i)
            {
                const auto toRemove = (i == 0) ? 2 * w : w;
                tabZone.removeFromLeft (tabButtons[i].getWidth() - toRemove);
                
                result.add (new DropZone (parentTree, this, i + 1, tabZone.removeFromLeft (w), DropZoneSide::TabBar));
            }
        }
        // Else we just need one drop zone
        else
            result.add (new DropZone (parentTree, this, -1, tabZone, DropZoneSide::TabBar));
    }
    
    // Recursive call for children
    for (int i = 0; i < children.size(); ++i)
    {
        auto c = children.getUnchecked(i);
		juce::Rectangle<int> childBounds;
        
        const int barWidth = layout.getBarWidth();
        
        int amountToRemove = vertical ? c->getHeight() : c->getWidth();
        
        if (i == 0)
            amountToRemove += (barWidth / 2) - w;
        else
            amountToRemove += barWidth;
        
        if (vertical)   childBounds = boundsInRoot.removeFromTop (amountToRemove);
        else            childBounds = boundsInRoot.removeFromLeft (amountToRemove);
        
        c->getDropZones (result, hideTabs, childBounds);
    }
    
}

PanelNode* PanelNode::findNodeWithComponent (Component* c)
{
    if (correspondingComp == c)
        return this;
    else
        for (auto child : children)
            if (auto node = child->findNodeWithComponent (c))
                return node;
    
    return nullptr;
}

TabbedPanel* PanelNode::replaceContentWithTabbedPanel()
{
    auto correspondingPanel = dynamic_cast<Panel*> (correspondingComp.get());
    
    if (correspondingPanel == nullptr)
        return nullptr;
    
    removeChildComponent (correspondingComp);
    
    auto tabbedPanel = new TabbedPanel (getPanelManager());
    tabbedPanel->addPanelAsTab (correspondingPanel);
    
    correspondingComp = tabbedPanel;
    addAndMakeVisible (tabbedPanel);
    
    panelTree->tabbedPanels.add (tabbedPanel);
    
    refreshPanelPreferences();
    resized();
    
    return tabbedPanel;
}

XmlElement* PanelNode::getStateAsXml (String name)
{
    XmlElement* e = new XmlElement (name);
    
    e->addChildElement (layout.getStateAsXml ("LAYOUT"));
    
    for (auto child : children)
        e->addChildElement (child->getStateAsXml ("CHILD"));
    
    if (auto tabbed = dynamic_cast<TabbedPanel*> (correspondingComp.get()))
    {
        auto panelState = new XmlElement ("TABBED");
        panelState->addChildElement (tabbed->getAsXml());
        e->addChildElement (panelState);
    }
    else if (auto panel = dynamic_cast<Panel*> (correspondingComp.get()))
    {
        auto panelState = new XmlElement ("PANEL");
        panelState->setAttribute ("panelId", panel->getPanelId());
        e->addChildElement (panelState);
    }
    
    return e;
}

void PanelNode::restoreStateFromXml (XmlElement* e)
{
    children.clear();
    
    if (e == nullptr)
        return;
    
    forEachXmlChildElementWithTagName (*e, child, "CHILD")
        if (auto c = addChildNode())
            c->restoreStateFromXml (child);
    
    if (auto panelState = e->getChildByName ("PANEL"))
        if (auto panel = getPanelManager()->findPanelWithId (panelState->getIntAttribute ("panelId")))
            setComponent (panel);
    
    if (auto panelState = e->getChildByName ("TABBED"))
    {
        auto tabbedPanel = new TabbedPanel (getPanelManager());
        correspondingComp = tabbedPanel;
        addAndMakeVisible (tabbedPanel);
        panelTree->tabbedPanels.add (tabbedPanel);
        
        tabbedPanel->restoreState (panelState->getChildElement (0));
    }
}

void PanelNode::restoreLayoutFromXml (XmlElement* e)
{
    if (e == nullptr)
        return;
    
    layout.restoreFromXml (e->getChildByName ("LAYOUT"));
    
    int i = -1;
    
    forEachXmlChildElementWithTagName (*e, child, "CHILD")
        if (auto c = children[++i])
            c->restoreLayoutFromXml (child);
}

PanelManager* PanelNode::getPanelManager() const
{
    if (auto tree = getParentTree())
        return &tree->panelManager;
    
    jassertfalse;
    return nullptr;
}

void PanelNode::refreshPanelPreferences()
{
    for (int i = 0; i < children.size(); ++i)
    {
        auto c = children.getUnchecked (i);
        
        if (auto pte = dynamic_cast<PanelTreeEmbedded*>(c->correspondingComp.get()))
        {
            auto pref = pte->getPanelPreferences();
            
            if (auto layoutItem = layout.getItem (i * 2))
            {
                if (layout.isVerticalLayout())
                {
                    layoutItem->minSize = pref.minHeight;
                    layoutItem->maxSize = pref.maxHeight;
                }
                else
                {
                    layoutItem->minSize = pref.minWidth;
                    layoutItem->maxSize = pref.maxWidth;
                }
            }
        }
    }
    
    for (auto c : children)
        c->refreshPanelPreferences();
}

//==============================================================================

PanelTree::PanelTree (PanelManager& manager) : overlay (*this), panelManager (manager)
{
    root = new PanelNode (false, this);
    
    addAndMakeVisible (root);
    addChildComponent (overlay);
}

PanelTree::~PanelTree()
{
    if (root)
        root->closeAllPanels();
    
    masterReference.clear();
}

void PanelTree::resized()
{    
    auto r = root->getBounds();
    auto desired = getLocalBounds();//.reduced (4);
    
    if (r == desired)
        root->resized();
    else
    {
        root->setBounds (desired);
        root->resized();
    }
    
    overlay.setBounds (desired);
}

PanelNode& PanelTree::getRootItem() { return *root; }

void PanelTree::setDropOverlayVisible (bool shouldBeVisible)
{
    overlay.setVisible (shouldBeVisible);
    overlay.toFront (shouldBeVisible);
}

#include "../Source/Layout/Windows/PanelWindow.h"
void PanelTree::moveContainedPanel (Panel* panel, DropZone* dropZone)
{
    if (! panel)
        return;
    
    overlay.setVisible (false);
    
    if (auto node = root->findNodeWithComponent (panel))
        movePanelFromNode (panel, node, dropZone);
    else if (panel->isInTab())
        movePanelFromTabs (panel, dropZone);
    
    refreshPanelPreferences();
    resized();
}

void PanelTree::insertPanel (Panel* panel, DropZone* zoneToDropTo)
{
    // To tab bar
    if (zoneToDropTo->side == TabBar)
    {
        auto tabbedPanel = dynamic_cast<TabbedPanel*> (zoneToDropTo->node->correspondingComp.get());
        
        // If not already a tabbed holder
        if (!tabbedPanel)
        {
            tabbedPanel = new TabbedPanel (&panelManager);
            tabbedPanels.add (tabbedPanel);
            
            if (auto correspPanel = dynamic_cast<Panel*> (zoneToDropTo->node->correspondingComp.get()))
                tabbedPanel->addPanelAsTab (correspPanel);
            
            zoneToDropTo->node->setComponent(tabbedPanel);
        }
        
        tabbedPanel->addPanelAsTab (panel, zoneToDropTo->insertIndex);
        
        refreshPanelPreferences();
        resized();
        
        return;
    }
    
    // To a new node
    if (auto newParent = zoneToDropTo->node)
    {
        newParent->insertComponent (panel, zoneToDropTo->insertIndex);
        
        refreshPanelPreferences();
        resized();
        return;
    }
    
    jassertfalse;
}


void PanelTree::movePanelFromNode (Panel* panel, PanelNode* node, DropZone* zoneToDropTo)
{
    auto containerNode = node->getParentNode();
    
    if (!containerNode)
        return;
    
    // To a node in same parent
    if (zoneToDropTo && containerNode == zoneToDropTo->node)
    {
        const auto oldIndex = containerNode->getIndexOfChildNode (node);
        auto insertIndex = zoneToDropTo->insertIndex;
        
        if (oldIndex < insertIndex)
            --insertIndex;
        
        if (insertIndex == containerNode->getNumChildren())
            --insertIndex;
        
        containerNode->moveNode (oldIndex, insertIndex);
        
        return;
    }
    
    // To same node : do nothing
    if (zoneToDropTo->node == node)
        return;
    
    // To tab bar
    if (zoneToDropTo->side == TabBar)
    {
        auto tabbedPanel = dynamic_cast<TabbedPanel*> (zoneToDropTo->node->correspondingComp.get());
        
        // If not already a tabbed holder
        if (!tabbedPanel)
        {
            tabbedPanel = new TabbedPanel (&panelManager);
            tabbedPanels.add (tabbedPanel);
            
            if (auto correspPanel = dynamic_cast<Panel*> (zoneToDropTo->node->correspondingComp.get()))
                tabbedPanel->addPanelAsTab (correspPanel);
            
            zoneToDropTo->node->setComponent(tabbedPanel);
        }
        
        containerNode->removeChildNode (node);
        tabbedPanel->addPanelAsTab (panel, zoneToDropTo->insertIndex);
        
        resized();
        
        return;
    }
    
    // To a new node
    if (auto newParent = zoneToDropTo->node)
    {
        newParent->insertComponent (panel, zoneToDropTo->insertIndex);
        containerNode->removeChildNode (node);
        
        resized();
        return;
    }
    
    jassertfalse;
}

void PanelTree::movePanelFromTabs (Panel* panel, DropZone* zoneToDropTo)
{
    auto sourcePanel = panel->getParentTabbedPanel();
    
    if (!sourcePanel)
        return;
    
    // To another tab holder
    if (zoneToDropTo && zoneToDropTo->side == TabBar)
    {
        auto destPanel = dynamic_cast<TabbedPanel*> (zoneToDropTo->node->correspondingComp.get());
        
        // If not already a tabbed holder
        if (!destPanel)
        {
            destPanel = new TabbedPanel (&panelManager);
            tabbedPanels.add (destPanel);
            
            if (auto correspPanel = dynamic_cast<Panel*> (zoneToDropTo->node->correspondingComp.get()))
                destPanel->addPanelAsTab (correspPanel);
            
            zoneToDropTo->node->setComponent(destPanel);
        }
        
        sourcePanel->removePanel (panel);
        destPanel->addPanelAsTab (panel, zoneToDropTo->insertIndex);
    }
    // To a new node
    else if (zoneToDropTo)
    {
        sourcePanel->removePanel (panel);
        
        if (auto newParent = zoneToDropTo->node)
            newParent->insertComponent (panel, zoneToDropTo->insertIndex);
    }
    
    resized();
}

void PanelTree::prepareDropOverlay (bool hideTabs)
{
    dropZones.clear();
    root->getDropZones (dropZones, hideTabs);
}

DropZone* PanelTree::lookForDropAreaAt (Point<int> pos)
{
    DropZone* zoneToDropTo = nullptr;
    
    for (auto zone : dropZones)
    {
        zone->highlighted = zone->bounds.contains (pos);
            
        if (zone->bounds.contains (pos))
            zoneToDropTo = zone;
    }
    
    overlay.repaint();
    
    return zoneToDropTo;
}

PanelNode* PanelTree::releaseRoot()
{
    removeChildComponent (root);
    return root.release();
}

void PanelTree::setRoot (PanelNode* panel)
{
    root = panel;
    addAndMakeVisible (root);
}

void PanelTree::refreshPanelPreferences()
{
    root->refreshPanelPreferences();
}

bool PanelTree::tryToCloseAllPanels()
{
    return root->tryToCloseAllPanels();
}

void PanelTree::closeParentWindow()
{
    auto window = findParentComponentOfClass<PanelWindow>();
    panelManager.removePanelWindow (window);
}

void PanelTree::showPanelInTabs (Panel* panel)
{
    for (auto tp : tabbedPanels)
    {
        const auto index = tp->getTabIndexForPanel (panel);
        
        if (index >= 0)
        {
            tp->setCurrentTab (index);
            break;
        }
    }
}

bool PanelTree::findAndRemovePanel (Panel* panel)
{
    if (panel->isInTab())
    {
        auto sourcePanel = panel->getParentTabbedPanel();
        sourcePanel->removePanel (panel);
        //checkTabbedPanelSize (sourcePanel);
        
        return true;
    }
    
    auto nodeToRemove = root->findNodeWithComponent (panel);
    
    if (!nodeToRemove)
        return false;
    
    // if node to remove is root, close this window
    if (nodeToRemove == root)
        closeParentWindow();
    else if (auto parentNode = nodeToRemove->getParentNode())
    {
        parentNode->removeChildNode (nodeToRemove);
        resized();
    }
    
    return true;
}

XmlElement* PanelTree::getStateAsXml()
{
    XmlElement* e = new XmlElement ("PanelTree");
    
    if (root == nullptr)
    return e;
    
    e->addChildElement (root->getStateAsXml ("ROOT"));
    
    return e;
}

void PanelTree::restoreFromXml (XmlElement* e)
{
    root->restoreStateFromXml (e->getChildElement(0));
    root->restoreLayoutFromXml (e->getChildElement(0));
}

void PanelTree::checkTabbedPanelSize (TabbedPanel* panel)
{
    // If source panel's size is 1, turn it into a regular node
    if (panel != nullptr && panel->getNumTabs() == 1)
    {
        auto panelToMove = panel->getTabbedPanel (0);
        auto node = root->findNodeWithComponent (panel);
        
        if (panelToMove && node)
        {
            tabbedPanels.removeObject (panel);
            node->setComponent (panelToMove);
            panelToMove->setParentTabbedPanel (nullptr);
            panelToMove->resized();
        }
    }
}

//==============================================================================

void PanelTree::DropOverlay::paint (Graphics& g)
{
    Array<Colour> colours;
    colours.add (Colours::orange);
    colours.add (Colours::blue);
    colours.add (Colours::green);
    colours.add (Colours::red);
    colours.add (Colours::violet);
        
    for (int i = 0; i < owner.dropZones.size(); ++i)
    {
        if (auto zone = owner.dropZones[i])
        {
            float opacity = zone->highlighted ? 0.8f : 0.1f;
            g.setColour (colours[zone->side].withAlpha(opacity));
            g.fillRect (zone->bounds);
        }
    }
}

