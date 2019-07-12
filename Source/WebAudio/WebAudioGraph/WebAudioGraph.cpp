/*
  ==============================================================================

    WebAudioGraph.cpp
    Created: 17 Aug 2018 12:22:46am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/WebAudioGraph/WebAudioGraph.h"

//==============================================================================

WebAudioGraph::WebAudioGraph (WebAudioGraphPanel& o) : owner (o)
{
    setZForConnections (4);
}

void WebAudioGraph::mouseDown (const MouseEvent& e)
{
    GraphPanel::mouseDown (e);
    owner.hideTips();
}

void WebAudioGraph::focusGained (FocusChangeType cause)
{
    if (cause == FocusChangeType::focusChangedDirectly)
        owner.gainFocus();
    
    GraphPanel::focusGained (cause);
}

#include "../Source/WebAudio/GraphElements/WebAudioInspectableElement.h"
#include "../Source/WebAudio/GraphElements/WebAudioNode.h"
#include "../Source/WebAudio/GraphElements/WebAudioContext.h"
#include "../Source/WebAudio/GraphElements/WebAudioDynamicRoute.h"
#include "../Source/WebAudio/GraphElements/WebAudioMessage.h"
#include "../Source/WebAudio/GraphElements/WebAudioData.h"
#include "../Source/WebAudio/GraphElements/WebAudioScript.h"
#include "../Source/WebAudio/GraphElements/WebAudioComment.h"

class UndoableRemove : public UndoableAction
{
public:
    UndoableRemove (WebAudioGraphPanel &p, GraphEmbeddedComponent* component, WebAudioGraph &g) : panel(p), graph (g), persistantRef (component)
    {
    }
    virtual ~UndoableRemove() {}
    
    bool perform() override
    {
        if (persistantRef == nullptr)
            return false;
        
        pos = persistantRef->getPosition();
        
        if (auto node = dynamic_cast<WebAudioNode*> (persistantRef.get()))
            persistantCtx = node->getAudioContext();
        
        if (graph.removeComponent (persistantRef.get(), false))
        {
            panel.navigableChanged();
            return true;
        }
        
        return false;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        graph.addComponent (persistantRef.get());
        persistantRef->setTopLeftPosition (pos);
        
        if (auto node = dynamic_cast<WebAudioNode*> (persistantRef.get()))
            node->setAudioContext (dynamic_cast<WebAudioContext*> (persistantCtx.get()));
        
        if (auto webEmb = dynamic_cast<WebAudioEmbedded*> (persistantRef.get()))
            webEmb->checkContainers (false);
        
        return true;
    }
    
private:
    WebAudioGraphPanel &panel;
    WebAudioGraph &graph;
    WebAudioEmbedded::Ptr persistantRef;
    WebAudioEmbedded::Ptr persistantCtx;
    Point<int> pos;
};

bool WebAudioGraph::removeComponent (GraphEmbeddedComponent* comp, bool undoable)
{
    if (undoable)
    {
        auto undoable = new UndoableRemove (owner, comp, *this);
        return getUndoManager().perform (undoable);
    }
    else
    {
        if (auto ctx = dynamic_cast<WebAudioContext*>(comp))
            for (auto n : ctx->getAllNodes())
                n->setAudioContext (nullptr);
        
        if (GraphPanel::removeComponent (comp, false))
        {
            if (auto cont = dynamic_cast<WebAudioContainer*>(comp))
                owner.removeContainer (cont);
            
            owner.navigableChanged();
            comp = nullptr;
            
            return true;
        }
        
        return false;
    }
}

class UndoableConnect : public UndoableAction
{
public:
    using Pin = GraphEmbeddedComponent::Pin;
    
    UndoableConnect (WebAudioGraph &g, Pin* s, Pin* d) : graph (g), source (s), dest (d)
    {
    }
    virtual ~UndoableConnect() {}
    
    bool perform() override
    {
        if (source == nullptr || dest == nullptr)
            return false;
        
        if (graph.createConnectionBetween (source, dest, false))
            return true;
        
        return false;
    }
    
    bool undo() override
    {
        if (source == nullptr || dest == nullptr)
            return false;
        
        if (graph.removeConnectionBetween (source, dest, false))
            return true;
        
        return true;
    }
    
private:
    WebAudioGraph &graph;
    const WeakReference<Pin> source;
    const WeakReference<Pin> dest;
};

bool WebAudioGraph::createConnectionBetween (Pin* source, Pin* dest, bool undoable)
{
    if (undoable)
    {
        return getUndoManager().perform (new UndoableConnect (*this, source, dest));
    }
    else
    {
        return GraphPanel::createConnectionBetween (source, dest, undoable);
    }
}

class UndoableDisconnect : public UndoableAction
{
public:
    using Pin = GraphEmbeddedComponent::Pin;
    
    UndoableDisconnect (WebAudioGraph &g, Pin* s, Pin* d) : graph (g), source (s), dest (d)
    {
    }
    
    virtual ~UndoableDisconnect() {}
    
    bool perform() override
    {
        if (source == nullptr || dest == nullptr)
            return false;
        
        if (graph.removeConnectionBetween (source, dest, false))
            return true;
        
        return false;
    }
    
    bool undo() override
    {
        if (source == nullptr || dest == nullptr)
            return false;
        
        if (graph.createConnectionBetween (source, dest, false))
            return true;
        
        return true;
    }
    
private:
    WebAudioGraph &graph;
    const WeakReference<Pin> source;
    const WeakReference<Pin> dest;
};

bool WebAudioGraph::removeConnectionBetween (Pin* source, Pin* dest, bool undoable)
{
    if (undoable)
    {
        return getUndoManager().perform (new UndoableDisconnect (*this, source, dest));
    }
    else
    {
        return GraphPanel::removeConnectionBetween (source, dest, false);
    }
}

bool WebAudioGraph::canConnect (Pin* first, Pin* second) const
{
    if (first == nullptr || second == nullptr)
        return false;
    
    if (areAlreadyConnectedTogether(first, second))
        return false;
    
    if (auto firstNode = dynamic_cast<WebAudioNode*>(first->getOwnerComponent()))
    {
        // Node -> node
        if (auto secondNode = dynamic_cast<WebAudioNode*>(second->getOwnerComponent()))
        {
            const auto firstPlacement = first->getPlacement();
            const auto secondPlacement = second->getPlacement();
            
            if (firstPlacement == Pin::PinOnBottom
                && secondPlacement != Pin::PinOnTop
                && secondPlacement != Pin::PinOnLeft)
            {
                return false;
            }
            else if (firstPlacement == Pin::PinOnLeft
                     && secondPlacement != Pin::PinOnBottom)
            {
                return false;
            }
            else if (firstPlacement == Pin::PinOnTop
                     && secondPlacement != Pin::PinOnBottom)
            {
                return false;
            }
            else if (firstPlacement == Pin::PinOnRight)
                return false;
            
            if (firstNode->getPublicName() == secondNode->getPublicName())
                return false;
            /*
             if (firstNode->getAudioContextName() == String() || secondNode->getAudioContextName() == String())
             return false;
             */
            if (firstNode->getAudioContextName() != secondNode->getAudioContextName())
                return false;
            
            return true;
        }
        // Node -> msg
        else if (auto destMsg = dynamic_cast<WebAudioMessage*>(second->getOwnerComponent()))
            if (first->getPlacement() == Pin::PinOnRight)
                return true;
        
        return false;
    }
    else if (auto ctx = dynamic_cast<WebAudioContext*>(first->getOwnerComponent()))
    {
        // Ctx -> msg
        if (auto destMsg = dynamic_cast<WebAudioMessage*>(second->getOwnerComponent()))
            return true;
    }
    else if (auto msg = dynamic_cast<WebAudioMessage*>(first->getOwnerComponent()))
    {
        // Msg -> ctx
        if (auto ctx = dynamic_cast<WebAudioContext*>(second->getOwnerComponent()))
            return true;
        else if (auto buffer = dynamic_cast<WebAudioData*>(second->getOwnerComponent()))
            return true;
        // Msg -> node
        else if (auto node = dynamic_cast<WebAudioNode*>(second->getOwnerComponent()))
            if (second->getPlacement() == Pin::PinOnRight)
                return true;
        
        return false;
    }
    else if (auto buffer = dynamic_cast<WebAudioData*>(first->getOwnerComponent()))
        if (auto msg = dynamic_cast<WebAudioMessage*>(second->getOwnerComponent()))
            return true;
    
    return false;
}

