/*
  ==============================================================================

    GraphEmbeddedComponent.h
    Created: 22 Aug 2017 1:42:14am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "GraphSelector.h"
#include "../../Application/PopupMenuHandler.h"

class GraphEmbeddedComponent
: public GraphSelectableItem, public PopupMenuHandler::PopupMenuHandlerClient, public ReferenceCountedObject
{
public:
    using Ptr = ReferenceCountedObjectPtr<GraphEmbeddedComponent>;
    
    GraphEmbeddedComponent();
    ~GraphEmbeddedComponent();
    
    GraphPanel* getParentGraph () const;
    void setParentGraph (GraphPanel* parent);
    
    String getPublicName() const { return publicName; }
    
    virtual void setPublicName (String newName)
    {
        publicName = newName;
        refreshUI();
    }
    virtual void showNameEditor() {}
    
    int getGraphId() const { return graphId; }
    void setGraphId (int newGraphId);
    
    void paint(Graphics &g) override;
    void resized() override;
    void moved() override { graphItemChanged(); }
    
    Colour getBackgroundColour() const { return backgroundColour; }
    virtual void setBackgroundColour (Colour newColour)
    {
        backgroundColour = newColour;
        repaint();
    }
    
    /* 
     This is used to show enabled commmands : copy and duplicate won't be shown.
     When trying to cut a non copyable item, it will make it invisible. Sub-classes
     should then use their own logic to make it visible again in GraphPanel::createFromXml
     */
    bool canBeCopied() const { return thisCanBeCopied; }
    void setCanBeCopied (bool canBeCopied) { thisCanBeCopied = canBeCopied; }
    
    bool canBeRenamed() const { return thisCanBeRenamed; }
    void setCanBeRenamed (bool canBeRenamed) { thisCanBeRenamed = canBeRenamed; }
    
    virtual String getUICompTypeName() const = 0;
    virtual void paintUI (Graphics &g, juce::Rectangle<int> contentBounds) {}
    virtual void resizeUI (juce::Rectangle<int> contentBounds) {}
    virtual void refreshUI() {}
    
    virtual bool canBeSelectedWithLasso() const { return true; }
    
    void mouseDown(const MouseEvent& e) override;
    void mouseDrag (const MouseEvent& e) override;
    void mouseUp (const MouseEvent &e) override;
    virtual void wasDragged (const MouseEvent& e, bool dragIsOver, bool isUniqueSelection) {}
    
    // Reacts to a delete command. Implements GraphSelectableItem().
    void deleteSelectedItem() override;
    
    virtual void wasConnected() {}
    virtual void wasDisconnected() {}
    virtual void wasRemovedFromGraph() {}
    
    //==============================================================================
    class Pin    : public Component
    {
    public:
        enum Placement
        {
            PinOnTop,
            PinOnRight,
            PinOnBottom,
            PinOnLeft
        };
        
        Pin (GraphEmbeddedComponent& ownerComp, Placement p, int channel);
        ~Pin();
        
        void paint (Graphics &g) override;
        
        GraphEmbeddedComponent* getOwnerComponent() const { return &owner; }
        
        void mouseDown (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        
        bool isAnInput() const { return placement == Placement::PinOnLeft || placement == Placement::PinOnTop; }
        Placement getPlacement() const { return placement; }
        
        int getNodeNumber() const { return owner.getGraphId(); }
        
        void setChannelNumber (int newChannel) { channelNumber = newChannel; }
        int getChannelNumber() const { return channelNumber; }
        
        bool isConnected() const { return numConnections > 0; }
        void incNumConnections() { ++numConnections; }
        void decNumConnections() { --numConnections; }
        
        void enablementChanged() override
        {
            setMouseCursor (isEnabled() ? MouseCursor::UpDownLeftRightResizeCursor : MouseCursor::NormalCursor);
            repaint();
        }
        
    private:
        friend class GraphEmbeddedComponent;
        friend class GraphPanel;
        
        const int pinWidth = 8;
        
        GraphEmbeddedComponent& owner;
        
        Placement placement;
        int channelNumber;
        
        int numConnections = 0;
        
        WeakReference<Pin>::Master masterReference;
        friend class WeakReference<Pin>;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Pin)
    };
    
    const OwnedArray<Pin>& getPins (Pin::Placement placement) const;
    
    void setNumPins (int numPins, Pin::Placement placement);
    
    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener() {}
        
        virtual void graphEmbeddedChanged() = 0;
    };
    
    void graphItemChanged() { for (auto& listener : listeners) listener->graphEmbeddedChanged(); }
    void addListener (Listener* listener) { listeners.addIfNotAlreadyThere (listener); }
    void removeListener (Listener* listener) { listeners.removeFirstMatchingValue (listener); }
    
    Pin* replaceWithTempPin (Pin* pinToMove);
    void setNumPinsUpTo (Pin::Placement placement, int num);
    void putTempPinsAtLast();
    
    Array<Pin::Placement> getAllPlacements() const
    {
        return { Pin::Placement::PinOnTop, Pin::Placement::PinOnRight, Pin::Placement::PinOnBottom, Pin::Placement::PinOnLeft };
    }
    
    virtual int getDefaultWidth() const { return 60; }
    virtual int getDefaultHeight() const { return 22; }
    
    //==============================================================================
    int getZ() const { return zIndex; }
    void setZ (int newZ) { zIndex = newZ; }
    
    Point<int> getPositionOnLastMouseDown() const { return positionOnLastMouseDown; }
    void setPositionOnLastMouseDown() { positionOnLastMouseDown = getPosition(); }
    void setPositionOnLastMouseDown (Point<int> pos) { positionOnLastMouseDown = pos; }
    
protected:
    // Non const version.
    OwnedArray<Pin>& getPinsInternal (Pin::Placement placement);
    
    void addPins (Pin::Placement placement, int numToAdd);
    void insertPin (Pin::Placement placement, int indexToInsert);
    void removePin (Pin::Placement placement, int indexToRemove);
    
    //==============================================================================
    friend class PopupMenuHandler;
    
    virtual void addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos) {}
    //virtual void handleExtraPopupMenuCommands (int result, Point<int> pos) override {}
    
    virtual void resizePins();
    void clearPins (bool undoable);
    
    int zIndex = 0;
    
    OwnedArray<Pin> leftPins;
    OwnedArray<Pin> rightPins;
    OwnedArray<Pin> topPins;
    OwnedArray<Pin> bottomPins;
    
private:
    WeakReference<GraphEmbeddedComponent>::Master masterReference;
    friend class WeakReference<GraphEmbeddedComponent>;
    
    virtual void showPopupMenu(Point<int> pos);
    
    void disconnectPin (Pin* pin, bool undoable);
    
    //==============================================================================
    Array<Listener*> listeners;
    
    String publicName;
    int graphId = -1;
    
    WeakReference<GraphPanel> parentGraphPanel;
    
    bool thisCanBeCopied = true;
    bool thisCanBeRenamed = true;
    
    int perPinSize = 12;
    
    Colour backgroundColour;
    Point<int> positionOnLastMouseDown;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphEmbeddedComponent)
};
