/*
  ==============================================================================

    WebAudioScript.h
    Created: 14 Oct 2018 10:38:38pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"

class WebAudioNodeInstance;
class CodeEditorPanel;

struct FunctionEditor
{
    FunctionEditor (WebAudioNodeInstance* i, CodeEditorPanel* e);
    ~FunctionEditor();
    
    WeakReference<WebAudioNodeInstance> instance;
    ScopedPointer<CodeEditorPanel> editor;
};

//==============================================================================
class FunctionEditorManager
{
public:
    FunctionEditorManager() {}
    
    CodeEditorPanel* findEditorForInstance (WebAudioNodeInstance* instance);
    void addEditorForInstance (CodeEditorPanel* editor, WebAudioNodeInstance* instance);
    void removeUnusedEditors();
    
private:
    OwnedArray<FunctionEditor> editors;
};

//==============================================================================
#include "WebAudioInspectableElement.h"

class WebAudioScript : public WebAudioFoldable
{
public:
    WebAudioScript (WebAudioGraphPanel& parent);
    ~WebAudioScript();
    
    bool isWebAudioElementEnabled() const override;
    String getUICompTypeName() const override;
        
    void setOptionValue (String optionName, String newValue) override {}
    
    void setPublicName (String newName) override;
    String getDefaultName() const override;
    
    void showEditorInTabs();
    void hideEditorPanel();
    
    void wasRemovedFromGraph() override;
    void deleteSelectedItem() override;
    
    CodeEditorPanel* getCodeEditorPanel() const;
    String getEditorContent() const;
    
    void addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos) override;
    void handleExtraPopupMenuCommands (int result, Point<int> pos) override;
    
private:
    bool editorHasContent() const;
    
    String getEditorPanelName() const;
    
    WeakReference<CodeEditorPanel> editorPanel;
    SharedResourcePointer<FunctionEditorManager> editorsManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioScript)
};
