/*
  ==============================================================================

    ProjectCommandTarget.h
    Created: 28 Aug 2017 11:15:01pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
#include "CommandIDs.h"

class ProjectManager;
class Project;
class RootWebAudioGraphPanel;

/** \brief The 'root' ApplicationCommandTarget with project-management commands (save, load, close...)
 */
class ProjectCommandTarget
:   public ApplicationCommandTarget
{
public:
    ProjectCommandTarget (ProjectManager* pm, Project* p) : projectManager (pm), project (p) {}
    ~ProjectCommandTarget() {}
    
    void prepareCommandTarget();
    ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    
private:
    RootWebAudioGraphPanel* getWebAudioGraphPanel();
    UndoManager* getMainUndoManager();
    
    WeakReference<ProjectManager> projectManager;
    WeakReference<Project> project;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectCommandTarget)
};

//==============================================================================
/** \brief MenuBarModel used for OSX native menu bar and Windows top menu bars.
 */
class MainMenuBarModel : public MenuBarModel
{
public:
    MainMenuBarModel (ProjectManager& manager);
    ~MainMenuBarModel();
    
    StringArray getMenuBarNames() override;
    PopupMenu getMenuForIndex (int topLevelMenuIndex, const String &menuName) override;
    void menuItemSelected (int menuItemID, int topLevelMenuIndex) override;
    
private:
    void addPanelItems (PopupMenu& menu);
    
    PopupMenu extraMacMenuItems;
    
    ProjectManager& projectManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainMenuBarModel)
};
