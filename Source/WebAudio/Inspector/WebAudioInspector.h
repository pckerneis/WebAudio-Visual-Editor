/*
  ==============================================================================

    WebAudioInspector.h
    Created: 4 Sep 2018 12:01:06am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../WebAudioGraph/WebAudioGraph.h"
#include "../Source/Layout/Panels/InspectorPanel.h"

class WebAudioInspectableElement;

class WebAudioInspector : public InspectorPanel, public ValueTree::Listener, public AsyncUpdater
{
    class CustomPropertyTree : public PropertyTree
    {
    public:
        CustomPropertyTree (WebAudioInspector& o) : owner (o) {}
        ~CustomPropertyTree() { masterReference.clear(); }
        
        PropertyItemComponent* createComponentForItem (PropertyItem* item) override;
        
        UndoManager* getUndoManager() override { return owner.getUndoManager(); }
        
    private:
        WebAudioInspectableElement* findElementWithName (String name);
        
        WeakReference<CustomPropertyTree>::Master masterReference;
        friend class WeakReference<CustomPropertyTree>;
        
        WebAudioInspector &owner;
    };
    
public:
    WebAudioInspector (PanelManager* manager, Project& p);
    
    Project& getProject() { return project; }
    
    UndoManager* getUndoManager() const;
    
    void refreshAsync();
    
private:
    void handleAsyncUpdate() override;
    
    // ValueTree::Listener
    void valueTreePropertyChanged (ValueTree &tree, const Identifier &property) override;
    void valueTreeChildAdded (ValueTree &, ValueTree &) override {}
    void valueTreeChildRemoved (ValueTree &, ValueTree &, int) override {}
    void valueTreeChildOrderChanged (ValueTree &, int, int) override {}
    void valueTreeParentChanged (ValueTree &) override {}
    
    struct CommonProperty
    {
        StringArray possibleNames;
        StringArray values;
        String type;
        String attributes;
        bool hasMultipleValues = false;
        ValueTree tree;
        Array<ValueTree> linkedTrees;
    };
    
    OwnedArray<CommonProperty> commonProperties;
    
    CommonProperty* findCommonPropertyWithName (String name);
    CommonProperty* findCommonPropertyWithValueTree (ValueTree& vt);
    
    static CommonProperty* createCommonPropertyWithPossibleNames (String name1, String name2 = String());
    
    void initCommonProperties();
    void showCommonProperties() override;
    
    Array<ValueTree> watchedProperties;
    
    ValueTree commonPropertiesValueTree;
    Project& project;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioInspector)
};
