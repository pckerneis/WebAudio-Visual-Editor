/*
  ==============================================================================

    UserLibraryManager.h
    Created: 25 Sep 2018 4:37:37pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Application/AppSettings.h"

class GraphElementPreset : public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<GraphElementPreset>;
    
    GraphElementPreset (String n, String interfaceName, File f) : name (n), interf (interfaceName), file (f) {}
    bool operator== (GraphElementPreset& other) { return name == other.name && interf == other.interf && file == other.file; }
    
    String getName() const { return name; }
    void setName (String newName) { name = newName; }

    File getFile() const { return file; }
    void setFile (File newFile) { file = newFile; }
    
    String getInterface() const { return interf; }
    
private:
    String name;
    const String interf;
    File file;
};

//==============================================================================
class GraphSnippet : public GraphElementPreset
{
public:
    GraphSnippet (String n, File f) : GraphElementPreset (n, "Snippets", f) {}
};

//==============================================================================
class UserLibraryManager
{
public:
    UserLibraryManager();
    ~UserLibraryManager() {}
    
    const StringArray& getInterfaceNames() const { return presetInterfaceNames; }
    const ReferenceCountedArray<GraphElementPreset>& getPresets() const { return presets; }
    
    GraphElementPreset* addPresetFile (String wantedName, String interfaceName, String document);
    void removePreset (GraphElementPreset* p);
    void renamePreset (GraphElementPreset* preset, String desiredName);
    
    const ReferenceCountedArray<GraphSnippet>& getSnippets() const { return snippets; }
    GraphSnippet* addSnippetFile (String wantedName, String document);
    
private:
    static File getPresetsDirectory();
    static File getSnippetsDirectory();
    
    void scanLibrary();
    void createPresetDirectory();
    
    StringArray presetInterfaceNames;
    ReferenceCountedArray<GraphElementPreset> presets;
    ReferenceCountedArray<GraphSnippet> snippets;
};
