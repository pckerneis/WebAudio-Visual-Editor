/*
  ==============================================================================

    ProjectCommandTarget.cpp
    Created: 28 Aug 2017 11:15:01pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "ProjectCommandTarget.h"

#include "Project.h"
void ProjectCommandTarget::prepareCommandTarget()
{
    auto& commandManager = Project::getApplicationCommandManager();
    
    if (auto thisComp = dynamic_cast<Component*>(this))
    {
        thisComp->addKeyListener (commandManager.getKeyMappings());
        thisComp->setWantsKeyboardFocus(true);
    }
    
    commandManager.registerAllCommandsForTarget (this);
}

void ProjectCommandTarget::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::projectNew,
        CommandIDs::projectLoad,
        CommandIDs::projectSave,
        CommandIDs::projectSaveAs,
        CommandIDs::projectClose,
        
        // Undo/Redo
        CommandIDs::undo,
        CommandIDs::redo,
        
        // Dummy commands
        CommandIDs::cut,
        CommandIDs::copy,
        CommandIDs::paste,
        CommandIDs::duplicateSelection,
        CommandIDs::del,
        CommandIDs::selectAll,
        CommandIDs::deselectAll,
        CommandIDs::rename,
        
        // Web audio
        CommandIDs::generateOutput,
        CommandIDs::testInBrowser,
        CommandIDs::revealOutputDirectory,
        
        // General
        CommandIDs::openPreferences,
        
        // Layout
        CommandIDs::restoreDefaultPanelLayout
    };
    
    commands.addArray (ids, numElementsInArray (ids));
}

#include "WebAudioGraph.h"
void ProjectCommandTarget::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    const auto graph = getWebAudioGraphPanel();
    const bool projectOpen = project != nullptr;
    const bool outputDirectoryExists = graph == nullptr ? false : graph->getOutputDirectory().exists();
    const String projectCategory ("Project");
    const String editCategory ("Edit");
    
    bool canRedo = false;
    bool canUndo = false;
    
    if (auto um = getMainUndoManager())
    {
        canRedo = um->canRedo();
        canUndo = um->canUndo();
    }
    
    switch (commandID)
    {
        case CommandIDs::projectNew :
            result.setInfo ("New Project", "Create a new project", projectCategory, 0);
            result.addDefaultKeypress ('n', ModifierKeys::commandModifier);
            break;
        case CommandIDs::projectLoad :
            result.setInfo ("Open Project...", "Open a project", projectCategory, 0);
            result.addDefaultKeypress ('o', ModifierKeys::commandModifier);
            break;
        case CommandIDs::projectSave :
            result.setInfo ("Save Project", "Save the current project", projectCategory, 0);
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier);
            result.setActive (projectOpen);
            break;
        case CommandIDs::projectSaveAs :
            result.setInfo ("Save Project as...", "Save project as a new file", projectCategory, 0);
            result.addDefaultKeypress ('s', ModifierKeys::commandModifier | ModifierKeys::shiftModifier );
            result.setActive (projectOpen);
            break;
        case CommandIDs::projectClose :
            result.setInfo ("Close Project", "Close the current project", projectCategory, 0);
            result.addDefaultKeypress ('w', ModifierKeys::commandModifier );
            result.setActive (projectOpen);
            break;
            
            // DUMMY COMMANDS
        case CommandIDs::undo :
            result.setInfo ("Undo", "Undo the last action", editCategory, 0);
            result.addDefaultKeypress ('z', ModifierKeys::commandModifier);
            result.setActive (canUndo);
            break;
        case CommandIDs::redo :
            result.setInfo ("Redo", "Redo previously canceled action", editCategory, 0);
            result.addDefaultKeypress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setActive (canRedo);
            break;
        case CommandIDs::cut :
            result.setInfo ("Cut", "Cut the selection to clipboard", editCategory, 0);
            result.addDefaultKeypress ('x', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
        case CommandIDs::copy :
            result.setInfo ("Copy", "Copy the selection to clipboard", editCategory, 0);
            result.addDefaultKeypress ('c', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
        case CommandIDs::paste :
            result.setInfo ("Paste", "Paste from the clipboard", editCategory, 0);
            result.addDefaultKeypress ('v', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
        case CommandIDs::duplicateSelection :
            result.setInfo ("Duplicate", "Duplicate the selection", editCategory, 0);
            result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
        case CommandIDs::del :
            result.setInfo ("Delete", "Delete the selection", editCategory, 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::noModifiers);
            result.addDefaultKeypress (KeyPress::deleteKey, ModifierKeys::noModifiers);
            result.setActive (false);
            break;
        case CommandIDs::selectAll :
            result.setInfo ("Select all", "Add all the content to the current selection", editCategory, 0);
            result.addDefaultKeypress ('a', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
        case CommandIDs::deselectAll :
            result.setInfo ("Deselect all", "Clears the current selection", editCategory, 0);
            result.addDefaultKeypress ('a', ModifierKeys::commandModifier  | ModifierKeys::shiftModifier);
            result.setActive (false);
            break;
        case CommandIDs::rename :
            result.setInfo ("Rename", "Renames the selected item", editCategory, 0);
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            result.setActive (false);
            break;
            
            // Web audio
        case CommandIDs::generateOutput :
            result.setInfo ("Generate project", "Generates the output files", "Generate", 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier);
            result.setActive (projectOpen);
            break;
            
        case CommandIDs::testInBrowser :
            result.setInfo ("Test in default browser", "Generates and opens in default browser", "Generate", 0);
            result.addDefaultKeypress ('b', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setActive (projectOpen);
            break;
            
        case CommandIDs::revealOutputDirectory :
            result.setInfo ("Reveal output directory", "Reveal output directory", "Generate", 0);
            result.addDefaultKeypress ('k', ModifierKeys::commandModifier);
            result.setActive (outputDirectoryExists);
            break;
        
        /*
        case CommandIDs::clearConsole :
            result.setInfo ("Clear console", "Clears the debugger console", "Script Engine", 0);
            //result.addDefaultKeypress ('k', ModifierKeys::commandModifier);
            result.setActive (projectOpen);
            break;
         */
            
            // General
        case CommandIDs::openPreferences :
            result.setInfo ("Preferences...", "Opens the app preferences window", "WebAudio Visual Editor", 0);
            result.addDefaultKeypress (',', ModifierKeys::commandModifier);
            break;
            
        case CommandIDs::restoreDefaultPanelLayout :
            result.setInfo ("Restore default layout", "Restores the panel layout back to default", "Window", 0);
            result.setActive (projectOpen);
            break;
            
        default:
            break;
    }
}

