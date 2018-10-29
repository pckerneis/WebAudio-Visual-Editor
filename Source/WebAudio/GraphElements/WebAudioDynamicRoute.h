/*
  ==============================================================================

    WebAudioDynamicRoute.h
    Created: 26 Aug 2018 9:21:12pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"
#include "WebAudioInspectableElement.h"
#include "NavigationPanel.h"

class WebAudioMessage;
class WebAudioNode;
class WebAudioScript;

class WebAudioDynamicRoute :    public WebAudioContainer
{
public:
    WebAudioDynamicRoute (WebAudioGraphPanel* parent);
    ~WebAudioDynamicRoute();
    
    String getUICompTypeName() const override { return GraphElementType::dynamicRouteType; }
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    
    int getDefaultWidth() const override { return 200; }
    int getDefaultHeight() const override { return 140; }
    
    Array<WebAudioMessage*> getAllMessages (bool sorted = false) const;
    Array<WebAudioScript*> getAllScripts (bool sorted = false) const;
    Array<WebAudioNode*> getAllNodes (bool sorted = false) const;
    Array<WebAudioEmbedded*> getAllScriptsAndMessages (bool sorted = false) const;
    
    void checkContainers (bool useHighlight) override;
    String getValidName (String wantedName) override;
    String getDefaultName() const override { return "dynamicRoute"; }
    void paint (Graphics &g) override;
    void editorShown (Label* label, TextEditor&) override;
    void addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos) override;
    void handleExtraPopupMenuCommands (int result, Point<int> pos) override;
    void setBackgroundColour (Colour newColour) override;
    
    bool hitTest (int x, int y) override;
    
private:
    WeakReference<WebAudioDynamicRoute>::Master masterReference;
    friend class WeakReference<WebAudioDynamicRoute>;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebAudioDynamicRoute)
};
