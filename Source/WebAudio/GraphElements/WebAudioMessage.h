/*
  ==============================================================================

    WebAudioMessage.h
    Created: 27 Aug 2018 4:35:09pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "WebAudioGraphElementTypeNames.h"
#include "WebAudioInspectableElement.h"
#include "PropertyComponentTypes.h"
#include "GraphTipComponent.h"

class WebAudioGraphPanel;
class Tip;

class WebAudioMessage : public WebAudioEmbedded,
                        public TextEditor::Listener,
                        public GraphTipClient,
                        public Timer,
                        public TooltipClient
{
public:
    WebAudioMessage (WebAudioGraphPanel* parent);
    
    void resized() override;
    
    String getUICompTypeName() const override { return GraphElementType::messageType; }
    void setPublicName (String newName) override;
    String getValidName (String wantedName) override { return wantedName; }
    void showNameEditor() override;
    String getDefaultName() const override { return "message"; }
    
    void textEditorTextChanged (TextEditor &e) override;
    void textEditorFocusLost (TextEditor &e) override;
    
    Array<Tip> getTips() override;
    void tipSelected (Tip tip) override;
    String getTooltip() override { return errorMessage; }
   
    void checkMessageValidity();
    void setErrorHighlightVisible (bool shouldBeVisible);
    String getErrorMessage() const { return errorMessage; }
    
    void wasConnected() override { checkMessageValidity(); }
    void wasDisconnected() override { checkMessageValidity(); }
    
    bool isWebAudioElementEnabled() const override { return errorMessage.isEmpty(); }
    void setBackgroundColour (Colour newColour) override;
    
private:
    ValueTree createNameProperty() override;
    
    void timerCallback() override;
    
    void setEditorEnabled (bool shouldBeEnabled);
    String getNameInInspector() const override;

    void adaptSizeToContent();
    
    Result checkMessage (String msg);
    
    //==============================================================================
    class CustomTextEditor : public TextEditor
    {
    public:
        CustomTextEditor (WebAudioMessage& o);
        
        bool keyPressed (const KeyPress &key) override;
        void focusGained (FocusChangeType cause) override;
        void focusLost (FocusChangeType cause) override;
        
    private:
        WebAudioMessage& owner;
    };
    
    //==============================================================================
    class ErrorHighlight : public Component
    {
    public:
        ErrorHighlight (WebAudioMessage& o) : owner (o) {}
        
        void paint (Graphics &g)
        {
            const int thickness = 2;
            g.setColour (owner.getBackgroundColour().contrasting (0.8f).withAlpha (0.5f));
            g.fillRect (getLocalBounds().withSizeKeepingCentre (getWidth() - 12, thickness));
        }
        
    private:
        WebAudioMessage& owner;
    };
    //==============================================================================

    CustomTextEditor editor;
    ErrorHighlight errorHighlight;
    SharedResourcePointer<TooltipWindow> tooltipWindow;
    String errorMessage;
    bool tipWindowIsVisible = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioMessage)
};
