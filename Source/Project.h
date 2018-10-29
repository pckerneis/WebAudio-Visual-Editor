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
    ~Project();
    
    /** \brief Returns the project name (which is the header file name without extension)
     */
    String getName() const;
    
    /** \brief The project directory is the parent directory to a project header file
     *  and it contains the output data (HTML5, audio files,...).
     */
    const File& getProjectDirectory() const;
    
    /** \brief Is this project marked as temporary ?
     *
     *  When a new project is created in the OS temporary directory, it is set as
     *  temporary so that its project directory get deleted when the project is saved
     *  for the first time (or when its closed without saving).
     *
     */
    bool isTemporary() const { return temporary; }
    
    //==============================================================================
    ProjectManager& getProjectManager() const { return projectManager; }
    ProjectCommandTarget& getCommandTarget() { return commandTarget; }
    
    Result saveAs (const File & f);
    Result save();
    Result saveAs();
    
    /** \brief Asks the user to save changes if any. Closes the project if the
     *  action doesn't get canceled by the user.
     */
    Result tryToClose();
    
    //==============================================================================
    // These could be replaced with SharedResourcePointers...
    static AudioDeviceManager& getAudioDeviceManager();
    static ApplicationCommandManager& getApplicationCommandManager();
    
    //==============================================================================
    /** \brief Returns the panel manager for this project. Each project has its own
     *  panel manager so that panels from different can't be 'mixed' in a same window
     */
    PanelManager& getPanelManager();
    
    /** \brief Returns the static panels for this project.
     */
    const OwnedArray<Panel>& getStaticPanels() const { return staticPanels; }
    
    /** \brief Returns the dynamic panels for this project such as script editors
     */
    Array<WeakReference<Panel>> getDynamicPanels() const;
    
    template<class T>
    /** \brief Tries to find a static panel given a class name.
     */
    T* findStaticPanelWithClass() const;
    
    //==============================================================================
    /** \brief Returns the WebAudioGraphPanel. Implements NavigationPanel::Navigable.
     */
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    
    /** \brief Implements NavigationPanel::Navigable.
     */
    StringPairArray getNavigationInfo() const override;
    
    //==============================================================================
    /** \brief Does window contains all static panels ? This is used to know if the
     *  fact of closing this window should close the project.
     */
    bool windowContainsAllStaticPanels (PanelWindow* window);
    
    /** \brief Closes all windows for this project and restores the default panel layout.
     */
    void restoreDefaultPanelLayout();
    
private:
    //To use weak references to this project
    WeakReference<Project>::Master masterReference;
    friend class WeakReference<Project>;
    
    // Only the ProjectManager can create project objects with the static methods.
    // other constructors shouldn't be used directly.
    friend class ProjectManager;
    friend class ProjectCommandTarget;
    
    static Project* createNewProject (String projectName, File destination, ProjectManager& pm);
    static Project* loadProject (String filePath, ProjectManager& pm);
    
    Project (ProjectManager& pm);
    Project (String n, String d, File f, File dir, ProjectManager& pm);
    
    //==============================================================================
    /** \brief Open default window asynced.
     */
    void handleAsyncUpdate() override;
    
    //==============================================================================
    /** \brief Creates a directory with dir's name and add a child directory to it named 'output'
     */
    static void createDirectories (const File & dir);
    
    /** \brief Tries to move the project directory to a new destination
     *
     *  Tries to make a copy of the current project directory to the destination, optionnaly
     *  deleting the source and removing the existing header files in the destination directory.
     *  If the directory couldn't be copied (forbidden access...), nothing happens and the
     *  project directory isn't changed.
     */
    void moveProjectDirectory (File destination, bool deleteSourceDirectory, bool removeHeaders = true);
    
    /** \brief Deletes recursively the current project directory. Used to delete temporary projects.
     */
    void deleteProjectDirectory();
    
    //==============================================================================
    /** \brief Saves the current config into a project header file.
     */
    bool saveProjectConfig (File destination);
    
    /** \brief Loads state from a project header file.
     */
    bool loadProjectConfig (const File & file);
    
    /** \brief Restores a PanelWindow state from its Xml representation.
     */
    void restorePanelWindow (XmlElement* state);
    
    /** \brief Restores the appearance of a panel from its Xml representation.
     *
     *  When loading a project, the layout is restored after the internal state
     */
    Panel* loadPanelLayout (XmlElement* state);
    
    /** \brief Restores the internal state of a panel from its Xml representation.
     */
    Panel* loadPanelState (XmlElement* state);
    
    /** \brief Tries to find a Panel with a given Xml tag name (set when saving the panel state).
     */
    Panel* findStaticPanel (XmlElement* state);
    
    //==============================================================================
    /** \brief Creates an instance for static panels.
     */
    void createStaticPanels();
    
    /** \brief Creates a new window with the default panel layout.
     */
    PanelWindow* openDefaultWindow();
    
    /** \brief Close all windows (without warning user!) and quit this project.
     *
     *  This method calls the project manager's quitProject method. This instance
     *  of Project will be destroyed when this method returns !
     */
    void closeWindowsAndQuit();
    
    /** \brief Shows interactive alert box to save changes to this project.
     */
    Result saveYesNoCancel();
    
    /** \brief Returns the project's undo manager.
     *
     *  Each project has its own undo manager which is mainly used for the WebAudioGraphPanel
     *  actions on elements (add/remove, connect, change appearance, change settings...).
     */
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
#include "EmbeddedFonts.h"

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

	// Just so that we're sure these object are created at startup and don't get destroyed during the app's lifetime
	SharedResourcePointer<LookAndFeelUpdater> lookAndFeelUpdater;
	SharedResourcePointer<EmbeddedFonts> embeddedFonts;

    ProjectCommandTarget commandTarget;
    ScopedPointer<StartWindow> startWindow;
    ScopedPointer<MainMenuBarModel> menuModel;

	OwnedArray<Project> projects;
    WeakReference<Project> activeProject;
    
    bool allowMultipleProjects = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ProjectManager)
};
