/*
  ==============================================================================

    GraphEmbeddedComponent.cpp
    Created: 22 Aug 2017 1:42:14am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/Layout/Graph/GraphEmbeddedComponent.h"
#include "GraphPanel.h"

GraphEmbeddedComponent::Pin::Pin (GraphEmbeddedComponent& ownerComp, Placement p, int channel) : owner (ownerComp), placement (p),channelNumber (channel)
{
    setMouseCursor (MouseCursor::UpDownLeftRightResizeCursor);
}


GraphEmbeddedComponent::Pin::~Pin ()
{
    owner.disconnectPin (this, false);
    masterReference.clear();
}

void GraphEmbeddedComponent::Pin::paint (Graphics &g)
{
    float x = 0;
    float y = 0;
    
    switch (placement)
    {
        case Placement::PinOnTop :      x = (getWidth() - pinWidth) * 0.5;  y = -pinWidth * 0.5;                   break;
        case Placement::PinOnRight :    x = getWidth() - (pinWidth * 0.5);  y = (getHeight() - pinWidth) * 0.5;    break;
        case Placement::PinOnBottom :   x = (getWidth() - pinWidth) * 0.5;  y = getHeight() - (pinWidth * 0.5);    break;
        case Placement::PinOnLeft :     x = -pinWidth * 0.5;                y = (getHeight() - pinWidth) * 0.5;    break;
    }
    
    const juce::Rectangle<float> r (x, y, pinWidth, pinWidth);
    
    g.setColour (Colours::black.withBrightness (isEnabled() ? 0.15 : 0.25));
    g.fillEllipse (r);
}

void GraphEmbeddedComponent::Pin::mouseDown (const MouseEvent& e)
{
    /*
    if (!isConnected()
        || (isConnected() && !owner.getAutoPinsManagement()))
     */
    if (isEnabled())
        owner.getParentGraph()->startTemporaryConnection(this);
}

void GraphEmbeddedComponent::Pin::mouseDrag (const MouseEvent& e)
{
    owner.getParentGraph()->moveTemporaryConnection();
};

void GraphEmbeddedComponent::Pin::mouseUp (const MouseEvent& e)
{
    owner.getParentGraph()->releaseTemporaryConnection();
}

//==============================================================================
GraphEmbeddedComponent::GraphEmbeddedComponent() : backgroundColour (Colours::darkgrey)
{
}

GraphEmbeddedComponent::~GraphEmbeddedComponent()
{
    masterReference.clear();
}

GraphPanel* GraphEmbeddedComponent::getParentGraph () const
{
    return parentGraphPanel;
}

void GraphEmbeddedComponent::setParentGraph (GraphPanel* parent)
{
    parentGraphPanel = parent;
}

void GraphEmbeddedComponent::setGraphId (int newGraphId)
{
    graphId = newGraphId;
}

#include "../Source/Layout/Graph/GraphDragger.h"
void GraphEmbeddedComponent::mouseDown(const MouseEvent& e)
{
    GraphSelectableItem::mouseDown(e);
    
    positionOnLastMouseDown = getPosition();
    
    toFront(true);
    
    if (getSelector() != nullptr && getSelector()->getDragger() != nullptr)
        getSelector()->getDragger()->startDragging (e);
}

void GraphEmbeddedComponent::mouseDrag (const MouseEvent& e)
{
    GraphSelectableItem::mouseDrag(e);
    
    if (! e.mouseWasDraggedSinceMouseDown())
        return;
    
    if (getSelector() != nullptr && getSelector()->getDragger() != nullptr)
        getSelector()->getDragger()->dragSelection (e);
    
    if (parentGraphPanel != nullptr)
        parentGraphPanel->adaptSizeToContent (true);
    
    if (auto vp = findParentComponentOfClass<Viewport>())
    {
        auto localEvent = e.getEventRelativeTo (vp);
        vp->autoScroll (localEvent.x, localEvent.y, 20, 5);
        beginDragAutoRepeat (10);
    }
    
    // for connections to redraw
    graphItemChanged();
}

void GraphEmbeddedComponent::mouseUp (const MouseEvent &e)
{
    GraphSelectableItem::mouseUp (e);
    
    if (parentGraphPanel != nullptr)
        parentGraphPanel->adaptSizeToContent();
    
    if (getSelector() != nullptr && getSelector()->getDragger() != nullptr)
        getSelector()->getDragger()->stopDragging (e);
    
    if (e.mouseWasClicked() && e.mods.isRightButtonDown())
    {
        setHighlighted (false);
        showPopupMenu(e.getPosition());
    }
}

#include "../Source/Application/CommandIDs.h"
#include "../Source/Project/Project.h"

PopupMenuHandler* popupMenuHandler;

