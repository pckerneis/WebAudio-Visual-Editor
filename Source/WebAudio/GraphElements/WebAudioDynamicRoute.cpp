/*
  ==============================================================================

    WebAudioDynamicRoute.cpp
    Created: 26 Aug 2018 9:21:12pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/GraphElements/WebAudioDynamicRoute.h"

#include "../Source/WebAudio/WebAudioGraph/WebAudioGraph.h"
WebAudioDynamicRoute::WebAudioDynamicRoute (WebAudioGraphPanel* parent) : WebAudioContainer (*parent,  Descriptor (GraphElementType::dynamicRouteType))
{
    setSize (getDefaultWidth(), getDefaultHeight());
    setZ (3);
    
    prepareLabel();
    label->setFont (label->getFont().italicised());
    
    setBackgroundColour (Colours::white);
    
    if (auto border = getChildComponent (0))
        border->setAlpha (0);
    
    prepareInspectablePropertiesTree (getUICompTypeName());
}

WebAudioDynamicRoute::~WebAudioDynamicRoute()
{
    masterReference.clear();
}

void WebAudioDynamicRoute::paint (Graphics &g)
{
    const auto s = GraphEmbeddedComponent::isSelected();
    const auto h = isHighlighted();
    
    float strokeThickness = 0.4;
    if (s)  strokeThickness += 0.2;
    if (h)  strokeThickness += 0.2;
    
    float alpha = 0.5f;
    if (s)  alpha += 0.2f;
    if (h)  alpha += 0.2f;
    
    const Colour strokeColour = Colours::black.withAlpha (h ? 0.8f : s ? 0.6f : 0.5f).overlaidWith (getBackgroundColour());
    
    Path p;
    p.addRectangle (getLocalBounds().reduced(1));
    
    if (s)
    {
        PathStrokeType strokeType (2.0f);
        g.setColour (AppSettings::getCurrentMainColour());
        g.strokePath(p, strokeType);
    }
    
    Path dashed;
    float dashes[2] = { 3., 4. };
    PathStrokeType strokeType (strokeThickness);
    strokeType.createDashedStroke (dashed, p, dashes, 2);
    
    g.setColour (strokeColour);
    g.strokePath (dashed, strokeType);
    
    paintUI (g, getLocalBounds().reduced (2));
}

#include "../Source/WebAudio/Helpers/JsCodeHelpers.h"
String WebAudioDynamicRoute::getValidName (String newName)
{
    if (getParentGraph() == nullptr)
        return newName;
    
    const String oldName = getPublicName();
    
    newName = JsCodeHelpers::removeIllegalCharactersForIdentifier(newName);
    
    if (JsCodeHelpers::isReservedKeyword (newName))
        newName = newName + "_";
    
    if (newName.isEmpty() && getPublicName().isNotEmpty())
        return getValidName (getPublicName());
    
    if (JsCodeHelpers::startsWithDigit (newName))
        newName = "_" + newName;
    
    bool nameIsAlreadyTaken = false;
    
    for (auto emb : getParentGraph()->getEmbeddedComponents())
    {
        if (emb == this)
            continue;
        
        if (auto dr = dynamic_cast<WebAudioDynamicRoute*>(emb))
        {
            if (dr->getPublicName() == newName &&
                dr->getInterfaceName() != getInterfaceName())
            {
                nameIsAlreadyTaken = true;
                break;
            }
        }
        else if (emb->getPublicName() == newName)
        {
            nameIsAlreadyTaken = true;
            break;
        }
    }
    
    if (nameIsAlreadyTaken)
        return parentPanel.getUnusedName (newName);
    
    return newName;
}

void WebAudioDynamicRoute::editorShown (Label* label, TextEditor& e)
{
    label->setInterceptsMouseClicks(true, true);
    e.setInputRestrictions(0, JsCodeHelpers::getAllowedCharactersForIdentifier());
}

#include "../Source/WebAudio/GraphElements/WebAudioContext.h"
void WebAudioDynamicRoute::checkContainers (bool useHighlight)
{
    const auto centre = getBounds().getCentre();
    
    for (auto cont : parentPanel.getContainers())
    {
        if (cont == this)
            continue;
        
        if (auto ctx = dynamic_cast<WebAudioContext*> (cont.get()))
        {
            if (ctx->getBounds().contains (centre))
            {
                ctx->addComponent (this);
                
                if (useHighlight)
                {
                    ctx->setHighlighted (true);
                    getParentGraph()->moveToFront (ctx);
                }
            }
            else
            {
                ctx->removeComponent (this);
                ctx->setHighlighted (false);
            }
        }
    }
}

#include "../Source/WebAudio/GraphElements/WebAudioNode.h"
Array<WeakReference<NavigationPanel::Navigable>> WebAudioDynamicRoute::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    
    for (auto n : getAllNodes (true))
        result.add (n);
    
    for (auto f : getAllScriptsAndMessages (true))
        result.add (f);
    
    return result;
}

#include "../Source/WebAudio/GraphElements/WebAudioMessage.h"
Array<WebAudioMessage*> WebAudioDynamicRoute::getAllMessages (bool sorted) const
{
    Array<WebAudioMessage*> result;
    
    for (auto emb : getContent())
        if (auto m = dynamic_cast<WebAudioMessage*> (emb))
            result.add (m);
    
    if (sorted)
    {
        WebAudioGraphPanel::ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

#include "../Source/WebAudio/GraphElements/WebAudioScript.h"
Array<WebAudioScript*> WebAudioDynamicRoute::getAllScripts (bool sorted) const
{
    Array<WebAudioScript*> result;
    
    for (auto emb : getContent())
        if (auto m = dynamic_cast<WebAudioScript*> (emb))
            result.add (m);
    
    if (sorted)
    {
        WebAudioGraphPanel::ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioNode*> WebAudioDynamicRoute::getAllNodes (bool sorted) const
{
    Array<WebAudioNode*> result;
    
    for (auto emb : getContent())
        if (auto m = dynamic_cast<WebAudioNode*> (emb))
            result.add (m);
    
    if (sorted)
    {
        WebAudioGraphPanel::ComponentPositionComparator comparator;
        result.sort (comparator);
    }
        
    return result;
}

Array<WebAudioEmbedded*> WebAudioDynamicRoute::getAllScriptsAndMessages (bool sorted) const
{
    Array<WebAudioEmbedded*> result;
    
    for (auto emb : getContent())
    {
        if (auto m = dynamic_cast<WebAudioMessage*> (emb))
            result.add (m);
        else if (auto f = dynamic_cast<WebAudioScript*> (emb))
            result.add (f);
    }
    
    if (sorted)
    {
        WebAudioGraphPanel::ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

#include "../Source/Application/CommandIDs.h"
void WebAudioDynamicRoute::addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos)
{
    WebAudioEmbedded::addExtraPopupMenuCommands (m, pos);
    
    m.addSeparator();
    
    PopupMenu addMenu;
    
    using Category = WebAudioGraphPanel::ElementCategory;
    auto flags = Category::nodeCategory | Category::dataCategory | Category::messageCategory | Category::contextCategory | Category::defaultDestinationNode;
    
    parentPanel.getAddMenuItems (addMenu, flags);
    
    m.addSubMenu ("Add...", addMenu);
}

void WebAudioDynamicRoute::handleExtraPopupMenuCommands (int result, Point<int> pos)
{
    WebAudioEmbedded::handleExtraPopupMenuCommands (result, pos);
    
    parentPanel.modalStateFinished (result, pos + getPosition(), this);
}

void WebAudioDynamicRoute::setBackgroundColour (Colour newColour)
{
    WebAudioEmbedded::setBackgroundColour (newColour); // Bypass WebAudioContainer's method
    
    setPropertyValue ("colour", getBackgroundColour().toString());
    
    if (label != nullptr)
        label->setColour (Label::textColourId, newColour.withAlpha (0.9f));
}

bool WebAudioDynamicRoute::hitTest (int x, int y)
{
    const Point<int> p (x + getX(), y + getY());
    const int margin = getBorderThickness().getLeft();
    
    const auto top = getLassoSelectionBounds();
    const auto left = getBounds().removeFromLeft (margin);
    const auto right = getBounds().removeFromRight (margin);
    const auto bottom = getBounds().removeFromBottom (margin);
    
    return top.contains (p) || right.contains (p)
        || bottom.contains (p) || left.contains (p);
}
