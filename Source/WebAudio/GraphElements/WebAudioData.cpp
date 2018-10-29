/*
  ==============================================================================

    WebAudioData.cpp
    Created: 28 Aug 2018 10:03:56pm  ment KERNEIS

  ==============================================================================
*/

#include "WebAudioData.h"

#include "JsCodeHelpers.h"
WebAudioData::WebAudioData (WebAudioGraphPanel* parent, Descriptor& descr) : WebAudioFoldable (*parent, descr)
{
    setZ (5);
    setSize (getDefaultWidth(), getDefaultHeight());
    
    prepareInspectablePropertiesTree (getUICompTypeName());
    
    prepareLabel();
    label->setJustificationType (Justification::centred);
}

WebAudioData::~WebAudioData()
{
    getInstanceManager().removeReference (this);
    
    masterReference.clear();
}

void WebAudioData::setOptionValue (String optionName, String newValue)
{
    WebAudioFoldable::setOptionValue (optionName, newValue);
    
    if (getInterfaceName() != "DecodableAudio")
        return;
    
    if (optionName == "useFileChooser")
    {
        const bool usingFileChooser = newValue.getIntValue() == true;
        
        if (auto item = optionsTree.findPropertyItemWithName ("url"))
            item->setEnabled (! usingFileChooser);
        
        if (auto item = optionsTree.findPropertyItemWithName ("autoDecode"))
            item->setEnabled (! usingFileChooser);
    }
}

void WebAudioData::setSourceFromAudioFile (AudioFileItem* item)
{
    const auto url = item == nullptr ? "" : item->getDataUrl().quoted();
    
    if (getInterfaceName() == "AudioElement")
        setOptionValue ("src", url);
    else if (getInterfaceName() == "DecodableAudio")
        setOptionValue ("url", url);
    
    if (auto p = optionsTree.findPropertyItemWithName ("url"))
        p->setEnabled (item == nullptr);
    
    else if (auto p = optionsTree.findPropertyItemWithName ("src"))
        p->setEnabled (item == nullptr);
    
    if (auto p = optionsTree.findPropertyItemWithName ("useFileChooser"))
    {
        if (item != nullptr)
            p->setValue ("0");
        
        p->setEnabled (item == nullptr);
    }
    
    linkedFile = item;
    
    if (linkedFile != nullptr)
        setValid (linkedFile->hasValidSource());
    else
        setValid (false);
}

void WebAudioData::audioFileValidityChanged (bool isNowValid)
{
    setValid (isNowValid);
}

void WebAudioData::setValid (bool isNowValid)
{
    if (isValid != isNowValid)
    {
        isValid = isNowValid;
        
        if (label != nullptr)
        {
            auto c = getLookAndFeel().findColour(Label::textColourId);
            label->setColour (Label::textColourId, isValid ? c : c.withMultipliedBrightness (0.5f));
            
            Font f (label->getFont());
            f.setStyleFlags (isValid ? Font::plain : Font::italic);
            label->setFont (f);
        }
        
        setTooltip (isValid ? "" : "Invalid audio file.");
        
        navigableChanged();
    }
}
