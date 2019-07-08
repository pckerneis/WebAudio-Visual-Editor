/*
  ==============================================================================

    OutputScriptPanel.h
    Created: 23 Oct 2018 8:22:47pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/ScriptEditor/JavascriptEditor.h"

class OutputScriptPanel : public CodeEditorPanel
{
public:
    OutputScriptPanel (PanelManager* manager) : CodeEditorPanel (manager, "Script output")
    {
        getCodeEditor().setReadOnly (true);
    }
    
    XmlElement* getAsXml() override
    {
        auto e = CodeEditorPanel::getAsXml();
        
        e->setTagName ("OutputScriptPanel");
        
        return e;
    }
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputScriptPanel)
};

