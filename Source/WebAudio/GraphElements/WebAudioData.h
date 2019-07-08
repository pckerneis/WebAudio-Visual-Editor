/*
  ==============================================================================

    WebAudioData.h
    Created: 28 Aug 2018 10:03:56pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Helpers/WebAudioGraphElementTypeNames.h"
#include "../Source/WebAudio/GraphElements/WebAudioInspectableElement.h"
#include "../AudioFilesPanel/AudioFilesPanel.h"

class WebAudioGraphPanel;
class Project;

class WebAudioData : public WebAudioFoldable, public SettableTooltipClient
{
public:
    WebAudioData (WebAudioGraphPanel* parent, Descriptor& descr);
    ~WebAudioData();
    
    String getUICompTypeName() const override { return GraphElementType::audioDataType; }
    
    void setOptionValue (String optionName, String newValue) override;
    
    void setSourceFromAudioFile (AudioFileItem* item);
    void audioFileValidityChanged (bool isNowValid);
    
    bool isWebAudioElementEnabled() const override { return isValid; }
    
    AudioFileItem* getLinkedAudioFile() const { return linkedFile; }
    
private:
    WeakReference<WebAudioData>::Master masterReference;
    friend class WeakReference<WebAudioData>;
    
    void setValid (bool isNowValid);
    
    bool isValid = true;
    
    AudioFileItem::Ptr linkedFile;
    
    SharedResourcePointer<TooltipWindow> tooltipWindow;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioData)
};




