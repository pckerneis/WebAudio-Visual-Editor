/*
  ==============================================================================

    WebAudioComment.h
    Created: 22 Oct 2018 3:11:34pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"
#include "WebAudioInspectableElement.h"


class WebAudioComment : public WebAudioEmbedded,
                        public TextEditor::Listener
{
public:
    WebAudioComment (WebAudioGraphPanel* parent);
    
    void paint (Graphics &g) override;
    void resized() override;
    
    String getUICompTypeName() const override { return GraphElementType::commentType; }
    void setPublicName (String newName) override;
    String getValidName (String wantedName) override { return wantedName; }
    void showNameEditor() override;
    String getDefaultName() const override { return "comment"; }
    
    //void textEditorTextChanged (TextEditor &e) override;
    void textEditorFocusLost (TextEditor &e) override;
    
    bool isWebAudioElementEnabled() const override { return true; }
    void setBackgroundColour (Colour newColour) override;
    
private:
    ValueTree createNameProperty() override;
    
    void setEditorEnabled (bool shouldBeEnabled);
    String getNameInInspector() const override;
    
    TextEditor editor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioComment)
};
