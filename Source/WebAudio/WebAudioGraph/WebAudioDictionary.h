/*
  ==============================================================================

    WebAudioNodeDictionnary.h
    Created: 18 Aug 2018 2:31:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct Descriptor
{
    struct Method
    {
        String name;
        String returns;
        Array<Descriptor> args;
        String memberOf;
        
        String helpText;
        
        int getNumRequiredArguments() const;
    };
    
    struct Interface
    {
        Interface (String n) : name (n) {}
        
        String name;
        String constructor;
        Array<Descriptor> properties;
        Array<Method> methods;
        String inheritance;
        StringPairArray overriden;
    };
    
    /** \brief Invalid Descriptor constructor */
    Descriptor();
    
    /** \brief Constructor for descriptors without special interface (such as MESSAGE descriptors) */
    Descriptor (String descriptorName);
    
    /** \brief Complete Descriptor constructor */
    Descriptor (String n, Interface interf, String defaultV = String(), String attribs = String(), String help = String());
    
    /** \brief Copy constructor */
    Descriptor (const Descriptor& other);
    
    bool isPrimitive() const;
    bool isUndefined() const;
    
    String getInterfaceDisplayName() const;
    
    //==============================================================================
    String name;
    Interface interf;
    String defaultValue;
    String attributes;
    
    String helpText;
};

//==============================================================================

struct EnumDescriptor
{
    const String name;
    const StringArray choices;
    
    bool isValid() const { return (name != String()); }
};

//==============================================================================

class WebAudioDictionary
{
public:
    WebAudioDictionary();
    
    Descriptor findDescriptorForInterface (String interfaceName);
    Descriptor findOptionsDescriptorForAudioNode (String interfaceName);
    
    
    StringArray getNodeInterfaceNames();
    const Array<Descriptor>& getNodeDescriptors();
    
    StringArray getContextInterfaceNames();
    const Array<Descriptor>& getContextDescriptors();
    
    StringArray getAudioDataInterfaceNames();
    const Array<Descriptor>& getAudioDataDescriptors();
    
    const Array<Descriptor>& getDictionaryDescriptors();
    
    EnumDescriptor findEnumWithName (String enumName);
    
private:
    String getFirstTextInChildElements (XmlElement* e, bool trimAndRemoveNewLines = true) const;
    
    bool isEnumInterfaceName (String interf) const;
    
    Array<Descriptor> nodeDescriptors;
    Array<Descriptor> contextDescriptors;
    Array<Descriptor> audioDataDescriptors;
    Array<Descriptor> otherDescriptors;
    Array<Descriptor> dictDescriptors;
    Array<EnumDescriptor> enumDescriptors;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioDictionary)
};
