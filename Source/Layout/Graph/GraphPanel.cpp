/*
  ==============================================================================

    GraphPanel.cpp
    Created: 21 Aug 2017 8:37:46pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "GraphPanel.h"


//==============================================================================

GraphPanel::Connection::Connection (GraphPanel* panel, Pin* source, Pin* dest) : sourcePin (source), destPin (dest), graphPanel (panel)
{
    if (source != nullptr)
        source->getOwnerComponent()->addListener (this);
    
    if (dest != nullptr)
        dest->getOwnerComponent()->addListener (this);
}

GraphPanel::Connection::~Connection()
{
    stopListening();
    
    masterReference.clear();
}

void GraphPanel::Connection::stopListening()
{
    if (sourcePin->getOwnerComponent() != nullptr)   // Because this is called in the destructor, we must be sure the UI comp still exists (at shutdown, it may not be the case...)
        sourcePin->getOwnerComponent()->removeListener (this);
    
    if (destPin->getOwnerComponent() != nullptr)
        destPin->getOwnerComponent()->removeListener (this);
}

ConnectionInfo GraphPanel::Connection::getInfo()
{
    return ConnectionInfo (sourcePin,
                           sourcePin->getPlacement(),
                           sourcePin->getChannelNumber(),
                           sourcePin->getOwnerComponent(),
                           destPin,
                           destPin->getPlacement(),
                           destPin->getChannelNumber(),
                           destPin->getOwnerComponent(),
                           this);
}

void GraphPanel::Connection::paint (Graphics &g)
{
    const Point<float> sourcePoint (getPinCentreInGraph (sourcePin).toFloat());
    const Point<float> destPoint (getPinCentreInGraph (destPin).toFloat());
    
    const int stroke = isSelected() ? 4 : 2;
    const Colour baseColour = isValid ? Colours::black : Colours::grey;
    const Colour colour = (isHighlighted() || isSelected()) ? baseColour : baseColour.withAlpha (0.7f);
        
    Path p (drawConnection (sourcePoint, destPoint, stroke, colour, sourcePin->getPlacement(), destPin->getPlacement(), g));
    
    PathStrokeType wideStroke (8.0f);
    wideStroke.createStrokedPath (hitPath, p);
}

Path GraphPanel::Connection::drawConnection (Point<float> from, Point<float> to, int stroke, Colour colour, Pin::Placement sourcePlacement, Pin::Placement destPlacement, Graphics &g)
{
    const int offsetX = to.getX() - from.getX();
    const int offsetY = to.getY() - from.getY();
    
    const int twist = 50;
    
    int marginX = offsetX * 0.35f;
    int marginY = offsetY * 0.35f;
    
    if (offsetX < twist)
        marginX = jmin (jmax (marginX, twist - offsetX), twist);
    
    if (offsetY < twist)
        marginY = jmin (jmax (marginY, twist - offsetY), twist);
    
    const Point<float> cp1 (getControlPoint (from.toInt(), sourcePlacement, marginX, marginY).toFloat());
    const Point<float> cp2 (getControlPoint (to.toInt(), destPlacement, marginX, marginY).toFloat());
    
    Path p;
    p.startNewSubPath(from);
    p.cubicTo (cp1, cp2, to);
    g.setColour(colour);
    g.strokePath(p, PathStrokeType(stroke));
    
    return p;
}

Path GraphPanel::Connection::drawTemporaryConnection (Point<float> from, Point<float> to, int stroke, Colour colour, Pin::Placement sourcePlacement, Graphics &g)
{
    const int offsetX = to.getX() - from.getX();
    const int offsetY = to.getY() - from.getY();
    
    const int twist = 50;
    
    int marginX = offsetX * 0.35f;
    int marginY = offsetY * 0.35f;
    
    if (offsetX < twist)
        marginX = jmin (jmax (marginX, twist - offsetX), twist);
    
    if (offsetY < twist)
        marginY = jmin (jmax (marginY, twist - offsetY), twist);
    
    const Point<float> cp1 (getControlPoint (from.toInt(), sourcePlacement, marginX, marginY).toFloat());
    const Point<float> cp2 (from + (to - from) * 0.85f);
    
    Path p;
    p.startNewSubPath(from);
    p.cubicTo (cp1, cp2, to);
    g.setColour(colour);
    g.strokePath(p, PathStrokeType(stroke));
    
    return p;
}

Point<int> GraphPanel::Connection::getControlPoint (Point<int> pinCentre, Pin::Placement placement, int marginX, int marginY)
{
    int cpX, cpY;
    
    if (placement == Pin::PinOnTop)
    {
        cpX = pinCentre.getX();
        cpY = pinCentre.getY() - marginY;
    }
    else if (placement == Pin::PinOnRight)
    {
        cpX = pinCentre.getX() + marginX;
        cpY = pinCentre.getY();
    }
    else if (placement == Pin::PinOnBottom)
    {
        cpX = pinCentre.getX();
        cpY = pinCentre.getY() + marginY;
    }
    else
    {
        cpX = pinCentre.getX() - marginX;
        cpY = pinCentre.getY();
    }
    
    return Point<int> (cpX, cpY);
}

void GraphPanel::Connection::graphEmbeddedChanged()
{
    repaint();
}

bool GraphPanel::Connection::hitTest (int x, int y)
{
    if (hitPath.contains ((float) x, (float) y))
    {
        double distanceFromStart, distanceFromEnd;
        getDistancesFromEnds (x, y, distanceFromStart, distanceFromEnd);
        
        // avoid clicking the connector when over the pin
        return distanceFromStart > 8.0 && distanceFromEnd > 8.0;
    }
    
    return false;
}

void GraphPanel::Connection::getDistancesFromEnds (int x, int y, double& distanceFromStart, double& distanceFromEnd)
{
    const Point<int> start (getPinCentreInGraph(sourcePin));
    const Point<int> end (getPinCentreInGraph(destPin));
    
    distanceFromStart = distance(start.getX(), start.getY(), x, y);
    distanceFromEnd = distance(end.getX(), end.getY(), x, y);
}

Point<int> GraphPanel::Connection::getPinCentreInGraph (const Pin* p)
{
    const auto placement = p->getPlacement();
    const auto pinBounds = p->getBounds();
    const auto ownerBounds = p->getOwnerComponent()->getBounds();
    
    Point<int> result;
    
    switch (placement)
    {
        case GraphEmbeddedComponent::Pin::PinOnTop :
            result = pinBounds.getCentre().withY(pinBounds.getY());
            break;
        case GraphEmbeddedComponent::Pin::PinOnRight :
            result = pinBounds.getCentre().withX(pinBounds.getRight());
            break;
        case GraphEmbeddedComponent::Pin::PinOnBottom :
            result = pinBounds.getCentre().withY(pinBounds.getBottom());
            break;
        case GraphEmbeddedComponent::Pin::PinOnLeft :
            result = pinBounds.getCentre().withX(pinBounds.getX());
            break;
    }
    
    return result.translated(ownerBounds.getX(),
                             ownerBounds.getY());
}

 void GraphPanel::Connection::deleteSelectedItem()
 {
     if (graphPanel != nullptr)
         graphPanel->removeConnectionBetween (sourcePin, destPin, true);
 }

//==============================================================================

GraphPanel::TemporaryConnection::TemporaryConnection (GraphPanel* panel, GraphEmbeddedComponent::Pin* pin, Point<int> point, bool pinToPoint)
:   fromPinToPoint (pinToPoint)
{
    if (fromPinToPoint)
        sourcePin = pin;
    else
        destPin = pin;
}

void GraphPanel::TemporaryConnection::paint (Graphics &g)
{
    Point<float> nodePos;
    const int stroke = 2;
    const Colour colour = Colours::black.withBrightness(0.1f);
    
    if (fromPinToPoint)
    {
        nodePos = Point<float> (sourcePin->getBounds().getCentre()
                                .translated(sourcePin->getOwnerComponent()->getX(),
                                            sourcePin->getOwnerComponent()->getY())
                                .toFloat());
        
        Connection::drawTemporaryConnection (nodePos, floatingPoint.toFloat(), stroke, colour, sourcePin->getPlacement(), g);
    }
    else
    {
        nodePos = Point<float> (destPin->getBounds().getCentre()
                                .translated(destPin->getOwnerComponent()->getPosition().getX(),
                                            destPin->getOwnerComponent()->getPosition().getY())
                                .toFloat());
        
        Connection::drawTemporaryConnection (nodePos, floatingPoint.toFloat(), stroke, colour, destPin->getPlacement(), g);
    }
}

void GraphPanel::TemporaryConnection::setPotentialDestination (Pin* newDestination)
{
    if (fromPinToPoint)     destPin = newDestination;
    else                    sourcePin = newDestination;
}

//==============================================================================

GraphPanel::Listener::~Listener()
{
    masterReference.clear();
}

//==============================================================================
int GraphPanel::addComponent (GraphEmbeddedComponent* comp, int x, int y)
{
    comp->setGraphId (++lastIdGiven);
    comp->setParentGraph (this);
    comp->setTopLeftPosition (x, y);
    comp->setSelector (&selector);
    comp->setPositionOnLastMouseDown (Point<int> (x, y));
    
    addAndMakeVisible (comp);
    
    embeddedComponents.addIfNotAlreadyThere (comp);
    
    moveToFront (comp, true);
    
    adaptSizeToContent();
    graphContentChanged();
        
    return lastIdGiven;
}

bool GraphPanel::removeComponent (GraphEmbeddedComponent* comp, bool undoable)
{
    if (embeddedComponents.contains (comp))
    {
        disconnect (comp, undoable);
        
        removeChildComponent (comp);
        embeddedComponents.removeObject (comp);
       
        adaptSizeToContent (false);
        graphContentChanged();
        
        comp->setParentGraph (nullptr);
        comp->wasRemovedFromGraph();
        
        return true;
    }
    
    return false;
}

GraphPanel::Connection* GraphPanel::createConnectionObject (GraphPanel* panel, Pin* source, Pin* dest)
{
    return new Connection (panel, source, dest);
}

bool GraphPanel::createConnectionBetween (int sourceId, int sourceChannel,
                                          GraphEmbeddedComponent::Pin::Placement sourcePlacement,
                                          int destId, int destChannel,
                                          GraphEmbeddedComponent::Pin::Placement destPlacement,
                                          bool undoable)
{
    if (sourceId < 0 && sourceChannel < 0 && destId < 0 && destChannel < 0)
        return false;
    
    auto source = lookForPinWithIds (sourceId, sourceChannel, sourcePlacement);
    auto dest = lookForPinWithIds (destId, destChannel, destPlacement);
    
    if (source == nullptr || dest == nullptr)
        return false;
        
    return createConnectionBetween (source, dest, undoable);
}

bool GraphPanel::createConnectionBetween (Pin* source, Pin* dest, bool undoable)
{
    if (source == nullptr || dest == nullptr)
        return false;
    
    Connection* newConnection = createConnectionObject (this, source, dest);
    connections.add (newConnection);
    addAndMakeVisible (newConnection);
    newConnection->setSelector (&selector);
    
    source->incNumConnections();
    dest->incNumConnections();
    
    resized();
    
    source->owner.wasConnected();
    dest->owner.wasConnected();
    
    return true;
}

bool GraphPanel::removeConnectionBetween (Pin* source, Pin* dest, bool undoable)
{
    if (source == nullptr || dest == nullptr)
        return false;
    
    Connection* connectionToRemove = nullptr;
    
    for (auto connection : connections)
        if (connection->getSourcePin() == source && connection->getDestinationPin() == dest)
            connectionToRemove = connection;
    
    if (connectionToRemove == nullptr)
        return false;
    
    removeChildComponent (connectionToRemove);
    connections.removeObject (connectionToRemove);
    
    source->decNumConnections();
    dest->decNumConnections();
    
    source->owner.wasDisconnected();
    dest->owner.wasDisconnected();
    
    return true;
}

bool GraphPanel::disconnect (GraphEmbeddedComponent* comp, bool undoable)
{
    if (comp == nullptr)
        return false;
    
    for (int i = connections.size(); --i >= 0;)
    {
        Pin* sourcePin = connections.getUnchecked(i)->getSourcePin();
        Pin* destPin = connections.getUnchecked(i)->getDestinationPin();
        
        if (sourcePin->getOwnerComponent() == comp || destPin->getOwnerComponent() == comp)
            removeConnectionBetween (sourcePin, destPin, undoable);
    }
    
    return false;
}

void GraphPanel::moveToFront (GraphEmbeddedComponent* comp, bool useZ)
{
    if (comp == nullptr)
        return;
    
    if (! useZ)
    {
        comp->toFront (false);
        return;
    }
    
    auto sortedComps = sortWithZ (false);
    
    bool compWasMoved = false;
    bool connectionsSorted = false;
    
    for (int i = sortedComps.size(); --i >= 0;)
    {
        auto other = sortedComps.getUnchecked (i);
        
        if (! compWasMoved && other->getZ() <= comp->getZ())
        {
            comp->toBack();
            compWasMoved = true;
        }
        
        if (! connectionsSorted && other->getZ() < connectionsZ)
        {
            for (auto c : connections)
                c->toBack();
            
            connectionsSorted = true;
        }
        
        other->toBack();
    }
    
    if (!compWasMoved)
        comp->toBack();
    
    if (! connectionsSorted)
        for (auto c : connections)
            c->toBack();
}

ReferenceCountedArray<GraphEmbeddedComponent> GraphPanel::sortWithZ (bool alsoArrangeComponents)
{
    class Comparator
    {
    public:
        static int compareElements (const GraphEmbeddedComponent* first,
                                    const GraphEmbeddedComponent* second)
        {
            return (first->getZ() < second->getZ()) ? -1 : ((second->getZ() < first->getZ()) ? 1 : compareSelectionState (first, second));
        }
        
        static int compareSelectionState (const GraphEmbeddedComponent* first,
                                          const GraphEmbeddedComponent* second)
        {
            const int firstSel = first->isSelected() ? 1 : 0;
            const int secondSel = second->isSelected() ? 1 : 0;
            
            return (firstSel < secondSel) ? -1 : (secondSel < firstSel ? 1 : 0);
        }
    };
    
    // Make a copy
    ReferenceCountedArray<GraphEmbeddedComponent> comps (embeddedComponents);
    
    Comparator comparator;
    comps.sort (comparator);
    
    if (alsoArrangeComponents)
    {
        bool connectionsSorted = false;
        
        for (int i = embeddedComponents.size(); --i >= 0;)
        {
            auto comp = embeddedComponents.getUnchecked(i);
            
            if (! connectionsSorted && comp->getZ() < connectionsZ)
            {
                for (auto c : connections)
                    c->toBack();
                
                connectionsSorted = true;
            }
            
            comp->toBack();
        }
        
        if (! connectionsSorted)
            for (auto c : connections)
                c->toBack();
    }
    
    return comps;
}

void GraphPanel::setZForConnections (int newZ)
{
    connectionsZ = newZ;
}
//==============================================================================

GraphPanel::GraphPanel() : selector (*this)
{
    prepareCommandTarget();
}

GraphPanel::~GraphPanel()
{
    masterReference.clear();
}

void GraphPanel::resized()
{
    if (tempConnection != nullptr)
        tempConnection->setBounds(getLocalBounds());
    
    for (auto& connection : connections)
        connection->setBounds(getLocalBounds());
}

void GraphPanel::adaptSizeToContent (bool stretchOnly)
{
    const int lastCompMargin = 40;
    int contentWidth = 0;
    int contentHeight = 0;
    
    for (auto comp : embeddedComponents)
    {
        const int width = comp->getRight() + lastCompMargin;
        if (width > contentWidth)
            contentWidth = width;
        
        const int height = comp->getBottom() + lastCompMargin;
        if (height > contentHeight)
            contentHeight = height;
    }
    
    bool shouldResize = true;
    
    if (contentWidth == getWidth() && contentHeight == getHeight())
        shouldResize = false;
    else if (stretchOnly && contentWidth < getWidth() && contentHeight < getHeight())
        shouldResize = false;
    
    if (shouldResize)
        setSize (contentWidth, contentHeight);
    
    for (auto l : listeners)
        l->graphPanelSizeMayHaveChanged();
}

void GraphPanel::mouseDown (const MouseEvent& e)
{
    if (isCurrentlyActive() && !e.mods.isAnyModifierKeyDown())
        deselectAll();
    
    if (lasso == nullptr && e.mods.isLeftButtonDown())
        startLasso (e);
}

void GraphPanel::mouseUp (const MouseEvent &e)
{
    if (e.mouseWasClicked() && e.mods.isRightButtonDown())
        showPopupMenu (e.getPosition());
    
    endLasso();
}

void GraphPanel::mouseDrag (const MouseEvent& e)
{
    if (e.mouseWasDraggedSinceMouseDown())
        dragLasso(e);
}

#include "WaveLookAndFeel.h"
void GraphPanel::startLasso (const MouseEvent& e)
{
    auto c = LookAndFeelUpdater::getLookAndFeel().getUIColour (LookAndFeel_V4::ColourScheme::UIColour::highlightedFill);
    
    addAndMakeVisible (lasso = new LassoComponent<GraphSelectableItem*>());
    lasso->setColour (LassoComponent<GraphSelectableItem*>::lassoFillColourId, c.withAlpha (0.2f));
    lasso->beginLasso (e, &selector);
    lasso->toBack();
}

void GraphPanel::dragLasso (const MouseEvent& e)
{
    if (lasso != nullptr)
    {
        lasso->dragLasso (e);
        lasso->toBack();
    }
}

void GraphPanel::endLasso()
{
    if (lasso != nullptr)
    {
        lasso->endLasso();
        lasso = nullptr;
    }
}

void GraphPanel::clear()
{
    removeAllChildren();
    connections.clear();
    embeddedComponents.clear();
}

void GraphPanel::startTemporaryConnection (GraphEmbeddedComponent::Pin* starterPin)
{
    if (starterPin != nullptr)
    {
        deleteTemporaryConnection();
        
        bool fromPinToPoint (starterPin->getPlacement() == Pin::Placement::PinOnRight
                             || starterPin->getPlacement() == Pin::Placement::PinOnBottom);
        
        tempConnection = new TemporaryConnection (this,
                                                  starterPin,
                                                  getMouseXYRelative(), // dragged point position
                                                  fromPinToPoint);
        addAndMakeVisible(tempConnection);
        resized();
    }
}

void GraphPanel::moveTemporaryConnection()
{
    if (tempConnection != nullptr)
    {
        tempConnection->move (getMouseXYRelative());
        GraphEmbeddedComponent::Pin* p = lookForPinAt (getMouseXYRelative());
        
        if (p != nullptr && canConnect (tempConnection->getDragSourcePin(), p))
        {
            auto newPin = p->owner.replaceWithTempPin (p);
            tempConnection->setPotentialDestination (newPin);
            setHighlighted (newPin->getOwnerComponent());
        }
        else
        {
            tempConnection->setPotentialDestination (nullptr);
            setHighlighted (nullptr);
        }
    }
}

void GraphPanel::releaseTemporaryConnection()
{
    undoManager.beginNewTransaction();
    
    if (tempConnection != nullptr)
        createConnectionBetween (tempConnection->getSourcePin(), tempConnection->getDestinationPin(), true);
    
    deleteTemporaryConnection();
    setHighlighted (nullptr);
    
    for (auto emb : embeddedComponents)
        emb->putTempPinsAtLast();
}

void GraphPanel::deleteTemporaryConnection()
{
    if (tempConnection != nullptr)
    {
        removeChildComponent(tempConnection);
        tempConnection = nullptr;
    }
}

bool GraphPanel::canConnect (Pin* source, Pin* destination) const
{
    if (source == nullptr || destination == nullptr /*|| source->owner == destination->owner*/)
        return false;
    
    if (areAlreadyConnectedTogether (source, destination))
        return false;
    
    return true;
}

