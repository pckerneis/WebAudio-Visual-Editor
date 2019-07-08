/*
  ==============================================================================

    LibraryPanel.h
    Created: 24 Sep 2018 2:05:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../Source/Layout/Panels/Panel.h"
#include "../Source/Layout/Widgets/ShapeButtons.h"
#include "../Source/WebAudio/LibraryPanel/UserLibraryManager.h"
#include "../WebAudioGraph/WebAudioDictionary.h"

class Project;

class PresetNavigation : public Component, public DragAndDropContainer
{
public:
    enum PresetCategory : char
    {
        nodeCategory = 'n',
        contextCategory = 'c',
        audioDataCategory = 'd',
        snippetCategory = 's',
        functionCategory = 'f',
        otherCategory = ' '
    };
    
    //==============================================================================
    class PresetHolder
    {
    public:
        PresetHolder (String n, PresetCategory i) : name (n), category (i) {}
        ~PresetHolder() { masterReference.clear(); }
        
        const String name;
        const PresetCategory category;
        ReferenceCountedArray<GraphElementPreset> presets;
        
        String getIconString();
        
        bool containsSubPresetMatchingFilter (String filter);
        
    private:
        WeakReference<PresetHolder>::Master masterReference;
        friend class WeakReference<PresetHolder>;
    };
    
    //==============================================================================
    class ItemComponent;
    
    class NavigationItem    :   public TreeViewItem
    {
    public:
        /** \brief Constructor. */
        NavigationItem (PresetNavigation& np, String name, GraphElementPreset* p, String iconString);
        ~NavigationItem() { masterReference.clear(); }
        
        String getIconString() const { return icon; }
        String getDisplayName() const { return displayName; }
        GraphElementPreset* getPreset() const { return preset; }
        
        // TreeViewItem implementation...
        String getUniqueName() const override;
        void paintItem (Graphics& g, int width, int height) override;
        void itemOpennessChanged (bool isNowOpen) override;
        Component* createItemComponent() override;
        int getItemHeight() const override { return 18; }
        
        void setAllParentsOpen (bool shouldBeOpen);
        String getFilter() const;
        ItemComponent* getItemComponent() const { return itemComp; }
        
        virtual void refreshSubItems() = 0;
        virtual NavigationItem* findSubItemWithPreset (const GraphElementPreset* p);
        virtual PresetCategory getPresetCategory() const;
        
    protected:
        PresetNavigation& navigationPanel;
        const GraphElementPreset::Ptr preset;
        
    private:
        WeakReference<NavigationItem>::Master masterReference;
        friend class WeakReference<NavigationItem>;
        
        String displayName;
        String icon;
        WeakReference<ItemComponent> itemComp;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationItem)
    };
    
    //==============================================================================
    class NavigationItemPreset : public NavigationItem
    {
    public:
        NavigationItemPreset (PresetNavigation& np, String name, GraphElementPreset* p, String iconString);
        
        void refreshSubItems() override {}
        bool mightContainSubItems() override { return false; }
        
        NavigationItem* findSubItemWithPreset (const GraphElementPreset* p) override;
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationItemPreset)
    };
    
    //==============================================================================
    class NavigationItemHolder : public NavigationItem
    {
    public:
        NavigationItemHolder (PresetNavigation& np, PresetNavigation::PresetHolder* ph);
        void refreshSubItems() override;
        bool mightContainSubItems() override { return presetHolder->presets.size() > 0; }
        
        PresetCategory getPresetCategory() const override { return presetHolder ? presetHolder->category : PresetCategory::otherCategory; }
        PresetHolder* getPresetHolder() const { return presetHolder; }
        
    private:
        WeakReference<PresetHolder> presetHolder;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationItemHolder)
    };
    
    //==============================================================================
    class NavigationCategory : public NavigationItem
    {
    public:
        NavigationCategory (PresetNavigation& np, String name, PresetNavigation::PresetHolder* ph);
        
        void refreshSubItems() override;
        bool mightContainSubItems() override { return true; }
        
        PresetCategory getPresetCategory() const override { return presetHolder ? presetHolder->category : PresetCategory::otherCategory; }
        
    private:
        WeakReference<PresetHolder> presetHolder;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationCategory)
    };
    
    //==============================================================================
    class NavigationRootItem : public NavigationItem
    {
    public:
        NavigationRootItem (PresetNavigation& np, String name);
        void refreshSubItems() override;
        bool mightContainSubItems() override { return true; }
        String getUniqueName() const override { return "root"; }
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NavigationRootItem)
    };
    
    //==============================================================================
    class ItemComponent : public Component, Label::Listener, public ApplicationCommandTarget
    {
    public:
        ItemComponent (NavigationItem* navItem, String icon, PresetNavigation* navPanel, Project& p);
        ~ItemComponent() { masterReference.clear(); }
        
        NavigationItem* getNavigationItem() const { return item; }
        
        bool isUserPreset() const;
        bool isFactoryPreset() const;
        
        void paint (Graphics &g) override;
        void resized() override;
        
        void mouseDown (const MouseEvent& e) override;
        void mouseUp (const MouseEvent& e) override;
        void mouseDrag (const MouseEvent& e) override;
        
        void showPopupMenu();
        
        void showInNativeFileExplorer() const;
        
        void addToGraph();
        
        void showNameEditor();
        void labelTextChanged (Label*) override;
        void editorHidden (Label *, TextEditor &) override;
        
        void remove();
        
        //==============================================================================
        void prepareCommandTarget();
        ApplicationCommandTarget* getNextCommandTarget() override;
        void getAllCommands (Array<CommandID>& commands) override;
        void getCommandInfo (CommandID commandID, ApplicationCommandInfo& result) override;
        bool perform (const InvocationInfo& info) override;
        
    private:
        WeakReference<ItemComponent>::Master masterReference;
        friend class WeakReference<ItemComponent>;
        
        Label label;
        String iconText;
        NavigationItem* item;
        PresetNavigation* navigationPanel;
        Project& project;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ItemComponent)
    };
    //==============================================================================

    PresetNavigation (Project& p);
    
    void refresh();
    
    void addPreset (GraphElementPreset* p);
    void removePreset (GraphElementPreset* p);
    void addSnippet (GraphElementPreset* p);
    
    ItemComponent* createComponentFor (PresetNavigation::NavigationItem* item);
    
    TreeView& getTreeView() { return treeView; }
    String getFilter() const;
            
    void resized() override;
    void paint (Graphics &g) override;
    
    void addPresetHolder (String name, PresetCategory cat) { presetHolders.add (new PresetHolder (name, cat)); }
    const OwnedArray<PresetHolder>& getPresetHolders() const { return presetHolders; }
    
    NavigationItem* findNavigationItemWithPreset (const GraphElementPreset* preset);
    void makeItemVisibleForPreset (const GraphElementPreset* preset);
    void storeOpenness();
    
