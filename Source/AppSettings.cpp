/*
  ==============================================================================

    AppSettings.cpp
    Created: 14 Dec 2017 10:24:19pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "AppSettings.h"

PropertiesFile::Options AppSettings::getPropertiesOptions()
{
    PropertiesFile::Options options;
    options.applicationName = "WebAudio Visual Editor";
    options.filenameSuffix = ".settings";
    options.osxLibrarySubFolder = "Application Support";
    options.folderName = "WebAudio Visual Editor";
    options.commonToAllUsers = false;
    
    return options;
}

PropertiesFile* AppSettings::getCommonSettings (ApplicationProperties& ap)
{
    ap.setStorageParameters(getPropertiesOptions());
    return ap.getCommonSettings(true);
}

void AppSettings::initApplicationSettings()
{
    ApplicationProperties ap;
    auto pf = getCommonSettings (ap);
    
    if (!pf->getFile().existsAsFile() || pf->getAllProperties().size() == 0)
    {
        pf->setValue ("tempDirectory", getDefaultTempPath());
        pf->setValue ("projectsDirectory", getDefaultProjectsPath());
        pf->setValue ("userLibraryDirectory", getDefaultLibraryPath());
        
        pf->setValue ("searchString", String());
        pf->setValue ("caseSensitiveSearch", true);
        
        setDefaultLook (pf);
        
        pf->saveIfNeeded ();
    }
}

void AppSettings::setDefaultLook()
{
    ApplicationProperties ap;
    setDefaultLook (getCommonSettings(ap));
}

Font AppSettings::findNativeMonospacedFont()
{
    // A few basic mono typeface names
    StringArray names;
    names.add ("Andale Mono");
    names.add ("Lucida Console");
    names.add ("Courier New");
    
    Array<Font> nativeFonts;
    Font::findFonts (nativeFonts);
    
    for (auto n : names)
        for (auto f : nativeFonts)
            if (f.getTypefaceName() == n)
                return f;
    
    // On Windows 10, looks like the font returned isn't actually monospaced...
    return Font (Font::getDefaultMonospacedFontName(), "", 13.0f);
}

void AppSettings::setDefaultLook (PropertiesFile* pf)
{
    pf->setValue ("mainColour", Colours::steelblue.toString());
    pf->setValue ("brightLook", false); // true means bright
    pf->setValue ("editorFont", findNativeMonospacedFont().toString());
    pf->setValue ("editorFontSize", 12.0f);
    pf->setValue ("editorBackground", Colour().toString());
    
    setCurrentEditorColourScheme (JavascriptTokeniser::getDefaultEditorColourScheme(), pf);
}

File AppSettings::getProjectsDirectory()
{
    ApplicationProperties ap;
    return File(getCommonSettings(ap)->getValue("projectsDirectory", getDefaultProjectsPath()));
}

File AppSettings::getUserLibraryDirectory()
{
    ApplicationProperties ap;
    return File(getCommonSettings(ap)->getValue("userLibraryDirectory", getDefaultLibraryPath()));
}

File AppSettings::getTempDirectory()
{
    ApplicationProperties ap;
    return File(getCommonSettings(ap)->getValue("tempDirectory", getDefaultTempPath()));
}

String AppSettings::getDefaultTempPath()
{
    return File::getSpecialLocation(File::tempDirectory).getFullPathName();
}

String AppSettings::getDefaultProjectsPath()
{
    return File::getSpecialLocation (File::SpecialLocationType::userDocumentsDirectory).getFullPathName() + "/WebAudio Visual Editor/Projects/";
}

String AppSettings::getDefaultLibraryPath()
{
    return File::getSpecialLocation (File::SpecialLocationType::userDocumentsDirectory).getFullPathName() + "/WebAudio Visual Editor/Library/";
}
//==============================================================================

String AppSettings::getCommonStringValue (String propertyName)
{
    ApplicationProperties ap;
    return getCommonSettings(ap)->getValue (propertyName, "");
}

void AppSettings::setCommonStringValue (String propertyName, String value)
{
    ApplicationProperties ap;
    getCommonSettings(ap)->setValue (propertyName, value);
}

bool AppSettings::getCommonBoolValue (String propertyName)
{
    ApplicationProperties ap;
    return getCommonSettings(ap)->getBoolValue (propertyName, false);
}

void AppSettings::setCommonBoolValue (String propertyName, bool value)
{
    ApplicationProperties ap;
    getCommonSettings(ap)->setValue (propertyName, value);
}

Colour AppSettings::getCommonColourValue (String propertyName)
{
    ApplicationProperties ap;
    return Colour::fromString (getCommonSettings(ap)->getValue (propertyName, Colour().toString()));
}

void AppSettings::setCommonColourValue (String propertyName, Colour colour)
{
    ApplicationProperties ap;
    getCommonSettings(ap)->setValue (propertyName, colour.toString());
    ap.saveIfNeeded();
    ap.closeFiles();
}

float AppSettings::getCommonFloatValue (String propertyName)
{
    ApplicationProperties ap;
    return getCommonSettings(ap)->getValue (propertyName, "0").getFloatValue();
}

void AppSettings::setCommonFloatValue (String propertyName, float value)
{
    ApplicationProperties ap;
    getCommonSettings(ap)->setValue (propertyName, value);
}

int AppSettings::getCommonIntValue (String propertyName)
{
    ApplicationProperties ap;
    return getCommonSettings(ap)->getIntValue (propertyName, 0);
}

void AppSettings::setCommonIntValue (String propertyName, int value)
{
    ApplicationProperties ap;
    getCommonSettings(ap)->setValue (propertyName, value);
}
//==============================================================================

String AppSettings::getSearchString() { return getCommonStringValue("searchString"); }
void AppSettings::setSearchString (String string) { setCommonStringValue("searchString", string); }

bool AppSettings::getCaseSensitiveSearch() { return getCommonBoolValue("caseSensitiveSearch"); }
void AppSettings::setCaseSensitiveSearch (bool caseSensitive) { setCommonBoolValue("caseSensitiveSearch", caseSensitive); }

//==============================================================================

Colour AppSettings::getCurrentMainColour()
{
    return getCommonColourValue ("mainColour");
}

#include "WaveLookAndFeel.h"
void AppSettings::setCurrentMainColour (Colour c)
{
    setCommonColourValue ("mainColour", c);
    LookAndFeelUpdater::getLookAndFeel().setMainColour (c);
}

bool AppSettings::isCurrentLookBright() { return getCommonBoolValue ("brightLook"); }
void AppSettings::setCurrentLook (bool bright) { setCommonBoolValue ("brightLook", bright); }

Font AppSettings::getCurrentEditorFont() 
{ 
	auto font = Font::fromString(getCommonStringValue("editorFont")).withHeight(getCurrentEditorFontSize());
	return font; 
}

void AppSettings::setCurrentEditorFont (Font f) { setCommonStringValue ("editorFont", f.toString()); }

float AppSettings::getCurrentEditorFontSize() { return getCommonFloatValue ("editorFontSize"); }
void AppSettings::setCurrentEditorFontSize (float s) { setCommonFloatValue ("editorFontSize", s); }

CodeEditorComponent::ColourScheme AppSettings::getCurrentEditorColourScheme()
{
    auto cs = isCurrentLookBright() ? JavascriptTokeniser::getDefaultEditorColourSchemeBright() : JavascriptTokeniser::getDefaultEditorColourScheme();
    
    for (auto& type : cs.types)
    type.colour = getCommonColourValue ("cs_" + type.name);
    
    return cs;
}

void AppSettings::setCurrentEditorColourScheme (CodeEditorComponent::ColourScheme cs)
{
    ApplicationProperties ap;
    
    if (auto pf = getCommonSettings (ap))
    setCurrentEditorColourScheme (cs, pf);
}

void AppSettings::setCurrentEditorColourScheme (CodeEditorComponent::ColourScheme cs, PropertiesFile* pf)
{
    for (auto& token : cs.types)
    pf->setValue ("cs_" + token.name, token.colour.toString());
}

Colour AppSettings::getCurrentEditorBackgroundColour() { return getCommonColourValue ("editorBackground"); }
void AppSettings::setCurrentEditorBackgroundColour (Colour c) { setCommonColourValue ("editorBackground", c); }

//==============================================================================
RecentlyOpenedFilesList AppSettings::getRecentlyOpenedProjectsList()
{
    RecentlyOpenedFilesList list;
    const auto stored = getCommonStringValue ("recentlyOpenedProjects");
    
    if (stored.isNotEmpty())
    list.restoreFromString (getCommonStringValue ("recentlyOpenedProjects"));
    
    return list;
}

void AppSettings::addToRecentlyOpenedProjects (File projectHeader)
{
    auto list = getRecentlyOpenedProjectsList();
    
    list.setMaxNumberOfItems (10);
    list.addFile (projectHeader);
    
    list.registerRecentFileNatively (projectHeader);
    
    setCommonStringValue ("recentlyOpenedProjects", list.toString());
}

void AppSettings::clearRecentlyOpenedProjectsList()
{
    setCommonStringValue ("recentlyOpenedProjects", String());
    //RecentlyOpenedFilesList::clearRecentFilesNatively();
}