void GraphPanel::setHighlighted (GraphEmbeddedComponent* compToHighlight)
{
    for (auto comp : embeddedComponents)
        comp->setHighlighted (comp == compToHighlight);
}

GraphEmbeddedComponent::Pin* GraphPanel::lookForPinAt (Point<int> p)
{
    for (auto comp : embeddedComponents)
        if (comp->getBounds().contains (p))
            for (auto placement : comp->getAllPlacements())
                for (auto pin : comp->getPins (placement))
                    if (pin->isEnabled() && getRelativeBoundsForPin (pin).contains(p))
                        return pin;
    
    return nullptr;
}

GraphEmbeddedComponent::Pin* GraphPanel::lookForPinWithIds (int graphId, int channelId, Pin::Placement placementToLookFor)
{
    for (auto comp : embeddedComponents)
        if (comp->getGraphId() == graphId)
            if (auto pin = comp->getPins (placementToLookFor)[channelId])
                return pin;
    
    return nullptr;
}

juce::Rectangle<int> GraphPanel::getRelativeBoundsForPin (Pin* p)
{
    if (p != nullptr)
        return juce::Rectangle<int> (p->getBounds().translated (p->getOwnerComponent()->getX(),
                                                          p->getOwnerComponent()->getY()));
    
    return juce::Rectangle<int> ();
}

