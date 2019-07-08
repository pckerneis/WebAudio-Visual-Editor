/*
  ==============================================================================

    WebAudioNodeDictionnary.cpp
    Created: 18 Aug 2018 2:31:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioDictionary.h"

Descriptor::Descriptor() : interf ("undefined") {}

Descriptor::Descriptor (String interfaceName) : name (interfaceName), interf ("") {}

Descriptor::Descriptor (String n, Interface interf, String defaultV, String attribs, String help) : name (n), interf (interf), defaultValue (defaultV), attributes (attribs), helpText (help) {}

Descriptor::Descriptor (const Descriptor& other) : name (other.name), interf (other.interf), defaultValue (other.defaultValue), attributes (other.attributes), helpText (other.helpText) {}

#include "../Helpers/JsCodeHelpers.h"
bool Descriptor::isPrimitive() const
{
    return JsCodeHelpers::isPrimitive (interf.name);
}

bool Descriptor::isUndefined() const
{
    return interf.name == "undefined";
}

String Descriptor::getInterfaceDisplayName() const
{
    return JsCodeHelpers::getInterfaceDisplayName (interf.name);
}

//==============================================================================

int Descriptor::Method::getNumRequiredArguments() const
{
    int result = 0;
    
    for (auto a : args)
    if (! a.attributes.contains ("optional"))
    ++result;
    
    return result;
}

//==============================================================================

WebAudioDictionary::WebAudioDictionary()
{
    const MemoryBlock block (BinaryData::dictionary_xml, BinaryData::dictionary_xmlSize);
    const String str (block.toString());
    const std::unique_ptr<XmlElement> xml (XmlDocument::parse (str));
    
    forEachXmlChildElementWithTagName(*xml, enumXml, "ENUM")
    {
        StringArray properties;
        
        const String name (enumXml->getStringAttribute ("name"));
        
        StringArray elements;
        
        forEachXmlChildElementWithTagName(*enumXml, element, "ELEMENT")
        {
            const String name (element->getStringAttribute("name").quoted());
            elements.add (name);
        }
        
        const EnumDescriptor newEnum ({ name, elements });
        enumDescriptors.add (newEnum);
    }
    
    forEachXmlChildElementWithTagName(*xml, interfaceXml, "INTERFACE")
    {
        const String interfaceName (interfaceXml->getStringAttribute ("name"));
        const String descriptorCategory (interfaceXml->getStringAttribute ("category"));
        const String defaultValue (interfaceXml->getStringAttribute ("defaultValue"));
        
        Descriptor newDescriptor (interfaceName, Descriptor::Interface(interfaceName), defaultValue);
        
        if (auto constrXml = interfaceXml->getChildByName ("CONSTRUCTOR"))
            newDescriptor.interf.constructor = constrXml->getStringAttribute ("value");
        
        Array<Descriptor> properties;
        forEachXmlChildElementWithTagName(*interfaceXml, propertyXml, "PROPERTY")
        {
            const String propertyName (propertyXml->getStringAttribute("name"));
            const String interf (propertyXml->getStringAttribute("interface"));
            const bool isEnum = isEnumInterfaceName (interf);
            
            const String defaultValue (propertyXml->getStringAttribute ("defaultValue"));
            const String attributes (propertyXml->getStringAttribute("attributes"));
            Descriptor paramDescriptor (propertyName, interf, isEnum ? defaultValue.quoted() : defaultValue, attributes);
            
            if (auto overXml = propertyXml->getChildByName ("OVERRIDE"))
            {
                for (int i = 0; i < overXml->getNumAttributes(); ++i)
                {
                    const String nestedName = propertyName + "." + overXml->getAttributeName (i);
                    
                    newDescriptor.interf.overriden.set (nestedName,
                                                           overXml->getAttributeValue(i));
                }
            }
            
            properties.add (paramDescriptor);
        }
        
        newDescriptor.interf.properties.swapWith (properties);
        
        Array<Descriptor::Method> methods;
        forEachXmlChildElementWithTagName(*interfaceXml, methodXml, "METHOD")
        {
            const String methodName (methodXml->getStringAttribute("name"));
            const String methodReturns (methodXml->getStringAttribute("returns"));
            const String methodHelp (getFirstTextInChildElements (methodXml));
            const String memberOf (interfaceName);
            
            Array<Descriptor> arguments;
            
            forEachXmlChildElementWithTagName (*methodXml, argXml, "ARGUMENT")
            {
                const String argName (argXml->getStringAttribute ("name"));
                const String argInterface (argXml->getStringAttribute ("interface"));
                const String defaultValue (argXml->getStringAttribute ("defaultValue"));
                const String attributes (argXml->getStringAttribute ("attributes"));
                const bool argIsEnum = isEnumInterfaceName (argInterface);
                const String help (getFirstTextInChildElements (argXml));
                
                Descriptor argDescriptor (argName, argInterface, argIsEnum ? defaultValue.quoted() : defaultValue, attributes, help);
                
                arguments.add (argDescriptor);
            }
            
            methods.add (Descriptor::Method ({ methodName, methodReturns, arguments, memberOf, methodHelp }));
        }
        
        newDescriptor.interf.methods.swapWith (methods);
        
        if (auto inhXml = interfaceXml->getChildByName ("INHERITANCE"))
        {
            newDescriptor.interf.inheritance = inhXml->getStringAttribute ("name");
            
            if (auto overXml = inhXml->getChildByName ("OVERRIDE"))
            {
                for (int i = 0; i < overXml->getNumAttributes(); ++i)
                    newDescriptor.interf.overriden.set (overXml->getAttributeName(i),
                                                           overXml->getAttributeValue(i));
            }
        }
        
        if (descriptorCategory == "NODE")
            nodeDescriptors.add (newDescriptor);
        else if (descriptorCategory == "CONTEXT")
            contextDescriptors.add (newDescriptor);
        else if (descriptorCategory == "AUDIODATA")
            audioDataDescriptors.add (newDescriptor);
        else
            otherDescriptors.add (newDescriptor);
    }
    
    forEachXmlChildElementWithTagName(*xml, dictXml, "DICT")
    {
        const String name (dictXml->getStringAttribute ("name"));
        
        Descriptor newDescriptor (name, Descriptor::Interface(name));
        
        Array<Descriptor> properties;
        forEachXmlChildElementWithTagName(*dictXml, propertyXml, "PROPERTY")
        {
            const String name (propertyXml->getStringAttribute("name"));
            const String interf (propertyXml->getStringAttribute("interface"));
            const String defaultValue (propertyXml->getStringAttribute ("defaultValue"));
            const String attributes (propertyXml->getStringAttribute("attributes"));
            
            Descriptor paramDescriptor (name, interf, defaultValue, attributes);
            
            properties.add (paramDescriptor);
        }
        
        newDescriptor.interf.properties.swapWith (properties);
        
        if (auto inhXml = dictXml->getChildByName ("INHERITANCE"))
            newDescriptor.interf.inheritance = inhXml->getStringAttribute ("name");
        
        dictDescriptors.add (newDescriptor);
    }
}

Descriptor WebAudioDictionary::findDescriptorForInterface (String interfaceName)
{
    for (auto d : nodeDescriptors)
        if (d.interf.name == interfaceName)
            return d;
    
    for (auto d : contextDescriptors)
        if (d.interf.name == interfaceName)
            return d;
    
    for (auto d : audioDataDescriptors)
        if (d.interf.name == interfaceName)
            return d;
    
    for (auto d : otherDescriptors)
        if (d.interf.name == interfaceName)
            return d;
    
    return Descriptor();
}

Descriptor WebAudioDictionary::findOptionsDescriptorForAudioNode (String interfaceName)
{
    String optionsInterface (interfaceName);
    
    if (optionsInterface.endsWith ("Node"))
        optionsInterface = optionsInterface.replaceSection (interfaceName.length() - 4, 4, "");
    
    optionsInterface += "Options";
    
    for (auto d : dictDescriptors)
        if (d.interf.name == optionsInterface)
            return d;
    
    return Descriptor();
}

EnumDescriptor WebAudioDictionary::findEnumWithName (String name)
{
    for (auto d : enumDescriptors)
        if (d.name == name)
            return d;
    
    return EnumDescriptor();
}

StringArray WebAudioDictionary::getNodeInterfaceNames()
{
    StringArray result;
    
    for (auto& descriptor : nodeDescriptors)
        result.add (descriptor.interf.name);
    
    return result;
}

const Array<Descriptor>& WebAudioDictionary::getNodeDescriptors()
{
    return nodeDescriptors;
}

StringArray WebAudioDictionary::getContextInterfaceNames()
{
    StringArray result;
    
    for (auto& descriptor : contextDescriptors)
        result.add (descriptor.interf.name);
    
    return result;
}

const Array<Descriptor>& WebAudioDictionary::getContextDescriptors()
{
    return contextDescriptors;
}

StringArray WebAudioDictionary::getAudioDataInterfaceNames()
{
    StringArray result;
    
    for (auto& descriptor : audioDataDescriptors)
        result.add (descriptor.interf.name);
    
    return result;
}

const Array<Descriptor>& WebAudioDictionary::getAudioDataDescriptors()
{
    return audioDataDescriptors;
}

const Array<Descriptor>& WebAudioDictionary::getDictionaryDescriptors()
{
    return dictDescriptors;
}

String WebAudioDictionary::getFirstTextInChildElements (XmlElement* e, bool trimAndRemoveNewLines) const
{
    if (e == nullptr)
        return String();
    
    forEachXmlChildElement (*e, c)
    {
        if (c->isTextElement())
        {
            auto s = c->getText();
            return trimAndRemoveNewLines ? s.removeCharacters (newLine).trim() : s;
        }
    }
    
    return String();
}

bool WebAudioDictionary::isEnumInterfaceName (String interf) const
{
    for (auto e : enumDescriptors)
        if (e.name == interf)
            return true;
    
    return false;
}
