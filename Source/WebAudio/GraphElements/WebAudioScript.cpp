/*
  ==============================================================================

    WebAudioScript.cpp
    Created: 14 Oct 2018 10:38:38pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/GraphElements/WebAudioScript.h"


#include "../Source/Layout/ScriptEditor/JavascriptEditor.h"
#include "../WebAudioGraph/WebAudioNodeInstance.h"

FunctionEditor::FunctionEditor (WebAudioNodeInstance* i, CodeEditorPanel* e) : instance (i), editor (e)
{}

FunctionEditor::~FunctionEditor()
{
    if (auto panelParentTree = editor->getParentTree())
    panelParentTree->findAndRemovePanel (editor);
}

//==============================================================================

CodeEditorPanel* FunctionEditorManager::findEditorForInstance (WebAudioNodeInstance* instance)
{
    for (auto e : editors)
    if (e->instance == instance)
    return e->editor.get();
    
    return nullptr;
}

void FunctionEditorManager::addEditorForInstance (CodeEditorPanel* editor, WebAudioNodeInstance* instance)
{
    jassert (editor != nullptr || instance != nullptr);
    
    editors.add (new FunctionEditor (instance, editor));
}

void FunctionEditorManager::removeUnusedEditors()
{
    for (int i = editors.size(); --i >= 0;)
    {
        auto instance = editors.getUnchecked (i)->instance;
        
        if (instance == nullptr)
        {
            editors.remove (i);
            continue;
        }
        
        if (instance->getNumReferencesCurrentlyInGraph() == 0)
        editors.remove (i);
    }
}

//==============================================================================
WebAudioScript::WebAudioScript (WebAudioGraphPanel& parent) : WebAudioFoldable (parent, Descriptor (GraphElementType::scriptType))
{
    setZ (5);
    setSize (getDefaultWidth(), getDefaultHeight());
    
    //prepareInspectablePropertiesTree (getUICompTypeName());
    
    prepareLabel();
    label->setJustificationType (Justification::centred);
    
    setOpenButtonVisible (false);
}

WebAudioScript::~WebAudioScript()
{
    editorsManager->removeUnusedEditors();
}

bool WebAudioScript::isWebAudioElementEnabled() const
{
    return true;
}

String WebAudioScript::getUICompTypeName() const
{
    return GraphElementType::scriptType;
}

#include "../WebAudioGraph/WebAudioGraph.h"
void WebAudioScript::setPublicName (String newName)
{
    const String oldName (getPublicName());
    
    if (oldName == newName)
    {
        WebAudioEmbedded::setPublicName (oldName);
        return;
    }
    
    bool shouldLoadPreviousContent = false;
    
    auto& instanceManager = getInstanceManager();
    const auto sourceInstance = instanceManager.findInstanceWithName (oldName);
    const auto destInstance = instanceManager.findInstanceWithName (newName);
    
    if (editorHasContent() && sourceInstance)
    {
        // Current instance will be lost if there's at least one destination instance and
        // this is the last reference to the source instance
        const int numRefSource = sourceInstance->getNumReferencesCurrentlyInGraph();
        
        if (destInstance)
        {
            const int numRefDest = destInstance->getNumReferencesCurrentlyInGraph();
            const bool currentInstanceWillBeLost = numRefSource == 1 && numRefDest > 0;
            
            if (currentInstanceWillBeLost)
            {
                const bool r = NativeMessageBox::showYesNoBox (AlertWindow::WarningIcon, "Code will be lost!", "Code associated with " + oldName.quoted() + " will be lost. Do you want to proceed anyway ?");
                
                if (! r)
                {
                    // Make sure label's text is updated to match old value
                    setPublicName (oldName);
                    return;
                }
            }
            
            // 'numRefDest == 0' should never be true but I leave it here in case instance's implementation changes
            shouldLoadPreviousContent = (numRefDest == 0);
        }
        else
        shouldLoadPreviousContent = true;
    }
    
    WebAudioFoldable::setPublicName (newName);
    
    const String previousContent (getEditorContent());
    
    // Close previous editor if needed
    if (editorPanel != nullptr && editorPanel->Panel::isShowing() && sourceInstance == nullptr)
    {
        hideEditorPanel();
    }
    
    // update code editor ref/content
    editorsManager->removeUnusedEditors();
    
    if (auto editor = editorsManager->findEditorForInstance (getInstance()))
    {
        editorPanel = editor;
    }
    else
    {
        auto panelManager = parentPanel.getPanelManager();
        
        editorPanel = new CodeEditorPanel (panelManager, getEditorPanelName());
        
        if (shouldLoadPreviousContent)
        editorPanel->getCodeEditor().loadContent (previousContent);
        
        editorsManager->addEditorForInstance (editorPanel, getInstance());
        
        showEditorInTabs();
    }
}

String WebAudioScript::getDefaultName() const
{
    return "script";
}

#include "../Source/Project/Project.h"
void WebAudioScript::showEditorInTabs()
{
    if (editorPanel == nullptr)
    return;
    
    // This panel could be showing somewhere else...
    hideEditorPanel();
    
    // Add to default (which should be a specialised subclass of code editor panel!)
    if (auto targetSibling = parentPanel.getProject().findStaticPanelWithClass<CodeEditorPanel>())
    {
        if (auto parentTabbedPanel = targetSibling->getParentTabbedPanel())
        {
            parentTabbedPanel->addPanelAsTab (editorPanel);
        }
        else if (auto parentNode = targetSibling->findParentComponentOfClass<PanelNode>())
        {
            // Else, create a tabbed panel and add editor
            if (auto parentTabbedPanel = parentNode->replaceContentWithTabbedPanel())
            parentTabbedPanel->addPanelAsTab (editorPanel);
        }
    }
}

void WebAudioScript::hideEditorPanel()
{
    if (editorPanel != nullptr)
    if (auto panelParentTree = editorPanel->getParentTree())
    panelParentTree->findAndRemovePanel (editorPanel);
}

void WebAudioScript::wasRemovedFromGraph()
{
    editorsManager->removeUnusedEditors();
}

void WebAudioScript::deleteSelectedItem()
{
    if (editorHasContent() && getInstance() != nullptr && getInstance()->getNumReferencesCurrentlyInGraph() == 1)
    {
        const bool r = NativeMessageBox::showYesNoBox (AlertWindow::WarningIcon, "Code will be lost!", "Code associated with " + getPublicName().quoted() + " will be lost. Do you want to proceed anyway ?");
        
        if (r)
        GraphEmbeddedComponent::deleteSelectedItem();
    }
    else
    {
        GraphEmbeddedComponent::deleteSelectedItem();
    }
}
CodeEditorPanel* WebAudioScript::getCodeEditorPanel() const
{
    return editorPanel;
}

String WebAudioScript::getEditorContent() const
{
    return editorPanel ? editorPanel->getCodeEditor().getAppliedContent() : String();
}

void WebAudioScript::addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos)
{
    WebAudioEmbedded::addExtraPopupMenuCommands (m, pos);
    
    m.addSeparator();
    m.addItem (81, "Show script", editorPanel != nullptr);
}

void WebAudioScript::handleExtraPopupMenuCommands (int result, Point<int> pos)
{
    if (result == 81)
    {
        if (editorPanel == nullptr)
        return;
        
        if (! editorPanel->isShowing())
        showEditorInTabs();
        else
        editorPanel->reveal();
    }
    else
    WebAudioFoldable::handleExtraPopupMenuCommands (result, pos);
}

bool WebAudioScript::editorHasContent() const
{
    return getEditorContent().isNotEmpty();
}

String WebAudioScript::getEditorPanelName() const
{
    return getPublicName() + " (code editor)";
}

//==============================================================================
