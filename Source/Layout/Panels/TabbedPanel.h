/*
  ==============================================================================

    TabbedPanel.h
    Created: 30 Apr 2018 10:34:41am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "Panel.h"

#include "../Widgets/ShapeButtons.h"

class TabbedPanel : public Panel, public Button::Listener
{
public:
    TabbedPanel (PanelManager* manager);
    ~TabbedPanel();
    
    void resized() override;
    
    int getNumTabs() const { return tabbedComponent.getNumTabs(); }
    void addPanelAsTab (Panel* panel, int insertIndex = -1);
    void removePanel (Panel* panel);
    Panel* getTabbedPanel (int index) const { return dynamic_cast<Panel*>(tabbedComponent.getTabContentComponent (index)); }
    
    bool isPanelOpenedAsTab (Panel* panel) const;
    int getTabIndexForPanel (Panel* panel) const;
    
    void setCurrentTab (int index);
    
    Array<juce::Rectangle<int>> getTabButtonBounds() const;
    
    void panelNameChanged (Panel* p);
    
    XmlElement* getAsXml() override;
    void restoreState (XmlElement* state) override;
    void panelWasClosed() override;
    
private:
    friend class TabbedPanelBarButton;
    WeakReference<TabbedPanel>::Master masterReference;
    friend class WeakReference<TabbedPanel>;
    
    void buttonClicked (Button* b) override;
    void closeTabButtonClicked (int indexToClose);
    void showTabButtonClicked (int tabNumber);
    
    //==============================================================================
    /** \brief Custom button with a close button for the TabHolder. */
    class TabbedPanelBarButton : public TabBarButton
    {
    public:
        TabbedPanelBarButton (const String &name, TabbedButtonBar &ownerBar, TabbedPanel &ownerTabbed, bool closeButtonEnabled);
        ~TabbedPanelBarButton() {}
        
        void createAndAddCloseButton (TabbedPanel *panel);
        
        void mouseDown (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        
    private:
        TabbedPanel& owner;
        
        SafePointer<ShapeButton> closeButton;
    };
    
    void mouseDownOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e);
    void mouseDragOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e);
    void mouseUpOnTabButton (TabbedPanelBarButton* button, const MouseEvent& e);
    
    //==============================================================================
    class PanelTabbedComponent : public TabbedComponent
    {
    public:
        PanelTabbedComponent(TabbedPanel& o);
        
        /** \brief Creates a custom tab button with a close button. */
        TabBarButton* createTabButton (const String& tabName, const int tabIndex) override;
        
        void lookAndFeelChanged() override;
        void resized() override;
        
        static Colour getTabBackgroundColour();
        
    private:
        TabbedPanel& owner;
    };
    
    //==============================================================================
    // Defined in TabbedPanel.cpp
    class TabbedBarMouseListener;
    
    PanelTabbedComponent tabbedComponent;
    ScopedPointer<MouseListener> mouseListener;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TabbedPanel)
};