XmlElement* WebAudioGraph::getXmlFor (GraphEmbeddedComponent* comp)
{
    if (comp == nullptr)
        return nullptr;
    
    auto e = GraphPanel::getXmlFor (comp);
    
    
    if (auto element = dynamic_cast<WebAudioEmbedded*>(comp))
    {
        e->setAttribute ("navigableUuid", element->getNavigableUuid().toString());
        e->setAttribute ("interface", element->getInterfaceName());
        e->setAttribute ("elementType", element->getUICompTypeName());
    }
    
    if (auto node = dynamic_cast<WebAudioNode*>(comp))
    {
        if (auto ctx = node->getAudioContext())
            e->setAttribute ("contextGraphId", ctx->getGraphId());
    }
    else if (auto f = dynamic_cast<WebAudioScript*>(comp))
    {
        if (auto editor = f->getCodeEditorPanel())
        {
            e->setAttribute ("editorPanelId", editor->getPanelId());
            e->setAttribute ("editorContent", f->getEditorContent());
        }
    }
    else if (auto data = dynamic_cast<WebAudioData*>(comp))
    {
        if (auto file = data->getLinkedAudioFile())
            e->setAttribute ("linkedFile", file->getUuid().toString());
    }
    
    if (auto foldable = dynamic_cast<WebAudioFoldable*>(comp))
    {
        e->setAttribute ("isOpen", foldable->isOpen());
        
        // Options
        XmlElement* options = new XmlElement ("OPTIONS");
        
        if (auto instance = foldable->getInstance())
            for (auto o : foldable->getInstance()->getOptions())
                options->setAttribute(o->name, o->defaultValue);
        
        e->addChildElement (options);
    }
    
    e->setAttribute ("backgroundColour", comp->getBackgroundColour().toString());
    
    return e;
}

#include "../Source/Project/Project.h"
#include "../Source/WebAudio/AudioFilesPanel/AudioFilesPanel.h"
#include "../Source/WebAudio/Helpers/WebAudioGraphElementTypeNames.h"
GraphEmbeddedComponent* WebAudioGraph::createFromXml (const XmlElement& xml, bool undoable)
{
    const int x = xml.getIntAttribute ("xpos");
    const int y = xml.getIntAttribute ("ypos");
    const int w = xml.getIntAttribute ("width");
    const int h = xml.getIntAttribute ("height");
    const String name (xml.getStringAttribute ("name"));
    const String interf (xml.getStringAttribute ("interface"));
    const String type (xml.getStringAttribute ("elementType"));
    const Colour backgroundColour (Colour::fromString (xml.getStringAttribute ("backgroundColour")));
    const Uuid navigableUuid = Uuid (xml.getStringAttribute ("navigableUuid"));
    const int graphId = xml.getIntAttribute ("graphId");
    
    WebAudioEmbedded* item = nullptr;
    
    if (type == GraphElementType::audioContextType)         item = owner.addAudioContext (interf, Point<int>(x, y));
    else if (type == GraphElementType::messageType)         item = owner.addMessage (x, y);
    else if (type == GraphElementType::dynamicRouteType)    item = owner.addDynamicRoute (Point<int>(x, y));
    else if (type == GraphElementType::scriptType)          item = owner.addScriptElement (x, y);
    else if (type == GraphElementType::audioDestinationNodeType)
    {
        const int ctxId = xml.getIntAttribute ("contextGraphId");
        auto ctx = dynamic_cast<WebAudioContext*>(findComponentWithGraphId(ctxId));
        
        item = owner.addDestinationNode (x, y, ctx);
    }
    else if (type == GraphElementType::audioNodeType)
    {
        SharedResourcePointer<WebAudioDictionary> dict;
        auto descriptor = dict->findDescriptorForInterface (interf);
        
        auto n = owner.addAudioNode(descriptor, x, y, nullptr);
        
        const int ctxId = xml.getIntAttribute ("contextGraphId");
        
        if (auto ctx = dynamic_cast<WebAudioContext*>(findComponentWithGraphId(ctxId)))
            n->setAudioContext (ctx);
        
        item = n;
    }
    else if (type == GraphElementType::audioDataType)
        item = owner.addAudioData (x, y, interf);
    else if (type == GraphElementType::commentType)
        item = owner.addComment (x, y);
    
    // Name (and instance if any) initialisation
    if (item != nullptr)
        item->initializeWithWantedName (name, xml.getBoolAttribute ("createNewInstance"));
    
    // Foldable additional init
    if (auto foldable = dynamic_cast<WebAudioFoldable*>(item))
    {
        // Folded status
        foldable->setOpen (xml.getBoolAttribute ("isOpen"));
        
        // Options
        if (auto options = xml.getChildByName ("OPTIONS"))
        {
            for (int i = 0; i < options->getNumAttributes(); ++i)
            {
                const String key = options->getAttributeName (i);
                const String value = options->getAttributeValue (i);
                foldable->setOptionValue (key, value);
            }
        }
    }
    
    // Layout
    if (item != nullptr)
    {
        item->setSize (w, h);
        item->setGraphId (graphId);
        item->setNavigableUuid (navigableUuid);
        item->setBackgroundColour (backgroundColour);
        item->checkContainers (false);
    }
    
    // Additional initialisations
    if (auto data = dynamic_cast<WebAudioData*>(item))
    {
        const auto uuid = Uuid (xml.getStringAttribute ("linkedFile"));
        
        if (auto afPanel = owner.getProject().findStaticPanelWithClass<AudioFilesPanel>())
            if (auto af = afPanel->findFileWithUuid (uuid))
                af->addUser (data);
    }
    
    if (auto f = dynamic_cast<WebAudioScript*>(item))
    {
        if (auto editor = f->getCodeEditorPanel())
        {
            editor->setPanelId (xml.getIntAttribute ("editorPanelId"));
            editor->getCodeEditor().loadContent (xml.getStringAttribute ("editorContent"));
        }
    }
    
    return item;
}