private:
    PresetHolder* findPresetHolder (String name);
    
    TreeView treeView;
    
    Array<WeakReference<NavigationItem>> interfaceItems;
    ScopedPointer<PresetNavigation::NavigationRootItem> root;
    OwnedArray<PresetHolder> presetHolders;
    
    Project& project;
    SharedResourcePointer<UserLibraryManager> libraryManager;
    SharedResourcePointer<WebAudioDictionary> dictionary;
    
    ScopedPointer<XmlElement> previousOpenness;
    bool isCurrentlyRestoringOpennessState = false;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PresetNavigation)
};

//==============================================================================
#include "../Source/Layout/Panels/Panel.h"

class LibraryPanel : public Panel, TextEditor::Listener, public AsyncUpdater
{
public:
    LibraryPanel (Project& p);
    ~LibraryPanel() {}
    
    String getFilter() const { return filterEditor.getText(); }
    
    void paint (Graphics& g) override;
    void resized() override;
    
    void addPresetFile (String instance, String interf, String doc, bool showNameEditor = false);
    void removePreset (GraphElementPreset* preset);
    void renamePreset (GraphElementPreset* preset, String desiredName);
    
    void addSnippetFile (String doc, bool showNameEditor = false);
    
    void textEditorTextChanged (TextEditor&) override { navigation.refresh(); }
    
    void handleAsyncUpdate() override;
    
private:
    TextEditor filterEditor;
    PresetNavigation navigation;
    
    GraphElementPreset::Ptr presetToShowNameFor;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LibraryPanel);
};
