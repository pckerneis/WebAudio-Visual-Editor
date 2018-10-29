/*
  ==============================================================================

    OrchestraSelector.h
    Created: 28 Jul 2017 10:23:00pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GraphSelector;
class GraphPanel;

// ==============================================================================
/** \brief A selectable item in the GraphPanel.
 *
 *  Classes that inherit from this can use isSelected() and isHighlighted() to adapt their appearance.
 **/
class GraphSelectableItem : public Component
{
public:
    /** \brief Constructor. */
    GraphSelectableItem();
    
    /** \brief Destructor : removes this from selection. */
    virtual ~GraphSelectableItem();
    
    /** \brief Is this item selected ? */
    bool isSelected() const { return isCurrentlySelected; }
    /** \brief Marks this item as selected. */
    void setSelected (bool shouldBeSelected);
    /** \brief Use key modifiers to determine if this should be added to selection after a click */
    void setSelectedBasedOnModifiers (const ModifierKeys& mods);
    
    /** \brief Is this item highlighted ? */
    bool isHighlighted() const { return isCurrentlyHighlighted; }
    /** \brief Marks as highlighted (called when mouse enters). */
    void setHighlighted (bool shouldBeHighlighted);
    
    /** \brief Sets as highlighted when mouse enters. */
    void mouseEnter (const MouseEvent& e) override;
    /** \brief Sets as highlighted when mouse exits. */
    void mouseExit (const MouseEvent& e) override;
    /** \brief Add/remove to selection based on the mod keys. */
    void mouseDown (const MouseEvent& e) override;
    /** \brief Add/remove to selection based on the mod keys. */
    void mouseUp (const MouseEvent& e) override;
    
    /** \brief Reacts to delete command. */
    virtual void deleteSelectedItem() {}
    
    bool canBeDeleted() const { return thisCanBeDeleted; }
    void setCanBeDeleted (bool canBeDeleted) { thisCanBeDeleted = canBeDeleted; }
    
    /** \brief Allow custom behaviors for lasso selection. */
    virtual juce::Rectangle<int> getLassoSelectionBounds() const { return getBounds(); }
    
protected:
    /** \brief Returns a pointer (possibly nullptr if none was set) to the selector. */
    GraphSelector* getSelector() const { return graphSelector; }
    
    /** \brief Used on initialisation in the sub-classes constructors or when the item is added. */
    void setSelector (GraphSelector* selector) { graphSelector = selector; }
    
private:
    friend class GraphPanel;
    friend class GraphSelector;
    
    WeakReference<GraphSelector> graphSelector;
    
    bool isCurrentlySelected = false;
    bool isCurrentlyHighlighted = false;
    bool triggerActionOnNextMouseUp = false;
    
    bool thisCanBeDeleted = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphSelectableItem)
};

// ==============================================================================
class GraphDragger;

/** \brief A selection manager attached to a GraphPanel.
 *
 *  This has a embedded GraphSelectableItem class from which should inherit every elements shown it the Graph that can be selected.
 *  The selector then manages mouse events and modifiers keys to allow multiple selection, grouped commands on special keys such as delete, etc...
 */
class GraphSelector : public MouseListener, public LassoSource<GraphSelectableItem*>
{
public:
    /** \brief Constructor. */
    GraphSelector (GraphPanel& graph);
    /** \brief Destructor. */
    ~GraphSelector();

    /** \brief Returns the number of selected elements. */
    int getNumSelected() const { return selectedItems.getNumSelected(); }
    /** \brief Returns a selected element by index. */
    GraphSelectableItem* getSelected (int index) { return selectedItems.getItemArray()[index]; }
    /** \brief Returns the current selection. */
    const Array<GraphSelectableItem*>& getSelection();
    
    /** \brief Sorts the current selection by position. */
    void sort();
        
    /** \brief Add item to selection based on keyboard modifiers. */
    void addToSelectionBasedOnModifiers (GraphSelectableItem* item, ModifierKeys modifiers);
    
    /** \brief Sets an array of items as the current selection. */
    void selectArray (const Array<GraphSelectableItem*>& objectsToSelect);
    /** \brief Deselects all and add item to selection. */
    void setUniqueSelection (GraphSelectableItem* item);
    /** \brief Add to selection array and mark as selected. */
    void addToSelection (GraphSelectableItem* item);
    /** \brief Set item to non selected and remove from selection array. */
    void removeFromSelection (GraphSelectableItem* item);
    
    /** \brief Set all selected to non selected and clears the selection array. */
    void deselectAll();
    /** \brief Tries to delete the components selected. */
    bool deleteSelection();
    
    /** \brief Returns a reference to the graph. */
    GraphPanel& getGraphPanel() const { return graphPanel; }
    
    /** \brief Returns a pointer to the GraphDragger. */
    GraphDragger* getDragger() const { return dragger; }
    
    void findLassoItemsInArea(Array<GraphSelectableItem*>& itemsFound, const juce::Rectangle<int>& area) override;
    
    SelectedItemSet<GraphSelectableItem*>& getLassoSelection() override;

    void selectionChanged();
    
    // ==============================================================================
    class SelectedSet : public SelectedItemSet<GraphSelectableItem*>
    {
    public:
        SelectedSet (GraphSelector& sel);
        ~SelectedSet();
        
        void itemSelected (GraphSelectableItem* item) override;
        void itemDeselected (GraphSelectableItem* item) override;
        
    private:
        
        GraphSelector& selector;
    };
    
    // ==============================================================================
    struct SelectedComparator
    {
        int compareElements (GraphSelectableItem* first, GraphSelectableItem* second);
    };
    
    // ==============================================================================
private:
    WeakReference<GraphSelector>::Master masterReference;
    friend class WeakReference<GraphSelector>;
    
    friend class GraphSelectableItem;
    ScopedPointer<GraphDragger> dragger;
    
    GraphPanel& graphPanel;
    
    SelectedSet selectedItems;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphSelector)
};
