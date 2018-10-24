/*
  ==============================================================================

    WebAudioGraphCodeGenerator.cpp
    Created: 20 Aug 2018 1:11:38am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioGraphCodeGenerator.h"

WebAudioGraphGenerator::WebAudioGraphGenerator (WebAudioGraphPanel& webAudioGraph) : graph (webAudioGraph)
{
    libraryCode = String::createStringFromData (BinaryData::wave_js, BinaryData::wave_jsSize);
}

void WebAudioGraphGenerator::generate()
{
    // Parameters
    const bool strictMode = true;
    const bool useDefaultContext = true;
    
    // Reset state
    useAudioPredecoder = false;
    htmlPage = "";
    outputScript = "";
    preloadAudioContent = "";
    buttonInfos.clear();
    privateMembers.clear();
    
    // Output
    String script;
    
    // Strict mode directive
    if (strictMode)
    {
        script += "\"use strict\";";
        script += newLine + newLine;
    }
    
    // Graph declaration as a module
    script += "var " + moduleName + " = (function() {";
    script += newLine;
    
    // Graph initialisation and private members
    const String initFunctionContent (generateAudioContexts (2));
    const String scriptBodies (generateScriptBodies (1));
    const String dynamicRoutes (generateDynamicRoutes (1));
    preloadAudioContent = generateAudioData (2);
    
    // Forward declaration of private members
    script += getPrivateMembersDeclaration (1);
    
    // Now function
    if (useDefaultContext)
    {
        script += getIndent (1) + "let now = function() {" + newLine;
        script += getIndent (2) + "return ctx.currentTime;" + newLine;
        script += getIndent (1) + "};" + newLine + newLine;
    }
    
    // Add button declarations
    declareButtons (script, 1);
    
    // Add script elements' functions declarations
    script += scriptBodies;
    
    // If predecoding is needed, add loadAudio function
    if (useAudioPredecoder)
        script += getPreloadDeclaration (1);
    
    // Add startAudio function
    script += getIndent (1) + "let startAudio = function() {";
    script += newLine;
    script += initFunctionContent;
    
    // End of startAudio function
    script += getIndent (1) + "};";
    script += newLine + newLine;
    
    // Add dynamic routes
    script += dynamicRoutes;
    
    // End of graph module
    script += "})();";
    
    // Generate html page
    htmlPage = getHtmlPage (getPageTitle(), controllersDivName);
    
    outputScript = script;
}

String WebAudioGraphGenerator::getCoreLibraryCode()
{
    return libraryCode;
}

String WebAudioGraphGenerator::getIndent (int indentLevel) const
{
    String indentChain;
    
    for (int i = 0; i < indentLevel; ++i)
        indentChain += indent;
    
    return indentChain;
}

void WebAudioGraphGenerator::declareButtons (String& buttonsScript, int indentLevelScript)
{
    const String ind (getIndent (indentLevelScript));
    const String ind2 (getIndent (indentLevelScript + 1));
    const String ind3 (getIndent (indentLevelScript + 2));
    
    buttonsScript += ind + "let prepareHtmlElements = function() {";
    buttonsScript += newLine;
    buttonsScript += ind2 + "var controllersDiv = document.querySelector('#" + controllersDivName + "');";
    buttonsScript += newLine;
    
    for (auto f : buttonInfos)
    {
        const String n (f->name);
        const auto colString = String ("#" + f->colour.toDisplayString (false)).quoted();
        
        buttonsScript += newLine;
        buttonsScript += ind2 + "var " + n + "Button = document.createElement ('button');" + newLine;
        buttonsScript += ind2 + n + "Button.appendChild (document.createTextNode('" + n + "'));" + newLine;
        buttonsScript += ind2 + n + "Button.onclick = function() {" + newLine;
        buttonsScript += ind3 + n + "();" + newLine;
        buttonsScript += ind2 + "}" + newLine + newLine;
        buttonsScript += ind2 + n + "Button.style.background = " + colString +";" + newLine;
        buttonsScript += ind2 + "controllersDiv.appendChild (" + n + "Button);" + newLine;
    }
    
    buttonsScript += ind + "}" + newLine + newLine;
    
    if (! useAudioPredecoder)
        buttonsScript += ind + "window.addEventListener ('load', prepareHtmlElements);" + newLine + newLine;
}

#include "WebAudioGraph.h"
#include "WebAudioNodeInstance.h"
#include "WebAudioNode.h"
#include "WebAudioMessage.h"
#include "WebAudioData.h"
#include "WebAudioContext.h"
#include "WebAudioScript.h"
#include "WebAudioDynamicRoute.h"

String WebAudioGraphGenerator::generateAudioContexts (int numIndents)
{
    const bool useDefaultContext = true;
    
    // Audio context prefix
    String output ((getIndent(numIndents) + audioContextHeader + newLine));
    
    struct DeclaredContext
    {
        const String name;
        String content;
        StringArray declaredNodes;
    };
    
    OwnedArray<DeclaredContext> declaredContexts;
    
    auto isContextDeclared = [&](String name)
    {
        for (auto ctx : declaredContexts)
            if (ctx->name == name)
                return true;
        
        return false;
    };
    
    auto getContextDeclaration = [&](String name)->DeclaredContext*
    {
        for (auto ctx : declaredContexts)
            if (ctx->name == name)
                return ctx;
        
        return nullptr;
    };
    
    auto isElementDeclared = [&](String name)->bool
    {
        for (auto ctx : declaredContexts)
            if (ctx->name == name || ctx->declaredNodes.contains (name))
                return true;
        
        return false;
    };
    
    StringArray declaredAudioElements;
    
    // Declare default context
    Array<WebAudioNode*> rootNodes;
    
    if (useDefaultContext)
    {
        auto contextDeclaration = new DeclaredContext ({ defaultContextName });
        declaredContexts.add (contextDeclaration);
        contextDeclaration->declaredNodes.add (defaultContextName + ".destination");
        
        contextDeclaration->content = newLine;
        contextDeclaration->content += getIndent (numIndents) + "// Context declaration (default context)" + newLine;
        contextDeclaration->content += getIndent (numIndents);
        contextDeclaration->content += defaultContextName + " = new AudioContext();";
        contextDeclaration->content += newLine + newLine;
        
        rootNodes = graph.getAllNodesWithoutContext (true);
        
        for (auto n : rootNodes)
        {
            const String identifier (n->getPublicName());
            const String interf (n->getInterfaceName());
            
            if (identifier.contains(".") || graph.getAllDynamicRoutesContaining (n).size() > 0)
                continue;
            
            if (! contextDeclaration->declaredNodes.contains (identifier))
            {
                contextDeclaration->content += getNodeDeclaration (n, numIndents);
                contextDeclaration->declaredNodes.add (identifier);
            }
        }
    }
    
    // Declare explicit contexts
    auto contexts = graph.getAllContexts (true);
    
    for (auto c : contexts)
    {
        // Context declaration
        const String contextIdentifier = c->getPublicName();
        DeclaredContext* contextDeclaration;
        bool firstDeclaration = true;
        
        if (! isContextDeclared (contextIdentifier))
        {
            contextDeclaration = new DeclaredContext ({ contextIdentifier });
            declaredContexts.add (contextDeclaration);
            contextDeclaration->declaredNodes.add (contextIdentifier + ".destination");
            
            contextDeclaration->content = newLine;
            contextDeclaration->content += getIndent (numIndents) + "// Context declaration (" + contextIdentifier + ")" + newLine;
            contextDeclaration->content += getIndent (numIndents);
            contextDeclaration->content += contextIdentifier + " = new " + c->getInterfaceName() + "();";
        }
        else
        {
            contextDeclaration = getContextDeclaration (contextIdentifier);
            firstDeclaration = false;
        }
        
        if (contextDeclaration == nullptr)
            continue;
        
        auto nodes = c->getAllNodes (true);
        
        String& destination = contextDeclaration->content;
        StringArray& declaredNodes = contextDeclaration->declaredNodes;
        
        // Nodes declaration
        if (firstDeclaration)
            destination += newLine;
        
        for (auto n : nodes)
        {
            const String identifier (n->getPublicName());
            const String interf (n->getInterfaceName());
            
            if (identifier.contains(".") || graph.getAllDynamicRoutesContaining(n).size() > 0)
                continue; // Attributes and dynamic nodes shouldn't be declared
            
            if (! declaredNodes.contains (identifier))
            {
                destination += getNodeDeclaration (n, numIndents);
                declaredNodes.add (n->getPublicName());
            }
        }
    }
    
    Array<ConnectionInfo> connectionsAdded;
    
    // Make connections
    for (auto c : graph.getGraphPanel().getConnections())
    {
        const auto info = c->getInfo();
        
        if (connectionsAdded.contains (info))
            continue;
        
        const String sourceName (info.sourceComp->getPublicName());
        const String destName (info.destComp->getPublicName());
        const bool destIsContextDestination = dynamic_cast<WebAudioDestinationNode*> (info.destComp);
        
        // We need to find the context name in order to get the right destination string
        String contextName;
        
        if (auto n = dynamic_cast<WebAudioNode*> (info.sourceComp))
            contextName = n->getAudioContextName();
        
        if (contextName.isEmpty() && useDefaultContext)
            contextName = defaultContextName;
        
        const auto ctxDeclaration = getContextDeclaration (contextName);
        
        if (ctxDeclaration == nullptr)
            continue;
        
        String& destination = ctxDeclaration->content;
        
        if (! ctxDeclaration->declaredNodes.contains (sourceName)
            || (! destIsContextDestination && ! ctxDeclaration->declaredNodes.contains (destName)))
            continue;
        
        destination += getConnectionDeclaration (info, numIndents);
        
        connectionsAdded.add (info);
    }

    // Concat context declarations
    for (auto declaration : declaredContexts)
        output += declaration->content;
    
    // Process messages and scripts at root
    auto sm = graph.getAllScriptsAndMessagesAtRoot (true);
    
    if (sm.size() > 0)
        output += newLine;
    
    for (auto scriptOrMsg : sm)
    {
        if (auto msg = dynamic_cast<WebAudioMessage*> (scriptOrMsg))
        {
            for (auto emb : graph.getGraphPanel().getAllConnected (msg, GraphEmbeddedComponent::Pin::PinOnLeft))
            {
                const String targetName (emb->getPublicName());
                
                if (! (isElementDeclared (targetName) || declaredAudioElements.contains (targetName)))
                    continue;
                
                output += getIndent (numIndents);
                output += emb->getPublicName() + "." + msg->getPublicName() + ";";
                output += newLine;
            }
        }
        else if (auto s = dynamic_cast<WebAudioScript*> (scriptOrMsg))
        {
            output += getIndent (numIndents);
            output += s->getPublicName() + "();";
            output += newLine;
        }
    }
    
    // So that a button is added for the graph initialisation
    buttonInfos.add (new ButtonInfo ({ "startAudio", Colours::white }));
    
    // Declare graph private members
    for (auto ctx : declaredContexts)
    {
        if (! ctx->name.contains ("."))
            privateMembers.addIfNotAlreadyThere (ctx->name);
        
        for (auto n : ctx->declaredNodes)
            if (!n.contains ("."))
                privateMembers.addIfNotAlreadyThere (n);
    }
    
    for (auto e : declaredAudioElements)
        privateMembers.addIfNotAlreadyThere (e);
    
    return output;
}

String WebAudioGraphGenerator::generateDynamicRoutes (int numIndents)
{
    struct DeclaredRoute
    {
        const String name;
        String nodeDeclarations;
        String connectionDeclarations;
        String messageDeclarations;
        StringArray declaredNodes;
        Colour colour;
    };
    
    OwnedArray<DeclaredRoute> declaredRoutes;
    
    auto isRouteDeclared = [&](String name)
    {
        for (auto f : declaredRoutes)
            if (f->name == name)
                return true;
        
        return false;
    };
    
    auto getRouteDeclaration = [&](String name)->DeclaredRoute*
    {
        for (auto f : declaredRoutes)
            if (f->name == name)
                return f;
        
        return nullptr;
    };
    
    const String ind (getIndent (numIndents));
    const String ind2 (getIndent (numIndents + 1));
    
    auto dynamicRoutes = graph.getAllDynamicRoutes (true);
    
    for (auto f : dynamicRoutes)
    {
        Array<GraphEmbeddedComponent*> nodesToConnect;
        
        const String routeName (f->getPublicName());
        const bool isAlreadyDeclared (isRouteDeclared (routeName));
        
        // Get DeclaredRoute for this element
        DeclaredRoute* routeDeclaration = nullptr;
        
        if (! isAlreadyDeclared)
        {
            routeDeclaration = new DeclaredRoute ({ routeName });
            routeDeclaration->colour = f->getBackgroundColour().withAlpha(1.0f);
            declaredRoutes.add (routeDeclaration);
        }
        else
            routeDeclaration = getRouteDeclaration (routeName);
            
        if (routeDeclaration == nullptr)
            continue;
        
        // Declare nodes for this element
        auto& nodeDeclarations = routeDeclaration->nodeDeclarations;
        auto nodes = f->getAllNodes (true);
        
        for (auto n : nodes)
        {
            if (auto dest = dynamic_cast<WebAudioDestinationNode*> (n))
                continue;
            
            const String nodeName (n->getPublicName());
            
            if (routeDeclaration->declaredNodes.contains (nodeName)
                || privateMembers.contains (nodeName))  // We don't want to 'override' nodes declared at root
                continue;
            
            nodeDeclarations += getNodeDeclaration (n, numIndents + 1);
            routeDeclaration->declaredNodes.add (nodeName);
            nodesToConnect.add (n);
        }
        
        // Connect nodes for this element
        auto& connectionDeclarations = routeDeclaration->connectionDeclarations;
        
        for (auto c : graph.getGraphPanel().getConnections())
        {
            auto info = c->getInfo();
            
            if (info.sourcePlacement == GraphEmbeddedComponent::Pin::PinOnRight)
                continue;
            
            if (nodesToConnect.contains (info.sourceComp) || nodesToConnect.contains (info.destComp))
                connectionDeclarations += getConnectionDeclaration (info, numIndents + 1);
        }
        
        // Add scripts and msgs this element
        auto& messagesDeclaration = routeDeclaration->messageDeclarations;
        
        for (auto scriptOrMsg : f->getAllScriptsAndMessages (true))
        {
            if (auto msg = dynamic_cast<WebAudioMessage*> (scriptOrMsg))
            {
                const auto msgContent = msg->getPublicName();
                
                if (msgContent.isEmpty())// || msg->getErrorMessage().isNotEmpty())
                    continue;
                
                for (auto emb : graph.getGraphPanel().getAllConnected (msg, GraphEmbeddedComponent::Pin::PinOnLeft))
                {
                    messagesDeclaration += ind2;
                    messagesDeclaration += emb->getPublicName() + "." + msgContent + ";";
                    messagesDeclaration += newLine;
                }
            }
            else if (auto f = dynamic_cast<WebAudioScript*> (scriptOrMsg))
            {
                messagesDeclaration += ind2;
                messagesDeclaration += f->getPublicName() + "();";
                messagesDeclaration += newLine;
            }
        }
    }
    
    // Merge declarations into output
    String output;
    
    for (auto declaration : declaredRoutes)
    {
        String content (declaration->nodeDeclarations);
        content += declaration->connectionDeclarations;
        
        if (declaration->connectionDeclarations.isNotEmpty() && declaration->messageDeclarations.isNotEmpty())
            content += newLine;
        
        content += declaration->messageDeclarations;
        
        if (content.isEmpty())
            continue;
        
        for (auto n : declaration->declaredNodes)
            privateMembers.addIfNotAlreadyThere(n);
        
        output += ind + "let " + declaration->name + " = function() {";
        output += newLine;
        output += content;
        output += ind + "}";
        output += newLine + newLine;
        
        buttonInfos.add (new ButtonInfo ({ declaration->name, declaration->colour }));
    }
    
    return output;
}

String WebAudioGraphGenerator::generateAudioData (int numIndents)
{
    auto dataElements = graph.getAllAudioData();
        
    String output;
    
    const String ctx ("adCtx");
    output += getIndent (numIndents) + "let " + ctx + " = new AudioContext();" + newLine + newLine;
    
    for (auto d : dataElements)
    {
        const String ind (getIndent (numIndents));
        const String ind2 (getIndent (numIndents + 1));
        
        const auto interfaceName = d->getInterfaceName();
        const auto instanceName = d->getPublicName();
        const auto instance = d->getInstance();
        
        if (instance == nullptr || privateMembers.contains (instanceName))
            continue;
        
        const auto& opt = instance->getOptions();
        
        if (interfaceName == "AudioBuffer")
        {
            output += ind + instanceName + " = " + ctx + ".createBuffer(";
            
            for (int i = 0; i < opt.size(); ++i)
            {
                if (i > 0)
                    output += ", ";
                
                const String v (opt.getUnchecked (i)->defaultValue);
                output += v.isEmpty() ? "0" : v;
            }
            
            output += String (");") + newLine + newLine;
        }
        else if (interfaceName == "DecodableAudio")
        {
            if (! useAudioPredecoder)
            {
                output += ind + "let predecoder = new AudioPredecoder();" + newLine + newLine;
                useAudioPredecoder = true;
            }
            
            output += ind + instanceName + " = new DecodableAudio (predecoder, " + ctx;
            
            for (int i = 0; i < opt.size(); ++i)
            {
                output += ", ";
                
                const String v (opt.getUnchecked (i)->defaultValue);
                output += v.isEmpty() ? "0" : v;
            }
            
            output += String (");") + newLine + newLine;
            
            useAudioPredecoder = true;
        }
        else if (interfaceName == "AudioElement")
        {
            output += ind + instanceName + " = document.createElement(\'audio\');" + newLine;
            
            for (int i = 0; i < opt.size(); ++i)
            {
                const String lineStart (ind + instanceName + ".");
                const String attrib (opt.getUnchecked(i)->name);
                const String v (opt.getUnchecked(i)->defaultValue);
                
                if (v.isNotEmpty())
                    output += lineStart + attrib + " = " + v + ";" + newLine;
            }
            
            output += ind + "document.querySelector(\"#controllers\").appendChild (" + instanceName + ");" + newLine;
        }
        
        privateMembers.add (instanceName);
    }
    
    return output;
}

String WebAudioGraphGenerator::generateScriptBodies (int numIndents) const
{
    StringArray declaredScripts;
    String output;
    
    auto scripts = graph.getAllScripts();
    
    const String ind (getIndent (numIndents));
    const String ind2 (getIndent (numIndents + 1));
    
    for (auto s : scripts)
    {
        const String scriptName (s->getPublicName());
        
        if (declaredScripts.contains (scriptName))
            continue;
        
        declaredScripts.add (scriptName);
        
        const StringArray lines (StringArray::fromLines (s->getEditorContent()));
        
        output += ind + "let " + scriptName + " = function() {" + newLine;
        
        for (auto l : lines)
            output += ind2 + l + newLine;
        
        output += ind + "};" + newLine + newLine;
    }
    
    return output;
}

String WebAudioGraphGenerator::getNodeDeclaration (const WebAudioNode* n, int numIndents) const
{
    if (n == nullptr)
        return String();
    
    auto instance = n->getInstance();
    
    if (instance == nullptr)
        return String();
    
    const String identifier (n->getPublicName());
    const String contextIdentifier (n->getAudioContext() == nullptr ? "ctx" : n->getAudioContextName());
    const String interf (n->getInterfaceName());
    
    if (interf == String())
        return String();
        
    // Constructors and options
    auto& options = instance->getOptions();
    
    String optionsString ("{");
    bool firstLine = true;
    
    for (auto o : options)
    {
        const String optionValue = o->defaultValue;
        
        if (optionValue == String())
            continue;
        
        if (! firstLine)
            optionsString += ",";
        
        optionsString += newLine;
        optionsString += getIndent (numIndents + 1) + o->name + ": " + optionValue;
        
        firstLine = false;
    }
    
    if (! firstLine)
    {
        optionsString += newLine;
        optionsString += getIndent (numIndents);
    }
    
    optionsString += "}";
    
    String declaration (getIndent (numIndents) + identifier + " = new " + interf + "(");
    declaration += contextIdentifier + ", " + optionsString + ");";
    declaration += newLine;
    declaration += newLine;
    
    return declaration;
}

String WebAudioGraphGenerator::getConnectionDeclaration (const ConnectionInfo& info, int numIndents) const
{
    if (auto destMsg = dynamic_cast<WebAudioMessage*>(info.destComp))
        return String();
    
    if (auto sourceMsg = dynamic_cast<WebAudioMessage*>(info.sourceComp))
        return String();
    
    // From node to audio param
    if (info.destPlacement == GraphEmbeddedComponent::Pin::PinOnLeft
        && info.sourcePlacement == GraphEmbeddedComponent::Pin::PinOnBottom)
    {
        auto destNode = dynamic_cast<WebAudioNode*>(info.destComp);
        
        if (destNode == nullptr)
            return String();
        
        if (auto instance = destNode->getInstance())
        {
            const auto& options = instance->getOptions();
            
            const String sourceName = info.sourceComp->getPublicName();
            const String destName = destNode->getPublicName() + "." + options[info.destIndex]->name;
            
            return getIndent (numIndents) + sourceName + ".connect(" + destName + ");" + newLine;
        }
        
        return String();
    }
    // From node output to node input
    else
    {
        // to ctx destination node
        if (auto destinationNode = dynamic_cast<WebAudioDestinationNode*> (info.destComp))
        {
            const String ctx = destinationNode->getAudioContext() == nullptr
                                ? "ctx" : destinationNode->getAudioContextName();
            const String sourceName = info.sourceComp->getPublicName();
            const String inputArg = info.destIndex > 0 ? ", " + String (info.destIndex) : "";
            const String outputArg = (info.sourceIndex > 0 || inputArg.isNotEmpty()) ? ", " + String (info.sourceIndex) : "";
            
            return getIndent (numIndents) + sourceName + ".connect(" + ctx + ".destination" + outputArg + inputArg + ");" + newLine;
        }
        
        // to a regular node
        const String sourceName = info.sourceComp->getPublicName();
        const String destName = info.destComp->getPublicName();
        const String inputArg = info.destIndex > 0 ? ", " + String (info.destIndex) : "";
        const String outputArg = (info.sourceIndex > 0 || inputArg.isNotEmpty()) ? ", " + String (info.sourceIndex) : "";
        
        return getIndent (numIndents) + sourceName + ".connect(" + destName + outputArg + inputArg + ");" + newLine;
    }
}

String WebAudioGraphGenerator::getHtmlPage (const String &pageName, String &controllersName)
{
    String s (String::createStringFromData (BinaryData::index_html, BinaryData::index_htmlSize));
    s = s.replace ("($title)", pageName);
    return s.replace ("($controllersdiv)", controllersName);
    //return s;
}

#include "Project.h"
String WebAudioGraphGenerator::getPageTitle() const
{
    const auto& proj = graph.getProject();
    return proj.isTemporary() ? "WebAudio graph" : proj.getName();
}

String WebAudioGraphGenerator::getPrivateMembersDeclaration (int numIndents) const
{
    if (privateMembers.isEmpty())
        return String();
    
    
    String str ("let ");
    
    for (int i = 0; i < privateMembers.size(); ++i)
    {
        str << privateMembers[i];
        
        if (i + 1 < privateMembers.size())
            str << ", ";
    }
    
    str << ";";
    
    return getIndent (numIndents) + str + newLine + newLine;
}


String WebAudioGraphGenerator::getPreloadDeclaration (int numIndents) const
{
    const String ind (getIndent (numIndents));
    const String ind2 (getIndent (numIndents + 1));
    
    String output;
    
    output += ind + "let loadAudio = function() {";
    output += newLine;
    output += preloadAudioContent;
    
    if (preloadAudioContent.isNotEmpty())
        output += newLine;
    
    if (useAudioPredecoder)
    {
        output += ind2 + "predecoder.onSuccess = prepareHtmlElements;" + newLine;
        output += ind2 + "predecoder.predecode();" + newLine;
    }
    
    output += ind + "}" + newLine + newLine;
    output += ind + "window.addEventListener ('load', loadAudio);" + newLine + newLine;
    
    return output;
}
