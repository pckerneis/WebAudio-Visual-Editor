/*
  ==============================================================================

    WebAudioInspector.cpp
    Created: 4 Sep 2018 12:01:06am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioInspector.h"

#include "PropertyComponentTypes.h"
#include "JsCodeHelpers.h"
#include "WebAudioInspectableElement.h"

struct ElementNameConstrainer : public PropertyValueConstrainer
{
public:
    ElementNameConstrainer (WebAudioInspectableElement* e) : element (e) {}
    
    String constrain (String value) override
    {
        if (element == nullptr)
            return value;
        
        return element->getValidName (value);
    }
    
private:
    WeakReference<WebAudioInspectableElement> element;
};


PropertyItemComponent* WebAudioInspector::CustomPropertyTree::createComponentForItem (PropertyItem* item)
{
    if (item == nullptr)
        return nullptr;
    
    const String name =  item->getValueTree()["name"];
    const String type = item->getValueTree()["componentType"];
    const String value = item->getValueTree()["value"];
    const String attributes = item->getValueTree()["attributes"];
    const String ownerName = item->getValueTree()["ownerName"];
    
    SharedResourcePointer<WebAudioDictionary> dict;
    auto enumDescriptor = dict->findEnumWithName (type);
    
    PropertyItemComponent* comp = nullptr;
    
    if (item->getValueTree().getType() == Identifier ("HEADER")
        || type.isEmpty())
    {
        return nullptr;
    }
    else if (enumDescriptor.isValid())
    {
        comp = new PropertyItemListComponent (*this, *item, name, enumDescriptor.choices, value);
    }
    else if (type == ComponentTypes::colourType)
    {
        return new PropertyItemColourComponent (*this, *item, name, value, attributes.contains ("multiple"));
    }
    else
    {
        auto pic = new PropertyItemTextComponent (*this, *item, name, value, attributes);
        
        if (type == ComponentTypes::elementName)
        {
            pic->getEditor().setInputRestrictions (0, JsCodeHelpers::getAllowedCharactersForIdentifier());
            pic->setConstrainer (new ElementNameConstrainer (findElementWithName (ownerName)));
        }
        if (type == ComponentTypes::ulongType || type == ComponentTypes::uintType)
        {
            pic->getEditor().setInputRestrictions (0, "0123456789");
            pic->setConstrainer (new LongValueConstrainer());
        }
        else if (type == ComponentTypes::intType)
        {
            pic->getEditor().setInputRestrictions (0, "0123456789-");
            pic->setConstrainer (new IntValueConstrainer());
        }
        else if (type == ComponentTypes::floatType || type == ComponentTypes::doubleType)
        {
            pic->getEditor().setInputRestrictions (0, "0123456789-.");
            pic->setConstrainer (new DoubleValueConstrainer());
        }
        
        comp = pic;
    }
    
    if (comp != nullptr)
    {
        /*
        comp->addListener (this);
        comps.add (comp);
         */
    }
    
    return comp;
}

#include "Project.h"
WebAudioInspectableElement* WebAudioInspector::CustomPropertyTree::findElementWithName (String name)
{
    auto graphPanel = owner.getProject().findStaticPanelWithClass<WebAudioGraphPanel>();
    
    if (graphPanel == nullptr)
        return nullptr;
    
    for (auto emb : graphPanel->getGraphPanel().getEmbeddedComponents())
        if (auto e = dynamic_cast<WebAudioInspectableElement*> (emb))
            if (e->getUniqueName() == name)
                return e;
    
    return nullptr;
}






//==============================================================================
WebAudioInspector::WebAudioInspector (PanelManager* manager, Project& p) : InspectorPanel (manager, new CustomPropertyTree (*this), true), commonPropertiesValueTree ("ROOT"), project (p)
{
    commonPropertiesValueTree.addListener (this);
}

UndoManager* WebAudioInspector::getUndoManager() const
{
    if (inspectedElements.size() > 1)
        return nullptr;
    
    if (auto graphPanel = project.findStaticPanelWithClass<WebAudioGraphPanel>())
        return &graphPanel->getUndoManager();
    
    return nullptr;
}

void WebAudioInspector::refreshAsync()
{
    triggerAsyncUpdate();
}

void WebAudioInspector::handleAsyncUpdate()
{
    populatePropertyPanel();
}

