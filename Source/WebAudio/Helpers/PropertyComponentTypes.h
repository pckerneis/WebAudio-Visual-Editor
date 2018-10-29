/*
  ==============================================================================

    PropertyComponentTypes.h
    Created: 4 Sep 2018 12:29:51am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

namespace ComponentTypes
{
#define DECLARE_ID(name) const juce::String name (#name);
    
    DECLARE_ID (textType)
    DECLARE_ID (identifierType)
    
    DECLARE_ID (boolType)
    DECLARE_ID (intType)
    DECLARE_ID (floatType)
    DECLARE_ID (doubleType)
    DECLARE_ID (uintType)
    DECLARE_ID (ulongType)
    
    DECLARE_ID (messageType)
    DECLARE_ID (elementName)
    DECLARE_ID (colourType)
    
#undef DECLARE_ID
}

struct IntValueConstrainer : public PropertyValueConstrainer
{
    virtual ~IntValueConstrainer() {}
    String constrain (String value) override
    {
        return String (value.getIntValue());
    }
};

struct LongValueConstrainer : public PropertyValueConstrainer
{
    virtual ~LongValueConstrainer() {}
    String constrain (String value) override
    {
        return String (value.getLargeIntValue());
    }
};

struct DoubleValueConstrainer : public PropertyValueConstrainer
{
    virtual ~DoubleValueConstrainer() {}
    String constrain (String value) override
    {
        return String (value.getDoubleValue());
    }
};

