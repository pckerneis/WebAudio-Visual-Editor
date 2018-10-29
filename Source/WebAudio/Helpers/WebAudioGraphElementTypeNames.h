/*
  ==============================================================================

    WebAudioGraphElementTypeNames.h
    Created: 14 Oct 2018 10:48:15pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

namespace GraphElementType
{
#define DECLARE_ID(name) const juce::String name (#name);
    
    DECLARE_ID (audioContextType)
    DECLARE_ID (audioDataType)
    DECLARE_ID (audioNodeType)
    DECLARE_ID (audioDestinationNodeType)
    DECLARE_ID (dynamicRouteType)
    DECLARE_ID (messageType)
    DECLARE_ID (scriptType)
    DECLARE_ID (commentType)
    
#undef DECLARE_ID
    
    static String getTypeDisplayString (String type);
}

String GraphElementType::getTypeDisplayString (String type)
{
    String name;
    
    if (type == audioContextType)               name = "Audio context";
    else if (type == audioDataType)             name = "Audio data";
    else if (type == audioNodeType)             name = "Audio node";
    else if (type == audioDestinationNodeType)  name = "Audio destination node";
    else if (type == dynamicRouteType)          name = "Dynamic route";
    else if (type == messageType)               name = "Message";
    else if (type == scriptType)                name = "Script";
    
    return name.quoted();
}