void WebAudioInspector::valueTreePropertyChanged (ValueTree &tree, const Identifier &property)
{
    if (property != Identifier("value"))
        return;
    
    if (auto cp = findCommonPropertyWithValueTree (tree))
    {
        UndoManager* um = nullptr;
        
        // Can't use getUndoManager() here because it'd return nullptr
        if (auto graphPanel = project.findStaticPanelWithClass<WebAudioGraphPanel>())
            um = &graphPanel->getUndoManager();
        
        if (um != nullptr)
            um->beginNewTransaction();
        
        for (auto t : cp->linkedTrees)
        {
            auto v = tree.getProperty (property);
            t.setProperty (property, v, um);
        }
    }
    else    // The tree changed should be a child of one of the inspected elems props
    {
        // Hard refresh...
        populatePropertyPanel();
    }
}

WebAudioInspector::CommonProperty* WebAudioInspector::findCommonPropertyWithName (String name)
{
    for (auto cp : commonProperties)
        if (cp->possibleNames.contains (name))
            return cp;
    
    return nullptr;
}

WebAudioInspector::CommonProperty* WebAudioInspector::findCommonPropertyWithValueTree (ValueTree& vt)
{
    for (auto cp : commonProperties)
        if (cp->tree == vt)
            return cp;
    
    return nullptr;
}

WebAudioInspector::CommonProperty* WebAudioInspector::createCommonPropertyWithPossibleNames (String name1, String name2)
{
    auto cp = new CommonProperty();
    cp->possibleNames.add (name1);
    
    if (name2 != String())
        cp->possibleNames.add (name2);
    
    return cp;
}

void WebAudioInspector::initCommonProperties()
{
    commonProperties.clear();
    
    // Not used for now as we need to define a proper behavior when only messages are selected
    // and when object with distinct interfaces are selected
    //commonProperties.add (createCommonPropertyWithPossibleNames ("name", "message"));
    commonProperties.add (createCommonPropertyWithPossibleNames ("x"));
    commonProperties.add (createCommonPropertyWithPossibleNames ("y"));
    commonProperties.add (createCommonPropertyWithPossibleNames ("width"));
    commonProperties.add (createCommonPropertyWithPossibleNames ("height"));
    commonProperties.add (createCommonPropertyWithPossibleNames ("colour"));
}

void WebAudioInspector::showCommonProperties()
{
    initCommonProperties();
    
    watchedProperties.clear();
    
    // Scan properties and find common ones
    for (int i = 0; i < inspectedElements.size(); ++i)
    {
        auto elem = inspectedElements.getUnchecked (i);
        const auto& props = elem->getInspectableProperties();
        
        watchedProperties.add (props);
        
        for (int j = 0; j < props.getNumChildren(); ++j)
        {
            auto propTree = props.getChild (j);
            
            if (propTree == ValueTree())
                continue;
            
            const String name (propTree["name"].toString());
            const String type (propTree["componentType"].toString());
            const String value (propTree["value"].toString());
            const String attributes (propTree["attributes"].toString());
            //const var ownerName (propTree["ownerName"]);
            
            if (auto cp = findCommonPropertyWithName (name))
            {
                if (cp->type == String())
                    cp->type = type;
                
                if (cp->attributes == String())
                    cp->attributes = attributes;
                
                cp->values.add (value);
                
                cp->linkedTrees.add (propTree);
            }
        }
    }
    
    // Check if property values are identical
    for (auto cp : commonProperties)
    {
        cp->hasMultipleValues = false;
        
        String previousValue;
        
        bool firstValue = true;
        
        for (auto v : cp->values)
        {
            if (firstValue)
            {
                previousValue = v;
                firstValue = false;
            }
            else if (previousValue != v)
            {
                cp->hasMultipleValues = true;
                break;
            }
        }
    }
    
    // Add listeners for inspectable properties
    for (auto vt : watchedProperties)
        vt.addListener (this);
    
    // Build common properties tree
    ValueTree elemsVt ("HEADER");
    
    elemsVt.setProperty("name", String(inspectedElements.size()) + " elements selected", nullptr);
    commonPropertiesValueTree.removeAllChildren  (nullptr);
    commonPropertiesValueTree.addChild (elemsVt, 1, nullptr);
    
    int index = 0;
    
    for (auto cp : commonProperties)
    {
        const String v (cp->hasMultipleValues ? "" : cp->values[0]);
        
        String attributes = cp->attributes;
        
        if (cp->hasMultipleValues)
            attributes += " multiple";
        
        ValueTree newTree (propertyTree->createTree ("PROPERTY", cp->possibleNames[0], cp->type, v, attributes));
        
        elemsVt.addChild (newTree, ++index, nullptr);
        
        cp->tree = newTree;
        
        newTree.addListener (this);
    }
    
    propertyTree->loadValueTree (commonPropertiesValueTree, 2);
    propertyTree->repaint();
}
