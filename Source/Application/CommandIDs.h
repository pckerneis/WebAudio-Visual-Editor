/*
  ==============================================================================

    CommandIDs.h
    Created: 28 Aug 2017 2:28:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

namespace CommandIDs
{
    enum CommandIDs
    {
        // these are JUCE defined cmds, the id shouldn't change
        quit                        = 0x1001,
        del                         = 0x1002,
        cut                         = 0x1003,
        copy                        = 0x1004,
        paste                       = 0x1005,
        selectAll                   = 0x1006,
        deselectAll                 = 0x1007,
        undo                        = 0x1008,
        redo                        = 0x1009,
        
        projectNew                  = 0x2100,
        projectLoad                 = 0x2101,
        projectSave                 = 0x2000,
        projectSaveAs               = 0x2001,
        projectClose                = 0x2002,
        
        openPreferences             = 0x2201,
        openAboutWindow             = 0x2202,
        
        duplicateSelection          = 0x4004,
        pasteAtMousePos             = 0x4008,
        
        rename                      = 0x5000,
        showFindPanel               = 0x5001,
        findNext                    = 0x5002,
        findPrevious                = 0x5003,
        
        applyChanges                = 0x5010,
        
        openInTabs                  = 0x6000,
        closeTab                    = 0x6001,
        
        clearConsole                = 0x7000,
        
        generateOutput              = 0x8010,
        testInBrowser               = 0x8011,
        revealOutputDirectory       = 0x8012,
        
        restoreDefaultPanelLayout   = 0x9000
    };
}