void GraphEmbeddedComponent::showPopupMenu(Point<int> pos)
{
    // When right clicking on a comp while there's already a popup menu open in another panel, this
    // may not have the keyboard focus yet when this method is called. So we need to force the keyboard
    // focus to make sure the commands displayed are up-to-date !
    grabKeyboardFocus();
    
    auto& acm = Project::getApplicationCommandManager();
    PopupMenu m;
    
    m.addCommandItem (&acm, CommandIDs::copy);
    m.addCommandItem (&acm, CommandIDs::cut);
    m.addCommandItem (&acm, CommandIDs::paste);
    m.addCommandItem (&acm, CommandIDs::duplicateSelection);
    m.addCommandItem (&acm, CommandIDs::del);
    
    if (canBeRenamed())
    {
        //m.addSeparator();
        m.addCommandItem (&acm, CommandIDs::rename);
    }
    
    addExtraPopupMenuCommands (m, pos);
    
    SharedResourcePointer<PopupMenuHandler> handler;
    popupMenuHandler = handler;
    
    popupMenuHandler->setCaller (this);
    
    int r = m.show();
    popupMenuHandler->handleResult (r, pos);
    popupMenuHandler = nullptr;
}

#include "../Source/Application/AppSettings.h"
void GraphEmbeddedComponent::paint(Graphics &g)
{
    const bool h = isHighlighted();
    const bool s = isSelected();
    
    float strokeAlpha = 0.5f;
    if (h)  strokeAlpha += 0.2f;
    if (s)  strokeAlpha += 0.2f;
    
    const Colour bgColour = s ? backgroundColour : backgroundColour.withMultipliedBrightness (h ? 0.95f : 0.85f);
    
    const Colour strokeColour = s ? AppSettings::getCurrentMainColour()
                                : bgColour.interpolatedWith (Colours::black, 0.5f);
    
    g.setColour (bgColour);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 4);
    
    float stroke = 0.5f;
    if (h)  stroke += 0.5f;
    if (s)  stroke += 1.2f;
    
    paintUI (g, getLocalBounds().reduced(2));
    
    g.setColour (strokeColour);
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.0f), 4, stroke);
}

void GraphEmbeddedComponent::resized()
{
    resizePins();
    resizeUI (getLocalBounds().reduced(2));
}

void GraphEmbeddedComponent::deleteSelectedItem()
{
    if (parentGraphPanel != nullptr && canBeDeleted())
        parentGraphPanel->removeComponent (this, true);
}

const OwnedArray<GraphEmbeddedComponent::Pin>& GraphEmbeddedComponent::getPins (Pin::Placement placement) const
{
    switch (placement)
    {
        case Pin::Placement::PinOnTop :     return topPins;
        case Pin::Placement::PinOnRight :   return rightPins;
        case Pin::Placement::PinOnBottom :  return bottomPins;
        case Pin::Placement::PinOnLeft :    return leftPins;
    }
}

OwnedArray<GraphEmbeddedComponent::Pin>& GraphEmbeddedComponent::getPinsInternal (Pin::Placement placement)
{
    switch (placement)
    {
        case Pin::Placement::PinOnTop :     return topPins;
        case Pin::Placement::PinOnRight :   return rightPins;
        case Pin::Placement::PinOnBottom :  return bottomPins;
        case Pin::Placement::PinOnLeft :    return leftPins;
    }
}

void GraphEmbeddedComponent::setNumPins (int numPins, Pin::Placement placement)
{
    auto& pins = getPinsInternal (placement);
    const int currentNumPins = pins.size();
    
    if (numPins == currentNumPins)
        return;
    
    const int start = jmax (numPins, currentNumPins);
    const int end = jmin (numPins, currentNumPins);
    
    for (int i = start; --i >= end;)
    {
        if (i >= numPins)
        {
            pins.remove (i);
        }
        else
        {
            Pin* p = new Pin (*this, placement, pins.size());
            pins.add (p);
            addAndMakeVisible (p);
        }
    }
    
    resizePins();
}

void GraphEmbeddedComponent::addPins (Pin::Placement placement, int numToAdd)
{
    if (numToAdd > 0)
    {
        auto& arrayToAddTo = getPinsInternal (placement);
        int firstIndex = arrayToAddTo.size();
    
        for (int i = 0; i < numToAdd; ++i)
        {
            Pin* p = new Pin (*this, placement, firstIndex + i);
            arrayToAddTo.add (p);
            addAndMakeVisible (p);
        }
    
        resizePins();
    }
}

void GraphEmbeddedComponent::removePin (Pin::Placement placement, int indexToRemove)
{
    auto& arrayToRemoveFrom = getPinsInternal (placement);
    
    if (indexToRemove >= 0 && indexToRemove < arrayToRemoveFrom.size())
    {
        //disconnectPin(arrayToRemoveFrom.getUnchecked(indexToRemove));
        arrayToRemoveFrom.remove (indexToRemove);
        
        resizePins();
    }
}

