/*
  ==============================================================================

    GraphPanel.h
    Created: 21 Aug 2017 8:37:46pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/Graph/GraphEmbeddedComponent.h"

struct ConnectionInfo;

//==============================================================================
class InternalClipboard;
class GraphContainer;

/** \brief A graphical component into which GraphEmbeddedComponent instances can be connected. */
class GraphPanel    : public Component, public ApplicationCommandTarget
{
public:
    typedef GraphEmbeddedComponent::Pin Pin;
    
    GraphPanel ();
    virtual ~GraphPanel();
    
    //==============================================================================
    void resized() override;
    void adaptSizeToContent(bool stretchOnly = false);
    void mouseDown (const MouseEvent& e) override;
    void mouseUp (const MouseEvent &e) override;
    void mouseDrag (const MouseEvent& e) override;
    void startLasso (const MouseEvent& e);
    void dragLasso (const MouseEvent& e);
    void endLasso();
    
    //==============================================================================
    GraphSelector& getSelector() { return selector; }
    void selectionChanged() { for (auto l : listeners) l->graphSelectionChanged(); }
    
    //==============================================================================
    const ReferenceCountedArray<GraphEmbeddedComponent>& getEmbeddedComponents() const { return embeddedComponents; }
    Array<WeakReference<GraphEmbeddedComponent>> getAllConnected (const GraphEmbeddedComponent* comp, Pin::Placement placement) const;
    Array<ConnectionInfo> getAllConnections (const GraphEmbeddedComponent* comp, Pin::Placement placement) const;
    
    //==============================================================================
    void clear();
    
    //==============================================================================
    virtual String getClipboardTagName() const { return "GRAPH_SELECTION"; }
    virtual void showPopupMenu (Point<int> position) {}
    virtual bool isCurrentlyActive() const { return true; }
    
    virtual bool canConnect (Pin* source, Pin* destination) const;
    virtual int addComponent (GraphEmbeddedComponent* comp, int x = 0, int y = 0);
    virtual bool removeComponent (GraphEmbeddedComponent* comp, bool undoable);

    virtual bool createConnectionBetween (Pin* source, Pin* dest, bool undoable);
    virtual bool removeConnectionBetween (Pin* source, Pin* dest, bool undoable);
    virtual bool disconnect (GraphEmbeddedComponent* comp, bool undoable);
    
    //==============================================================================
    virtual void graphContentChanged() {}
    
    //==============================================================================
    void moveToFront (GraphEmbeddedComponent* comp, bool useZ = true);    
    ReferenceCountedArray<GraphEmbeddedComponent> sortWithZ (bool alsoArrangeComponents);
    void setZForConnections (int newZ);
    
    //==============================================================================
    /** \brief A connection in the GraphPanel. It links two pins together. */
    class Connection : public GraphEmbeddedComponent::Listener, public GraphSelectableItem
    {
    public:
        typedef GraphEmbeddedComponent::Pin Pin;
        
        Connection (GraphPanel* panel, Pin* source, Pin* dest);
        ~Connection();
        
        Pin* getSourcePin() const { return sourcePin; }
        Pin* getDestinationPin() const { return destPin; }
        ConnectionInfo getInfo();
       
        void paint (Graphics &g) override;
        
        static Path drawConnection (Point<float> from, Point<float> to, int stroke, Colour colour, Pin::Placement sourcePlacement, Pin::Placement destPlacement, Graphics &g);
        static Path drawTemporaryConnection (Point<float> from, Point<float> to, int stroke, Colour colour, Pin::Placement sourcePlacement, Graphics &g);
        
        void graphEmbeddedChanged() override;
        bool hitTest (int x, int y) override;
        void deleteSelectedItem() override;
        
        void setValid (bool shouldBeValid)
        {
            isValid = shouldBeValid;
            repaint();
        }
        
    private:
        void stopListening();
        
        void getDistancesFromEnds (int x, int y, double& distanceFromStart, double& distanceFromEnd);
        Point<int> getPinCentreInGraph (const Pin* p);
        
        static Point<int> getControlPoint (Point<int> pinCentre, Pin::Placement placement, int marginX, int marginY);
        
        Pin* sourcePin;
        Pin* destPin;
        Path hitPath;
        WeakReference<GraphPanel> graphPanel;
        bool isValid = true;
        
        static double distance (int x1, int y1, int x2, int y2) { return sqrt((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)); }
        
