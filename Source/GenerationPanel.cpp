/*
  ==============================================================================

    GenerationPanel.cpp
    Created: 25 Aug 2018 9:13:34pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "GenerationPanel.h"

CodeGenerationPanel::CodeGenerationPanel (Project& proj) : Panel (&proj.getPanelManager()), generateButton ("Generate"), publishButton ("Publish"), project (proj)
{
    setPanelName ("GeneratorPanel");
    
    addAndMakeVisible (publishButton);
    addAndMakeVisible (generateButton);
    
    publishButton.setTooltip ("Test in browser");
    generateButton.setTooltip ("Generate output files");
    
    publishButton.addListener (this);
    generateButton.addListener (this);
}

void CodeGenerationPanel::resized()
{
    const int buttonWidth = 30;
    
    if (getWidth() > getHeight())
    {
        juce::Rectangle<int> r = getLocalBounds().withSizeKeepingCentre(2 * buttonWidth, buttonWidth);
        
        generateButton.setBounds (r.removeFromLeft (buttonWidth).reduced (8));
        publishButton.setBounds (r.reduced (8));
    }
    else
    {
        juce::Rectangle<int> r = getLocalBounds().withSizeKeepingCentre(buttonWidth, buttonWidth * 2);
        
        generateButton.setBounds (r.removeFromTop (buttonWidth).reduced (8));
        publishButton.setBounds (r.reduced (8));
    }
    
    Panel::resized();
}

void CodeGenerationPanel::paint (Graphics &g)
{
    g.fillAll (getLookAndFeel().findColour (ResizableWindow::backgroundColourId));
}

PanelTreeEmbedded::PanelPreferences CodeGenerationPanel::getPanelPreferences()
{
    const bool isHorizontal = getWidth() > getHeight();
    
    if (isHorizontal)
        return PanelTreeEmbedded::PanelPreferences({
            30, -1,
            30, 30,
            30, 30
        });
    else
        return PanelTreeEmbedded::PanelPreferences({
            30, 30,
            30, -1,
            30, 30
        });
}

void CodeGenerationPanel::buttonClicked (Button* b)
{
    auto graphPanel = project.findStaticPanelWithClass<RootWebAudioGraphPanel>();
    
    if (graphPanel == nullptr)
        return;
    
    if (b == &generateButton)
        graphPanel->generateOutput (true);
    else if (b == &publishButton)
        graphPanel->testInBrowser();
}
