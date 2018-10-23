/*
  ==============================================================================

    AudioFilesPanel.h
    Created: 9 Sep 2018 10:11:14pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "Panel.h"
#include "AudioFilesManager.h"
#include "ShapeButtons.h"

class AudioFileItem;

class PreviewWidget : public Component, public LoadedAudioFile::Listener
{
public:
    PreviewWidget() {}
    ~PreviewWidget() {}
    
    void resized() override {}
    void paint (Graphics& g) override;
    void mouseDown (const MouseEvent& e) override;
    
    void setPreviewedFile (LoadedAudioFile* file);
    LoadedAudioFile* getPreviewedFile() { return currentFile; }
    
    void audioFileWasLoaded (LoadedAudioFile* file) override;
    
    class Listener
    {
    public:
        virtual ~Listener() {}
        
        virtual void audioThumbnailClicked (double relativePosition) = 0;
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
private:
    void paintAudioThumbnail (Graphics& g, juce::Rectangle<int> bounds);
    
    void paintMessage (String message, Graphics& g, juce::Rectangle<int> bounds);
    
    Array<Listener*> listeners;
    
    WeakReference<LoadedAudioFile> currentFile;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreviewWidget)
};

//==============================================================================
class Project;

class AudioFilesPanel :     public Panel,
                            public Button::Listener,
                            public PreviewWidget::Listener,
                            public TableListBoxModel,
                            public Timer,
                            public Label::Listener,
                            public FileDragAndDropTarget
{
public:
    AudioFilesPanel (Project& p);
    ~AudioFilesPanel();
    
    AudioFileItem* addFile (File f);
    void removeFile (AudioFileItem* item);
    void removeFile (int row);
    
    const ReferenceCountedArray<AudioFileItem>& getSubItems() const { return subItems; }
    
    AudioFileItem* findFileWithUuid (Uuid uuid) const;
    
    //==============================================================================
    void setPreviewedFile (LoadedAudioFile* f);
    void goToInitialPosition();
    void triggerPlay();
    void triggerStop();
    
    //==============================================================================
    void resized() override;
    void buttonClicked (Button* b) override;
    
    int getNumRows() override { return subItems.size(); }
    void paintRowBackground (Graphics& g, int rowNumber, int width, int height, bool rowIsSelected) override;
    void paintCell (Graphics& g, int rowNumber, int columnId, int width, int height, bool rowIsSelected) override;
    void backgroundClicked (const MouseEvent &) override;
    Component* refreshComponentForCell (int rowNumber, int columnId, bool /*isRowSelected*/, Component* existingComponentToUpdate) override;
    void selectedRowsChanged (int row) override;
    void sortOrderChanged (int newSortColumnId, bool isForwards) override;
    void cellClicked (int rowNumber, int columnId, const MouseEvent& e)    override
    {
        if (e.mods.isRightButtonDown())
            showPopupMenu (rowNumber);
    }
    void deleteKeyPressed (int lastRowSelected) override { removeFileUndoable (lastRowSelected); }
    
    //==============================================================================
    struct ItemSorter
    {
        ItemSorter (bool forwards, int columnNumber) : direction (forwards ? 1 : -1), column (columnNumber) {}
        
        int compareElements (AudioFileItem* first, AudioFileItem* second) const;
        
    private:
        int direction;
        int column;
    };
    
    //==============================================================================
    XmlElement* getAsXml() override;
    XmlElement* getXmlFor (AudioFileItem* item);
    void restoreState (XmlElement* e) override;
    
    //==============================================================================
    class AudioFilePlayer : public AudioSource, private Thread
    {
    public:
        //==============================================================================
        class RefCountedBuffer : public ReferenceCountedObject
        {
        public:
            typedef ReferenceCountedObjectPtr<RefCountedBuffer> Ptr;
            
            RefCountedBuffer (int numChannels, int numSamples) : buffer (numChannels, numSamples) {}
            ~RefCountedBuffer() {}
            
            AudioSampleBuffer* getAudioSampleBuffer() { return &buffer; }
            
        private:
            AudioSampleBuffer buffer;
            
            JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RefCountedBuffer)
        };
        //==============================================================================
        
        AudioFilePlayer (AudioFilesPanel& panel);
        ~AudioFilePlayer();
        
        void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override {}
        void releaseResources() override {}
        void getNextAudioBlock (const AudioSourceChannelInfo &bufferInfo) override;
        void setAudioFile (LoadedAudioFile* file);
        
        void play();
        void stop();
        
        float getPlaybackPositionRelative() const { return endPosition == 0 ? 0.0f : static_cast<float>(playbackPosition) / static_cast<float>(endPosition); }
        void setPlaybackPositionRelative (double newPos) { playbackPosition = newPos * endPosition; }
        
        bool isPlaying() const { return playing; }
        
    private:
        void run() override;
        void checkForBuffersToFree();
        
        WeakReference<LoadedAudioFile> currentFile;
        WeakReference<LoadedAudioFile> lastLoadedFile;
        
        RefCountedBuffer::Ptr currentBuffer;
        Array<RefCountedBuffer::Ptr> buffers;
        
        int playbackPosition;
        int endPosition;
        bool playing;
        
        AudioFilesPanel& owner;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilePlayer)
    };
    
    //==============================================================================
    
    class Playhead : public Component, public Timer
    {
    public:
        Playhead (AudioFilePlayer& afp) : player (afp) { setInterceptsMouseClicks (false, false); }
        ~Playhead() { stopTimer(); }
        
        void timerCallback() override;
        void paint (Graphics& g) override;
        void setActive (bool shouldBeActive);
        
    private:
        AudioFilePlayer& player;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Playhead)
    };
    
    //==============================================================================
    
    class EditableAliasComponent : public Label
    {
    public:
        EditableAliasComponent(AudioFilesPanel& panel);
        ~EditableAliasComponent() {}
        
        void mouseDown (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        
        void textWasEdited() override;
        
        void setRowAndColumn (int rowNumber, int colNumber);
        int getRow() const { return row; }
        
    private:
        AudioFilesPanel& owner;
        
        int row, column;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EditableAliasComponent)
    };
    
    //==============================================================================
    
    ApplicationCommandTarget* getNextCommandTarget() override;
    void prepareCommandTarget();
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    
    //==============================================================================
    
    bool isInterestedInFileDrag (const StringArray &files) override;
    void fileDragEnter (const StringArray &files, int x, int y) override;
    void fileDragExit (const StringArray &files) override;
    void filesDropped (const StringArray &files, int x, int y) override;
    
    //==============================================================================
