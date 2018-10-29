/*
  ==============================================================================

    JavascriptEditor.cpp
    Created: 7 Jul 2017 1:08:25pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "JavascriptEditor.h"

JsCodeEditorComponent::JsCodeEditorComponent (CodeDocument& doc, JavascriptTokeniser* tokeniser) : CodeEditorComponent (doc, tokeniser)
{
	const auto defaultFont = AppSettings::getCurrentEditorFont();
    setFont (defaultFont);
    setTabSize (4, true);
    setScrollbarThickness (10);
    
    auto c = LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId).overlaidWith (AppSettings::getCurrentEditorBackgroundColour());
    
    setColour (CodeEditorComponent::backgroundColourId, c);
}

void JsCodeEditorComponent::handleReturnKey()
{
    CodeEditorComponent::handleReturnKey();
    
    CodeDocument::Position pos (getCaretPos());
    
    String blockIndent, lastLineIndent;
    getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent);
    
    const String remainderOfBrokenLine (pos.getLineText());
    const int numLeadingWSChars = getLeadingWhitespace (remainderOfBrokenLine).length();
    
    if (numLeadingWSChars > 0)
        getDocument().deleteSection (pos, pos.movedBy (numLeadingWSChars));
    
    if (remainderOfBrokenLine.trimStart().startsWithChar ('}'))
        insertTextAtCaret (blockIndent);
    else
        insertTextAtCaret (lastLineIndent);
    
    const String previousLine (pos.movedByLines (-1).getLineText());
    const String trimmedPreviousLine (previousLine.trim());
    
    if ((trimmedPreviousLine.startsWith ("if ")
         || trimmedPreviousLine.startsWith ("if(")
         || trimmedPreviousLine.startsWith ("for ")
         || trimmedPreviousLine.startsWith ("for(")
         || trimmedPreviousLine.startsWith ("while(")
         || trimmedPreviousLine.startsWith ("while "))
        && trimmedPreviousLine.endsWithChar (')'))
    {
        insertTabAtCaret();
    }
}

void JsCodeEditorComponent::insertTextAtCaret (const String& newText)
{
    if (getHighlightedRegion().isEmpty())
    {
        const CodeDocument::Position pos (getCaretPos());
        
        if ((newText == "{" || newText == "}")
            && pos.getLineNumber() > 0
            && pos.getLineText().trim().isEmpty())
        {
            moveCaretToStartOfLine (true);
            
            String blockIndent, lastLineIndent;
            if (getIndentForCurrentBlock (pos, getTabString (getTabSize()), blockIndent, lastLineIndent))
            {
                CodeEditorComponent::insertTextAtCaret (blockIndent);
                
                if (newText == "{")
                    insertTabAtCaret();
            }
        }
    }
    
    CodeEditorComponent::insertTextAtCaret (newText);
}

void JsCodeEditorComponent::findNext (bool forwards, bool skipCurrentSelection, bool secondPass, Range<int> initialHightlight)
{
    const Range<int> highlight (getHighlightedRegion());
    const CodeDocument::Position startPos (getDocument(), skipCurrentSelection ? highlight.getEnd()
                                           : highlight.getStart());
    int lineNum = startPos.getLineNumber();
    int linePos = startPos.getIndexInLine();
    
    const int totalLines = getDocument().getNumLines();
    const String searchText (getSearchString());
    const bool caseSensitive = isCaseSensitiveSearch();
    
    for (int linesToSearch = totalLines; --linesToSearch >= 0;)
    {
        String line (getDocument().getLine (lineNum));
        int index;
        
        if (forwards)
        {
            index = caseSensitive ? line.indexOf (linePos, searchText)
            : line.indexOfIgnoreCase (linePos, searchText);
        }
        else
        {
            if (linePos >= 0)
                line = line.substring (0, linePos);
            
            index = caseSensitive ? line.lastIndexOf (searchText)
            : line.lastIndexOfIgnoreCase (searchText);
        }
        
        if (index >= 0)
        {
            const CodeDocument::Position p (getDocument(), lineNum, index);
            selectRegion (p, p.movedBy (searchText.length()));
            break;
        }
        
        if (forwards)
        {
            linePos = 0;
            lineNum = (lineNum + 1) % totalLines;
        }
        else
        {
            if (--lineNum < 0)
                lineNum = totalLines - 1;
            
            linePos = -1;
        }
        
        if (lineNum == 0 && !forwards)
        {
            if (secondPass)
                setHighlightedRegion (initialHightlight);
            else
            {
                setHighlightedRegion(Range<int>(getDocument().getNumCharacters(), getDocument().getNumCharacters()));
                findNext (forwards, skipCurrentSelection, true, highlight);
            }
        }
        
        if (lineNum == totalLines - 1 && forwards)
        {
            if (secondPass)
                setHighlightedRegion (initialHightlight);
            else
            {
                setHighlightedRegion(Range<int>(0, 0));
                findNext (forwards, skipCurrentSelection, true, highlight);
            }
        }
    }
}

int JsCodeEditorComponent::getBraceCount (String::CharPointerType line)
{
    int braces = 0;
    
    for (;;)
    {
        const juce_wchar c = line.getAndAdvance();
        
        if (c == 0)                         break;
        else if (c == '{')                  ++braces;
        else if (c == '}')                  --braces;
        else if (c == '/')                  { if (*line == '/') break; }
        else if (c == '"' || c == '\'')     { while (! (line.isEmpty() || line.getAndAdvance() == c)) {} }
    }
    
    return braces;
}

String JsCodeEditorComponent::getLeadingWhitespace (String line)
{
    line = line.removeCharacters ("\r\n");
    const String::CharPointerType endOfLeadingWS (line.getCharPointer().findEndOfWhitespace());
    return String (line.getCharPointer(), endOfLeadingWS);
}

bool JsCodeEditorComponent::getIndentForCurrentBlock (CodeDocument::Position pos, const String& tab,
                                                      String& blockIndent, String& lastLineIndent)
{
    int braceCount = 0;
    bool indentFound = false;
    
    while (pos.getLineNumber() > 0)
    {
        pos = pos.movedByLines (-1);
        
        const String line (pos.getLineText());
        const String trimmedLine (line.trimStart());
        
        braceCount += getBraceCount (trimmedLine.getCharPointer());
        
        if (braceCount > 0)
        {
            blockIndent = getLeadingWhitespace (line);
            if (! indentFound)
                lastLineIndent = blockIndent + tab;
            
            return true;
        }
        
        if ((! indentFound) && trimmedLine.isNotEmpty())
        {
            indentFound = true;
            lastLineIndent = getLeadingWhitespace (line);
        }
    }
    
    return false;
}

//==============================================================================

JsEditor::JsEditor() : editor (codeDocument, &tokeniser)
{
    addAndMakeVisible (editor);
    addAndMakeVisible (topBar);
    
    codeDocument.addListener (this);
    
    prepareCommandTarget();
}

bool JsEditor::getUseApplyButton() const
{
    return useApplyButton;
}

void JsEditor::setUseApplyButton (bool shouldUseApplyButton)
{
    useApplyButton = shouldUseApplyButton;
    resized();
}

void JsEditor::applyChanges()
{
    lastAppliedContent = getContent();
    resized();
    
    for (auto l : listeners)
    if (l)
    l->codeChangesApplied (this);
}

void JsEditor::paint (Graphics& g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));   // clear the background
}

bool JsEditor::saveToFile (const File &file)
{
    file.deleteFile();
    
    FileOutputStream os (file);
    const String content (getAppliedContent());
    
    if (os.openedOk())
    {
        os.writeText (content, false, false, "\r\n");
        return true;
    }
    
    return false;
}

void JsEditor::loadContent (String newContent, bool applyContent)
{
    editor.loadContent (newContent);
    
    if (applyContent)
        applyChanges();
}

Result JsEditor::loadDocument (const File &file)
{
    editor.loadContent (file.loadFileAsString());
    lastAppliedContent = file.loadFileAsString();
    
    codeDocument.setSavePoint();
    
    return Result::ok();
}

bool JsEditor::applyYesNoCancel()
{
    if (!hasChangedSinceLastApply())
        return true;
    
    const int r = NativeMessageBox::showYesNoCancelBox (AlertWindow::QuestionIcon,
                                                        TRANS("Unapplied changes!"),
                                                        TRANS("Do you want to apply changed made to the script?"));
    if (r == 1)         // Save
    {
        applyChanges();
        return true;
    }
    else if (r == 2)    // Don't save
    {
        editor.loadContent (lastAppliedContent);
        return true;
    }
    else if (r == 0)  {} // Cancel
    
    return false;
}

#include "Project.h"
void JsEditor::codeDocumentChanged()
{
    // checks if the bar needs to be shown/hidden
    resized();
    numFindResultMayHaveChanged();
    
    Project::getApplicationCommandManager().commandStatusChanged();
}

// Component
void JsEditor::resized()
{
    auto r = getLocalBounds();
    
    if (barShouldBeVisible())
    {
        const auto topArea = r.removeFromTop (getTopBarHeight());
        
        // force resized in case a section should be shown/hidden
        if (topBar.getBounds() != topArea)  topBar.setBounds (topArea);
        else                                topBar.resized();
    }
    else
        topBar.setBounds (juce::Rectangle<int>());
    
    editor.setBounds (r);
}

void JsEditor::lookAndFeelChanged()
{
    editor.setColourScheme (AppSettings::getCurrentEditorColourScheme());
    editor.setFont(AppSettings::getCurrentEditorFont().withHeight(AppSettings::getCurrentEditorFontSize()));
    
    auto c = LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId).overlaidWith (AppSettings::getCurrentEditorBackgroundColour());
    
    editor.setColour (CodeEditorComponent::backgroundColourId, c);
}

void JsEditor::showFindPanel()
{
    topBar.setFindPanelVisible (true);
    topBar.editor.grabKeyboardFocus();
    topBar.editor.selectAll();
}

int JsEditor::getTopBarHeight() const
{
    if (!barShouldBeVisible())
        return 0;
    
    if (getWidth() < 400 && topBar.findPanelVisible)
        return 52;
    
    return 28;
}

void JsEditor::numFindResultMayHaveChanged()
{
    topBar.numFoundLabel.setText (String (getNumOccurenceOfSearchString()), dontSendNotification);
}

int JsEditor::getNumOccurenceOfSearchString () const
{
    const bool ignoreCase = !editor.isCaseSensitiveSearch();
    const auto s = editor.getSearchString();
    auto content = getContent();
    
    if (s == String() || content == String())
        return 0;
    
    int count = 0;
    
    while (content != String())
    {
        content = content.fromFirstOccurrenceOf (s, false, ignoreCase);
        
        if (content != String())
            ++count;
    }
    
    return count;
}

void JsEditor::setCaseSensitiveSearch (bool shouldBe)
{
    editor.setCaseSensitiveSearch (shouldBe);
    
    numFindResultMayHaveChanged();
}

void JsEditor::prepareCommandTarget()
{
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    
    commandManager.registerAllCommandsForTarget (this);
}

void JsEditor::getAllCommands (Array <CommandID>& commands)
{
    const CommandID ids[] =
    {
        CommandIDs::showFindPanel,
        CommandIDs::findNext,
        CommandIDs::findPrevious,
        CommandIDs::applyChanges
    };
    
    commands.addArray (ids, numElementsInArray (ids));
}

void JsEditor::getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result)
{
    //const bool anythingSelected = jsEditor.isHighlightActive();
    const bool changesToApply = hasChangedSinceLastApply();
    
    switch (commandID)
    {
        case CommandIDs::showFindPanel:
            result.setInfo (TRANS ("Find..."), TRANS ("Searches for text in the current document."), "Edit", 0);
            result.defaultKeypresses.add (KeyPress ('f', ModifierKeys::commandModifier, 0));
            break;
            
        case CommandIDs::findNext:
            result.setInfo (TRANS ("Find Next"), TRANS ("Searches for the next occurrence of the current search-term."), "Edit", 0);
            result.defaultKeypresses.add (KeyPress ('g', ModifierKeys::commandModifier, 0));
            break;
            
        case CommandIDs::findPrevious:
            result.setInfo (TRANS ("Find Previous"), TRANS ("Searches for the previous occurrence of the current search-term."), "Edit", 0);
            result.defaultKeypresses.add (KeyPress ('g', ModifierKeys::commandModifier | ModifierKeys::shiftModifier, 0));
            result.defaultKeypresses.add (KeyPress ('d', ModifierKeys::commandModifier, 0));
            break;
            
        case CommandIDs::applyChanges:
            result.setInfo (TRANS ("Apply changes"), TRANS ("Apply the changes made to current file."), "Edit", 0);
            result.defaultKeypresses.add (KeyPress ('b', ModifierKeys::commandModifier, 0));
            result.setActive (changesToApply);
            break;
            
        default:
            break;
    }
}

bool JsEditor::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::showFindPanel:     showFindPanel();           return true;
        case CommandIDs::findNext:          findNext (true, true);     return true;
        case CommandIDs::findPrevious:      findNext (false, false);   return true;
        case CommandIDs::applyChanges:      applyChanges(); resized(); return true;
        default:                            break;
    }
    
    return true;
}

void JsEditor::grabKeyboardFocus()
{
    editor.grabKeyboardFocus();
}

void JsEditor::refreshSearchBar()
{
    topBar.editor.setText (JsCodeEditorComponent::getSearchString(), dontSendNotification);
    numFindResultMayHaveChanged();
    resized();
}

void JsEditor::setReadOnly (bool shouldBeReadOnly) { editor.setReadOnly (shouldBeReadOnly); }
bool JsEditor::isReadOnly() { return editor.isReadOnly(); }
//==============================================================================

JsEditor::TopBar::TopBar() : caseButton ("Case-sensitive"), prevButton ("<"), nextButton (">"), closeButton ("close"), applyButton ("Apply")
{
    addAndMakeVisible (editor);
    
    label.setText ("Find", dontSendNotification);
    label.setJustificationType(Justification::right);
    addAndMakeVisible (label);
    
    caseButton.setToggleState (JsCodeEditorComponent::isCaseSensitiveSearch(), dontSendNotification);
    caseButton.addListener (this);
    addAndMakeVisible (caseButton);
    
    prevButton.addListener (this);
    prevButton.setConnectedEdges (Button::ConnectedOnRight);
    addAndMakeVisible (prevButton);
    
    nextButton.addListener (this);
    nextButton.setConnectedEdges (Button::ConnectedOnLeft);
    addAndMakeVisible (nextButton);
    
    closeButton.addListener (this);
    closeButton.setBorderSize (BorderSize<int> (4, 6, 4, 6));
    addAndMakeVisible (closeButton);
    
    editor.setText (JsCodeEditorComponent::getSearchString());
    editor.addListener (this);
    
    applyButton.addListener (this);
    addAndMakeVisible (applyButton);
    
    addAndMakeVisible (numFoundLabel);
}

void JsEditor::TopBar::paint (Graphics& g)
{
    g.setColour (Colours::black.withMultipliedAlpha (0.15f));
    
    if (isApplyButtonVisible() && findPanelVisible)
        g.fillRect (applyButton.getX() - 5, 0, 1, getHeight());
    
    g.fillRect (getLocalBounds().removeFromBottom (1));
}

void JsEditor::TopBar::resized()
{
    auto r = getLocalBounds().reduced (5, 2);
    
    const bool applyButtonVisible = isApplyButtonVisible();
    
    if (getWidth() < 400 && findPanelVisible)
    {
        auto upLine = r.withTrimmedBottom (proportionOfHeight (0.5f));
        auto botLine = r.withTrimmedTop (proportionOfHeight (0.5f));
        
        if (applyButtonVisible)
        {
            applyButton.setBounds (upLine.removeFromRight (80).withY (r.proportionOfHeight(0.25f)));
            upLine.removeFromRight (8);
        }
        
        nextButton.setBounds (upLine.removeFromRight (20));
        prevButton.setBounds (upLine.removeFromRight (20));
        
        label.setBounds (upLine.removeFromLeft (40));
        
        
        editor.setBounds (upLine.reduced (10, 0));
        
        caseButton.setBounds (botLine.removeFromLeft (110).reduced (0, 4));
        numFoundLabel.setBounds (juce::Rectangle<int> (prevButton.getX() - 10, botLine.getY(), 30, botLine.getHeight()));
        closeButton.setBounds (juce::Rectangle<int> (nextButton.getX(), botLine.getY(), 20, botLine.getHeight()));
        
    }
    else
    {
        if (applyButtonVisible)
        {
            applyButton.setBounds (r.removeFromRight (80));
            r.removeFromRight (8);
        }
    
        if (findPanelVisible)
        {
            closeButton.setBounds (r.removeFromRight (20));
            
            const int caseWidth = jmin (110, proportionOfWidth (0.25f));
            caseButton.setBounds (r.removeFromRight (caseWidth).reduced (0, 4));
            
            nextButton.setBounds (r.removeFromRight (20));
            prevButton.setBounds (r.removeFromRight (20));
            
            label.setBounds (r.removeFromLeft (40));
            numFoundLabel.setBounds (r.removeFromRight (30));
            editor.setBounds (r.reduced (10, 0));
        }
    }
    
    if (!findPanelVisible)
    {
        const auto r = juce::Rectangle<int>();
        closeButton.setBounds (r);
        caseButton.setBounds (r);
        nextButton.setBounds (r);
        prevButton.setBounds (r);
        label.setBounds (r);
        editor.setBounds (r);
        numFoundLabel.setBounds (r);
    }
    
    if (!applyButtonVisible)
        applyButton.setBounds (juce::Rectangle<int>());
    
    repaint();
}

void JsEditor::TopBar::buttonClicked (Button* b)
{
    if (auto editor = getCodeEditor())
    {
        if (b == &nextButton)
            editor->findNext (true, true);
        else if (b == &prevButton)
            editor->findNext (false, false);
        else if (b == &caseButton)
            editor->setCaseSensitiveSearch (caseButton.getToggleState());
        else if (b == &closeButton)
            setFindPanelVisible (false);
        else if (b == &applyButton)
        {
            editor->applyChanges();
            editor->resized();
        }
    }
}

void JsEditor::TopBar::textEditorTextChanged (TextEditor& te)
{
    JsCodeEditorComponent::setSearchString (editor.getText());
    
    if (auto jsEditor = getCodeEditor())
    {
        jsEditor->findNext (true, false);
        numFoundLabel.setText (String(jsEditor->getNumOccurenceOfSearchString()), dontSendNotification);
    }
}

JsEditor* JsEditor::TopBar::getCodeEditor() const
{
    return findParentComponentOfClass <JsEditor>();
}

void JsEditor::TopBar::setFindPanelVisible (bool shouldBeVisible)
{
    if (findPanelVisible != shouldBeVisible)
    {
        findPanelVisible = shouldBeVisible;
        
        // checks if the bar needs to be shown/hidden
        if (auto jsEditor = getCodeEditor())
            jsEditor->resized();
    }
}

void JsEditor::TopBar::textEditorReturnKeyPressed (TextEditor&)
{
    JsCodeEditorComponent::setSearchString (editor.getText());
    
    Project::getApplicationCommandManager().invokeDirectly (CommandIDs::findNext, true);
    
    if (auto editor = getCodeEditor())
        numFoundLabel.setText (String(editor->getNumOccurenceOfSearchString()), dontSendNotification);
}

void JsEditor::TopBar::textEditorEscapeKeyPressed (TextEditor&)
{
    setFindPanelVisible (false);
}

bool JsEditor::TopBar::isApplyButtonVisible() const
{
    if (auto editor = getCodeEditor())
        if (! editor->getUseApplyButton())
            return false;
    
    bool b = false;
    
    if (auto codeTab = getCodeEditor())
        b = codeTab->isApplyButtonVisible();
    
    return b;
}

//==============================================================================

CodeEditorPanel::CodeEditorPanel (PanelManager* manager, String panelName) : Panel (manager)
{
    addAndMakeVisible (jsEditor);
    jsEditor.addListener (this);
    
    setPanelName (panelName);
}

void CodeEditorPanel::resized()
{
    Panel::resized();
    
    jsEditor.setBounds (getLocalBounds().withTrimmedTop (getHeaderHeight()));
}

bool CodeEditorPanel::tryToClosePanel()
{
    return (jsEditor.getUseApplyButton() && jsEditor.hasChangedSinceLastApply()) ? jsEditor.applyYesNoCancel() : true;
}

void CodeEditorPanel::codeChangesApplied (JsEditor* editor)
{
    for (auto l : listeners)
        if (l)
            l->codeChangesApplied (editor, this);
}

void CodeEditorPanel::panelWasClosed()
{
    for (auto l : listeners)
        if (l)
            l->codeEditorPanelWasClosed (this);
}

XmlElement* CodeEditorPanel::getAsXml()
{
    auto e = new XmlElement (getPanelName().isEmpty() ? "CodeEditorPanel" : getPanelName().removeCharacters (" "));
    e->setAttribute ("panelId", getPanelId());
    
    auto applied = e->createNewChildElement ("APPLIED");
    applied->addTextElement (jsEditor.getAppliedContent());
    
    auto unapplied = e->createNewChildElement ("UNAPPLIED");
    unapplied->addTextElement (jsEditor.getUnappliedContent());
    
    return e;
}

void CodeEditorPanel::restoreState (XmlElement* e)
{
    if (e == nullptr)
    return;
    
    setPanelId (e->getIntAttribute ("panelId"));
    
    if (auto app = e->getChildByName ("APPLIED"))
        if (app->getNumChildElements() > 0 && app->getChildElement(0)->isTextElement())
            jsEditor.setAppliedContent (app->getChildElement (0)->getText());
    
    if (auto unapp = e->getChildByName ("UNAPPLIED"))
        if (unapp->getNumChildElements() > 0 && unapp->getChildElement(0)->isTextElement())
            jsEditor.setUnappliedContent (unapp->getChildElement (0)->getText());
}
