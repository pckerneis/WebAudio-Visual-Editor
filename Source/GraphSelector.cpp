/*
  ==============================================================================

    GraphSelector.cpp
    Created: 28 Jul 2017 10:23:00pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "GraphSelector.h"

//==============================================================================
GraphSelectableItem::GraphSelectableItem()
{
}

GraphSelectableItem::~GraphSelectableItem()
{
    if (graphSelector != nullptr)
        graphSelector->removeFromSelection (this);
}

void GraphSelectableItem::setSelected (bool shouldBeSelected)
{
    if (isCurrentlySelected != shouldBeSelected)
    {
        isCurrentlySelected = shouldBeSelected;
        repaint();
    }
}

void GraphSelectableItem::setSelectedBasedOnModifiers (const ModifierKeys& mods)
{
    if (graphSelector != nullptr)
        graphSelector->addToSelectionBasedOnModifiers (this, mods);
}

void GraphSelectableItem::setHighlighted (bool shouldBeHighlighted)
{
    if (isCurrentlyHighlighted != shouldBeHighlighted)
    {
        isCurrentlyHighlighted = shouldBeHighlighted;
        repaint();
    }
}

void GraphSelectableItem::mouseEnter (const MouseEvent& e)
{
    isCurrentlyHighlighted = true;
    repaint();
}

void GraphSelectableItem::mouseExit (const MouseEvent& e)
{
    isCurrentlyHighlighted = false;
    repaint();
}

void GraphSelectableItem::mouseDown (const MouseEvent& e)
{
    if (graphSelector != nullptr)
        triggerActionOnNextMouseUp = graphSelector->selectedItems.addToSelectionOnMouseDown (this, e.mods);
    
    if (auto thisComp = dynamic_cast<Component*>(this))
        thisComp->toFront(true);
}

void GraphSelectableItem::mouseUp (const MouseEvent& e)
{
    if (graphSelector != nullptr)
        graphSelector->selectedItems.addToSelectionOnMouseUp (this,
                                                              e.mods,
                                                              e.mouseWasDraggedSinceMouseDown(),
                                                              triggerActionOnNextMouseUp);
}

//==============================================================================

#include "GraphPanel.h"
#include "GraphDragger.h"

GraphSelector::GraphSelector (GraphPanel& graph) : graphPanel (graph), selectedItems (*this)
{
    dragger = new GraphDragger (*this);
}

GraphSelector::~GraphSelector()
{
    masterReference.clear();
}

void GraphSelector::sort()
{
    SelectedComparator compar;
    auto arr = selectedItems.getItemArray();
    arr.sort (compar);
    
    selectedItems.deselectAll();
    for (auto sel : arr)
    addToSelection (sel);
}

void GraphSelector::selectArray (const Array<GraphSelectableItem*>& objectsToSelect)
{
    deselectAll();
    
    for (auto o : objectsToSelect)
    addToSelection (o);
}

const Array<GraphSelectableItem*>& GraphSelector::getSelection()
{
    return selectedItems.getItemArray();
}

void GraphSelector::addToSelectionBasedOnModifiers (GraphSelectableItem* item, ModifierKeys modifiers)
{
    selectedItems.addToSelectionBasedOnModifiers (item, modifiers);
}

void GraphSelector::selectionChanged()
{
    graphPanel.selectionChanged();
}

void GraphSelector::setUniqueSelection (GraphSelectableItem* item)
{
    deselectAll();
    selectedItems.addToSelection (item);
}

void GraphSelector::addToSelection (GraphSelectableItem* item)
{
    if (!selectedItems.getItemArray().contains (item))
        selectedItems.addToSelection (item);
}

void GraphSelector::removeFromSelection (GraphSelectableItem* item)
{
    if (selectedItems.getItemArray().contains (item))
        selectedItems.deselect (item);
}

void GraphSelector::deselectAll()
{
    selectedItems.deselectAll();
}

bool GraphSelector::deleteSelection()
{
    if (selectedItems.getNumSelected() == 0)
        return false;
    
    bool allWereDeleted = true;
    
    graphPanel.getUndoManager().beginNewTransaction();
    
    for (int i = selectedItems.getNumSelected(); --i >= 0;)
    {
        auto item = selectedItems.getItemArray().getUnchecked(i);
        
        if (item->canBeDeleted())
            item->deleteSelectedItem();
        else
            allWereDeleted = false;
    }
    
    if (!allWereDeleted)
        return false;
    
    selectedItems.deselectAll();
    
    return true;
}

void GraphSelector::findLassoItemsInArea(Array<GraphSelectableItem*>& itemsFound, const juce::Rectangle<int>& area)
{
    for (auto comp : graphPanel.getEmbeddedComponents())
        if (!area.getIntersection(comp->getLassoSelectionBounds()).isEmpty() && comp->canBeSelectedWithLasso())
            itemsFound.add (comp);
}

SelectedItemSet<GraphSelectableItem*>& GraphSelector::getLassoSelection()
{
    return selectedItems;
}

//==============================================================================
#include "GraphEmbeddedComponent.h"
int GraphSelector::SelectedComparator::compareElements (GraphSelectableItem* first, GraphSelectableItem* second)
{
    auto firstEmb = dynamic_cast<GraphEmbeddedComponent*>(first);
    auto secondEmb = dynamic_cast<GraphEmbeddedComponent*>(second);
    
    if (firstEmb == nullptr || secondEmb == nullptr)
        return 0;
    
    return firstEmb->getGraphId() - secondEmb->getGraphId();
}

//==============================================================================
GraphSelector::SelectedSet::SelectedSet (GraphSelector& sel) : selector (sel) {}
GraphSelector::SelectedSet::~SelectedSet() {}

void GraphSelector::SelectedSet::itemSelected (GraphSelectableItem* item)
{
    if (!item->isCurrentlySelected)
    {
        item->isCurrentlySelected = true;
        item->repaint();
        
        selector.selectionChanged();
    }
}

void GraphSelector::SelectedSet::itemDeselected (GraphSelectableItem* item)
{
    if (item->isCurrentlySelected)
    {
        item->isCurrentlySelected = false;
        item->repaint();
        
        selector.selectionChanged();
    }
}
