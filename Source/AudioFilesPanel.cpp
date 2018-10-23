/*
  ==============================================================================

    AudioFilesPanel.cpp
    Created: 9 Sep 2018 10:11:14pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "AudioFilesPanel.h"

void PreviewWidget::setPreviewedFile (LoadedAudioFile* file)
{
    if (currentFile != file)
    {
        if (currentFile != nullptr)
            currentFile->removeListener (this);
        
        currentFile = file;
        
        if (currentFile != nullptr)
            file->addListener (this);
        
        repaint();
    }
}

void PreviewWidget::paint (Graphics& g)
{
    if (currentFile != nullptr)
    {
        if (currentFile->isNotAvailable())
            paintMessage ("File not found.", g, getLocalBounds());
        else if (currentFile->isLoaded())
            paintAudioThumbnail (g, getLocalBounds());
        else
            paintMessage ("Loading waveform...", g, getLocalBounds());
    }
    else
        paintMessage ("No audio file selected.", g, getLocalBounds());
}

void PreviewWidget::mouseDown (const MouseEvent& e)
{
    const double relPos = (double)e.x / getWidth();
    
    for (auto l : listeners)
        l->audioThumbnailClicked (relPos);
}

void PreviewWidget::audioFileWasLoaded (LoadedAudioFile* file)
{
    if (file == currentFile)
        repaint();
}

void PreviewWidget::paintAudioThumbnail (Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour (Colours::darkgrey);
    g.fillRect (bounds);
    
    g.setColour (Colours::white);
    
    if (currentFile != nullptr)
    {
        auto& thumbnail = currentFile->getThumbnail();
        thumbnail.drawChannels (g, bounds,
                                0.0, thumbnail.getTotalLength(), // start, end
                                1.0f); // v zoom
    }
}

void PreviewWidget::paintMessage (String message, Graphics& g, juce::Rectangle<int> bounds)
{
    g.setColour (Colours::darkgrey);
    g.fillRect (bounds);
    g.setColour (Colours::white);
    g.drawFittedText (message, bounds, Justification::centred, 1.0f);
}

//==============================================================================

#include "Project.h"

AudioFilesPanel::AudioFilesPanel (Project& p) : Panel (&p.getPanelManager()), project (p), addButton ("add"), playButton ("play"), filePlayer (*this), playhead (filePlayer)
{
    Project::getAudioDeviceManager().addAudioCallback (&audioSourcePlayer);
    audioSourcePlayer.setSource (&filePlayer);
    
    addButton.addListener (this);
    addButton.setBorderSize (BorderSize<int> (5));
    addAndMakeVisible (addButton);
    
    playButton.addListener (this);
    playButton.setBorderSize (BorderSize<int> (5));
    addAndMakeVisible (playButton);
    
    addAndMakeVisible (preview);
    preview.addListener (this);
    
    addAndMakeVisible (playhead);
    
    addAndMakeVisible (table);
    table.setModel (this);
    //table.setColour (ListBox::outlineColourId, Colours::grey);
    //table.setOutlineThickness (1);
    
    table.getHeader().addColumn ("", 1, 16, 16, 16, TableHeaderComponent::ColumnPropertyFlags::notResizable);
    table.getHeader().addColumn ("Output name", 2, 50);
    table.getHeader().addColumn ("Location", 3, 50);
    table.getHeader().setStretchToFitActive (true);
    table.setHeaderHeight (22);
    
    addAndMakeVisible (fileInfo);
    
    setWantsKeyboardFocus (true);
    
    setPanelName ("Audio files");
    sendPanelBorderToFront();
    
    prepareCommandTarget();
    
    startTimer (timerInterval);
}

AudioFilesPanel::~AudioFilesPanel()
{
    audioSourcePlayer.setSource (nullptr);
    Project::getAudioDeviceManager().removeAudioCallback (&audioSourcePlayer);
}

void AudioFilesPanel::resized()
{
    Panel::resized();
    
    const int topMargin = getHeaderHeight();
    const int previewHeight = 32;
    const int infosHeight = 24;
    const int buttonWidth = 22;
    
	juce::Rectangle<int> r (getLocalBounds().withTrimmedTop (topMargin));
    
    auto previewArea = r.removeFromBottom (previewHeight).reduced (6);
    auto infoArea = r.removeFromBottom (infosHeight);
    
    addButton.setBounds (previewArea.removeFromRight (buttonWidth));
    playButton.setBounds (previewArea.removeFromRight (buttonWidth));
    preview.setBounds (previewArea);
    playhead.setBounds (previewArea);
    fileInfo.setBounds (infoArea);
    
    table.setBounds (r);
}

void AudioFilesPanel::buttonClicked (Button* b)
{
    if (b == &addButton)
        addButtonClicked();
    else if (b == &playButton)
        playButtonClicked();
}

void AudioFilesPanel::addButtonClicked()
{
    FileChooser chooser ("Select an audio file to add...", File(), "*.wav, *.aif, *.mp3, *.ogg");
    
    if (chooser.browseForFileToOpen())
        addFileUndoable (chooser.getResult());
}

#include "CommandIDs.h"
void AudioFilesPanel::showPopupMenu (int row)
{
    auto item = subItems[row];
    
    if (item == nullptr)
        return;
    
    PopupMenu m;
    auto& acm = Project::getApplicationCommandManager();
    
    m.addCommandItem (&acm, CommandIDs::rename);
    
    const String relocateString (item->hasValidSource() ? "Change file..." : "Relocate file...");
    
    m.addItem (2, relocateString);
    m.addSeparator();
    m.addCommandItem (&acm, CommandIDs::del);
    m.addSeparator();
    
    PopupMenu addAsMenu;
    
    addAsMenu.addItem (3, "new audio element");
    addAsMenu.addItem (4, "new decodable audio");
    
    m.addSubMenu("Add to graph as...", addAsMenu);
    
    const int r = m.show();
    
    if (r == 2)         modifyFileWindow (row);
    else if (r == 3)    addToGraph (row, "AudioElement");
    else if (r == 4)    addToGraph (row, "DecodableAudio");
}

class UndoableAddFileAction : public UndoableAction
{
public:
    UndoableAddFileAction (AudioFilesPanel& p, File f) : panel (p), file (f)
    {}
    
    bool perform() override
    {
        if (persistantRef != nullptr)
        {
            panel.subItems.add (persistantRef);
            panel.table.updateContent();
            
            return true;
        }
        
        persistantRef = panel.addFile (file);
        
        return true;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        panel.removeFile (persistantRef);
        
        return true;
    }
    
    AudioFileItem* getItem() { return persistantRef; }
    
private:
    AudioFileItem::Ptr persistantRef;
    AudioFilesPanel &panel;
    File file;
};

#include "WebAudioGraph.h"
AudioFileItem* AudioFilesPanel::addFileUndoable (File f)
{
    if (auto graph = project.findStaticPanelWithClass<RootWebAudioGraphPanel>())
    {
        auto undoable = new UndoableAddFileAction (*this, f);
        
        auto &um = graph->getUndoManager();
        um.beginNewTransaction();
        um.perform (undoable);
        
        return undoable->getItem();
    }
    
    return nullptr;
}

class UndoableRemoveFileAction : public UndoableAction
{
public:
    UndoableRemoveFileAction (AudioFilesPanel& p, int rowNumber) : panel (p), row (rowNumber)
    {}
    
    bool perform() override
    {
        persistantRef = panel.subItems[row];
        
        users.clear();
        
        for (auto u : persistantRef->getUsers())
            users.add (u);
        
        panel.removeFile (row);
        
        return true;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        panel.subItems.insert (row, persistantRef);
        panel.table.updateContent();
        
        for (auto u : users)
            persistantRef->addUser (u);
        
        return true;
    }
    
    AudioFileItem* getItem() { return persistantRef; }
    
private:
    AudioFileItem::Ptr persistantRef;
    AudioFilesPanel &panel;
    const int row;
    Array<WeakReference<WebAudioData>> users;
};

void AudioFilesPanel::removeFileUndoable (int row)
{
    if (auto graph = project.findStaticPanelWithClass<RootWebAudioGraphPanel>())
    {
        auto undoable = new UndoableRemoveFileAction (*this, row);
        
        auto &um = graph->getUndoManager();
        um.beginNewTransaction();
        um.perform (undoable);
    }
}

#include "WebAudioDictionary.h"
#include "WebAudioData.h"
void AudioFilesPanel::addToGraph (int row, String interfaceName)
{
    auto graph = project.findStaticPanelWithClass<RootWebAudioGraphPanel>();
    auto item = subItems[row];
    
    if (graph == nullptr || item == nullptr)
        return;
    
    SharedResourcePointer<WebAudioDictionary> dict;
    const auto d = dict->findDescriptorForInterface (interfaceName);
    
    const auto pos = graph->getLocalBounds().getCentre();
    auto comp = graph->createAndAddUndoable (d, pos);
    
    if (auto d = dynamic_cast<WebAudioData*> (comp))
    {
        item->addUser (d);
        d->checkAndSetName (item->getOutputNameWithoutExtension() + d->getInterfaceName());
        graph->getSelector().setUniqueSelection (d);
        d->showNameEditor();
    }
}

void AudioFilesPanel::modifyFileWindow (int row)
{
    FileChooser chooser ("Select an audio file", File(), "*.wav, *.aif, *.mp3");
    
    if (row < subItems.size() && chooser.browseForFileToOpen())
    {
        auto chosen = chooser.getResult();
        
        if (!filesManager->isAlreadyLoaded(chosen.getFullPathName()))
        {
            const String alias = subItems.getUnchecked (row)->getOutputName();
            removeFile (row);
            
            if (auto newItem = addFile (chooser.getResult()))
            {
                newItem->setOutputName(alias);
                subItems.move (subItems.size() - 1, row);
                
                table.updateContent();
            }
        }
    }
}

AudioFileItem* AudioFilesPanel::addFile (File file)
{
    if (!filesManager->isAlreadyLoaded (file.getFullPathName()))
    {
        const int sampleRate = Project::getAudioDeviceManager().getCurrentAudioDevice()->getCurrentSampleRate();
        auto loadedFile = filesManager->loadFile (file.getFullPathName(), sampleRate);
        
        auto item = new AudioFileItem (loadedFile, *this);
        subItems.add (item);
        
        item->setOutputName (file.getFileName());
        
        //setPreviewedFile (loadedFile);
        table.updateContent();
        
        if (! filePlayer.isPlaying())
            table.selectRow (subItems.size() - 1);
        
        return item;
    }
    
    return nullptr;
}

void AudioFilesPanel::removeFile (AudioFileItem* item)
{
    if (subItems.contains (item))
    {
        if (preview.getPreviewedFile() == item->getAudioFile())
        {
            triggerStop();
            goToInitialPosition();
            preview.setPreviewedFile (nullptr);
        }
        
        //filesManager->removeFile (item->getFullPath());
        
        for (auto u : item->getUsers())
            if (u)
                u->setSourceFromAudioFile (nullptr);
        
        subItems.removeObject (item);
        table.updateContent();
    }
}

void AudioFilesPanel::removeFile (int num)
{
    jassert (num < subItems.size());
    
    if (auto item = subItems[num])
        removeFile (item);
}


AudioFileItem* AudioFilesPanel::findFileWithUuid (Uuid uuid) const
{
    for (auto f : getSubItems())
        if (f->getUuid() == uuid)
            return f;
    
    return nullptr;
}

void AudioFilesPanel::playButtonClicked()
{
    if (playButton.getToggleState())
        triggerPlay();
    else
        triggerStop();
}

void AudioFilesPanel::goToInitialPosition()
{
    filePlayer.setPlaybackPositionRelative (0);
}

void AudioFilesPanel::triggerPlay()
{
    filePlayer.play();
    playhead.setActive (true);
    
    if (!playButton.getToggleState())
        playButton.setToggleState (true, NotificationType::dontSendNotification);
}

void AudioFilesPanel::triggerStop()
{
    filePlayer.stop();
    playhead.setActive (false);
    
    if (playButton.getToggleState())
        playButton.setToggleState (false, NotificationType::dontSendNotification);
}

void AudioFilesPanel::setPreviewedFile (LoadedAudioFile* f)
{
    triggerStop();
    
    preview.setPreviewedFile (f);
    filePlayer.setAudioFile (f);
    
    if (f != nullptr)
    {
        if (f->isNotAvailable())
            fileInfo.setCurrentText ("Size : unknown");
        else
        {
            const int64 size = File(f->getFullPath()).getSize();
            fileInfo.setCurrentText ("Size : " + File::descriptionOfSizeInBytes (size));
        }
    }
    else
        fileInfo.setCurrentText ("");
}

void AudioFilesPanel::audioThumbnailClicked (double relativePosition)
{
    filePlayer.setPlaybackPositionRelative (relativePosition);
    playhead.repaint();
}

void AudioFilesPanel::aliasWasEdited (int row, String text)
{
    if (auto item = subItems[row])
        if (item->getOutputName() != text)
        {
            item->setOutputName (text);
            table.updateContent();
        }
}

void AudioFilesPanel::paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected)
{
    const Colour alternateColour (getLookAndFeel().findColour (ListBox::backgroundColourId)
                                  .interpolatedWith (getLookAndFeel().findColour (ListBox::textColourId), 0.03f));
    if (rowIsSelected)
        g.fillAll (Colours::grey.withMultipliedAlpha(0.5f));
    else if (rowNumber % 2)
        g.fillAll (alternateColour);
}

void AudioFilesPanel::paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected)
{
    g.setColour (getLookAndFeel().findColour (ListBox::backgroundColourId));
    g.fillRect (width - 1, 0, 1, height);
    
    auto item = subItems[rowNumber];
    
    if (item == nullptr)
        return;
    
    const bool hasValidSource = item->hasValidSource();
    
    String text;
    
    if (columnId == 3)
    {
        text = item->getFullPath();
        
        Font f (12.0f);
        Colour c (getLookAndFeel().findColour (ListBox::textColourId));
        
        if (! hasValidSource)
            f = f.italicised();
        
        g.setFont (f);
        g.setColour (hasValidSource ? c : c.withMultipliedAlpha (0.5f));
    }
    else if (columnId == 1 && ! hasValidSource)
    {
        text = " !";
        
        g.setFont (Font (12.0f).boldened());
        g.setColour (getLookAndFeel().findColour (ListBox::textColourId));
    }
    
    if (text.isEmpty())
        return;
   
    g.drawText (text, 2, 0, width - 2, height, Justification::centredLeft, true);
}

void AudioFilesPanel::backgroundClicked (const MouseEvent &)
{
    table.deselectAllRows();
}

Component* AudioFilesPanel::refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/,
                                                     Component* existingComponentToUpdate)
{
    if (columnId == 3 || columnId == 1)
    {
        jassert (existingComponentToUpdate == nullptr);
        return nullptr;
    }
    
    EditableAliasComponent* textLabel = static_cast<EditableAliasComponent*> (existingComponentToUpdate);
    
    if (textLabel == nullptr)
        textLabel = new EditableAliasComponent (*this);
    
    textLabel->setRowAndColumn (rowNumber, columnId);
    textLabel->setText (subItems[rowNumber]->getOutputName(), NotificationType::dontSendNotification);
    textLabel->setFont (Font (12.0f));
    textLabel->setMinimumHorizontalScale(0.9f);
    textLabel->addListener (this);
    return textLabel;
}

void AudioFilesPanel::selectedRowsChanged (int row)
{
    if (auto item = subItems[row])
    {
        if (auto f = item->getAudioFile())
            setPreviewedFile (f);
    }
    else
        setPreviewedFile (nullptr);
    
    rowSelected = row;
}

void AudioFilesPanel::sortOrderChanged (int newSortColumnId, bool isForwards)
{
    if (newSortColumnId != 0)
    {
        ItemSorter sorter (isForwards, newSortColumnId);
        subItems.sort (sorter);
        
        table.updateContent();
    }
}

void AudioFilesPanel::timerCallback()
{
    checkFilesExistence();
}

void AudioFilesPanel::checkFilesExistence()
{
    for (auto f : subItems)
        f->checkFileExistence();
    
    table.updateContent();
    table.repaint();
}

int AudioFilesPanel::getRowNumberForFile (LoadedAudioFile* file) const
{
    for (int i = 0; i < subItems.size(); ++i)
        if (subItems.getUnchecked(i)->getAudioFile() == file)
            return i;
    
    return -1;
}

XmlElement* AudioFilesPanel::getAsXml()
{
    XmlElement* e = new XmlElement ("AudioFilesPanel");
    e->setAttribute ("panelId", getPanelId());
    
    for (auto item : subItems)
        e->addChildElement (getXmlFor (item));
    
    return e;
}

XmlElement* AudioFilesPanel::getXmlFor (AudioFileItem* item)
{
    XmlElement* e = new XmlElement ("AudioFile");
    
    if (item != nullptr)
    {
        e->setAttribute ("alias", item->getOutputName());
        e->setAttribute ("fullPath", item->getFullPath());
        e->setAttribute ("uuid", item->getUuid().toString());
    }
    
    return e;
}

void AudioFilesPanel::restoreState (XmlElement* e)
{
    if (e == nullptr)
        return;
    
    setPanelId (e->getIntAttribute ("panelId"));
    
    forEachXmlChildElement (*e, item)
    {
        const String alias (item->getStringAttribute ("alias"));
        const String fullPath (item->getStringAttribute ("fullPath"));
        const String uuid (item->getStringAttribute ("uuid"));
        
        if (auto newFile = addFile (File(fullPath)))
        {
            newFile->setOutputName (alias);
            newFile->setUuid (uuid);
        }
    }
}

void AudioFilesPanel::showAliasEditor (int row)
{
    if (auto rowComp = table.getComponentForRowNumber(row))
        for (auto c : rowComp->getChildren())
            if (auto aliasComp = dynamic_cast<EditableAliasComponent*>(c))
                aliasComp->showEditor();
}

ApplicationCommandTarget* AudioFilesPanel::getNextCommandTarget()
{
    return project.findStaticPanelWithClass<RootWebAudioGraphPanel>();
}

void AudioFilesPanel::prepareCommandTarget()
{
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    
    commandManager.registerAllCommandsForTarget (this);
}

void AudioFilesPanel::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::del,
        CommandIDs::rename
    };
    
    commands.addArray (ids, numElementsInArray (ids));
}

void AudioFilesPanel::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    const String editCategory ("Edit");
    
    const bool oneItemSelected = (rowSelected >= 0);
    
    switch (commandID)
    {
        case CommandIDs::del :
            result.setInfo ("Delete", "Delete the selection", editCategory, 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::noModifiers);
            result.addDefaultKeypress (KeyPress::deleteKey, ModifierKeys::noModifiers);
            result.setActive (oneItemSelected);
            break;
        case CommandIDs::rename :
            result.setInfo ("Rename", "Renames the selected item", editCategory, 0);
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            result.setActive (oneItemSelected);
            break;
    }
}

bool AudioFilesPanel::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::del :
            removeFileUndoable (rowSelected);
            break;
        case CommandIDs::rename :
            showAliasEditor(rowSelected);
            break;
    }
    
    return true;
}

bool AudioFilesPanel::isInterestedInFileDrag (const StringArray &files)
{
    bool interested = true;
    
    for (auto f : files)
    {
        File file (f);
        
        if (! (file.hasFileExtension ("wav") || file.hasFileExtension ("mp3") || file.hasFileExtension ("ogg")))
            interested = false;
    }
    
    return interested;
}

void AudioFilesPanel::fileDragEnter (const StringArray &files, int x, int y)
{
    if (isInterestedInFileDrag (files))
        gainFocus();
}

void AudioFilesPanel::fileDragExit (const StringArray &files)
{
    if (auto manager = getPanelManager())
        manager->disableFocus();
}

void AudioFilesPanel::filesDropped (const StringArray &files, int x, int y)
{
    for (auto f : files)
        addFile (File (f));
}

//==============================================================================

int AudioFilesPanel::ItemSorter::compareElements (AudioFileItem* first, AudioFileItem* second) const
{
    if (column == 1) // sort by alias
    {
        return direction * first->getOutputName().compareNatural (second->getOutputName());
    }
    else if (column == 2) // sort by path
    {
        return direction * first->getFullPath().compareNatural (second->getFullPath());
    }
    
    return 0;
}

//==============================================================================

AudioFilesPanel::AudioFilePlayer::AudioFilePlayer (AudioFilesPanel& panel) : Thread ("Audio file player"), playbackPosition (0), endPosition (0), owner (panel)
{
    startThread (500);
}

AudioFilesPanel::AudioFilePlayer::~AudioFilePlayer()
{
    stopThread (500);
}

void AudioFilesPanel::AudioFilePlayer::getNextAudioBlock (const AudioSourceChannelInfo &bufferInfo)
{
    bufferInfo.clearActiveBufferRegion();
    
    if (playing && playbackPosition < endPosition && currentBuffer != nullptr)
    {
        RefCountedBuffer::Ptr bufferUsed (currentBuffer);
        
        const AudioSampleBuffer& sourceBuffer (*bufferUsed->getAudioSampleBuffer());
        const int destStart = bufferInfo.startSample;
        const int numSamples = jmin (bufferInfo.numSamples, sourceBuffer.getNumSamples() - playbackPosition);
        
        for (int channel = 0; channel < bufferInfo.buffer->getNumChannels(); ++channel)
            bufferInfo.buffer->copyFrom (channel, destStart,
                                         sourceBuffer, channel % sourceBuffer.getNumChannels(),
                                         playbackPosition, numSamples);
        
        playbackPosition += bufferInfo.numSamples;
    }
    else
    {
        stop();
        //owner.triggerStop();
        //owner.goToInitialPosition();
    }
}

void AudioFilesPanel::AudioFilePlayer::setAudioFile (LoadedAudioFile* file)
{
    stop();
    playbackPosition = 0;
    
    if (file != nullptr && currentFile != file)
        currentFile = file;
}

void AudioFilesPanel::AudioFilePlayer::play()
{
    playing = true;
}

void AudioFilesPanel::AudioFilePlayer::stop()
{
    playing = false;
}

void AudioFilesPanel::AudioFilePlayer::run()
{
    while (!threadShouldExit())
    {
        checkForBuffersToFree();
        
        if (lastLoadedFile != currentFile)
        {
            //currentBuffer = nullptr;
            
            if (currentFile->isLoaded())
            {
                const auto& bufferToCopy = currentFile->getBuffer();
                
                RefCountedBuffer::Ptr newBuffer = new RefCountedBuffer (bufferToCopy.getNumChannels(), bufferToCopy.getNumSamples());
                newBuffer->getAudioSampleBuffer()->makeCopyOf (bufferToCopy);
                
                currentBuffer = newBuffer;
                buffers.add (newBuffer);
                
                endPosition = bufferToCopy.getNumSamples();
                lastLoadedFile = currentFile;
            }
        }
        
        wait (500);
    }
}

void AudioFilesPanel::AudioFilePlayer::checkForBuffersToFree()
{
    for (int i = buffers.size(); --i >= 0;)
    {
        RefCountedBuffer::Ptr buffer (buffers.getUnchecked(i));
        
        if (buffer->getReferenceCount() == 2) // refs are the ref in buffers and the buffer local variable above
            buffers.remove (i); // so the buffer isn't used in the audio thread.
    }
}


//==============================================================================
void AudioFilesPanel::Playhead::timerCallback()
{
    repaint();
}

void AudioFilesPanel::Playhead::paint (Graphics& g)
{
    const float relPos = player.getPlaybackPositionRelative();
    
    Colour colour (Colours::white);
    
    if (auto lf = dynamic_cast<LookAndFeel_V4*>(&LookAndFeel::getDefaultLookAndFeel()))
    {
        typedef LookAndFeel_V4::ColourScheme::UIColour UIColour;
        colour = lf->getCurrentColourScheme().getUIColour(UIColour::highlightedFill);
    }
    
    g.setColour (colour);
    g.drawLine (relPos * getWidth(), 0,
                relPos * getWidth(), getHeight(),
                2.0f);
    
}

void AudioFilesPanel::Playhead::setActive (bool shouldBeActive)
{
    if (shouldBeActive)
        startTimer (50);
    else
        stopTimer();
}

//==============================================================================

AudioFilesPanel::EditableAliasComponent::EditableAliasComponent(AudioFilesPanel& panel) : owner (panel)
{
    setEditable (false, true);
}

void AudioFilesPanel::EditableAliasComponent::mouseDown (const MouseEvent& e)
{
    owner.table.selectRowsBasedOnModifierKeys (row, e.mods, false);
    Label::mouseDown (e);
}

void AudioFilesPanel::EditableAliasComponent::mouseUp (const MouseEvent& e)
{
    if (e.mods.isRightButtonDown() && e.mouseWasClicked())
        owner.showPopupMenu (row);
}

void AudioFilesPanel::EditableAliasComponent::textWasEdited()
{
    owner.aliasWasEdited (row, getText());
}

void AudioFilesPanel::EditableAliasComponent::setRowAndColumn (int rowNumber, int colNumber)
{
    row = rowNumber;
    column = colNumber;
}

//==============================================================================

AudioFileItem::AudioFileItem (LoadedAudioFile* file, AudioFilesPanel& owner) : panel (owner), audioFile (file)
{
    if (file != nullptr)
    {
        const File f (file->getFullPath());
        
        outputName = f.getFileName();
    }
    
    checkFileExistence();
}

#include "WebAudioData.h"
void AudioFileItem::setOutputName (String newName)
{
    const String ext (getFile().getFileExtension());
    
    if (newName.endsWith (ext))
        newName = newName.replaceSection(newName.length() - ext.length(), ext.length(), "");
    
    newName = newName.removeCharacters ("\\/:*?|<>\"");
    newName = newName.trim();
    
    const String wantedName (newName + ext);
    
    for (auto item : panel.getSubItems())
        if (item->getOutputName() == wantedName && item != this)
            return setOutputName (newName + "_");
    
    outputName = newName + ext;
    
    for (auto u : users)
        if (u)
            u->setSourceFromAudioFile (this);
}

void AudioFileItem::addUser (WebAudioData* newUser)
{
    if (newUser == nullptr)
        return;
    
    users.addIfNotAlreadyThere (newUser);
    
    newUser->setSourceFromAudioFile (this);
}

void AudioFileItem::checkFileExistence()
{
    const bool fileExists = getFile().exists();
    
    if (sourceExists != fileExists)
    {
        sourceExists = fileExists;
        
        for (auto u : users)
            u->audioFileValidityChanged (fileExists);
    }
}
