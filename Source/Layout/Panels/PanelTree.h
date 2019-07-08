/*
  ==============================================================================

    PanelTree.h
    Created: 22 Apr 2018 11:17:02am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../ResizableLayoutManager.h"

class PanelNode;
class Panel;

//==============================================================================
// Base class for Panels
class PanelTreeEmbedded
{
public:
    struct PanelPreferences
    {
        float minWidth;
        float maxWidth;
        float minHeight;
        float maxHeight;
        
        float defaultWidth;
        float defaultHeight;
        
        static PanelPreferences defaultPreferences() { return PanelPreferences({ 100, -1, 24, -1, 400, 400 }); }
    };
    
    virtual ~PanelTreeEmbedded() {}
    virtual PanelPreferences getPanelPreferences() { return PanelPreferences::defaultPreferences(); }
};

//==============================================================================
enum DropZoneSide
{
    SideTop = 0,
    SideBottom,
    SideLeft,
    SideRight,
    TabBar
};

//==============================================================================
class PanelTree;

struct DropZone
{
    DropZone (PanelTree* t, PanelNode* n, int i, juce::Rectangle<int> b, DropZoneSide s) : tree (t), node (n), insertIndex (i), bounds (b), side (s) {}
    ~DropZone() { masterReference.clear(); }
    
    PanelTree *const tree;
    PanelNode *const node;
    const int insertIndex;
    const juce::Rectangle<int> bounds;
    const DropZoneSide side;
    
    bool highlighted = false;
    
private:
    WeakReference<DropZone>::Master masterReference;
    friend class WeakReference<DropZone>;
};
//==============================================================================

class PanelTree;
class TabbedPanel;
class PanelManager;

class PanelNode : public Component
{
public:
    PanelNode (bool isVertical, PanelTree* tree);
    ~PanelNode() {}
    
    void resized() override;
    
    PanelNode* addChildNode (float defaultSize = -1., float minSize = 24, float maxSize = -1.);
    void setComponent (Component* c);
    
    TabbedPanel* replaceContentWithTabbedPanel();
    
    XmlElement* getStateAsXml (String name);
    void restoreStateFromXml (XmlElement* e);
    void restoreLayoutFromXml (XmlElement* e);
    
private:
    friend class PanelTree;
    
    int getNumChildren() const { return children.size(); }
    
    bool tryToCloseAllPanels();
    bool closeAllPanels();
    
    // If after removal, this node is empty, it will ask its parent to delete itself. Beware if
    // you use any reference to this node after this method call!
    PanelNode* removeChildNode (PanelNode* n);
    
    // This will first try to put a corresponding comp. If there's already a corresponding comp but no children,
    // it will first move this comp to a new child before creating a new child node for the new comp to insert.
    void insertComponent (Component* c, int index);
    
    void insertNode (PanelNode* newChild, int index);
    void moveNode (int oldIndex, int newIndex);
    void moveNode (PanelNode* n, int newIndex);
    
    // Searches recursively for possible drop zones.
    void getDropZones (OwnedArray<DropZone>& result, bool hideTabs, juce::Rectangle<int> boundsInRoot = juce::Rectangle<int>());
    
    int getIndexOfChildNode (PanelNode* node) const;
    // Searches recursively for a node with a given corresponding comp
    PanelNode* findNodeWithComponent (Component* c);
    
    PanelNode* getParentNode() const { return dynamic_cast<PanelNode*>(getParentComponent()); }
    PanelTree* getParentTree() const { return findParentComponentOfClass<PanelTree>(); }
    
    PanelManager* getPanelManager() const;
    
    ResizableLayoutManager& getLayoutManager() { return layout; }
    
    void refreshPanelPreferences();
    
    ResizableLayoutManager layout;
    OwnedArray<PanelNode> children;
    WeakReference<PanelTree> panelTree;
    
    WeakReference<Component> correspondingComp;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelNode)
};

//==============================================================================
class TabbedPanel;
class PanelWindow;

class PanelTree : public Component
{
public:
    PanelTree (PanelManager& manager);
    ~PanelTree();
    
    void resized() override;
    
    PanelNode& getRootItem();
    
    void moveContainedPanel (Panel* panel, DropZone* zone);
    
    void prepareDropOverlay (bool hideTabs);
    void setDropOverlayVisible (bool shouldBeVisible);
    
    DropZone* lookForDropAreaAt (Point<int> pos);
    
    PanelNode* releaseRoot();
    void setRoot (PanelNode* panel);
    
    void refreshPanelPreferences();
    
    bool tryToCloseAllPanels();
    void closeParentWindow();
    
    void showPanelInTabs (Panel* panel);
    
    void insertPanel (Panel* panel, DropZone* zoneToDropTo);
    bool findAndRemovePanel (Panel* panel);
    
    XmlElement* getStateAsXml();
    void restoreFromXml (XmlElement* e);
    
    void checkTabbedPanelSize (TabbedPanel* panel);
    
private:
    void movePanelFromNode (Panel* panel, PanelNode* node, DropZone* zoneToDropTo);
    void movePanelFromTabs (Panel* panel, DropZone* zoneToDropTo);
    
    //==============================================================================
    class DropOverlay : public Component
    {
    public:
        DropOverlay (PanelTree& o) : owner(o) {}
        ~DropOverlay() {}
        
        void paint (Graphics& g) override;
        
    private:
        PanelTree& owner;
    };
    //==============================================================================
    
    WeakReference<PanelTree>::Master masterReference;
    friend class WeakReference<PanelTree>;
    
    friend class DropOverlay;
    friend class PanelNode;
    
    DropOverlay overlay;
    
    ScopedPointer<PanelNode> root;
    OwnedArray<DropZone> dropZones;
    OwnedArray<TabbedPanel> tabbedPanels;
    
    PanelManager& panelManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelTree)
};
