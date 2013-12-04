#include "PluginExampleInterface.h"
#include "PluginExamplePlugin.h"
#include <QMessageBox>

void PluginExamplePlugin::initPluginExample(MainWindowInterface *main)
{
    if(main->get_selPOI_instruction())
        QMessageBox::warning (0,"Plugin Example","Hello world, you are inside the plugin example\nmain returned true");
    else
        QMessageBox::warning (0,"Plugin Example","Hello world, you are inside the plugin example\nmain returned false");

}