bool WebAudioGraph::isCurrentlyActive() const
{
    return owner.isVisibleAndHasFocus();
}

void WebAudioGraph::showPopupMenu (Point<int> position)
{
    return owner.showPopupMenu (position);
}

void WebAudioGraph::containerMoved()
{
    for (auto& node : owner.getAllNodes())
        node->checkContainers (false);
    
    for (auto& dr : owner.getAllDynamicRoutes())
        dr->checkContainers (false);
    
    for (auto& msg : owner.getAllMessages())
        msg->checkContainers (false);
    
    for (auto& f : owner.getAllScripts())
        f->checkContainers (false);
    
    for (auto& b : owner.getAllAudioData())
        b->checkContainers (false);
    
    for (auto c : owner.getAllComments())
        c->checkContainers (false);
    
    repaint();
}

void WebAudioGraph::loadStateFromXml (const XmlElement& e)
{
    GraphPanel::loadStateFromXml (e);
    owner.graphSelectionChanged();
}

class UndoablePaste : public UndoableAction
{
public:
    UndoablePaste (WebAudioGraph &g, const String& xml, Point<int> off) : graph (g), xmlDoc (xml), offset (off) {}
    
    bool perform() override
    {
        if (comps.isEmpty())
        {
            auto pasted = graph.pasteAndReturn (xmlDoc, connectionInfo);
            
            for (auto comp : pasted)
            {
                comp->setTopLeftPosition (comp->getX() + offset.getX(), comp->getY() + offset.getY());
                
                if (auto webEmb = dynamic_cast<WebAudioEmbedded*> (comp))
                    comps.add (new PastedComp (webEmb));
            }
            
            return true;
        }
        
        for (auto pasted : comps)
        {
            graph.addComponent (pasted->comp, pasted->pos.x, pasted->pos.y);
            pasted->comp->setPositionOnLastMouseDown();
        }
        
        for (auto info : connectionInfo)
            graph.createConnectionBetween (info.sourcePin, info.destPin, false);
        
        // It'd be better to properly reset the containers for each comp
        for (auto pasted : comps)
            if (auto webEmb = dynamic_cast<WebAudioEmbedded*> (pasted->comp.get()))
                webEmb->checkContainers (false);
        
        return true;
    }
    
    bool undo() override
    {
        for (auto pasted : comps)
            graph.removeComponent (pasted->comp, false);
        
        return true;
    }
    
private:
    WebAudioGraph &graph;
    const String xmlDoc;
    Point<int> offset;
    
    struct PastedComp
    {
        PastedComp (WebAudioEmbedded* c) : comp (c), pos (c->getPosition()) {}
        
        WebAudioEmbedded::Ptr comp;
        const Point<int> pos;
        Array<ConnectionInfo> connections;
    };
    
    OwnedArray<PastedComp> comps;
    Array<ConnectionInfo> connectionInfo;
};

void WebAudioGraph::pasteAndAddOffset (const String& xmlDoc, Point<int> offset)
{
    getUndoManager().beginNewTransaction();
    getUndoManager().perform (new UndoablePaste (*this, xmlDoc, offset));
}


juce::Rectangle<int> WebAudioGraph::findBoundingBox (const String& xmlDoc)
{
    XmlDocument doc (xmlDoc);
    ScopedPointer<XmlElement> elem = doc.getDocumentElement();
    
    if (elem == nullptr)
        return juce::Rectangle<int>();

#undef max
#undef min
    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    int maxRight = std::numeric_limits<int>::min();
    int maxBottom = std::numeric_limits<int>::min();
    int num = 0;
    
    forEachXmlChildElementWithTagName(*(elem.get()), e, "Embedded")
    {
        const int x = e->getIntAttribute ("xpos");
        const int y = e->getIntAttribute ("ypos");
        const int w = e->getIntAttribute ("width");
        const int h = e->getIntAttribute ("height");
        
        minX = jmin (x, minX);
        minY = jmin (y, minY);
        maxRight = jmax (x + w, maxRight);
        maxBottom = jmax (y + h, maxBottom);
        
        ++num;
    }
    
    if (num == 0)
        return juce::Rectangle<int>();
    
    return juce::Rectangle<int> (minX, minY, maxRight, maxBottom);
}

void WebAudioGraph::graphContentChanged()
{
    for (auto& emb : embeddedComponents)
        if (auto cont = dynamic_cast<WebAudioContainer*>(emb))
            cont->checkContent();
    
    for (int i = getSelector().getNumSelected(); --i >= 0;)
    {
        auto sel = getSelector().getSelected (i);
        
        if (auto emb = dynamic_cast<GraphEmbeddedComponent*> (sel))
            if (!getEmbeddedComponents().contains (emb))
                getSelector().removeFromSelection (sel);
    }
}

void WebAudioGraph::pastePreset (const String& xmlDoc, Point<int> offset)
{
    const auto bbox = findBoundingBox (xmlDoc);
    Point<int> centre = Point<int> (bbox.getWidth() / 2, bbox.getHeight() / 2);
    pasteAndAddOffset (xmlDoc, offset - centre);
}

//==============================================================================