private:
    
    void labelTextChanged (Label*) override {}
    
    void editorShown (Label*, TextEditor&) override
    {
        stopTimer();
    }
    
    void editorHidden (Label*, TextEditor&) override
    {
        startTimer (timerInterval);
    }
    
    void timerCallback() override;
    void checkFilesExistence();
    
    int getRowNumberForFile (LoadedAudioFile* file) const;
    //==============================================================================
    void showAliasEditor (int row);
    void aliasWasEdited (int row, String text);
    //==============================================================================
    void addButtonClicked();
    void playButtonClicked();
    void audioThumbnailClicked (double relativePosition) override;
    void modifyFileWindow (int row);
    void showPopupMenu (int row);
    void addToGraph (int row, String interfaceName);
    //==============================================================================
    friend class UndoableAddFileAction;
    AudioFileItem* addFileUndoable (File f);
    friend class UndoableRemoveFileAction;
    void removeFileUndoable (int row);
    
    Project& project;
    
    TableListBox table;
    PlusButton addButton;
    PlayStopButton playButton;
    SharedResourcePointer<AudioFilesManager> filesManager;
    AudioFilePlayer filePlayer;
    AudioSourcePlayer audioSourcePlayer;
    Playhead playhead;
    ReferenceCountedArray<AudioFileItem> subItems;
    PreviewWidget preview;
    
    class FileInfoComponent : public Component
    {
    public:
        FileInfoComponent() {}
        
        void paint (Graphics& g) override
        {
            g.setColour (Colours::white.withMultipliedAlpha (0.9f));
            g.drawFittedText (currentText, getLocalBounds().reduced (4), Justification::left, 2);
        }
        
        void setCurrentText (String txt)
        {
            currentText = txt;
            repaint();
        }
        
    private:
        String currentText;
    };
    
    FileInfoComponent fileInfo;
    
    int rowSelected = -1;
    
    int timerInterval = 1500;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilesPanel)
};

//==============================================================================
class WebAudioData;

class AudioFileItem : public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<AudioFileItem>;
    
    AudioFileItem (LoadedAudioFile* file, AudioFilesPanel& owner);
    ~AudioFileItem() {}
    
    void setOutputName (String newName);
    String getOutputName() const { return outputName; }
    String getOutputNameWithoutExtension() const
    {
        const String ext (getFile().getFileExtension());
        
        if (outputName.endsWith (ext))
            return outputName.replaceSection (outputName.length() - ext.length(), ext.length(), "");
        
        return outputName;
    }
    
    LoadedAudioFile* getAudioFile() const { return audioFile; }
    
    String getFullPath() const { return audioFile == nullptr ? String() : audioFile->getFullPath(); }
    File getFile() const { return File (getFullPath()); }
    String getDataUrl() const { return "/data/" + outputName; }
    
    const Array<WeakReference<WebAudioData>>& getUsers() { return users; }
    void addUser (WebAudioData* newUser);
    
    Uuid getUuid() const { return uuid; }
    void setUuid (String uuidString) { uuid = uuidString; }
    
    void checkFileExistence();    
    bool hasValidSource() const { return sourceExists; }
    
private:
    AudioFilesPanel& panel;
    WeakReference<LoadedAudioFile> audioFile;
    
    Array<WeakReference<WebAudioData>> users;
    
    String outputName;
    Uuid uuid;
    
    bool sourceExists;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFileItem)
};
