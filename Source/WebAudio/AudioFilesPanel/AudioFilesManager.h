/*
  ==============================================================================

    AudioFilesManager.h
    Created: 10 Sep 2018 7:04:23pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once
#include "../JuceLibraryCode/JuceHeader.h"

class AudioFilesManager;

//==============================================================================

class LoadedAudioFile : private Thread
{
public:
    LoadedAudioFile (String fileToLoad, AudioFormatManager& afManager, AudioFilesManager* fManager, int sampleRate);
    ~LoadedAudioFile();
    
    bool isNotAvailable() const { return ! available; }
    bool isLoaded() const { return fullyLoaded; }
    
    String getFullPath() const { return filePath; }
    
    AudioThumbnail& getThumbnail() { return thumbnail; }
    AudioSampleBuffer& getBuffer() { return buffer; }
    
    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener()
        {
            if (file != nullptr)
                file->removeListener (this);
        }
        
        virtual void audioFileWasLoaded (LoadedAudioFile* f) = 0;
        
    private:
        WeakReference<LoadedAudioFile> file;
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    //==============================================================================
    
private:
    WeakReference<LoadedAudioFile>::Master masterReference;
    friend class WeakReference<LoadedAudioFile>;
    
    Array<Listener*> listeners;
    
    bool bufferLoaded() const { return buffer.getNumChannels() > 0; }
    bool thumbnailLoaded() const { return thumbnail.isFullyLoaded(); }
    
    void run() override;
    
    void loadBuffer (String pathToOpen);
    void loadThumbnail (String pathToOpen);
    
    void wasLoaded();
    static void* notifyMessageThread (void* userData);
    
    bool fullyLoaded = false;
    bool available = false;
    
    const String filePath;
    
    AudioFormatManager& formatManager;
    AudioSampleBuffer buffer;
    
    ScopedPointer<AudioFormatReaderSource> readerSource;
    
    AudioThumbnailCache thumbnailCache;
    AudioThumbnail thumbnail;
    
    WeakReference<AudioFilesManager> filesManager;
    
    int targetSampleRate;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LoadedAudioFile)
};

//==============================================================================

class AudioFilesManager
{
public:
    AudioFilesManager();
    ~AudioFilesManager();
    
    LoadedAudioFile* loadFile (String path, int targetSampleRate);
    void removeFile (String path);
    
    bool isAlreadyLoaded (String path);
    
    LoadedAudioFile* findFileWithFullPath (String fullPath);
    
    void reset();
    
private:
    WeakReference<AudioFilesManager>::Master masterReference;
    friend class WeakReference<AudioFilesManager>;
    
    AudioFormatManager formatManager;
    
    OwnedArray<LoadedAudioFile> files;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioFilesManager)
};
