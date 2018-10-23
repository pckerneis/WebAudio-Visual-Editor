/*
  ==============================================================================

    Windows.cpp
    Created: 24 Jul 2017 7:44:31pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "StartWindow.h"

//==============================================================================
#include "Project.h"

#include "AppSettings.h"
StartWindowContent::StartWindowContent(ProjectManager* projManager) : ProjectCommandTarget (projManager, nullptr), recentListBox ("Recent files", this)
{    
    newProjectButton.setButtonText("Start a new project");
    addAndMakeVisible (&newProjectButton);
    newProjectButton.addListener(this);
    
    openProjectButton.setButtonText("Open a project");
    addAndMakeVisible (&openProjectButton);
    openProjectButton.addListener(this);
    
    refresh();
    recentListBox.setRowHeight (20);
    addAndMakeVisible (recentListBox);
    
    recentListLabel.setText ("Recent projects :", dontSendNotification);
    recentListLabel.setEditable (false);
    addAndMakeVisible (recentListLabel);
    
    projectManager = projManager;
    
    prepareCommandTarget();
    
    //Grab focus after everything is set...
    triggerAsyncUpdate();
}

void StartWindowContent::handleAsyncUpdate()
{
    // Allows key commands to be used when the window hasn't been clicked yet.
    grabKeyboardFocus();
}

void StartWindowContent::paint (Graphics& g)
{
    const auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId);
    g.fillAll (bg);   // clear the background
    
    g.setOpacity (0.23f);
    const auto bgImage = ImageCache::getFromMemory (BinaryData::aboutbg_png, BinaryData::aboutbg_pngSize);
    g.drawImageAt (bgImage, 0, 0, false);
    
    auto r = getLocalBounds().reduced (12);
    const Font montserrat (Typeface::createSystemTypefaceFor (BinaryData::MontserratLight_ttf,
                                                              BinaryData::MontserratLight_ttfSize));
    g.setFont (montserrat);
    g.setFont (26.0f);
    g.setColour (bg.contrasting());
    
    g.drawFittedText ("WebAudio Visual Editor", r.removeFromTop (r.proportionOfHeight(0.4f)), Justification::centred, 1, 1.0);
}

void StartWindowContent::StartWindowContent::resized()
{
	juce::Rectangle<int> r (getLocalBounds().reduced (10));
    auto top = r.removeFromTop (r.proportionOfHeight (0.4f));
    
    newProjectButton.setBounds (r.removeFromTop (28).reduced (2));
    openProjectButton.setBounds (r.removeFromTop (28).reduced (2));
    recentListLabel.setBounds (r.removeFromTop (24).withTrimmedTop (6).reduced (8, 0));
    recentListBox.setBounds (r.reduced (6));
}

void StartWindowContent::StartWindowContent::buttonClicked (Button* b)
{
    if (b == &newProjectButton)
        newProjectButtonClicked();
    if (b == &openProjectButton)
        openProjectButtonClicked();
}

void StartWindowContent::newProjectButtonClicked()
{
    projectManager->createNewProject();
    projectManager->closeStartWindow();
}

void StartWindowContent::openProjectButtonClicked()
{
    if (projectManager->loadProject())
        projectManager->closeStartWindow();
}

void StartWindowContent::closeButtonPressed()
{
    projectManager->closeStartWindow();
}

int StartWindowContent::getNumRows()
{
    return recentList.getNumFiles();
}

void StartWindowContent::paintListBoxItem (int rowNumber, Graphics &g, int width, int height, bool rowIsSelected)
{
    auto bg = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting().withAlpha (0.1f);
    
    if (rowIsSelected)
        g.fillAll (bg);
    
    const bool fileExists = recentList.getFile (rowNumber).exists();
    
    auto col = getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting();
    
    Font f (14.0f);
    
    if (! fileExists)
    {
        col = col.withBrightness (0.5f);
        f = f.italicised();
    }
        
    g.setFont (f);
    g.setColour (col);
    
    String txt (recentList.getFile (rowNumber).getFileNameWithoutExtension());
    
    g.drawText (txt,
                8, 2, width - 8, height - 2,
                Justification::topLeft, true);
}

void StartWindowContent::listBoxItemDoubleClicked (int row, const MouseEvent &)
{
    const auto list = AppSettings::getRecentlyOpenedProjectsList();
    
    if (list.getFile (row).exists())
        projectManager->loadProject (list.getFile (row));
}

void StartWindowContent::refresh()
{
    recentList = AppSettings::getRecentlyOpenedProjectsList();
    recentListBox.updateContent();
}

//==============================================================================

StartWindow::StartWindow (String name, ProjectManager* projectManager)
: DocumentWindow (name,
                  WaveLookAndFeel::getWindowBackgroundColour(),
                  DocumentWindow::TitleBarButtons::closeButton | DocumentWindow::TitleBarButtons::minimiseButton)
{
    setUsingNativeTitleBar (true);
    setTitleBarHeight (18);
    setResizable(false, false);
    
    content = new StartWindowContent (projectManager);
    content->setSize (windowWidth, windowHeight);
    setContentOwned (content, true);
    
    centreWithSize (windowWidth, windowHeight);
    setVisible (true);
}

void StartWindow::closeButtonPressed()
{
    content->closeButtonPressed();
    JUCEApplication::getInstance()->systemRequestedQuit();
}

ScopedPointer<StartWindow> startWindow;

void StartWindow::open (ProjectManager* pm)
{
    startWindow = new StartWindow ("Welcome to WebAudio Visual Editor", pm);
}

void StartWindow::close()
{
    startWindow = nullptr;
}

void StartWindow::refresh()
{
    if (startWindow != nullptr)
        startWindow->refreshContent();
}

void StartWindow::refreshContent()
{
    content->refresh();
}