#include "PreferencesWindow.h"
bool ProjectCommandTarget::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::projectNew :
            projectManager->createNewProject();
            break;
            
        case CommandIDs::projectLoad :
            projectManager->loadProject();
            break;
            
        case CommandIDs::projectSave :
            if (project != nullptr)
                project->save();
            break;
            
        case CommandIDs::projectSaveAs :
            if (project != nullptr)
                project->saveAs();
            break;
            
        case CommandIDs::projectClose :
            if (project != nullptr)
                project->tryToClose();
            break;
            
            // Web audio
        case CommandIDs::generateOutput:
            if (auto graph = getWebAudioGraphPanel())
                graph->generateOutput (true);
            break;
            
        case CommandIDs::testInBrowser:
            if (auto graph = getWebAudioGraphPanel())
                graph->testInBrowser();
            break;
            
        case CommandIDs::revealOutputDirectory:
            if (auto graph = getWebAudioGraphPanel())
                graph->revealOutputDirectory();
            break;
            
            // General
        case CommandIDs::openPreferences :
            PreferencesWindow::show();
            break;
            
        case CommandIDs::restoreDefaultPanelLayout :
            if (project != nullptr)
                project->restoreDefaultPanelLayout();
            break;
            
            // Undo/Redo
        case CommandIDs::undo :
            if (auto um = getMainUndoManager())
                um->undo();
            break;
            
        case CommandIDs::redo :
            if (auto um = getMainUndoManager())
                um->redo();
            break;
            
        default:
            break;
    }
    
    return true;
}

RootWebAudioGraphPanel* ProjectCommandTarget::getWebAudioGraphPanel()
{
    if (project == nullptr)
        return nullptr;
    
    return project->findStaticPanelWithClass<RootWebAudioGraphPanel>();
}

UndoManager* ProjectCommandTarget::getMainUndoManager()
{
    if (auto graph = getWebAudioGraphPanel())
        return &graph->getUndoManager();
    
    return nullptr;
}
//==============================================================================

MainMenuBarModel::MainMenuBarModel (ProjectManager& manager) : projectManager (manager)
{
    auto& acm = Project::getApplicationCommandManager();
    setApplicationCommandManagerToWatch(&acm);
    
    extraMacMenuItems.addItem (1, "About WebAudio Visual Editor");
    extraMacMenuItems.addSeparator();
    extraMacMenuItems.addItem (2, "Preferences...");

#if JUCE_MAC
	MenuBarModel::setMacMainMenu(this, &extraMacMenuItems);
#endif // JUCE_MAC
}

MainMenuBarModel::~MainMenuBarModel()
{
#if JUCE_MAC
    MenuBarModel::setMacMainMenu(nullptr);
#endif // JUCE_MAC
}

StringArray MainMenuBarModel::getMenuBarNames()
{
    StringArray arr;
    arr.add ("File");
    arr.add ("Edit");
    arr.add ("Generate");
    arr.add ("Window");
    return arr;
}

