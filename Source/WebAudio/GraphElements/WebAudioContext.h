/*
  ==============================================================================

    WebAudioContext.h
    Created: 17 Aug 2018 3:17:37pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"
#include "WebAudioInspectableElement.h"

class Project;
class WebAudioDestinationNode;
class WebAudioNode;

class WebAudioContext :     public WebAudioContainer
{
public:
    WebAudioContext (Project& proj, WebAudioGraphPanel* parent, Descriptor& descr);
    ~WebAudioContext();
    
    Array<WebAudioNode*> getAllNodes (bool sorted = false) const;
    
    void paint (Graphics& g) override;
    
    void addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos) override;
    void handleExtraPopupMenuCommands (int result, Point<int> pos) override;
    
    String getUICompTypeName() const override { return GraphElementType::audioContextType; }
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    void editorShown (Label* label, TextEditor&) override;
    
    String getValidName (String wantedName) override;
    void setPublicName (String newName) override;
    
    void addDestinationNode (Point<int> pos);
    Array<WeakReference<WebAudioDestinationNode>>& getDestinationNodes();
    
private:
    WeakReference<WebAudioContext>::Master masterReference;
    friend class WeakReference<WebAudioContext>;
    
    Project& project;
    
    friend class WebAudioDestinationNode;
    Array<WeakReference<WebAudioDestinationNode>> destinationNodes;

    SharedResourcePointer<WebAudioDictionary> nodeDictionary;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioContext)
};
