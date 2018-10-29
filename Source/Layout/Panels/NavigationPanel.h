/*
  ==============================================================================

    NavigationPanel.h
    Created: 24 Jul 2017 11:22:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "Panel.h"

//==============================================================================
class NavigationItem;
class MainWindowContent;

/** \brief A project navigation panel.
 *
 *  This panel shows a treeview of the current project setup and allows a quick access to the main elements of the project.
 */
class NavigationPanel
:   public Panel, public Timer
{
public:
    NavigationPanel (PanelManager* manager);
    ~NavigationPanel();
    
    //==============================================================================
    /** \brief Represents an item that can be added to the NavigationPanel. */
    class Navigable
    {
    public:        
        Navigable() {}
        virtual ~Navigable();
        
        /** \brief Returns the info to display in the NagigationPanel. */
        virtual StringPairArray getNavigationInfo() const { return StringPairArray(); }
        virtual Array<WeakReference<Navigable>> getSubNavigables() { return Array<WeakReference<Navigable>>(); }
        
        virtual void reveal (ModifierKeys mods) {}
        virtual void open () {}
        
        NavigationPanel* getNavigationPanel() const { return navigationPanel; }
        void setNavigationPanel (NavigationPanel* panel) { navigationPanel = panel; }
        
        /** \brief Alerts the NavigationPanel that something has changed */
        void navigableChanged() { if (navigationPanel) navigationPanel->refresh(); }
        
        void setNavigableUuid (Uuid newUuid) { uuid = newUuid; }
        const Uuid& getNavigableUuid() const { return uuid; }
        
    private:
        friend class NavigationPanel;
        WeakReference<NavigationPanel> navigationPanel;
        
        Uuid uuid;
        
        WeakReference<Navigable>::Master masterReference;
        friend class WeakReference<Navigable>;
    };
    
    //==============================================================================
    void setRootNavigable (Navigable* root);
    virtual void refresh (bool async = true);
    void selectionChanged();
    
    void resized() override;
    
    XmlElement* getAsXml() override;
    void restoreLayout (XmlElement* e) override;
    
    virtual Component* createComponentFor (NavigationItem* item, Navigable* navigable);
    
    void timerCallback() override;
    
private:
    WeakReference<NavigationPanel>::Master masterReference;
    friend class WeakReference<NavigationPanel>;
    
    /** \brief Recursively checks the item to update their selection state. */
    void recursiveSelectionUpdate (NavigationItem* item);
    
    TreeView navigationTree;
    ScopedPointer<NavigationItem> rootItem;
    WeakReference<Navigable> rootNavigable;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationPanel)
};

//==============================================================================
/** \brief An item in the NavigationPanel. */
class NavigationItem    :   public TreeViewItem
{
public:
    /** \brief Constructor. */
    NavigationItem (NavigationPanel& np, NavigationPanel::Navigable* navigableItem);
    
    // TreeViewItem implementation...
    String getUniqueName() const override;
    bool mightContainSubItems() override;
    
    void setVisible (bool shouldBeVisible) { isVisible = shouldBeVisible; }
    void paintItem (Graphics& g, int width, int height) override;
    void itemOpennessChanged (bool isNowOpen) override;
    
    // Object creation is done by the NavigationPanel
    Component* createItemComponent() override { return navigationPanel.createComponentFor (this, navigable); }
    
    void itemClicked (const MouseEvent &e) override;
    void itemDoubleClicked (const MouseEvent &e) override;
    
    void refreshSubItems();
    void updateSelectionState();
    
    int getItemHeight() const override { return 18; }
    
    //==============================================================================
    class ItemMouseTarget : public Component
    {
    public:
        ItemMouseTarget (NavigationItem& ownerItem) : owner (ownerItem) {}
        
        void mouseUp (const MouseEvent& e)
        {
            if (e.mouseWasClicked())
                owner.itemClicked (e.getEventRelativeTo (&owner.navigationPanel));
        }
        
    private:
        friend class ItemMouseTarget;
        
        NavigationItem& owner;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemMouseTarget)
    };
    //==============================================================================
    
private:
    NavigationPanel& navigationPanel;
    WeakReference<NavigationPanel::Navigable> navigable;
    bool isVisible = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationItem)
};
