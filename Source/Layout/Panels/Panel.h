/*
  ==============================================================================

    Panel.h
    Created: 31 Aug 2017 8:42:59pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "PanelTree.h"

class PanelManager;
class TabbedPanel;

/** \brief A panel is a section of the app interface that can be moved around, tabbed, hidden... */
class Panel : public Component, public PanelTreeEmbedded, public ApplicationCommandTarget
{
public:
    Panel (PanelManager* manager, bool wantsMouseListener = true);
    ~Panel();
    
    void paint (Graphics& g) override;
    void resized() override;
    
    void mouseDown (const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent& e) override;
    
    void gainFocus();
    bool hasPanelFocus() const { return hasFocus; }
    
    void setPanelManager (PanelManager* pm);
    
    void reveal();
    
    virtual void hasGainedFocus() {}
    virtual void hasLostFocus() {}
    
    virtual void setPanelName (String name);
    String getPanelName() const { return panelName; }
    
    // Returns true if the panel can be closed. False could indicate a user cancel
    virtual bool tryToClosePanel() { return true; }
    virtual void panelWasClosed() {}
    
    void sendPanelToFront();
    
    bool isInTab() const { return getParentTabbedPanel() != nullptr; }
    void setParentTabbedPanel (TabbedPanel* panel);
    TabbedPanel* getParentTabbedPanel() const;
    void showInTabs();
    
    PanelManager* getPanelManager() const { return panelManager; }
    PanelTree* getParentTree() const;
    PanelWindow* getParentPanelWindow() const;
    
    // Default implementation only care about the panel ID
    virtual XmlElement* getAsXml();
    virtual void restoreState (XmlElement* e);
    virtual void restoreLayout (XmlElement* e) {}
    
    int getPanelId() const { return panelId; }
    void setPanelId (int newId) { panelId = newId; }
    
    ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
    void getAllCommands (Array<CommandID> &commands) override {}
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo &result) override {}
    bool perform (const InvocationInfo &info) override { return false; }
    
protected:
    int getHeaderHeight() const { return isInTab() || panelName == String() ? 0 : 26; }
    void sendPanelBorderToFront() { border.toFront (false); }
    
private:
    WeakReference<Panel>::Master masterReference;
    friend class WeakReference<Panel>;
    
    //==============================================================================
    /** \brief A border visible when a panel has focus. */
    class FocusBorder : public Component
    {
    public:
        FocusBorder (Panel& p) : panel (p) {}
        ~FocusBorder () {}
        
        void paint (Graphics& g) override;
        
    private:
        Panel& panel;
    };
    //==============================================================================
    /** \brief A mouse listener allowing a Panel to grab focus. */
    class PanelMouseListener : public MouseListener
    {
    public:
        /** \brief Constructor. */
        PanelMouseListener(Panel& p) : panel (p) {}
        /** \brief Empty destructor. */
        ~PanelMouseListener() {}
        
        /** \brief Gives focus to the Panel when mouse is pressed. */
        void mouseDown (const MouseEvent& e) override;
        
    private:
        Panel& panel;
    };
    //==============================================================================
    
    friend class PanelManager;
    /** \brief Sets the current focus state for this. Only the PanelManager should call this method. */
    void setFocus (bool shouldHaveFocus);
    
    WeakReference<PanelManager> panelManager;
    WeakReference<TabbedPanel> parentTabbedPanel;
    
    bool hasFocus = false;
    FocusBorder border;
    PanelMouseListener mouseListener;
    
    int panelId = -1;
    
    String panelName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Panel)
};

//==============================================================================
class Project;

/** \brief A class to manage the focus of Panel objects. */
class PanelManager
{
public:
    /** \brief Empty constructor. */
    PanelManager (Project& project);
    /** \brief Empty destructor. */
    ~PanelManager();
    
    /** \brief Gives the focus to a unique Panel. */
    void setFocus (Panel* panelToGiveFocusTo);
    
    /** \brief Enables focus : call setFocus() with the last Panel that had the focus. */
    void enableFocus();
    /** \brief disable focus : call setFocus() with nullptr. */
    void disableFocus();
    
    PanelWindow* createAndAddPanelWindow (Panel* content, juce::Rectangle<int> bounds = juce::Rectangle<int>());
    void addPanelWindow (PanelWindow* w);
    void removePanelWindow (PanelWindow* w);
    void closeAllWindows();
    void quitFullScreenMode();
    
    void startDragging (Panel* panel, const MouseEvent& e);
    void dragPanel (Panel* panel, const MouseEvent& e);
    void stopDragging (Panel* panel, const MouseEvent& e);
    
    String getWindowsTitle() const;
    void refreshWindowsTitle();
    
    OwnedArray<PanelWindow>& getWindows() { return panelWindows; }
    Array<WeakReference<Panel>>& getPanels() { return panels; }
    
    Panel* findPanelWithId (int idToLookFor);
    
private:
    //==============================================================================
    /** \brief Component that is being dragged when a panel is dragged outside of its window. */
    class ComponentScreenshot : public Component
    {
    public:
        ComponentScreenshot() {}
        
        void paint (Graphics& g) override;
        
        Image image;
    };
    //==============================================================================

    WeakReference<PanelManager>::Master masterReference;
    friend class WeakReference<PanelManager>;
    
    friend class Panel;
    // Shouldn't be called directly : use Panel::setPanelManager() instead.
    void addPanel (Panel* panel);
    // Shouldn't be called directly : use Panel::setPanelManager() instead.
    void removePanel (Panel* panel);
    
    Project& project;
    
    Array<WeakReference<Panel>> panels;
    OwnedArray<PanelWindow> panelWindows;
    
    WeakReference<Panel> lastFocus;
    
    WeakReference<DropZone> dropZone;
    ComponentScreenshot screenshot;
    
    int previousPanelId = -1;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelManager)
};
