/*
  ==============================================================================

    WebServer.h
    Created: 16 Aug 2018 11:36:11pm
    Author:  Pierre-Clément KERNEIS

  ==============================================================================
*/

#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class LocalServer   : public Mongoose::WebController
{
public:
    //==============================================================================
    LocalServer (File directory, int port = 8080);    
    ~LocalServer();
    
    void stream (Mongoose::Request &request, Mongoose::StreamResponse &response) {}
    
    File getDirectory() const { return dir; }
    
    int getPortNumber() const { return portNumber; }
    
private:
    ScopedPointer<Mongoose::Server> server;
    int portNumber;
    File dir;
    
    struct PortNumberManager
    {
        Array<int> portNumbers;
        
        int getAvailablePortNumber (int desiredPort, bool addFound);
    };
    
    SharedResourcePointer<PortNumberManager> portNumberManager;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LocalServer)
};
