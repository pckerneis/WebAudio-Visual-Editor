
#include "Project.h"
#include "AppSettings.h"
#include "StartWindow.h"
#include "AboutWindow.h"

//==============================================================================
/** \brief Application instance
 *
 *  This class manage an instance of the application.
 */
class WaveApplication  : public JUCEApplication
{
public:    
    //==============================================================================
    /** \brief Constructor */
    WaveApplication() {}
    
    /** \brief Application's name */
    const String getApplicationName() override       { return ProjectInfo::projectName; }
    /** \brief Application's version number */
    const String getApplicationVersion() override    { return ProjectInfo::versionString; }
    /** \brief Specifies if more than one instance of the application is allowed  */
    bool moreThanOneInstanceAllowed() override       { return false; }

    //==============================================================================
    /** \brief Called at application initialisation */
    void initialise (const String& commandLine) override
    {
        AppSettings::initApplicationSettings();

#ifndef JUCE_MAC
        // Project files association for Windows (on OSX, this is done in the .plist file)
		WindowsRegistry::registerFileAssociation(".waveproj", "waveProject", "Project file for WebAudio Visual Editor",
			File::getSpecialLocation (File::currentExecutableFile), 3, true);
#endif
                
        projectManager = new ProjectManager();

        
#ifndef JUCE_MAC
        // On Windows, "Open with" command, when the app isn't running, will trigger
        // this method
		const File projToLoad (commandLine.unquoted());
		
		if (projToLoad.existsAsFile())
			projectManager->loadProject (projToLoad);
#endif
    }
    
    /** \brief Specifies if more than one instance of the application is allowed */
    void shutdown() override
    {
        // Close about screen if it's showing
        AboutScreen::close();
        
        projectManager = nullptr;
    }

    //==============================================================================
    
    /** \brief System tries to close the app
     *
     * This is called when the app is being asked to quit: you can ignore this request and let the app carry on running, or call quit() to allow the app to close.
     */
    void systemRequestedQuit() override
    {
        if (projectManager)
            projectManager->tryToQuitApplication();
        else
            quit();
    }

    void anotherInstanceStarted (const String& commandLine) override
    {
        // When opening a document with this app, the commandLine arg is the file path to
        // the document to open.
        projectManager->loadProject (commandLine.unquoted());
    }

private:
    ScopedPointer<ProjectManager> projectManager;
};

//==============================================================================
// This macro generates the main() routine that launches the app.
START_JUCE_APPLICATION (WaveApplication)
