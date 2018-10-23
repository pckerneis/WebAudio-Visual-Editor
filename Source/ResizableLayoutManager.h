/*
  ==============================================================================

    ResizableLayoutManager.h
    Created: 30 Oct 2017 1:56:07pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class ResizableLayoutManager;

/** \brief A draggable bar used by ResizableLayoutManager. */
class ResizableLayoutBar : public Component
{
public:
    ResizableLayoutBar (ResizableLayoutManager& resizableLayout, int index, bool vertical);
    ~ResizableLayoutBar() {}
    
    void paint (Graphics& g) override;
    
    void mouseDown (const MouseEvent&) override;
    void mouseDrag (const MouseEvent& e) override;
    
    void hasBeenMoved();
    
    void setItemIndex (int newIndex) { itemIndex = newIndex; }
    
    void enablementChanged() override;
    
private:
    friend class ResizableLayoutManager;
    
    void setVertical (bool shouldBeVertical);
    
    ResizableLayoutManager& layout;
    int itemIndex;
    bool isVertical;
    int mouseDownPos;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResizableLayoutBar)
};

//==============================================================================
/** \brief A component bounds management system that allow child components to be stacked horizontally
 * or verticaly and resized.
 *
 *  This is one of the base elements of the PanelTree.
 */
class ResizableLayoutManager
{
public:
    ResizableLayoutManager (Component& itemsContainer, bool vertical, int resizeBarWidth = 4, bool upwardAlignement = true);
    ~ResizableLayoutManager();
    
    int getNumItems() const { return items.size(); }
    
    void addItem (Component* comp, float minSize, float maxSize, float defaultSize, bool makeVisibleInContainer = false, int insertIndex = -1, bool shrinkOthers = false);
    void removeItem (Component* comp);
    
    void removeAllItems();
    
    void move (int from, int to);
    
    void layOutComponents (juce::Rectangle<int> bounds, bool resizeOtherDimensions);
    
    int getBarWidth() const { return barWidth; }
    void setBarWidth (int w) { barWidth = w; }
        
    int getItemCurrentPosition (int index);
    void setItemPosition (int index, int newPos);
    
    void setTotalSize (int newSize);
    
    void setKeepsProportionsWhenResized (bool shouldKeep) { keepProportions = shouldKeep; }
    
    // true by default
    void setAlignsUpward (bool shouldAlignUpward) { alignsUpward = shouldAlignUpward; }
    void setBarsEnablement (bool enabled);
    
    bool isVerticalLayout() const { return isVertical; }
    void setVertical (bool shouldBeVertical);
    
    //==============================================================================
    struct Item
    {
        Component* component;
        float minSize;
        float maxSize;
        float defaultSize;
    };
    
    Item* getItem (int index) const { return items[index]; }
    //==============================================================================
    
    XmlElement* getStateAsXml (String tagName);
    XmlElement* getAsXml (Item* item);
    
    void restoreFromXml (XmlElement* e);
    
private:
    int getAbsoluteSize (float size, int totalSize) const { return size > 0 ? size : size * (- totalSize); }
    
    Item* getItemWithComponent (Component* c) const;
    int getItemIndex (Item* item) const;
    
    bool componentIsAlreadyThere (Component* c) const;
    
    void updateBarIndexes();
    void updateLayout();
    
    void reduceSizesToFit (int fromItem, int toItem, int targetSize, bool upwards);
    void expandSizesToFill (int fromItem, int toItem, int targetSize, bool upwards);
    
    bool isVertical;

    float barWidth;
    Component& container;
    
    OwnedArray<Item> items;
    OwnedArray<ResizableLayoutBar> bars;
    
    int totalSize;
    
    bool keepProportions = false;
    bool alignsUpward;
    bool barsEnabled = true;
    bool active = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ResizableLayoutManager)
};
