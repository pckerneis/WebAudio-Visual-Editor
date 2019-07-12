/*
  ==============================================================================

    WebAudioGraph.h
    Created: 17 Aug 2018 12:22:46am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/Graph/GraphPanel.h"
class WebAudioGraphPanel;

// A GraphPanel with undoable add, remove, connect,...
class WebAudioGraph : public GraphPanel
{
public:
    WebAudioGraph (WebAudioGraphPanel& o);
    
    WebAudioGraphPanel& getOwnerPanel() { return owner; }
    
    void containerMoved();
    
    void pastePreset (const String& xmlDoc, Point<int> offset);
    
    void mouseDown (const MouseEvent& e) override;
    void focusGained (FocusChangeType cause) override;
    
    // Graph panel overrides
    String getClipboardTagName() const override { return "SECTION_GRAPH_SELECTION"; }
    bool isCurrentlyActive() const override;
    void showPopupMenu (Point<int> position) override;
    bool removeComponent (GraphEmbeddedComponent* comp, bool undoable) override;
    void graphContentChanged() override;
    void loadStateFromXml (const XmlElement& e) override;
    XmlElement* getXmlFor (GraphEmbeddedComponent* comp) override;
    GraphEmbeddedComponent* createFromXml (const XmlElement& xml, bool undoable) override;
    bool canConnect (Pin* source, Pin* destination) const override;
    bool createConnectionBetween (Pin* source, Pin* dest, bool undoable) override;
    bool removeConnectionBetween (Pin* source, Pin* dest, bool undoable) override;
    
private:
    static juce::Rectangle<int> findBoundingBox (const String& xmlDoc);
    void pasteAndAddOffset (const String& xmlDoc, Point<int> offset) override;
    
    friend class UndoablePaste;
    Array<GraphEmbeddedComponent*> pasteAndReturn (const String &xml, Array<ConnectionInfo>& connectionInfo) { return pasteInternal (xml, false, connectionInfo); }
    
    WebAudioGraphPanel& owner;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioGraph)
};

//==============================================================================
#include "../Source/Layout/Panels/NavigationPanel.h"
#include "../Source/WebAudio/WebAudioGraph/GraphTipComponent.h"
#include "../Source/WebAudio/WebAudioGraph/WebAudioNodeInstance.h"

class Project;
class InspectorPanel;

class WebAudioDictionary;
class Descriptor;

class WebAudioEmbedded;
class WebAudioContainer;
class PropertyTree;

class WebAudioNode;
class WebAudioDestinationNode;
class WebAudioContext;
class WebAudioDynamicRoute;
class WebAudioMessage;
class WebAudioData;
class WebAudioScript;
class WebAudioComment;

class WebAudioGraphPanel :  public Panel,
							public ApplicationCommandTarget,
                            public GraphPanel::Listener,
                            public NavigationPanel::Navigable,
                            public DragAndDropTarget
{    
public:
    enum ElementCategory
    {
        contextCategory         = 1,
        dataCategory            = 2,
        nodeCategory            = 4,
        messageCategory         = 8,
        dynamicRouteCategory    = 16,
        defaultDestinationNode  = 32,
        functionCategory        = 64,
        commentCategory         = 128,
        
        allCategories           = 255
    };
    
    WebAudioGraphPanel (Project& project);
    ~WebAudioGraphPanel();
    
    WebAudioGraph& getGraphPanel() { return graphPanel; }    
    Project& getProject() { return project; }
    PropertyTree* getInspectorPropertyTree() const;
    GraphSelector& getSelector();
    WebAudioInstanceManager& getInstanceManager() { return instanceManager; }
    
    void hideTips() { tipComp.hideTips(); }
    
    void resized() override;
    void paint (Graphics &g) override;
    void showPopupMenu (Point<int> pos);
    void modalStateFinished (int returnValue, Point<int> pos, WebAudioContainer* c = nullptr);
    void getAddMenuItems (PopupMenu& m, int categoryFlags = ElementCategory::allCategories) const;
    
    bool isVisibleAndHasFocus() const;
    
    void graphSelectionChanged() override;
    void graphPanelSizeMayHaveChanged() override;
    
    void stretchGraphPanelIfNeeded();
    
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    StringPairArray getNavigationInfo() const override;
    void reveal (ModifierKeys mods) override;
    void revealComponent (GraphEmbeddedComponent* compToReveal, ModifierKeys mods);
    
    //==============================================================================
    ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands (Array<CommandID>& commands) override;
    void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    
    GraphEmbeddedComponent* createAndAddUndoable (Descriptor descr, Point<int> pos, String name = String(), WebAudioContainer* c = nullptr);
    
    WebAudioContext* addAudioContext (String interfaceName, Point<int> pos);
    WebAudioNode* addAudioNode (Descriptor& descriptor, int xPos, int yPos, WebAudioContext* ctx);
    WebAudioDestinationNode* addDestinationNode (int xPos, int yPos, WebAudioContext* ctx);
    WebAudioDynamicRoute* addDynamicRoute (Point<int> pos);
    WebAudioMessage* addMessage (int xPos, int yPos, WebAudioDynamicRoute* dr = nullptr);
    WebAudioData* addAudioData (int xPos, int yPos, String interf);
    WebAudioScript* addScriptElement (int xPos, int yPos);
    WebAudioComment* addComment (int xPos, int yPos);
    
    Array<WebAudioContext*> getAllContexts (bool sorted = false) const;
    
    Array<WebAudioNode*> getAllNodes() const;
    Array<WebAudioNode*> getAllNodesInContexts() const;
    Array<WebAudioNode*> getAllNodesWithoutContext (bool sorted = false) const;
    
    Array<WebAudioDynamicRoute*> getAllDynamicRoutes (bool sorted = false) const;
    Array<WebAudioDynamicRoute*> getAllDynamicRoutesContaining (GraphEmbeddedComponent* comp) const;
    
    Array<WebAudioMessage*> getAllMessages() const;
    Array<WebAudioMessage*> getAllMessagesAtRoot() const;
    
    Array<WebAudioData*> getAllAudioData (bool sorted = false) const;
    
    Array<WebAudioScript*> getAllScripts() const;
    Array<WebAudioScript*> getAllScriptsAtRoot() const;
    
    Array<WebAudioEmbedded*> getAllScriptsAndMessagesAtRoot (bool sorted = false) const;
    
    Array<WebAudioComment*> getAllComments() const;
    
    XmlElement* getAsXml() override;
    void restoreState (XmlElement* state) override;
    
    void saveSelectionAsPreset();
    void pastePreset (const String& xmlDoc, Point<int> offset)
    {
        graphPanel.pastePreset (xmlDoc, offset);
        graphPanel.grabKeyboardFocus();
    }
    
    String getUnusedName (String desiredName) const;
    
    Point<int> getCenterOfVisibleGraphArea() const;
    
    //==============================================================================
    bool isInterestedInDragSource (const SourceDetails &dragSourceDetails) override;
    void itemDropped (const SourceDetails &dragSourceDetails) override;
    void itemDragEnter (const SourceDetails &dragSourceDetails) override;
    void itemDragExit (const SourceDetails &dragSourceDetails) override;
    
    UndoManager& getUndoManager() { return graphPanel.getUndoManager(); }
    
    //==============================================================================
    struct ComponentPositionComparator
    {
        static int compareElements (Component* first, Component* second)
        {
            const int firstX = first->getX();
            const int firstY = first->getY();
            const int secondX = second->getX();
            const int secondY = second->getY();
            
            return firstY < secondY ? -1 : firstY > secondY ? 1 : firstX < secondX ? -1 : firstX > secondX ? 1 : 0;
        }
    };
    
    //==============================================================================
    const Array<WeakReference<WebAudioContainer>>& getContainers() const { return containers; }
    void addContainer (WebAudioContainer* cont);
    void removeContainer (WebAudioContainer* cont);
    
protected:
    Project& project;
    
private:
    WebAudioEmbedded* findElementWithName (String name, int category = allCategories) const;
    
    WeakReference<WebAudioGraphPanel>::Master masterReference;
    friend class WeakReference<WebAudioGraphPanel>;
    
    Viewport viewport;
    WebAudioGraph graphPanel;
    InspectorPanel& inspector;
    GraphTipComponent tipComp;
    
    Array<WeakReference<WebAudioContainer>> containers;
    
    WebAudioInstanceManager instanceManager;
    SharedResourcePointer<WebAudioDictionary> nodeDictionary;
        
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioGraphPanel)
};

//==============================================================================
#include "../CodeGenerator/WebAudioGraphCodeGenerator.h"
#include "../Source/Layout/ScriptEditor/JavascriptEditor.h"
#include "../Source/WebServer/WebServer.h"

class Project;

class RootWebAudioGraphPanel : public WebAudioGraphPanel
{
public:
    RootWebAudioGraphPanel (Project& proj);
    ~RootWebAudioGraphPanel() { masterReference.clear(); }
    
    void generateOutput (bool openInEditor = true);
    void testInBrowser();
    
    File getOutputDirectory() const;
    void revealOutputDirectory() const;
    
private:
    WeakReference<RootWebAudioGraphPanel>::Master masterReference;
    friend class WeakReference<RootWebAudioGraphPanel>;
    void copyAudioFiles();
    
    Project& project;
    ScopedPointer<LocalServer> server;
    WebAudioGraphGenerator generator;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RootWebAudioGraphPanel)
};
