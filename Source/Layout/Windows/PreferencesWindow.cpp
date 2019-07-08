/*
  ==============================================================================

    PreferencesWindow.cpp
    Created: 13 Dec 2017 7:23:17pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/Layout/Windows/PreferencesWindow.h"

class ListPropertyComponent : public ChoicePropertyComponent
{
public:
    ListPropertyComponent (String name, StringArray list) : ChoicePropertyComponent (name)
    {
        setPreferredHeight (22);
        
        choices.addArray (list);
    }
    
    void setIndex (int newIndex)
    {
        if (currentIndex != newIndex)
        {
            currentIndex = newIndex;
            
            for (auto l : listeners)
            l->listChoiceChanged (this);
        }
    }
    
    int getIndex() const
    {
        return currentIndex;
    }
    
    class Listener
    {
        public:
        Listener() {}
        virtual ~Listener() {}
        
        virtual void listChoiceChanged (ListPropertyComponent*) = 0;
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    private:
    int currentIndex = 0;
    
    Array<Listener*> listeners;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ListPropertyComponent)
};

class TextButtonPropertyComponent : public PropertyComponent, public Button::Listener
{
    public:
    TextButtonPropertyComponent (String name, String textOn, String textOff = String(), bool isToggle = false, bool defaultState = true) : PropertyComponent (name), textWhenOn (textOn), textWhenOff (textOff)
    {
        setPreferredHeight (22);
        
        button.setToggleState (defaultState, dontSendNotification);
        
        if (isToggle)
        {
            button.setClickingTogglesState(true);
            updateButtonText();
        }
        else
        button.setButtonText (textOn);
        
        addAndMakeVisible (button);
        
        button.addListener (this);
    }
    
    void resized() override
    {
        if (Component* const c = getChildComponent(0))
        c->setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this).withWidth (120));
    }
    
    void refresh() override {}
    
    bool getState() const { return button.getToggleState(); }
    void setState (bool newState) { button.setToggleState(newState, dontSendNotification); updateButtonText(); }
    
    void buttonClicked (Button*) override
    {
        updateButtonText();
        
        for (auto l : listeners)
        l->buttonPropertyClicked (this);
    }
    
    class Listener
    {
        public:
        Listener() {}
        virtual ~Listener() {}
        
        virtual void buttonPropertyClicked (TextButtonPropertyComponent*) = 0;
    };
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    private:
    void updateButtonText()
    {
        const String newText = button.getToggleState() ? textWhenOn : textWhenOff;
        button.setButtonText(newText);
    }
    
    Array<Listener*> listeners;
    
    TextButton button;
    
    String textWhenOn;
    String textWhenOff;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TextButtonPropertyComponent)
};

//==============================================================================
class IncDecPropertyComponent : public SliderPropertyComponent
{
    public:
#undef min
#undef max
    IncDecPropertyComponent (String name, String suffix = String(), double defaultValue = 0, double min = 0, double max = std::numeric_limits<double>::max(), double increment = 1, Value* v = new Value()) : SliderPropertyComponent (*v, name, min, max, increment), value (v)
    {
        setPreferredHeight (22);
        
        slider.setSliderStyle (Slider::SliderStyle::IncDecButtons);
        slider.setIncDecButtonsMode(Slider::incDecButtonsNotDraggable);
        
        setValue (defaultValue);
        slider.setValue (defaultValue);
        
        slider.setTextValueSuffix (suffix);
    }
    ~IncDecPropertyComponent() {}
    
    void resized() override
    {
        if (Component* const c = getChildComponent(0))
        c->setBounds (getLookAndFeel().getPropertyComponentContentPosition (*this).withWidth (120));
    }
    
    //void sliderValueChanged(Slider* s) override { value->setValue (slider.getValue()); }
    
    void setValue (double newValue) override { value->setValue (newValue); }
    double getValue () const override { return value->getValue(); }
    
    void addListener (Slider::Listener* l) { slider.addListener (l); }
    void removeListener (Slider::Listener* l) { slider.removeListener (l); }
    
    protected:
    ScopedPointer<Value> value;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (IncDecPropertyComponent)
};

//==============================================================================
class MsSliderPropertyComponent : public IncDecPropertyComponent
{
    public:
    MsSliderPropertyComponent (String name, double defaultValue = 0, double min = 0, double max = std::numeric_limits<double>::max())
    : IncDecPropertyComponent (name, " ms", defaultValue, min, max, 1)
    {
    }
    ~MsSliderPropertyComponent() {}
    
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MsSliderPropertyComponent)
};

//==============================================================================
class FontSizePropertyComponent : public IncDecPropertyComponent
{
    public:
    FontSizePropertyComponent (String name = "Font size", double defaultValue = 14.0) : IncDecPropertyComponent (name, "", defaultValue, 8, 52, 0.5)
    {
        slider.setSliderStyle (Slider::SliderStyle::IncDecButtons);
        slider.setIncDecButtonsMode(Slider::incDecButtonsNotDraggable);
        
        setValue (defaultValue);
        slider.setValue (defaultValue);
    }
    ~FontSizePropertyComponent() {}
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FontSizePropertyComponent)
};

//==============================================================================
class PortPropertyComponent : public IncDecPropertyComponent
{
    public:
    PortPropertyComponent (String name = "Port number", int defaultValue = 9000)
    : IncDecPropertyComponent (name, "", defaultValue, 0, 9999) {}
    ~PortPropertyComponent() {}
    
    private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PortPropertyComponent);
};
//==============================================================================

class TextColourPropertyComponent : public BooleanPropertyComponent
{
    public:
    TextColourPropertyComponent(String name = "Text colour") : BooleanPropertyComponent (name, "Bright", "Dark")
    {
        setPreferredHeight (22);
    }
    ~TextColourPropertyComponent() {}
    
    void setState (bool b) override {}
    bool getState() const override { return true; }
    
    private:
};
//==============================================================================

#include "../Source/Layout/EmbeddedFonts.h"
class FontChoicePropertyComponent : public ChoicePropertyComponent
{
public:
    FontChoicePropertyComponent(String name, Font defaultFont) : ChoicePropertyComponent (name)
    {
        setPreferredHeight (22);

		Font::findFonts (fonts);

        int defaultIndex = 0;
        
        for (int i = 0; i < fonts.size(); ++i)
        {
			const auto typefaceName = fonts[i].getTypefaceName();

            choices.add (typefaceName);
            
            if (typefaceName == defaultFont.getTypeface()->getName())
				defaultIndex = i;
        }
        
        setIndex (defaultIndex);
    }
    
    ~FontChoicePropertyComponent() {}
    
    void setIndex (int newIndex) override
    {
        if (index != newIndex)
        {
            index = newIndex;
            changed();
        }
    }
    int getIndex() const override { return index; }
    
    Font getChosenFont() { return fonts[index]; }
    void setChosenFont (Font font)
    {
		const auto typefaceName = font.getTypefaceName();

        for (int i = 0; i < choices.size(); ++i)
        {
            if (choices[i] == typefaceName)
            {
                setIndex (i);
                refresh();
                break;
            }
        }
    }
    
    //==============================================================================
    class Listener
    {
        public:
        Listener() {}
        virtual ~Listener() {}
        
        virtual void fontPicked (FontChoicePropertyComponent* fc) = 0;
    };
    //==============================================================================
    
    void addListener (Listener* l) { listeners.addIfNotAlreadyThere (l); }
    void removeListener (Listener* l) { listeners.removeFirstMatchingValue (l); }
    
    void changed() { for (auto l : listeners) l->fontPicked (this); }

private:
    Array<Listener*> listeners;
    
    int index = 0;
    Array<Font> fonts;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FontChoicePropertyComponent)
};
//==============================================================================


//==============================================================================
#include "../Source/Layout/Widgets/ColourPicker.h"
#include "../Source/Application/AppSettings.h"
#include "../Source/Project/Project.h"


class AppPreferencesPanel::PreferencesPage
:   public Component,
    public ColourPickerButton::Listener,
    public FontChoicePropertyComponent::Listener,
    public Slider::Listener,
    public TextPropertyComponent::Listener,
    public TextButtonPropertyComponent::Listener,
    public ListPropertyComponent::Listener
{
public:    
    PreferencesPage (PageName name) : pageName (name)
    {
        addAndMakeVisible (panel);
        
        refreshPage();
    }
    
    ~PreferencesPage() {}
    
    void resized() override { panel.setBounds (getLocalBounds()); }
    
    PropertyComponent* getPropertyComponentWithName (String name) const
    {
        for (auto pc : properties)
            if (pc->getName() == name)
                return pc;
        
        return nullptr;
    }
    
    void refreshPage()
    {
        panel.clear();
        properties.clear();
        colourSchemeProps.clear();
        
        switch (pageName)
        {
            case lookPage:      prepareLookPage();      break;
                
            default:
                break;
        }
    }
    
    void apply()
    {
        switch (pageName)
        {
            case lookPage:
                AppSettings::setCurrentEditorColourScheme(getUserColourScheme());
                
                if (auto cpc = dynamic_cast<ColourPropertyComponent*>(getPropertyComponentWithName ("Main colour")))
                    AppSettings::setCurrentMainColour (cpc->getChosenColour());
                
                if (auto font = dynamic_cast<FontChoicePropertyComponent*>(getPropertyComponentWithName ("Font")))
                    AppSettings::setCurrentEditorFont(font->getChosenFont());
                
                if (auto fontSize = dynamic_cast<FontSizePropertyComponent*>(getPropertyComponentWithName ("Font size")))
                    AppSettings::setCurrentEditorFontSize (fontSize->getValue());
                
                LookAndFeelUpdater::lookAndFeelChanged();
                break;
                
            default:
                break;
        }
    }
    
    CodeEditorComponent::ColourScheme getUserColourScheme() const
    {
        auto cs = AppSettings::getCurrentEditorColourScheme();
        
        for (auto p : colourSchemeProps)
            cs.set (p->getName(), p->getChosenColour());
            
            return cs;
    }
    
    void setUserColourScheme (CodeEditorComponent::ColourScheme cs)
    {
        jassert (cs.types.size() == colourSchemeProps.size());
        
        for (int i = cs.types.size(); --i >= 0;)
            colourSchemeProps[i]->setChosenColour(cs.types[i].colour);
    }
    
    void colourPicked (ColourPickerButton*, Colour) override
    {
        apply();
    }
    
    void fontPicked (FontChoicePropertyComponent*) override
    {
        apply();
    }
    
    void sliderValueChanged (Slider*) override
    {
        apply();
    }
    
    void textPropertyComponentChanged (TextPropertyComponent *) override
    {
        apply();
    }
    
    void buttonPropertyClicked (TextButtonPropertyComponent* b) override
    {
        switch (pageName)
        {
            case lookPage:
            {
                if (b->getName() == "Reset look to default")
                {
                    AppSettings::setDefaultLook();
                    LookAndFeelUpdater::lookAndFeelChanged();
                }
                break;
            }
        }
        
        apply();
    }
    
    void listChoiceChanged (ListPropertyComponent* l) override
    {
        apply();
    }
    
    void lookAndFeelChanged() override
    {
        if (pageName == lookPage)
        {
            setUserColourScheme (AppSettings::getCurrentEditorColourScheme());
            
            if (auto cpc = dynamic_cast<ColourPropertyComponent*>(getPropertyComponentWithName ("Main colour")))
                cpc->setChosenColour(AppSettings::getCurrentMainColour());
            
            if (auto toggle = dynamic_cast<TextButtonPropertyComponent*>(getPropertyComponentWithName("Look")))
                toggle->setState(AppSettings::isCurrentLookBright());
            
            if (auto font = dynamic_cast<FontChoicePropertyComponent*>(getPropertyComponentWithName ("Font")))
                font->setChosenFont(AppSettings::getCurrentEditorFont());
            
            if (auto cpc = dynamic_cast<ColourPropertyComponent*>(getPropertyComponentWithName ("Background")))
                cpc->setChosenColour(AppSettings::getCurrentEditorBackgroundColour ());
            
            if (auto fontSize = dynamic_cast<FontSizePropertyComponent*>(getPropertyComponentWithName ("Font size")))
                fontSize->setValue(AppSettings::getCurrentEditorFontSize());
        }
    }
    
private:
    void prepareLookPage()
    {
        Array<PropertyComponent*> generalProps;
        
        auto defaultButton = new TextButtonPropertyComponent ("Reset look to default", "Reset");
        generalProps.add (defaultButton);
        defaultButton->addListener (this);
        
        auto mainCpc = new ColourPropertyComponent("Main colour", AppSettings::getCurrentMainColour());
        generalProps.add (mainCpc);
        mainCpc->addListener (this);
        
        panel.addSection ("General", generalProps);
        properties.addArray (generalProps, 0);
        
        Array<PropertyComponent*> editorProps;
        
        auto fontChoicePc = new FontChoicePropertyComponent ("Font", AppSettings::getCurrentEditorFont());
        editorProps.add (fontChoicePc);
        fontChoicePc->addListener (this);
        
        auto fontSizePc = new FontSizePropertyComponent("Font size", AppSettings::getCurrentEditorFontSize());
        editorProps.add (fontSizePc);
        fontSizePc->addListener (this);
        
        panel.addSection ("Script editor", editorProps);
        properties.addArray (editorProps, 0);
        
        auto cs = AppSettings::getCurrentEditorColourScheme();
        Array<PropertyComponent*> csProps;
        
        for (auto token : cs.types)
        {
            auto cpc = new ColourPropertyComponent (token.name, token.colour);
            colourSchemeProps.add (cpc);
            csProps.add (cpc);
            cpc->addListener (this);
        }
        
        panel.addSection ("Colour scheme", csProps);
    }
    
    PropertyPanel panel;
    
    Array<PropertyComponent*> properties;
    Array<ColourPropertyComponent*> colourSchemeProps;
    
    PageName pageName;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PreferencesPage)
};

//==============================================================================

AppPreferencesPanel::AppPreferencesPanel() : tabbedComp (TabbedButtonBar::TabsAtTop)
{
    addAndMakeVisible (tabbedComp);
    
    addPreferenceTab (PageName::lookPage);
}

void AppPreferencesPanel::resized()
{
    tabbedComp.setBounds (getLocalBounds());
}

void AppPreferencesPanel::addPreferenceTab (PageName pageName)
{
    String name;
    
    if (pageName == lookPage)
        name = "Look";
    
    tabbedComp.addTab (name,
                       LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                       createComponentForPage(pageName),
                       true);
}

Component* AppPreferencesPanel::createComponentForPage (const PageName pageName)
{
    return new PreferencesPage (pageName);
}

void AppPreferencesPanel::lookAndFeelChanged()
{
    const Colour c (getLookAndFeel().findColour (ResizableWindow::backgroundColourId).withMultipliedBrightness(0.9));
    
    for (int i = 0; i < tabbedComp.getNumTabs(); ++i)
        tabbedComp.setTabBackgroundColour(i, c);
    
    tabbedComp.setColour (TabbedComponent::backgroundColourId, c);
}

//==============================================================================
PreferencesWindow::PreferencesWindow ()
:   DocumentWindow ("",
                    Desktop::getInstance().getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId),
                    DocumentWindow::TitleBarButtons::closeButton | DocumentWindow::TitleBarButtons::minimiseButton)

{
    setUsingNativeTitleBar (true);
    setResizable (false, false);
    
    content = new AppPreferencesPanel();
    content->setSize (windowWidth, windowHeight);
    setContentOwned (content, true);
    
    centreWithSize (windowWidth, windowHeight);
    setVisible (true);
}

void PreferencesWindow::closeButtonPressed()
{
    close();
}

ScopedPointer<PreferencesWindow> preferencesWindow;

void PreferencesWindow::show()
{
    if (preferencesWindow == nullptr)
        preferencesWindow = new PreferencesWindow();
    
    preferencesWindow->toFront(true);
}

void PreferencesWindow::close()
{
    if (preferencesWindow != nullptr)
        preferencesWindow = nullptr;
}
