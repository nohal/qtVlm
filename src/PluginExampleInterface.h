#ifndef PLUGINEXAMPLEINTERFACE_H
#define PLUGINEXAMPLEINTERFACE_H
#include <QtPlugin>
#include "MainWindowInterface.h"
class PluginExampleInterface
{
public:
    virtual ~PluginExampleInterface(){}
    virtual void initPluginExample(MainWindowInterface * main)=0;
};
Q_DECLARE_INTERFACE(PluginExampleInterface,"qtVlm.plugins.pluginInterfaceExample/1.0")

#endif // PLUGINEXAMPLEINTERFACE_H
