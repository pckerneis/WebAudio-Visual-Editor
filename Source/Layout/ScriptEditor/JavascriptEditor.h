/*
 ==============================================================================
 
 CodeEditor.h
 Created: 6 Jul 2017 10:26:52am
 Author:  Pierre-Clément KERNEIS
 
 ==============================================================================
 */

#pragma once

#include "../Source/Application/AppSettings.h"

class JavascriptTokeniser;

//==============================================================================
class JsCodeEditorComponent : public CodeEditorComponent
{
public:
    JsCodeEditorComponent (CodeDocument& doc, JavascriptTokeniser* tokeniser);
    ~JsCodeEditorComponent() {}
    
    void handleReturnKey() override;
    void insertTextAtCaret (const String& newText) override;
    
    void findNext (bool forwards, bool skipCurrentSelection, bool secondPass = false, Range<int> initialHightlight = Range<int>());
    
    static String getSearchString()                 { return AppSettings::getSearchString(); }
    static void setSearchString (const String& s)   { AppSettings::setSearchString (s); }
    
    static bool isCaseSensitiveSearch()             { return AppSettings::getCaseSensitiveSearch(); }
    static void setCaseSensitiveSearch (bool b)     { AppSettings::setCaseSensitiveSearch (b); }
    
private:
    int getBraceCount (String::CharPointerType line);
    String getLeadingWhitespace (String line);
    bool getIndentForCurrentBlock (CodeDocument::Position pos, const String& tab,
                                   String& blockIndent, String& lastLineIndent);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsCodeEditorComponent)
};

//==============================================================================
#include "../Source/Layout/Widgets/ShapeButtons.h"
#include "../Source/Layout/ScriptEditor/JavascriptCodeTokeniser.h"

/** \brief Javascript editor component with search bar and apply button
 */
class JsEditor :    public Component,
                    private CodeDocument::Listener,
                    public ApplicationCommandTarget
{
public:
    JsEditor();
    ~JsEditor() { masterReference.clear(); }
    
    bool getUseApplyButton() const;
    void setUseApplyButton (bool shouldUseApplyButton);
    
    void applyChanges();
    
    void setUnappliedContent (String newContent) { loadContent (newContent, false); }
    String getUnappliedContent() const { return getContent(); }
    
    void setAppliedContent (String newContent) { loadContent (newContent, true); }
    String getAppliedContent() const { return useApplyButton ? lastAppliedContent : getContent(); }
    
    bool hasChangedSinceLastApply() const { return lastAppliedContent != getContent(); }
    bool applyYesNoCancel();
    
    void loadContent (String newContent, bool applyContent = true);
    Result loadDocument (const File &file);
    bool saveToFile (const File& file);
    
    void grabKeyboardFocus();
    
    void refreshSearchBar();
    
    void setReadOnly (bool shouldBeReadOnly);
    bool isReadOnly();

    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener() { masterReference.clear(); }
        
        virtual void codeChangesApplied (JsEditor* editor) = 0;
        
    private:
        WeakReference<Listener>::Master masterReference;
        friend class WeakReference<Listener>;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    //==============================================================================
private:
    WeakReference<JsEditor>::Master masterReference;
    friend class WeakReference<JsEditor>;
    
    Array<WeakReference<Listener>> listeners;
    
    void paint (Graphics& g) override;
    void lookAndFeelChanged() override;
    void resized() override;
    
    //==============================================================================
    String getContent() const { return codeDocument.getAllContent(); }
    
    //==============================================================================
    class TopBar : public Component, private TextEditor::Listener, private Button::Listener
    {
    public:
        TopBar();
        
        void paint (Graphics& g) override;
        void resized() override;
        void buttonClicked (Button* b) override;
        
        void setFindPanelVisible (bool shouldBeVisible);
        
        JsEditor* getCodeEditor() const;
        
        void textEditorTextChanged (TextEditor& te) override;
        void textEditorReturnKeyPressed (TextEditor&) override;
        void textEditorEscapeKeyPressed (TextEditor&) override;
        
    private:
        bool isApplyButtonVisible() const;
        
        friend class JsEditor;
        
        TextEditor editor;
        Label label;
        
        ToggleButton caseButton;
        TextButton prevButton;
        TextButton nextButton;
        CloseButton closeButton;
        TextButton applyButton;
        
        Label numFoundLabel;
        
        bool findPanelVisible = false;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TopBar)
    };
    
    //==============================================================================
    int getTopBarHeight() const;
    
    void showFindPanel();
    bool barShouldBeVisible() const { return isApplyButtonVisible() || topBar.findPanelVisible; }
    bool isApplyButtonVisible() const { return useApplyButton && hasChangedSinceLastApply(); }
    void numFindResultMayHaveChanged();
    
    void setCaseSensitiveSearch (bool shouldBe);
    int getNumOccurenceOfSearchString () const;
    void findNext (bool forwards, bool skipCurrentSelection) { editor.findNext (forwards, skipCurrentSelection); }
    bool isHighlightActive() const { return editor.isHighlightActive(); }
    
    //==============================================================================
    void prepareCommandTarget();
    ApplicationCommandTarget* getNextCommandTarget() override { return findFirstTargetParentComponent(); }
    void getAllCommands (Array <CommandID>& commands) override;
    void getCommandInfo (const CommandID commandID, ApplicationCommandInfo& result) override;
    bool perform (const InvocationInfo& info) override;
    
    //==============================================================================
    // CodeDocument::Listener implementation
    void codeDocumentTextInserted (const String&, int) override   { codeDocumentChanged(); }
    void codeDocumentTextDeleted (int, int) override              { codeDocumentChanged(); }
    void codeDocumentChanged();
    
    //==============================================================================
    TopBar topBar;
    JavascriptTokeniser tokeniser;
    CodeDocument codeDocument;
    JsCodeEditorComponent editor;
    String lastAppliedContent;
    bool useApplyButton = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JsEditor)
};

//==============================================================================

#include "../Source/Application/CommandIDs.h"

class CodeEditorPanel : public Panel, public JsEditor::Listener
{
public:
    CodeEditorPanel (PanelManager* manager, String panelName);
    ~CodeEditorPanel() { masterReference.clear(); }
    
    const JsEditor& getCodeEditor() const { return jsEditor; }
    JsEditor& getCodeEditor() { return jsEditor; }
    
    void resized() override;
    bool tryToClosePanel() override;
    
    PanelTreeEmbedded::PanelPreferences getPanelPreferences() override
    {
        return PanelTreeEmbedded::PanelPreferences({  100, -1, 24, -1, 700, 850  });
    }
    
    //==============================================================================
    class Listener
    {
    public:
        Listener() {}
        virtual ~Listener() { masterReference.clear(); }
        
        virtual void codeChangesApplied (JsEditor* editor, CodeEditorPanel* panel) = 0;
        virtual void codeEditorPanelWasClosed (CodeEditorPanel* panel) = 0;
        
    private:
        WeakReference<Listener>::Master masterReference;
        friend class WeakReference<Listener>;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Listener)
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    //==============================================================================
    void codeChangesApplied (JsEditor* editor) override;
    
    void panelWasClosed() override;
    XmlElement* getAsXml() override;
    void restoreState (XmlElement* e) override;
    
private:
    WeakReference<CodeEditorPanel>::Master masterReference;
    friend class WeakReference<CodeEditorPanel>;
    
    Array<WeakReference<Listener>> listeners;
    
    //==============================================================================
    JsEditor jsEditor;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CodeEditorPanel)
};
