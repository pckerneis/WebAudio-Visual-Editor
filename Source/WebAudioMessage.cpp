/*
  ==============================================================================

    WebAudioMessage.cpp
    Created: 27 Aug 2018 4:35:09pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebAudioMessage.h"



#include "JavascriptCodeTokeniser.h"
class QuickCodeParser
{
public:
    QuickCodeParser (const String& code)
    {
        document.insertText (0, code);
        iterator = new CodeDocument::Iterator (document);
        createTokens (0, code, *iterator, tokeniser, tokens);
    }
    
    struct SyntaxToken
    {
        // Shouldn't be left here.. TODO
        SyntaxToken () {}
        
        SyntaxToken (const String& t, const int len, const int type) noexcept
        : text (t), trimmed (t.trim()), length (len), tokenType (type)
        {}
        
        bool operator== (const SyntaxToken& other) const noexcept
        {
            return tokenType == other.tokenType
            && length == other.length
            && text == other.text;
        }
        
        String text;
        String trimmed;
        int length;
        int tokenType;
    };
    
    const Array<SyntaxToken>& getTokens() const { return tokens; }
    
private:
    static void createTokens (int startPosition, const String& lineText,
                              CodeDocument::Iterator& source,
                              CodeTokeniser& tokeniser,
                              Array<SyntaxToken>& newTokens)
    {
        CodeDocument::Iterator lastIterator (source);
        const int lineLength = lineText.length();
        
        for (;;)
        {
            int tokenType = tokeniser.readNextToken (source);
            int tokenStart = lastIterator.getPosition();
            int tokenEnd = source.getPosition();
            
            if (tokenEnd <= tokenStart)
                break;
            
            tokenEnd -= startPosition;
            
            if (tokenEnd > 0)
            {
                tokenStart -= startPosition;
                const int start = jmax (0, tokenStart);
                
                newTokens.add (SyntaxToken (lineText.substring (start, tokenEnd), tokenEnd - start, tokenType));
                
                if (tokenEnd >= lineLength)
                    break;
            }
            
            lastIterator = source;
        }
        
        source = lastIterator;
    }
    
    Array<SyntaxToken> tokens;
    
    JavascriptTokeniser tokeniser;
    CodeDocument document;
    ScopedPointer<CodeDocument::Iterator> iterator;
};

//==============================================================================
#include "WebAudioGraph.h"

WebAudioMessage::WebAudioMessage (WebAudioGraphPanel* parent) : WebAudioEmbedded (*parent, Descriptor (GraphElementType::messageType)), editor (*this), errorHighlight (*this)
{
    setZ (5);
    setSize (getDefaultWidth(), getDefaultHeight());
    
    setNumPins (1, Pin::Placement::PinOnLeft);
    
    prepareInspectablePropertiesTree (getUICompTypeName());
    
    addAndMakeVisible (editor);
    editor.addListener (this);
    setEditorEnabled (false);
    
    addChildComponent (errorHighlight);
    errorHighlight.setInterceptsMouseClicks (false, false);
    
    // Allows the text editor to give away its focus when this is clicked
    setWantsKeyboardFocus (true);
    
    constrainer.setMaximumHeight (GraphEmbeddedComponent::getDefaultHeight());
}

void WebAudioMessage::resized()
{
    WebAudioEmbedded::resized();
    
    auto top = getLocalBounds().removeFromTop (getDefaultHeight());
    
    editor.setBounds (top.reduced (5, 0));
    errorHighlight.setBounds (top);
    
    checkContainers (false);
    setSizeProperties();
}

void WebAudioMessage::setPublicName (String newName)
{
    WebAudioEmbedded::setPublicName (newName);
    
    editor.setText (newName, false);
    
    checkMessageValidity();
        
    if (tipWindowIsVisible)
        showTips (&editor);
    
    //if (! wasManuallyResized())
    adaptSizeToContent();
}

void WebAudioMessage::showNameEditor()
{
    if (canBeRenamed())
        setEditorEnabled (true);
}

void WebAudioMessage::textEditorTextChanged (TextEditor& e)
{
    if (tipWindowIsVisible)
        showTips (&editor);
    else
        startTimer (1000);
    
    if (! wasManuallyResized())
        adaptSizeToContent();
}

void WebAudioMessage::textEditorFocusLost (TextEditor& e)
{
    const String newText (e.getText());
    
    parentPanel.getUndoManager().beginNewTransaction();
    setPropertyValue (nameLabel, newText, true);
    
    WebAudioEmbedded::setPublicName (newText);
    
    setEditorEnabled (false);
    setErrorHighlightVisible (errorMessage.isNotEmpty());
    
    startTimer (200);
}

void WebAudioMessage::timerCallback()
{
    auto tipWindow = getTipWindow();
    
    const bool shouldCloseTipWindow = (tipWindow == nullptr)
                                        || ! Process::isForegroundProcess()
                                        || (! editor.hasKeyboardFocus (true) && ! tipWindow->hasKeyboardFocus (true));
    
    if (! tipWindowIsVisible && ! shouldCloseTipWindow)
    {
        tipWindowIsVisible = true;
        showTips (&editor);
        setEditorEnabled (true);
        
        return;
    }
    
    if (shouldCloseTipWindow)
    {
        hideTips();
        stopTimer();
        tipWindowIsVisible = false;
        setEditorEnabled (false);
        
        return;
    }
    
    if (tipWindowIsVisible)
        startTimer (200);
}

#include "JsCodeHelpers.h"
struct TipHelpers
{
    static Tip methodToTip (Descriptor::Method method, String prefix)
    {
        const String memberOf ("(" + method.memberOf + ") ");
        const String returnType (JsCodeHelpers::getInterfaceDisplayName (method.returns));
        String args;
        String argsDescriptions;
        
        bool first = true;
        
        for (auto a : method.args)
        {
            if (! first)
                args += ", ";
            
            argsDescriptions += newLine;
            
            args += a.name;
            argsDescriptions += " - " + a.name + " (" + a.getInterfaceDisplayName() + ") : " + a.helpText;
            
            first = false;
        }
        
        const String listText (memberOf + prefix + method.name + "(" + args + ")");
        const String shortText (prefix + method.name + "(" + args + ")");
        String helpText (method.helpText);
        
        if (method.returns.isNotEmpty())
            helpText += (" Returns : " + returnType);
        
        if (argsDescriptions.isNotEmpty())
            helpText += argsDescriptions;
        
        return Tip (shortText, listText, helpText);
    }

    static void searchTipsInInterface (String interfaceName, Array<Tip>& results, const StringArray& searchStrings, String prefix)
    {
        if (results.size() >= 100)
            return;
     
        SharedResourcePointer<WebAudioDictionary> dict;
        auto d = dict->findDescriptorForInterface (interfaceName);
        
        if (d.isUndefined())
            return;
        
        for (auto m : d.interf.methods)
        {
            Tip t = TipHelpers::methodToTip (m, prefix);
            
            if (searchStrings.size() == 0)
                results.addIfNotAlreadyThere (t);
            else
                for (auto s : searchStrings)
                    if (t.textInList.containsIgnoreCase (s))
                        results.addIfNotAlreadyThere (t);
        }
        
        if (d.interf.inheritance.isNotEmpty())
            TipHelpers::searchTipsInInterface (d.interf.inheritance, results,  searchStrings, prefix);
        
        for (auto p : d.interf.properties)
        {
            // ignore these properties to avoid cycle...
            if (d.interf.name == "AudioNode" && p.interf.name == "BaseAudioContext")
                continue;
            
            if (! prefix.contains (p.name))
                TipHelpers::searchTipsInInterface (p.interf.name, results,  searchStrings, prefix + p.name + ".");
        }
    }
};

Array<Tip> WebAudioMessage::getTips()
{
    StringArray addedInterfaces;
    Array<Tip> r;
    
    StringArray searchStrings;
    
    const auto text = editor.getText();
    QuickCodeParser parser (text);
    const auto &tokens = parser.getTokens();
    
    // Split current text into tokens
    for (auto t : tokens)
        if (t.tokenType == JavascriptTokeniser::tokenType_identifier)
            searchStrings.addIfNotAlreadyThere(t.trimmed);
    
    if (auto parentGraph = getParentGraph())
    {
        for (auto connected : parentGraph->getAllConnected (this, Pin::PinOnLeft))
        {
            if (auto webElem = dynamic_cast<WebAudioInspectableElement*> (connected.get()))
            {
                const String interf (webElem->getInterfaceName());
                
                // We don't want duplicate interfaces
                if (addedInterfaces.contains (interf))
                    continue;
                
                addedInterfaces.add (interf);
                
                TipHelpers::searchTipsInInterface (interf, r, searchStrings, "");
            }
        }
    }
    
    return r;
}

void WebAudioMessage::tipSelected (Tip tip)
{
    const String newText (tip.text);
    
    parentPanel.getUndoManager().beginNewTransaction();
    setPropertyValue (nameLabel, newText, true);
    
    setPublicName (newText);
    
    setEditorEnabled (true);
}

void WebAudioMessage::checkMessageValidity()
{
    auto r = checkMessage (getPublicName());
    setErrorHighlightVisible (r.failed());
    errorMessage = r.getErrorMessage();
}

void WebAudioMessage::setErrorHighlightVisible (bool shouldBeVisible)
{
    errorHighlight.setVisible (shouldBeVisible);
    
    if (shouldBeVisible)
        errorHighlight.toFront (false);
    
    auto f = editor.getFont();
    f.setStyleFlags (shouldBeVisible ? 2 : 0);
    editor.applyFontToAllText (f);
}

void WebAudioMessage::setBackgroundColour (Colour newColour)
{
    WebAudioEmbedded::setBackgroundColour (newColour);
    
    editor.applyColourToAllText (newColour.contrasting().withAlpha (1.0f));
    //editor.setColour (TextEditor::highlightColourId, newColour.contrasting().withAlpha (1.0f));
    //editor.setColour (TextEditor::highlightedTextColourId, newColour.withAlpha (1.0f));
}

void WebAudioMessage::setEditorEnabled (bool shouldBeEnabled)
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

String WebAudioMessage::getNameInInspector() const
{
    String shortName (getPublicName().substring (0, 22));
    
    if (shortName != getPublicName())
        shortName += "...";
    
    return "Message : " + shortName.quoted();
}

void WebAudioMessage::adaptSizeToContent()
{
    int w = editor.getFont().getStringWidth (editor.getText()) + 20;
    w = jmax (getDefaultWidth(), w);
    
    setSize (w, getDefaultHeight());
    setSizeProperties();
}

struct MessageHelpers
{
    static Result getMethodArguments (const Array<QuickCodeParser::SyntaxToken>& tokens, int startIndex, const Descriptor::Method& method, StringArray& values, int& endIndex)
    {
        if (tokens[startIndex].trimmed != "(")
            return Result::fail ("Expected \'(\' but found " + tokens[startIndex].trimmed.quoted('\''));
        
        if (method.args.size() == 0)
        {
            if (tokens[startIndex + 1].trimmed != ")")
                return Result::fail ("Method " + method.name + " doesn't expect any argument");
            
            endIndex = startIndex + 2;
            
            return Result::ok();
        }
        else
        {
            int argIndex = startIndex;
            bool endOfMethod = false;
            int numLeftParen = 0;
            
            for (auto arg : method.args)
            {
                ++ argIndex;
                
                const String argName (arg.name);
                String argValue;
                
                if (tokens[argIndex].trimmed == ",")
                    return Result::fail ("Expected arg \'" + argName + "\' but found " + tokens[argIndex].trimmed.quoted('\''));
                
                bool previousWasIdentifierOrValue = false;
                
                for (;;)
                {
                    if (argIndex >= tokens.size())
                        break;
                    
                    auto t = tokens[argIndex];
                    
                    if (t.trimmed == ";")
                        return Result::fail ("Unexpected token ';'");
                    
                    if (t.trimmed == "," && numLeftParen == 0)
                        break;
                    
                    if (t.trimmed == "(")
                        ++numLeftParen;
                    
                    if (t.trimmed == ")")
                    {
                        --numLeftParen;
                        
                        if (numLeftParen < 0)
                        {
                            endOfMethod = true;
                            break;
                        }
                    }
                    
                    const bool isIdentifier = (t.tokenType == JavascriptTokeniser::tokenType_identifier);
                    
                    const bool isNumber = t.tokenType == JavascriptTokeniser::tokenType_float
                                        || t.tokenType == JavascriptTokeniser::tokenType_integer;
                    
                    const bool isString = (t.tokenType == JavascriptTokeniser::tokenType_string);
                    
                    const bool isIdentifierOrValue = (isIdentifier || isString || isNumber);
                    
                    if (previousWasIdentifierOrValue && isIdentifierOrValue)
                        return Result::fail ("Unexpected token " + t.trimmed.quoted('\''));
                    
                    previousWasIdentifierOrValue = isIdentifierOrValue;
                    
                    argValue += t.trimmed;
                    ++argIndex;
                }
                
                values.add (argValue);
                
                if (endOfMethod)
                    break;
            }
            
            if (! endOfMethod)
            {
                if (argIndex >= tokens.size())
                    return Result::fail ("Missing \')\'.");
                
                return Result::fail ("Expected \')\' but found " + tokens[argIndex].trimmed.quoted('\''));
            }
            
            endIndex = argIndex + 2;
            
            return Result::ok();
        }
    }

    static Result searchTargetInInterface (String interfaceName, const Array<QuickCodeParser::SyntaxToken>& tokens, int startIndex, String notFound)
    {
        SharedResourcePointer<WebAudioDictionary> dict;
        
        Descriptor descriptor (dict->findDescriptorForInterface(interfaceName));
        
        if (descriptor.isUndefined() || startIndex >= tokens.size())
            return Result::fail (notFound);
        
        const String tokenText = tokens[startIndex].trimmed;
        
        Array<Descriptor::Method> suitableMethods;
        
        // Check own and inherited methods
        for (auto m : descriptor.interf.methods)
            if (m.name == tokenText)
                suitableMethods.add (m);
        
        for (int i = 0; i < suitableMethods.size(); ++i)
        {
            auto m = suitableMethods[i];
            
            if (m.name == tokenText)
            {
                StringArray values;
                int endIndex;
                
                // Get the user specified arguments
                auto r = MessageHelpers::getMethodArguments (tokens, startIndex + 1, m, values, endIndex);
                
                // This may have failed if there were arguments specified whereas the method doesn't
                // expect any. If so, we still need to check if there's another suitable method with
                // the correct number of arguments.
                if (r.failed())
                {
                    if (i == suitableMethods.size() - 1)
                        return r;
                    else
                        continue;
                }
                
                // Check extra tokens
                if (endIndex < tokens.size())
                {
                    if (tokens[endIndex].trimmed != ";")
                        return Result::fail ("Unexpected token " + tokens[endIndex].trimmed.quoted('\''));
                    else if (endIndex + 1 < tokens.size())
                        return Result::fail ("Unexpected token " + tokens[endIndex + 1].trimmed.quoted('\''));
                }
                
                // Check num required arguments
                const int numSpecifiedArgs = values.size();
                
                if (numSpecifiedArgs < m.getNumRequiredArguments())
                {
                    const String argName = m.args[numSpecifiedArgs].name;
                    return Result::fail ("Argument " + argName.quoted('\'') + " should be specified.");
                }
                
                return Result::ok();
            }
        }
        
        // Check attributes
        for (auto a : descriptor.interf.properties)
        {
            if (a.name == tokenText)
            {
                if (tokens[startIndex + 1].trimmed == ".")
                {
                    auto r = MessageHelpers::searchTargetInInterface (a.interf.name, tokens, startIndex + 2, notFound);
                    
                    if (r.wasOk() || r.getErrorMessage() != notFound)
                        return r;
                }
            }
        }
        
        auto r = MessageHelpers::searchTargetInInterface (descriptor.interf.inheritance, tokens, startIndex, notFound);
        
        if (r.wasOk() || r.getErrorMessage() != notFound)
            return r;
        
        return Result::fail (notFound);
    }
};

Result WebAudioMessage::checkMessage (String msg)
{
    if (getParentGraph() == nullptr || msg.trim().isEmpty())
        return Result::ok();
    
    using TokenType = JavascriptTokeniser::TokenType;
    
    QuickCodeParser parser (msg);
    const auto &tokens = parser.getTokens();
    
    for (auto t : tokens)
        if (t.tokenType == TokenType::tokenType_error)
            return Result::fail ("Error : invalid symbol \'" + t.text + "\' found.");
    
    if (tokens.size() < 3  // A minimal message should be composed of at least <identifier> + '(' + ')'
        || tokens[0].tokenType != TokenType::tokenType_identifier)
        return Result::fail ("Can't find suitable target for message " + msg.quoted() + ".");
    
    StringArray connected;
    
    for (auto c : getParentGraph()->getAllConnected (this, Pin::PinOnLeft))
        if (auto webElem = dynamic_cast<WebAudioInspectableElement*> (c.get()))
            connected.addIfNotAlreadyThere (webElem->getInterfaceName());
    
    SharedResourcePointer<WebAudioDictionary> dict;
    
    for (auto interfaceName : connected)
    {
        auto r = MessageHelpers::searchTargetInInterface (interfaceName, tokens, 0, "NOTFOUND");
        
        if (r.wasOk() || r.getErrorMessage() != "NOTFOUND")
            return r;
    }
    
    return Result::fail ("Can't find suitable target for message " + msg.quoted() + ".");
}

ValueTree WebAudioMessage::createNameProperty()
{
    return createEditableProperty ("message",  ComponentTypes::messageType, "", "");
}

//==============================================================================

WebAudioMessage::CustomTextEditor::CustomTextEditor (WebAudioMessage& o) : owner (o)
{
    setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    setColour (TextEditor::outlineColourId, Colours::transparentBlack);
}

bool WebAudioMessage::CustomTextEditor::keyPressed (const KeyPress &key)
{
    auto tipComponent = owner.getGraphTipComponent();
    
    if (tipComponent == nullptr)
        return TextEditor::keyPressed (key);
    
    const int currentIndex = tipComponent->getCurrentTipIndex();
    
    if (key == KeyPress::downKey)
        tipComponent->setCurrentTipIndex (currentIndex + 1, false);
    else if (key == KeyPress::upKey)
        tipComponent->setCurrentTipIndex (currentIndex - 1, false);
    else if (key == KeyPress::returnKey)
    {
        if (currentIndex >= 0)
            tipComponent->setCurrentTipIndex (currentIndex, true);
        else
        {
            owner.textEditorFocusLost (*this);
            
            if (auto p = owner.getParentGraph())
                p->grabKeyboardFocus();
        }
    }
    
    return TextEditor::keyPressed (key);
}

void WebAudioMessage::CustomTextEditor::focusGained (FocusChangeType cause)
{
    TextEditor::focusGained (cause);
    
    owner.startTimer (2000);
    owner.setErrorHighlightVisible (false);
}

void WebAudioMessage::CustomTextEditor::focusLost (FocusChangeType cause)
{
    //TextEditor::focusLost (cause);
    
    // We filter to avoid having the editor disabled when the tip window is showing
    if (cause == FocusChangeType::focusChangedByMouseClick)
        owner.textEditorFocusLost (*this);
}
