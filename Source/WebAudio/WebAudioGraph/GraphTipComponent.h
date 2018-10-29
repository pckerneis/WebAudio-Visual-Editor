/*
  ==============================================================================

    GraphTipComponent.h
    Created: 4 Sep 2018 4:35:31pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

struct Tip
{
    Tip() {}
    Tip (String t, String listText, String longDescription);
    
    bool operator== (const Tip &other) const;
    
    String text;
    String textInList;
    String description;
};

//==============================================================================
class GraphTipClient;

class TipWindow : public Component
{
public:
    TipWindow();
    
    void setCurrentTipIndex (int newIndex, bool shouldScrollToMakeItemVisible);
    void setTipsAndSize (const Array<Tip>& newTips);
    
    void paint (Graphics &g) override;
    void resized() override;
    
    void setClient (GraphTipClient* newClient);
    
    void setHelpOnTop (bool shouldBeOnTop);
    
private:
    void adaptSize();
    
    //==============================================================================
    class TipList : public Component
    {
    public:
        TipList (TipWindow& o);
        
        void paint (Graphics &g) override;
        void mouseDown (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        
        int getCurrentIndex() const;
        void setCurrentIndex (int newIndex);
        
    private:
        int getIndexForPosition (Point<int> position) const;
        
        int currentTipSelected;
        
        TipWindow &owner;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TipList)
    };
    
    //==============================================================================
    TipList tipList;
    Viewport viewport;
    
    friend class TipList;
    friend class GraphTipComponent;
    
    Array<Tip> tips;
    const int elementHeight = 18;
    const int maxHeight = 140;
    int footerHeight = 20;
    bool helpOnTop = false;
    
    GlyphArrangement longDescription;
    
    WeakReference<GraphTipClient> client;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TipWindow)
};

//==============================================================================
class GraphTipComponent :   public Component
{
public:
    GraphTipComponent();
    ~GraphTipComponent();
    
    void showTipsFor (GraphTipClient* client, Component* targetComp);
    void hideTips();
    
    int getCurrentTipIndex() const;
    void setCurrentTipIndex (int newIndex, bool notifyClient);
    
    TipWindow& getTipWindow();
    
private:
    void setWindowPosition();
    
    WeakReference<GraphTipComponent>::Master masterReference;
    friend class WeakReference<GraphTipComponent>;
    
    TipWindow window;
    WeakReference<Component> currentTarget;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GraphTipComponent)
};

//==============================================================================
class GraphTipClient
{
public:
    virtual ~GraphTipClient();
    
    virtual Array<Tip> getTips() = 0;
    virtual void tipSelected (Tip tip) = 0;
    
    void showTips (Component* target);
    void hideTips();
    
    GraphTipComponent* getGraphTipComponent() const;
    void setGraphTipComponent (GraphTipComponent* tipComp);
    
    TipWindow* getTipWindow();
    
private:
    WeakReference<GraphTipClient>::Master masterReference;
    friend class WeakReference<GraphTipClient>;
    
    WeakReference<GraphTipComponent> graphTipComponent;
};
