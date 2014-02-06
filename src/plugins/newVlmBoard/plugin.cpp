/**********************************************************************
qtVlm: Virtual Loup de mer GUI
Copyright (C) 2013 - Christophe Thomas aka Oxygen77

http://qtvlm.sf.net

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
***********************************************************************/
#include "BoardVlmNew.h"

#ifdef QT_V5
#include <QtCore/qplugin.h>
#else
#include <QtPlugin>
#endif
#include <QtDesigner/QDesignerContainerExtension>
#include <QtDesigner/QDesignerCustomWidgetInterface>

class BoardVlmNewPlugin: public  QObject, public QDesignerCustomWidgetInterface
{
   Q_OBJECT
   Q_INTERFACES(QDesignerCustomWidgetInterface)
#ifdef QT_V5
   Q_PLUGIN_METADATA(IID "qtVlm.plugins.boardInterface/1.1")
#endif
private:
   bool             initialized;

public:
   explicit BoardVlmNewPlugin (void): initialized (false) {}

   QString  name          (void) const { return "BoardVlmNew"; }
   bool     isContainer   (void) const { return true; /* Required for it to be a toplevel widget in Qt designer */ }
   bool     isInitialized (void) const { return initialized; }
   QIcon    icon          (void) const { return QIcon(); }
   QString  codeTemplate  (void) const { return QString(); }
   QString  whatsThis     (void) const { return QString(); }
   QString  toolTip       (void) const { return QString(); }
   QString  group         (void) const { return "QtVlm"; }

   QString includeFile (void) const { return "BoardVlmNew.h"; }

   QWidget* createWidget (QWidget* parent)
   {
      return new BoardVlmNew (parent);
   }

   void initialize (QDesignerFormEditorInterface*)
   {
      if (initialized) return;
      initialized = true;
   }
};
#ifndef QT_V5
Q_EXPORT_PLUGIN2(newVlmBoard,BoardVlmNewPlugin)

#endif
#include "plugin.moc"
