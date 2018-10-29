/*
  ==============================================================================

    JavascriptCodeTokeniser.cpp
    Created: 2 Nov 2017 10:22:47pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "JavascriptCodeTokeniser.h"


bool JsTokeniserFunctions::isIdentifierStart (const juce_wchar c) noexcept
{
    return CharacterFunctions::isLetter (c)
    || c == '_' || c == '$';
}
    
bool JsTokeniserFunctions::isIdentifierBody (const juce_wchar c) noexcept
{
    return CharacterFunctions::isLetterOrDigit (c)
    || c == '_' || c == '$';
}
    
bool JsTokeniserFunctions::isSpecialKeyword (String::CharPointerType token, const int tokenLength) noexcept
{
    static const char* const keywords4Char[] =
    { "Math", nullptr };
    
    static const char* const keywords5Char[] =
    { "print", nullptr };
    
    static const char* const keywords7Char[] =
    { "println", nullptr };
    
    static const char* const keywordsOther[] =
    { "addInstruction", "clearConsole", "parseInt", "duration", nullptr };
    
    const char* const* k;
    
    switch (tokenLength)
    {
        case 4:     k = keywords4Char; break;
        case 5:     k = keywords5Char; break;
        case 7:     k = keywords7Char; break;
            
        default:
            if (tokenLength < 2 || tokenLength > 16)
                return false;
            
            k = keywordsOther;
            break;
    }
    
    for (int i = 0; k[i] != 0; ++i)
        if (token.compare (CharPointer_ASCII (k[i])) == 0)
            return true;
    
    return false;
}
    
bool JsTokeniserFunctions::isReservedKeyword (String::CharPointerType token, const int tokenLength) noexcept
{
    static const char* const keywords2Char[] =
    { "if", "do", "in", "of", nullptr };
    
    static const char* const keywords3Char[] =
    { "for", "new", "var", "let", "try", nullptr };
    
    static const char* const keywords4Char[] =
    { "case", "else", "null", "eval", "true", "this", "void", "with", nullptr };
    
    static const char* const keywords5Char[] =
    { "break", "false", "while", "catch", "class" "const", "super", "throw", "yield", nullptr };
    
    static const char* const keywords6Char[] =
    { "return", "typeof", "export", "import", "switch", nullptr };
    
    static const char* const keywordsOther[] =
    { "function", "undefined", "continue", "debugger", "default", "delete", "extends", 
		"finally", "instanceof", nullptr };
    
    const char* const* k;
    
    switch (tokenLength)
    {
        case 2:     k = keywords2Char; break;
        case 3:     k = keywords3Char; break;
        case 4:     k = keywords4Char; break;
        case 5:     k = keywords5Char; break;
        case 6:     k = keywords6Char; break;
            
        default:
            if (tokenLength < 2 || tokenLength > 16)
                return false;
            
            k = keywordsOther;
            break;
    }
    
    for (int i = 0; k[i] != 0; ++i)
        if (token.compare (CharPointer_ASCII (k[i])) == 0)
            return true;
    
    return false;
}

//==============================================================================

CodeEditorComponent::ColourScheme JavascriptTokeniser::getDefaultEditorColourScheme()
{
    static const CodeEditorComponent::ColourScheme::TokenType types[] =
    {
        { "Error",          Colour (Colours::red) },
        { "Comment",        Colour (Colours::grey) },
        { "Keyword",        Colour (Colours::skyblue) },
        { "Operator",       Colour (Colours::white) },
        { "Identifier",     Colour (Colours::white) },
        { "Integer",        Colour (Colours::lightseagreen) },
        { "Float",          Colour (Colours::lightseagreen) },
        { "String",         Colour (Colours::orange) },
        { "Bracket",        Colour (Colours::white) },
        { "Punctuation",    Colour (Colours::white) },
        { "Special",        Colour (Colours::lightsteelblue) }
    };
    
    CodeEditorComponent::ColourScheme cs;
    
    for (unsigned int i = 0; i < sizeof (types) / sizeof (types[0]); ++i)
        cs.set (types[i].name, types[i].colour);
    
    return cs;
}

CodeEditorComponent::ColourScheme JavascriptTokeniser::getDefaultEditorColourSchemeBright()
{
    static const CodeEditorComponent::ColourScheme::TokenType types[] =
    {
        { "Error",          Colour (Colours::red) },
        { "Comment",        Colour (Colours::grey) },
        { "Keyword",        Colour (Colours::skyblue) },
        { "Operator",       Colour (Colours::black) },
        { "Identifier",     Colour (Colours::black) },
        { "Integer",        Colour (Colours::lightseagreen) },
        { "Float",          Colour (Colours::lightseagreen) },
        { "String",         Colour (Colours::orange) },
        { "Bracket",        Colour (Colours::black) },
        { "Punctuation",    Colour (Colours::black) },
        { "Special",        Colour (Colours::lightsteelblue) }
    };
    
    CodeEditorComponent::ColourScheme cs;
    
    for (unsigned int i = 0; i < sizeof (types) / sizeof (types[0]); ++i)
        cs.set (types[i].name, types[i].colour);
    
    return cs;
}

#include "AppSettings.h"
CodeEditorComponent::ColourScheme JavascriptTokeniser::getDefaultColourScheme()
{
    return AppSettings::getCurrentEditorColourScheme();
}


int JavascriptTokeniser::readNextToken (CodeDocument::Iterator& source)
{
    source.skipWhitespace();
    
    const juce_wchar firstChar = source.peekNextChar();
    
    switch (firstChar)
    {
        case 0:
            break;
            
        case '0':   case '1':   case '2':   case '3':   case '4':
        case '5':   case '6':   case '7':   case '8':   case'9':
        case '.':
        {
            int result = CppTokeniserFunctions::parseNumber (source);
            
            if (result == tokenType_error)
            {
                source.skip();
                
                if (firstChar ==  '.')
                    return tokenType_punctuation;
            }
            
            return result;
        }
            
        case ',':   case ';':   case ':':
            source.skip();
            return tokenType_punctuation;
            
        case '(':   case ')':
        case '{':   case '}':
        case '[':   case ']':
            source.skip();
            return tokenType_bracket;
            
        case '"':
        case '\'':
            CppTokeniserFunctions::skipQuotedString (source);
            return tokenType_string;
            
        case '+':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '+', '=');
            return tokenType_operator;
            
        case '-':
        {
            source.skip();
            int result = CppTokeniserFunctions::parseNumber (source);
            
            if (result == tokenType_error)
            {
                CppTokeniserFunctions::skipIfNextCharMatches (source, '-', '=');
                return tokenType_operator;
            }
            
            return result;
        }
            
        case '*':   case '%':
        case '=':   case '!':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return tokenType_operator;
            
        case '/':
        {
            source.skip();
            
            if (source.peekNextChar() == '/')
            {
                source.skipToEndOfLine();
                return tokenType_comment;
            }
            
            if (source.peekNextChar() == '*')
            {
                source.skip();
                CppTokeniserFunctions::skipComment (source);
                return tokenType_comment;
            }
            
            if (source.peekNextChar() == '=')
                source.skip();
            
            return tokenType_operator;
        }
            
        case '>':   case '<':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, firstChar);
            CppTokeniserFunctions::skipIfNextCharMatches (source, firstChar);
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return tokenType_operator;
            
        case '|':   case '&':   case '^':
            source.skip();
            CppTokeniserFunctions::skipIfNextCharMatches (source, firstChar);
            CppTokeniserFunctions::skipIfNextCharMatches (source, '=');
            return tokenType_operator;
            
        case '~':   case '?':
            source.skip();
            return tokenType_operator;
            
        default:
            if (JsTokeniserFunctions::isIdentifierStart (firstChar))
                return JsTokeniserFunctions::parseIdentifier (source);
            
            source.skip();
            break;
    }
    
    return tokenType_error;
}