#include "../Source/Project/Project.h"
#include "../Source/WebAudio/Inspector/WebAudioInspector.h"
WebAudioGraphPanel::WebAudioGraphPanel (Project& proj) : Panel (&proj.getPanelManager()), project (proj), graphPanel (*this), inspector (*proj.findStaticPanelWithClass<WebAudioInspector>())
{
    graphPanel.addListener (this);
    addAndMakeVisible (viewport);
    viewport.setViewedComponent (&graphPanel, false);
    
    // Prepare command target
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    commandManager.registerAllCommandsForTarget (this);
    
    if (auto inspector = project.findStaticPanelWithClass<WebAudioInspector>())
        if (auto tree = inspector->getPropertyTree())
            tree->setUndoManager (&graphPanel.getUndoManager());
    
    addAndMakeVisible (tipComp);
}

WebAudioGraphPanel::~WebAudioGraphPanel()
{
    graphPanel.removeListener (this);
    masterReference.clear();
}

PropertyTree* WebAudioGraphPanel::getInspectorPropertyTree() const
{
    return inspector.getPropertyTree();
}

GraphSelector& WebAudioGraphPanel::getSelector() {
    return graphPanel.getSelector();
}

void WebAudioGraphPanel::resized()
{
    Panel::resized();
    
    viewport.setBounds (getLocalBounds().withTrimmedTop (getHeaderHeight()));
    graphPanel.adaptSizeToContent();
    
    tipComp.setBounds (getLocalBounds().withTrimmedTop (getHeaderHeight()));
}

void WebAudioGraphPanel::paint (Graphics &g)
{
    Panel::paint (g);
    
    auto b = getLocalBounds().withTrimmedTop (getHeaderHeight());
    g.setColour(getLookAndFeel().findColour (ResizableWindow::backgroundColourId).withMultipliedBrightness(0.9));
    g.fillRect (b);
}

#include "../Source/Application/CommandIDs.h"
void WebAudioGraphPanel::showPopupMenu (Point<int> pos)
{
    PopupMenu m;
    
    m.addCommandItem (&Project::getApplicationCommandManager(), CommandIDs::pasteAtMousePos);
    m.addSeparator();
    PopupMenu addMenu;
    getAddMenuItems (addMenu);
    
    m.addSubMenu ("Add...", addMenu);
    
    class Callback : public ModalComponentManager::Callback
    {
    public:
        Callback (WebAudioGraphPanel* graph, Point<int> position) : owner (graph), pos (position) {}
        
        void modalStateFinished (int result)
        {
            if (owner != nullptr)
                owner->modalStateFinished (result, pos);
        }
        
    private:
        WeakReference<WebAudioGraphPanel> owner;
        Point<int> pos;
    };
    
    m.showMenuAsync (PopupMenu::Options().withMinimumWidth (100)
                     .withMaximumNumColumns (3),
                     new Callback (this, pos));
}


void WebAudioGraphPanel::getAddMenuItems (PopupMenu &m, int categoryFlags) const
{
    if (categoryFlags & ElementCategory::contextCategory)
    {
        PopupMenu ctxMenu;
        // 100 is reserved for context local destination node (in WebAudioContext.h)
        ctxMenu.addItem (101, "Add AudioContext");
        ctxMenu.addItem (102, "Add OfflineAudioContext");
        m.addSubMenu ("Audio context...", ctxMenu);
    }
    
    if (categoryFlags & ElementCategory::dataCategory)
    {
        PopupMenu audioMenu;
        audioMenu.addItem (103, "Add AudioElement");
        audioMenu.addItem (104, "Add AudioBuffer");
        audioMenu.addItem (105, "Add DecodableAudio");
        m.addSubMenu ("Audio data...", audioMenu);
    }
    
    if (categoryFlags & ElementCategory::nodeCategory)
    {
        PopupMenu nodeMenu;
        
        const auto interfaceNames = nodeDictionary->getNodeInterfaceNames();
        const int numNodes = interfaceNames.size();
        
        for (int i = 0; i < numNodes; ++i)
            nodeMenu.addItem (i + 200, "Add " + interfaceNames[i]);
        
        m.addSubMenu ("Audio node...", nodeMenu);
    }
    
    if (categoryFlags & ElementCategory::messageCategory)
        m.addItem (106, "Add message");
    
    if (categoryFlags & ElementCategory::dynamicRouteCategory)
        m.addItem (107, "Add dynamic route");
    
    if (categoryFlags & ElementCategory::functionCategory)
        m.addItem (108, "Add script object");
    
    if (categoryFlags & ElementCategory::commentCategory)
        m.addItem (109, "Add comment");
    
    if (categoryFlags & ElementCategory::defaultDestinationNode)
        m.addItem (110, "Add destination node");
}

#include "WebAudioDictionary.h"
void WebAudioGraphPanel::modalStateFinished (int result, Point<int> pos, WebAudioContainer* c)
{
    SharedResourcePointer<WebAudioDictionary> dict;
    
    Descriptor d;
    
    if (result == 0)            return;
    else if (result == 101)     d = dict->findDescriptorForInterface ("AudioContext");
    else if (result == 102)     d = dict->findDescriptorForInterface ("OfflineAudioContext");
    else if (result == 103)     d = dict->findDescriptorForInterface ("AudioElement");
    else if (result == 104)     d = dict->findDescriptorForInterface ("AudioBuffer");
    else if (result == 105)     d = dict->findDescriptorForInterface ("DecodableAudio");
    else if (result == 106)     d = Descriptor (GraphElementType::messageType);
    else if (result == 107)     d = Descriptor (GraphElementType::dynamicRouteType);
    else if (result == 108)     d = Descriptor (GraphElementType::scriptType);
    else if (result == 109)     d = Descriptor (GraphElementType::commentType);
    else if (result == 110)     d = dict->findDescriptorForInterface ("AudioDestinationNode");
    else if (result >= 200)     d = nodeDictionary->getNodeDescriptors()[result - 200];

    auto newElement = createAndAddUndoable (d, pos, "", c);

    if (newElement != nullptr)
    {
        graphPanel.getSelector().setUniqueSelection (newElement);
        newElement->showNameEditor();
    }
}

void WebAudioGraphPanel::graphPanelSizeMayHaveChanged()
{
    stretchGraphPanelIfNeeded();
}

void WebAudioGraphPanel::stretchGraphPanelIfNeeded()
{
    const int width = jmax (graphPanel.getWidth(), viewport.getMaximumVisibleWidth());
    const int height = jmax (graphPanel.getHeight(), viewport.getMaximumVisibleHeight());
    
    if (width != graphPanel.getWidth() || height != graphPanel.getHeight())
        graphPanel.setSize (width, height);
}

