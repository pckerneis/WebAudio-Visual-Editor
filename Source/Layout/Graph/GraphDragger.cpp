/*
  ==============================================================================

    GraphDragger.cpp
    Created: 30 Aug 2017 7:28:28pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "GraphDragger.h"

#include "GraphEmbeddedComponent.h"
#include "GraphSelector.h"
#include "GraphPanel.h"

void GraphDragger::startDragging (const MouseEvent& e)
{
    itemsToDrag.clear();
    
    for (int i = 0; i < selector.getNumSelected(); ++i)
    {
        if (auto comp = dynamic_cast<GraphEmbeddedComponent*>(selector.getSelected(i)))
        {
            comp->setPositionOnLastMouseDown();
            itemsToDrag.add (new DraggedComponent (comp, comp->getPosition()));
        }
    }
    
    if (itemsToDrag.size() > 0)
        setConstrainer();
    
    if (auto vp = selector.getGraphPanel().findParentComponentOfClass<Viewport>())
        viewportPositionOnMouseDown = vp->getViewPosition();
}

void GraphDragger::dragSelection (const MouseEvent& e)
{
    const auto vpOffset = getViewportOffsetSinceLastMouseDown();
    
    int offsetX = e.getOffsetFromDragStart().getX() + vpOffset.x;
    offsetX = jmax (-maxLeft, offsetX);
    
    int offsetY = e.getOffsetFromDragStart().getY() + vpOffset.y;
    offsetY = jmax (-maxUp, offsetY);
    
    const Point<int> offset (offsetX, offsetY);
    
    if (e.mods.isShiftDown())
    {
        for (auto item : itemsToDrag)
        {
            if (item->component == e.originalComponent)
                e.originalComponent->setTopLeftPosition (offset + item->initialPosition);
            else
            {
                if (e.mods.isCtrlDown() || e.mods.isCommandDown())
                    item->component->setTopLeftPosition (item->initialPosition);
                
                item->latestPositionShifted = item->component->getPosition() - offset;
            }
        }
        
        return;
    }
    
    for (auto item : itemsToDrag)
    {
        if (auto comp = item->component)
        {
            comp->setTopLeftPosition (offset + item->latestPositionShifted);
            comp->wasDragged (e, false, itemsToDrag.size() == 1);
        }
    }
}


void GraphDragger::stopDragging (const MouseEvent& e)
{
    selector.getGraphPanel().getUndoManager().beginNewTransaction();
    
    if (e.mouseWasDraggedSinceMouseDown())
        for (auto item : itemsToDrag)
            if (auto comp = item->component)
                comp->wasDragged (e, true, itemsToDrag.size() == 1);
}

Point<int> GraphDragger::getViewportOffsetSinceLastMouseDown() const
{
    if (auto vp = selector.getGraphPanel().findParentComponentOfClass<Viewport>())
        return  vp->getViewPosition() - viewportPositionOnMouseDown;
    
    return Point<int>();
}

void GraphDragger::setConstrainer()
{
    maxLeft = -1;
    maxUp = -1;
    
    for (auto item : itemsToDrag)
    {
        if (maxLeft < 0 || item->initialPosition.getX() < maxLeft)
            maxLeft = item->initialPosition.getX();
        
        if (maxUp < 0 || item->component->getY() < maxUp)
            maxUp = item->initialPosition.getY();
    }
}



GraphDragger::DraggedComponent::DraggedComponent (GraphEmbeddedComponent* comp, Point<int> initialPos) : component (comp), initialPosition (initialPos), latestPositionShifted (initialPosition)
{
}
