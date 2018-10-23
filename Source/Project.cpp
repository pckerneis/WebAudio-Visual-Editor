/*
  ==============================================================================

    Project.cpp
    Created: 6 Jul 2017 3:15:49pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/


#include "Project.h"

String Project::projectFileTagName = "WAVEProject";
String Project::projectFileExtension = ".waveproj";

ScopedPointer<AudioDeviceManager> audioDeviceManager;
ScopedPointer<ApplicationCommandManager> applicationCommandManager;

AudioDeviceManager& Project::getAudioDeviceManager()
{
    if (audioDeviceManager == nullptr)
    {
        audioDeviceManager = new AudioDeviceManager();
        audioDeviceManager->initialiseWithDefaultDevices (0, 2);
    }
    
    return *audioDeviceManager;
}

ApplicationCommandManager& Project::getApplicationCommandManager()
{
    if (applicationCommandManager == nullptr)
        applicationCommandManager = new ApplicationCommandManager();
    
    return *applicationCommandManager;
}

PanelManager& Project::getPanelManager()
{
    return panelManager;
}

Project::Project(ProjectManager& pm) : projectManager (pm), panelManager (*this), commandTarget (&pm, this)
{
    createStaticPanels();
    // This constructor is called when loading a project. We don't need to open the default window as the
    // panel layout will be restored anyway.
    //triggerAsyncUpdate();
}

#include "ProjectCommandTarget.h"
#include "AppSettings.h"
Project::Project(String n, String d, File f, File dir, ProjectManager& pm)
: creationDate(d), projectFile(f), projectDirectory (dir), projectManager (pm), panelManager (*this), commandTarget (&pm, this)
{
    createStaticPanels();
    // This constructor is called when creating a new project. To ensure the static panels are properly created
    // at the time we create the window, we use an async update.
    triggerAsyncUpdate();
}

void Project::handleAsyncUpdate()
{
    projectManager.closeStartWindow();
    
    openDefaultWindow();
}

Project* Project::createNewProject (String projectName, File destination, ProjectManager& pm)
{
    String creationDate = Time::getCurrentTime().toString (true, true, true, true);
    
    if (! destination.exists())
        destination.createDirectory();
    
    File projectDirectory = destination.getChildFile(projectName);
    createDirectories (projectDirectory);
    File projectConfig = projectDirectory.getChildFile(projectName).withFileExtension (projectFileExtension);
    
    ScopedPointer<Project> p = new Project (projectName, creationDate, projectConfig, projectDirectory, pm);
    
    if(! p->saveProjectConfig (projectConfig))
        return nullptr;
    
    return p.release();
}

Project* Project::loadProject (String filePath, ProjectManager& pm)
{
    File f (filePath);
    
    if (!f.exists())
        return nullptr;
    
    ScopedPointer<Project> p = new Project (pm);
    if (! p->loadProjectConfig (f))
        return nullptr;
    
    Project* project = p.release();
    return project;
}

Project::~Project()
{
    masterReference.clear();
}

void Project::closeWindowsAndQuit()
{
    panelManager.closeAllWindows();
    projectManager.quitProject (this);
}

#include "PanelWindow.h"

Result Project::saveAs (const File & file)
{
    const File oldConfigFile (projectFile);
    
    String configPath;
    const bool parentDirectoryChanged = file.getParentDirectory() != projectDirectory;
        
    if (temporary || parentDirectoryChanged)
    {
        const File oldDir = projectDirectory;
        const String projectName = file.getFileName();
        const File newDir = file.withFileExtension("");
        
        configPath = newDir.getChildFile(projectName).withFileExtension (projectFileExtension).getFullPathName();
        
        if (temporary)
            projectFile.deleteFile();
        
        moveProjectDirectory (newDir, temporary); // Delete the TEMP dir
        temporary = false;
    }
    else
        configPath = file.withFileExtension (projectFileExtension).getFullPathName();
        
    if (! saveProjectConfig (configPath))
        return Result::fail ("Couldn't save project !");
    
    temporary = false;
    
    if (auto um = getMainUndoManager())
        um->clearUndoHistory();
    
    panelManager.refreshWindowsTitle();
    
    return Result::ok();
}

Result Project::saveAs()
{
    const File f = temporary ? AppSettings::getProjectsDirectory() : projectDirectory;
    const String fp = "";
    
    FileChooser fc ("Choose a destination...",
                    f,
                    fp,            // File pattern
                    true);                  // Native
    
    if (fc.browseForFileToSave (true)) // Warns the user when the file already exists
    {
        File chosenFile = fc.getResult();
        
        return saveAs (chosenFile);
    }
    
    return Result::fail ("Save as : command canceled");
}

Result Project::tryToClose()
{
    // If there's nothing to undo, just quit
    if (auto um = getMainUndoManager())
    {
        if (! um->canUndo())
        {
            if (temporary)
                deleteProjectDirectory();
            
            closeWindowsAndQuit();
            return Result::ok();
        }
    }
    
    // Or try to save project
    Result r = saveYesNoCancel();
    
    if (r)   // If user chose not to save or if the project has been saved
    {
        if (temporary)
            deleteProjectDirectory();
        
        closeWindowsAndQuit();
        return Result::ok();
    }
    
    return Result::fail ("Command canceled.");
}

Result Project::saveYesNoCancel()
{
    const int r = NativeMessageBox::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                        TRANS("Unsaved changes !"),
                                                        TRANS("Do you want to save the project?"));
    switch(r) {
        case 2:     // NO
            return Result::ok();
            break;
        case 1:     // YES
            return save();

        case 0:     // CANCEL
            return Result::fail("Command canceled");
        default:
            return Result::fail("Unknown behavior when trying to save project.");
    }
}

Result Project::save()
{
    if (temporary)
        return saveAs();
    
    if (! saveProjectConfig (projectFile))    // Save on current file
        return Result::fail("Couldn't save the project at the specified destination.");
    
    if (auto um = getMainUndoManager())
        um->clearUndoHistory();
    
    return Result::ok();
}


const File& Project::getProjectDirectory() const
{
    return projectDirectory;
}

void Project::createDirectories (const File & directory)
{
    const File dir = directory.withFileExtension("");
    dir.createDirectory();
    
    dir.getChildFile("output").createDirectory();
}

void Project::moveProjectDirectory (File destination, bool deleteSourceDirectory, bool removeHeaders)
{
    auto previousDirectory = projectDirectory;
    
    if (previousDirectory.copyDirectoryTo (destination))
    {
        projectDirectory = destination;
        
        if (deleteSourceDirectory)
            previousDirectory.deleteRecursively();
        
        if (removeHeaders)
        {
            Array<File> headers;
            
            projectDirectory.findChildFiles(headers, File::findFiles, false);
            
            for (auto f : headers)
                f.deleteFile();
        }
    }
}

void Project::deleteProjectDirectory()
{
    if (projectDirectory == File())
        return;
    
    projectDirectory.deleteRecursively();
}

bool Project::saveProjectConfig (File destination)
{
    // Create a property set
    PropertySet ps;
    
    ps.setValue("name", name);
    ps.setValue("created", creationDate);
    ps.setValue("directory", projectDirectory.getFullPathName());
    
    // Create the XML element from the property set
    ScopedPointer<XmlElement> element = new XmlElement (projectFileTagName);
    element->addChildElement (ps.createXml ("ProjectConfig"));
    
    // Save state for all panels
    XmlElement* panelsXml = new XmlElement ("Panels");
    
    for (int i = 0; i < staticPanels.size(); ++i)
        panelsXml->addChildElement (staticPanels.getUnchecked (i)->getAsXml());
    
    element->addChildElement (panelsXml);
    
    // Save state for all windows
    XmlElement* windowsXml = new XmlElement ("PanelWindows");
    
    for (auto w : panelManager.getWindows())
        windowsXml->addChildElement (w->getStateAsXml());
    
    element->addChildElement (windowsXml);
    
    projectFile = destination;
    
    File dir = projectFile.getParentDirectory();
    
    if (!dir.exists())
        createDirectories(dir);
    
    auto result = element->writeToFile (destination, String());
    
    if (result && !isTemporary())
        AppSettings::addToRecentlyOpenedProjects (destination);
    
    return result;
}

bool Project::loadProjectConfig (const File & file)
{
    XmlDocument xmlDoc (file);
    ScopedPointer<XmlElement> element = xmlDoc.getDocumentElement();
    
    if (element == nullptr)
        return false;
    
    if (element->getTagName() != projectFileTagName)
        return false;
    
    // Restore the project's property set
    PropertySet ps;
    
    if (auto projectConf = element->getChildByName ("ProjectConfig"))
        ps.restoreFromXml (*projectConf);
    else
        return false;
    
    // Restore project's internal state
    name = ps.getValue ("name");    // Not sure this is even used anymore...
    creationDate = ps.getValue ("created");
    projectDirectory = ps.getValue ("directory");
    
    projectFile = file;
    
    // Restore internal state of each panels
    if (auto panelsXml = element->getChildByName ("Panels"))
        forEachXmlChildElement (*panelsXml, item)
            loadPanelState (item);
    
    // Restore the panel windows
    if (auto windowsXml = element->getChildByName ("PanelWindows"))
        forEachXmlChildElement (*windowsXml, item)
            restorePanelWindow (item);
    
    // We restore the layout for each panels after the panels window have been restored
    // to make sure that the panels have their definitive size.
    if (auto panelsXml = element->getChildByName ("Panels"))
        forEachXmlChildElement (*panelsXml, item)
            loadPanelLayout (item);
    
    return true;
}

void Project::restorePanelWindow (XmlElement* state)
{
    if (state == nullptr)
        return;
    
    auto window = new PanelWindow (panelManager, *this);
    
    // Give some default size to the window
    auto userArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    window->centreWithSize (userArea.getWidth() * 0.9, userArea.getHeight() * 0.9);
    panelManager.addPanelWindow (window);
    
    window->restoreFromXml (state);
}

Panel* Project::loadPanelState (XmlElement* state)
{
    if (state == nullptr)
        return nullptr;
    
    auto p = findStaticPanel (state);
    
    if (p != nullptr)
        p->restoreState (state);
    
    return p;
}

Panel* Project::loadPanelLayout (XmlElement* state)
{
    if (state == nullptr)
        return nullptr;
    
    auto p = findStaticPanel (state);
    
    if (p != nullptr)
        p->restoreLayout (state);
    
    return p;
}

#include "WebAudioInspector.h"
#include "WebAudioGraph.h"
#include "GenerationPanel.h"
#include "AudioFilesPanel.h"
#include "LibraryPanel.h"
#include "OutputScriptPanel.h"

Panel* Project::findStaticPanel (XmlElement* state)
{
    const String tag = state->getTagName();
    
    if (tag == "NavigationPanel")               return getNavigationPanel();
    else if (tag == "WebAudioGraph")            return findStaticPanelWithClass<WebAudioGraphPanel>();
    else if (tag == "InspectorPanel")           return findStaticPanelWithClass<WebAudioInspector>();
    else if (tag == "GeneratorPanel")           return findStaticPanelWithClass<CodeGenerationPanel>();
    else if (tag == "OutputScriptPanel")        return findStaticPanelWithClass<OutputScriptPanel>();
    else if (tag == "AudioFilesPanel")          return findStaticPanelWithClass<AudioFilesPanel>();
    else if (tag == "Library")                  return findStaticPanelWithClass<LibraryPanel>();
    else                                        return nullptr;
}

String Project::getName() const
{
    return projectFile.getFileNameWithoutExtension();
}

#include "JavascriptEditor.h"
#include "WebAudioNavigation.h"

void Project::createStaticPanels()
{
    // Inspector should be created before the audio graph
    auto webAudioInspector = new WebAudioInspector (&panelManager, *this);
    staticPanels.add (webAudioInspector);
    
    // FilesPanel should be created before the audio graph! (save/restore order)
    auto filesPanel = new AudioFilesPanel (*this);
    staticPanels.add (filesPanel);
    
    auto rootWebAudioGraph = new RootWebAudioGraphPanel (*this);
    staticPanels.add (rootWebAudioGraph);
    
    auto generationPanel = new CodeGenerationPanel (*this);
    staticPanels.add (generationPanel);
    
    auto generatedPanel = new OutputScriptPanel (&panelManager);
    staticPanels.add (generatedPanel);
    
    auto nav = new WebAudioNavigation (*this);
    staticPanels.add (nav);
    nav->setRootNavigable (this);
    
    auto lib = new LibraryPanel (*this);
    staticPanels.add (lib);
}

bool Project::windowContainsAllStaticPanels (PanelWindow* window)
{
    for (auto panel : staticPanels)
    {
        auto w = panel->getParentPanelWindow();
        
        if (w != nullptr && w != window)
            return false;
    }
    
    return true;
}

void Project::restoreDefaultPanelLayout()
{
    panelManager.closeAllWindows();
    openDefaultWindow();
}

PanelWindow* Project::openDefaultWindow()
{
    auto window = new PanelWindow (panelManager, *this);
    auto userArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    window->centreWithSize (userArea.getWidth() * 0.9, userArea.getHeight() * 0.9);
    panelManager.addPanelWindow (window);
    
    auto& panelRoot = window->panelTree.getRootItem();
    
    auto l = panelRoot.addChildNode (-0.2, 100);
    l->addChildNode (-0.6, 100)->setComponent (getNavigationPanel());
    l->addChildNode (-0.4, 100)->setComponent (findStaticPanelWithClass<AudioFilesPanel>());
    
    auto c = panelRoot.addChildNode (-0.6, 100);
    auto ctop = c->addChildNode (-0.40, 100);
    ctop->addChildNode (-0.2, 100)->setComponent (findStaticPanelWithClass<LibraryPanel>());
    ctop->addChildNode (-0.8, 100)->setComponent (findStaticPanelWithClass<WebAudioGraphPanel>());
    
    c->addChildNode (30, 30,30)->setComponent (findStaticPanelWithClass<CodeGenerationPanel>());
    c->addChildNode (-0.4, 100)->setComponent (findStaticPanelWithClass<CodeEditorPanel>());
    
    panelRoot.addChildNode (-0.2, 100)->setComponent (findStaticPanelWithClass<WebAudioInspector>());
    
    return window;
}

Array<WeakReference<NavigationPanel::Navigable>> Project::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    
    if (auto webAudioGraph = findStaticPanelWithClass<WebAudioGraphPanel>())
        result.add (webAudioGraph);
    
    return result;
}

StringPairArray Project::getNavigationInfo() const
{
    StringPairArray info;
    
    info.set ("name", "Project");
    info.set ("type", "HEADER");
    info.set ("category", "PROJECT");
    info.set ("selected", String(int(false)));
    
    return info;
}

#include "WebAudioScript.h"
Array<WeakReference<Panel>> Project::getDynamicPanels() const
{
    Array<WeakReference<Panel>> result;
    
    if (auto graph = findStaticPanelWithClass<RootWebAudioGraphPanel>())
        for (auto f : graph->getAllScripts())
            if (auto editor = f->getCodeEditorPanel())
                result.add (editor);
        
    return result;
}

UndoManager* Project::getMainUndoManager()
{
    if (auto graphPanel = findStaticPanelWithClass<WebAudioGraphPanel>())
        return &graphPanel->getUndoManager();
    
    return nullptr;
}

//==============================================================================

ProjectManager::ProjectManager() : commandTarget (this, nullptr)
{
    auto projectsDirectory = AppSettings::getProjectsDirectory();
    
    if (! projectsDirectory.exists())
        projectsDirectory.createDirectory();
    
    menuModel = new MainMenuBarModel (*this);
    openStartWindow();
}

#include "PreferencesWindow.h"
ProjectManager::~ProjectManager()
{
    masterReference.clear();
    
    PreferencesWindow::close();
    
    closeStartWindow();
    projects.clear();
    audioDeviceManager = nullptr;
    
    menuModel = nullptr;
    applicationCommandManager = nullptr;
}

void ProjectManager::createNewProject()
{
    projects.add (Project::createNewProject ("Untitled project", File(AppSettings::getTempDirectory()), *this));
}

void ProjectManager::quitProject (Project* p)
{
    projects.removeObject (p);
    
    if (projects.isEmpty())
        openStartWindow();
}

bool ProjectManager::loadProject (File projectHeader)
{
    if (! allowMultipleProjects && ! projects.isEmpty())
    {
        if (! tryToCloseProject (projects[0]))
            return false;
        
        projects.clear();
    }
    
    auto project = Project::loadProject (projectHeader.getFullPathName(), *this);
    
    if (project == nullptr)
    {
        AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Project loading error", "This project can't be opened!");
        return false;
    }
    
    AppSettings::addToRecentlyOpenedProjects (projectHeader);
    project->temporary = false;
    
    projects.add (project);
    closeStartWindow();
    
    return true;
}

bool ProjectManager::loadProject()
{
    if (! allowMultipleProjects && ! projects.isEmpty())
    {
        if (! tryToCloseProject (projects[0]))
            return false;
        
        projects.clear();
    }
    
    FileChooser fc ("Choose a file to open...",
                    AppSettings::getProjectsDirectory(),
                    "*" + Project::projectFileExtension,
                    true);
        
    if (fc.browseForFileToOpen())
    {
        const File & f = fc.getResult();
        auto project = Project::loadProject (f.getFullPathName(), *this);
        
        if (project == nullptr)
        {
            AlertWindow::showMessageBox (AlertWindow::WarningIcon, "Project loading error", "This project can't be opened!");
            return false;
        }
        
        AppSettings::addToRecentlyOpenedProjects (f);
        project->temporary = false;
        
        projects.add (project);
        closeStartWindow();
        
        return true;
    }
    
    return false;
}

#include "StartWindow.h"
void ProjectManager::openStartWindow()
{
    StartWindow::open (this);
}

void ProjectManager::closeStartWindow()
{
    StartWindow::close();
}

bool ProjectManager::tryToCloseProject (Project* project)
{
    if (project == nullptr)
        return true;    // No need to close project !
    
    return project->tryToClose();
}

bool ProjectManager::tryToQuitApplication()
{
    for (auto p : projects)
        if (! p->tryToClose())
            return false;
    
    if (JUCEApplicationBase* app = JUCEApplicationBase::getInstance())
        app->quit();
    
    return true;
}
