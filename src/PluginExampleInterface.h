#ifndef PLUGINEXAMPLEINTERFACE_H
#define PLUGINEXAMPLEINTERFACE_H
#include <QtPlugin>
class PluginExampleInterface
{
public:
    virtual ~PluginExampleInterface(){}
    virtual void initPluginExample(void)=0;
};
Q_DECLARE_INTERFACE(PluginExampleInterface,"qtVlm.plugins.pluginInterfaceExample/1.0")

#endif // PLUGINEXAMPLEINTERFACE_H
