/*
  ==============================================================================

    WebAudioNode.h
    Created: 17 Aug 2018 12:22:34am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"
#include "WebAudioInspectableElement.h"

class Project;
class WebAudioGraphPanel;
class WebAudioContext;

class WebAudioNode :        public WebAudioFoldable
{
public:
    WebAudioNode (Project& proj, Descriptor& descriptor, WebAudioGraphPanel* parent, WebAudioContext* ctx, bool canBeRenamed = true);
    ~WebAudioNode();
    
    String getAudioContextName() const;
    WebAudioContext* getAudioContext() const { return audioContext; }
    virtual void setAudioContext (WebAudioContext* ctx);
    
    void setNumInputs (int numInputs) override;
    void setNumOutputs (int numOutputs) override;
    
    void checkContainers (bool useHighlight) override;
    void checkConnectionsValidity();
    
    void mouseUp (const MouseEvent &e) override;
    String getUICompTypeName() const override { return GraphElementType::audioNodeType; }
    void setPublicName (String newName) override;
    void editorShown (Label*, TextEditor&) override;
    bool isWebAudioElementEnabled() const override { return /*audioContext != nullptr;*/ true; }
    void refreshOptionsTree() override;
    void resizePins() override;
    
    bool nameIsAlreadyTaken (String newName) override;
    
    void wasConnected() override { checkConnectionsValidity(); }
    
protected:
    Project& project;
    
private:
    WeakReference<WebAudioNode>::Master masterReference;
    friend class WeakReference<WebAudioNode>;
    
    WeakReference<WebAudioContext> audioContext;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioNode)
};

//==============================================================================

class WebAudioDestinationNode : public WebAudioNode
{
public:
    WebAudioDestinationNode (Project& proj, Descriptor& descriptor, WebAudioGraphPanel* parent, WebAudioContext* ctx);
    ~WebAudioDestinationNode();
    
    String getUICompTypeName() const override { return GraphElementType::audioDestinationNodeType; }
    void checkContainers (bool useHighlight) override;
    void setPublicName (String newName) override;
    
    // This node can't be manually renamed so we assume that its name is always valid
    // Plus, the inherited method from WebAudioNode would remove the dot character
    String getValidName (String wanted) override { return wanted; }
    void initializeWithWantedName (String name, bool createNewInstance) override;

private:
    WeakReference<WebAudioDestinationNode>::Master masterReference;
    friend class WeakReference<WebAudioDestinationNode>;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioDestinationNode)
};
