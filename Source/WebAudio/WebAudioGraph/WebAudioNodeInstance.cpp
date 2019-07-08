/*
  ==============================================================================

    WebAudioNodeInstance.cpp
    Created: 23 Aug 2018 12:18:31am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioNodeInstance.h"

#include "WebAudioDictionary.h"
#include "../GraphElements/WebAudioInspectableElement.h"

WebAudioNodeInstance::WebAudioNodeInstance (String instanceName, String interfaceName, WebAudioInspectableElement* ref, Array<Descriptor> o) : name (instanceName), interf (interfaceName)
{
    SharedResourcePointer<WebAudioDictionary> dict;
    interfaceDescriptor = Descriptor (dict->findDescriptorForInterface (interfaceName));
    
    auto optionsDescr = dict->findOptionsDescriptorForAudioNode (interfaceName);
    
    if (o.isEmpty())
    {
        for (auto prop : optionsDescr.interf.properties)
            options.add (new Descriptor (prop));
    }
    else
    {
        for (auto prop : o)
            options.add (new Descriptor (prop));
    }
    
    references.add (ref);
}

int WebAudioNodeInstance::getNumReferencesCurrentlyInGraph() const
{
    int num = 0;
    
    for (auto elem : references)
        if (auto webElem = dynamic_cast<WebAudioEmbedded*> (elem.get()))
            if (webElem->getParentGraph() != nullptr)
                ++num;
    
    return num;
}

void WebAudioNodeInstance::optionsChanged()
{
    for (auto n : references)
        n->refreshOptionsTree();
}

//==============================================================================

WebAudioNodeInstance* WebAudioInstanceManager::addReference (WebAudioInspectableElement* node)
{
    if (node == nullptr)
        return nullptr;
    
    const String name = node->getElementName();
    const String interf = node->getInterfaceName();
    
    if (auto instance = findInstanceWithName (name))
    {
        // You shouldn't add again a reference with a different interface...
        jassert (interf == instance->getInterface());
        
        instance->references.addIfNotAlreadyThere (node);
        node->setInstance (instance);
        
        return instance;
    }
    
    auto newInstance = new WebAudioNodeInstance (name, interf, node);
    instances.add (newInstance);
    node->setInstance (newInstance);
    
    return newInstance;
}

WebAudioNodeInstance* WebAudioInstanceManager::updateReference (WebAudioInspectableElement* node)
{
    if (node == nullptr)
        return nullptr;
    
    auto currentInstance = node->getInstance();
    
    if (currentInstance == nullptr)
        return addReference (node);
    
    Array<Descriptor> options;
    
    for (auto &o : currentInstance->getOptions())
        options.add (*o);
    
    removeReference (node);
    const String name = node->getElementName();
    const String interf = node->getInterfaceName();
    
    if (auto instance = findInstanceWithName (name))
    {
        // You shouldn't add again a reference with a different interface...
        jassert (interf == instance->getInterface());
        
        instance->references.addIfNotAlreadyThere (node);
        node->setInstance (instance);
        return instance;
    }
    
    auto newInstance = new WebAudioNodeInstance (name, interf, node, options);
    //newInstance->getOptions();
    instances.add (newInstance);
    node->setInstance (newInstance);
    
    return newInstance;
}

void WebAudioInstanceManager::removeReference (WebAudioInspectableElement* node)
{
    if (node == nullptr)
        return;
    
    if (auto instance = node->getInstance())
    {
        instance->references.removeFirstMatchingValue (node);
        
        if (instance->references.isEmpty())
            instances.removeObject (instance);
    }
}

WebAudioNodeInstance* WebAudioInstanceManager::findInstanceWithName (String variableName) const
{
    for (auto instance : instances)
        if (instance->getName() == variableName)
            return instance;
    
    return nullptr;
}
