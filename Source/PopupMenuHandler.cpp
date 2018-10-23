/*
  ==============================================================================

    PopupMenuHandler.cpp
    Created: 18 Aug 2018 4:14:57pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "PopupMenuHandler.h"

void PopupMenuHandler::setCaller (PopupMenuHandlerClient* comp)
{
    caller = comp;
}

void PopupMenuHandler::handleResult (int result, Point<int> pos)
{
    if (caller != nullptr)
        caller->handleExtraPopupMenuCommands (result, pos);
}
