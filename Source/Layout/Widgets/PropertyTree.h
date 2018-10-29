/*
  ==============================================================================

    PropertyTree.h
    Created: 19 Aug 2018 4:20:49am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

//==============================================================================
struct PropertyValueConstrainer
{
    virtual ~PropertyValueConstrainer() {}
    virtual String constrain (String value) =  0;
};

//==============================================================================
class PropertyTree;
class PropertyItem;

class PropertyItemComponent : public Component
{
public:
    PropertyItemComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, PropertyValueConstrainer* c = nullptr);
    ~PropertyItemComponent();
    
    String getPropertyName() const { return propertyName; }
    
    //==============================================================================
    class Listener
    {
    public:
        virtual ~Listener() { masterReference.clear(); }
        
        virtual void propertyItemValueChanged (PropertyItemComponent* comp, String newValue, String oldValue) = 0;
        
    private:
        WeakReference<Listener>::Master masterReference;
        friend class WeakReference<Listener>;
    };
    //==============================================================================
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    void propertyValueChanged (String oldValue);
    
    void setConstrainer (PropertyValueConstrainer* c);
    
    virtual String getTextValue() const = 0;
    virtual void setTextValue (String txt, bool sendNotification) = 0;
    
    virtual void setTextColour (Colour newColour) = 0;
    
    virtual void setShowsMultipleValues (bool showsMultiple)
    {
        showsMultipleValues = showsMultiple;
    }
    
    bool getShowsMultipleValues() const { return showsMultipleValues; }
    
    void setLabelWidth (int newWidth) { labelWidth = newWidth; resized(); }
    
    void mouseUp (const MouseEvent &e) override;
    void setItemSelected (bool shouldBeSelected);
    
protected:
    friend class PropertyItem;
    
    void checkValueAndNotify (String oldValue);
    
    PropertyTree& tree;
    PropertyItem& item;
    ScopedPointer<PropertyValueConstrainer> constrainer;
    
    int labelWidth = 0;
    
private:
    WeakReference<PropertyItemComponent>::Master masterReference;
    friend class WeakReference<PropertyItemComponent>;
    
    Array<WeakReference<Listener>> listeners;
    const String propertyName;
    
    bool showsMultipleValues = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItemComponent);
};
//==============================================================================
class PropertyItem;

class PropertyItemBooleanComponent : public PropertyItemComponent, public Button::Listener
{
public:
    PropertyItemBooleanComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, bool value);
    ~PropertyItemBooleanComponent() { masterReference.clear(); }
    
    void resized() override;
    
    void buttonClicked (Button* b) override;
    
    void enablementChanged() override
    {
        button.setEnabled (isEnabled());
    }
    
    String getTextValue() const override;
    void setTextValue (String txt, bool sendNotification) override;
    
    void setTextColour (Colour newColour) override {}
    
private:
    WeakReference<PropertyItemBooleanComponent>::Master masterReference;
    friend class WeakReference<PropertyItemBooleanComponent>;
    
    ToggleButton button;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItemBooleanComponent);
};

//==============================================================================
class PropertyItem;

class PropertyItemTextComponent : public PropertyItemComponent, public TextEditor::Listener
{
public:
    PropertyItemTextComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, String value, String attributes, PropertyValueConstrainer* c = nullptr);
    ~PropertyItemTextComponent() { masterReference.clear(); }

    void resized() override;

    TextEditor& getEditor();
    
    void textEditorTextChanged (TextEditor &e) override
    {
        setShowsMultipleValues (false);
    }
    
    void textEditorReturnKeyPressed (TextEditor &e) override;
    void textEditorEscapeKeyPressed (TextEditor &e) override;
    void textEditorFocusLost (TextEditor &e) override;
        
    void enablementChanged() override
    {
        auto f = editor.getFont().withStyle (isEnabled() ? Font::plain : Font::italic);
        editor.applyFontToAllText (f);
        
        auto c = isEnabled() ? Colours::white : Colours::grey;
        editor.applyColourToAllText (c);
    }
    
    String getTextValue() const override;
    void setTextValue (String txt, bool sendNotification) override;
    
    void setTextColour (Colour newColour) override
    {
        editor.applyColourToAllText (newColour);
    }
    
    void setShowsMultipleValues (bool showsMultiple) override
    {
        if (showsMultiple == getShowsMultipleValues())
            return;
        
        PropertyItemComponent::setShowsMultipleValues (showsMultiple);
        
        const Colour defaultColour (getLookAndFeel().findColour (TextEditor::textColourId));
        editor.applyColourToAllText (showsMultiple ? Colours::grey : defaultColour);
        
        auto f = editor.getFont();
        f.setStyleFlags (showsMultiple ? Font::italic : Font::plain);
        editor.applyFontToAllText (f);
        
        if (showsMultiple)
            editor.setText ("multiple", dontSendNotification);
        
        editor.setSelectAllWhenFocused (showsMultiple);
    }
    
    //==============================================================================
    class CustomTextEditor : public TextEditor
    {
    public:
        CustomTextEditor (PropertyItemComponent& o) : owner (o) {}
        
        void mouseUp (const MouseEvent &e) override
        {
            if (e.mouseWasClicked())
                owner.setItemSelected (true);
            
            TextEditor::mouseUp (e);
        }
        
        void focusLost (FocusChangeType cause) override
        {
            TextEditor::focusLost (cause);
            
            setHighlightedRegion (Range<int>(0, 0));
        }
        
    private:
        PropertyItemComponent& owner;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomTextEditor)
    };
    //==============================================================================
    
private:
    WeakReference<PropertyItemTextComponent>::Master masterReference;
    friend class WeakReference<PropertyItemTextComponent>;
    
    CustomTextEditor editor;
    String oldValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItemTextComponent);
};

//==============================================================================
class PropertyItemListComponent : public PropertyItemComponent, ComboBox::Listener
{
public:
    PropertyItemListComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, StringArray values, String defaultValue);
    ~PropertyItemListComponent() { masterReference.clear(); }
    
    void resized() override;
    
    ComboBox& getComboBox();
    void comboBoxChanged (ComboBox* cb) override;
    
    void enablementChanged() override
    {
        comboBox.setEnabled (isEnabled());
    }
    
    String getTextValue() const override;
    void setTextValue (String txt, bool notify) override;
    
    void setTextColour (Colour newColour) override
    {
        comboBox.setColour (ComboBox::textColourId, newColour);
    }
    
private:
    WeakReference<PropertyItemListComponent>::Master masterReference;
    friend class WeakReference<PropertyItemListComponent>;
    
    ComboBox comboBox;
    String oldValue;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItemListComponent);
};

//==============================================================================
class PropertyItemColourComponent : public PropertyItemComponent, public TextEditor::Listener
{
public:
    PropertyItemColourComponent (PropertyTree& propertyTree, PropertyItem& propItem, String name, String defaultValue, bool showsMultiple, bool useAlphaChannel = false);
    ~PropertyItemColourComponent() { masterReference.clear(); }
    
    void paint (Graphics &g) override;
    void resized() override;
    
    String getTextValue() const override;
    void setTextValue (String txt, bool notify) override;
    
    void setTextColour (Colour c) override {}
    
    void textEditorTextChanged (TextEditor &) override;
    void textEditorReturnKeyPressed (TextEditor &) override;
    void textEditorEscapeKeyPressed (TextEditor &) override;
    void textEditorFocusLost (TextEditor &) override;
    
    void setCurrentColour (Colour c, bool sendNotification);
    Colour getCurrentColour() const { return currentColour; }

    void enablementChanged() override
    {
        editor.setEnabled (isEnabled());
        button.setEnabled (isEnabled());
    }
    
    void setShowsMultipleValues (bool showsMultiple) override
    {
        if (showsMultiple == getShowsMultipleValues())
            return;
        
        PropertyItemComponent::setShowsMultipleValues (showsMultiple);
        
        checkerBoardIsVisible = ! showsMultiple;
        
        const Colour defaultColour (getLookAndFeel().findColour (TextEditor::textColourId));
        editor.applyColourToAllText (showsMultiple ? Colours::grey : defaultColour);
        
        auto f = editor.getFont();
        f.setStyleFlags (showsMultiple ? Font::italic : Font::plain);
        editor.applyFontToAllText (f);
        
        if (showsMultiple)
            editor.setText ("multiple", dontSendNotification);
        
        editor.setSelectAllWhenFocused (showsMultiple);
        
        repaint();
    }
    
private:
    void refreshElementColours();
    
    //==============================================================================
    class ColourButton  :   public TextButton, public ChangeListener
    {
    public:
        ColourButton (PropertyItemColourComponent& o, Colour defaultColour, bool usingAlpha);
        
        void paint (Graphics &g) override;
        void clicked() override;
        void changeListenerCallback (ChangeBroadcaster* source) override;
                
        void setOpen (bool shouldBeOpen);
        
    private:
        PropertyItemColourComponent& owner;
        bool useAlpha;
        bool isOpen;
    };
    
    //==============================================================================
    class CustomColourSelector : public ColourSelector
    {
    public:
        CustomColourSelector (int flags, PropertyItemColourComponent& o);
        ~CustomColourSelector();
        
    private:
        PropertyItemColourComponent& owner;
    };
    
    Colour textToColour (String text)
    {
        if (! useAlpha)
            text = "FF" + text;
        
        return Colour::fromString (text);
    }
    
    WeakReference<PropertyItemColourComponent>::Master masterReference;
    friend class WeakReference<PropertyItemColourComponent>;
    
    friend class CustomColourSelector;
    friend class ColourButton;
    friend class PropertyItem;
    
    Colour currentColour;
    Colour lastColourPicked;
    bool checkerBoardIsVisible = true;
    const bool useAlpha;
    
    PropertyItemTextComponent::CustomTextEditor editor;
    ColourButton button;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItemColourComponent);
};

//==============================================================================
class PropertyItem    :     public TreeViewItem,
                            public ValueTree::Listener,
                            public PropertyItemComponent::Listener,
                            public AsyncUpdater
{
public:
    PropertyItem (const ValueTree& v, PropertyTree& pt);
    ~PropertyItem();
    
    // TreeViewItem implementation...
    String getUniqueName() const override;
    bool mightContainSubItems() override;
    void paintItem (Graphics& g, int width, int height) override;
    void itemOpennessChanged (bool isNowOpen) override;
    Component* createItemComponent() override;
        
    static void getSelectedTreeViewItems (TreeView& treeView, OwnedArray<ValueTree>& items);
    void itemClicked (const MouseEvent &e) override;
    void itemDoubleClicked (const MouseEvent &e) override;
    
    void setValue (String newValue) { tree.setProperty ("value", newValue, nullptr); }
    ValueTree& getValueTree() { return tree; }
    
    void setItemHeight (int newHeight) { itemHeight = newHeight; }
    int getItemHeight() const override;
    
    void saveOpenness();
    void restoreOpenness();
    
    // ValueTree::Listener
    void valueTreePropertyChanged (ValueTree &treeWhosePropertyHasChanged, const Identifier &property) override;
    void valueTreeChildAdded (ValueTree &parentTree, ValueTree &childWhichHasBeenAdded) override {}
    void valueTreeChildRemoved (ValueTree &parentTree, ValueTree &childWhichHasBeenRemoved, int indexFromWhichChildWasRemoved) override {}
    void valueTreeChildOrderChanged (ValueTree &parentTreeWhoseChildrenHaveMoved, int oldIndex, int newIndex) override {}
    void valueTreeParentChanged (ValueTree &treeWhoseParentHasChanged) override {}
    
    // PropertyItemComponent::Listener
    void propertyItemValueChanged (PropertyItemComponent* comp, String newValue, String oldValue) override;
    
    PropertyItem* findItemWithPropertyName (String name);
    
    void setEnabled (bool shouldBeEnabled);
    bool isEnabled() const { return itemEnabled; }
    
    void handleAsyncUpdate() override;
    
    void setTextColour (Colour newColour)
    {
        if (propertyItemComponent != nullptr)
            propertyItemComponent->setTextColour (newColour);
        
        for (int i = 0; i < getNumSubItems(); ++i)
            if (auto propItem = dynamic_cast<PropertyItem*> (getSubItem (i)))
                propItem->setTextColour (newColour);
    }
    
private:
    String getTextValue() const;
    void setTextValue (String newValue, bool sendNotification);
    
    friend class PropertyTree;
    
    void refreshSubItems();
    
    ValueTree tree;
    PropertyTree& propertyTree;
    WeakReference<PropertyItemComponent> propertyItemComponent;
    
    ScopedPointer<XmlElement> openness;
    
    int itemHeight = -1;
    bool itemEnabled = true;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyItem)
};

//==============================================================================
class PropertyTree : public Component
{
public:
    PropertyTree();
    ~PropertyTree();
    
    void resized();
    
    int getDefaultItemHeight() const { return 18; }
    
    PropertyItem* getRootItem();
    void setRootItemVisible (bool shouldBeVisible);
    
    virtual PropertyItemComponent* createComponentForItem (PropertyItem* item);

    void loadValueTree (ValueTree valueTree, int unfoldLevel = 0);
    ValueTree createTree (String type, String name, String componentType = "", String value = "", String attributes = "");
    
    void setUndoManager (UndoManager* um) { undoManager = um; }
    virtual UndoManager* getUndoManager() { return undoManager; }
    
    void setIndentSize (int newSize);
    
    void setTextColour (Colour newColour);
    Colour getTextColour() const;
    
    PropertyItem* findPropertyItemWithName (String name) const
    {
        if (rootItem == nullptr)
            return nullptr;
        
        return rootItem->findItemWithPropertyName (name);
    }
    
private:
    void saveOpenness (TreeViewItem* item);
    void restoreOpenness (TreeViewItem* item);
    void setOpenRecursively (bool shouldBeOpen, TreeViewItem* item, int currentLevel, int unfoldLevel);
        
    TreeView treeView;
    ScopedPointer<PropertyItem> rootItem;
    Colour textColour;
    UndoManager* undoManager = nullptr;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PropertyTree)
};
