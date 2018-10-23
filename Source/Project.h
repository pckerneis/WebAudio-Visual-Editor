/*
  ==============================================================================

    Project.h
    Created: 6 Jul 2017 3:15:49pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "NavigationPanel.h"
#include "ProjectCommandTarget.h"

class ProjectManager;

class Project : private AsyncUpdater, public NavigationPanel::Navigable
{
public:
    static Project* createNewProject (String projectName, File destination, ProjectManager& pm);
    static Project* loadProject (String filePath, ProjectManager& pm);
    ~Project();
    
    ProjectManager& getProjectManager() const { return projectManager; }
    ProjectCommandTarget& getCommandTarget() { return commandTarget; }
    
    //==============================================================================
    bool isTemporary() const { return temporary; }
    
    Result saveAs(const File & f);
    Result save();
    Result saveAs();
    Result tryToClose();
    
    const File& getProjectDirectory() const;
    
    static void createDirectories (const File & dir);
    void moveProjectDirectory (File destination, bool deleteSourceDirectory, bool removeHeaders = true);
    void deleteProjectDirectory();
    
    //==============================================================================
    String getName() const;
    
    //==============================================================================
    static AudioDeviceManager& getAudioDeviceManager();
    static ApplicationCommandManager& getApplicationCommandManager();
    
    //==============================================================================
    PanelManager& getPanelManager();
    
    const OwnedArray<Panel>& getStaticPanels() const { return staticPanels; }
    Array<WeakReference<Panel>> getDynamicPanels() const;
    
    template<class T>
    T* findStaticPanelWithClass() const;
    
    //==============================================================================
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    StringPairArray getNavigationInfo() const override;
    
    //==============================================================================
    Panel* findPanelWithId (int panelId);
    
    //==============================================================================
    bool windowContainsAllStaticPanels (PanelWindow* window);
    void restoreDefaultPanelLayout();
    
private:
    //To use weak references to this project
    WeakReference<Project>::Master masterReference;
    friend class WeakReference<Project>;
    
    friend class ProjectManager;
    friend class ProjectCommandTarget;
    
    Project (ProjectManager& pm);
    Project (String n, String d, File f, File dir, ProjectManager& pm);
    
    //==============================================================================
    // Open default window async
    void handleAsyncUpdate() override;
    
    //==============================================================================
    bool saveProjectConfig (File destination);
    bool loadProjectConfig (const File & file);
    
    //friend class PanelNode;
    
    void restorePanelWindow (XmlElement* state);
    Panel* loadPanelLayout (XmlElement* state);
    Panel* loadPanelState (XmlElement* state);
    Panel* findStaticPanel (XmlElement* state);
    
    void createStaticPanels();
    PanelWindow* openDefaultWindow();
    
    void quit();
    Result saveYesNoCancel();
    
    UndoManager* getMainUndoManager();
    
    bool temporary = true;
    
    String name;
    String creationDate;
    File projectFile;
    File projectDirectory;
    
    OwnedArray<Panel> staticPanels;
    
    ProjectManager& projectManager;
    PanelManager panelManager;
    
    ProjectCommandTarget commandTarget;
    
    static String projectFileTagName;
    static String projectFileExtension;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Project)
};

template<class PanelType>
PanelType* Project::findStaticPanelWithClass() const
{
    for (auto p : staticPanels)
        if (auto tp = dynamic_cast<PanelType*> (p))
            return tp;
    
    jassertfalse;
    
    return nullptr;
}

//==============================================================================
#include "WaveLookAndFeel.h"

class StartWindow;
class MainMenuBarModel;

class ProjectManager
{
public:
    ProjectManager();
    ~ProjectManager();
    
    void createNewProject();
    
    bool loadProject (File projectHeader);
    bool loadProject();
    
    const bool hasProjectOpen() { return ! projects.isEmpty(); }
    
    bool tryToCloseProject (Project* project);
    void quitProject (Project* p);
    bool tryToQuitApplication();
    
    void openStartWindow();
    void closeStartWindow();
    
    ProjectCommandTarget& getCommandTarget() { return commandTarget; }
    
    Project* getActiveProject() const { return activeProject; }
    void setActiveProject (Project* project) { activeProject = project; }

	MainMenuBarModel* getMenuBarModel() { return menuModel; }
    
private:
    WeakReference<ProjectManager>::Master masterReference;
    friend class WeakReference<ProjectManager>;
    
    SharedResourcePointer<LookAndFeelUpdater> lookAndFeelUpdater;
    ProjectCommandTarget commandTarget;
    
    ScopedPointer<StartWindow> startWindow;
    OwnedArray<Project> projects;
    ScopedPointer<MainMenuBarModel> menuModel;
    
    WeakReference<Project> activeProject;
    
    bool allowMultipleProjects = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectManager)
};