        WeakReference<Connection>::Master masterReference;
        friend class WeakReference<Connection>;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Connection)
    };
    
    virtual Connection* createConnectionObject (GraphPanel* panel, Pin* source, Pin* dest);
    
    //==============================================================================
    void sortConnections();
    OwnedArray<Connection>& getConnections() { return connections; }
    bool areAlreadyConnectedTogether (Pin* one, Pin* another) const;
    
    //==============================================================================
    class TemporaryConnection : public Component
    {
    public:
        TemporaryConnection (GraphPanel* panel, GraphEmbeddedComponent::Pin* pin, Point<int> point, bool pinToPoint);
        
        void paint (Graphics &g) override;
        
        Pin* getSourcePin() const { return sourcePin; }
        // NOTE : here "destination" refers to the direction of this connection. Its acceptation differs in
        // setPotentialDestination() where destination means a Pin this connection would be linked to if it was released.
        Pin* getDestinationPin() const { return destPin; }
        Pin* getDragSourcePin() const { return fromPinToPoint ? sourcePin : destPin; }
        
        void move (Point<int> p) { floatingPoint = p; repaint(); }
        
        // Specify a Pin destination. The connection will be effective only after the temp was released. */
        void setPotentialDestination (Pin* newDestination);
        
    private:
        WeakReference<Pin> sourcePin;
        WeakReference<Pin> destPin;
        
        WeakReference<GraphPanel> graphPanel;
        
        Point<int> floatingPoint;
        bool fromPinToPoint;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TemporaryConnection)
    };
    
    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener();
        
        virtual void graphSelectionChanged() {}
        virtual void graphPanelSizeMayHaveChanged() {}
        
    private:
        WeakReference<Listener>::Master masterReference;
        friend class WeakReference<Listener>;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };
    
    void addListener (Listener* listener) { listeners.addIfNotAlreadyThere (listener); }
    void removeListener (Listener* listener) { listeners.removeFirstMatchingValue (listener); }
    
    //==============================================================================
    //          Command target
    void prepareCommandTarget();
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;

    // Checks clipboard validity.
    bool canPaste() const;
    
    bool hasUniqueSelection() const { return selector.getNumSelected() == 1; }
    bool anythingSelected() const { return selector.getNumSelected() > 0; }
    bool anythingToSelect() const { return embeddedComponents.size() > 0; }
    
    void selectAll() { for (auto comp : embeddedComponents) selector.addToSelection (comp); }
    void deselectAll() { selector.deselectAll(); }
    
    bool deleteSelection() { return selector.deleteSelection(); }
    void copySelection();
    void cutSelection();
    void duplicateSelection();
    void paste();
    void pasteAtMousePos();
    
    void renameSelected();
    
    //==============================================================================
    //          Xml save/restore
    virtual XmlElement* getStateAsXml();
    virtual void loadStateFromXml (const XmlElement& xml);
    
    virtual XmlElement* getXmlFor (GraphEmbeddedComponent* comp);
    virtual XmlElement* getXmlFor (Connection* conn);
    virtual GraphEmbeddedComponent* createFromXml (const XmlElement& xml, bool undoable) { return nullptr; }
    
    XmlElement* getSelectionAsXml();
    String getSelectionAsString();
    
    /* NOTE : There's an undomanager here but it's only used in sub-class for now ! */
    UndoManager& getUndoManager() { return undoManager; }
protected:
    void refreshGraphIds();
    GraphEmbeddedComponent* findComponentWithGraphId (int graphId);
    
    ReferenceCountedArray<GraphEmbeddedComponent> embeddedComponents;
    
    virtual void releaseTemporaryConnection();
    
    virtual void pasteAndAddOffset (const String& xmlDoc, Point<int> offset);
    Array<GraphEmbeddedComponent*> pasteInternal (const String& xmlDoc, bool undoable, Array<ConnectionInfo>& connectionInfo);
    
private:
    // Used when restoring the graphs' state.
    bool createConnectionBetween (int sourceId, int sourceChannel, Pin::Placement sourcePlac,
                                  int destId, int destChannel, Pin::Placement destPlac,
                                  bool undoable);
    
    void startTemporaryConnection (GraphEmbeddedComponent::Pin* starterPin);
    void moveTemporaryConnection();
    void deleteTemporaryConnection();
    
    void setHighlighted (GraphEmbeddedComponent* compToHighlight);
    
    Pin* lookForPinAt (Point<int> p);
    Pin* lookForPinWithIds (int graphId, int channelId, Pin::Placement placementToLookFor);
	juce::Rectangle<int> getRelativeBoundsForPin (Pin* p);
        
    //==============================================================================
    WeakReference<GraphPanel>::Master masterReference;
    friend class WeakReference<GraphPanel>;
    friend class GraphEmbeddedComponent::Pin;
    
    int lastIdGiven = -1;
    String lastDuplicated;
    int numConsecutiveDuplicate = 0;
    int connectionsZ = 0;
    
    SharedResourcePointer<InternalClipboard> clipboard;
    GraphSelector selector;
    
    OwnedArray<Connection> connections;
    ScopedPointer<TemporaryConnection> tempConnection;
    ScopedPointer<LassoComponent<GraphSelectableItem*>> lasso;
    Array<WeakReference<Listener>> listeners;
    
    UndoManager undoManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GraphPanel)
};

//==============================================================================

struct ConnectionInfo
{
    typedef GraphEmbeddedComponent::Pin Pin;
    
    ConnectionInfo (Pin* sPin, Pin::Placement sPlacement,
                    int sIndex, GraphEmbeddedComponent *const sComp,
                    Pin* dPin, Pin::Placement dPlacement,
                    int dIndex, GraphEmbeddedComponent *const dComp,
                    WeakReference<GraphPanel::Connection> c);
    
    WeakReference<Pin> sourcePin;
    const Pin::Placement sourcePlacement;
    const int sourceIndex;
    GraphEmbeddedComponent *const sourceComp;
    
    WeakReference<Pin> destPin;
    const Pin::Placement destPlacement;
    const int destIndex;
    GraphEmbeddedComponent *const destComp;
    
    WeakReference<GraphPanel::Connection> connection;
    
    bool operator== (const ConnectionInfo& other) const
    {
        return sourcePin == other.sourcePin
        && sourcePlacement == other.sourcePlacement
        && sourceIndex == other.sourceIndex
        && sourceComp == other.sourceComp
        && destPin == other.destPin
        && destPlacement == other.destPlacement
        && destIndex == other.destIndex
        && destComp == other.destComp
        && connection == other.connection;
    }
};
