/*
  ==============================================================================

    WebAudioInspectableElement.h
    Created: 24 Aug 2018 2:44:49am
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/Panels/InspectorPanel.h"
#include "../WebAudioGraph/WebAudioNodeInstance.h"

class GraphEmbeddedComponent;
class WebAudioGraph;
class WebAudioGraphPanel;

class WebAudioInspectableElement :  public InspectableElement
{
public:
    WebAudioInspectableElement (WebAudioGraphPanel& panel, Descriptor descr, GraphEmbeddedComponent* comp);
    ~WebAudioInspectableElement() { masterReference.clear(); }
    
    WebAudioGraph& getWebAudioGraph();
    
    void setPropertyValue (String key, String value, bool undoable = false, ValueTree::Listener* exclude = nullptr);
    void setPositionProperties();
    void setSizeProperties();
    
    WebAudioNodeInstance* getInstance() const;
    ValueTree getPropertyValueTree();
    
    ValueTree createEditableProperty (String name, String type, String defaultValue, String attributes = String());
    void addPropertyRecursive (const Descriptor& param, ValueTree container, int& index);
    
    String getInterfaceName() const { return privateDescriptor.interf.name; }
    
    virtual bool isWebAudioElementEnabled() const { return true; }
    virtual String getElementName() const = 0;
    virtual Colour getElementColour() const = 0;
    
    virtual void setNumInputs (int numInputs) {}
    virtual void setNumOutputs (int numInputs) {}
    
    virtual void checkContainers (bool useHighlight) = 0;
    virtual void refreshOptionsTree() {}
    
    const int maxInputs = 32;
    const int maxOutputs = 32;
    
    static void setPropertyValue (ValueTree container, String key, String value, UndoManager* um = nullptr, ValueTree::Listener* exclude = nullptr);
    static ValueTree findPropertyWithName (ValueTree container, String name);
    
    virtual String getValidName (String wantedName) { return wantedName.isEmpty() ? getElementName() : wantedName; }
    
    Array<Descriptor::Method> getAllMethods() const;
    
protected:
    virtual String getNameInInspector() const { return getElementName(); }
    
    virtual void prepareInspectablePropertiesTree (String typeName);
    virtual ValueTree createNameProperty();
    String getTypeDisplayString (String type) const;

    WebAudioGraphPanel& parentPanel;
    const Descriptor privateDescriptor;
    
    // Set in prepareInspectablePropertiesTree. Shouldn't be changed otherwise
    String nameLabel;
    
private:
    friend class WebAudioInstanceManager;
    void setInstance (WebAudioNodeInstance* newInstance);
    
    WeakReference<WebAudioInspectableElement>::Master masterReference;
    friend class WeakReference<WebAudioInspectableElement>;
    
    WeakReference<WebAudioNodeInstance> instance;
    GraphEmbeddedComponent *const embeddedComponent;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioInspectableElement)
};

//==============================================================================
#include "../Source/Layout/Graph/GraphEmbeddedComponent.h"
#include "../Source/Layout/Panels/NavigationPanel.h"

class WebAudioEmbedded  :   public WebAudioInspectableElement,
                            public GraphEmbeddedComponent,
                            public NavigationPanel::Navigable,
                            public Label::Listener
{
public:
    WebAudioEmbedded (WebAudioGraphPanel& panel, Descriptor descr);
    ~WebAudioEmbedded() {}
    
    WebAudioInstanceManager& getInstanceManager();
    
    BorderSize<int> getBorderThickness() const;
    void setResizable (bool shouldBeResizable);
    bool isResizable() const;
    
    void resized() override;
    void mouseDown (const MouseEvent &e) override;
    void mouseUp (const MouseEvent &e) override;
    void wasDragged (const MouseEvent &e, bool dragIsOver, bool isUniqueSelection) override;
    
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
    StringPairArray getNavigationInfo() const override;
    void reveal (ModifierKeys mods) override;
    
    void inspectablePropertyChanged (ValueTree &tree, const Identifier &property) override;
    String getElementName() const override { return getPublicName(); }
    
    void showNameEditor() override;
    void editorShown (Label* l, TextEditor& editor) override;
    void editorHidden (Label*, TextEditor&) override;
    void labelTextChanged (Label* labelThatHasChanged) override;
    
    void checkContainers (bool useHighlight) override;
    void checkAndSetName (String newName) { return setPublicName (getValidName (newName)); }
    virtual String getDefaultName() const;
    void setPublicName (String newName) override;
    Colour getElementColour() const override { return getBackgroundColour(); }
    void setBackgroundColour (Colour newColour) override;
    String getValidName (String wantedName) override;
    virtual bool nameIsAlreadyTaken (String name);
    
    void deleteSelectedItem() override;
    
    virtual void initializeWithWantedName (String name, bool createNewInstance);
    
    void addExtraPopupMenuCommands (PopupMenu& m, Point<int> pos) override;
    void handleExtraPopupMenuCommands (int result, Point<int> pos) override;
    
    bool isUniqueSelection() const
    {
        if (auto selector = getSelector())
            return selector->getNumSelected() == 1 && selector->getSelected(0) == this;
        
        return false;
    }
    
protected:
    class CustomLabel : public Label
    {
    public:
        CustomLabel (WebAudioEmbedded& o) : owner (o) {}
        
        void mouseUp (const MouseEvent &e) override
        {
            // We don't want to alter the selection after a drag
            if (! e.mouseWasDraggedSinceMouseDown())
                owner.setSelectedBasedOnModifiers (e.mods);
            
            Label::mouseUp (e);
        }
        
    private:
        WebAudioEmbedded& owner;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CustomLabel)
    };
    
    void prepareLabel();
    virtual Label* createLabel() { return new CustomLabel (*this); }
    
    //==============================================================================
    class CustomResizableBorder : public ResizableBorderComponent
    {
    public:
        CustomResizableBorder (WebAudioEmbedded& o, ComponentBoundsConstrainer *constrainer);
        
        void paint (Graphics& g) override
        {
            if (! isHidden)
                ResizableBorderComponent::paint (g);
        }
        
        void mouseDown (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        
        void setHidden (bool shouldBeHidden)
        {
            isHidden = shouldBeHidden;
            repaint();
        }
        
    private:
        WebAudioEmbedded& owner;
		juce::Rectangle<int> previousBounds;
        bool isHidden = false;
    };
    
    virtual void wasResizedWithBorder() { manuallyResized = true; }
    
    bool wasManuallyResized() { return manuallyResized; }
    
    void setResizableBorderHidden (bool shouldBeHidden)
    {
        if (resizableBorder != nullptr)
            resizableBorder->setHidden (shouldBeHidden);
    }
    
    //==============================================================================
    
    ScopedPointer<CustomResizableBorder> resizableBorder;
    ComponentBoundsConstrainer constrainer;
    bool manuallyResized = false;
    ScopedPointer<Label> label;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioEmbedded)
};

//==============================================================================
class WebAudioFoldable  :   public WebAudioEmbedded
{
public:
    WebAudioFoldable (WebAudioGraphPanel& panel, Descriptor descr);
    ~WebAudioFoldable() {}
    
    void setOpenButtonVisible (bool shouldBeVisible);
    void setOpen (bool shouldBeOpen);
    bool isOpen() const;
    
    void mouseEnter (const MouseEvent &e) override;
    void mouseMove (const MouseEvent &e) override;
    void mouseExit (const MouseEvent &e) override;
    void mouseUp (const MouseEvent &e) override;

    void paintUI (Graphics &g, juce::Rectangle<int> contentBounds) override;
    void resizeUI (juce::Rectangle<int> contentBounds) override;
    void refreshUI() override;
    
    void refreshOptionsTree() override;
    
    int getDefaultWidth() const override { return GraphEmbeddedComponent::getDefaultWidth() + 15; }
    
    void prepareOptionsTree();
    virtual void setOptionValue (String optionName, String newValue);
    
    void setBackgroundColour (Colour newColour) override;

    void setPublicName (String newName) override;
    void initializeWithWantedName (String name, bool createNewInstance) override;
    
protected:
    virtual void setConstrainerLimits();
    
    int getWidthWhenClosed() const;
    int getHeightWhenOpen() const;
    
    //==============================================================================
    class OptionsPropertyTree : public PropertyTree, PropertyItemComponent::Listener
    {
    public:
        OptionsPropertyTree (WebAudioFoldable& ownerComp) : owner (ownerComp) {}
        ~OptionsPropertyTree();
        
        PropertyItemComponent* createComponentForItem (PropertyItem* item) override;
        void propertyItemValueChanged (PropertyItemComponent* comp, String newValue, String oldValue) override;
        
    private:
        WebAudioFoldable& owner;
        Array<WeakReference<PropertyItemComponent>> comps;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OptionsPropertyTree)
    };
    //==============================================================================

    OptionsPropertyTree optionsTree;
    
    int maximumWidth = 380;
    
private:
    int lastWidthWhenOpen = 0;
    bool isMouseOverButton = false;
    bool isCurrentlyOpen = false;
    bool openButtonVisible = true;
    int numOptions = 0;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioFoldable)
};

//==============================================================================

class WebAudioContainer :   public WebAudioEmbedded
{
public:
    WebAudioContainer (WebAudioGraphPanel &parent, Descriptor descr);
    ~WebAudioContainer() { masterReference.clear(); }
    
    void addComponent (GraphEmbeddedComponent* compToAdd);
    void removeComponent (GraphEmbeddedComponent* compToRemove);
    const ReferenceCountedArray<GraphEmbeddedComponent>& getContent() const;
    void checkContent();
        
    void mouseDown (const MouseEvent& e) override;
    void mouseUp (const MouseEvent &e) override;
    void mouseDrag (const MouseEvent &e) override;
    void mouseMove (const MouseEvent &e) override;
    void wasDragged (const MouseEvent &e, bool dragIsOver, bool isUniqueSelection) override;
    
    void resizeUI (juce::Rectangle<int> contentBounds) override;
    Array<WeakReference<NavigationPanel::Navigable>> getSubNavigables() override;
        
    int getDefaultWidth() const override { return 300; }
    int getDefaultHeight() const override { return 200; }
    
	juce::Rectangle<int> getLassoSelectionBounds() const override { return getBounds().removeFromTop (GraphEmbeddedComponent::getDefaultHeight()); }
    
    void inspectablePropertyChanged (ValueTree &tree, const Identifier &property) override;
    
private:
    WeakReference<WebAudioContainer>::Master masterReference;
    friend class WeakReference<WebAudioContainer>;
    
    bool isBeingDragged = false;
    bool shouldPrepareDrag = false;
    ReferenceCountedArray<GraphEmbeddedComponent> components;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebAudioContainer)
};