void GraphPanel::sortConnections()
{
    struct ConnectionComparator
    {
        int compareElements (Connection* first, Connection* second)
        {
            const int sourceComp = first->getSourcePin()->getChannelNumber() - second->getSourcePin()->getChannelNumber();
            const int destComp = first->getDestinationPin()->getChannelNumber() - second->getDestinationPin()->getChannelNumber();
            return sourceComp == 0 ? destComp : sourceComp;
        }
    };
    
    ConnectionComparator comparator;
    connections.sort (comparator);
}

bool GraphPanel::areAlreadyConnectedTogether (Pin* one, Pin* another) const
{
    for (auto connection : connections)
    {
        if (connection->getSourcePin() == one)
            if (connection->getDestinationPin() == another)
                return true;
        
        if (connection->getSourcePin() == another)
            if (connection->getDestinationPin() == one)
                return true;
    }
    
    return false;
}

Array<WeakReference<GraphEmbeddedComponent>> GraphPanel::getAllConnected (const GraphEmbeddedComponent* comp, Pin::Placement placement) const
{
    using Placement = GraphEmbeddedComponent::Pin::Placement;
    
    Array<WeakReference<GraphEmbeddedComponent>> r;
    
    auto infos = getAllConnections (comp, placement);
    
    for (auto info : infos)
    {
        if (placement == Placement::PinOnBottom || placement == Placement::PinOnRight)
            r.addIfNotAlreadyThere (info.destComp);
        else
            r.addIfNotAlreadyThere (info.sourceComp);
    }
    
    return r;
}

