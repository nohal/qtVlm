
#ifndef PLUGINEXAMPLEPLUGIN_H
#define PLUGINEXAMPLEPLUGIN_H

#include <QDialog>
#include "PluginExampleInterface.h"
#include "MainWindowInterface.h"
class Q_DECL_EXPORT PluginExamplePlugin : public QObject, public PluginExampleInterface
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "qtVlm.plugins.pluginInterfaceExample/1.0")
    Q_INTERFACES(PluginExampleInterface)

public:
    void initPluginExample(MainWindowInterface * main);
};
#endif // PLUGINEXAMPLEPLUGIN_H
