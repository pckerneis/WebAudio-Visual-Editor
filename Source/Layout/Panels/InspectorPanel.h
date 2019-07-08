/*
  ==============================================================================

    InspectorPanel.h
    Created: 16 Aug 2017 9:53:27am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Widgets/PropertyTree.h"

class InspectableElement : public PropertyItem
{
public:
    InspectableElement (ValueTree tree, PropertyTree& pt);
    virtual ~InspectableElement();
    
    virtual void inspectablePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) = 0;
    
    const ValueTree& getInspectableProperties() const { return inspectableProperties; }
    
protected:
    void valueTreePropertyChanged (ValueTree &tree, const Identifier &property) override;
    
    ValueTree inspectableProperties;
    
private:
    friend class InspectorPanel;
    
    WeakReference<InspectableElement>::Master masterReference;
    friend class WeakReference<InspectableElement>;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectableElement)
};

//==============================================================================
#include "Panel.h"

class InspectorPanel :  public Panel
{
public:
    InspectorPanel (PanelManager* manager, PropertyTree* customTree = nullptr, bool useCommonProps = false);
    ~InspectorPanel() {}
    
    void paint (Graphics& g) override;
    void resized() override;
    
    void setInspectedItems (Array<InspectableElement*> elems);
    
    bool showsMultipleItems() const { return inspectedElements.size() > 1; }
    
    PropertyTree* getPropertyTree() { return propertyTree.get(); }
    
    // Panel implementation
    XmlElement* getAsXml() override;
    
protected:
    void populatePropertyPanel();
    virtual void showCommonProperties() {}
    
    ScopedPointer<PropertyTree> propertyTree;
    Array<InspectableElement*> inspectedElements;
    ValueTree inspectedElementsTree;
    
private:
    bool useCommonProperties;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (InspectorPanel)
};