Array<ConnectionInfo> GraphPanel::getAllConnections (const GraphEmbeddedComponent* comp, Pin::Placement placement) const
{
    using Placement = GraphEmbeddedComponent::Pin::Placement;
    
    Array<ConnectionInfo> result;
    
    if (comp == nullptr)
        return result;
    
    const OwnedArray<Pin>& pins = comp->getPins(placement);
    
    if (placement == Placement::PinOnBottom || placement == Placement::PinOnRight)
    {
        for (auto pin : pins)
            for (auto connection : connections)
                if (connection->getSourcePin() == pin)
                    result.add (connection->getInfo());
    }
    else
    {
        for (auto pin : pins)
            for (auto connection : connections)
                if (connection->getDestinationPin() == pin)
                    result.add (connection->getInfo());
    }
    
    return result;
}

#include "InternalClipboard.h"
bool GraphPanel::canPaste() const
{
    SharedResourcePointer<InternalClipboard> clipboard;
    const String cb = clipboard->getClipboard();
    
    if (cb == String())
        return false;
    
    XmlDocument doc (cb);
    ScopedPointer<XmlElement> elem = doc.getDocumentElement();
    
    if (elem == nullptr)
        return false;
    
    if (!elem->hasTagName (getClipboardTagName()))
        return false;
    
    if (elem->getNumChildElements() == 0)
        return false;
    
    return true;
}

