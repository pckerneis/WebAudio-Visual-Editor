/*
  ==============================================================================

    StartWindow.h
    Created: 24 Jul 2017 7:44:31pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

//==============================================================================
#include "ProjectCommandTarget.h"
#include "WaveLookAndFeel.h"

class ProjectManager;

/** \brief StartWindow content, contains basic startup actions like creating a new project or opening an existing one.  */
class StartWindowContent
: public Component,
    public Button::Listener,
    public ProjectCommandTarget,
    private AsyncUpdater,
    public LookAndFeelUpdater::Listener,
    public ListBoxModel
{
public:
    /** \brief Constructor. Takes a reference to the project manager (the window's owner) for calling methods such as loadProject() or createNewProject(). */
    StartWindowContent (ProjectManager* projManager);
    /** \brief Empty destructor. */
    ~StartWindowContent() {}
    
    /** \brief Grabs keyboard focus after the window's initialisation. */
    void handleAsyncUpdate() override;
    
    /** \brief Draws a solid background. */
    void paint (Graphics& g) override;
    /** \brief Called when resized to adapt button positions. */
    void resized() override;
    
    /** \brief Called when a button was clicked. */
    void buttonClicked (Button* b) override;
    /** \brief Called when the 'New project' button was clicked. */
    void newProjectButtonClicked();
    /** \brief Called when the 'Load project' button was clicked. */
    void openProjectButtonClicked();
    /** \brief Closes this window when the StartWindow's close button was clicked. */
    void closeButtonPressed();
    
    // Recent file list box : ListBoxModel implementation
    int getNumRows() override;
    void paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected) override;
    void listBoxItemDoubleClicked (int row, const MouseEvent &) override;
    
    /** \brief Update recent projects list box content. */
    void refresh();
    
private:
    TextButton newProjectButton;
    TextButton openProjectButton;
    
    WeakReference<ProjectManager> projectManager;
    
    RecentlyOpenedFilesList recentList;
    ListBox recentListBox;
    Label recentListLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartWindowContent)
};

//==============================================================================

/** \brief A window that holds basic startup content */
class StartWindow    : public DocumentWindow
{
public:
    /** \brief Constructor */
    StartWindow (String name, ProjectManager* projectManager);
    
    /** \brief Closes the window and quits the app when the close button was pressed. */
    void closeButtonPressed() override;
    
    static void open (ProjectManager* pm);
    static void close();
    static void refresh();
    
private:
    void refreshContent();
    
    SafePointer<StartWindowContent> content;
    const int windowWidth = 350;
    const int windowHeight = 450;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (StartWindow)
};
