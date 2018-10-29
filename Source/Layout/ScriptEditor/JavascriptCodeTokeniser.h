/*
  ==============================================================================

    JavascriptCodeTokeniser.h
    Created: 2 Nov 2017 10:22:47pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class JavascriptTokeniser   : public CodeTokeniser
{
public:
    JavascriptTokeniser() {}
    ~JavascriptTokeniser() {}
    
    static CodeEditorComponent::ColourScheme getDefaultEditorColourScheme();
    static CodeEditorComponent::ColourScheme getDefaultEditorColourSchemeBright();
    CodeEditorComponent::ColourScheme getDefaultColourScheme() override;
    
    int readNextToken (CodeDocument::Iterator& source) override;
    
    enum TokenType
    {
        tokenType_error = 0,
        tokenType_comment,
        tokenType_keyword,
        tokenType_operator,
        tokenType_identifier,
        tokenType_integer,
        tokenType_float,
        tokenType_string,
        tokenType_bracket,
        tokenType_punctuation,
        tokenType_special
    };
    
private:
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JavascriptTokeniser)
};



struct JsTokeniserFunctions
{
    static bool isIdentifierStart (const juce_wchar c) noexcept;
    static bool isIdentifierBody (const juce_wchar c) noexcept;
    static bool isSpecialKeyword (String::CharPointerType token, const int tokenLength) noexcept;
    static bool isReservedKeyword (String::CharPointerType token, const int tokenLength) noexcept;
    
    template <typename Iterator>
    static int parseIdentifier (Iterator& source) noexcept
    {
        int tokenLength = 0;
        String::CharPointerType::CharType possibleIdentifier [100];
        String::CharPointerType possible (possibleIdentifier);
        
        while (isIdentifierBody (source.peekNextChar()))
        {
            const juce_wchar c = source.nextChar();
            
            if (tokenLength < 20)
                possible.write (c);
            
            ++tokenLength;
        }
        
        if (tokenLength > 1 && tokenLength <= 16)
        {
            possible.writeNull();
            
            if (JsTokeniserFunctions::isReservedKeyword (String::CharPointerType (possibleIdentifier), tokenLength))
                return JavascriptTokeniser::tokenType_keyword;
            else if (JsTokeniserFunctions::isSpecialKeyword (String::CharPointerType (possibleIdentifier), tokenLength))
                return JavascriptTokeniser::tokenType_special;
        }
        
        return JavascriptTokeniser::tokenType_identifier;
    }
};