void GraphPanel::copySelection()
{
    clipboard->setClipboard (getSelectionAsString());
}

String GraphPanel::getSelectionAsString()
{
    ScopedPointer<XmlElement> sel (getSelectionAsXml());
    return sel->createDocument (String());
}

XmlElement* GraphPanel::getSelectionAsXml()
{
    refreshGraphIds();
    selector.sort();
    
    auto selection = new XmlElement (getClipboardTagName());
    
    Array<int> graphIds;
    
    for (auto item : selector.getSelection())
    {
        if (auto emb = dynamic_cast<GraphEmbeddedComponent*> (item))
        {
            selection->addChildElement (getXmlFor (emb));
            graphIds.add (emb->getGraphId());
        }
    }
    
    if (selection->getNumChildElements() > 0)
    {
        XmlElement* connectionsXml = new XmlElement ("CONNECTIONS");
        
        for (auto connection : connections)
        {
            const int sourceId = connection->getSourcePin()->getNodeNumber();
            const int destId = connection->getDestinationPin()->getNodeNumber();
            
            if (graphIds.contains (sourceId) && graphIds.contains (destId))
                connectionsXml->addChildElement (getXmlFor (connection));
        }
        
        selection->addChildElement (connectionsXml);
    }
    
    return selection;
}

void GraphPanel::cutSelection()
{
    copySelection();
    
    for (auto selected : selector.getSelection())
        if (! selected->canBeDeleted())
            selected->setVisible (false);
    
    deleteSelection();
}