Array<WeakReference<NavigationPanel::Navigable>> WebAudioGraphPanel::getSubNavigables()
{
    Array<WeakReference<Navigable>> result;
    
    for (auto n : getAllNodesWithoutContext (true))
        if (getAllDynamicRoutesContaining(n).isEmpty())
            result.add (n);
    
    for (auto b : getAllAudioData (true))
        result.add (b);
    
    for (auto n : getAllScriptsAndMessagesAtRoot (true))
        result.add (n);
    
    for (auto ctx : getAllContexts (true))
        result.add (ctx);
    
    for (auto dr : getAllDynamicRoutes (true))
        result.add (dr);
    
    return result;
}

StringPairArray WebAudioGraphPanel::getNavigationInfo() const
{
    StringPairArray info;
    
    info.set ("name", getPanelName());
    
    info.set ("type", "GRAPH");
    info.set ("category", "WEBAUDIOGRAPH");
    
    return info;
}

void WebAudioGraphPanel::reveal (ModifierKeys mods)
{
}

void WebAudioGraphPanel::revealComponent (GraphEmbeddedComponent* compToReveal, ModifierKeys mods)
{
    if (isInTab())
        showInTabs();
    
    if (auto window = getParentPanelWindow())
        window->toFront (true);
    
    graphPanel.getSelector().addToSelectionBasedOnModifiers (compToReveal, mods);
}

void WebAudioGraphPanel::graphSelectionChanged()
{
    Array<InspectableElement*> elems;
    
    for (auto elem : graphPanel.getEmbeddedComponents())
        if (elem->isSelected())
            if (auto inspectable = dynamic_cast<InspectableElement*>(elem))
                elems.add (inspectable);
    
    inspector.setInspectedItems (elems);
    
    if (NavigationPanel* np = getNavigationPanel())
        np->selectionChanged();
    
    Project::getApplicationCommandManager().commandStatusChanged();
}

bool WebAudioGraphPanel::isVisibleAndHasFocus() const
{
    return isShowing() && hasPanelFocus();
}

ApplicationCommandTarget* WebAudioGraphPanel::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

#include "../Source/Application/CommandIDs.h"
void WebAudioGraphPanel::getAllCommands (Array<CommandID>& commands)
{
    commands.add (CommandIDs::undo);
    commands.add (CommandIDs::redo);
}

void WebAudioGraphPanel::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    switch (commandID)
    {
        case CommandIDs::undo :
            result.setInfo ("Undo", "Undo the last action", "Edit", 0);
            result.addDefaultKeypress ('z', ModifierKeys::commandModifier);
            result.setActive (graphPanel.getUndoManager().canUndo());
            break;
        case CommandIDs::redo :
            result.setInfo ("Redo", "Redo previously canceled action", "Edit", 0);
            result.addDefaultKeypress ('z', ModifierKeys::commandModifier | ModifierKeys::shiftModifier);
            result.setActive (graphPanel.getUndoManager().canRedo());
            break;
    }
}

bool WebAudioGraphPanel::perform (const InvocationInfo& info)
{
    if (info.commandID == CommandIDs::undo)
        return graphPanel.getUndoManager().undo();
    else if (info.commandID == CommandIDs::redo)
        return graphPanel.getUndoManager().redo();
    
    return false;
}

class UndoableAddAction : public UndoableAction
{
public:
    UndoableAddAction (WebAudioGraphPanel &p, WebAudioGraph &g, Descriptor& descr, Point<int> position, WebAudioContainer* c, String n = String(), bool createNew = true) : panel(p), graph (g), name (n), descriptor (descr), pos (position), containerRef (c), createNewInstance (createNew)
    {}
    
    bool perform() override
    {
        if (persistantRef != nullptr)
        {
            graph.addComponent (persistantRef);
            persistantRef->setCentrePosition (pos);
            
            if (auto node = dynamic_cast<WebAudioNode*>(persistantRef.get()))
                node->setAudioContext (dynamic_cast<WebAudioContext*> (containerRef.get()));
            
            if (auto emb = dynamic_cast<WebAudioEmbedded*>(persistantRef.get()))
                emb->checkContainers (false);
            
            panel.navigableChanged();
            
            return true;
        }
        
        const String descriptorName = descriptor.name;
        const String interfaceName = descriptor.interf.name;
        bool isAudioNode = false;
        
        SharedResourcePointer<WebAudioDictionary> dict;
        
        for (auto d : dict->getNodeDescriptors())
        {
            if (d.interf.name == interfaceName)
            {
                isAudioNode = true;
                break;
            }
        }
        
        if (interfaceName == "AudioDestinationNode" || isAudioNode)
        {
            auto context = dynamic_cast<WebAudioContext*> (containerRef.get());
            WebAudioNode* n = nullptr;
            
            if (interfaceName == "AudioDestinationNode")
                n = panel.addDestinationNode (pos.x, pos.y, context);
            else
                n = panel.addAudioNode (descriptor, pos.x, pos.y, context);
            
            persistantRef = n;
        }
        else if (interfaceName == "AudioContext" || interfaceName == "OfflineAudioContext")
        {
            auto ctx = panel.addAudioContext (interfaceName, pos);
            persistantRef = ctx;
        }
        else if (interfaceName == "AudioBuffer" || interfaceName == "DecodableAudio" || interfaceName == "AudioElement")
        {
            auto b = panel.addAudioData (pos.x, pos.y, interfaceName);
            persistantRef = b;
        }
        else if (descriptorName == GraphElementType::dynamicRouteType)
        {
            auto dr = panel.addDynamicRoute (pos);
            persistantRef = dr;
        }
        else if (descriptorName == GraphElementType::messageType)
        {
            auto dr = dynamic_cast<WebAudioDynamicRoute*> (containerRef.get());
            
            auto msg = panel.addMessage (pos.x, pos.y, dr);
            persistantRef = msg;
        }
        else if (descriptorName == GraphElementType::scriptType)
        {
            auto f = panel.addScriptElement (pos.x, pos.y);
            persistantRef = f;
        }
        else if (descriptorName == GraphElementType::commentType)
        {
            auto c = panel.addComment (pos.x, pos.y);
            persistantRef = c;
        }
        
        if (auto webEmb = dynamic_cast<WebAudioEmbedded*> (persistantRef.get()))
        {
            webEmb->initializeWithWantedName (name, createNewInstance);
            webEmb->checkContainers (false);
        }
        
        if (persistantRef != nullptr)
        {
            persistantRef->setCentrePosition (pos);
            persistantRef->setPositionOnLastMouseDown();
        }
        
        panel.navigableChanged();
        
        return true;
    }
    
