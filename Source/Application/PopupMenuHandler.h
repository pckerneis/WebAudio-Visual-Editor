/*
  ==============================================================================

    PopupMenuHandler.h
    Created: 18 Aug 2018 4:14:57pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"
/** \brief Object that holds a weak reference to the object that triggered a popup menu.
 *
 *  Because the caller object may have been deleted by a "del" command, we need to check
 *  that it's still existing before using it.
 */
class PopupMenuHandler
{
public:
    PopupMenuHandler() {}
    
    //==============================================================================
    class PopupMenuHandlerClient
    {
    public:
        virtual ~PopupMenuHandlerClient() { masterReference.clear(); }
        virtual void handleExtraPopupMenuCommands (int result, Point<int> pos) {};
        
    private:
        WeakReference<PopupMenuHandlerClient>::Master masterReference;
        friend class WeakReference<PopupMenuHandlerClient>;
    };
    //==============================================================================
    
    void setCaller (PopupMenuHandlerClient* comp);
    void handleResult (int result, Point<int> pos);
    
private:
    WeakReference<PopupMenuHandlerClient> caller;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PopupMenuHandler)
};