void GraphEmbeddedComponent::insertPin (Pin::Placement placement, int indexToInsert)
{
    auto& arrayToInsertIn = getPinsInternal (placement);
    
    if (indexToInsert >= 0)
    {
        Pin* p = new Pin (*this, placement, indexToInsert);
        arrayToInsertIn.insert (indexToInsert, p);
        addAndMakeVisible (p);
        
        for (int i = 0; i < arrayToInsertIn.size(); ++i)
            arrayToInsertIn.getUnchecked(i)->setChannelNumber(i);
        
        resizePins();
    }
}

void GraphEmbeddedComponent::resizePins()
{
    const int pinWidth = 10;
    
	juce::Rectangle<float> topPinsArea (getLocalBounds().removeFromTop (12).toFloat());
    const float numTop = topPins.size();
    for (int i = 0; i < numTop; ++i)
        topPins[i]->setBounds (topPinsArea.removeFromLeft (float(getWidth()) / numTop).withSizeKeepingCentre (pinWidth, pinWidth).getSmallestIntegerContainer());
    
	juce::Rectangle<float> rightPinsArea (getLocalBounds().removeFromRight (12).toFloat());
    const float numRight = rightPins.size();
    for (int i = 0; i < numRight; ++i)
        rightPins[i]->setBounds (rightPinsArea.removeFromTop (float(getHeight()) / numRight).withSizeKeepingCentre (pinWidth, pinWidth).getSmallestIntegerContainer());
    
	juce::Rectangle<float> botPinsArea (getLocalBounds().removeFromBottom (12).toFloat());
    const float numBot = bottomPins.size();
    for (int i = 0; i < numBot; ++i)
        bottomPins[i]->setBounds (botPinsArea.removeFromLeft (float(getWidth()) / numBot).withSizeKeepingCentre (pinWidth, pinWidth).getSmallestIntegerContainer());
    
	juce::Rectangle<float> leftPinsArea (getLocalBounds().removeFromLeft (12).toFloat());
    const float numLeft = leftPins.size();
    for (int i = 0; i < numLeft; ++i)
        leftPins[i]->setBounds (leftPinsArea.removeFromTop (float(getHeight()) / numLeft).withSizeKeepingCentre (pinWidth, pinWidth).getSmallestIntegerContainer());
    
    // For connections to repaint
    graphItemChanged();
}

void GraphEmbeddedComponent::clearPins (bool undoable)
{
    if (parentGraphPanel != nullptr)
        parentGraphPanel->disconnect (this, undoable);
    
    auto placements = getAllPlacements();
    
    for (auto placement : placements)
    {
        for (auto pin : getPinsInternal (placement))
        {
            removeChildComponent (pin);
            getPinsInternal (placement).removeObject (pin);
        }
    }
}

GraphEmbeddedComponent::Pin* GraphEmbeddedComponent::replaceWithTempPin (Pin* pinToMove)
{
    if (pinToMove == nullptr)
        return pinToMove;
    
    const Pin::Placement placement = pinToMove->getPlacement();
    
    if (getPins(placement).size() < 2)
        return pinToMove;
    
    if (!pinToMove->isConnected())
        return pinToMove;
    
    int emptyPinIndex = -1;
    int indexToMoveTo = -1;
    
    for (int i = 0; i < getPins (placement).size(); ++i)
    {
        auto pin = getPins(placement)[i];
        
        if (!pin->isConnected())
            emptyPinIndex = pin->getChannelNumber();
        
        if (pin == pinToMove)
            indexToMoveTo = i;
    }
    
    if (emptyPinIndex >= 0 && indexToMoveTo >= 0 && emptyPinIndex != indexToMoveTo)
    {
        getPinsInternal(placement).move(emptyPinIndex, indexToMoveTo);
        
        for (int i = 0; i < getPins(placement).size(); ++i)
            getPins(placement)[i]->setChannelNumber (i);
        
        resizePins();
        
        return getPins(placement)[indexToMoveTo];
    }
    
    return pinToMove;
}

void GraphEmbeddedComponent::putTempPinsAtLast()
{
    auto placements = getAllPlacements();
    
    for (auto p : placements)
    {
        auto& pins = getPins(p);
        if (pins.size() > 0 && pins.getLast()->isConnected())
            replaceWithTempPin(pins.getLast());
    }
}
void GraphEmbeddedComponent::setNumPinsUpTo (Pin::Placement placement, int num)
{
    auto& array = getPins (placement);
    
    const int lastIndex = array.size() - 1;
    if (lastIndex < num)
        addPins (placement, num - lastIndex);
}

void GraphEmbeddedComponent::disconnectPin (Pin* pin, bool undoable)
{
    if (pin != nullptr && pin->isConnected() && parentGraphPanel != nullptr)
        for (auto connection : parentGraphPanel->getConnections())
            if (connection->getSourcePin() == pin || connection->getDestinationPin() == pin)
                parentGraphPanel->removeConnectionBetween(connection->getSourcePin(), connection->getDestinationPin(), undoable);
}
