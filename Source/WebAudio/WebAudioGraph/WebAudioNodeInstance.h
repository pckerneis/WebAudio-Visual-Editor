/*
  ==============================================================================

    WebAudioNodeInstance.h
    Created: 23 Aug 2018 12:18:31am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioDictionary.h"

class WebAudioInspectableElement;

class WebAudioNodeInstance
{
public:
    WebAudioNodeInstance (String instanceName, String interfaceName, WebAudioInspectableElement* reference, Array<Descriptor> options = Array<Descriptor>());
    ~WebAudioNodeInstance() { masterReference.clear(); }
    
    String getName() const { return name; }
    String getInterface() const { return interf; }
    
    int getNumReferences() const { return references.size(); }
    Array<WeakReference<WebAudioInspectableElement>> getReferences() const { return references; }
    // getNumReferences can be misleading as there can be "hidden" references to an instance
    // e.g. in an UndoableAction
    int getNumReferencesCurrentlyInGraph() const;
    
    OwnedArray<Descriptor>& getOptions() { return options; }
    
    Descriptor& getInterfaceDescriptor() { return interfaceDescriptor; }
    const Descriptor& getInterfaceDescriptor() const { return interfaceDescriptor; }
    
    void optionsChanged();
    
private:
    WeakReference<WebAudioNodeInstance>::Master masterReference;
    friend class WeakReference<WebAudioNodeInstance>;
    
    friend class WebAudioInstanceManager;
    
    Array<WeakReference<WebAudioInspectableElement>> references;
    
    Descriptor interfaceDescriptor;
    const String name;
    const String interf;
    OwnedArray<Descriptor> options;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioNodeInstance)
};

//==============================================================================

class WebAudioInstanceManager
{
public:
    WebAudioInstanceManager() {}
    
    WebAudioNodeInstance* addReference (WebAudioInspectableElement* node);
    WebAudioNodeInstance* updateReference (WebAudioInspectableElement* node);
    void removeReference (WebAudioInspectableElement* node);
    
    WebAudioNodeInstance* findInstanceWithName (String variableName) const;
    
private:
    OwnedArray<WebAudioNodeInstance> instances;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioInstanceManager)
};
