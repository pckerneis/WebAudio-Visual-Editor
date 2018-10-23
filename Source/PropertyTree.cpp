/*
  ==============================================================================

    PropertyTree.cpp
    Created: 19 Aug 2018 4:20:49am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "PropertyTree.h"

PropertyItemComponent::PropertyItemComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, PropertyValueConstrainer* c) : tree (propertyTree), item (propItem), constrainer (c), propertyName (name)
{
    setInterceptsMouseClicks (true, true);
}

PropertyItemComponent::~PropertyItemComponent()
{
    masterReference.clear();
}

void PropertyItemComponent::propertyValueChanged (String oldValue)
{
    if (oldValue == getTextValue())
        return;
    
    for (auto l : listeners)
        l->propertyItemValueChanged (this, getTextValue(), oldValue);
}

void PropertyItemComponent::setConstrainer (PropertyValueConstrainer* c)
{
    constrainer = c;
}

void PropertyItemComponent::checkValueAndNotify (String oldValue)
{
    if (getTextValue() == oldValue)
        return;
    
    if (constrainer == nullptr)
    {
        propertyValueChanged (oldValue);
        return;
    }
    
    auto constrained = constrainer->constrain (getTextValue());
    
    if (constrained != getTextValue())
        setTextValue (constrained, false);
    
    propertyValueChanged (oldValue);
}

void PropertyItemComponent::mouseUp (const MouseEvent &e)
{
    if (e.mouseWasClicked())
        item.setSelected (true, true);
}

void PropertyItemComponent::setItemSelected (bool shouldBeSelected)
{
    item.setSelected (shouldBeSelected, shouldBeSelected);
}

//==============================================================================
PropertyItemBooleanComponent::PropertyItemBooleanComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, bool value) : PropertyItemComponent (propertyTree, propItem, name)
{
    addAndMakeVisible (button);
    button.addListener (this);
}
void PropertyItemBooleanComponent::resized()
{
    auto right = getLocalBounds().withTrimmedLeft (labelWidth);
    button.setBounds (right.removeFromLeft (getHeight()).reduced (0, 2));
}

String PropertyItemBooleanComponent::getTextValue() const
{
    return String (int (button.getToggleState()));
}

void PropertyItemBooleanComponent::setTextValue (String txt, bool notify)
{
    button.setToggleState (txt.getIntValue(), notify ? sendNotificationAsync : dontSendNotification);
}

void PropertyItemBooleanComponent::buttonClicked (Button* b)
{
    setItemSelected (true);
    checkValueAndNotify (String (int (! b->getToggleState())));
}

//==============================================================================

PropertyItemTextComponent::PropertyItemTextComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, String value, String attributes, PropertyValueConstrainer* c) : PropertyItemComponent (propertyTree, propItem, name, c), editor (*this), oldValue (value)
{
    addAndMakeVisible (editor);
    editor.setText (value);
    editor.addListener (this);
    
    if (attributes.contains ("readonly"))
        editor.setReadOnly (true);
    
    if (attributes.contains ("multiline"))
    {
        editor.setMultiLine (true);
        propItem.setItemHeight (42);
        propItem.treeHasChanged();
    }
    
    if (attributes.contains ("multiple"))
        setShowsMultipleValues (true);
    
    editor.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    editor.setColour (TextEditor::focusedOutlineColourId, propertyTree.getTextColour().withMultipliedAlpha (0.1f));
}

void PropertyItemTextComponent::resized()
{
    editor.setBounds (getLocalBounds().withTrimmedLeft (labelWidth));
}

String PropertyItemTextComponent::getTextValue() const
{
    return editor.getText();
}

void PropertyItemTextComponent::setTextValue (String txt, bool sendNotification)
{
    editor.setText (txt, sendNotification);
    oldValue = editor.getText();
}

TextEditor& PropertyItemTextComponent::getEditor()
{
    return editor;
}

void PropertyItemTextComponent::textEditorReturnKeyPressed (TextEditor &e)
{
    if (oldValue == editor.getText() || getShowsMultipleValues())
        return moveKeyboardFocusToSibling (true);
    
    checkValueAndNotify (oldValue);
    oldValue = editor.getText();
    
    moveKeyboardFocusToSibling (true);
}

void PropertyItemTextComponent::textEditorEscapeKeyPressed (TextEditor &e)
{
    if (oldValue == editor.getText() || getShowsMultipleValues())
        return moveKeyboardFocusToSibling (true);
    
    editor.setText (oldValue, dontSendNotification);
    
    moveKeyboardFocusToSibling (true);
}

void PropertyItemTextComponent::textEditorFocusLost (TextEditor &e) 
{
    tree.repaint();
    
    if (oldValue == editor.getText() || getShowsMultipleValues())
        return;
    
    checkValueAndNotify (oldValue);
    oldValue = editor.getText();
}
//==============================================================================

PropertyItemListComponent::PropertyItemListComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, StringArray values, String value) : PropertyItemComponent (propertyTree, propItem, name), oldValue (value)
{
    addAndMakeVisible (comboBox);
    
    comboBox.setEditableText (false);
    comboBox.addItemList (values, 1);
    comboBox.setText (value);
    comboBox.addListener (this);
    comboBox.setColour (ComboBox::backgroundColourId, Colours::transparentBlack);
    comboBox.setColour (ComboBox::outlineColourId, Colours::transparentBlack);
}

void PropertyItemListComponent::resized()
{
    comboBox.setBounds (getLocalBounds().withTrimmedLeft (labelWidth));
}

String PropertyItemListComponent::getTextValue() const
{
    return comboBox.getText();
}

void PropertyItemListComponent::setTextValue (String txt, bool notify)
{
    comboBox.setText (txt, notify ? NotificationType::sendNotification : NotificationType::dontSendNotification);
    oldValue = getTextValue();
}

ComboBox& PropertyItemListComponent::getComboBox()
{
    return comboBox;
}

void PropertyItemListComponent::comboBoxChanged (ComboBox* cb)
{
    if (oldValue == getTextValue())
        return;
    
    propertyValueChanged (oldValue);
    oldValue = getTextValue();
}

//==============================================================================

PropertyItemColourComponent::PropertyItemColourComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, String defaultValue, bool showsMultiple, bool alpha) : PropertyItemComponent (propertyTree, propItem, name), currentColour (Colour::fromString (defaultValue)), lastColourPicked (currentColour), useAlpha (alpha), editor (*this), button (*this, currentColour, useAlpha)
{
    
    addAndMakeVisible (button);
    addAndMakeVisible (editor);
    
    editor.setText (currentColour.toDisplayString (useAlpha), dontSendNotification);
    editor.setInputRestrictions (useAlpha ? 8 : 6, "0123456789aAbBcCdDeEfF");
    editor.setSelectAllWhenFocused (true);
    editor.addListener (this);
    
    if (showsMultiple)
        setShowsMultipleValues (true);
    else
        refreshElementColours();
}

void PropertyItemColourComponent::paint (Graphics &g)
{
    if (checkerBoardIsVisible)
    {
        const auto r = getLocalBounds().withTrimmedLeft (labelWidth + 4);
        g.fillCheckerBoard (r.withTrimmedRight (getHeight()).toFloat(), 10, 10, Colours::black, Colours::white);
    }
}
 
void PropertyItemColourComponent::resized()
{
    if (labelWidth == 0)
        return;
    
    auto r = getLocalBounds().withTrimmedLeft (labelWidth + 4);
    button.setBounds (r.removeFromRight (getHeight()));
    editor.setBounds (r);
}

String PropertyItemColourComponent::PropertyItemColourComponent::getTextValue() const
{
    return currentColour.toString();
}

void PropertyItemColourComponent::setTextValue (String txt, bool notify)
{
    setCurrentColour (textToColour (txt), notify);
}

void PropertyItemColourComponent::textEditorTextChanged (TextEditor &)
{
    setShowsMultipleValues (false);
    setCurrentColour (textToColour (editor.getText()), false);
}

void PropertyItemColourComponent::textEditorReturnKeyPressed (TextEditor &)
{
    if (! getShowsMultipleValues())
        setCurrentColour (currentColour, true);
    
    moveKeyboardFocusToSibling (true);
}

void PropertyItemColourComponent::textEditorEscapeKeyPressed (TextEditor &)
{
    if (! getShowsMultipleValues())
        setCurrentColour (lastColourPicked, false);
    
    moveKeyboardFocusToSibling (true);
}

void PropertyItemColourComponent::textEditorFocusLost (TextEditor &)
{
    if (! getShowsMultipleValues())
        setCurrentColour (currentColour, true);
}

void PropertyItemColourComponent::setCurrentColour (Colour c, bool sendNotification)
{
    currentColour = c;
    
    refreshElementColours();
    
    if (sendNotification)
    {
        checkValueAndNotify (lastColourPicked.toString());
        lastColourPicked = c;
    }
}

void PropertyItemColourComponent::refreshElementColours()
{
    setShowsMultipleValues (false);
    
    if (currentColour.toString() != "0")
        editor.setText (currentColour.toDisplayString (useAlpha), dontSendNotification);
    
    editor.applyColourToAllText (currentColour.contrasting());
    editor.setColour (TextEditor::backgroundColourId, currentColour);
}

//==============================================================================
PropertyItemColourComponent::ColourButton::ColourButton (PropertyItemColourComponent& o, Colour defaultColour, bool usingAlpha)
: TextButton (""), owner (o), useAlpha (usingAlpha), isOpen (false)
{
    setSize (10, 24);
}

void PropertyItemColourComponent::ColourButton::paint (Graphics &g)
{
    TextButton::paint (g);
    
    getLookAndFeel().drawTreeviewPlusMinusBox (g,
                                               getLocalBounds().withSizeKeepingCentre (12, 12).toFloat(),
                                               findColour (TextButton::buttonColourId),
                                               isOpen,
                                               isMouseOver());
}

void PropertyItemColourComponent::ColourButton::clicked()
{
    owner.setItemSelected (true);
    setOpen (true);
}

void PropertyItemColourComponent::ColourButton::changeListenerCallback (ChangeBroadcaster* source)
{
    if (ColourSelector* cs = dynamic_cast<ColourSelector*> (source))
        owner.setCurrentColour (cs->getCurrentColour(), false);
}

void PropertyItemColourComponent::ColourButton::setOpen (bool shouldBeOpen)
{
    if (shouldBeOpen == isOpen)
        return;
    
    if (! shouldBeOpen)
    {
        isOpen = false;
        repaint();
        return;
    }
    
    auto flags = ColourSelector::showSliders | ColourSelector::showColourspace
    | (useAlpha ? ColourSelector::showAlphaChannel : 0);
    
    ColourSelector* colourSelector = new CustomColourSelector (flags, owner);
    colourSelector->setName ("background");
    colourSelector->setCurrentColour (findColour (TextButton::buttonColourId));
    colourSelector->addChangeListener (this);
    colourSelector->setColour (ColourSelector::backgroundColourId, Colours::transparentBlack);
    colourSelector->setCurrentColour (owner.getCurrentColour());
    colourSelector->setSize (250, 220);
    
    auto &cob = CallOutBox::launchAsynchronously (colourSelector, getScreenBounds(), nullptr);
    cob.setArrowSize (2.0f);
    cob.setDismissalMouseClicksAreAlwaysConsumed (true);
    
    isOpen = shouldBeOpen;
    repaint();
}

//==============================================================================

PropertyItemColourComponent::CustomColourSelector::CustomColourSelector (int flags, PropertyItemColourComponent& o) : ColourSelector (flags), owner (o)
{}

PropertyItemColourComponent::CustomColourSelector::~CustomColourSelector()
{
    owner.button.setOpen (false);
    owner.setCurrentColour(owner.getCurrentColour(), true);
}

//==============================================================================

PropertyItem::PropertyItem (const ValueTree& v, PropertyTree& pt) :   tree (v), propertyTree (pt)
{
    if (tree.getProperty("selected"))
        setSelected(true, false);
    
    tree.addListener (this);
}

PropertyItem::~PropertyItem()
{
    //tree.setProperty ("OPEN", isOpen(), nullptr);
    tree.removeListener (this);
}

String PropertyItem::getTextValue() const
{
    if (propertyItemComponent != nullptr)
        propertyItemComponent->getTextValue();
    
    return String();
}

void PropertyItem::setTextValue (String newValue, bool sendNotification)
{
    if (propertyItemComponent != nullptr)
        propertyItemComponent->setTextValue (newValue, sendNotification);
}

String PropertyItem::getUniqueName() const
{
    ValueTree parent = tree.getParent();
    
    StringArray hierarchy;
    
    while (parent.isValid())
    {
        hierarchy.add (parent["name"]);
        parent = parent.getParent();
    }
    
    String chain;
    
    for (int i = hierarchy.size(); --i >= 0;)
        chain += (hierarchy[i] + ".");
    
    chain += tree["name"].toString();
    return chain;
}

bool PropertyItem::mightContainSubItems()
{
    return tree.getNumChildren() > 0;
}

void PropertyItem::paintItem (Graphics& g, int width, int height)
{
    auto c = propertyTree.getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting().withAlpha (0.1f);
    
    if (isSelected())
        g.fillAll (c);
    
    auto col = propertyTree.getTextColour();
    Colour sel = itemEnabled ? col : col.withAlpha (0.5f);
    g.setColour (sel);
    
    Font f (14.0f);
    g.setFont (f);
    
    String txt (tree["name"].toString());
    
    if (propertyItemComponent != nullptr)
        txt += " :";
    
    const int wantedTextWidth = f.getStringWidth (txt) + 2;
    
    int textWidth = propertyItemComponent ? jmin (wantedTextWidth, int (width * 0.6f)) : width - 4;
    
    g.drawText (txt,
                4, 2, textWidth, height,
                Justification::topLeft, true);
    
    if (propertyItemComponent != nullptr)
        propertyItemComponent->setLabelWidth (textWidth);
}

void PropertyItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen && getNumSubItems() == 0)
        refreshSubItems();
    else
        clearSubItems();
    
    tree.setProperty ("OPEN", isNowOpen, nullptr);
}

Component* PropertyItem::createItemComponent()
{
    propertyItemComponent = propertyTree.createComponentForItem (this);
    
    if (propertyItemComponent != nullptr)
    {
        propertyItemComponent->addListener (this);
        propertyItemComponent->setTextColour (propertyTree.getTextColour());
    }
    
    return propertyItemComponent;
}

void PropertyItem::getSelectedTreeViewItems (TreeView& treeView, OwnedArray<ValueTree>& items)
{
    const int numSelected = treeView.getNumSelectedItems();
    
    for (int i = 0; i < numSelected; ++i)
        if (const PropertyItem* item = dynamic_cast<PropertyItem*> (treeView.getSelectedItem(i)))
            items.add (new ValueTree (item->tree));
}

void PropertyItem::itemClicked (const MouseEvent &e)
{    
    if (mightContainSubItems())
        setOpen (! isOpen());
    
    repaintItem();
}

void PropertyItem::itemDoubleClicked (const MouseEvent &e)
{
}

int PropertyItem::getItemHeight() const
{
    return itemHeight > 0 ? itemHeight : propertyTree.getDefaultItemHeight();
}

void PropertyItem::saveOpenness()
{
    tree.setProperty("OPEN", isOpen(), nullptr);
}

void PropertyItem::restoreOpenness()
{
    setOpen (tree["OPEN"]);
}

void PropertyItem::valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property)
{
    if (treeWhosePropertyHasChanged.getType() == Identifier("HEADER"))
        propertyTree.repaint();
    else if (treeWhosePropertyHasChanged == tree)
        if (property == Identifier("value"))
            triggerAsyncUpdate();
}

void PropertyItem::handleAsyncUpdate()
{
    if (propertyItemComponent)
        propertyItemComponent->setTextValue (tree["value"].toString(), false);
}

void PropertyItem::propertyItemValueChanged (PropertyItemComponent* comp, String newValue, String oldValue)
{
    auto um = propertyTree.getUndoManager();
    
    if (um != nullptr)
        um->beginNewTransaction();
    
    tree.setProperty ("value", newValue, um);
}

void PropertyItem::refreshSubItems()
{
    clearSubItems();
    
    for (int i = 0; i < tree.getNumChildren(); ++i)
        addSubItem (new PropertyItem (tree.getChild (i), propertyTree));
}

PropertyItem* PropertyItem::findItemWithPropertyName (String name)
{
    if (tree["name"] == name)
        return this;
    
    for (int i = 0; i < getNumSubItems(); ++i)
        if (auto propItem = dynamic_cast<PropertyItem*> (getSubItem (i)))
            if (auto r = propItem->findItemWithPropertyName (name))
                return r;
    
    return nullptr;
}

void PropertyItem::setEnabled (bool shouldBeEnabled)
{
    itemEnabled = shouldBeEnabled;
    
    if (propertyItemComponent)
        propertyItemComponent->setEnabled (shouldBeEnabled);
    
    propertyTree.repaint();
}

//==============================================================================

PropertyTree::PropertyTree() : textColour (Colours::white)
{
    addAndMakeVisible (treeView);
    treeView.setIndentSize (12);
}

PropertyTree::~PropertyTree()
{
    treeView.setRootItem (nullptr);
}

void PropertyTree::resized()
{
    treeView.setBounds (getLocalBounds().withTrimmedTop (3));
}

void PropertyTree::setRootItemVisible (bool shouldBeVisible)
{
    treeView.setRootItemVisible (shouldBeVisible);
}

void PropertyTree::setIndentSize (int newSize)
{
    treeView.setIndentSize (newSize);
}

PropertyItemComponent* PropertyTree::createComponentForItem (PropertyItem* item)
{
    if (item == nullptr)
        return nullptr;
    
    const String name = item->tree["name"];
    const String type = item->tree["componentType"];
    const String value = item->tree["value"];
    const String attributes = item->tree["attributes"];
    
    if (type == "text")
        return new PropertyItemTextComponent (*this, *item, name, value, attributes);
    
    else if (type == "colour")
        return new PropertyItemColourComponent (*this, *item, name, value, attributes.contains ("multiple"));
    
    return nullptr;
}

void PropertyTree::loadValueTree (ValueTree valueTree, int unfoldLevel)
{
    if (rootItem && rootItem->getNumSubItems() != 0)
        saveOpenness (rootItem);
    
    treeView.setRootItem (rootItem = new PropertyItem (valueTree, *this));
    
    restoreOpenness (rootItem);
    
    if (unfoldLevel > 0)
        setOpenRecursively (true, rootItem, 0, unfoldLevel);
}

ValueTree PropertyTree::createTree (String type, String name, String componentType, String value, String attributes)
{
    ValueTree vt (type);
    vt.setProperty ("type", type, nullptr);
    vt.setProperty ("name", name, nullptr);
    
    vt.setProperty ("componentType", componentType, nullptr);
    vt.setProperty ("value", value, nullptr);
    vt.setProperty ("attributes", attributes, nullptr);
    
    return vt;
}

PropertyItem* PropertyTree::getRootItem()
{
    return rootItem.get();
}

void PropertyTree::setTextColour (Colour newColour)
{
    textColour = newColour;
    
    if (rootItem != nullptr)
        rootItem->setTextColour (newColour);
}

Colour PropertyTree::getTextColour() const
{
    return textColour;
}

void PropertyTree::saveOpenness (TreeViewItem* item)
{
    if (auto propItem = dynamic_cast<PropertyItem*>(item))
        propItem->saveOpenness();
    
    if (item != nullptr)
        for (int i = 0; i < item->getNumSubItems(); ++i)
            saveOpenness (item->getSubItem (i));
}

void PropertyTree::restoreOpenness (TreeViewItem* item)
{
    if (auto propItem = dynamic_cast<PropertyItem*>(item))
        propItem->restoreOpenness();
    
    if (item != nullptr)
        for (int i = 0; i < item->getNumSubItems(); ++i)
            restoreOpenness (item->getSubItem (i));
    
}

void PropertyTree::setOpenRecursively (bool shouldBeOpen, TreeViewItem* item, int currentLevel, int unfoldLevel)
{
    if (currentLevel == unfoldLevel)
        return;
    
    if (item == nullptr)
        return;
    
    item->setOpen (shouldBeOpen);
    
    for (int i = 0; i < item->getNumSubItems(); ++i)
        setOpenRecursively (shouldBeOpen, item->getSubItem(i), currentLevel + 1, unfoldLevel);
}
