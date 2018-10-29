/*
  ==============================================================================

    AudioFilesManager.cpp
    Created: 10 Sep 2018 7:04:23pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "AudioFilesManager.h"

LoadedAudioFile::LoadedAudioFile (String fileToLoad, AudioFormatManager& afManager, AudioFilesManager* fManager, int sampleRate)
: Thread (fileToLoad), filePath (fileToLoad), formatManager (afManager),thumbnailCache (3), thumbnail (512, formatManager, thumbnailCache), filesManager (fManager), targetSampleRate (sampleRate)
{
    const File f (fileToLoad);
    
    if (f.existsAsFile())
    {
        loadThumbnail (filePath);
        available = true;
        startThread (500);
    }
    else
        available = false;
}

LoadedAudioFile::~LoadedAudioFile()
{
    stopThread (500);
    
    masterReference.clear();
}

void LoadedAudioFile::run()
{
    while (! threadShouldExit())
    {
        if (!bufferLoaded())
            loadBuffer (filePath);
        
        if (bufferLoaded() && thumbnailLoaded())
        {
            auto msgManager = MessageManager::getInstance();
            msgManager->callFunctionOnMessageThread (notifyMessageThread, (void*) this);
            
            fullyLoaded = true;
            
            signalThreadShouldExit ();
        }
        
        wait (500);
    }
}

void LoadedAudioFile::loadBuffer (String pathToOpen)
{
    if (pathToOpen.isNotEmpty())
    {
        const File file (pathToOpen);
        ScopedPointer<AudioFormatReader> reader (formatManager.createReaderFor (file));
        
        double ratio = reader->sampleRate / targetSampleRate;
        
        AudioSampleBuffer tempBuffer;
        
        tempBuffer.setSize(reader->numChannels, (int)reader->lengthInSamples);
        
        
        if (reader != nullptr)
            reader->read (&tempBuffer, 0, (int)reader->lengthInSamples, 0, true, true);
        
        if (ratio == 1.0f)
            buffer.makeCopyOf(tempBuffer);
        else
        {
            buffer.setSize(reader->numChannels, (int)reader->lengthInSamples / ratio);
            
            for (int channel = 0; channel < tempBuffer.getNumChannels(); channel++)
            {
                ScopedPointer<LagrangeInterpolator> interpolator = new LagrangeInterpolator();
                
                const float *input = tempBuffer.getArrayOfReadPointers()[channel];
                float *output = buffer.getArrayOfWritePointers()[channel];
                
                interpolator->process (ratio, input, output, buffer.getNumSamples());
            }
        }
    }
}

void LoadedAudioFile::loadThumbnail (String pathToOpen)
{
    if (pathToOpen.isNotEmpty())
    {
        const File file (pathToOpen);
        AudioFormatReader* reader (formatManager.createReaderFor (file));
        
        if (reader != nullptr)
        {
            ScopedPointer<AudioFormatReaderSource> newSource = new AudioFormatReaderSource (reader, true);
            thumbnail.setSource (new FileInputSource (file));
            readerSource = newSource.release();
        }
    }
}

void LoadedAudioFile::wasLoaded()
{
    for (auto l : listeners)
        l->audioFileWasLoaded (this);
}

void* LoadedAudioFile::notifyMessageThread (void* userData)
{
    static_cast<LoadedAudioFile*> (userData)->wasLoaded();
    return nullptr;
}

//==============================================================================

AudioFilesManager::AudioFilesManager()
{
    formatManager.registerBasicFormats();
}

AudioFilesManager::~AudioFilesManager()
{
    masterReference.clear();
}

LoadedAudioFile* AudioFilesManager::loadFile (String path, int targetSampleRate)
{
    auto laf = new LoadedAudioFile (path, formatManager, this, targetSampleRate);
    files.add (laf);
    
    return laf;
}

void AudioFilesManager::removeFile (String path)
{
    for (int i = files.size(); --i >= 0;)
        if (files.getUnchecked(i)->getFullPath() == path)
            files.remove (i);
}

bool AudioFilesManager::isAlreadyLoaded (String path)
{
    for (auto f : files)
        if (f->getFullPath() == path)
            return true;
    
    return false;
}

LoadedAudioFile* AudioFilesManager::findFileWithFullPath (String fullPath)
{
    if (fullPath != String())
        for (auto f : files)
            if (f->getFullPath() == fullPath)
                return f;
    
    return nullptr;
}

void AudioFilesManager::reset()
{
    files.clear();
}
