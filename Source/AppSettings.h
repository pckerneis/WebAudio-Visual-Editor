/*
  ==============================================================================

    AppSettings.h
    Created: 8 Jul 2017 8:02:22pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "JavascriptCodeTokeniser.h"

/** \brief Static methods to access the app properties file */
struct AppSettings
{
    /** \brief Properties file options provided to access config file.  */
    static PropertiesFile::Options getPropertiesOptions();
    
    /** \brief Shortcut to main properties file.  */
    static PropertiesFile* getCommonSettings (ApplicationProperties& ap);
    
    /** \brief Application settings initialisation. Creates a default properties file if needed. */
    static void initApplicationSettings();
    
    static void setDefaultLook();
    static void setDefaultLook (PropertiesFile* pf);
    
    //==============================================================================
    /** \brief Get the user's project directory */
    static File getProjectsDirectory();
    
    static File getUserLibraryDirectory();
    static File getTempDirectory();
    static String getDefaultTempPath();
    static String getDefaultProjectsPath();
    static String getDefaultLibraryPath();
    
    //==============================================================================
    // Shortcuts to get/set properties
    static String getCommonStringValue (String propertyName);
    static void setCommonStringValue (String propertyName, String value);
    
    static bool getCommonBoolValue (String propertyName);
    static void setCommonBoolValue (String propertyName, bool value);
    
    static Colour getCommonColourValue (String propertyName);
    static void setCommonColourValue (String propertyName, Colour colour);
    
    static float getCommonFloatValue (String propertyName);
    static void setCommonFloatValue (String propertyName, float value);
    
    static int getCommonIntValue (String propertyName);
    static void setCommonIntValue (String propertyName, int value);
    
    //==============================================================================
    static String getSearchString();
    static void setSearchString (String string);
    
    static bool getCaseSensitiveSearch();
    static void setCaseSensitiveSearch (bool caseSensitive);
    
    //==============================================================================
    static Colour getCurrentMainColour();
    static void setCurrentMainColour (Colour c);
    
    static bool isCurrentLookBright();
    static void setCurrentLook (bool bright);
    
    static Font getCurrentEditorFont();
    static void setCurrentEditorFont (Font f);
    
    static float getCurrentEditorFontSize();
    static void setCurrentEditorFontSize (float s);
    
    static CodeEditorComponent::ColourScheme getCurrentEditorColourScheme();
    static void setCurrentEditorColourScheme (CodeEditorComponent::ColourScheme cs);
    static void setCurrentEditorColourScheme (CodeEditorComponent::ColourScheme cs, PropertiesFile* pf);
    
    static Colour getCurrentEditorBackgroundColour();
    static void setCurrentEditorBackgroundColour (Colour c);
    
    //==============================================================================
    static RecentlyOpenedFilesList getRecentlyOpenedProjectsList();
    static void addToRecentlyOpenedProjects (File projectHeader);
    static void clearRecentlyOpenedProjectsList();
};
