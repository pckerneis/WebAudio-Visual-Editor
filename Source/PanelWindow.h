/*
  ==============================================================================

    PanelWindow.h
    Created: 30 Apr 2018 5:07:59pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "PanelTree.h"
#include "WaveLookAndFeel.h"

class Project;
class PanelManager;

/** \brief A panel window contains one or more panels embedded in a PanelTree. On Windows, it has a menu bar. */
class PanelWindow : public DocumentWindow, public ApplicationCommandTarget, public LookAndFeelUpdater::Listener, public DragAndDropContainer
{
public:
    PanelWindow (Panel* contentPanel, PanelManager& manager, Project& proj);
    ~PanelWindow();
    
    void closeButtonPressed() override;
    void closeWithoutWarning();
    
    XmlElement* getStateAsXml();
    void restoreFromXml (XmlElement* state);
    
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID> &commands) override {}
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo &result) override {}
    bool perform (const InvocationInfo &info) override { return false; }
    
    // Because of a crash when closing windows in kiosk mode, "real" fullscreen is not allowed...
    void maximiseButtonPressed() override;
    
private:
    void activeWindowStatusChanged() override;
    
    //==============================================================================
    // Only there to add some padding around the panel tree
    class PanelWindowContentHolder : public Component
    {
    public:
        PanelWindowContentHolder (PanelWindow& o, PanelTree& tree);
        
        void resized() override;
        
    private:
        PanelWindow& owner;
        PanelTree& panelTree;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelWindowContentHolder)
    };
    //==============================================================================

    friend class Project;
    friend class PanelManager;
    
    PanelWindow (PanelManager& manager, Project& project);
    
    PanelTree panelTree;
    PanelManager& panelManager;
    Project& project;
    PanelWindowContentHolder contentHolder;
    
	juce::Rectangle<int> previousBounds;
	    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PanelWindow)
};