void GraphPanel::duplicateSelection()
{
    const String select (getSelectionAsString());
    
    if (select != lastDuplicated)
    {
        lastDuplicated = select;
        numConsecutiveDuplicate = 0;
    }
    
    auto consec = ++ numConsecutiveDuplicate;
    const Point<int> offset (20 * consec, 20 * consec);
    
    pasteAndAddOffset (select, offset);
}

void GraphPanel::paste()
{
    const String clipboardContent (clipboard->getClipboard());
    const int consecutive = clipboard->getAndIncNumConsecutivePaste();
    const Point<int> offset (20 * consecutive, 20 * consecutive);
    
    pasteAndAddOffset (clipboardContent, offset);
}

void GraphPanel::pasteAndAddOffset (const String& xmlDoc, Point<int> offset)
{
    Array<ConnectionInfo> info;
    auto pasted = pasteInternal (xmlDoc, true, info);
    
    for (auto comp : pasted)
        comp->setTopLeftPosition (comp->getX() + offset.getX(), comp->getY() + offset.getY());
}

void GraphPanel::pasteAtMousePos()
{
    const String clipboardContent (clipboard->getClipboard());
    const Point<int> mousePos = getMouseXYRelative();
    
    Array<ConnectionInfo> info;
    auto pasted = pasteInternal (clipboardContent, true, info);
    
    int leftMost = -1;
    int topMost = -1;
    
    for (auto comp : pasted)
    {
        if (leftMost < 0 || comp->getX() < leftMost)
            leftMost = comp->getX();
        
        if (topMost < 0 || comp->getY() < topMost)
            topMost = comp->getY();
    }
    
    Point<int> offset (mousePos.getX() - leftMost, mousePos.getY() - topMost);
    
    for (auto comp : pasted)
        comp->setTopLeftPosition (comp->getX() + offset.getX(), comp->getY() + offset.getY());
}

Array<GraphEmbeddedComponent*> GraphPanel::pasteInternal (const String& xmlDoc, bool undoable, Array<ConnectionInfo>& connectionInfo)
{
    XmlDocument doc (xmlDoc);
    ScopedPointer<XmlElement> elem = doc.getDocumentElement();
    
    Array<GraphEmbeddedComponent*> pastedComps;
    
    Array<std::pair<int, int>> idRemappers;
    
    if (! elem->hasTagName (getClipboardTagName()))
        return pastedComps;
    
    forEachXmlChildElementWithTagName(*(elem.get()), e, "Embedded")
    {
        if (auto comp = dynamic_cast<GraphEmbeddedComponent*>(createFromXml (*e, undoable)))
        {
            comp->setGraphId (++lastIdGiven);
            pastedComps.add (comp);
            idRemappers.addIfNotAlreadyThere (std::pair<int, int> (e->getIntAttribute("graphId"), comp->getGraphId()));
        }
    }
    
    if (pastedComps.size() > 0)
    {
        deselectAll();
        
        for (auto p : pastedComps)
            selector.addToSelection (p);
        
        auto getMappedId = [&](int originalId)
        {
            for (auto m : idRemappers)
                if (m.first == originalId)
                    return m.second;
            
            return -1;
        };
        
        forEachXmlChildElementWithTagName(*(elem->getChildByName("CONNECTIONS")), e, "Connection")
        {
            const int srcId = e->getIntAttribute ("sourceId");
            const int srcChan = e->getIntAttribute("sourceChannel");
            const int srcPlac = e->getIntAttribute ("sourcePlacement");
            const int dstId = e->getIntAttribute ("destId");
            const int dstChan =  e->getIntAttribute("destChannel");
            const int dstPlac = e->getIntAttribute ("destPlacement");
            
            createConnectionBetween (getMappedId (srcId),
                                     srcChan,
                                     static_cast<Pin::Placement>(srcPlac),
                                     getMappedId (dstId),
                                     dstChan,
                                     static_cast<Pin::Placement>(dstPlac),
                                     undoable);
            
            connectionInfo.add (connections.getLast()->getInfo());
        }
    }
    
    return pastedComps;
}

