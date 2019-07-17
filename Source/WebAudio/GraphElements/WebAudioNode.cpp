/*
  ==============================================================================

    WebAudioNode.cpp
    Created: 17 Aug 2018 12:22:34am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/GraphElements/WebAudioNode.h"

#include "../../Project/Project.h"
#include "../WebAudioGraph/WebAudioGraph.h"
#include "../Helpers/JsCodeHelpers.h"

WebAudioNode::WebAudioNode (Project& proj, Descriptor& descriptor, WebAudioGraphPanel* parent, WebAudioContext* ctx, bool canBeRenamed)
:  WebAudioFoldable (*parent, descriptor), project (proj)
{
    setZ (5);
    
    prepareLabel();
    
    setParentGraph (&getWebAudioGraph());
    setAudioContext (ctx);
    
    setCanBeRenamed (canBeRenamed);
    
    //prepareInspectablePropertiesTree (getUICompTypeName());
    
    //setPropertyValue ("context", getAudioContextName());
}

#include "../WebAudioGraph/WebAudioNodeInstance.h"
WebAudioNode::~WebAudioNode()
{
    getInstanceManager().removeReference (this);
    masterReference.clear();
}

#include "../Source/WebAudio/GraphElements/WebAudioContext.h"
String WebAudioNode::getAudioContextName() const
{
    return audioContext ? audioContext->getPublicName() : "";
}

void WebAudioNode::setAudioContext (WebAudioContext* ctx)
{
    if (audioContext == ctx)
        return;
    
    if (audioContext != nullptr)
        audioContext->removeComponent (this);
    
    audioContext = ctx;
    
    if (audioContext != nullptr)
        audioContext->addComponent (this);
    
    navigableChanged();
    
    checkConnectionsValidity();
    
    setPropertyValue ("context", getAudioContextName());
    repaint();
}

#include "../Source/WebAudio/GraphElements/WebAudioDynamicRoute.h"

void WebAudioNode::checkContainers (bool useHighlight)
{
    String shouldBeInContext;
    
    if (getAudioContextName() != String())
        shouldBeInContext = getAudioContextName();
    else
    {
        for (auto n : parentPanel.getAllNodesInContexts())
        {
            if (n != this && n->getPublicName() == getPublicName())
            {
                shouldBeInContext = n->getAudioContextName();
                break;
            }
        }
    }
    
    const auto centre = getBounds().removeFromTop (getDefaultHeight()).getCentre();
    bool targetContextFound = false;
    
    for (auto cont : parentPanel.getContainers())
    {
        if (auto ctx = dynamic_cast<WebAudioContext*> (cont.get()))
        {
            bool wrongContext = false;
            
            if (shouldBeInContext != String())
                if (shouldBeInContext != ctx->getPublicName())
                    wrongContext = true;
            
            if (! wrongContext && ! targetContextFound && ctx->getBounds().contains (centre))
            {
                ctx->addComponent (this);
                setAudioContext (ctx);
                targetContextFound = true;
                
                if (useHighlight)
                {
                    ctx->setHighlighted (true);
                    getParentGraph()->moveToFront(ctx);
                }
            }
            else
            {
                ctx->removeComponent (this);
                ctx->setHighlighted (false);
            }
        }
        else if (auto dr = dynamic_cast<WebAudioDynamicRoute*> (cont.get()))
        {
            if (dr->getBounds().contains (centre))
            {
                dr->addComponent (this);
                
                if (useHighlight)
                {
                    dr->setHighlighted (true);
                    getParentGraph()->moveToFront(dr);
                }
            }
            else
            {
                dr->removeComponent (this);
                dr->setHighlighted (false);
            }
        }
    }
    
    if (! targetContextFound)
        setAudioContext (nullptr);
}

void WebAudioNode::mouseUp (const MouseEvent &e)
{
    WebAudioFoldable::mouseUp (e);
    
    checkConnectionsValidity();
}

void WebAudioNode::checkConnectionsValidity()
{
    if (getParentGraph() == nullptr)
        return;
    
    auto connectionsAsSource = getParentGraph()->getAllConnections (this, Pin::PinOnRight);
    connectionsAsSource.addArray (getParentGraph()->getAllConnections (this, Pin::PinOnBottom));
    
    Array<ConnectionInfo> alreadyChecked;
    
    for (auto info : connectionsAsSource)
    {
        if (alreadyChecked.contains (info))
            continue;
        
        alreadyChecked.add (info);
        
        info.connection->setValid (true);
        
        auto dest = info.destComp;
        
        if (auto node = dynamic_cast<WebAudioNode*> (dest))
        {
            if (node->getPublicName() == getPublicName())
            {
                // Node to audio param
                const bool nodeToParam = info.sourcePlacement == Pin::PinOnRight && info.destPlacement == Pin::PinOnBottom;
                
                info.connection->setValid (nodeToParam);
                break;
            }
            
            if (node->getAudioContextName() != getAudioContextName())
            {
                info.connection->setValid (false);
                break;
            }
        }
    }
    
    auto connectionsAsDest = getParentGraph()->getAllConnections (this, Pin::PinOnLeft);
    connectionsAsDest.addArray (getParentGraph()->getAllConnections (this, Pin::PinOnTop));
    
    for (auto info : connectionsAsDest)
    {
        if (alreadyChecked.contains (info))
            continue;
        
        alreadyChecked.add (info);
        
        info.connection->setValid (true);
        
        auto source = info.sourceComp;
        
        if (auto node = dynamic_cast<WebAudioNode*> (source))
        {
            if (node->getPublicName() == getPublicName())
            {
                // Node to audio param
                const bool nodeToParam = info.sourcePlacement == Pin::PinOnRight && info.destPlacement == Pin::PinOnBottom;
                
                info.connection->setValid (nodeToParam);
                break;
            }
            if (node->getAudioContextName() != getAudioContextName())
            {
                info.connection->setValid (false);
                break;
            }
        }
    }
}

void WebAudioNode::setNumInputs (int numInputs)
{
    numInputs = jmin (numInputs, maxInputs);
    setNumPins (numInputs, Pin::Placement::PinOnTop);
}

void WebAudioNode::setNumOutputs (int numOutputs)
{
    numOutputs = jmin (numOutputs, maxOutputs);
    setNumPins (numOutputs, Pin::Placement::PinOnBottom);
}

void WebAudioNode::editorShown (Label* l, TextEditor& editor)
{
    label->setInterceptsMouseClicks(true, true);
    label->getCurrentTextEditor()->setInputRestrictions(0, JsCodeHelpers::getAllowedCharactersForIdentifier());
    
    setHighlighted (false);
}

void WebAudioNode::refreshOptionsTree()
{
    WebAudioFoldable::refreshOptionsTree();
    
    auto instance = getInstance();
    
    if (instance == nullptr)
        return;
    
    auto &options = instance->getOptions();
    
    setNumPins (options.size(), Pin::PinOnLeft);
    
    auto &pins = getPinsInternal (Pin::PinOnLeft);
    
    for (int i = 0; i < options.size(); ++i)
    {
        const auto optionName = options.getUnchecked (i)->name;
        String optionInterface;
        
        for (auto prop : privateDescriptor.interf.properties)
        {
            if (prop.name == optionName)
            {
                optionInterface = prop.interf.name;
                break;
            }
        }
        
        pins[i]->setVisible (optionInterface == "AudioParam");
    }
    
    resized();
}

void WebAudioNode::resizePins()
{
    const int pinWidth = 10;
    
    GraphEmbeddedComponent::resizePins();
    
    if (! isOpen())
    {
        for (auto pin : leftPins)
        {
            pin->setBounds (getLocalBounds().removeFromLeft (12).withSizeKeepingCentre (pinWidth, pinWidth));
            pin->setEnabled (false);
        }
    }
    else
    {
		juce::Rectangle<float> leftPinsArea (getLocalBounds().withTrimmedTop(getDefaultHeight()).removeFromLeft (12).toFloat());
        const float numLeft = leftPins.size();
        const float h = numLeft * optionsTree.getDefaultItemHeight();
        
        for (int i = 0; i < numLeft; ++i)
        {
            leftPins[i]->setBounds (leftPinsArea.removeFromTop (h / numLeft).withSizeKeepingCentre (pinWidth, pinWidth).getSmallestIntegerContainer());
            leftPins[i]->setEnabled (true);
        }
    }
    
    // For connections to repaint
    graphItemChanged();
}

void WebAudioNode::setPublicName (String newName)
{
    WebAudioFoldable::setPublicName (newName);
    
    checkConnectionsValidity();
}

bool WebAudioNode::nameIsAlreadyTaken (String name)
{
    if (WebAudioEmbedded::nameIsAlreadyTaken (name))
        return true;
    
    for (auto emb : getParentGraph()->getEmbeddedComponents())
    {
        if (auto node = dynamic_cast<WebAudioNode*>(emb))
        {
            if (node != this && node->getPublicName() == name &&
                node->getAudioContextName() != getAudioContextName())
                return true;
        }
    }
    
    return false;
}

//==============================================================================
WebAudioDestinationNode::WebAudioDestinationNode (Project& proj, Descriptor& descriptor, WebAudioGraphPanel* parent, WebAudioContext* ctx) : WebAudioNode (proj, descriptor, parent, ctx, false)
{
    setOpenButtonVisible (false);
    
    setPublicName (".destination");
    
    if (ctx)
        ctx->addComponent (this);
}

WebAudioDestinationNode::~WebAudioDestinationNode()
{
    if (auto ctx = getAudioContext())
        ctx->destinationNodes.removeFirstMatchingValue (this);
    
    masterReference.clear();
}

void WebAudioDestinationNode::checkContainers (bool useHighlight)
{
    auto context = getAudioContext();
    const String shouldBeInContext = context ? context->getPublicName() : "";
    
    const auto centre = getBounds().getCentre();
    bool targetContainerFound = false;
    
    WebAudioContext* newCtx = nullptr;
    
    for (auto cont : parentPanel.getContainers())
    {
        if (auto ctx = dynamic_cast<WebAudioContext*> (cont.get()))
        {
            const bool wrongContext = (shouldBeInContext != String() && shouldBeInContext != ctx->getPublicName());
            
            if (! wrongContext && ! targetContainerFound && ctx->getBounds().contains (centre))
            {
                newCtx = ctx;
                targetContainerFound = true;
                
                if (useHighlight)
                {
                    ctx->setHighlighted (true);
                    getParentGraph()->moveToFront(ctx);
                }
            }
            else
            {
                ctx->setHighlighted (false);
                ctx->removeComponent (this);
            }
        }
        else if (auto dr = dynamic_cast<WebAudioDynamicRoute*> (cont.get()))
        {
            if (dr->getBounds().contains (centre))
            {
                dr->addComponent (this);
                
                if (useHighlight)
                {
                    dr->setHighlighted (true);
                    getParentGraph()->moveToFront(dr);
                }
            }
            else
            {
                dr->removeComponent (this);
                dr->setHighlighted (false);
            }
        }
    }
    
    if (newCtx == getAudioContext())
        return;
    
    if (context != nullptr)
    {
        context->removeComponent (this);
        context->getDestinationNodes().removeFirstMatchingValue (this);
    }
    
    if (newCtx != nullptr)
    {        
        newCtx->getDestinationNodes().add (this);
        newCtx->addComponent (this);
    }
    
    setAudioContext (newCtx);
}

void WebAudioDestinationNode::setPublicName (String newName)
{
    WebAudioFoldable::setPublicName (newName);
        
    setSize (getWidthWhenClosed(), getDefaultHeight());
    setSizeProperties();
    
    checkConnectionsValidity();
}

void WebAudioDestinationNode::initializeWithWantedName (String name, bool createNewInstance)
{
    // We need to ignore the default name attribution
    prepareOptionsTree();
}
