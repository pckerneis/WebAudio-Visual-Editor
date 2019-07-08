/*
  ==============================================================================

    UserLibraryManager.cpp
    Created: 25 Sep 2018 4:37:37pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/LibraryPanel/UserLibraryManager.h"

UserLibraryManager::UserLibraryManager()
{
    scanLibrary();
}

GraphElementPreset* UserLibraryManager::addPresetFile (String wantedName, String interfaceName, String document)
{
    if (! getPresetsDirectory().exists())
        createPresetDirectory();
    
    auto d = getPresetsDirectory().getChildFile (interfaceName);
    
    if (! d.isDirectory())
        d.createDirectory();
    
    auto output = d.getNonexistentChildFile (wantedName, ".xml", true);
    
    FileOutputStream fos (output);
    
    if (fos.openedOk())
    {
        fos.writeText (document, false, false, "\r\n");

        auto newPreset = new GraphElementPreset (output.getFileNameWithoutExtension(), interfaceName, output);
        presets.add (newPreset);
        
        return newPreset;
    }
    
    // Retry later ?
    jassertfalse;
    
    return nullptr;
}

void UserLibraryManager::removePreset (GraphElementPreset* p)
{
    if (p == nullptr)
        return;
    
    if (p->getFile().existsAsFile())
        p->getFile().moveToTrash();
    
    presets.removeObject (p);
}

void UserLibraryManager::renamePreset (GraphElementPreset* preset, String desiredName)
{
    if (preset == nullptr)
        return;
    
    auto file = preset->getFile();
    
    if (file.getFileNameWithoutExtension() == desiredName)
        return;
    
    auto newOutput = file.getParentDirectory().getNonexistentChildFile (desiredName, ".xml", true);
    
    if (newOutput == file)
        return;
    
    file.moveFileTo (newOutput);
    preset->setFile (newOutput);
    preset->setName (newOutput.getFileNameWithoutExtension());
}

GraphSnippet* UserLibraryManager::addSnippetFile (String wantedName, String document)
{
    auto d = getSnippetsDirectory();
    
    if (d.isDirectory())
    {
        auto output = d.getNonexistentChildFile (wantedName, ".xml", true);
        
        FileOutputStream fos (output);
        
        if (fos.openedOk())
        {
            fos.writeText (document, false, false, "\r\n");
            
            auto newPreset = new GraphSnippet (output.getFileNameWithoutExtension(), output);
            snippets.add (newPreset);
            
            return newPreset;
        }
    }
    
    return nullptr;
}

File UserLibraryManager::getPresetsDirectory()
{
    return AppSettings::getUserLibraryDirectory().getChildFile ("Presets");
}

File UserLibraryManager::getSnippetsDirectory()
{
    return AppSettings::getUserLibraryDirectory().getChildFile ("Snippets");
}

void UserLibraryManager::scanLibrary()
{
    presetInterfaceNames.clear();
    presets.clear();
    
    auto presetsDir = getPresetsDirectory();
    
    if (! presetsDir.exists() || presetsDir.getNumberOfChildFiles (File::findFilesAndDirectories) == 0)
        createPresetDirectory();
    else
    {
        Array<File> childDirectories;
        presetsDir.findChildFiles(childDirectories, File::findDirectories, false);
        
        for (auto f : childDirectories)
        {
            const String interfaceName (f.getFileNameWithoutExtension());
            presetInterfaceNames.add (interfaceName);
            
            Array<File> presetFiles;
            f.findChildFiles (presetFiles, File::findFiles, false);
            
            for (auto pf : presetFiles)
                if (pf.getFileExtension() == ".xml")
                    presets.add (new GraphElementPreset (pf.getFileNameWithoutExtension(), interfaceName, pf));
        }
    }
    
    snippets.clear();
    
    auto snippetsDir = getSnippetsDirectory();
    
    if (! snippetsDir.exists())
        snippetsDir.createDirectory();
    else
    {
        Array<File> snippetFiles;
        snippetsDir.findChildFiles (snippetFiles, File::findFiles, false);
        
        for (auto pf : snippetFiles)
            if (pf.getFileExtension() == ".xml")
                snippets.add (new GraphSnippet (pf.getFileNameWithoutExtension(), pf));
    }
}

#include "../WebAudioGraph/WebAudioDictionary.h"
void UserLibraryManager::createPresetDirectory()
{
    auto presetsDir = getPresetsDirectory();
    
    presetsDir.createDirectory();
    
    SharedResourcePointer<WebAudioDictionary> dict;
    
    for (auto n : dict->getNodeInterfaceNames())
    {
        presetsDir.getChildFile(n).createDirectory();
        presetInterfaceNames.add (n);
    }
    
    for (auto n : dict->getContextInterfaceNames())
    {
        presetsDir.getChildFile(n).createDirectory();
        presetInterfaceNames.add (n);
    }
    
    for (auto n : dict->getAudioDataInterfaceNames())
    {
        presetsDir.getChildFile(n).createDirectory();
        presetInterfaceNames.add (n);
    }
    
}
