/*
  ==============================================================================

    WebAudioComment.cpp
    Created: 22 Oct 2018 3:11:34pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioComment.h"

#include "WebAudioGraph.h"

WebAudioComment::WebAudioComment (WebAudioGraphPanel* parent) : WebAudioEmbedded (*parent, Descriptor (GraphElementType::commentType))
{
    setZ (5);
    setSize (getDefaultWidth() * 1.8f, getDefaultHeight());
    
    prepareInspectablePropertiesTree (getUICompTypeName());
    
    addAndMakeVisible (editor);
    editor.addListener (this);
    setEditorEnabled (false);
    editor.setMultiLine (true);
    editor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    editor.setReturnKeyStartsNewLine (true);
    
    setBackgroundColour (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting());
    setResizableBorderHidden (true);
    
    // Allows the text editor to give away its focus when this is clicked
    setWantsKeyboardFocus (true);
}

#include "AppSettings.h"
void WebAudioComment::paint (Graphics &g)
{
    const bool h = isHighlighted();
    const bool s = GraphEmbeddedComponent::isSelected();
    
    float strokeAlpha = 0.5f;
    if (h)  strokeAlpha += 0.2f;
    if (s)  strokeAlpha += 0.2f;
    
    const Colour strokeColour = s ? AppSettings::getCurrentMainColour()
                                : Colours::transparentBlack;
    
    float stroke = 0.5f;
    if (h)  stroke += 0.5f;
    if (s)  stroke += 1.2f;
    
    paintUI (g, getLocalBounds().reduced(2));
    
    g.setColour (strokeColour);
    g.drawRect (getLocalBounds().toFloat().reduced (1.0f), stroke);
}

void WebAudioComment::resized()
{
    WebAudioEmbedded::resized();
    
    editor.setBounds (getLocalBounds());
}

void WebAudioComment::setPublicName (String newName)
{
    WebAudioEmbedded::setPublicName (newName);
    
    editor.setText (newName, false);
}

void WebAudioComment::showNameEditor()
{
    if (canBeRenamed())
        setEditorEnabled (true);
}

void WebAudioComment::textEditorFocusLost (TextEditor& e)
{
    const String newText (e.getText());
    
    parentPanel.getUndoManager().beginNewTransaction();
    setPropertyValue (nameLabel, newText, true);
    
    WebAudioEmbedded::setPublicName (newText);
    
    setEditorEnabled (false);
}

void WebAudioComment::setBackgroundColour (Colour newColour)
{
    WebAudioEmbedded::setBackgroundColour (newColour);
    
    editor.applyColourToAllText (newColour);
}

void WebAudioComment::setEditorEnabled (bool shouldBeEnabled)
{
    if (shouldBeEnabled)
    {
        editor.setWantsKeyboardFocus (true);
        editor.grabKeyboardFocus();
        editor.setInterceptsMouseClicks (true, true);
        editor.setReadOnly (false);
    }
    else
    {
        editor.setWantsKeyboardFocus (false);
        editor.setInterceptsMouseClicks (false, false);
        editor.setReadOnly (true);
    }
}

String WebAudioComment::getNameInInspector() const
{
    String shortName (getPublicName().substring (0, 22));
    
    if (shortName != getPublicName())
        shortName += "...";
    
    return "Comment : " + shortName.quoted();
}

#include "PropertyComponentTypes.h"
ValueTree WebAudioComment::createNameProperty()
{
    return createEditableProperty ("comment",  ComponentTypes::messageType, "", "");
}
