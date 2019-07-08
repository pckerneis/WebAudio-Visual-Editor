/*
  ==============================================================================

    ConsoleComponent.cpp
    Created: 19 Apr 2018 2:13:48pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "ConsoleComponent.h"

ConsoleComponent::ConsoleComponent()
{
    display.setMultiLine (true);
    display.setReadOnly (true);
    display.setCaretVisible (false);
    display.setFont (Font (Font::getDefaultMonospacedFontName(), 14.0f, Font::plain));
    addAndMakeVisible (&display);
}

void ConsoleComponent::resized()
{
    display.setBounds (getLocalBounds());
}

void ConsoleComponent::print (const String& message)
{
    // Check if caller thread is message thread
    if (auto mm = MessageManager::getInstance())
    {
        if (!mm->isThisTheMessageThread())
        {
            Message msg ({ &display, message });
            mm->callFunctionOnMessageThread (printInternal, (void*)(&msg));
            return;
        }
    }
    
    display.moveCaretToEnd();
    display.insertTextAtCaret (message);
    
}

void ConsoleComponent::println (const String& message)
{
    print (message + newLine);
}

void ConsoleComponent::clear()
{
    // Check if caller thread is message thread
    if (auto mm = MessageManager::getInstance())
    {
        if (!mm->isThisTheMessageThread())
        {
            Message msg ({ &display, String() });
            mm->callFunctionOnMessageThread (clearInternal, (void*)(&msg));
            return;
        }
    }
    
    display.clear();
}

void* ConsoleComponent::printInternal (void* udata)
{
    if (auto msg = static_cast<Message*> (udata))
    {
        if (auto display = msg->display)
        {
            display->moveCaretToEnd();
            display->insertTextAtCaret (msg->text);
        }
    }
    
    return nullptr;
}

void* ConsoleComponent::clearInternal (void* udata)
{
    if (auto msg = static_cast<Message*> (udata))
        if (auto display = msg->display)
            display->clear();
    
    return nullptr;
}

//==============================================================================

#include "../Source/Project/Project.h"
#include "../Source/Application/CommandIDs.h"
void ConsoleComponent::ConsoleEditor::addPopupMenuItems (PopupMenu &m, const MouseEvent *mouseClickEvent)
{
    TextEditor::addPopupMenuItems (m, mouseClickEvent);
    m.addCommandItem (&Project::getApplicationCommandManager(), CommandIDs::clearConsole);
}
