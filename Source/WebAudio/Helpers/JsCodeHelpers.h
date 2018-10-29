/*
  ==============================================================================

    JsCodeHelpers.h
    Created: 21 Aug 2018 11:54:53pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

#include "WebAudioDictionary.h"
#include "JavascriptCodeTokeniser.h"

struct JsCodeHelpers
{
    static bool startsWithDigit (String string)
    {
        for (int i = 0; i < 10; ++i)
            if (string.startsWith (String(i)))
                return true;
        
        return false;
    }
    
    static String removeIllegalCharactersForIdentifier (String wanted)
    {
        return wanted.retainCharacters (getAllowedCharactersForIdentifier());
    }
    
    static String getAllowedCharactersForIdentifier()
    {
        return uppercase() + lowercase() + digits() + "_$";
    }
    
    static String getDefaultInstanceName (const Descriptor &d)
    {
        String interfaceName (d.interf.name.isEmpty() ? d.name : d.interf.name);
        
        if (interfaceName.isEmpty())
            return String();
        
        if (interfaceName == interfaceName.toLowerCase())
            return interfaceName;
        
        if (interfaceName == interfaceName.toUpperCase())
            return interfaceName.toLowerCase();
        
        String name (interfaceName);
        
        if (name.endsWith("Node"))
            name = name.replaceSection (name.length() - 4, 4, "");
        
        String start = name.initialSectionContainingOnly (uppercase());
        
        if (start.isEmpty())
            return name;
            
        start = start.toLowerCase();
        
        if (start.length() > 1)
            start = start.replaceSection (start.length() - 1, 1, "");
        
        return name.replaceSection (0,
                                    start.length(),
                                    start.toLowerCase());
    }
    
    static String getInterfaceDisplayName (const String& name)
    {
        if (name.contains (":"))
        {
            const String templateArg (name.substring (name.indexOfChar (':') + 1));
            const String templateType (name.substring (0, name.indexOfChar (':')));
            
            return templateType + "<" + templateArg + ">";
        }
        
        return name;
    }
    
    static bool isPrimitive (const String& interfaceName)
    {
        return  interfaceName == "int"      || interfaceName == "double"
        || interfaceName == "float"         || interfaceName == "string"
        || interfaceName == "unsigned long" || interfaceName == "boolean";
    }
    
    static String uppercase() {     return "ABCDEFGHIJKLMNOPQRSTUVWXYZ"; }
    static String lowercase() {     return "abcdefghijklmnopqrstuvwxyz"; }
    static String digits() {        return "0123456789"; }
    
    static bool isReservedKeyword (String& name)
    {
        return JsTokeniserFunctions::isReservedKeyword (name.getCharPointer(), name.length());
    }
};

