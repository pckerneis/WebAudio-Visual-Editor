/*
  ==============================================================================

    ResizableLayoutManager.cpp
    Created: 30 Oct 2017 1:56:07pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "ResizableLayoutManager.h"

ResizableLayoutBar::ResizableLayoutBar (ResizableLayoutManager& layout_, int index, bool vertical) : layout (layout_),
itemIndex (index),
isVertical (vertical)
{
    setRepaintsOnMouseActivity (true);
    setMouseCursor (vertical ? MouseCursor::LeftRightResizeCursor
                    : MouseCursor::UpDownResizeCursor);
}

void ResizableLayoutBar::paint (Graphics& g)
{
    const bool mouseIsOver = isEnabled() && isMouseOver();
    const bool mouseIsDown = isEnabled() && isMouseButtonDown();
    
    getLookAndFeel().drawStretchableLayoutResizerBar (g,
                                                      getWidth(), getHeight(),
                                                      isVertical,
                                                      mouseIsOver,
                                                      mouseIsDown);
}

void ResizableLayoutBar::mouseDown (const MouseEvent&)
{
    mouseDownPos = layout.getItemCurrentPosition (itemIndex);
}

void ResizableLayoutBar::mouseDrag (const MouseEvent& e)
{
    const int desiredPos = mouseDownPos + (isVertical ? e.getDistanceFromDragStartX()
                                           : e.getDistanceFromDragStartY());
    
    if (isEnabled() && layout.getItemCurrentPosition (itemIndex) != desiredPos)
    {
        layout.setItemPosition (itemIndex, desiredPos);
        hasBeenMoved();
    }
}

void ResizableLayoutBar::hasBeenMoved()
{
    if (Component* parent = getParentComponent())
        parent->resized();
}

void ResizableLayoutBar::enablementChanged()
{
    if (isEnabled())
        setMouseCursor (isVertical ? MouseCursor::LeftRightResizeCursor : MouseCursor::UpDownResizeCursor);
    else
        setMouseCursor (MouseCursor::NormalCursor);
}

void ResizableLayoutBar::setVertical (bool shouldBeVertical)
{
    isVertical = shouldBeVertical;
    
    setMouseCursor (isVertical ? MouseCursor::LeftRightResizeCursor
                    : MouseCursor::UpDownResizeCursor);
    
    enablementChanged();
}

//==============================================================================

ResizableLayoutManager::ResizableLayoutManager (Component& itemsContainer, bool vertical, int resizeBarWidth, bool upwardAlignement) : isVertical (vertical), barWidth (resizeBarWidth), container (itemsContainer), alignsUpward (upwardAlignement)
{
}

ResizableLayoutManager::~ResizableLayoutManager()
{
    for (auto b : bars)
            container.removeChildComponent (b);
}

void ResizableLayoutManager::addItem (Component* comp, float minSize, float maxSize, float defaultSize, bool makeVisibleInContainer, int insertIndex, bool shrinkOthers)
{
    if (! componentIsAlreadyThere (comp))
    {
        if (items.size() > 0)
        {
            const int barIndex = items.size();
            
            auto bar = new ResizableLayoutBar (*this, barIndex, !isVertical);
            bars.add (bar);
            
            items.add (new Item ({ bar, barWidth, barWidth, barWidth }));
            
            container.addAndMakeVisible (bar);
            bar->toBack();
        }
        
        if (active && shrinkOthers && totalSize > 0)
        {
            float newItemSize = getAbsoluteSize(defaultSize, totalSize);
            
            float shrinkRatio = 1.0 - newItemSize / (float)totalSize;
            
            for (auto item : items)
            {
                if (auto bar = dynamic_cast<ResizableLayoutBar*>(item->component))
                {}
                else
                {
                    const float minSize = getAbsoluteSize (item->minSize, totalSize);
                    const float maxSize = getAbsoluteSize (item->maxSize, totalSize);
                    
                    item->defaultSize = jlimit (minSize, maxSize, item->defaultSize * shrinkRatio);
                }
            }
        }
        
        items.add (new Item ({ comp, minSize, maxSize, defaultSize }));
        
        if (makeVisibleInContainer)
            container.addAndMakeVisible (comp);
        
        if (insertIndex >= 0 && insertIndex < items.size())
            items.move(items.size() - 1, insertIndex);
        
        updateBarIndexes();
        
        updateLayout();
    }
}

void ResizableLayoutManager::removeItem (Component* comp)
{
    if (auto item = getItemWithComponent (comp))
    {
        const int compSize = isVertical ? comp->getHeight() : comp->getWidth();
        int indexToRemove = getItemIndex (item);
        const int numItems = items.size();
        
        const bool isLastItem = indexToRemove == numItems - 1;
        
        // Remove the bar located after this element if not the last one
        if (!isLastItem && items.size() > 1)
        {
            auto bar = dynamic_cast<ResizableLayoutBar*>(items.getUnchecked(indexToRemove + 1)->component);
            container.removeChildComponent (bar);
            bars.removeObject (bar);
            items.remove (indexToRemove + 1);
        }
        
        container.removeChildComponent(item->component);
        items.removeObject(item);
        
        
        // If the last one, remove the last element which should be a bar
        if (isLastItem && items.size() > 0)
        {
            auto item = items.getLast();
            
            for (int i = 0; i < bars.size(); ++i)
            {
                if (bars.getUnchecked(i) == item->component)
                {
                    bars.remove(i);
                    container.removeChildComponent (item->component);
                    
                    break;
                }
                
                if (i == bars.size() - 1)
                    jassertfalse;
            }
            
            items.removeObject (item);
        }
        
        updateBarIndexes();
        
        if (active)
        {
            for (auto i : items)
            {
                const int oldSize = getAbsoluteSize (i->defaultSize, totalSize);
                i->defaultSize = oldSize + (compSize / (items.size() - bars.size()));
            }
        
            updateLayout();
        }
    }
}

void ResizableLayoutManager::removeAllItems()
{
    for (auto i : items)
        container.removeChildComponent(i->component);
    
    items.clear();
    bars.clear();
}

void ResizableLayoutManager::move (int from, int to)
{
    if (from == to)
        return;
    
    items.move(from, to);
    
    if (from < to)
        items.move(from, to - 1);
    else
        items.move(from, to + 1);
    
    updateBarIndexes();
    updateLayout();
}

void ResizableLayoutManager::layOutComponents (juce::Rectangle<int> bounds, bool resizeOtherDimensions)
{
    resizeOtherDimensions = true;
    
    if (bounds.isEmpty())
        return;
    
    if (items.size() == 0)
        return;
    
    setTotalSize (isVertical ? bounds.getHeight() : bounds.getWidth());
    
     if (!active)
         return;
    
    for (auto item : items)
    {
        const int minSize = getAbsoluteSize (item->minSize, totalSize);
        const int maxSize = getAbsoluteSize (item->maxSize, totalSize);
        const int defaultSize = getAbsoluteSize (item->defaultSize, totalSize);
        const int clippedSize = jlimit (minSize, maxSize, defaultSize);
        
        item->defaultSize = clippedSize;
    }
    
    reduceSizesToFit (0, items.size(), totalSize, alignsUpward);
    expandSizesToFill (0, items.size(), totalSize, alignsUpward);
    
    int newPos = isVertical ? bounds.getY() : bounds.getX();
    
    for (int i = 0; i < items.size(); ++i)
    {
        auto item = items.getUnchecked(i);
        
        int newWidth;
        int newHeight;
        int newX;
        int newY;
        
        if (isVertical)
        {
            newX = bounds.getX();
            newY = newPos;
            newWidth = resizeOtherDimensions ? bounds.getWidth() : item->component->getWidth();
            
            if (i == items.size() - 1)
                newHeight = (totalSize - newPos) + bounds.getY();
            else
                newHeight = getAbsoluteSize(item->defaultSize, totalSize);
            
            newPos += newHeight;
        }
        else
        {
            newX = newPos;
            newY = bounds.getY();
            
            if (i == items.size() - 1)
                newWidth = (totalSize - newPos) + bounds.getX();
            else
                newWidth = getAbsoluteSize(item->defaultSize, totalSize);
            
            newHeight = resizeOtherDimensions ? bounds.getHeight() : item->component->getHeight();
            
            newPos += newWidth;
        }
        
        item->component->setBounds (newX, newY, newWidth, newHeight);
    }
}

int ResizableLayoutManager::getItemCurrentPosition (int index)
{
    if (index >= items.size())
        return -1;
    
    int position = 0;
    
    for (int i = 0; i < index; ++i)
        position += getAbsoluteSize(items.getUnchecked(i)->defaultSize, totalSize);
    
    return position;
}

void ResizableLayoutManager::setItemPosition (int index, int newPos)
{
    if (index >= items.size() || !active)
        return;
    
    // Determine the max position
    int maxNewPos = totalSize;
    for (int i = index; i < items.size(); ++i)
        maxNewPos -= getAbsoluteSize (items.getUnchecked(i)->minSize, totalSize);
    
    newPos = jmin (newPos, maxNewPos);
    
    int deltaToPreviousPos = newPos;
    
    for (int i = 0; i < index; ++i)
    {
        auto item = items.getUnchecked (i);
        
        deltaToPreviousPos -= getAbsoluteSize (item->defaultSize, totalSize);
        
        if (i == index - 1)
        {
            int newSize = getAbsoluteSize (item->defaultSize, totalSize) + deltaToPreviousPos;
            newSize = jmin (newSize, getAbsoluteSize (item->maxSize, totalSize));
            newSize = jmax (newSize, getAbsoluteSize (item->minSize, totalSize));
            
            item->defaultSize = item->defaultSize > 0 ? newSize : newSize / (-totalSize);
            
            if (deltaToPreviousPos > 0)
            {
                reduceSizesToFit (index, items.size(), totalSize - newPos, true);
                expandSizesToFill (0, index, newPos, false);
            }
            else
            {
                expandSizesToFill (index, items.size(), totalSize - newPos, true);
                reduceSizesToFit (0, index, newPos, false);
            }
        }
    }
}

void ResizableLayoutManager::setTotalSize (int newSize)
{
    const int oldSize = totalSize;
    
    totalSize = newSize;
    
    if (keepProportions && oldSize != 0 && oldSize != newSize)
    {
        const int numItemsToResize = items.size() - bars.size();
        const int delta = newSize - oldSize;
        
        auto contentSize = totalSize;
        
        for (int i = 0; i < items.size(); ++i)
        {
            auto item = items.getUnchecked(i);
            
            if (auto bar = dynamic_cast<ResizableLayoutBar*> (item->component))
                contentSize -= barWidth;
            else
            {
                if (i == items.size() - 1)
                    item->defaultSize = contentSize;
                else
                {
                    int deltaForThisItem = roundFloatToInt((float)delta / (float)numItemsToResize);
                    int target = getAbsoluteSize (item->defaultSize, oldSize) + deltaForThisItem;
            
                    target = jlimit (getAbsoluteSize(item->minSize, totalSize),
                                     getAbsoluteSize(item->maxSize, totalSize),
                                     target);
            
                
                    item->defaultSize = target;
                
                    contentSize -= target;
                }
            }
        }
    }
    
    if (totalSize > 0)
        active = true;
    else
        active = false;
}

void ResizableLayoutManager::setBarsEnablement (bool enabled)
{
    for (auto b : bars)
    {
        b->setEnabled (enabled);
        b->repaint();
    }
    
    barsEnabled = enabled;
}

void ResizableLayoutManager::setVertical (bool shouldBeVertical)
{
    isVertical = shouldBeVertical;
    
    for (auto bar : bars)
    bar->setVertical (! isVertical);
    
    container.resized();
}

bool ResizableLayoutManager::componentIsAlreadyThere (Component* c) const
{
    return getItemWithComponent (c) != nullptr;
}

ResizableLayoutManager::Item* ResizableLayoutManager::getItemWithComponent (Component* c) const
{
    for (auto item : items)
        if (item->component == c)
            return item;
    
    return nullptr;
}

int ResizableLayoutManager::getItemIndex (Item* item) const
{
    for (int i = items.size(); --i >= 0;)
        if (items.getUnchecked(i) == item)
            return i;
    
    return -1;
}

void ResizableLayoutManager::updateBarIndexes()
{
    for (int i = 0; i < items.size(); ++i)
        if (auto bar = dynamic_cast<ResizableLayoutBar*>(items[i]->component))
            bar->setItemIndex (i);
}

void ResizableLayoutManager::updateLayout()
{
    container.resized();
}

void ResizableLayoutManager::reduceSizesToFit (int fromItem, int toItem, int targetSize, bool upwards)
{
    jassert (fromItem >= 0 && fromItem <= items.size()
             && toItem >= 0 && toItem <= items.size()
             && fromItem < toItem);
    
    int totalContentSize = 0;
    
    for (int i = fromItem; i < toItem; ++i)
        totalContentSize += getAbsoluteSize(items.getUnchecked (i)->defaultSize, totalSize);
    
    if (totalContentSize > targetSize)
    {
        std::function<bool (Item*, int&)> reduceSize = [this](Item* item, int& qtyToRemove)
        {
            const int canRemove = getAbsoluteSize (item->defaultSize, totalSize) - getAbsoluteSize (item->minSize, totalSize);
            
            if (canRemove > 0)
            {
                const int toRemoveForThisItem = jmin (qtyToRemove, canRemove);
                
                qtyToRemove -= toRemoveForThisItem;
                
                if (item->defaultSize > 0)
                    item->defaultSize -= toRemoveForThisItem;
                else
                    item->defaultSize = float(getAbsoluteSize (item->defaultSize, totalSize) - toRemoveForThisItem) / float(-totalSize);
            }
            
            if (qtyToRemove <= 0)
                return false;
            
            return true;
        };
        
        int toRemove = totalContentSize - targetSize;
        
        if (upwards)
        {
            for (int i = fromItem; i < toItem; ++i)
                if (!reduceSize (items.getUnchecked (i), toRemove))
                    break;
        }
        else
            for (int i = toItem; --i >= fromItem;)
                if (!reduceSize (items.getUnchecked (i), toRemove))
                    break;
    }
}

void ResizableLayoutManager::expandSizesToFill (int fromItem, int toItem, int targetSize, bool upwards)
{
    
     jassert (fromItem >= 0 && fromItem <= items.size()
              && toItem >= 0 && toItem <= items.size()
              && fromItem < toItem);
     
    int totalContentSize = 0;
    
    for (int i = fromItem; i < toItem; ++i)
        totalContentSize += getAbsoluteSize(items.getUnchecked (i)->defaultSize, totalSize);
    
    if (totalContentSize < targetSize)
    {
        std::function<bool (Item*, int&)> expandSize = [this](Item* item, int& qtyToAdd)
        {
            const int canAdd = getAbsoluteSize (item->maxSize, totalSize) - getAbsoluteSize (item->defaultSize, totalSize);
            
            if (canAdd > 0)
            {
                int toAddForThisItem = jmin (qtyToAdd, canAdd);
                
                qtyToAdd -= toAddForThisItem;
                
                if (item->defaultSize > 0)
                    item->defaultSize += toAddForThisItem;
                else
                    item->defaultSize = float(getAbsoluteSize (item->defaultSize, totalSize) + toAddForThisItem) / float(-totalSize);
            }
            
            if (qtyToAdd <= 0)
                return false;
            
            return true;
        };

        int toAdd = targetSize - totalContentSize;
        
        if (upwards)
        {
            for (int i = fromItem; i < toItem; ++i)
                if (! expandSize (items.getUnchecked (i), toAdd))
                    break;
        }
        else
            for (int i = toItem; --i >= fromItem;)
                if (! expandSize (items.getUnchecked (i), toAdd))
                    break;
    }
}

XmlElement* ResizableLayoutManager::getStateAsXml (String tagName)
{
    updateBarIndexes();
    
    XmlElement* e = new XmlElement (tagName);
    
    e->setAttribute ("barsEnabled", barsEnabled);
    e->setAttribute ("isVertical", isVertical);
    
    for (int i = 0; i < items.size(); ++i)
        e->addChildElement (getAsXml (items.getUnchecked(i)));
    
    return e;
}

XmlElement* ResizableLayoutManager::getAsXml (Item* item)
{
    XmlElement* e = new XmlElement ("ITEM");
    
    e->setAttribute ("keepProportions", keepProportions);
    
    if (item != nullptr)
    {
        e->setAttribute ("minSize", item->minSize);
        e->setAttribute ("maxSize", item->maxSize);
        e->setAttribute ("defaultSize", item->defaultSize);
    }
    
    return e;
}

void ResizableLayoutManager::restoreFromXml (XmlElement* e)
{
    if (e == nullptr)
        return;
    
    int i = 0;
    
    setVertical (e->getBoolAttribute ("isVertical"));
    
    forEachXmlChildElementWithTagName (*e, c, "ITEM")
    {
        if (i < items.size())
        {
            items.getUnchecked(i)->minSize = c->getDoubleAttribute ("minSize");
            items.getUnchecked(i)->maxSize = c->getDoubleAttribute ("maxSize");
            items.getUnchecked(i)->defaultSize = c->getDoubleAttribute ("defaultSize");
        }
        
        ++i;
    }
    
    setBarsEnablement (e->getBoolAttribute ("barsEnabled"));
    
    keepProportions = false;
    updateLayout();
    keepProportions = e->getBoolAttribute ("keepProportions");
}