void GraphPanel::renameSelected()
{
    if (auto emb = dynamic_cast<GraphEmbeddedComponent*>(selector.getSelected(0)))
        emb->showNameEditor();
}

#include "Project.h"
void GraphPanel::prepareCommandTarget()
{
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    
    commandManager.registerAllCommandsForTarget (this);
}

ApplicationCommandTarget* GraphPanel::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

#include "CommandIDs.h"
void GraphPanel::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::cut,
        CommandIDs::copy,
        CommandIDs::paste,
        CommandIDs::duplicateSelection,
        CommandIDs::del,
        CommandIDs::selectAll,
        CommandIDs::deselectAll,
        CommandIDs::rename,
        CommandIDs::pasteAtMousePos
    };
    
    commands.addArray (ids, numElementsInArray (ids));
}

void GraphPanel::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    const String editCategory ("Edit");
    
    bool hasUniqueNonCopyableSelection = false;
    bool hasUniqueNonDeletableSelection = false;
    
    if (hasUniqueSelection())
        if (auto embedded = dynamic_cast<GraphEmbeddedComponent*>(selector.getSelected(0)))
        {
            if (! embedded->canBeCopied())
                hasUniqueNonCopyableSelection = true;
            
            if (! embedded->canBeDeleted())
                hasUniqueNonDeletableSelection = true;
        }
    
    switch (commandID)
    {
        case CommandIDs::cut :
            result.setInfo ("Cut", "Cut the selection to clipboard", editCategory, 0);
            result.addDefaultKeypress ('x', ModifierKeys::commandModifier);
            result.setActive (anythingSelected());
            break;
        case CommandIDs::copy :
            result.setInfo ("Copy", "Copy the selection to clipboard", editCategory, 0);
            result.addDefaultKeypress ('c', ModifierKeys::commandModifier);
            result.setActive (anythingSelected() && ! hasUniqueNonCopyableSelection);
            break;
        case CommandIDs::paste :
            result.setInfo ("Paste", "Paste from the clipboard", editCategory, 0);
            result.addDefaultKeypress ('v', ModifierKeys::commandModifier);
            result.setActive (canPaste());
            break;
        case CommandIDs::duplicateSelection :
            result.setInfo ("Duplicate", "Duplicate the selection", editCategory, 0);
            result.addDefaultKeypress ('d', ModifierKeys::commandModifier);
            result.setActive (anythingSelected() && ! hasUniqueNonCopyableSelection);
            break;
        case CommandIDs::del :
            result.setInfo ("Delete", "Delete the selection", editCategory, 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::noModifiers);
            result.addDefaultKeypress (KeyPress::deleteKey, ModifierKeys::noModifiers);
            result.setActive (anythingSelected() && ! hasUniqueNonDeletableSelection);
            break;
        case CommandIDs::selectAll :
            result.setInfo ("Select all", "Add all the content to the current selection", editCategory, 0);
            result.addDefaultKeypress ('a', ModifierKeys::commandModifier);
            result.setActive (anythingToSelect());
            break;
        case CommandIDs::deselectAll :
            result.setInfo ("Deselect all", "Clears the current selection", editCategory, 0);
            result.addDefaultKeypress ('a', ModifierKeys::commandModifier  | ModifierKeys::shiftModifier);
            result.setActive (anythingSelected());
            break;
        case CommandIDs::pasteAtMousePos :
            result.setInfo ("Paste", "Paste from the clipboard", editCategory, 0);
            result.setActive (canPaste());
            break;
        case CommandIDs::rename :
            result.setInfo ("Rename", "Renames the selected element", editCategory, 0);
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            result.setActive (hasUniqueSelection());
            break;
    }
}