    bool undo() override
    {
        if (persistantRef == nullptr)
            return false;
        
        graph.removeComponent (persistantRef, false);
        
        panel.navigableChanged();
        
        return true;
    }
    
    GraphEmbeddedComponent* getComponent() { return persistantRef; }
    
private:
    WebAudioEmbedded::Ptr persistantRef;
    
    WebAudioGraphPanel &panel;
    WebAudioGraph &graph;
    const String name;
    Descriptor descriptor;
    const Point<int> pos;
    WebAudioContainer::Ptr containerRef;
    const bool createNewInstance;
};

GraphEmbeddedComponent* WebAudioGraphPanel::createAndAddUndoable (Descriptor descr, Point<int> pos, String name, WebAudioContainer* c)
{
    UndoableAddAction* undoable (new UndoableAddAction (*this, graphPanel, descr, pos, c, name));
    
    graphPanel.getUndoManager().beginNewTransaction();
    graphPanel.getUndoManager().perform (undoable);
    return undoable->getComponent();
}

WebAudioContext* WebAudioGraphPanel::addAudioContext (String interfaceName, Point<int> pos)
{
    SharedResourcePointer<WebAudioDictionary> dict;
    auto descriptor = dict->findDescriptorForInterface (interfaceName);
    
    auto ctx = new WebAudioContext (getProject(), this, descriptor);
    graphPanel.addComponent (ctx, pos.x, pos.y);
    ctx->setPositionProperties();
    
    for (auto n : getAllNodesWithoutContext())
        n->checkContainers (false);
    
    for (auto dr : getAllDynamicRoutes())
        dr->checkContainers (false);
    
    for (auto msg : getAllMessages())
        msg->checkContainers (false);
    
    for (auto f : getAllScripts())
        f->checkContainers (false);
    
    for (auto b : getAllAudioData())
        b->checkContainers (false);
    
    for (auto c : getAllComments())
        c->checkContainers (false);
    
    navigableChanged();
    
    addContainer (ctx);
    
    return ctx;
}

WebAudioNode* WebAudioGraphPanel::addAudioNode (Descriptor& descriptor, int xPos, int yPos, WebAudioContext* ctx)
{
    auto newNode = new WebAudioNode (project,
                                     descriptor,
                                     this,
                                     ctx);
    
    graphPanel.addComponent (newNode, xPos, yPos);
    
    if (ctx != nullptr)
        newNode->setPropertyValue ("colour", ctx->getBackgroundColour().toString());
    
    newNode->setPositionProperties();
    navigableChanged();
    
    return newNode;
}

WebAudioDestinationNode* WebAudioGraphPanel::addDestinationNode (int xPos, int yPos, WebAudioContext* ctx)
{
    SharedResourcePointer<WebAudioDictionary> dict;
    auto descriptor = dict->findDescriptorForInterface ("AudioDestinationNode");
    
    auto newNode = new WebAudioDestinationNode (project,
                                                descriptor,
                                                this,
                                                ctx);
    
    if (ctx != nullptr)
        newNode->setPropertyValue ("colour", ctx->getBackgroundColour().toString());
    
    graphPanel.addComponent (newNode, xPos, yPos);
    newNode->setPositionProperties();
    navigableChanged();
    
    return newNode;
}

WebAudioDynamicRoute* WebAudioGraphPanel::addDynamicRoute (Point<int> pos)
{
    WebAudioDynamicRoute* dr = new WebAudioDynamicRoute (this);
    graphPanel.addComponent (dr, pos.x, pos.y);
    dr->setPositionProperties();
    
    for (auto n : getAllNodes())
        n->checkContainers (false);
    
    for (auto f : getAllScripts())
        f->checkContainers (false);
    
    for (auto msg : getAllMessages())
        msg->checkContainers (false);
    
    for (auto b : getAllAudioData())
        b->checkContainers (false);
    
    for (auto c : getAllComments())
        c->checkContainers (false);
    
    navigableChanged();
    
    addContainer (dr);
    
    return dr;
}

WebAudioMessage* WebAudioGraphPanel::addMessage (int xPos, int yPos, WebAudioDynamicRoute* dr)
{    
    auto newMsg = new WebAudioMessage (this);
    graphPanel.addComponent (newMsg, xPos, yPos);
    newMsg->setPositionProperties();
    newMsg->checkContainers (false);
    newMsg->setGraphTipComponent (&tipComp);
    
    navigableChanged();
    
    return newMsg;
}

WebAudioData* WebAudioGraphPanel::addAudioData (int xPos, int yPos, String interfaceName)
{
    SharedResourcePointer<WebAudioDictionary> dict;
    auto descriptor = dict->findDescriptorForInterface (interfaceName);
    
    if (descriptor.isUndefined())
        descriptor = Descriptor (interfaceName);
    
    auto buffer = new WebAudioData (this, descriptor);
    graphPanel.addComponent (buffer, xPos, yPos);
    buffer->setPositionProperties();
    buffer->checkContainers (false);
    navigableChanged();
    
    return buffer;
}

WebAudioScript* WebAudioGraphPanel::addScriptElement (int xPos, int yPos)
{
    auto newFunc = new WebAudioScript (*this);
    graphPanel.addComponent (newFunc, xPos, yPos);
    newFunc->setPositionProperties();
    newFunc->checkContainers (false);
    
    navigableChanged();
    
    return newFunc;
}

WebAudioComment* WebAudioGraphPanel::addComment (int xPos, int yPos)
{
    auto newComment = new WebAudioComment (this);
    graphPanel.addComponent (newComment, xPos, yPos);
    newComment->setPositionProperties();
    newComment->checkContainers (false);
    
    navigableChanged();
    
    return newComment;
}

