/*
  ==============================================================================

    GraphTipComponent.cpp
    Created: 4 Sep 2018 4:35:31pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "GraphTipComponent.h"

Tip::Tip (String t, String listText, String longDescription) : text (t), textInList (listText), description (longDescription) {}

bool Tip::operator== (const Tip &other) const
{
    return text == other.text
        && textInList == other.textInList
        && description == other.description;
}

//==============================================================================

TipWindow::TipWindow() : tipList (*this)
{
    viewport.setViewedComponent (&tipList, false);
    viewport.setScrollBarsShown (true, false);
    addAndMakeVisible (viewport);
}

void TipWindow::setCurrentTipIndex (int newIndex, bool shouldScrollToMakeItemVisible)
{
    if (tips.size() == 0)
        newIndex = 0;
    else
        newIndex %= tips.size();
    
    if (newIndex < 0)
        newIndex = tips.size() - 1;
    
    tipList.setCurrentIndex (newIndex);
    
    if (shouldScrollToMakeItemVisible)
    {
        const int itemTop = newIndex * elementHeight;
        const int itemBottom = itemTop + elementHeight;
        
        const int currentOffset = viewport.getViewPositionY();
        
        if (itemTop - currentOffset < 0)
        {
            viewport.setViewPosition (0, itemTop);
        }
        else if (itemBottom - currentOffset > viewport.getViewHeight())
        {
            viewport.setViewPosition (0, itemBottom - viewport.getViewHeight());
        }
    }
    
    adaptSize();
    repaint();
}

void TipWindow::setTipsAndSize (const Array<Tip>& newTips)
{
    tips = newTips;
    
    adaptSize();
    repaint();
}

void TipWindow::adaptSize()
{
    Font f (elementHeight * 0.85f);
    
    const int newListHeight = tips.size() * elementHeight;
    int newWidth = -1;
    
    // Find biggest element width
    for (auto t : tips)
    {
        auto w = f.getStringWidth (t.textInList);
        
        if (newWidth < 0 || newWidth < w)
            newWidth = w;
    }
    
    Font footerFont (elementHeight * 0.7f);
    
    // Prepare footer text with the new width
    longDescription.clear();
    longDescription.addFittedText (footerFont, tips[tipList.getCurrentIndex()].description, 0, 0, newWidth - 8, 80, Justification::topLeft, 10, 0.9f);
    footerHeight = longDescription.getBoundingBox (0, -1, true).getHeight() + 10;
    
    if (newWidth > 0)
    {
        tipList.setSize (newWidth, newListHeight);
        setSize (newWidth + 6, jmin (newListHeight, maxHeight) + footerHeight);
    }
}

void TipWindow::paint (Graphics &g)
{
    // Background fill
    const Colour bgColour (getLookAndFeel().findColour (TextButton::buttonOnColourId));
    g.setColour (bgColour.withAlpha (0.8f));
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 4);
    
    // Footer separator
    g.setColour (bgColour.contrasting().withAlpha (0.2f));
    
    if (helpOnTop)
        g.fillRect (getLocalBounds().removeFromTop (footerHeight).removeFromBottom (1));
    else
        g.fillRect (getLocalBounds().removeFromBottom (footerHeight).removeFromTop (1));
    
    // Footer fill
    const int footerY = helpOnTop ? 0 : getHeight() - footerHeight;
        
    Path p;
    p.addRoundedRectangle (0., footerY, getWidth(), footerHeight,
                           4., 4.,
                           helpOnTop, helpOnTop, ! helpOnTop, ! helpOnTop);
    g.fillPath (p);
    
    // Footer text
    g.setColour (bgColour.contrasting());
    longDescription.draw (g, AffineTransform::translation (4, footerY + 4));
}

void TipWindow::resized()
{
    if (helpOnTop)
        viewport.setBounds (getLocalBounds().withTrimmedTop (footerHeight));
    else
        viewport.setBounds (getLocalBounds().withTrimmedBottom (footerHeight));
}

void TipWindow::setClient (GraphTipClient* newClient)
{
    client = newClient;
}

void TipWindow::setHelpOnTop (bool shouldBeOnTop)
{
    helpOnTop = shouldBeOnTop;
}

//==============================================================================

TipWindow::TipList::TipList (TipWindow& o) : currentTipSelected (-1), owner (o)
{}

void TipWindow::TipList::paint (Graphics &g)
{
    const Colour bgColour (getLookAndFeel().findColour (TextButton::buttonOnColourId));
    const Colour textColour (bgColour.contrasting());
    
    auto r = getLocalBounds().reduced (3, 0);
    
    for (int i = 0; i < owner.tips.size(); ++i)
    {
        auto area = r.removeFromTop (owner.elementHeight);
        
        // Highlight selected
        if (i == currentTipSelected)
        {
            g.setColour (textColour.withAlpha (0.22f));
            g.fillRect (area);
        }
        
        // Text
        g.setColour (textColour);
        g.drawText (owner.tips[i].textInList, area, Justification::left);
    }
}

void TipWindow::TipList::mouseDown (const MouseEvent& e)
{
    owner.setCurrentTipIndex (getIndexForPosition (e.getPosition()), true);
}

void TipWindow::TipList::mouseDrag (const MouseEvent& e)
{
    owner.setCurrentTipIndex (getIndexForPosition (e.getPosition()), true);
}

void TipWindow::TipList::mouseUp (const MouseEvent& e)
{
    // Notify client
    if (e.mouseWasClicked())
        if (owner.client != nullptr)
            owner.client->tipSelected (owner.tips[currentTipSelected]);
}

int TipWindow::TipList::getCurrentIndex() const
{
    return currentTipSelected;
}

void TipWindow::TipList::setCurrentIndex (int newIndex)
{
    currentTipSelected = newIndex;
    repaint();
}

int TipWindow::TipList::getIndexForPosition (Point<int> position) const
{
    const int numTips = owner.tips.size();
    const double y = (double)position.getY() / (double)getHeight();
    return y * numTips;
}

//==============================================================================

GraphTipComponent::GraphTipComponent()
{
    setInterceptsMouseClicks (false, false);
}

GraphTipComponent::~GraphTipComponent()
{
    masterReference.clear();
}

void GraphTipComponent::showTipsFor (GraphTipClient* client, Component* targetComp)
{
    currentTarget = targetComp;
    
    auto tips = client->getTips();
    
    if (tips.size() == 0)
        return;
    
    window.tipList.setCurrentIndex (-1);
    window.setTipsAndSize (tips);
    
    if (! window.isOnDesktop())
    {
        window.addToDesktop (ComponentPeer::StyleFlags::windowIsTemporary
                             | ComponentPeer::StyleFlags::windowHasDropShadow);
        
        window.setAlwaysOnTop (true);
        window.setVisible (true);
    }
    
    window.setClient (client);
    setWindowPosition();
}

void GraphTipComponent::hideTips()
{
    // We're using the unchecked method so that the value doesn't get constrained
    window.tipList.setCurrentIndex (-1);
    window.repaint();
    window.viewport.setViewPosition (0, 0);
    
    window.removeFromDesktop();
    window.setVisible (false);
}

int GraphTipComponent::getCurrentTipIndex() const
{
    return window.tipList.getCurrentIndex();
}

void GraphTipComponent::setCurrentTipIndex (int newIndex, bool notifyClient)
{
    window.setCurrentTipIndex (newIndex, true);
    setWindowPosition();
    
    if (notifyClient && window.client)
        window.client->tipSelected (window.tips[newIndex]);
}

TipWindow& GraphTipComponent::getTipWindow()
{
    return window;
}

void GraphTipComponent::setWindowPosition()
{
    if (currentTarget == nullptr)
        return;
    
    // Determine the window's position based on available space
    const juce::Rectangle<int> userArea = Desktop::getInstance().getDisplays().getMainDisplay().userArea;
    const Point<int> targetBottomLeft = currentTarget->getScreenBounds().getBottomLeft();
    const Point<int> positionIfOnTop = currentTarget->getScreenBounds().getPosition().translated(0, - window.getHeight());
    
    const bool windowOnTop = targetBottomLeft.getY() + window.getHeight() > userArea.getBottom();
    const auto windowPos = windowOnTop ? positionIfOnTop : targetBottomLeft;
    
    ComponentBoundsConstrainer constrainer;
    constrainer.setMinimumOnscreenAmounts (0xffffff, 0xffffff, 0xffffff, 0xffffff);
    constrainer.setBoundsForComponent (&window, window.getBounds().withPosition (windowPos), false, false, false, false);
    
    window.setHelpOnTop (windowOnTop);
    window.resized();
    window.repaint();
}

//==============================================================================

GraphTipClient::~GraphTipClient()
{
    masterReference.clear();
}

void GraphTipClient::showTips (Component* target)
{
    if (graphTipComponent)
        graphTipComponent->showTipsFor (this, target);
}

void GraphTipClient::hideTips()
{
    if (graphTipComponent)
        graphTipComponent->hideTips();
}

GraphTipComponent* GraphTipClient::getGraphTipComponent() const
{
    return graphTipComponent;
}

void GraphTipClient::setGraphTipComponent (GraphTipComponent* tipComp)
{
    graphTipComponent = tipComp;
}

TipWindow* GraphTipClient::getTipWindow()
{
    if (graphTipComponent)
        return &graphTipComponent->getTipWindow();
    
    return nullptr;
}

