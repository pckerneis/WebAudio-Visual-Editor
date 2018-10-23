/*
  ==============================================================================

    WebServer.cpp
    Created: 16 Aug 2018 11:36:11pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#include "WebServer.h"

LocalServer::LocalServer (File directory, int port) : portNumber (port), dir (directory)
{
    portNumber = portNumberManager->getAvailablePortNumber (portNumber, true);
    
    server = new Mongoose::Server (portNumber, directory.getFullPathName().getCharPointer());
    server->registerController (this);
    server->start();
}

LocalServer::~LocalServer()
{
    portNumberManager->portNumbers.removeFirstMatchingValue (portNumber);
}

int LocalServer::PortNumberManager::getAvailablePortNumber (int desiredPort, bool addFound)
{
    if (portNumbers.contains (desiredPort))
        return getAvailablePortNumber (desiredPort + 1, addFound);
    
    // Found
    if (addFound)
        portNumbers.add (desiredPort);
    
    return desiredPort;
}