PopupMenu MainMenuBarModel::getMenuForIndex (int topLevelMenuIndex, const String &menuName)
{
    auto& acm = Project::getApplicationCommandManager();
    
    if (menuName == "File")
    {
        PopupMenu popup;
        
        popup.addCommandItem(&acm, CommandIDs::projectNew);
        popup.addCommandItem(&acm, CommandIDs::projectLoad);
        
        PopupMenu recentSubMenu;
        auto recentList = AppSettings::getRecentlyOpenedProjectsList();
        // It'd better to have inative items for non existent files 
        recentList.createPopupMenuItems (recentSubMenu, 2, false, true);
        
        recentSubMenu.addSeparator();
        recentSubMenu.addItem (1, "Clear recent");
        
        popup.addSubMenu ("Open recent...", recentSubMenu);
        
        popup.addCommandItem(&acm, CommandIDs::projectSave);
        popup.addCommandItem(&acm, CommandIDs::projectSaveAs);
        popup.addSeparator();
        popup.addCommandItem(&acm, CommandIDs::projectClose);
        
        return popup;
    }
    else if (menuName == "Edit")
    {
        PopupMenu popup;
        
        popup.addCommandItem(&acm, CommandIDs::undo);
        popup.addCommandItem(&acm, CommandIDs::redo);
        popup.addSeparator();
        popup.addCommandItem(&acm, CommandIDs::cut);
        popup.addCommandItem(&acm, CommandIDs::copy);
        popup.addCommandItem(&acm, CommandIDs::paste);
        popup.addCommandItem(&acm, CommandIDs::duplicateSelection);
        popup.addCommandItem(&acm, CommandIDs::del);
        popup.addSeparator();
        popup.addCommandItem(&acm, CommandIDs::selectAll);
        popup.addCommandItem(&acm, CommandIDs::deselectAll);
        popup.addSeparator();
        popup.addCommandItem(&acm, CommandIDs::rename);
        popup.addSeparator();
        popup.addCommandItem(&acm, CommandIDs::showFindPanel);
        popup.addCommandItem(&acm, CommandIDs::findNext);
        popup.addCommandItem(&acm, CommandIDs::findPrevious);
        //popup.addSeparator();
        //popup.addCommandItem(&acm, CommandIDs::applyChanges);
        
        return popup;
    }
    else if (menuName == "Generate")
    {
        PopupMenu popup;
        
        popup.addCommandItem(&acm, CommandIDs::generateOutput);
        popup.addCommandItem(&acm, CommandIDs::testInBrowser);
        popup.addCommandItem(&acm, CommandIDs::revealOutputDirectory);
        
        return popup;
    }
    else if (menuName == "Window")
    {
        PopupMenu popup;
        
        addPanelItems (popup);
        
        return popup;
    }
    else
        return PopupMenu();
}

void MainMenuBarModel::addPanelItems (PopupMenu& menu)
{
    auto project = projectManager.getActiveProject();
    
    if (project == nullptr)
        return;
    
    menu.addCommandItem (&Project::getApplicationCommandManager(), CommandIDs::restoreDefaultPanelLayout);
    
    menu.addSeparator();
    
    auto& staticPanels = project->getStaticPanels();
    
    for (int i = 0; i < staticPanels.size(); ++i)
    {
        const String name = staticPanels.getUnchecked(i)->getPanelName();
        const bool showing = staticPanels.getUnchecked(i)->isShowing();
        
        menu.addItem (i + 1, name, true, showing);
    }
    
    menu.addSeparator();
    
    auto dynamicPanels = project->getDynamicPanels();
    
    for (int i = 0; i < dynamicPanels.size(); ++i)
    {
        const auto panel = dynamicPanels.getUnchecked(i);
        const String name = panel->getPanelName();
        const bool showing = panel->isShowing();
        const bool canBeRevealed = showing || panel->isInTab();
        
        if (canBeRevealed)
            menu.addItem (i + 1 + staticPanels.size(), name, true, showing);
    }
}

#include "AboutWindow.h"
#include "StartWindow.h"
void MainMenuBarModel::menuItemSelected (int menuItemID, int topLevelMenuIndex)
{
    if (topLevelMenuIndex == -1)
    {
        if (menuItemID == 1)    // About
        {
            AboutScreen::show();
        }
        else if (menuItemID == 2)
        {
            Project::getApplicationCommandManager().invokeDirectly (CommandIDs::openPreferences, false);
        }
    }
    else if (topLevelMenuIndex == 0)
    {
        auto recentList = AppSettings::getRecentlyOpenedProjectsList();
        
        if (menuItemID  == 1)
        {
            AppSettings::clearRecentlyOpenedProjectsList();
            menuItemsChanged();
            StartWindow::refresh();
        }
        else
        {
            const int recentIndex = menuItemID - 2;
            
            if (recentIndex < recentList.getNumFiles())
                projectManager.loadProject (recentList.getFile (menuItemID - 2));
        }
    }
    else if (topLevelMenuIndex == 3)
    {
        auto project = projectManager.getActiveProject();
        
        if (project == nullptr)
            return;
        
        auto& staticPanels = project->getStaticPanels();
        auto dynamicPanels = project->getDynamicPanels();
        
        if (auto panel = staticPanels [menuItemID - 1])
            panel->reveal();
        
        if (auto panel = dynamicPanels [menuItemID - staticPanels.size() - 1])
            panel->reveal();
    }
}
