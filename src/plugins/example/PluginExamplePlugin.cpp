#include "PluginExampleInterface.h"
#include "PluginExamplePlugin.h"
#include <QMessageBox>

void PluginExamplePlugin::initPluginExample(void)
{
    QMessageBox::warning (0,
        "Plugin Example",
        "Hello world, you are inside the plugin example");
}
