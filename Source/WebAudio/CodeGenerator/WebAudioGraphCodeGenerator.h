/*
  ==============================================================================

    WebAudioGraphCodeGenerator.h
    Created: 20 Aug 2018 1:11:38am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class WebAudioGraphPanel;
class WebAudioNode;
class ConnectionInfo;

class WebAudioGraphGenerator
{
public:
    WebAudioGraphGenerator (WebAudioGraphPanel& webAudioGraph);
    
    void generate();
    
    const String& getGeneratedScript() const { return outputScript; }
    const String& getGeneratedHtmlPage() const { return htmlPage; }
    bool isUsingCoreLibrary() const { return useAudioPredecoder; }
    
    String getCoreLibraryCode();
    
private:
    String getIndent (int indentLevel) const;
    
    String generateAudioContexts (int numIndents);
    String generateDynamicRoutes (int numIndents);
    String generateAudioData (int numIndents);
    String generateScriptBodies (int numIndents) const;

    void declareButtons (String& buttonsScript, int indentLevelScript);
    String getNodeDeclaration (const WebAudioNode* n, int numIndents) const;
    String getConnectionDeclaration (const ConnectionInfo& info, int numIndents) const;
    String getPrivateMembersDeclaration (int numIndents) const;
    String getPreloadDeclaration (int numIndents) const;
    
    String getHtmlPage (const String &pageName, String &controllersName);
    String getPageTitle() const;

    WebAudioGraphPanel& graph;
    
    String libraryCode;
    const String defaultContextName = "ctx";
    String moduleName = "graph";
    String controllersDivName = "controllers";
    String htmlPage;
    String outputScript;
    StringArray privateMembers;
    bool useAudioPredecoder;
    String preloadAudioContent;
    
    struct ButtonInfo
    {
        const String name;
        const Colour colour;
    };
    
    OwnedArray<ButtonInfo> buttonInfos;
    
    const String audioContextHeader = "var AudioContext = window.AudioContext || window.webkitAudioContext;";
    const String indent = "   ";
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioGraphGenerator)
};
