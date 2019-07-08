/*
  ==============================================================================

    WebAudioInspectableElement.cpp
    Created: 24 Aug 2018 2:44:49am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/GraphElements/WebAudioInspectableElement.h"

#include "../Source/WebAudio/WebAudioGraph/WebAudioGraph.h"
WebAudioInspectableElement::WebAudioInspectableElement (WebAudioGraphPanel &parent, Descriptor descr, GraphEmbeddedComponent* c) : InspectableElement (ValueTree ("InspectableProperties"), *parent.getInspectorPropertyTree()), parentPanel (parent), privateDescriptor (descr), embeddedComponent (c)
{
}

WebAudioGraph& WebAudioInspectableElement::getWebAudioGraph()
{
    return parentPanel.getGraphPanel();
}

void WebAudioInspectableElement::setPropertyValue (String key, String value, bool undoable, ValueTree::Listener* exclude)
{
    UndoManager* um = undoable ? &parentPanel.getUndoManager() : nullptr;
    
    WebAudioInspectableElement::setPropertyValue (inspectableProperties, key, value, um, exclude);
    
    if (key == "numberOfInputs")
        setNumInputs (value.getIntValue());
    
    if (key == "numberOfOutputs")
        setNumOutputs (value.getIntValue());
}

ValueTree WebAudioInspectableElement::getPropertyValueTree()
{
    ValueTree props ("HEADER");
    props.setProperty ("name", "Attributes", nullptr);
    props.setProperty ("OPEN", true, nullptr);
    
    auto &descriptor = instance ? instance->getInterfaceDescriptor() : privateDescriptor;
    
    int index = 0;
    addPropertyRecursive (descriptor, props, index);
    
    return props;
}

void WebAudioInspectableElement::setPropertyValue (ValueTree container, String key, String value, UndoManager* um, ValueTree::Listener* exclude)
{
    ValueTree vt (findPropertyWithName (container, key));
    
    if (vt != ValueTree())
        vt.setPropertyExcludingListener (exclude, "value", value, um);
}

ValueTree WebAudioInspectableElement::findPropertyWithName (ValueTree container, String name)
{
    const String containerName = container["name"];
    const String containerType (container.getType().toString());
    
    if (name.contains ("."))
    {
        StringArray tokens;
        tokens.addTokens (name, ".", "\"");
        
        const String childName = tokens[0];
        const String attributeName = name.replaceSection(0, childName.length() + 1, "");
        
        for (int i = 0; i < container.getNumChildren(); ++i)
        {
            auto c = container.getChild(i);
            const String propName = c.getProperty ("name");
            
            if (propName.startsWith (childName))
            {
                ValueTree vt = findPropertyWithName(c, attributeName);
                
                if (vt != ValueTree())
                    return vt;
            }
        }
    }
    
    if (containerName == name && containerType != "HEADER")
        return container;

    if (containerName.startsWith (name) && containerName.contains ("(AudioParam)"))
        return findPropertyWithName (container, "value");
    
    for (int i = 0; i < container.getNumChildren(); ++i)
    {
        ValueTree vt = findPropertyWithName (container.getChild(i), name);
        
        if (vt != ValueTree())
            return vt;
    }
    
    return ValueTree();
}

ValueTree WebAudioInspectableElement::createEditableProperty (String name, String type, String defaultValue, String attributes)
{
    ValueTree property ("PROPERTY");
    property.setProperty ("name", name, nullptr);
    property.setProperty ("componentType", type, nullptr);
    property.setProperty ("value", defaultValue, nullptr);
    property.setProperty ("attributes", attributes, nullptr);
    property.setProperty ("ownerName", getUniqueName(), nullptr);
    
    return property;
}

void WebAudioInspectableElement::addPropertyRecursive (const Descriptor& param, ValueTree container, int& index)
{
    if (param.isUndefined())
        return;
    
    SharedResourcePointer<WebAudioDictionary> dict;
    const Descriptor interfaceDescriptor (dict->findDescriptorForInterface (param.interf.name));
    
    const String concatName (param.name + " (" + param.interf.name + ")");
    const String defaultValue (param.defaultValue);
    const String attributes (param.attributes);
    
    const bool useEditor = param.isPrimitive() || interfaceDescriptor.isUndefined() || param.attributes.contains ("reference");
    const bool useLongName = ! useEditor && param.name != param.interf.name;
    
    ValueTree vt = createEditableProperty (useLongName ? concatName : param.name, useEditor ? "text" : "", defaultValue, attributes);
    container.addChild (vt, ++index, nullptr);
    
    const String overridenValue = param.interf.overriden.getValue (param.name, "NULL");
    
    if (overridenValue != "NULL")
        vt.setProperty ("value", overridenValue, nullptr);
    
    int paramIndex = -1;
    
    for (auto& subParam : interfaceDescriptor.interf.properties)
        addPropertyRecursive (subParam, vt, ++paramIndex);
    
    container.addChild (vt, ++index, nullptr);
    
    if (param.interf.inheritance != String())
    {
        auto inherited = dict->findDescriptorForInterface (param.interf.inheritance);
        addPropertyRecursive (inherited, container, ++index);
        
        for (int i = 0; i < param.interf.overriden.size(); ++i)
        {
            const String key (param.interf.overriden.getAllKeys()[i]);
            const String value (param.interf.overriden.getAllValues()[i]);
            
            setPropertyValue (container, key, value);
            
            if (key == "numberOfInputs")
                setNumInputs (value.getIntValue());
            
            if (key == "numberOfOutputs")
                setNumOutputs (value.getIntValue());
        }
    }
}

#include "../Source/WebAudio/Helpers/PropertyComponentTypes.h"
#include "../Source/Layout/Graph/GraphEmbeddedComponent.h"
#include "../Source/WebAudio/Helpers/WebAudioGraphElementTypeNames.h"
void WebAudioInspectableElement::prepareInspectablePropertiesTree (String typeName)
{
    // Inspectable properties
    inspectableProperties = ValueTree ("HEADER");
    inspectableProperties.setProperty ("name", getNameInInspector(), nullptr);
    inspectableProperties.setProperty ("OPEN", true, nullptr);
    
    const String x (embeddedComponent->getX());
    const String y (embeddedComponent->getY());
    const String w (embeddedComponent->getWidth());
    const String h (embeddedComponent->getHeight());
    const String t (GraphElementType::getTypeDisplayString (typeName));
    
    auto nameProp = createNameProperty();
    nameLabel = nameProp.getProperty ("name");
    
    inspectableProperties.addChild (nameProp, 0, nullptr);
    inspectableProperties.addChild (createEditableProperty ("elementType", ComponentTypes::textType, t), 1, nullptr);
    inspectableProperties.addChild (createEditableProperty ("x",        ComponentTypes::uintType, x), 2, nullptr);
    inspectableProperties.addChild (createEditableProperty ("y",        ComponentTypes::uintType, y), 3, nullptr);
    inspectableProperties.addChild (createEditableProperty ("width",    ComponentTypes::uintType, w), 4, nullptr);
    inspectableProperties.addChild (createEditableProperty ("height",   ComponentTypes::uintType, h), 5, nullptr);
    
    if (getInterfaceName().isNotEmpty())
        inspectableProperties.addChild (createEditableProperty ("interface", ComponentTypes::identifierType, getInterfaceName(), "readonly"),
                                        6, nullptr);
    
    inspectableProperties.addChild (createEditableProperty ("colour",   ComponentTypes::colourType, getElementColour().toString()), 7, nullptr);
    
    auto properties = getPropertyValueTree();
    
    if (properties.getNumChildren() > 0 && getInterfaceName().isNotEmpty())
        inspectableProperties.addChild (properties, 8, nullptr);
    
    inspectableProperties.addListener (this);
}

ValueTree WebAudioInspectableElement::createNameProperty()
{
    return createEditableProperty ("name",  ComponentTypes::elementName, "", embeddedComponent->canBeRenamed() ? "" : "readonly");
}

#include "../Source/WebAudio/Inspector/WebAudioInspector.h"
#include "../Source/Project/Project.h"
void WebAudioInspectableElement::setPositionProperties()
{
    // Exclude this from the ValueTree listeners so that the property doesn't get applied
    setPropertyValue ("x", String (embeddedComponent->getX()), false, this);
    setPropertyValue ("y", String (embeddedComponent->getY()), false, this);
    
    if (auto inspectorPanel = parentPanel.getProject().findStaticPanelWithClass<WebAudioInspector>())
        if (inspectorPanel->showsMultipleItems())
            inspectorPanel->refreshAsync();
}

void WebAudioInspectableElement::setSizeProperties()
{
    // Exclude this from the ValueTree listeners so that the property doesn't get applied
    setPropertyValue ("width", String (embeddedComponent->getWidth()), false, this);
    setPropertyValue ("height", String (embeddedComponent->getHeight()), false, this);
    
    if (auto inspectorPanel = parentPanel.getProject().findStaticPanelWithClass<WebAudioInspector>())
        if (inspectorPanel->showsMultipleItems())
            inspectorPanel->refreshAsync();
}

WebAudioNodeInstance* WebAudioInspectableElement::getInstance() const
{
    return instance;
}

void WebAudioInspectableElement::setInstance (WebAudioNodeInstance* newInstance)
{
    if (newInstance == instance)
        return;
    
    instance = newInstance;
    
    refreshOptionsTree();
}

Array<Descriptor::Method> WebAudioInspectableElement::getAllMethods() const
{
    SharedResourcePointer<WebAudioDictionary> dict;
    Array<Descriptor::Method> array (privateDescriptor.interf.methods);
    
    String inheritance (privateDescriptor.interf.inheritance);
    
    while (inheritance.isNotEmpty())
    {
        auto descr = dict->findDescriptorForInterface (inheritance);
        
        if (descr.isUndefined())
            break;
        
        array.addArray (descr.interf.methods);
        
        inheritance = descr.interf.inheritance;
    }
    
    return array;
}

//==============================================================================
class UndoableBoundsSetter : public UndoableAction
{
public:
    UndoableBoundsSetter (WebAudioEmbedded* ref, juce::Rectangle<int> bounds, juce::Rectangle<int> oldBounds) : persistantRef (ref), newBounds (bounds), oldBounds (oldBounds), shouldActuallyPerform (false) {}
    
    virtual ~UndoableBoundsSetter() {}
    
    bool perform() override
    {
        setBounds (newBounds);
        
        return true;
    }
    
    bool undo() override
    {
        setBounds (oldBounds);

        return true;
    }
    
private:
    void setBounds (juce::Rectangle<int> newBounds)
    {
        // When the first call is made, the command has already happened...
        if (shouldActuallyPerform)
            persistantRef->setBounds (newBounds);
        
        shouldActuallyPerform = true;
        
        if (auto webEmb = dynamic_cast<WebAudioEmbedded*>(persistantRef.get()))
        {
            webEmb->checkContainers (false);
            webEmb->navigableChanged();
            webEmb->setPositionProperties();
            webEmb->setSizeProperties();
        }
        
        if (auto cont = dynamic_cast<WebAudioContainer*>(persistantRef.get()))
            cont->getWebAudioGraph().containerMoved();
        
        if (auto graph = persistantRef->getParentGraph())
            graph->adaptSizeToContent();
    }
    
    WebAudioEmbedded::Ptr persistantRef;
	juce::Rectangle<int> newBounds;
	juce::Rectangle<int> oldBounds;
    bool shouldActuallyPerform;
};

//==============================================================================
WebAudioEmbedded::CustomResizableBorder::CustomResizableBorder (WebAudioEmbedded& o, ComponentBoundsConstrainer *constrainer) : ResizableBorderComponent (&o, constrainer), owner (o)
{
}

void WebAudioEmbedded::CustomResizableBorder::mouseDown (const MouseEvent& e)
{
    ResizableBorderComponent::mouseDown (e);
    
    previousBounds = owner.getBounds();
    
    auto &undoManager = owner.parentPanel.getUndoManager();
    undoManager.beginNewTransaction();
}

void WebAudioEmbedded::CustomResizableBorder::mouseUp (const MouseEvent& e)
{
    ResizableBorderComponent::mouseUp (e);
    
    if (previousBounds != owner.getBounds())
    {
        auto &undoManager = owner.parentPanel.getUndoManager();
        auto undoable = new UndoableBoundsSetter (&owner, owner.getBounds(), previousBounds);
        undoManager.perform (undoable);
    }
    
    previousBounds = owner.getBounds();
    
    owner.wasResizedWithBorder();
}

void WebAudioEmbedded::CustomResizableBorder::mouseDrag (const MouseEvent& e)
{
    ResizableBorderComponent::mouseDrag (e);
    
    owner.setSizeProperties();
    owner.setPositionProperties();
}

//==============================================================================
WebAudioEmbedded::WebAudioEmbedded (WebAudioGraphPanel& panel, Descriptor descr) : WebAudioInspectableElement (panel, descr, this)
{
    if (getAllMethods().size() > 0)
        addPins (Pin::PinOnRight, 1);
    
    setResizable (true);
    
    constrainer.setMinimumSize (GraphEmbeddedComponent::getDefaultWidth(),
                                GraphEmbeddedComponent::getDefaultHeight());
}

WebAudioInstanceManager& WebAudioEmbedded::getInstanceManager()
{
    return parentPanel.getInstanceManager();
}

BorderSize<int> WebAudioEmbedded::getBorderThickness() const
{
    return BorderSize<int>(4);
}

void WebAudioEmbedded::setResizable (bool shouldBeResizable)
{
    if (shouldBeResizable == isResizable())
        return;
    
    if (shouldBeResizable)
    {
        resizableBorder = new CustomResizableBorder (*this, &constrainer);
        resizableBorder->setBorderThickness (getBorderThickness());
        addAndMakeVisible (resizableBorder);
    }
    else
    {
        removeChildComponent (resizableBorder);
        resizableBorder = nullptr;
    }
}

bool WebAudioEmbedded::isResizable() const
{
    return resizableBorder != nullptr;
}

void WebAudioEmbedded::mouseDown (const MouseEvent &e)
{
    GraphEmbeddedComponent::mouseDown (e);
    //parentPanel.getUndoManager().beginNewTransaction();
}

void WebAudioEmbedded::resized()
{
    GraphEmbeddedComponent::resized();
    
    if (resizableBorder != nullptr)
    {
        resizableBorder->setSize (getWidth(), getHeight());
        resizableBorder->toBack();
    }
}

class UndoablePositionSetter : public UndoableAction
{
public:
    UndoablePositionSetter (WebAudioEmbedded::Ptr ref, Point<int> pos, Point<int> startPos) : persistantRef (ref), newPos (pos), oldPos (startPos) {}
    
    virtual ~UndoablePositionSetter() {}
    
    bool perform() override
    {
        moveTo (newPos);
        
        return true;
    }
    
    bool undo() override
    {
        moveTo (oldPos);
        
        return true;
    }
    
private:
    void moveTo (Point<int> newPosition)
    {
        if (shouldReallyPerform)
            persistantRef->setTopLeftPosition (newPosition);
        
        shouldReallyPerform = true;
        
        if (auto webEmb = dynamic_cast<WebAudioEmbedded*>(persistantRef.get()))
        {
            webEmb->checkContainers (false);
            webEmb->navigableChanged();
            webEmb->setPositionProperties();
        }
            
        if (auto cont = dynamic_cast<WebAudioContainer*>(persistantRef.get()))
            cont->getWebAudioGraph().containerMoved();
        
        if (auto graph = persistantRef->getParentGraph())
            graph->adaptSizeToContent();
    }
    
    WebAudioEmbedded::Ptr persistantRef;
    Point<int> newPos;
    Point<int> oldPos;
    
    bool shouldReallyPerform = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (UndoablePositionSetter)
};

void WebAudioEmbedded::mouseUp (const MouseEvent &e)
{
    GraphEmbeddedComponent::mouseUp (e);
    
    if (e.mouseWasClicked() && e.getNumberOfClicks() == 2)
        showNameEditor();
    
    for (auto cont : parentPanel.getContainers())
        if (cont->getContent().contains (this))
            cont->setHighlighted (false);
}

void WebAudioEmbedded::wasDragged (const MouseEvent &e, bool dragIsOver, bool isUniqueSelection)
{
    if (dragIsOver)
    {
        auto& undoManager = parentPanel.getUndoManager();
        auto undoable = new UndoablePositionSetter (this, getPosition(),
                                                    getPositionOnLastMouseDown());
        undoManager.perform (undoable);
        
        return;
    }
    
    if (isUniqueSelection)
        checkContainers (true);
}

Array<WeakReference<NavigationPanel::Navigable>> WebAudioEmbedded::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    return result;
}

StringPairArray WebAudioEmbedded::getNavigationInfo() const
{
    StringPairArray info;
    info.set ("name", getPublicName());
    info.set ("type", getUICompTypeName());
    info.set ("category", "WEBAUDIOGRAPH");
    info.set ("selected", GraphSelectableItem::isSelected() ? "TRUE" : "FALSE");
    
    return info;
}

void WebAudioEmbedded::reveal (ModifierKeys mods)
{
    parentPanel.revealComponent (this, mods);
}

void WebAudioEmbedded::inspectablePropertyChanged (ValueTree &tree, const Identifier &property)
{
    if (tree.getType() == Identifier("PROPERTY"))
    {
        const auto propName = tree["name"];
        const auto value = tree["value"];
        
        if (propName == nameLabel)      setPublicName (getValidName (value));
        else if (propName == "x")       setTopLeftPosition (value, getY());
        else if (propName == "y")       setTopLeftPosition (getX(), value);
        else if (propName == "width")
        {
            const auto constrained = jlimit (constrainer.getMinimumWidth(),
                                             constrainer.getMaximumWidth(),
                                             int(value));
            
            if (constrained != int(value))
            {
                parentPanel.getUndoManager().undoCurrentTransactionOnly();
                return setPropertyValue ("width", String(constrained), false);
            }
            
            setSize (value, getHeight());
        }
        else if (propName == "height")
        {
            const auto constrained = jlimit (constrainer.getMinimumHeight(),
                                             constrainer.getMaximumHeight(),
                                             int(value));
            
            if (constrained != int(value))
            {
                parentPanel.getUndoManager().undoCurrentTransactionOnly();
                return setPropertyValue ("height", String(constrained), false);
            }
            
            setSize (getWidth(), value);
        }
        else if (propName == "colour")
        {
            setBackgroundColour (Colour::fromString(value.toString()));
            
            if (auto navPanel = getNavigationPanel())
                navPanel->repaint();
        }
        
        if (propName == "width" || propName == "height")
            manuallyResized = true;
        
        if (propName == "width" || propName == "height" || propName == "x" || propName == "y")
        {
            getWebAudioGraph().adaptSizeToContent();
            checkContainers (isUniqueSelection());
            navigableChanged();
        }
    }
}

void WebAudioEmbedded::showNameEditor()
{
    if (canBeRenamed() && label != nullptr && label->isShowing())
        label->showEditor();
}

void WebAudioEmbedded::editorShown (Label* l, TextEditor& editor)
{
    label->setInterceptsMouseClicks (true, true);
    setHighlighted (false);
}

void WebAudioEmbedded::editorHidden (Label*, TextEditor&)
{
    label->setInterceptsMouseClicks (false, false);
}

void WebAudioEmbedded::labelTextChanged (Label* labelThatHasChanged)
{
    const String newText = labelThatHasChanged->getText();
    const String validName = getValidName (newText);
    
    if (newText == String())
    {
        labelThatHasChanged->setText (getPublicName(), dontSendNotification);
        return;
    }
    
    if (newText != validName)
        labelThatHasChanged->setText (validName, dontSendNotification);
   
    parentPanel.getUndoManager().beginNewTransaction();
    setPropertyValue (nameLabel, validName, true);
}

void WebAudioEmbedded::setPublicName (String newName)
{
    GraphEmbeddedComponent::setPublicName (newName);
    
    setPropertyValue (nameLabel, newName, nullptr);
    
    if (inspectableProperties.isValid())
        inspectableProperties.setProperty ("name", getNameInInspector(), nullptr);
    
    if (label != nullptr)
        label->setText (newName, dontSendNotification);
}

void WebAudioEmbedded::checkContainers (bool useHighlight)
{
    if (getParentGraph() == nullptr)
        return;
    
    const auto centre = getBounds().getCentre();
    
    bool shouldRefreshNavigable = false;
    
    for (auto cont : parentPanel.getContainers())
    {
        const bool isAlreadyContained = cont->getContent().contains (this);
        
        if (cont->getBounds().contains (centre))
        {
            if (! isAlreadyContained)
            {
                cont->addComponent (this);
                shouldRefreshNavigable = true;
            }
            
            if (useHighlight)
            {
                cont->setHighlighted (true);
                getParentGraph()->moveToFront (cont);
            }
        }
        else
        {
            if (isAlreadyContained)
            {
                cont->removeComponent (this);
                shouldRefreshNavigable = true;
            }
            
            cont->setHighlighted (false);
        }
    }
    
    if (shouldRefreshNavigable)
        navigableChanged();
}

#include "../Source/WebAudio/Helpers/JsCodeHelpers.h"
String WebAudioEmbedded::getDefaultName() const
{
    return JsCodeHelpers::getDefaultInstanceName (privateDescriptor);
}

void WebAudioEmbedded::setBackgroundColour (Colour newColour)
{
    GraphEmbeddedComponent::setBackgroundColour (newColour);
    setPropertyValue ("colour", getBackgroundColour().toString());
    
    if (label != nullptr)
        label->setColour (Label::textColourId, newColour.contrasting());
}

String WebAudioEmbedded::getValidName (String wantedName)
{
    const String oldName = getPublicName();
    
    if (wantedName.isEmpty() && getPublicName().isNotEmpty())
        return getValidName (getPublicName());
    
    wantedName = JsCodeHelpers::removeIllegalCharactersForIdentifier (wantedName);
    
    if (JsCodeHelpers::startsWithDigit (wantedName))
        wantedName = "_" + wantedName;
    
    if (JsCodeHelpers::isReservedKeyword (wantedName))
        wantedName = wantedName + "_";
    
    if (nameIsAlreadyTaken (wantedName))
        return parentPanel.getUnusedName (wantedName);
    
    return wantedName;
}

bool WebAudioEmbedded::nameIsAlreadyTaken (String name)
{
    for (auto emb : getParentGraph()->getEmbeddedComponents())
    {
        if (emb == this)
            continue;
        
        if (emb->getPublicName() == name)
        {
            if (emb->getUICompTypeName() != getUICompTypeName())
                return true;
        
            if (auto webEmb = dynamic_cast<WebAudioEmbedded*> (emb))
            {
                if (webEmb->getInterfaceName() != getInterfaceName())
                    return true;
            }
        }
    }
    
    return false;
}

void WebAudioEmbedded::deleteSelectedItem()
{
    GraphEmbeddedComponent::deleteSelectedItem();
}

#include "../Source/WebAudio/Helpers/JsCodeHelpers.h"
void WebAudioEmbedded::initializeWithWantedName (String name, bool createNewInstance)
{
    name = name.isEmpty() ? getDefaultName() : name;
    
    name = getValidName (name);
    
    if (createNewInstance)
        name = parentPanel.getUnusedName (name);
    
    setPublicName (name);
}

#include "../Source/WebAudio/Helpers/WebAudioGraphElementTypeNames.h"
void WebAudioEmbedded::addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos)
{
    m.addSeparator();
    
    if (auto sel = getSelector())
    {
        const int num = sel->getNumSelected();
        
        bool enabled = false;
        
        for (int i = 0; i < num; ++i)
        {
            if (auto emb = dynamic_cast<GraphEmbeddedComponent*> (sel->getSelected (i)))
            {
                const auto type = emb->getUICompTypeName();
                
                if (type == GraphElementType::audioNodeType || type == GraphElementType::audioContextType
                    || type == GraphElementType::audioDataType || type == GraphElementType::scriptType)
                {
                    enabled = true;
                    break;
                }
            }
        }
        
        const String txt (num == 1 ? "Save as preset" : "Save selection as snippet");
        
        m.addItem (80, txt, enabled);
    }
}

void WebAudioEmbedded::handleExtraPopupMenuCommands (int result, Point<int> pos)
{
    if (result == 80)
        parentPanel.saveSelectionAsPreset();
}

void WebAudioEmbedded::prepareLabel()
{
    label = createLabel();
    
    addAndMakeVisible (label);
    label->setJustificationType (Justification::centred);
    label->setInterceptsMouseClicks (false, false);
    label->addListener (this);
    
    resized();
}

//==============================================================================
WebAudioFoldable::WebAudioFoldable (WebAudioGraphPanel& panel, Descriptor descr) : WebAudioEmbedded (panel, descr), optionsTree (*this)
{
    setSize (getDefaultWidth(), getDefaultHeight());
    setSizeProperties();
    
    addChildComponent (&optionsTree);
    
    optionsTree.setIndentSize (0);
    
    setConstrainerLimits();
}
void WebAudioFoldable::setOpenButtonVisible (bool shouldBeVisible)
{
    openButtonVisible = shouldBeVisible;
    resized();
}

void WebAudioFoldable::setOpen (bool shouldBeOpen)
{
    if (isCurrentlyOpen != shouldBeOpen)
    {
        isCurrentlyOpen = shouldBeOpen;
        
        optionsTree.setVisible (isCurrentlyOpen);
        //setResizable (isCurrentlyOpen);
        
        const int h = isCurrentlyOpen ? getHeightWhenOpen() : getDefaultHeight();
        int w = manuallyResized ? getWidth() : isCurrentlyOpen ? lastWidthWhenOpen : getWidthWhenClosed();
        
        if (w <= 0)
            w = getWidthWhenClosed();
        
        setConstrainerLimits();
        setSize (w, h);
    }
}

bool WebAudioFoldable::isOpen() const
{
    return isCurrentlyOpen;
}

void WebAudioFoldable::mouseEnter (const MouseEvent &e)
{
    const juce::Rectangle<int> area (0, 0, getDefaultHeight(), getDefaultHeight());
    
    isMouseOverButton = area.contains(e.getPosition());
    
    GraphSelectableItem::mouseEnter (e);
}

void WebAudioFoldable::mouseMove (const MouseEvent &e)
{
    const bool oldValue = isMouseOverButton;
    const juce::Rectangle<int> area (0, 0, getDefaultHeight(), getDefaultHeight());
    
    isMouseOverButton = area.contains(e.getPosition());
    
    if (isMouseOverButton != oldValue)
        repaint();
}

void WebAudioFoldable::mouseExit (const MouseEvent &e)
{
    isMouseOverButton = false;
    
    GraphSelectableItem::mouseExit (e);
}

class UndoableOpennessSetter : public UndoableAction
{
public:
    UndoableOpennessSetter (WebAudioFoldable* ref, bool open) : persistantRef (ref), shouldBeOpen (open) {}
    virtual ~UndoableOpennessSetter() {}
    
    bool perform() override
    {
        if (persistantRef == nullptr)
            return false;
        
        if (auto foldable = dynamic_cast<WebAudioFoldable*>(persistantRef.get()))
            foldable->setOpen (shouldBeOpen);
        
        return true;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        if (auto foldable = dynamic_cast<WebAudioFoldable*>(persistantRef.get()))
            foldable->setOpen (! shouldBeOpen);
        
        return true;
    }
    
private:
    WebAudioEmbedded::Ptr persistantRef;
    const bool shouldBeOpen;
};

void WebAudioFoldable::mouseUp (const MouseEvent &e)
{
    GraphEmbeddedComponent::mouseUp (e);
    
    const juce::Rectangle<int> expandButtonArea (0, 0, getDefaultHeight(), getDefaultHeight());
    const juce::Rectangle<int> headerArea (getLocalBounds().removeFromTop(getDefaultHeight()));
    
    if (e.mouseWasClicked() && ! e.mods.isRightButtonDown())
    {
        const auto numClicks = e.getNumberOfClicks();
        
        // Expand button clicked
        if (openButtonVisible && expandButtonArea.contains (e.getPosition()))
        {
            auto &um = parentPanel.getUndoManager();
            auto undoable = new UndoableOpennessSetter (this, ! isOpen());
            um.beginNewTransaction();
            um.perform (undoable);
        }
        // Show name editor on double click
        else if (label != nullptr && headerArea.contains (e.getPosition()) && numClicks == 2 && canBeRenamed())
            label->showEditor();
    }
    
    // Remove containers highlight
    for (auto cont : parentPanel.getContainers())
        if (cont->getContent().contains (this))
            cont->setHighlighted (false);
}

void WebAudioFoldable::paintUI (Graphics &g, juce::Rectangle<int> contentBounds)
{
    const juce::Rectangle<int> area (0, 0, getDefaultHeight(), getDefaultHeight());
    
    if (openButtonVisible)
        getLookAndFeel().drawTreeviewPlusMinusBox (g, area.reduced(3).toFloat(), getBackgroundColour(),
                                                   isCurrentlyOpen, isMouseOverButton);
    
    if (getHeight() >= getDefaultHeight())
    {
        g.setColour (getBackgroundColour().withMultipliedBrightness(0.8f));
        g.fillRect (contentBounds.withTrimmedTop (getDefaultHeight() - 3).removeFromTop (1));
    }
}

void WebAudioFoldable::resizeUI (juce::Rectangle<int> contentBounds)
{
    auto r = getLocalBounds();
    auto top = r.removeFromTop(getDefaultHeight());
    
    if (openButtonVisible)
        top = top.withTrimmedLeft (getDefaultHeight() * 0.6);
    
    if (label != nullptr)
        label->setBounds (top.reduced (3));
    
    if (! r.isEmpty())
        optionsTree.setBounds (r.reduced(4, 0));
    
    if (isCurrentlyOpen)
        lastWidthWhenOpen = getWidth();
    
    setSizeProperties();
}

void WebAudioFoldable::refreshUI()
{
    if (label != nullptr)
        label->setText (getPublicName(), NotificationType::dontSendNotification);
}

void WebAudioFoldable::refreshOptionsTree()
{
    auto instance = getInstance();
    auto rootItem = optionsTree.getRootItem();
    
    if (rootItem == nullptr || instance == nullptr)
        return;
    
    auto &options = instance->getOptions();
    int index = 0;
    
    for (auto o : options)
    {
        if (index >= rootItem->getNumSubItems())
            break;
        
        auto subItem = rootItem->getSubItem (index);
        
        if (auto prop = dynamic_cast<PropertyItem*>(subItem))
            prop->setValue (o->defaultValue);
        
        setPropertyValue (o->name, o->defaultValue);
        
        ++index;
    }
    
    optionsTree.resized();
}

void WebAudioFoldable::prepareOptionsTree()
{
    auto root = optionsTree.createTree ("HEADER", "Options");
    
    if (auto instance = getInstance())
    {
        auto &options = instance->getOptions();
        
        int index = 0;
        
        for (auto o : options)
        {
            Identifier componentType;
            
            SharedResourcePointer<WebAudioDictionary> dict;
            auto enumDescriptor = dict->findEnumWithName (o->interf.name);
            
            if (enumDescriptor.isValid())                   componentType = enumDescriptor.name;
            else if (o->interf.name == "unsigned long")  componentType = ComponentTypes::ulongType;
            else if (o->interf.name == "int")            componentType = ComponentTypes::intType;
            else if (o->interf.name == "float")          componentType = ComponentTypes::floatType;
            else if (o->interf.name == "double")         componentType = ComponentTypes::doubleType;
            else if (o->interf.name == "boolean")        componentType = ComponentTypes::boolType;
            else                                            componentType = ComponentTypes::textType;
            
            root.addChild (optionsTree.createTree ("OPTION", o->name, componentType.toString(), o->defaultValue), ++index, nullptr);
            
            setPropertyValue (o->name, o->defaultValue);
        }
        
        numOptions = options.size();
    }
    
    optionsTree.loadValueTree (root, 1);
    optionsTree.setRootItemVisible (false);
    optionsTree.setIndentSize (0);
    
    setConstrainerLimits();
}

void WebAudioFoldable::setOptionValue (String optionName, String newValue)
{
    auto instance = getInstance();
    
    if (instance == nullptr)
        return;
    
    auto& options = instance->getOptions();
    
    for (auto o : options)
    {
        if (o->name == optionName)
        {
            o->defaultValue = newValue;
            instance->optionsChanged();
            break;
        }
    }
    
    optionsTree.repaint();
}

void WebAudioFoldable::setBackgroundColour (Colour newColour)
{
    WebAudioEmbedded::setBackgroundColour (newColour);
    optionsTree.setTextColour (newColour.contrasting());
}

void WebAudioFoldable::setPublicName (String name)
{
    WebAudioEmbedded::setPublicName (name);
    
    getInstanceManager().updateReference (this);
    refreshOptionsTree();
    
    if (! isOpen())
        setSize (getWidthWhenClosed(), getDefaultHeight());
    else if (getWidth() < getWidthWhenClosed())
        setSize (getWidthWhenClosed(), getHeight());
    
    setSizeProperties();
    setConstrainerLimits();
}

void WebAudioFoldable::initializeWithWantedName (String name, bool createNewInstance)
{
    WebAudioEmbedded::initializeWithWantedName (name, createNewInstance);
    
    prepareOptionsTree();
}

void WebAudioFoldable::setConstrainerLimits()
{
    int fixedHeight = isOpen() ? getHeightWhenOpen() : getDefaultHeight();
    
    constrainer.setSizeLimits (getWidthWhenClosed(), fixedHeight,
                               maximumWidth, fixedHeight);
}

int WebAudioFoldable::getWidthWhenClosed() const
{
    if (label == nullptr)
        return getDefaultWidth();
    
    int w = label->getFont().getStringWidth (getPublicName()) + getDefaultHeight();
    return jlimit (getDefaultWidth(), maximumWidth, w);
}

int WebAudioFoldable::getHeightWhenOpen() const
{
    return getDefaultHeight() + (numOptions * optionsTree.getDefaultItemHeight()) + 10;
}

//==============================================================================
WebAudioFoldable::OptionsPropertyTree::~OptionsPropertyTree()
{
    for (auto comp : comps)
        comp->removeListener (this);
}

PropertyItemComponent* WebAudioFoldable::OptionsPropertyTree::createComponentForItem (PropertyItem* item)
{
    if (item == nullptr)
        return nullptr;
    
    const String name =  item->getValueTree()["name"];
    const String type = item->getValueTree()["componentType"];
    const String value = item->getValueTree()["value"];
    const String attributes = item->getValueTree()["attributes"];
    
    SharedResourcePointer<WebAudioDictionary> dict;
    auto enumDescriptor = dict->findEnumWithName (type);
    
    PropertyItemComponent* comp = nullptr;
    
    if (enumDescriptor.isValid())
    {
        comp = new PropertyItemListComponent (*this, *item, name, enumDescriptor.choices, value);
    }
    else if (type == ComponentTypes::boolType)
    {
        comp = new PropertyItemBooleanComponent (*this, *item, name, value.getIntValue());
    }
    else
    {
        auto pic = new PropertyItemTextComponent (*this, *item, name, value, attributes);
        
        if (type == ComponentTypes::ulongType)
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
        comp->setEnabled (item->isEnabled());
        comp->setTextValue (value, false);
        comp->addListener (this);
        comps.add (comp);
    }
    
    return comp;
}


class UndoableOptionSetter : public UndoableAction
{
public:
    UndoableOptionSetter (WebAudioFoldable* ref, PropertyItemComponent* comp, String v, String old) : persistantRef (ref), itemComp (comp), newValue (v), oldValue (old) {}
    virtual ~UndoableOptionSetter() {}
    
    bool perform() override
    {
        if (persistantRef == nullptr)
            return false;
        
        if (auto foldable = dynamic_cast<WebAudioFoldable*>(persistantRef.get()))
            foldable->setOptionValue (itemComp->getPropertyName(), newValue);
        
        return true;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        if (auto foldable = dynamic_cast<WebAudioFoldable*>(persistantRef.get()))
            foldable->setOptionValue (itemComp->getPropertyName(), oldValue);
        
        return true;
    }
    
private:
    WebAudioEmbedded::Ptr persistantRef;
    PropertyItemComponent* itemComp;
    const String newValue;
    const String oldValue;
};


void WebAudioFoldable::OptionsPropertyTree::propertyItemValueChanged (PropertyItemComponent* comp, String newValue, String oldValue)
{
    auto &um = owner.parentPanel.getUndoManager();
    um.beginNewTransaction();
    um.perform (new UndoableOptionSetter (&owner, comp, newValue, oldValue));
}

//==============================================================================

WebAudioContainer::WebAudioContainer (WebAudioGraphPanel &parent, Descriptor descr) : WebAudioEmbedded (parent, descr)
{
    setResizable (true);
    constrainer.setMinimumSize (GraphEmbeddedComponent::getDefaultWidth(), GraphEmbeddedComponent::getDefaultHeight());
}

Array<WeakReference<NavigationPanel::Navigable>> WebAudioContainer::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    
    for (auto emb : getContent())
        if (auto nav = dynamic_cast<NavigationPanel::Navigable*> (emb))
            result.add (nav);
    
    return result;
}

void WebAudioContainer::addComponent (GraphEmbeddedComponent* compToAdd)
{
    components.addIfNotAlreadyThere (compToAdd);
}

void WebAudioContainer::removeComponent (GraphEmbeddedComponent* compToRemove)
{
    components.removeObject (compToRemove);
}

void WebAudioContainer::checkContent()
{
    if (auto graph = getParentGraph())
        for (int i = components.size(); --i >= 0;)
            if (! graph->getEmbeddedComponents().contains (components.getUnchecked(i)))
                components.remove (i);
    
    navigableChanged();
}

const ReferenceCountedArray<GraphEmbeddedComponent>& WebAudioContainer::getContent() const
{
    return components;
}

#include "../Source/Layout/Graph/GraphDragger.h"
void WebAudioContainer::mouseDown (const MouseEvent& e)
{
    GraphSelectableItem::mouseDown(e);
    
    const juce::Rectangle<int> draggableZone (getLocalBounds().removeFromTop (22));
    
    isBeingDragged = false;
    
    if (e.mods.isLeftButtonDown() && draggableZone.contains (e.getPosition())
        && getSelector() != nullptr && getSelector()->getDragger() != nullptr)
    {
        shouldPrepareDrag = true;
        getParentGraph()->moveToFront (this);
    }
    
    if (! shouldPrepareDrag && ! isBeingDragged && e.mods.isLeftButtonDown())
        getParentGraph()->startLasso (e.getEventRelativeTo (getParentGraph()));
}

void WebAudioContainer::mouseUp (const MouseEvent &e)
{
    GraphEmbeddedComponent::mouseUp (e);
    
    getParentGraph()->endLasso();
    isBeingDragged = false;
    
    if (e.mouseWasClicked() && e.getNumberOfClicks() == 2
        && getLocalBounds().removeFromTop (22).contains (e.getPosition()))
        showNameEditor();
    
    if (e.mouseWasDraggedSinceMouseDown())
        getWebAudioGraph().containerMoved();
}

void WebAudioContainer::mouseDrag (const MouseEvent &e)
{
    if (! e.mouseWasDraggedSinceMouseDown())
        return;
    
    if (shouldPrepareDrag)
    {
        parentPanel.getUndoManager().beginNewTransaction();
        
        Array<GraphSelectableItem*> previousSelection (getSelector()->getSelection());
        
        bool selectionWasChanged = false;
        
        // Add contained components of other containers in selection
        for (auto elem : previousSelection)
        {
            if (auto cont = dynamic_cast<WebAudioContainer*> (elem))
            {
                bool addContent = true;
                
                for (auto comp : cont->getContent())
                {
                    if (previousSelection.contains (comp))
                    {
                        addContent = false;
                        selectionWasChanged = false;
                        break;
                    }
                }
                
                if (addContent)
                    for (auto emb : cont->getContent())
                        getSelector()->addToSelection (emb);
                
                if (addContent)
                    selectionWasChanged = true;
            }
        }
        
        // Start dragging with this bunch of components
        getSelector()->getDragger()->startDragging (e);
        isBeingDragged = true;
        
        // And restore previous selection
        if (selectionWasChanged)
            getSelector()->selectArray (previousSelection);
        
        shouldPrepareDrag = false;
    }
    
    if (! isBeingDragged)
        getParentGraph()->dragLasso (e.getEventRelativeTo (getParentGraph()));
    else
    {
        GraphEmbeddedComponent::mouseDrag (e);
        //getWebAudioGraph().containerMoved (false);
    }
}

void WebAudioContainer::mouseMove (const MouseEvent &e)
{
    const juce::Rectangle<int> draggableZone (getLocalBounds().removeFromTop (22));
    
    if (draggableZone.contains (e.getPosition()))
        setMouseCursor (MouseCursor::DraggingHandCursor);
    else
        setMouseCursor (MouseCursor::NormalCursor);
}

void WebAudioContainer::resizeUI (juce::Rectangle<int> contentBounds)
{
    auto r = getLocalBounds();
    auto top = r.removeFromTop (GraphEmbeddedComponent::getDefaultHeight());
    
    if (label != nullptr)
        label->setBounds (top.reduced (3));
}

void WebAudioContainer::wasDragged (const MouseEvent &e, bool dragIsOver, bool isUniqueSelection)
{
    WebAudioEmbedded::wasDragged (e, dragIsOver, isUniqueSelection);
    
    if (dragIsOver)
        getWebAudioGraph().containerMoved();
}

void WebAudioContainer::inspectablePropertyChanged (ValueTree &tree, const Identifier &property)
{
    WebAudioEmbedded::inspectablePropertyChanged (tree, property);
    
    if (tree.getType() == Identifier("PROPERTY"))
    {
        const auto propName = tree["name"];
        
        if (propName == "width" || propName == "height" || propName == "x" || propName == "y")
            getWebAudioGraph().containerMoved();
    }
    
}