bool GraphPanel::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::cut :
            cutSelection();
            break;
        case CommandIDs::copy :
            copySelection();
            break;
        case CommandIDs::paste :
            paste();
            break;
        case CommandIDs::duplicateSelection :
            duplicateSelection();
            break;
        case CommandIDs::del :
            deleteSelection();
            break;
        case CommandIDs::selectAll :
            selectAll();
            break;
        case CommandIDs::deselectAll :
            deselectAll();
            break;
        case CommandIDs::rename :
            renameSelected();
            break;
        case CommandIDs::pasteAtMousePos :
            pasteAtMousePos();
            break;
    }
    
    return true;
}

GraphEmbeddedComponent* GraphPanel::findComponentWithGraphId (int graphId)
{
    for (auto emb : embeddedComponents)
        if (emb->getGraphId() == graphId)
            return emb;
    
    return nullptr;
}

void GraphPanel::refreshGraphIds()
{
    int i = -1;
    
    for (auto emb : embeddedComponents)
        emb->setGraphId (++i);
    
    lastIdGiven = i;
}

XmlElement* GraphPanel::getStateAsXml()
{
    XmlElement* xml = new XmlElement ("GraphPanel");
    
    refreshGraphIds();
    
    Array<GraphEmbeddedComponent*> comps;
    Array<GraphEmbeddedComponent*> selection;
    
    for (auto emb : embeddedComponents)
    {
        comps.add (emb);
        
        if (emb->isSelected())
            selection.add (emb);
    }
    
    for (auto emb : comps)
        xml->addChildElement (getXmlFor (emb));
    
    sortConnections();
    
    for (int i = 0; i < connections.size(); ++i)
        xml->addChildElement (getXmlFor (connections[i]));
    
    XmlElement* selXml = new XmlElement ("SELECTION");
    
    for (auto s : selection)
    {
        auto sel = new XmlElement ("SELECTED");
        sel->setAttribute ("graphId", s->getGraphId());
        selXml->addChildElement (sel);
    }
    
    xml->addChildElement (selXml);
    
    return xml;
}

void GraphPanel::loadStateFromXml (const XmlElement& xml)
{
    clear();
    
    forEachXmlChildElementWithTagName (xml, e, "Embedded")
    {
        createFromXml (*e, false);
    }
    
    forEachXmlChildElementWithTagName (xml, e, "Connection")
    {
        createConnectionBetween (e->getIntAttribute ("sourceId"),
                                 e->getIntAttribute ("sourceChannel"),
                                 static_cast<Pin::Placement>(e->getIntAttribute ("sourcePlacement")),
                                 e->getIntAttribute ("destId"),
                                 e->getIntAttribute ("destChannel"),
                                 static_cast<Pin::Placement>(e->getIntAttribute ("destPlacement")),
                                 false);
    }
    
    if (auto selection = xml.getChildByName("SELECTION"))
    {
        forEachXmlChildElementWithTagName(*selection, e, "SELECTED")
        {
            selector.addToSelection (findComponentWithGraphId(e->getIntAttribute ("graphId")));
        }
    }
}

XmlElement* GraphPanel::getXmlFor (GraphEmbeddedComponent* comp)
{
    XmlElement* e = new XmlElement ("Embedded");
    
    if (comp == nullptr)
        return e;
    
    e->setAttribute ("name", comp->getPublicName());
    e->setAttribute ("graphId", comp->getGraphId());
    e->setAttribute ("uiComp", comp->getUICompTypeName());
    e->setAttribute ("xpos", comp->getX());
    e->setAttribute ("ypos", comp->getY());
    e->setAttribute ("width", comp->getWidth());
    e->setAttribute ("height", comp->getHeight());
    
    return e;
}

XmlElement* GraphPanel::getXmlFor (Connection* conn)
{
    XmlElement* e = new XmlElement ("Connection");
    
    e->setAttribute ("sourceId",        conn->getSourcePin()->getNodeNumber());
    e->setAttribute ("sourceChannel",   conn->getSourcePin()->getChannelNumber());
    e->setAttribute ("sourcePlacement", conn->getSourcePin()->getPlacement());
    e->setAttribute ("destId",          conn->getDestinationPin()->getNodeNumber());
    e->setAttribute ("destChannel",     conn->getDestinationPin()->getChannelNumber());
    e->setAttribute ("destPlacement",   conn->getDestinationPin()->getPlacement());
    
    return e;
}

//==============================================================================
ConnectionInfo::ConnectionInfo (Pin* sPin, Pin::Placement sPlacement,
                                int sIndex, GraphEmbeddedComponent *const sComp,
                                Pin* dPin, Pin::Placement dPlacement,
                                int dIndex, GraphEmbeddedComponent *const dComp,
                                WeakReference<GraphPanel::Connection> c)
:   sourcePin (sPin),
    sourcePlacement (sPlacement),
    sourceIndex (sIndex),
    sourceComp (sComp),
    destPin (dPin),
    destPlacement (dPlacement),
    destIndex (dIndex),
    destComp (dComp),
    connection (c)
{}
