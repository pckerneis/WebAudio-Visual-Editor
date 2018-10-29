/*
  ==============================================================================

    GraphDragger.h
    Created: 30 Aug 2017 7:28:28pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class GraphEmbeddedComponent;
class GraphSelector;

/** \brief A GraphEmbeddedComponents dragger which allows multi-selection drag. */
class GraphDragger
{
public:
    /** \brief Empty constructor. */
    GraphDragger (GraphSelector& sel) : selector (sel) {}
    /** \brief Empty destructor. */
    ~GraphDragger() {}
    
    /** \brief Stores the initial position for the current selection. */
    void startDragging (const MouseEvent& e);
    
    /** \brief Updates the position of the selection based on constraints. */
    void dragSelection (const MouseEvent& e);
    
    /** \brief Stores the initial position for the current selection. */
    void stopDragging (const MouseEvent& e);
    
    Point<int> getViewportOffsetSinceLastMouseDown() const;
    
private:
    /** \brief Determines the maximum drag offset for the current items. */
    void setConstrainer ();
    
    /** \brief Holds a pointer to a dragged GraphEmbeddedComponent as well as its initial position when startDragging was called. */
    struct DraggedComponent
    {
        /** \brief Constructor. */
        DraggedComponent (GraphEmbeddedComponent* comp, Point<int> initialPos);
        
        GraphEmbeddedComponent* component;
        const Point<int> initialPosition;
        Point<int> latestPositionShifted;
    };
    
    GraphSelector& selector;
    
    OwnedArray<DraggedComponent> itemsToDrag;
    
    int startX;
    int startY;
    
    int maxLeft;
    int maxUp;
    
    Point<int> viewportPositionOnMouseDown;
};
