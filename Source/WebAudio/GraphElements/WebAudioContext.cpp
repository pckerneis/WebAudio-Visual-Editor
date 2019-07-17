/*
  ==============================================================================

    WebAudioContext.cpp
    Created: 17 Aug 2018 3:17:37pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/GraphElements/WebAudioContext.h"

#include "../Source/Project/Project.h"
#include "../Source/WebAudio/WebAudioGraph/WebAudioGraph.h"

#include "../Source/WebAudio/Helpers/JsCodeHelpers.h"
WebAudioContext::WebAudioContext (Project& proj, WebAudioGraphPanel* parent, Descriptor& descr)
:  WebAudioContainer (*parent, descr), project (proj)
{
    setSize (getDefaultWidth(), getDefaultHeight());
    setSizeProperties();
    setZ (0);
    
    prepareLabel();
    label->setJustificationType (Justification::left);
    
    setParentGraph (&parent->getGraphPanel());
    
    //prepareInspectablePropertiesTree (getUICompTypeName());
    
    setNumPins (1, Pin::Placement::PinOnRight);
}

#include "../Source/WebAudio/GraphElements/WebAudioNode.h"
WebAudioContext::~WebAudioContext()
{
    for (auto node : getAllNodes())
        if (node != nullptr)
            node->checkConnectionsValidity();
    
    getInstanceManager().removeReference (this);
    
    masterReference.clear();
}

void WebAudioContext::paint (Graphics& g)
{
    const auto s = GraphEmbeddedComponent::isSelected();
    const float stroke = s ? 1.6 : 1.0;
    const Colour bgColour = getBackgroundColour().withMultipliedAlpha (s ? 0.9f : 0.7f);
    
    const Colour strokeColour = s ? AppSettings::getCurrentMainColour()
                                : bgColour.interpolatedWith (Colours::black, 0.5f);
    
    auto r = getLocalBounds().reduced (getBorderThickness().getLeft());
    
    g.setColour (bgColour);
    g.fillRect (r);
    
    g.setColour (strokeColour);
    g.drawRect (getLocalBounds().reduced (4), stroke);
    
    paintUI (g, getLocalBounds().reduced (6));
}

#include "../Source/Application/CommandIDs.h"
void WebAudioContext::addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos)
{
    WebAudioEmbedded::addExtraPopupMenuCommands (m, pos);
    
    m.addSeparator();
    
    PopupMenu addMenu;
    addMenu.addItem (100, "Add destination node");
    
    using Category = WebAudioGraphPanel::ElementCategory;
    auto flags = Category::nodeCategory | Category::dataCategory | Category::messageCategory | Category::dynamicRouteCategory;
    
    parentPanel.getAddMenuItems (addMenu, flags);
    
    m.addSubMenu ("Add...", addMenu);
}

void WebAudioContext::handleExtraPopupMenuCommands (int result, Point<int> pos)
{
    WebAudioEmbedded::handleExtraPopupMenuCommands (result, pos);
    
    pos += getPosition();
    
    if (result == 0)
        return;
    else if (result == 100)
        parentPanel.createAndAddUndoable (nodeDictionary->findDescriptorForInterface ("AudioDestinationNode"), pos, "", this);
    else
        parentPanel.modalStateFinished (result, pos, this);
}

Array<WeakReference<NavigationPanel::Navigable>> WebAudioContext::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    
    for (auto n : getAllNodes (true))
        result.add (n);
    
    return result;
}

Array<WebAudioNode*> WebAudioContext::getAllNodes (bool sorted) const
{
    Array<WebAudioNode*> nodes;
    
    for (auto c : getContent())
        if (auto node = dynamic_cast<WebAudioNode*>(c))
            nodes.add (node);
    
    if (sorted)
    {
        WebAudioGraphPanel::ComponentPositionComparator comparator;
        nodes.sort (comparator);
    }
    
    return nodes;
}

#include "../Source/WebAudio/Helpers/JsCodeHelpers.h"
#include "../Source/WebAudio/GraphElements/WebAudioDynamicRoute.h"
String WebAudioContext::getValidName (String newName)
{
    newName = JsCodeHelpers::removeIllegalCharactersForIdentifier (newName);
    
    if (JsCodeHelpers::startsWithDigit (newName))
        newName = "_" + newName;
    
    if (JsCodeHelpers::isReservedKeyword (newName))
        newName = newName + "_";
    
    bool nameIsAlreadyTaken = false;
    
    for (auto emb : getParentGraph()->getEmbeddedComponents())
    {
        if (auto ctx = dynamic_cast<WebAudioContext*>(emb))
        {
            if (ctx->getPublicName() == newName && ctx != this && ctx->getInterfaceName() != getInterfaceName())
            {
                nameIsAlreadyTaken = true;
                break;
            }
        }
        else if (auto node = dynamic_cast<WebAudioNode*>(emb))
        {
            if (node->getPublicName() == newName)
            {
                nameIsAlreadyTaken = true;
                break;
            }
        }
        else if (auto func = dynamic_cast<WebAudioDynamicRoute*>(emb))
        {
            if (func->getPublicName() == newName)
            {
                nameIsAlreadyTaken = true;
                break;
            }
        }
    }
    
    if (nameIsAlreadyTaken)
        return parentPanel.getUnusedName (newName);
    
    return newName;
}

void WebAudioContext::setPublicName (String newName)
{
    WebAudioContainer::setPublicName (newName);
    
    for (auto node : getAllNodes())
        node->checkConnectionsValidity();
    
    getInstanceManager().updateReference (this);
}

void WebAudioContext::editorShown (Label* label, TextEditor&)
{
    label->setInterceptsMouseClicks(true, true);
    label->getCurrentTextEditor()->setInputRestrictions(0, JsCodeHelpers::getAllowedCharactersForIdentifier());
}

void WebAudioContext::addDestinationNode (Point<int> pos)
{
    Point<int> posInCtx = pos + getPosition();
    
    auto node = parentPanel.addDestinationNode (posInCtx.x, posInCtx.y, this);
    addComponent (node);

    node->setPublicName (".destination");
    node->setCanBeRenamed (false);
    
    navigableChanged();
    getParentGraph()->getSelector().setUniqueSelection (node);
    
    destinationNodes.add (node);
}

Array<WeakReference<WebAudioDestinationNode>>& WebAudioContext::getDestinationNodes()
{
    return destinationNodes;
}