Array<WebAudioContext*> WebAudioGraphPanel::getAllContexts (bool sorted) const
{
    Array<WebAudioContext*> result;
    
    for (auto cont : getContainers())
        if (auto ctx = dynamic_cast<WebAudioContext*> (cont.get()))
            result.add (ctx);
    
    if (sorted)
    {
        ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioNode*> WebAudioGraphPanel::getAllNodes() const
{
    Array<WebAudioNode*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto n = dynamic_cast<WebAudioNode*> (emb))
            result.add (n);
    
    return result;
}

Array<WebAudioNode*> WebAudioGraphPanel::getAllNodesInContexts() const
{
    Array<WebAudioNode*> result;
    
    for (auto c : getAllContexts())
        result.addArray (c->getAllNodes());
    
    return result;
}

Array<WebAudioNode*> WebAudioGraphPanel::getAllNodesWithoutContext (bool sorted) const
{
    Array<WebAudioNode*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto n = dynamic_cast<WebAudioNode*> (emb))
            if (n->getAudioContext() == nullptr)
                result.add (n);
    
    if (sorted)
    {
        ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioDynamicRoute*> WebAudioGraphPanel::getAllDynamicRoutes (bool sorted) const
{
    Array<WebAudioDynamicRoute*> result;
    
    for (auto cont : getContainers())
        if (auto dr = dynamic_cast<WebAudioDynamicRoute*> (cont.get()))
            result.add (dr);
    
    if (sorted)
    {
        ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioDynamicRoute*> WebAudioGraphPanel::getAllDynamicRoutesContaining (GraphEmbeddedComponent* comp) const
{
    Array<WebAudioDynamicRoute*> result;
    
    for (auto cont : getContainers())
        if (auto dr = dynamic_cast<WebAudioDynamicRoute*> (cont.get()))
            if (dr->getContent().contains (comp))
                result.add (dr);
    
    return result;
}

Array<WebAudioMessage*> WebAudioGraphPanel::getAllMessages() const
{
    Array<WebAudioMessage*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto m = dynamic_cast<WebAudioMessage*> (emb))
            result.add (m);
    
    return result;
}

Array<WebAudioMessage*> WebAudioGraphPanel::getAllMessagesAtRoot() const
{
    Array<WebAudioMessage*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto m = dynamic_cast<WebAudioMessage*> (emb))
            if (getAllDynamicRoutesContaining (m).isEmpty())
                result.add (m);
    
    return result;
}

Array<WebAudioData*> WebAudioGraphPanel::getAllAudioData (bool sorted) const
{
    Array<WebAudioData*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto b = dynamic_cast<WebAudioData*> (emb))
            result.add (b);
    
    if (sorted)
    {
        ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioScript*> WebAudioGraphPanel::getAllScripts() const
{
    Array<WebAudioScript*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto f = dynamic_cast<WebAudioScript*> (emb))
            result.add (f);
    
    return result;
}

Array<WebAudioScript*> WebAudioGraphPanel::getAllScriptsAtRoot() const
{
    Array<WebAudioScript*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto f = dynamic_cast<WebAudioScript*> (emb))
            if (getAllDynamicRoutesContaining (f).isEmpty())
                result.add (f);
    
    return result;
}

Array<WebAudioEmbedded*> WebAudioGraphPanel::getAllScriptsAndMessagesAtRoot (bool sorted) const
{
    const auto functions = getAllScriptsAtRoot();
    const auto msgs = getAllMessagesAtRoot();
    
    Array<WebAudioEmbedded*> result;
    result.addArray (functions);
    result.addArray (msgs);
    
    if (sorted)
    {
        ComponentPositionComparator comparator;
        result.sort (comparator);
    }
    
    return result;
}

Array<WebAudioComment*> WebAudioGraphPanel::getAllComments() const
{
    Array<WebAudioComment*> result;
    
    for (auto emb : graphPanel.getEmbeddedComponents())
        if (auto c = dynamic_cast<WebAudioComment*> (emb))
            result.add (c);
    
    return result;
}

XmlElement* WebAudioGraphPanel::getAsXml()
{
    auto e = new XmlElement ("WebAudioGraph");
    e->setAttribute ("panelId", getPanelId());
    
    e->addChildElement (graphPanel.getStateAsXml());
    
    return e;
}

void WebAudioGraphPanel::restoreState (XmlElement* state)
{
    if (state == nullptr)
        return;
    
    setPanelId (state->getIntAttribute ("panelId"));
    
    if (auto graphXml = state->getChildByName ("GraphPanel"))
        graphPanel.loadStateFromXml (*graphXml);
    
    graphPanel.sortWithZ (true);
    navigableChanged();
    
    graphPanel.getUndoManager().clearUndoHistory ();
}

#include "../Source/WebAudio/LibraryPanel/UserLibraryManager.h"
#include "../Source/WebAudio/LibraryPanel/LibraryPanel.h"
void WebAudioGraphPanel::saveSelectionAsPreset()
{
    ScopedPointer<XmlElement> selXml (graphPanel.getSelectionAsXml());
    int numElements = 0;
    String lastInterfaceName;
    String lastInstanceName;
    String lastType;
    
    // Apply offset, determine num embedded and interface name (used if unique selection)
    int minX = std::numeric_limits<int>::max();
    int minY = std::numeric_limits<int>::max();
    
    forEachXmlChildElementWithTagName(*(selXml.get()), e, "Embedded")
    {
        minX = jmin (minX, e->getIntAttribute ("xpos"));
        minY = jmin (minY, e->getIntAttribute ("ypos"));
        
        ++numElements;
        lastInterfaceName = e->getStringAttribute ("interface");
        lastInstanceName = e->getStringAttribute ("name");
        lastType = e->getStringAttribute ("elementType");
        
        // This will make sure a new name is given whenever we want to restore the preset
        e->setAttribute ("createNewInstance", true);
    }
    
    forEachXmlChildElementWithTagName(*(selXml.get()), e, "Embedded")
    {
        e->setAttribute ("xpos", e->getIntAttribute ("xpos") - minX);
        e->setAttribute ("ypos", e->getIntAttribute ("ypos") - minY);
    }
    
    if (numElements == 1)
    {
        if (lastType == GraphElementType::scriptType)
            lastInterfaceName = "Scripts";
        
        if (auto lib = project.findStaticPanelWithClass<LibraryPanel>())
            lib->addPresetFile (lastInstanceName, lastInterfaceName, selXml->createDocument(""), true);
    }
    else
    {
        if (auto lib = project.findStaticPanelWithClass<LibraryPanel>())
            lib->addSnippetFile (selXml->createDocument(""), true);
    }
}

String WebAudioGraphPanel::getUnusedName (String desiredName) const
{
    int categoryFlags = contextCategory | nodeCategory | dataCategory | dynamicRouteCategory | functionCategory;
    
    WebAudioEmbedded* comp = findElementWithName (desiredName, categoryFlags);
    
    if (comp == nullptr)
        return desiredName;
    
    // Remove suffix
    int startIndex = 0;
    int suffixStart = 0;
    
    while (suffixStart >= 0)
    {
        suffixStart = desiredName.indexOfChar (startIndex, '_');
        
        if (suffixStart >= 0 && desiredName.substring (suffixStart + 1).containsOnly ("0123456789"))
            desiredName = desiredName.substring (0, suffixStart);
        
        startIndex = suffixStart + 1;
    }
    
    int number = 1;
    
    String newName;
    
    do
    {
        newName = desiredName;
        
        if (CharacterFunctions::isDigit (desiredName.getLastCharacter()))
            newName << '_'; // pad with an underscore if the name already ends in a digit
        
        newName << ++number;
        
        comp = findElementWithName (newName, categoryFlags);
        
    } while (comp != nullptr);
    
    return newName;
}

Point<int> WebAudioGraphPanel::getCenterOfVisibleGraphArea() const
{
    return viewport.getViewArea().getCentre();
}

bool WebAudioGraphPanel::isInterestedInDragSource (const SourceDetails &dragSourceDetails)
{
    return dragSourceDetails.description.toString() == "GraphElementPreset";
}

#include "../Source/WebAudio/LibraryPanel/LibraryPanel.h"
void WebAudioGraphPanel::itemDropped (const SourceDetails &dragSourceDetails)
{
    if (auto item = dynamic_cast<PresetNavigation::ItemComponent*>(dragSourceDetails.sourceComponent.get()))
    {
        auto pos = dragSourceDetails.localPosition - graphPanel.getPosition() - viewport.getPosition();
        
        if (auto holder = dynamic_cast<PresetNavigation::NavigationItemHolder*> (item->getNavigationItem()))
        {
            auto newElem = createAndAddUndoable (nodeDictionary->findDescriptorForInterface(holder->getDisplayName()), pos);
            
            if (newElem != nullptr)
            {
                graphPanel.getSelector().setUniqueSelection (newElem);
                newElem->showNameEditor();
            }
        }
        else if (auto nav = dynamic_cast<PresetNavigation::NavigationItem*> (item->getNavigationItem()))
        {
            if (auto preset = nav->getPreset())
            {
                auto file = preset->getFile();
                
                if (! file.existsAsFile())
                    return;
                
                graphPanel.pastePreset (file.loadFileAsString(), pos);
            }
        }
        
        graphPanel.grabKeyboardFocus();
    }
}

void WebAudioGraphPanel::itemDragEnter (const SourceDetails &dragSourceDetails)
{
    gainFocus();
}

void WebAudioGraphPanel::itemDragExit (const SourceDetails &dragSourceDetails)
{
    if (auto manager = getPanelManager())
        manager->disableFocus();
}

void WebAudioGraphPanel::addContainer (WebAudioContainer* cont)
{
    containers.addIfNotAlreadyThere (cont);
}

void WebAudioGraphPanel::removeContainer (WebAudioContainer* cont)
{
    containers.removeFirstMatchingValue (cont);
}

WebAudioEmbedded* WebAudioGraphPanel::findElementWithName (String name, int category) const
{
    if (category & contextCategory)
    {
        for (auto c : getAllContexts())
            if (c->getPublicName() == name)
                return c;
    }
    
    if (category & dataCategory)
    {
        for (auto d : getAllAudioData())
            if (d->getPublicName() == name)
                return d;
    }
    
    if (category & nodeCategory)
    {
        for (auto n : getAllNodes())
            if (n->getPublicName() == name)
                return n;
    }
    
    if (category & messageCategory)
    {
        for (auto m : getAllMessages())
            if (m->getPublicName() == name)
                return m;
    }
    
    if (category & dynamicRouteCategory)
    {
        for (auto dr : getAllDynamicRoutes())
            if (dr->getPublicName() == name)
                return dr;
    }
    
    if (category & functionCategory)
    {
        for (auto f : getAllScripts())
            if (f->getPublicName() == name)
                return f;
    }
    
    return nullptr;
}

//==============================================================================

RootWebAudioGraphPanel::RootWebAudioGraphPanel (Project& proj) : WebAudioGraphPanel (proj), project (proj), generator (*this)
{
    setPanelName ("Audio graph");
}

void RootWebAudioGraphPanel::generateOutput (bool openInEditor)
{
    auto projectDirectory = project.getProjectDirectory();
    
    // Create directories
    auto outputDir = projectDirectory.getChildFile ("output");
    auto jsDir = outputDir.getChildFile ("js");
    auto dataDir = outputDir.getChildFile ("data");
    
    if (! outputDir.exists())   outputDir.createDirectory();
    if (! jsDir.exists())       jsDir.createDirectory();
    if (! dataDir.exists())     dataDir.createDirectory();
    
    // Copy audio files
    copyAudioFiles();
    
    // Generate code
    generator.generate();
    
    // Generate code files
    auto writeToFile = [=](File f, String c)
    {
        f.deleteFile();
        
        FileOutputStream fos (f);
        
        if (fos.openedOk())
            fos.writeText (c, false, false, "\r\n");
    };
    
    if (generator.isUsingCoreLibrary())
        writeToFile (outputDir.getChildFile ("js/wave.js"), generator.getCoreLibraryCode());
    
    writeToFile (outputDir.getChildFile ("js/audio.js"), generator.getGeneratedScript());
    writeToFile (outputDir.getChildFile ("index.html"), generator.getGeneratedHtmlPage());
    
    writeToFile (outputDir.getChildFile ("style.css"),
                 String::createStringFromData (BinaryData::style_css,
                                               BinaryData::style_cssSize));
    
    // Open in internal code preview panel
    if (! openInEditor)
        return;
    
    if (auto editor = project.findStaticPanelWithClass<CodeEditorPanel>())
    {
        editor->getCodeEditor().setReadOnly (true);
        editor->getCodeEditor().loadContent (generator.getGeneratedScript(), true);
    }
}

void RootWebAudioGraphPanel::testInBrowser()
{
    generateOutput (true);
    
    const auto outputDir = getOutputDirectory();
    
    int port = 8080;
    
    // if server hasn't been set or if the output directory changed
    if (server == nullptr || server->getDirectory() != outputDir)
        server = new LocalServer (outputDir, port);
    
    // If 8080 is already taken, the port may have changed
    port = server->getPortNumber();
    
    URL url ("http://localhost:" + String (port));
    url.launchInDefaultBrowser();
}

File RootWebAudioGraphPanel::getOutputDirectory() const
{
    return project.getProjectDirectory().getChildFile ("output");
}

void RootWebAudioGraphPanel::revealOutputDirectory() const
{
    getOutputDirectory().revealToUser();
}

void RootWebAudioGraphPanel::copyAudioFiles()
{
    const File outputDirectory = getOutputDirectory();
    
    if (auto filesPanel = project.findStaticPanelWithClass<AudioFilesPanel>())
    {
        for (auto f : filesPanel->getSubItems())
        {
            auto source = f->getFile();
            auto dest = outputDirectory.getChildFile ("data/" + f->getOutputName());
            
            if (dest.exists() || ! source.exists())
                continue;
            
            source.copyFileTo (dest);
        }
    }
}
