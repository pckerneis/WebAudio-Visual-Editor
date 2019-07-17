/*
  ==============================================================================

    InspectorPanel.cpp
    Created: 16 Aug 2017 9:53:27am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "InspectorPanel.h"

InspectableElement::InspectableElement (ValueTree tree, PropertyTree& pt) : PropertyItem (tree, pt)
{
    inspectableProperties = tree;
    inspectableProperties.addListener (this);
}

InspectableElement::~InspectableElement()
{
    masterReference.clear();
}

void InspectableElement::valueTreePropertyChanged (ValueTree &tree, const Identifier &property)
{
    PropertyItem::valueTreePropertyChanged (tree, property);
    
    // Because this property change really often and should be hidden from the actual inspectable properties
    if (property != Identifier ("OPEN"))
        inspectablePropertyChanged (tree, property);
}

//==============================================================================
InspectorPanel::InspectorPanel (PanelManager* manager, PropertyTree* customTree, bool commonProps) : Panel (manager), useCommonProperties (commonProps)
{
    //propertyTree = customTree == nullptr ? new PropertyTree() : customTree;
    
    inspectedElementsTree = propertyTree->createTree ("LABEL", "Selected");
    //propertyTree->setRootItemVisible (false);
    //addAndMakeVisible (propertyTree);
    setPanelName ("Inspector");
}

void InspectorPanel::paint (Graphics& g)
{
    Panel::paint (g);
    g.setColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    g.fillRect (getLocalBounds().withTrimmedTop (getHeaderHeight()));
}

void InspectorPanel::resized()
{
    Panel::resized();
	
	if (propertyTree != nullptr)
		propertyTree->setBounds (getLocalBounds().withTrimmedTop(getHeaderHeight()));
}

void InspectorPanel::setInspectedItems (Array<InspectableElement*> elems)
{
    inspectedElements = elems;
    populatePropertyPanel();
}

void InspectorPanel::populatePropertyPanel()
{
    inspectedElementsTree.removeAllChildren (nullptr);
    
    if (useCommonProperties && inspectedElements.size() > 1)
        return showCommonProperties();
    
    int index = -1;
    
    for (auto elem : inspectedElements)
        inspectedElementsTree.addChild (elem->inspectableProperties, ++index, nullptr);
    

	if (propertyTree == nullptr)
		return;

    propertyTree->loadValueTree (inspectedElementsTree, 1);
    propertyTree->repaint();
}

XmlElement* InspectorPanel::getAsXml()
{
    auto e = new XmlElement("InspectorPanel");
    e->setAttribute ("panelId", getPanelId());
    
    return e;
}
