/*
  ==============================================================================

    LibraryPanel.cpp
    Created: 24 Sep 2018 2:05:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "../Source/WebAudio/LibraryPanel/LibraryPanel.h"

String PresetNavigation::PresetHolder::getIconString()
{
    return String ((char*)(&category), 1);
}

bool PresetNavigation::PresetHolder::containsSubPresetMatchingFilter (String filter)
{
    for (auto p : presets)
        if (p->getName().containsIgnoreCase (filter))
            return true;
    
    return false;
}

//==============================================================================
PresetNavigation::NavigationItem::NavigationItem (PresetNavigation& np, String name, GraphElementPreset* p, String iconString) : navigationPanel (np), preset (p), displayName (name), icon (iconString)
{
}

// TreeViewItem implementation...
String PresetNavigation::NavigationItem::getUniqueName() const
{
    return preset == nullptr ? displayName : preset->getFile().getFullPathName();
}

void PresetNavigation::NavigationItem::paintItem (Graphics& g, int width, int height)
{
    auto c = navigationPanel.getLookAndFeel().findColour (ResizableWindow::backgroundColourId).contrasting().withAlpha (0.1f);
    
    if (isSelected())
        g.fillAll (c);
}

void PresetNavigation::NavigationItem::itemOpennessChanged (bool isNowOpen)
{
    if (isNowOpen && getNumSubItems() == 0)
        refreshSubItems();
    else
        clearSubItems();
    
    if (getFilter().isEmpty())
        navigationPanel.storeOpenness();
}

Component* PresetNavigation::NavigationItem::createItemComponent()
{
    itemComp = navigationPanel.createComponentFor (this);
    return itemComp;
}

void PresetNavigation::NavigationItem::setAllParentsOpen (bool shouldBeOpen)
{
    setOpen (shouldBeOpen);
    
    if (auto parent = dynamic_cast<NavigationItem*> (getParentItem()))
        parent->setAllParentsOpen (shouldBeOpen);
}

String PresetNavigation::NavigationItem::getFilter() const
{
    return navigationPanel.getFilter();
}

PresetNavigation::NavigationItem* PresetNavigation::NavigationItem::findSubItemWithPreset (const GraphElementPreset* p)
{
    for (int i = 0; i < getNumSubItems(); ++i)
        if (auto navItem = dynamic_cast<NavigationItem*> (getSubItem (i)))
            if (auto itemFound = navItem->findSubItemWithPreset (p))
                return itemFound;
    
    return nullptr;
}

PresetNavigation::PresetCategory PresetNavigation::NavigationItem::getPresetCategory() const
{
    if (auto holder = navigationPanel.findPresetHolder(preset->getInterface()))
        return holder->category;
    
    return PresetCategory::otherCategory;
}

//==============================================================================

PresetNavigation::NavigationItemPreset::NavigationItemPreset (PresetNavigation& np, String name, GraphElementPreset* p, String iconString) : NavigationItem (np, name, p, iconString)
{}

PresetNavigation::NavigationItem* PresetNavigation::NavigationItemPreset::findSubItemWithPreset (const GraphElementPreset* p)
{
    return preset == p ? this : nullptr;
}

//==============================================================================

PresetNavigation::NavigationItemHolder::NavigationItemHolder (PresetNavigation& np, PresetNavigation::PresetHolder* ph)
: NavigationItem (np, ph->name, nullptr, ph->getIconString()), presetHolder (ph)
{}

void PresetNavigation::NavigationItemHolder::refreshSubItems()
{
    clearSubItems();
    
    for (auto p : presetHolder->presets)
    {
        if (p->getName().contains (getFilter()))
            addSubItem (new NavigationItemPreset (navigationPanel, p->getName(), p, getIconString()));
    }
}

//==============================================================================

PresetNavigation::NavigationCategory::NavigationCategory (PresetNavigation& np, String name, PresetNavigation::PresetHolder* ph) : NavigationItem (np, name, nullptr, ph->getIconString()), presetHolder (ph)
{}

void PresetNavigation::NavigationCategory::refreshSubItems()
{
    for (auto ph : navigationPanel.getPresetHolders())
    {
        if (ph->category == presetHolder->category)
            if (ph->name.containsIgnoreCase (getFilter()) || ph->containsSubPresetMatchingFilter (getFilter()))
                addSubItem (new NavigationItemHolder (navigationPanel, ph));
    }
}

//==============================================================================

PresetNavigation::NavigationRootItem::NavigationRootItem (PresetNavigation& np, String name)
: NavigationItem (np, name, nullptr, String()) {}

void PresetNavigation::NavigationRootItem::refreshSubItems()
{
    clearSubItems();
    
    addSubItem (new NavigationCategory (navigationPanel, "Audio contexts", new PresetHolder ("Audio contexts", PresetCategory::contextCategory)));
    addSubItem (new NavigationCategory (navigationPanel, "Audio data", new PresetHolder ("Audio data", PresetCategory::audioDataCategory)));
    addSubItem (new NavigationCategory (navigationPanel, "Audio nodes", new PresetHolder ("Audio nodes", PresetCategory::nodeCategory)));
    
    addSubItem (new NavigationItemHolder (navigationPanel, navigationPanel.findPresetHolder ("Scripts")));
    addSubItem (new NavigationItemHolder (navigationPanel, navigationPanel.findPresetHolder ("Snippets")));
}
//==============================================================================

PresetNavigation::PresetNavigation (Project& p) : project (p)
{
    root = new NavigationRootItem (*this, "Root");
    
    for (auto interf : dictionary->getContextInterfaceNames())
        addPresetHolder (interf, PresetCategory::contextCategory);
    
    for (auto interf : dictionary->getNodeInterfaceNames())
        addPresetHolder (interf, PresetCategory::nodeCategory);
    
    for (auto interf : dictionary->getAudioDataInterfaceNames())
        addPresetHolder (interf,  PresetCategory::audioDataCategory);
    
    addPresetHolder ("Scripts", PresetCategory::functionCategory);
    
    for (auto p : libraryManager->getPresets())
        if (auto holder = findPresetHolder (p->getInterface()))
            holder->presets.add (p);
    
    addPresetHolder ("Snippets", PresetCategory::snippetCategory);
    
    for (auto s : libraryManager->getSnippets())
        if (auto holder = findPresetHolder ("Snippets"))
            holder->presets.add (s);
    
    treeView.setRootItem (root);
    
    addAndMakeVisible (treeView);
    treeView.setDefaultOpenness (false);
    treeView.setIndentSize (12);
    treeView.setMultiSelectEnabled (false);
    treeView.setRootItemVisible (false);
}

void PresetNavigation::refresh()
{
    treeView.setDefaultOpenness (getFilter().isNotEmpty());
    
    root->refreshSubItems();
    
    if (getFilter().isEmpty() && previousOpenness != nullptr)
    {
        isCurrentlyRestoringOpennessState = true;
        treeView.restoreOpennessState (*previousOpenness.get(), true);
        isCurrentlyRestoringOpennessState = false;
    }
}

void PresetNavigation::addPreset (GraphElementPreset* p)
{
    if (auto holder = findPresetHolder (p->getInterface()))
        holder->presets.add (p);
    
    refresh();
}

void PresetNavigation::addSnippet (GraphElementPreset* p)
{
    if (auto holder = findPresetHolder ("Snippets"))
        holder->presets.add (p);
    
    refresh();
}

void PresetNavigation::removePreset (GraphElementPreset* preset)
{
    if (auto holder = findPresetHolder (preset->getInterface()))
        holder->presets.removeObject (preset);
    
    refresh();
}

String PresetNavigation::getFilter() const
{
    if (auto panel = findParentComponentOfClass<LibraryPanel>())
        return panel->getFilter();

    return String();
}

PresetNavigation::ItemComponent* PresetNavigation::createComponentFor (PresetNavigation::NavigationItem* item)
{
    return new ItemComponent (item, String (item->getIconString()), this, project);
}

void PresetNavigation::resized()
{
    treeView.setBounds (getLocalBounds());
}

void PresetNavigation::paint (Graphics &g)
{
    g.fillAll (Colours::black.withAlpha (0.05f));
}

PresetNavigation::PresetHolder* PresetNavigation::findPresetHolder (String name)
{
    for (auto ph : presetHolders)
        if (ph->name == name)
            return ph;
    
    return nullptr;
}

PresetNavigation::NavigationItem* PresetNavigation::findNavigationItemWithPreset (const GraphElementPreset* preset)
{
    if (root == nullptr)
        return nullptr;
    
    for (int i = 0; i < root->getNumSubItems(); ++i)
        if (auto item = dynamic_cast<NavigationItem*> (root->getSubItem (i)))
            if (auto foundItem = item->findSubItemWithPreset (preset))
                return foundItem;
    
    return nullptr;
}

void PresetNavigation::makeItemVisibleForPreset (const GraphElementPreset* preset)
{
    if (root == nullptr)
        return;
    
    const auto phToLookFor = findPresetHolder (preset->getInterface());
    
    if (phToLookFor == nullptr)
        return;
    
    const auto category = phToLookFor->category;
    
    for (int i = 0; i < root->getNumSubItems(); ++i)
    {
        if (auto categoryItem = dynamic_cast<NavigationCategory*> (root->getSubItem (i)))
        {
            if (category == categoryItem->getPresetCategory())
            {
                categoryItem->setOpen (true);
                
                for (int j = 0; j < categoryItem->getNumSubItems(); ++j)
                {
                    if (auto phItem = dynamic_cast<NavigationItemHolder*> (categoryItem->getSubItem (j)))
                    {
                        if (phItem->getPresetHolder() == phToLookFor)
                        {
                            phItem->setOpen (true);
                            break;
                        }
                    }
                }
                
                break;
            }
        }
        
        if (auto phItem = dynamic_cast<NavigationItemHolder*> (root->getSubItem (i)))
        {
            if (phItem->getPresetHolder() == phToLookFor)
            {
                phItem->setOpen (true);
                break;
            }
        }
    }
}

void PresetNavigation::storeOpenness()
{
    if (! isCurrentlyRestoringOpennessState)
        previousOpenness = treeView.getOpennessState (true);
}
//==============================================================================

PresetNavigation::ItemComponent::ItemComponent (NavigationItem* navItem, String icon, PresetNavigation* navPanel, Project& p) : iconText (icon), item (navItem), navigationPanel (navPanel), project (p)
{
    label.setText (navItem->getDisplayName(), dontSendNotification);
    label.addListener (this);
    label.setInterceptsMouseClicks (false, false);
    label.setMinimumHorizontalScale (0.9f);
    addAndMakeVisible (label);
    
    prepareCommandTarget();
}

bool PresetNavigation::ItemComponent::isUserPreset() const
{
    return dynamic_cast<PresetNavigation::NavigationItemPreset*> (item) != nullptr;
}

bool PresetNavigation::ItemComponent::isFactoryPreset() const
{
    if (auto holder = dynamic_cast<PresetNavigation::NavigationItemHolder*> (item))
        return holder->getDisplayName() != "Snippets";
    
    return false;
}

void PresetNavigation::ItemComponent::paint (Graphics &g)
{
    auto r = getLocalBounds();
    const auto iconArea = r.removeFromLeft (getHeight());
    
    const Colour bg (LookAndFeel::getDefaultLookAndFeel().findColour (ResizableWindow::backgroundColourId));
    
    // Icon
    g.setColour (bg.contrasting());
    g.fillRoundedRectangle (iconArea.reduced (3).toFloat(), 2.);
    
    g.setColour (bg);
    g.setFont (Font::fromString (Font::getDefaultMonospacedFontName()));
    g.drawFittedText (iconText, iconArea, Justification::centred, 1);
}

void PresetNavigation::ItemComponent::resized()
{
    auto r = getLocalBounds().withTrimmedLeft (getHeight() + 4);
    label.setBounds (r);
}

void PresetNavigation::ItemComponent::mouseDown (const MouseEvent& e)
{
    item->setSelected (true, true, dontSendNotification);
}

void PresetNavigation::ItemComponent::mouseUp (const MouseEvent& e)
{
    if (e.mouseWasClicked())
    {
        if (e.mods.isRightButtonDown())
            showPopupMenu();
        if (e.getNumberOfClicks() == 2)
            addToGraph();
    }
}

void PresetNavigation::ItemComponent::mouseDrag (const MouseEvent& e)
{
    if (auto container = DragAndDropContainer::findParentDragContainerFor (this))
    {
        var descr ("GraphElementPreset");
        container->startDragging (descr, this, Image(), true);
    }
}

#include "../../Project/Project.h"
#include "../../Application/CommandIDs.h"
void PresetNavigation::ItemComponent::showPopupMenu()
{
    PopupMenu m;
    
    if (isUserPreset() || isFactoryPreset())
    {
        m.addItem (3, "Add to graph");
        m.addSeparator();
    }
    
    if (isUserPreset())
    {
        auto& acm = project.getApplicationCommandManager();
        m.addCommandItem (&acm, CommandIDs::rename);
        m.addCommandItem (&acm, CommandIDs::del);
        m.addItem (4,
                    #if JUCE_MAC
                        "Reveal in Finder");
                    #else
                        "Reveal in Explorer");
                    #endif
    }
    
    const int result = m.show();
    
    if (result == 0)        { }
    else if (result == 3)   addToGraph();
    else if (result == 4)   showInNativeFileExplorer();
}

void PresetNavigation::ItemComponent::showInNativeFileExplorer() const
{
    if (auto nav = dynamic_cast<PresetNavigation::NavigationItem*> (item))
        if (auto preset = nav->getPreset())
            preset->getFile().revealToUser();
}

#include "../WebAudioGraph/WebAudioGraph.h"
void PresetNavigation::ItemComponent::addToGraph()
{
    if (! (isFactoryPreset() || isUserPreset()))
        return;
    
    auto graph = project.findStaticPanelWithClass<RootWebAudioGraphPanel>();
    
    if (graph == nullptr)
        return;
    
    Point<int> pos = graph->getCenterOfVisibleGraphArea();
    
    if (auto holder = dynamic_cast<PresetNavigation::NavigationItemHolder*> (item))
    {
        SharedResourcePointer<WebAudioDictionary> dict;
        auto newElem = graph->createAndAddUndoable (dict->findDescriptorForInterface (holder->getDisplayName()), pos);
        
        if (newElem != nullptr)
        {
            graph->getSelector().setUniqueSelection (newElem);
            newElem->showNameEditor();
        }
    }
    else if (auto nav = dynamic_cast<PresetNavigation::NavigationItem*> (item))
    {
        if (auto preset = nav->getPreset())
        {
            auto file = preset->getFile();
            
            if (! file.existsAsFile())
                return;
            
            graph->pastePreset (file.loadFileAsString(), pos);
        }
    }
}

void PresetNavigation::ItemComponent::showNameEditor()
{        
    label.showEditor();
}

void PresetNavigation::ItemComponent::labelTextChanged (Label*)
{
    if (item->getPreset() == nullptr)
        return;
    
    if (auto lib = project.findStaticPanelWithClass<LibraryPanel>())
    {
        lib->renamePreset (item->getPreset(), label.getText());
        
        const String newName (item->getPreset()->getName());
        
        if (newName != label.getText())
            label.setText (newName, dontSendNotification);
    }
}

void PresetNavigation::ItemComponent::editorHidden (Label *, TextEditor &)
{
    label.setInterceptsMouseClicks (false, false);
}

void PresetNavigation::ItemComponent::remove()
{
    if (auto lib = project.findStaticPanelWithClass<LibraryPanel>())
        lib->removePreset (item->getPreset());
}

void PresetNavigation::ItemComponent::prepareCommandTarget()
{
    auto& commandManager = Project::getApplicationCommandManager();
    addKeyListener (commandManager.getKeyMappings());
    setWantsKeyboardFocus(true);
    
    commandManager.registerAllCommandsForTarget (this);
}

ApplicationCommandTarget* PresetNavigation::ItemComponent::getNextCommandTarget()
{
    return findFirstTargetParentComponent();
}

void PresetNavigation::ItemComponent::getAllCommands (Array<CommandID>& commands)
{
    const CommandID ids[] = {
        CommandIDs::del,
        CommandIDs::rename
    };
    
    commands.addArray (ids, numElementsInArray (ids));
}

void PresetNavigation::ItemComponent::getCommandInfo (CommandID commandID, ApplicationCommandInfo& result)
{
    const String editCategory ("Edit");
    
    switch (commandID)
    {
        case CommandIDs::del :
            result.setInfo ("Delete", "Delete the selection", editCategory, 0);
            result.addDefaultKeypress (KeyPress::backspaceKey, ModifierKeys::noModifiers);
            result.addDefaultKeypress (KeyPress::deleteKey, ModifierKeys::noModifiers);
            result.setActive (isUserPreset());
            break;
        case CommandIDs::rename :
            result.setInfo ("Rename", "Renames the selected element", editCategory, 0);
            result.addDefaultKeypress ('r', ModifierKeys::commandModifier);
            result.setActive (isUserPreset());
            break;
    }
}

bool PresetNavigation::ItemComponent::perform (const InvocationInfo& info)
{
    switch (info.commandID)
    {
        case CommandIDs::del :
            remove();
            break;
        case CommandIDs::rename :
            showNameEditor();
            break;
    }
    
    return true;
}

//==============================================================================

LibraryPanel::LibraryPanel (Project& p) : Panel (&p.getPanelManager()), navigation (p)
{
    addAndMakeVisible (navigation);
    
    addAndMakeVisible (filterEditor);
    filterEditor.setTextToShowWhenEmpty ("Filter...", Colours::grey);
    filterEditor.setColour (TextEditor::outlineColourId, Colours::transparentBlack);
    filterEditor.setColour (TextEditor::backgroundColourId, Colours::transparentBlack);
    filterEditor.addListener (this);
    
    setWantsKeyboardFocus (true);
    
    setPanelName ("Library");
}

void LibraryPanel::paint (Graphics& g)
{
    Panel::paint (g);
    
    auto r = getLocalBounds().withTrimmedTop (getHeaderHeight());
    auto top = r.removeFromTop (22);
    
    g.setColour (Colours::black.withAlpha (0.2f));
    g.fillRect (r.removeFromTop (1));
}

void LibraryPanel::resized()
{
    Panel::resized();
    
    auto r = getLocalBounds().withTrimmedTop (getHeaderHeight());
    auto top = r.removeFromTop (22);
    
    filterEditor.setBounds (top);
    navigation.setBounds (r.reduced (0, 1));
}

void LibraryPanel::addPresetFile (String instance, String interf, String doc, bool showNameEditor)
{
    SharedResourcePointer<UserLibraryManager> library;
    auto p = library->addPresetFile (instance, interf, doc);
    
    if (p == nullptr)
        return;
    
    navigation.addPreset (p);
    
    if (showNameEditor)
    {
        filterEditor.setText ("", sendNotification);
        presetToShowNameFor = p;
        triggerAsyncUpdate();
    }
}

void LibraryPanel::removePreset (GraphElementPreset* preset)
{
    SharedResourcePointer<UserLibraryManager> library;
    library->removePreset (preset);
    navigation.removePreset (preset);
}

void LibraryPanel::renamePreset (GraphElementPreset* preset, String desiredName)
{
    if (preset == nullptr)
        return;
    
    SharedResourcePointer<UserLibraryManager> library;
    library->renamePreset (preset, desiredName);
}

void LibraryPanel::addSnippetFile (String doc, bool showNameEditor)
{
    SharedResourcePointer<UserLibraryManager> library;
    auto p = library->addSnippetFile ("snippet", doc);
    navigation.addSnippet (p);
    
    if (showNameEditor)
    {
        filterEditor.setText ("", sendNotification);
        presetToShowNameFor = p;
        triggerAsyncUpdate();
    }
}

void LibraryPanel::handleAsyncUpdate()
{
    if (presetToShowNameFor == nullptr)
        return;
    
    navigation.makeItemVisibleForPreset (presetToShowNameFor);
    
    if (auto item = navigation.findNavigationItemWithPreset (presetToShowNameFor))
    {
        navigation.getTreeView().scrollToKeepItemVisible (item);
        
        if (auto comp = item->getItemComponent())
        {
            gainFocus();
            comp->showNameEditor();
            return;
        }
    }
}
