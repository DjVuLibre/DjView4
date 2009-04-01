//C-  -*- C++ -*-
//C- -------------------------------------------------------------------
//C- DjView4
//C- Copyright (c) 2006-  Leon Bottou
//C-
//C- This software is subject to, and may be distributed under, the
//C- GNU General Public License, either version 2 of the license,
//C- or (at your option) any later version. The license should have
//C- accompanied the software or you may obtain a copy of the license
//C- from the Free Software Foundation at http://www.fsf.org .
//C-
//C- This program is distributed in the hope that it will be useful,
//C- but WITHOUT ANY WARRANTY; without even the implied warranty of
//C- MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//C- GNU General Public License for more details.
//C-  ------------------------------------------------------------------

// $Id$

#if AUTOCONF
# include "config.h"
#else
# define HAVE_STRING_H 1
# define HAVE_STRERROR 1
# ifndef WIN32
#  define HAVE_UNISTD_H 1
# endif
#endif

#if HAVE_STRING_H
# include <string.h>
#endif
#if HAVE_UNISTD_H
# include <unistd.h>
#endif
#include <errno.h>

#include <libdjvu/miniexp.h>
#include <libdjvu/ddjvuapi.h>

#include <QAction>
#include <QActionGroup>
#include <QApplication>
#include <QBoxLayout>
#include <QClipboard>
#include <QCloseEvent>
#include <QComboBox>
#include <QCoreApplication>
#include <QDebug>
#include <QDialog>
#include <QDir>
#include <QDockWidget>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QEvent>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFontMetrics>
#include <QBoxLayout>
#include <QFont>
#include <QFrame>
#include <QIcon>
#include <QImageWriter>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QList>
#include <QMainWindow>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QMimeData>
#include <QObject>
#include <QPair>
#include <QPalette>
#include <QProcess>
#include <QRegExp>
#include <QRegExp>
#include <QRegExpValidator>
#include <QScrollBar>
#include <QSettings>
#include <QShortcut>
#include <QSlider>
#include <QStackedLayout>
#include <QStatusBar>
#include <QString>
#include <QTextStream>
#include <QTimer>
#include <QTabWidget>
#include <QToolBar>
#include <QUrl>
#include <QWhatsThis>

#include "qdjvu.h"
#include "qdjvuhttp.h"
#include "qdjvuwidget.h"
#include "qdjview.h"
#include "qdjviewprefs.h"
#include "qdjviewdialogs.h"
#include "qdjviewsidebar.h"



/*! \class QDjView
  \brief The main viewer interface.

  Class \a QDjView defines the djvu viewer graphical user interface. It is
  composed of a main window with menubar, toolbar, statusbar and a dockable
  sidebar.  The center is occupied by a \a QDjVuWidget. */



// ----------------------------------------
// FILL USER INTERFACE COMPONENTS


void
QDjView::fillPageCombo(QComboBox *pageCombo)
{
  pageCombo->clear();
  int n = documentPages.size();
  for (int j=0; j<n; j++)
    {
      QString name = pageName(j);
      pageCombo->addItem(name, QVariant(j));
    }
}


void
QDjView::fillZoomCombo(QComboBox *zoomCombo)
{
  zoomCombo->clear();
  zoomCombo->addItem(tr("FitWidth","zoomCombo"), QDjVuWidget::ZOOM_FITWIDTH);
  zoomCombo->addItem(tr("FitPage","zoomCombo"), QDjVuWidget::ZOOM_FITPAGE);
  zoomCombo->addItem(tr("Stretch","zoomCombo"), QDjVuWidget::ZOOM_STRETCH);
  zoomCombo->addItem(tr("1:1","zoomCombo"), QDjVuWidget::ZOOM_ONE2ONE);
  zoomCombo->addItem(tr("300%","zoomCombo"), 300);
  zoomCombo->addItem(tr("200%","zoomCombo"), 200);
  zoomCombo->addItem(tr("150%","zoomCombo"), 150);
  zoomCombo->addItem(tr("100%","zoomCombo"), 100);
  zoomCombo->addItem(tr("75%","zoomCombo"), 75);
  zoomCombo->addItem(tr("50%","zoomCombo"), 50);
}


void
QDjView::fillModeCombo(QComboBox *modeCombo)
{
  modeCombo->clear();
  modeCombo->addItem(tr("Color","modeCombo"), QDjVuWidget::DISPLAY_COLOR);
  modeCombo->addItem(tr("Stencil","modeCombo"), QDjVuWidget::DISPLAY_STENCIL);
  modeCombo->addItem(tr("Foreground","modeCombo"), QDjVuWidget::DISPLAY_FG);
  modeCombo->addItem(tr("Background","modeCombo"), QDjVuWidget::DISPLAY_BG);
  modeCombo->addItem(tr("Hidden Text","modeCombo"), QDjVuWidget::DISPLAY_TEXT);
}  


void
QDjView::fillToolBar(QToolBar *toolBar)
{
  // Hide toolbar
  bool wasHidden = toolBar->isHidden();
  toolBar->hide();
  toolBar->clear();
  // Hide combo boxes
  modeCombo->hide();
  pageCombo->hide();
  zoomCombo->hide();
  // Use options to compose toolbar
  if (viewerMode == STANDALONE) 
    {
      if (tools & QDjViewPrefs::TOOL_NEW)
        toolBar->addAction(actionNew);
      if (tools & QDjViewPrefs::TOOL_OPEN)
        toolBar->addAction(actionOpen);
    }
  if (tools & QDjViewPrefs::TOOL_SAVE)
    {
      toolBar->addAction(actionSave);
    }
  if (tools & QDjViewPrefs::TOOL_PRINT)
    {
      toolBar->addAction(actionPrint);
    }
  if (tools & QDjViewPrefs::TOOL_FIND)
    {
      toolBar->addAction(actionFind);
    }
  if (tools & QDjViewPrefs::TOOL_SELECT)
    {
      toolBar->addAction(actionSelect);
    }
  if (tools & QDjViewPrefs::TOOL_LAYOUT)
    {
      toolBar->addAction(actionLayoutContinuous);
      toolBar->addAction(actionLayoutSideBySide);
    }
  if ((tools & QDjViewPrefs::TOOL_MODECOMBO) ||
      (tools & QDjViewPrefs::TOOL_MODEBUTTONS) )
    {
      modeCombo->show();
      toolBar->addWidget(modeCombo);
    }
  if (tools & QDjViewPrefs::TOOL_ZOOMCOMBO)
    {
      zoomCombo->show();
      toolBar->addWidget(zoomCombo);
    }
  if (tools & QDjViewPrefs::TOOL_ZOOMBUTTONS)
    {
      toolBar->addAction(actionZoomIn);
      toolBar->addAction(actionZoomOut);
    }
  if (tools & QDjViewPrefs::TOOL_ROTATE)
    {
      toolBar->addAction(actionRotateRight);
      toolBar->addAction(actionRotateLeft);
    }
  if (tools & QDjViewPrefs::TOOL_PAGECOMBO)
    {
      pageCombo->show();
      toolBar->addWidget(pageCombo);
    }
  if (tools & QDjViewPrefs::TOOL_FIRSTLAST)
    {
      toolBar->addAction(actionNavFirst);
    }
  if (tools & QDjViewPrefs::TOOL_PREVNEXT)
    {
      toolBar->addAction(actionNavPrev);
      toolBar->addAction(actionNavNext);
    }
  if (tools & QDjViewPrefs::TOOL_FIRSTLAST)
    {
      toolBar->addAction(actionNavLast);
    }
  if (tools & QDjViewPrefs::TOOL_BACKFORW)
    {
      toolBar->addAction(actionBack);
      toolBar->addAction(actionForw);
    }
  if (tools & QDjViewPrefs::TOOL_WHATSTHIS)
    {
      toolBar->addAction(actionWhatsThis);
    }
  // Allowed areas
  Qt::ToolBarAreas areas = 0;
  if (tools & QDjViewPrefs::TOOLBAR_TOP)
    areas |= Qt::TopToolBarArea;
  if (tools & QDjViewPrefs::TOOLBAR_BOTTOM)
    areas |= Qt::BottomToolBarArea;
  if (!areas)
    areas = Qt::TopToolBarArea | Qt::BottomToolBarArea;
  toolBar->setAllowedAreas(areas);
  if (! (toolBarArea(toolBar) & areas))
    {
      removeToolBar(toolBar);
      if (areas & Qt::TopToolBarArea)
        addToolBar(Qt::TopToolBarArea, toolBar);
      else
        addToolBar(Qt::BottomToolBarArea, toolBar);
    }
  // Done
  toolBar->setVisible(!wasHidden);
  toolsCached = tools;
}




// ----------------------------------------
// ACTIONS


QAction *
QDjView::makeAction(QString text)
{
  return new QAction(text, this);
}

QAction *
QDjView::makeAction(QString text, bool value)
{
  QAction *action = new QAction(text, this);
  action->setCheckable(true);
  action->setChecked(value);
  return action;
}

static inline QAction * 
operator<<(QAction *action, QIcon icon)
{
  action->setIcon(icon);
  return action;
}

static inline QAction * 
operator<<(QAction *action, QActionGroup &group)
{
  action->setActionGroup(&group);
  return action;
}

static inline QAction * 
operator<<(QAction *action, QKeySequence shortcut)
{
  QList<QKeySequence> shortcuts = action->shortcuts();
# ifdef Q_WS_MAC
  shortcuts.append(shortcut);
# else
  shortcuts.prepend(shortcut);
# endif
  action->setShortcuts(shortcuts);
  return action;
}

static inline QAction * 
operator<<(QAction *action, QString string)
{
  if (action->text().isEmpty())
    action->setText(string);
  else if (action->statusTip().isEmpty())
    action->setStatusTip(string);
  else if (action->whatsThis().isEmpty())
    action->setWhatsThis(string);
  return action;
}
  
struct Trigger {
  QObject *object;
  const char *slot;
  Trigger(QObject *object, const char *slot)
    : object(object), slot(slot) { } 
};

static inline QAction *
operator<<(QAction *action, Trigger trigger)
{
  QObject::connect(action, SIGNAL(triggered(bool)),
                   trigger.object, trigger.slot);
  return action;
}

static inline QAction *
operator<<(QAction *action, QVariant variant)
{
  action->setData(variant);
  return action;
}

void
QDjView::createActions()
{
  // Create action groups
  zoomActionGroup = new QActionGroup(this);
  modeActionGroup = new QActionGroup(this);
  rotationActionGroup  = new QActionGroup(this);
  
  // Create all actions
  actionNew = makeAction(tr("&New", "File|"))
    << QKeySequence(tr("Ctrl+N", "File|New"))
    << QIcon(":/images/icon_new.png")
    << tr("Create a new DjView window.")
    << Trigger(this, SLOT(performNew()));

  actionOpen = makeAction(tr("&Open", "File|"))
    << QKeySequence(tr("Ctrl+O", "File|Open"))
    << QIcon(":/images/icon_open.png")
    << tr("Open a DjVu document.")
    << Trigger(this, SLOT(performOpen()));

  actionOpenLocation = makeAction(tr("Open &Location...", "File|"))
    << tr("Open a remote DjVu document.")
    << QIcon(":/images/icon_web.png")
    << Trigger(this, SLOT(performOpenLocation()));

  actionClose = makeAction(tr("&Close", "File|"))
    << QKeySequence(tr("Ctrl+W", "File|Close"))
    << QIcon(":/images/icon_close.png")
    << tr("Close this window.")
    << Trigger(this, SLOT(close()));

  actionQuit = makeAction(tr("&Quit", "File|"))
    << QKeySequence(tr("Ctrl+Q", "File|Quit"))
    << QIcon(":/images/icon_quit.png")
    << tr("Close all windows and quit the application.")
    << Trigger(QCoreApplication::instance(), SLOT(closeAllWindows()));

  actionSave = makeAction(tr("Save &as...", "File|"))
    << QKeySequence(tr("Ctrl+S", "File|SaveAs"))
    << QIcon(":/images/icon_save.png")
    << tr("Save the DjVu document.")
    << Trigger(this, SLOT(saveAs()));
  
  actionExport = makeAction(tr("&Export as...", "File|"))
    << QKeySequence(tr("Ctrl+E", "File|ExportAs"))
    << QIcon(":/images/icon_save.png")
    << tr("Export DjVu page or document to other formats.")
    << Trigger(this, SLOT(exportAs()));

  actionPrint = makeAction(tr("&Print...", "File|"))
    << QKeySequence(tr("Ctrl+P", "File|Print"))
    << QIcon(":/images/icon_print.png")
    << tr("Print the DjVu document.")
    << Trigger(this, SLOT(print()));

  actionFind = makeAction(tr("&Find...", "Edit|"))
    << QKeySequence(tr("Ctrl+F", "Edit|Find"))
    << QIcon(":/images/icon_find.png")
    << tr("Find text in the document.")
    << Trigger(this, SLOT(performFind()));

  actionFindNext = makeAction(tr("Find &Next", "Edit|"))
    << QKeySequence(tr("Ctrl+F3", "Edit|Find Next"))
    << QKeySequence(tr("F3", "Edit|Find Next"))
    << tr("Find next occurence of search text in the document.")
    << Trigger(findWidget, SLOT(findNext()));

  actionFindPrev = makeAction(tr("Find &Previous", "Edit|"))
    << QKeySequence(tr("Shift+F3", "Edit|Find Previous"))
    << tr("Find previous occurence of search text in the document.")
    << Trigger(findWidget, SLOT(findPrev()));

  actionSelect = makeAction(tr("&Select", "Edit|"), false)
    << QKeySequence(tr("Ctrl+F2", "Edit|Select"))
    << QKeySequence(tr("F2", "Edit|Select"))
    << QIcon(":/images/icon_select.png")
    << tr("Select a rectangle in the document.")
    << Trigger(this, SLOT(performSelect(bool)));
  
  actionZoomIn = makeAction(tr("Zoom &In", "Zoom|"))
    << QIcon(":/images/icon_zoomin.png")
    << tr("Increase the magnification.")
    << Trigger(widget, SLOT(zoomIn()));

  actionZoomOut = makeAction(tr("Zoom &Out", "Zoom|"))
    << QIcon(":/images/icon_zoomout.png")
    << tr("Decrease the magnification.")
    << Trigger(widget, SLOT(zoomOut()));

  actionZoomFitWidth = makeAction(tr("Fit &Width", "Zoom|"),false)
    << tr("Set magnification to fit page width.")
    << QVariant(QDjVuWidget::ZOOM_FITWIDTH)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoomFitPage = makeAction(tr("Fit &Page", "Zoom|"),false)
    << tr("Set magnification to fit page.")
    << QVariant(QDjVuWidget::ZOOM_FITPAGE)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoomOneToOne = makeAction(tr("One &to one", "Zoom|"),false)
    << tr("Set full resolution magnification.")
    << QVariant(QDjVuWidget::ZOOM_ONE2ONE)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom300 = makeAction(tr("&300%", "Zoom|"), false)
    << tr("Magnify 300%")
    << QVariant(300)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom200 = makeAction(tr("&200%", "Zoom|"), false)
    << tr("Magnify 20%")
    << QVariant(200)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom150 = makeAction(tr("150%", "Zoom|"), false)
    << tr("Magnify 150%")
    << QVariant(200)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom100 = makeAction(tr("&100%", "Zoom|"), false)
    << tr("Magnify 100%")
    << QVariant(100)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom75 = makeAction(tr("&75%", "Zoom|"), false)
    << tr("Magnify 75%")
    << QVariant(75)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionZoom50 = makeAction(tr("&50%", "Zoom|"), false)
    << tr("Magnify 50%")
    << QVariant(50)
    << Trigger(this, SLOT(performZoom()))
    << *zoomActionGroup;

  actionNavFirst = makeAction(tr("&First Page", "Go|"))
    << QIcon(":/images/icon_first.png")
    << tr("Jump to first document page.")
    << Trigger(widget, SLOT(firstPage()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionNavNext = makeAction(tr("&Next Page", "Go|"))
    << QIcon(":/images/icon_next.png")
    << tr("Jump to next document page.")
    << Trigger(widget, SLOT(nextPage()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionNavPrev = makeAction(tr("&Previous Page", "Go|"))
    << QIcon(":/images/icon_prev.png")
    << tr("Jump to previous document page.")
    << Trigger(widget, SLOT(prevPage()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionNavLast = makeAction(tr("&Last Page", "Go|"))
    << QIcon(":/images/icon_last.png")
    << tr("Jump to last document page.")
    << Trigger(widget, SLOT(lastPage()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionBack = makeAction(tr("&Backward", "Go|"))
    << QIcon(":/images/icon_back.png")
    << tr("Backward in history.")
    << Trigger(this, SLOT(performUndo()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionForw = makeAction(tr("&Forward", "Go|"))
    << QIcon(":/images/icon_forw.png")
    << tr("Forward in history.")
    << Trigger(this, SLOT(performRedo()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionRotateLeft = makeAction(tr("Rotate &Left", "Rotate|"))
    << QIcon(":/images/icon_rotateleft.png")
    << tr("Rotate page image counter-clockwise.")
    << Trigger(widget, SLOT(rotateLeft()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionRotateRight = makeAction(tr("Rotate &Right", "Rotate|"))
    << QIcon(":/images/icon_rotateright.png")
    << tr("Rotate page image clockwise.")
    << Trigger(widget, SLOT(rotateRight()))
    << Trigger(this, SLOT(updateActionsLater()));

  actionRotate0 = makeAction(tr("Rotate &0\260", "Rotate|"), false)
    << tr("Set natural page orientation.")
    << QVariant(0)
    << Trigger(this, SLOT(performRotation()))
    << *rotationActionGroup;

  actionRotate90 = makeAction(tr("Rotate &90\260", "Rotate|"), false)
    << tr("Turn page on its left side.")
    << QVariant(1)
    << Trigger(this, SLOT(performRotation()))
    << *rotationActionGroup;

  actionRotate180 = makeAction(tr("Rotate &180\260", "Rotate|"), false)
    << tr("Turn page upside-down.")
    << QVariant(2)
    << Trigger(this, SLOT(performRotation()))
    << *rotationActionGroup;

  actionRotate270 = makeAction(tr("Rotate &270\260", "Rotate|"), false)
    << tr("Turn page on its right side.")
    << QVariant(3)
    << Trigger(this, SLOT(performRotation()))
    << *rotationActionGroup;
  
  actionInformation = makeAction(tr("&Information...", "Edit|"))
    << QKeySequence(tr("Ctrl+I", "Edit|Information"))
    << tr("Show information about the document encoding and structure.")
    << Trigger(this, SLOT(performInformation()));

  actionMetadata = makeAction(tr("&Metadata...", "Edit|"))
    << QKeySequence(tr("Ctrl+M", "Edit|Metadata"))
    << tr("Show the document and page meta data.")
    << Trigger(this, SLOT(performMetadata()));

  actionWhatsThis = QWhatsThis::createAction(this);

  actionAbout = makeAction(tr("&About DjView..."))
#ifndef Q_WS_MAC
    << QIcon(":/images/djview.png")
#endif
    << tr("Show information about this program.")
    << Trigger(this, SLOT(performAbout()));

  actionDisplayColor = makeAction(tr("&Color", "Display|"), false)
    << tr("Display everything.")
    << Trigger(widget, SLOT(displayModeColor()))
    << Trigger(this, SLOT(updateActionsLater()))
    << *modeActionGroup;

  actionDisplayBW = makeAction(tr("&Stencil", "Display|"), false)
    << tr("Only display the document bitonal stencil.")
    << Trigger(widget, SLOT(displayModeStencil()))
    << Trigger(this, SLOT(updateActionsLater()))
    << *modeActionGroup;

  actionDisplayForeground 
    = makeAction(tr("&Foreground", "Display|"), false)
    << tr("Only display the foreground layer.")
    << Trigger(widget, SLOT(displayModeForeground()))
    << Trigger(this, SLOT(updateActionsLater()))
    << *modeActionGroup;

  actionDisplayBackground
    = makeAction(tr("&Background", "Display|"), false)
    << tr("Only display the background layer.")
    << Trigger(widget, SLOT(displayModeBackground()))
    << Trigger(this, SLOT(updateActionsLater()))
    << *modeActionGroup;
  
  actionDisplayHiddenText
    = makeAction(tr("&Hidden Text", "Display|"), false)
    << tr("Overlay a representation of the hidden text layer.")
    << Trigger(widget, SLOT(displayModeText()))
    << Trigger(this, SLOT(updateActionsLater()))
    << *modeActionGroup;
  
  actionPreferences = makeAction(tr("Prefere&nces...", "Settings|")) 
    << QIcon(":/images/icon_prefs.png")
    << tr("Show the preferences dialog.")
    << Trigger(this, SLOT(performPreferences()));

  actionViewSideBar = makeAction(tr("Show &Sidebar", "Settings|"), true)
    << QKeySequence(tr("Ctrl+F9", "Settings|Show sidebar"))
    << QKeySequence(tr("F9", "Settings|Show sidebar"))
    << tr("Show/hide the side bar.")
    << Trigger(this, SLOT(showSideBar(bool)));

  actionViewToolBar = toolBar->toggleViewAction()
    << tr("Show &Toolbar", "Settings|")
    << QKeySequence(tr("Ctrl+F10", "Settings|Show toolbar"))
    << QKeySequence(tr("F10", "Settings|Show toolbar"))
    << tr("Show/hide the standard tool bar.")
    << Trigger(this, SLOT(updateActionsLater()));

  actionViewStatusBar = makeAction(tr("Show Stat&usbar", "Settings|"), true)
    << tr("Show/hide the status bar.")
    << Trigger(statusBar,SLOT(setVisible(bool)))
    << Trigger(this, SLOT(updateActionsLater()));

  actionViewFullScreen 
    = makeAction(tr("F&ull Screen","View|"), false)
    << QKeySequence(tr("Ctrl+F11","View|FullScreen"))
    << QKeySequence(tr("F11","View|FullScreen"))
    << QIcon(":/images/icon_fullscreen.png")
    << tr("Toggle full screen mode.")
    << Trigger(this, SLOT(performViewFullScreen(bool)));

  actionLayoutContinuous = makeAction(tr("&Continuous", "Layout|"), false)
    << QIcon(":/images/icon_continuous.png")
    << QKeySequence(tr("Ctrl+F4", "Layout|Continuous"))
    << QKeySequence(tr("F4", "Layout|Continuous"))
    << tr("Toggle continuous layout mode.")
    << Trigger(widget, SLOT(setContinuous(bool)))
    << Trigger(this, SLOT(updateActionsLater()));

  actionLayoutSideBySide = makeAction(tr("Side &by Side", "Layout|"), false)
    << QIcon(":/images/icon_sidebyside.png")
    << QKeySequence(tr("Ctrl+F5", "Layout|SideBySide"))
    << QKeySequence(tr("F5", "Layout|SideBySide"))
    << tr("Toggle side-by-side layout mode.")
    << Trigger(widget, SLOT(setSideBySide(bool)))
    << Trigger(this, SLOT(updateActionsLater()));
  
  actionLayoutCoverPage = makeAction(tr("Co&ver Page", "Layout|"), false)
#ifdef Q_WS_MAC
    << QIcon(":/images/icon_coverpage.png")
#endif
    << QKeySequence(tr("Ctrl+F6", "Layout|CoverPage"))
    << QKeySequence(tr("F6", "Layout|CoverPage"))
    << tr("Show the cover page alone in side-by-side mode.")
    << Trigger(widget, SLOT(setCoverPage(bool)))
    << Trigger(this, SLOT(updateActionsLater()));
  
  actionLayoutRightToLeft = makeAction(tr("&Right to Left", "Layout|"), false)
#ifdef Q_WS_MAC
    << QIcon(":/images/icon_righttoleft.png")
#endif
    << QKeySequence(tr("Ctrl+Shift+F6", "Layout|RightToLeft"))
    << QKeySequence(tr("Shift+F6", "Layout|RightToLeft"))
    << tr("Show pages right-to-left in side-by-side mode.")
    << Trigger(widget, SLOT(setRightToLeft(bool)))
    << Trigger(this, SLOT(updateActionsLater()));
  
  actionCopyUrl = makeAction(tr("Copy &URL", "Edit|"))
    << tr("Save an URL for the current page into the clipboard.")
    << QKeySequence(tr("Ctrl+C", "Edit|CopyURL"))
    << Trigger(this, SLOT(performCopyUrl()));
  
  actionCopyOutline = makeAction(tr("Copy &Outline", "Edit|"))
    << tr("Save the djvused code for the outline into the clipboard.")
    << Trigger(this, SLOT(performCopyOutline()));
  
  actionCopyAnnotation = makeAction(tr("Copy &Annotations", "Edit|"))
    << tr("Save the djvused code for the page annotations into the clipboard.")
    << Trigger(this, SLOT(performCopyAnnotation()));
  
  actionAbout->setMenuRole(QAction::AboutRole);
  actionQuit->setMenuRole(QAction::QuitRole);
  actionPreferences->setMenuRole(QAction::PreferencesRole);

  // Enumerate all actions
  QAction *a;
  QObject *o;
  allActions.clear();
  foreach(o, children())
    if ((a = qobject_cast<QAction*>(o)))
      allActions << a;
}


void
QDjView::createMenus()
{
  // Layout main menu
  QMenu *fileMenu = menuBar->addMenu(tr("&File", "File|"));
  if (viewerMode == STANDALONE)
    {
      fileMenu->addAction(actionNew);
      fileMenu->addAction(actionOpen);
      fileMenu->addAction(actionOpenLocation);
      recentMenu = fileMenu->addMenu(tr("Open &Recent"));
      recentMenu->menuAction()->setIcon(QIcon(":/images/icon_open.png"));
      connect(recentMenu, SIGNAL(aboutToShow()), this, SLOT(fillRecent()));
      fileMenu->addSeparator();
    }
  fileMenu->addAction(actionSave);
  fileMenu->addAction(actionExport);
  fileMenu->addAction(actionPrint);
  if (viewerMode == STANDALONE)
    {
      fileMenu->addSeparator();
      fileMenu->addAction(actionClose);
      fileMenu->addAction(actionQuit);
    }
  QMenu *editMenu = menuBar->addMenu(tr("&Edit", "Edit|"));
  editMenu->addAction(actionSelect);
  editMenu->addAction(actionCopyUrl);
  editMenu->addAction(actionCopyOutline);
  editMenu->addAction(actionCopyAnnotation);
  editMenu->addSeparator();
  editMenu->addAction(actionFind);
  editMenu->addAction(actionFindNext);
  editMenu->addAction(actionFindPrev);
  QMenu *viewMenu = menuBar->addMenu(tr("&View", "View|"));
  QMenu *zoomMenu = viewMenu->addMenu(tr("&Zoom","View|Zoom"));
  zoomMenu->addAction(actionZoomIn);
  zoomMenu->addAction(actionZoomOut);
  zoomMenu->addSeparator();
  zoomMenu->addAction(actionZoomOneToOne);
  zoomMenu->addAction(actionZoomFitWidth);
  zoomMenu->addAction(actionZoomFitPage);
  zoomMenu->addSeparator();
  zoomMenu->addAction(actionZoom300);
  zoomMenu->addAction(actionZoom200);
  zoomMenu->addAction(actionZoom150);
  zoomMenu->addAction(actionZoom100);
  zoomMenu->addAction(actionZoom75);
  zoomMenu->addAction(actionZoom50);
  QMenu *rotationMenu = viewMenu->addMenu(tr("&Rotate","View|Rotate"));
  rotationMenu->addAction(actionRotateLeft);
  rotationMenu->addAction(actionRotateRight);
  rotationMenu->addSeparator();
  rotationMenu->addAction(actionRotate0);
  rotationMenu->addAction(actionRotate90);
  rotationMenu->addAction(actionRotate180);
  rotationMenu->addAction(actionRotate270);
  QMenu *modeMenu = viewMenu->addMenu(tr("&Display","View|Display"));
  modeMenu->addAction(actionDisplayColor);
  modeMenu->addAction(actionDisplayBW);
  modeMenu->addAction(actionDisplayForeground);
  modeMenu->addAction(actionDisplayBackground);
  modeMenu->addAction(actionDisplayHiddenText);
  viewMenu->addSeparator();
  viewMenu->addAction(actionLayoutContinuous);
  viewMenu->addAction(actionLayoutSideBySide);
  viewMenu->addAction(actionLayoutCoverPage);
  viewMenu->addAction(actionLayoutRightToLeft);
  viewMenu->addSeparator();
  viewMenu->addAction(actionInformation);
  viewMenu->addAction(actionMetadata);
  if (viewerMode == STANDALONE)
    viewMenu->addSeparator();
  if (viewerMode == STANDALONE)
    viewMenu->addAction(actionViewFullScreen);
  QMenu *gotoMenu = menuBar->addMenu(tr("&Go", "Go|"));
  gotoMenu->addAction(actionNavFirst);
  gotoMenu->addAction(actionNavPrev);
  gotoMenu->addAction(actionNavNext);
  gotoMenu->addAction(actionNavLast);
  gotoMenu->addSeparator();
  gotoMenu->addAction(actionBack);
  gotoMenu->addAction(actionForw);
  QMenu *settingsMenu = menuBar->addMenu(tr("&Settings", "Settings|"));
  settingsMenu->addAction(actionViewSideBar);
  settingsMenu->addAction(actionViewToolBar);
  settingsMenu->addAction(actionViewStatusBar);
  settingsMenu->addSeparator();
  settingsMenu->addAction(actionPreferences);
  QMenu *helpMenu = menuBar->addMenu(tr("&Help", "Help|"));
  helpMenu->addAction(actionWhatsThis);
  helpMenu->addSeparator();
  helpMenu->addAction(actionAbout);

  // Layout context menu
  gotoMenu = contextMenu->addMenu(tr("&Go", "Go|"));
  gotoMenu->addAction(actionNavFirst);
  gotoMenu->addAction(actionNavPrev);
  gotoMenu->addAction(actionNavNext);
  gotoMenu->addAction(actionNavLast);
  zoomMenu = contextMenu->addMenu(tr("&Zoom","View|Zoom"));
  zoomMenu->addAction(actionZoomIn);
  zoomMenu->addAction(actionZoomOut);
  zoomMenu->addSeparator();
  zoomMenu->addAction(actionZoomOneToOne);
  zoomMenu->addAction(actionZoomFitWidth);
  zoomMenu->addAction(actionZoomFitPage);
  zoomMenu->addSeparator();
  zoomMenu->addAction(actionZoom300);
  zoomMenu->addAction(actionZoom200);
  zoomMenu->addAction(actionZoom150);
  zoomMenu->addAction(actionZoom100);
  zoomMenu->addAction(actionZoom75);
  zoomMenu->addAction(actionZoom50);
  rotationMenu = contextMenu->addMenu(tr("&Rotate","View|Rotate"));
  rotationMenu->addAction(actionRotateLeft);
  rotationMenu->addAction(actionRotateRight);
  rotationMenu->addSeparator();
  rotationMenu->addAction(actionRotate0);
  rotationMenu->addAction(actionRotate90);
  rotationMenu->addAction(actionRotate180);
  rotationMenu->addAction(actionRotate270);
  modeMenu = contextMenu->addMenu(tr("&Display","View|Display"));
  modeMenu->addAction(actionDisplayColor);
  modeMenu->addAction(actionDisplayBW);
  modeMenu->addAction(actionDisplayForeground);
  modeMenu->addAction(actionDisplayBackground);
  modeMenu->addAction(actionDisplayHiddenText);
  contextMenu->addSeparator();
  contextMenu->addAction(actionLayoutContinuous);
  contextMenu->addAction(actionLayoutSideBySide);
  contextMenu->addAction(actionLayoutCoverPage);
  contextMenu->addSeparator();
  contextMenu->addAction(actionFind);
  contextMenu->addAction(actionInformation);
  contextMenu->addAction(actionMetadata);
  contextMenu->addSeparator();
  contextMenu->addAction(actionSave);
  contextMenu->addAction(actionExport);
  contextMenu->addAction(actionPrint);
  contextMenu->addSeparator();
  contextMenu->addAction(actionViewSideBar);
  contextMenu->addAction(actionViewToolBar);
  contextMenu->addAction(actionViewFullScreen);
  contextMenu->addSeparator();
  contextMenu->addAction(actionPreferences);
  contextMenu->addSeparator();
  contextMenu->addAction(actionWhatsThis);
  contextMenu->addAction(actionAbout);
}


/*! Update graphical interface components.
  This is called whenever something changes
  using function \a QDjView::updateActionsLater().
  It updates the all gui compnents. */

void
QDjView::updateActions()
{
  // Rebuild toolbar if necessary
  if (tools != toolsCached)
    fillToolBar(toolBar);
  
  // Enable all actions
  foreach(QAction *action, allActions)
    action->setEnabled(true);

  // Some actions are explicitly disabled
  actionSave->setEnabled(savingAllowed);
  actionExport->setEnabled(savingAllowed);
  actionPrint->setEnabled(printingAllowed);
  
  // Some actions are only available in standalone mode
  actionNew->setVisible(viewerMode == STANDALONE);
  actionOpen->setVisible(viewerMode == STANDALONE);
  actionOpenLocation->setVisible(viewerMode == STANDALONE);
  actionClose->setVisible(viewerMode == STANDALONE);
  actionQuit->setVisible(viewerMode == STANDALONE);
  actionViewFullScreen->setVisible(viewerMode == STANDALONE);  
  setAcceptDrops(viewerMode == STANDALONE);

  // Zoom combo and actions
  int zoom = widget->zoom();
  actionZoomIn->setEnabled(zoom < QDjVuWidget::ZOOM_MAX);
  actionZoomOut->setEnabled(zoom < 0 || zoom > QDjVuWidget::ZOOM_MIN);
  actionZoomFitPage->setChecked(zoom == QDjVuWidget::ZOOM_FITPAGE);
  actionZoomFitWidth->setChecked(zoom == QDjVuWidget::ZOOM_FITWIDTH);
  actionZoomOneToOne->setChecked(zoom == QDjVuWidget::ZOOM_ONE2ONE);
  actionZoom300->setChecked(zoom == 300);
  actionZoom200->setChecked(zoom == 200);
  actionZoom150->setChecked(zoom == 150);
  actionZoom100->setChecked(zoom == 100);
  actionZoom75->setChecked(zoom == 75);
  actionZoom50->setChecked(zoom == 50);
  zoomCombo->setEnabled(!!document);
  int zoomIndex = zoomCombo->findData(QVariant(zoom));
  zoomCombo->clearEditText();
  zoomCombo->setCurrentIndex(zoomIndex);
  if (zoomIndex < 0 &&
      zoom >= QDjVuWidget::ZOOM_MIN && 
      zoom <= QDjVuWidget::ZOOM_MAX)
    zoomCombo->setEditText(QString("%1%").arg(zoom));
  else if (zoomIndex >= 0)
    zoomCombo->setEditText(zoomCombo->itemText(zoomIndex));

  // Mode combo and actions
  QDjVuWidget::DisplayMode mode = widget->displayMode();
  actionDisplayColor->setChecked(mode == QDjVuWidget::DISPLAY_COLOR);
  actionDisplayBW->setChecked(mode == QDjVuWidget::DISPLAY_STENCIL);
  actionDisplayBackground->setChecked(mode == QDjVuWidget::DISPLAY_BG);
  actionDisplayForeground->setChecked(mode == QDjVuWidget::DISPLAY_FG);
  actionDisplayHiddenText->setChecked(mode == QDjVuWidget::DISPLAY_TEXT);
  modeCombo->setCurrentIndex(modeCombo->findData(QVariant(mode)));
  modeCombo->setEnabled(!!document);

  // Rotations
  int rotation = widget->rotation();
  actionRotate0->setChecked(rotation == 0);
  actionRotate90->setChecked(rotation == 1);
  actionRotate180->setChecked(rotation == 2);
  actionRotate270->setChecked(rotation == 3);
  
  // Page combo and actions
  int pagenum = documentPages.size();
  int pageno = widget->page();
  pageCombo->clearEditText();
  pageCombo->setCurrentIndex(pageCombo->findData(QVariant(pageno)));
  if (pageno >= 0 && pagenum > 0)
    pageCombo->setEditText(pageName(pageno));
  pageCombo->setEnabled(pagenum > 0);
  actionNavFirst->setEnabled(pagenum>0 && pageno>0);
  actionNavPrev->setEnabled(pagenum>0 && pageno>0);
  actionNavNext->setEnabled(pagenum>0 && pageno<pagenum-1);
  actionNavLast->setEnabled(pagenum>0 && pageno<pagenum-1);

  // Layout actions
  actionLayoutContinuous->setChecked(widget->continuous());  
  actionLayoutSideBySide->setChecked(widget->sideBySide());
  actionLayoutCoverPage->setEnabled(widget->sideBySide());
  actionLayoutCoverPage->setChecked(widget->coverPage());  
  actionLayoutRightToLeft->setEnabled(widget->sideBySide());
  actionLayoutRightToLeft->setChecked(widget->rightToLeft());  
  
  // UndoRedo
  undoTimer->stop();
  undoTimer->start(250);
  actionBack->setEnabled(undoList.size() > 0);
  actionForw->setEnabled(redoList.size() > 0);

  // Misc
  textLabel->setVisible(prefs->showTextLabel);
  actionCopyOutline->setVisible(prefs->advancedFeatures);
  actionCopyAnnotation->setVisible(prefs->advancedFeatures);
  actionCopyUrl->setEnabled(pagenum > 0);
  actionCopyOutline->setEnabled(pagenum > 0);
  actionCopyAnnotation->setEnabled(pagenum > 0);
  actionViewSideBar->setChecked(hiddenSideBarTabs() == 0);
  
  // Disable almost everything when document==0
  if (! document)
    {
      foreach(QAction *action, allActions)
        if (action != actionNew &&
            action != actionOpen &&
            action != actionOpenLocation &&
            action != actionClose &&
            action != actionQuit &&
            action != actionPreferences &&
            action != actionViewToolBar &&
            action != actionViewStatusBar &&
            action != actionViewSideBar &&
            action != actionViewFullScreen &&
            action != actionWhatsThis &&
            action != actionAbout )
          action->setEnabled(false);
    }
  
  // Finished
  updateActionsScheduled = false;

  // Notify plugin
  if (viewerMode != STANDALONE)  
    emit pluginOnChange();
}



// ----------------------------------------
// WHATSTHIS HELP


void
QDjView::createWhatsThis()
{
#define HELP(x,y) { QString s(x); Help h(s); h y; }
  
  struct Help {
    QString s;
    Help(QString s) : s(s) { }
    Help& operator>>(QWidget *w) { w->setWhatsThis(s); return *this; }
    Help& operator>>(QAction *a) { a->setWhatsThis(s); return *this; }
  };
  
  QString mc, ms, ml;
#ifdef Q_WS_MAC
  mc = tr("Control Left Mouse Button");
#else
  mc = tr("Right Mouse Button");
#endif
  ms = prefs->modifiersToString(prefs->modifiersForSelect);
  ms = ms.replace("+"," ");
  ml = prefs->modifiersToString(prefs->modifiersForLens);
  ml = ml.replace("+"," ");
  
  HELP(tr("<html><b>Selecting a rectangle.</b><br/> "
          "Once a rectangular area is selected, a popup menu "
          "lets you copy the corresponding text or image. "
          "Instead of using this tool, you can also hold %1 "
          "and use the Left Mouse Button."
          "</html>").arg(ms),
       >> actionSelect );
  
  HELP(tr("<html><b>Zooming.</b><br/> "
          "Choose a zoom level for viewing the document. "
          "Zoom level 100% displays the document for a 100 dpi screen. "
          "Zoom levels <tt>Fit Page</tt> and <tt>Fit Width</tt> ensure "
          "that the full page or the page width fit in the window. "
          "</html>"),
       >> actionZoomIn >> actionZoomOut
       >> actionZoomFitPage >> actionZoomFitWidth
       >> actionZoom300 >> actionZoom200 >> actionZoom150
       >> actionZoom75 >> actionZoom50
       >> zoomCombo );
  
  HELP(tr("<html><b>Rotating the pages.</b><br/> "
          "Choose to display pages in portrait or landscape mode. "
          "You can also turn them upside down.</html>"),
       >> actionRotateLeft >> actionRotateRight
       >> actionRotate0 >> actionRotate90
       >> actionRotate180 >> actionRotate270 );

  HELP(tr("<html><b>Display mode.</b><br/> "
          "DjVu images compose a background layer and a foreground layer "
          "using a stencil. The display mode specifies with layers "
          "should be displayed.</html>"),
            >> actionDisplayColor >> actionDisplayBW
            >> actionDisplayForeground >> actionDisplayBackground
            >> modeCombo );

  HELP(tr("<html><b>Navigating the document.</b><br/> "
          "The page selector lets you jump to any page by name. "
          "The navigation buttons jump to the first page, the previous "
          "page, the next page, or the last page. </html>"),
       >> actionNavFirst >> actionNavPrev 
       >> actionNavNext >> actionNavLast
       >> pageCombo );

  HELP(tr("<html><b>Document and page information.</b><br> "
          "Display a dialog window for viewing "
          "encoding information pertaining to the document "
          "or to a specific page.</html>"),
       >> actionInformation );
  
  HELP(tr("<html><b>Document and page metadata.</b><br> "
          "Display a dialog window for viewing metadata "
          "pertaining to the document "
          "or to a specific page.</html>"),
       >> actionMetadata );

  HELP(tr("<html><b>Continuous layout.</b><br/> "
          "Display all the document pages arranged vertically "
          "inside the scrollable document viewing area.</html>"),
       >> actionLayoutContinuous );
  
  HELP(tr("<html><b>Side by side layout.</b><br/> "
          "Display pairs of pages side by side "
          "inside the scrollable document viewing area.</html>"),
       >> actionLayoutSideBySide );
  
  HELP(tr("<html><b>Page information.</b><br/> "
          "Display information about the page located under the cursor: "
          "the sequential page number, the page size in pixels, "
          "and the page resolution in dots per inch. </html>"),
       >> pageLabel );
  
  HELP(tr("<html><b>Cursor information.</b><br/> "
          "Display the position of the mouse cursor "
          "expressed in page coordinates. </html>"),
       >> mouseLabel );

  HELP(tr("<html><b>Document viewing area.</b><br/> "
          "This is the main display area for the DjVu document. <ul>"
          "<li>Arrows and page keys to navigate the document.</li>"
          "<li>Space and BackSpace to read the document.</li>"
          "<li>Keys <tt>+</tt> <tt>-</tt> <tt>[</tt> <tt>]</tt> "
          "to zoom or rotate the document.</li>"
          "<li>Left Mouse Button for panning and selecting links.</li>"
          "<li>%3 for displaying the contextual menu.</li>"
          "<li>%1 Left Mouse Button for selecting text or images.</li>"
          "<li>%2 for popping the magnification lens.</li>"
          "</ul></html>").arg(ms).arg(ml).arg(mc),
       >> widget );
  
  HELP(tr("<html><b>Document viewing area.</b><br/> "
          "This is the main display area for the DjVu document. "
          "But you must first open a DjVu document to see anything."
          "</html>"),
       >> splash );

#undef HELP    
}



// ----------------------------------------
// APPLY PREFERENCES



QDjViewPrefs::Saved *
QDjView::getSavedPrefs(void)
{
  if (viewerMode==EMBEDDED_PLUGIN)
    return &prefs->forEmbeddedPlugin;
  if (viewerMode==FULLPAGE_PLUGIN)
    return &prefs->forFullPagePlugin;
  if (actionViewFullScreen->isChecked())
    return &prefs->forFullScreen;
  else
    return &prefs->forStandalone;
}


void 
QDjView::enableContextMenu(bool enable)
{
  QMenu *oldContextMenu = widget->contextMenu();
  if (!enable || oldContextMenu != contextMenu)
    {
      // remove context menu
      widget->setContextMenu(0);
      // disable shortcuts
      if (oldContextMenu)
        foreach(QAction *action, widget->actions())
          widget->removeAction(action);
    }
  if (enable && oldContextMenu != contextMenu)
    {
      // setup context menu
      widget->setContextMenu(contextMenu);
      // enable shortcuts
      if (contextMenu)
        {
          // all explicit shortcuts in context menu
          widget->addAction(contextMenu->menuAction());
          // things that do not have a context menu entry
          widget->addAction(actionFindNext);
          widget->addAction(actionFindPrev);
        }
    }
}


void 
QDjView::enableScrollBars(bool enable)
{
  Qt::ScrollBarPolicy policy = Qt::ScrollBarAlwaysOff;
  if (enable) 
    policy = Qt::ScrollBarAsNeeded;
  widget->setHorizontalScrollBarPolicy(policy);
  widget->setVerticalScrollBarPolicy(policy);
}


void 
QDjView::applyOptions(bool remember)
{
  menuBar->setVisible(options & QDjViewPrefs::SHOW_MENUBAR);
  toolBar->setVisible(options & QDjViewPrefs::SHOW_TOOLBAR);
  statusBar->setVisible(options & QDjViewPrefs::SHOW_STATUSBAR);
  enableScrollBars(options & QDjViewPrefs::SHOW_SCROLLBARS);
  widget->setDisplayFrame(options & QDjViewPrefs::SHOW_FRAME);
  widget->setDisplayMapAreas(options & QDjViewPrefs::SHOW_MAPAREAS);
  widget->setContinuous(options & QDjViewPrefs::LAYOUT_CONTINUOUS);
  widget->setSideBySide(options & QDjViewPrefs::LAYOUT_SIDEBYSIDE);
  widget->setCoverPage(options & QDjViewPrefs::LAYOUT_COVERPAGE);
  widget->setRightToLeft(options & QDjViewPrefs::LAYOUT_RIGHTTOLEFT);
  widget->enableMouse(options & QDjViewPrefs::HANDLE_MOUSE);
  widget->enableKeyboard(options & QDjViewPrefs::HANDLE_KEYBOARD);
  widget->enableHyperlink(options & QDjViewPrefs::HANDLE_LINKS);
  enableContextMenu(options & QDjViewPrefs::HANDLE_CONTEXTMENU);
  if (!remember)
    showSideBar((bool)(options & QDjViewPrefs::SHOW_SIDEBAR));
}


void 
QDjView::updateOptions(void)
{
  options = 0;
  if (! menuBar->isHidden())
    options |= QDjViewPrefs::SHOW_MENUBAR;
  if (! toolBar->isHidden())
    options |= QDjViewPrefs::SHOW_TOOLBAR;
  if (! statusBar->isHidden())
    options |= QDjViewPrefs::SHOW_STATUSBAR;
  if (! hiddenSideBarTabs())
    options |= QDjViewPrefs::SHOW_SIDEBAR;    
  if (widget->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff)
    options |= QDjViewPrefs::SHOW_SCROLLBARS;
  if (widget->displayFrame())
    options |= QDjViewPrefs::SHOW_FRAME;
  if (widget->displayMapAreas())
    options |= QDjViewPrefs::SHOW_MAPAREAS;
  if (widget->continuous())
    options |= QDjViewPrefs::LAYOUT_CONTINUOUS;
  if (widget->sideBySide())
    options |= QDjViewPrefs::LAYOUT_SIDEBYSIDE;
  if (widget->coverPage())
    options |= QDjViewPrefs::LAYOUT_COVERPAGE;
  if (widget->rightToLeft())
    options |= QDjViewPrefs::LAYOUT_RIGHTTOLEFT;
  if (widget->mouseEnabled())
    options |= QDjViewPrefs::HANDLE_MOUSE;    
  if (widget->keyboardEnabled())
    options |= QDjViewPrefs::HANDLE_KEYBOARD;
  if (widget->hyperlinkEnabled())
    options |= QDjViewPrefs::HANDLE_LINKS;
  if (widget->contextMenu())
    options |= QDjViewPrefs::HANDLE_CONTEXTMENU;
}



void
QDjView::applySaved(Saved *saved)
{
  // default dock geometry
  tabifyDockWidget(thumbnailDock, outlineDock);
  tabifyDockWidget(thumbnailDock, findDock);
  thumbnailDock->raise();
  // main saved states
  options = saved->options;
  if (saved->state.size() > 0)
    restoreState(saved->state);
  applyOptions(saved->remember);
  widget->setZoom(saved->zoom);
  // global window size in standalone mode
  if (saved == &prefs->forStandalone)
    if (! (prefs->windowSize.isNull()))
      resize(prefs->windowSize);
}


static const Qt::WindowStates 
unusualWindowStates = (Qt::WindowMinimized |
                       Qt::WindowMaximized |
                       Qt::WindowFullScreen );


void
QDjView::updateSaved(Saved *saved)
{
  updateOptions();
  if (saved->remember)
    {
      // main saved states
      saved->zoom = widget->zoom();
      saved->state = saveState();
      saved->options = options;
      // avoid confusing options in standalone mode
      if (saved == &prefs->forStandalone &&
          !(saved->options & QDjViewPrefs::HANDLE_CONTEXTMENU) &&
          !(saved->options & QDjViewPrefs::SHOW_MENUBAR) )
        saved->options |= (QDjViewPrefs::SHOW_MENUBAR |
                           QDjViewPrefs::SHOW_SCROLLBARS |
                           QDjViewPrefs::SHOW_STATUSBAR |
                           QDjViewPrefs::SHOW_FRAME |
                           QDjViewPrefs::HANDLE_CONTEXTMENU );
      // main window size in standalone mode
      if (saved == &prefs->forStandalone)
        if (! (windowState() & unusualWindowStates))
          prefs->windowSize = size();
    }
}


void
QDjView::applyPreferences(void)
{
  // Saved preferences
  applySaved(getSavedPrefs());
  
  // Other preferences
  djvuContext.setCacheSize(prefs->cacheSize);
  widget->setPixelCacheSize(prefs->pixelCacheSize);
  widget->setModifiersForLens(prefs->modifiersForLens);
  widget->setModifiersForSelect(prefs->modifiersForSelect);
  widget->setModifiersForLinks(prefs->modifiersForLinks);
  widget->setGamma(prefs->gamma);
  widget->setScreenDpi(prefs->resolution ? prefs->resolution : physicalDpiY());
  widget->setLensSize(prefs->lensSize);
  widget->setLensPower(prefs->lensPower);

  // Thumbnail preferences
  thumbnailWidget->setSize(prefs->thumbnailSize);
  thumbnailWidget->setSmart(prefs->thumbnailSmart);
  
  // Search preferences
  findWidget->setWordOnly(prefs->searchWordsOnly);
  findWidget->setCaseSensitive(prefs->searchCaseSensitive);
  // Too risky:  findWidget->setRegExpMode(prefs->searchRegExpMode);

  // Special preferences for embedded plugins
  if (viewerMode == EMBEDDED_PLUGIN)
    {
      setMinimumSize(QSize(8,8));
      widget->setBorderBrush(QBrush(Qt::white));
      widget->setBorderSize(0);
    }
  
  // Preload full screen prefs.
  fsSavedNormal = prefs->forStandalone;
  fsSavedFullScreen = prefs->forFullScreen;
}


void 
QDjView::preferencesChanged(void)
{
  applyPreferences();
  createWhatsThis();
  updateActions();
}



// ----------------------------------------
// SESSION MANAGEMENT


void
QDjView::saveSession(QSettings *s)
{
  Saved saved;
  updateSaved(&saved);
  s->setValue("name", objectName());
  s->setValue("options", prefs->optionsToString(saved.options));
  s->setValue("state", saved.state);
  s->setValue("tools", prefs->toolsToString(tools));
  s->setValue("documentUrl", getDecoratedUrl().toString());
}

void  
QDjView::restoreSession(QSettings *s)
{
  QUrl du = QUrl(s->value("documentUrl").toString());
  Tools tools = this->tools;
  Saved saved;
  updateSaved(&saved);
  if (s->contains("options"))
    saved.options = prefs->stringToOptions(s->value("options").toString());
  if (s->contains("zoom")) // compat
    saved.zoom = s->value("zoom").toInt();
  if (s->contains("state"))
    saved.state = s->value("state").toByteArray();
  if (s->contains("tools"))
    tools = prefs->stringToTools(s->value("tools").toString());
  if (s->contains("name"))
    setObjectName(s->value("name").toString());
  applySaved(&saved);
  updateActionsLater();
  if (du.isValid())
    open(du);
  if (document && s->contains("pageNo")) // compat
    goToPage(s->value("pageNo", 0).toInt());
}




// ----------------------------------------
// QDJVIEW ARGUMENTS


static bool
string_is_on(QString val)
{
  QString v = val.toLower();
  return v == "yes" || v == "on" || v == "true" || v == "1";
}

static bool
string_is_off(QString val)
{
  QString v = val.toLower();
  return v == "no" || v == "off" || v == "false" || v == "0";
}

static QString
get_boolean(bool b)
{
  return QString(b ? "yes" : "no");
}

static bool
parse_boolean(QString key, QString val, 
              QList<QString> &errors, bool &answer)
{
  answer = false;
  if (string_is_off(val))
    return true;
  answer = true;
  if (string_is_on(val) || val.isNull())
    return true;
  errors << QDjView::tr("Option '%1' requires boolean argument.").arg(key);
  return false;
}

static void
illegal_value(QString key, QString value, QList<QString> &errors)
{
  errors << QDjView::tr("Illegal value '%2' for option '%1'.")
    .arg(key).arg(value);
}

static bool
parse_position(QString value, double &x, double &y)
{
  bool okay0, okay1;
  QStringList val = value.split(",");
  if (val.size() == 2)
    {
      x = val[0].toDouble(&okay0);
      y = val[1].toDouble(&okay1);
      return okay0 && okay1;
    }
  return false;
}

static bool 
parse_highlight(QString value, 
                int &x, int &y, int &w, int &h, 
                QColor &color)
{
  bool okay0, okay1, okay2, okay3;
  QStringList val = value.split(",");
  if (val.size() < 4)
    return false;
  x = val[0].toInt(&okay0);
  y = val[1].toInt(&okay1);
  w = val[2].toInt(&okay2);
  h = val[3].toInt(&okay3);
  if (! (okay0 && okay1 && okay2 && okay3))
    return false;
  if (val.size() < 5)
    return true;
  QString c = val[4];
  if (c[0] != '#')
    c = "#" + c;
  color.setNamedColor(c);
  if (! color.isValid())
    return false;
  if (val.size() <= 5)
    return true;
  return false;
}

template<class T> static inline void
set_reset(QFlags<T> &x, bool plus, bool minus, T y)
{
  if (plus)
    x |= y;
  else if (minus)
    x &= ~y;
}


void
QDjView::parseToolBarOption(QString option, QStringList &errors)
{
  QString str = option.toLower();
  int len = str.size();
  bool wantselect = false;
  bool wantmode = false;
  bool minus = false;
  bool plus = false;
  int npos = 0;
  while (npos < len)
    {
      int pos = npos;
      npos = str.indexOf(QRegExp("[-+,]"), pos);
      if (npos < 0) 
        npos = len;
      QString key = str.mid(pos, npos-pos).trimmed();
      if (string_is_off(key) && !plus && !minus)
        options &= ~QDjViewPrefs::SHOW_TOOLBAR;
      else if (string_is_on(key) && !plus && !minus)
        options |= QDjViewPrefs::SHOW_TOOLBAR;
      else if (key=="bottom" && !plus && !minus) {
        options |= QDjViewPrefs::SHOW_TOOLBAR;
        tools &= ~QDjViewPrefs::TOOLBAR_TOP;
      } else if (key=="top" && !plus && !minus) {
        options |= QDjViewPrefs::SHOW_TOOLBAR;
        tools &= ~QDjViewPrefs::TOOLBAR_BOTTOM;
      } else if (key=="auto" && !plus && !minus)
        tools |= QDjViewPrefs::TOOLBAR_AUTOHIDE;
      else if (key=="always" && !plus && !minus) {
        options |= QDjViewPrefs::SHOW_TOOLBAR;
        tools &= ~QDjViewPrefs::TOOLBAR_AUTOHIDE;
      } else if (key.contains(QRegExp("^(fore|back|color|bw)(_button)?$")))
        wantmode |= plus;
      else if (key=="pan" || key=="zoomsel" || key=="textsel")
        wantselect |= plus;
      else if (key=="modecombo")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_MODECOMBO);
      else if (key=="rescombo" || key=="zoomcombo")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_ZOOMCOMBO);
      else if (key=="zoom")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_ZOOMBUTTONS);
      else if (key=="rotate")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_ROTATE);
      else if (key=="search" || key=="find")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_FIND);
      else if (key=="print")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_PRINT);
      else if (key=="save")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_SAVE);
      else if (key=="pagecombo")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_PAGECOMBO);
      else if (key=="backforw")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_BACKFORW);
      else if (key=="prevnext" || key=="prevnextpage")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_PREVNEXT);
      else if (key=="firstlast" || key=="firstlastpage")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_FIRSTLAST);
      // lizards
      else if (key=="doublepage")  // lizards!
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_LAYOUT);
      else if (key=="calibrate" || key=="ruler" || key=="lizard")
        errors << tr("Toolbar option '%1' is not implemented.").arg(key);
      // djview4
      else if (key=="select")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_SELECT);
      else if (key=="new")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_NEW);
      else if (key=="open")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_OPEN);
      else if (key=="layout")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_LAYOUT);
      else if (key=="help")
        set_reset(tools, plus, minus, QDjViewPrefs::TOOL_WHATSTHIS);
      else if (key!="")
        errors << tr("Toolbar option '%1' is not recognized.").arg(key);
      // handle + or -
      if (npos < len)
        {
          Tools base = QDjViewPrefs::TOOLBAR_AUTOHIDE |
            QDjViewPrefs::TOOLBAR_TOP | QDjViewPrefs::TOOLBAR_BOTTOM;
          if (str[npos] == '-')
            {
              if (!minus && !plus)
                tools |= (prefs->tools & ~base);
              plus = false;
              minus = true;
            }
          else if (str[npos] == '+')
            {
              if (!minus && !plus)
                tools &= base;
              minus = false;
              plus = true;
            }
          npos += 1;
        }
    }
  if (wantmode)
    tools |= QDjViewPrefs::TOOL_MODECOMBO;
  if (wantselect)
    tools |= QDjViewPrefs::TOOL_SELECT;
}



/*! Parse a qdjview option described by a pair \a key, \a value.
  Such options can be provided on the command line 
  or passed with the document URL as pseudo-CGI arguments.
  Return a list of error strings. */

QStringList
QDjView::parseArgument(QString key, QString value)
{
  bool okay;
  QStringList errors;
  key = key.toLower();
  
  if (key == "fullscreen" || key == "fs")
    {
      if (viewerMode != STANDALONE)
        errors << tr("Option '%1' requires a standalone viewer.").arg(key);
      if (parse_boolean(key, value, errors, okay))
        if (actionViewFullScreen->isChecked() != okay)
          actionViewFullScreen->activate(QAction::Trigger);
    }
  else if (key == "toolbar")
    {
      parseToolBarOption(value, errors);
      toolBar->setVisible(options & QDjViewPrefs::SHOW_TOOLBAR);
    }
  else if (key == "page")
    {
      goToPage(value);
    }
  else if (key == "pageno")
    {
      goToPage(value.toInt()-1);
    }
  else if (key == "cache")
    {
      // see QDjVuDocument::setUrl(...)
      parse_boolean(key, value, errors, okay);
    }
  else if (key == "passive" || key == "passivestretch")
    {
      if (parse_boolean(key, value, errors, okay) && okay)
        {
          enableScrollBars(false);
          enableContextMenu(false);
          menuBar->setVisible(false);
          statusBar->setVisible(false);
          thumbnailDock->setVisible(false);
          outlineDock->setVisible(false);
          findDock->setVisible(false);
          toolBar->setVisible(false);
          if (key == "passive")
            widget->setZoom(QDjVuWidget::ZOOM_FITPAGE);
          else
            widget->setZoom(QDjVuWidget::ZOOM_STRETCH);
          widget->setContinuous(false);
          widget->setSideBySide(false);
          widget->enableKeyboard(false);
          widget->enableMouse(false);
        }
    }
  else if (key == "scrollbars")
    {
      if (parse_boolean(key, value, errors, okay))
        enableScrollBars(okay);
    }
  else if (key == "menubar")    // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        menuBar->setVisible(okay);
    }
  else if (key == "sidebar" ||  // new for djview4
           key == "navpane" )   // lizards
    {
      showSideBar(value, errors);
    }
  else if (key == "statusbar")   // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        statusBar->setVisible(okay);
    }
  else if (key == "continuous")  // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        widget->setContinuous(okay);
    }
  else if (key == "side_by_side" ||
           key == "sidebyside")  // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        widget->setSideBySide(okay);
    }
  else if (key == "firstpagealone" ||
           key == "first_page_alone" ||
           key == "cover_page" ||
           key == "coverpage")   // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        widget->setCoverPage(okay);
    }
  else if (key == "right_to_left" ||
           key == "righttoleft") // new for djview4
    {
      if (parse_boolean(key, value, errors, okay))
        widget->setRightToLeft(okay);
    }
  else if (key == "layout")      // lizards
    {
      bool layout = false;
      bool continuous = false;
      bool sidebyside = false;
      foreach (QString s, value.split(","))
        {
          if (s == "single")
            layout = true, continuous = sidebyside = false;
          else if (s == "double") 
            layout = sidebyside = true;
          else if (s == "continuous")
            layout = continuous = true;
          else if (s == "ltor")
            widget->setRightToLeft(false);
          else if (s == "rtol")
            widget->setRightToLeft(true);
          else if (s == "cover")
            widget->setCoverPage(true);
          else if (s == "nocover")
            widget->setCoverPage(false);
          else if (s == "gap")
            widget->setSeparatorSize(12);
          else if (s == "nogap")
            widget->setSeparatorSize(0);
        }
      if (layout)
        {
          widget->setContinuous(continuous);
          widget->setSideBySide(sidebyside);
        }
    }
  else if (key == "frame")
    {
      if (parse_boolean(key, value, errors, okay))
        widget->setDisplayFrame(okay);
    }
  else if (key == "background")
    {
      QColor color;
      color.setNamedColor((value[0] == '#') ? value : "#" + value);
      if (color.isValid())
        widget->setBorderBrush(color);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "menu")
    {
      if (parse_boolean(key, value, errors, okay))
        enableContextMenu(okay);
    }
  else if (key == "keyboard")
    {
      if (parse_boolean(key, value, errors, okay))
        widget->enableKeyboard(okay);
    }
  else if (key == "mouse")
    {
      if (parse_boolean(key, value, errors, okay))
        widget->enableMouse(okay);
    }
  else if (key == "links")
    {
      if (parse_boolean(key, value, errors, okay))
        widget->enableHyperlink(okay);
    }
  else if (key == "zoom")
    {
      int z = value.toInt(&okay);
      QString val = value.toLower();
      if (val == "one2one")
        widget->setZoom(QDjVuWidget::ZOOM_ONE2ONE);
      else if (val == "width" || val == "fitwidth")
        widget->setZoom(QDjVuWidget::ZOOM_FITWIDTH);
      else if (val == "page" || val == "fitpage")
        widget->setZoom(QDjVuWidget::ZOOM_FITPAGE);
      else if (val == "stretch")
        widget->setZoom(QDjVuWidget::ZOOM_STRETCH);
      else if (okay && z >= QDjVuWidget::ZOOM_MIN 
               && z <= QDjVuWidget::ZOOM_MAX)
        widget->setZoom(z);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "mode")
    {
      QString val = value.toLower();
      if (val == "color")
        widget->setDisplayMode(QDjVuWidget::DISPLAY_COLOR);
      else if (val == "bw" || val == "mask" || val == "stencil")
        widget->setDisplayMode(QDjVuWidget::DISPLAY_STENCIL);
      else if (val == "fore" || val == "foreground" || val == "fg")
        widget->setDisplayMode(QDjVuWidget::DISPLAY_FG);
      else if (val == "back" || val == "background" || val == "bg")
        widget->setDisplayMode(QDjVuWidget::DISPLAY_BG);
      else if (val == "text" || val == "hiddentext")
        widget->setDisplayMode(QDjVuWidget::DISPLAY_TEXT);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "hor_align" || key == "halign")
    {
      QString val = value.toLower();
      if (val == "left")
        widget->setHorizAlign(QDjVuWidget::ALIGN_LEFT);
      else if (val == "center")
        widget->setHorizAlign(QDjVuWidget::ALIGN_CENTER);
      else if (val == "right")
        widget->setHorizAlign(QDjVuWidget::ALIGN_RIGHT);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "ver_align" || key == "valign")
    {
      QString val = value.toLower();
      if (val == "top")
        widget->setVertAlign(QDjVuWidget::ALIGN_TOP);
      else if (val == "center")
        widget->setVertAlign(QDjVuWidget::ALIGN_CENTER);
      else if (val == "bottom")
        widget->setVertAlign(QDjVuWidget::ALIGN_BOTTOM);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "highlight")
    {                           
      int x,y,w,h;
      QColor color;
      if (value.isEmpty())
        widget->clearHighlights(widget->page());
      else if (parse_highlight(value, x, y, w, h, color))
        pendingHilite << StringPair(pendingPage, value);
      else
        illegal_value(key, value, errors);
      performPendingLater();
    }
  else if (key == "rotate")
    {
      if (value == "0")
        widget->setRotation(0);
      else if (value == "90")
        widget->setRotation(1);
      else if (value == "180")
        widget->setRotation(2);
      else if (value == "270")
        widget->setRotation(3);
      else
        illegal_value(key, value, errors);
    }
  else if (key == "print")
    {
      if (parse_boolean(key, value, errors, okay))
        printingAllowed = okay;
    }
  else if (key == "save")
    {
      if (parse_boolean(key, value, errors, okay))
        savingAllowed = okay;
    }
  else if (key == "notoolbar" || 
           key == "noscrollbars" ||
           key == "nomenu" )
    {
      QString msg = tr("Deprecated option '%1'").arg(key);
      qWarning((const char*)msg.toLocal8Bit());
      if (key == "notoolbar" && value.isNull())
        toolBar->setVisible(false);
      else if (key == "noscrollbars" && value.isNull())
        enableScrollBars(false);
      else if (key == "nomenu" && value.isNull())
        enableContextMenu(false);
    }
  else if (key == "thumbnails")
    {
      if (! showSideBar(value+",thumbnails"))
        illegal_value(key, value, errors);
    }
  else if (key == "outline")
    {
      if (! showSideBar(value+",outline"))
        illegal_value(key, value, errors);
    }
  else if (key == "find") // new for djview4
    {
      if (findWidget) 
        {
          findWidget->setText(QString::null);
          findWidget->setRegExpMode(false);
          findWidget->setWordOnly(true);
          findWidget->setCaseSensitive(false);
        }
      pendingFind = value;
      if (! value.isEmpty())
        performPendingLater();
    }
  else if (key == "showposition")
    {
      double x=0, y=0;
      pendingPosition.clear();
      if (! parse_position(value, x, y)) 
        illegal_value(key, value, errors);
      else
        {
          pendingPosition << x << y;
          performPendingLater();
        } 
    }
  else if (key == "logo" || key == "textsel" || key == "search")
    {
      QString msg = tr("Option '%1' is not implemented.").arg(key);
      qWarning((const char*)msg.toLocal8Bit());
    }
  else
    {
      errors << tr("Option '%1' is not recognized.").arg(key);
    }
  updateActionsLater();
  return errors;
}


/*! Parse a \a QDjView option expressed a a string \a key=value.
  This is very useful for processing command line options. */

QStringList
QDjView::parseArgument(QString keyEqualValue)
{
  int n = keyEqualValue.indexOf("=");
  if (n < 0)
    return parseArgument(keyEqualValue, QString());
  else
    return parseArgument(keyEqualValue.left(n),
                         keyEqualValue.mid(n+1));
}


/*! Parse the \a QDjView options passed via 
  the CGI query arguments of \a url. 
  This is called by \a QDjView::open(QDjVuDocument, QUrl). */

void 
QDjView::parseDjVuCgiArguments(QUrl url)
{
  QStringList errors;
  // parse
  bool djvuopts = false;
  QPair<QString,QString> pair;
  foreach(pair, url.queryItems())
    {
      if (pair.first.toLower() == "djvuopts")
        djvuopts = true;
      else if (djvuopts)
        errors << parseArgument(pair.first, pair.second);
    }
  // warning for errors
  if (djvuopts && errors.size() > 0)
    foreach(QString error, errors)
      qWarning((const char*)error.toLocal8Bit());
}


/*! Return a copy of the url without the
  CGI query arguments corresponding to DjVu options. */

QUrl 
QDjView::removeDjVuCgiArguments(QUrl url)
{
  QList<QPair<QString,QString> > args;
  bool djvuopts = false;
  QPair<QString,QString> pair;
  foreach(pair, url.queryItems())
    {
      if (pair.first.toLower() == "djvuopts")
        djvuopts = true;
      else if (!djvuopts)
        args << pair;
    }
  QUrl newurl = url;
  newurl.setQueryItems(args);
  return newurl;
}


/*! Returns the most adequate value
  that could be passed to \a parseArgument
  for the specified \a key. */

QString      
QDjView::getArgument(QString key)
{
  key = key.toLower();
  if (key == "pages") // readonly
    return QString::number(pageNum());
  else if (key == "page")
    return pageName(widget->page());
  else if (key == "pageno")
    return QString::number(widget->page()+1);
  else if (key == "fullscreen" || key == "fs") 
    return get_boolean((viewerMode == STANDALONE) && 
                       actionViewFullScreen->isChecked());    
  else if (key == "scrollbars")
    return get_boolean(widget->horizontalScrollBarPolicy() != 
                       Qt::ScrollBarAlwaysOff );
  else if (key == "menubar")
    return get_boolean(menuBar->isVisible());
  else if (key == "statusbar")
    return get_boolean(statusBar->isVisible());
  else if (key == "continuous")
    return get_boolean(widget->continuous());
  else if (key == "side_by_side" || key == "sidebyside")
    return get_boolean(widget->sideBySide());
  else if (key == "coverpage" || key == "firstpagealone" || 
           key == "first_page_alone" )
    return get_boolean(widget->coverPage());
  else if (key == "righttoleft" || key == "right_to_left")
    return get_boolean(widget->rightToLeft());
  else if (key == "frame")
    return get_boolean(widget->displayFrame());
  else if (key == "keyboard")
    return get_boolean(widget->keyboardEnabled());
  else if (key == "mouse")
    return get_boolean(widget->mouseEnabled());
  else if (key == "links")
    return get_boolean(widget->hyperlinkEnabled());
  else if (key == "menu")
    return get_boolean(widget->contextMenu() != 0);
  else if (key == "rotate")
    return QString::number(widget->rotation() * 90);
  else if (key == "print")
    return get_boolean(printingAllowed);
  else if (key == "save")
    return get_boolean(savingAllowed);
  else if (key == "background")
    {
      QBrush brush = widget->borderBrush();
      if (brush.style() == Qt::SolidPattern)
        return brush.color().name();
    }
  else if (key == "layout")
    {
      QStringList l;
      if (widget->sideBySide())
        l << QString("double");
      if (widget->continuous())
        l << QString("continuous");
      if (l.isEmpty())
        l << QString("single");
      l << QString(widget->rightToLeft() ? "rtol" : "ltor");
      l << QString(widget->coverPage() ? "cover" : "nocover");
      l << QString(widget->separatorSize() ? "gap" : "nogap");
      return l.join(",");
    }
  else if (key == "zoom")
    {
      int z = widget->zoom();
      if (z == QDjVuWidget::ZOOM_ONE2ONE)
        return QString("one2one");
      else if (z == QDjVuWidget::ZOOM_FITWIDTH)
        return QString("width");
      else if (z == QDjVuWidget::ZOOM_FITPAGE)
        return QString("page");
      else if (z == QDjVuWidget::ZOOM_STRETCH)
        return QString("stretch");
      else if (z >= QDjVuWidget::ZOOM_MIN && z <= QDjVuWidget::ZOOM_MAX)
        return QString::number(z);
    }
  else if (key == "mode")
    {
      QDjVuWidget::DisplayMode m = widget->displayMode();
      if (m == QDjVuWidget::DISPLAY_COLOR)
        return QString("color");
      else if (m == QDjVuWidget::DISPLAY_STENCIL)
        return QString("bw");
      else if (m == QDjVuWidget::DISPLAY_FG)
        return QString("fore");
      else if (m == QDjVuWidget::DISPLAY_BG)
        return QString("back");
      else if (m == QDjVuWidget::DISPLAY_TEXT)
        return QString("text");
    }
  else if (key == "hor_align" || key == "halign")
    {
      QDjVuWidget::Align a = widget->horizAlign();
      if (a == QDjVuWidget::ALIGN_LEFT)
        return QString("left");
      else if (a == QDjVuWidget::ALIGN_CENTER)
        return QString("center");
      else if (a == QDjVuWidget::ALIGN_RIGHT)
        return QString("right");
    }
  else if (key == "ver_align" || key == "valign")
    {
      QDjVuWidget::Align a = widget->vertAlign();
      if (a == QDjVuWidget::ALIGN_TOP)
        return QString("top");
      else if (a == QDjVuWidget::ALIGN_CENTER)
        return QString("center");
      else if (a == QDjVuWidget::ALIGN_BOTTOM)
        return QString("bottom");
    }
  else if (key == "showposition")
    {
      QPoint center = widget->rect().center();
      QDjVuWidget::Position pos = widget->positionWithClosestAnchor(center);
      double ha = pos.hAnchor / 100.0;
      double va = pos.vAnchor / 100.0;
      return QString("%1,%2").arg(ha).arg(va);
    }
  else if (key == "passive" || key == "passivestretch")
    {
      if (widget->horizontalScrollBarPolicy() == Qt::ScrollBarAlwaysOff 
          && !menuBar->isVisible() && !statusBar->isVisible()
          && !thumbnailDock->isVisible() && !outlineDock->isVisible()
          && !findDock->isVisible() && !toolBar->isVisible()
          && !widget->continuous() && !widget->sideBySide()
          && !widget->keyboardEnabled() && !widget->mouseEnabled() 
          && widget->contextMenu() == 0 )
        {
          int z = widget->zoom();
          if (key == "passive" && z == QDjVuWidget::ZOOM_FITPAGE)
            return QString("yes");
          if (key == "passivestretch" && z == QDjVuWidget::ZOOM_STRETCH)
            return QString("yes");
        }
      return QString("no");
    }
  else if (key == "sidebar")
    { // without position 
      QStringList what;
      if (thumbnailDock->isVisible())
        what << "thumbnails";
      if (outlineDock->isVisible())
        what << "outline";
      if (findDock->isVisible())
        what << "find";
      if (what.size() > 0)
        return QString("yes,") + what.join(",");
      else
        return QString("no");
    }
  else if (key == "toolbar")
    { // Currently without toolbar content
      return get_boolean(toolBar->isVisible());
    }
  else if (key == "highlight")
    { // Currently not working
      return QString();
    }
  return QString();
}








// ----------------------------------------
// QDJVIEW


/*! \enum QDjView::ViewerMode
  The viewer mode is selected at creation time.
  Mode \a STANDALONE corresponds to a standalone program.
  Modes \a FULLPAGE_PLUGIN and \a EMBEDDED_PLUGIN
  correspond to a netscape plugin. */


/*! Construct a \a QDjView object using
  the specified djvu context and viewer mode. */

QDjView::QDjView(QDjVuContext &context, ViewerMode mode, QWidget *parent)
  : QMainWindow(parent),
    viewerMode(mode),
    djvuContext(context),
    document(0),
    hasNumericalPageTitle(true),
    updateActionsScheduled(false),
    performPendingScheduled(false),
    printingAllowed(true),
    savingAllowed(true)
{
  // Main window setup
  setWindowTitle(tr("DjView"));
  setWindowIcon(QIcon(":/images/djview.png"));
#ifndef Q_WS_MAC
  if (QApplication::windowIcon().isNull())
    QApplication::setWindowIcon(windowIcon());
#endif

  // Determine unique object name
  if (mode == STANDALONE) 
    {
      QWidgetList wl = QApplication::topLevelWidgets();
      static int num = 0;
      QString name;
      while(name.isEmpty()) 
        {
          name = QString("djview%1").arg(num++);
          foreach(QWidget *w, wl)
            if (w->objectName() == name)
              name = QString::null;
        }
      setObjectName(name);
    }
  
  // Basic preferences
  prefs = QDjViewPrefs::instance();
  options = QDjViewPrefs::defaultOptions;
  tools = prefs->tools;
  toolsCached = 0;
  fsWindowState = 0;
  
  // Create dialogs
  errorDialog = new QDjViewErrorDialog(this);
  infoDialog = 0;
  metaDialog = 0;
  
  // Create djvu widget
  QWidget *central = new QWidget(this);
  widget = new QDjVuWidget(central);
  widget->setFrameShape(QFrame::NoFrame);
  if (viewerMode == STANDALONE)
    {
      widget->setFrameShadow(QFrame::Sunken);
      widget->setFrameShape(QFrame::Box);
    }
  widget->viewport()->installEventFilter(this);
  connect(widget, SIGNAL(errorCondition(int)),
          this, SLOT(errorCondition(int)));
  connect(widget, SIGNAL(error(QString,QString,int)),
          errorDialog, SLOT(error(QString,QString,int)));
  connect(widget, SIGNAL(error(QString,QString,int)),
          this, SLOT(error(QString,QString,int)));
  connect(widget, SIGNAL(info(QString)),
          this, SLOT(info(QString)));
  connect(widget, SIGNAL(layoutChanged()),
          this, SLOT(updateActionsLater()));
  connect(widget, SIGNAL(pageChanged(int)),
          this, SLOT(updateActionsLater()));
  connect(widget, SIGNAL(pointerPosition(const Position&,const PageInfo&)),
          this, SLOT(pointerPosition(const Position&,const PageInfo&)));
  connect(widget, SIGNAL(pointerEnter(const Position&,miniexp_t)),
          this, SLOT(pointerEnter(const Position&,miniexp_t)));
  connect(widget, SIGNAL(pointerLeave(const Position&,miniexp_t)),
          this, SLOT(pointerLeave(const Position&,miniexp_t)));
  connect(widget, SIGNAL(pointerClick(const Position&,miniexp_t)),
          this, SLOT(pointerClick(const Position&,miniexp_t)));
  connect(widget, SIGNAL(pointerSelect(const QPoint&,const QRect&)),
          this, SLOT(pointerSelect(const QPoint&,const QRect&)));

  // Create splash screen
  splash = new QLabel(central);
  splash->setFrameShape(QFrame::Box);
  splash->setFrameShadow(QFrame::Sunken);
  splash->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  splash->setPixmap(QPixmap(":/images/splash.png"));
  QPalette palette = splash->palette();
  palette.setColor(QPalette::Background, Qt::white);
  splash->setPalette(palette);
  splash->setAutoFillBackground(true);

  // Create central layout
  layout = new QStackedLayout(central);
  layout->addWidget(widget);
  layout->addWidget(splash);
  layout->setCurrentWidget(splash);
  setCentralWidget(central);

  // Create context menu
  contextMenu = new QMenu(this);
  recentMenu = 0;

  // Create menubar
  menuBar = new QMenuBar(this);
  setMenuBar(menuBar);

  // Create statusbar
  statusBar = new QStatusBar(this);
  QFont font = QApplication::font();
  font.setPointSize((font.pointSize() * 3 + 3) / 4);
  QFontMetrics metric(font);
  textLabelTimer = new QTimer(this);
  textLabelTimer->setSingleShot(true);
  connect(textLabelTimer,SIGNAL(timeout()),this,SLOT(updateTextLabel()));
  textLabel = new QLabel(statusBar);
  textLabel->setFont(font);
  textLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  textLabel->setFrameStyle(QFrame::Panel);
  textLabel->setFrameShadow(QFrame::Sunken);
  textLabel->setMinimumWidth(metric.width("M")*48);
  statusBar->addPermanentWidget(textLabel);
  pageLabel = new QLabel(statusBar);
  pageLabel->setFont(font);
  pageLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  pageLabel->setFrameStyle(QFrame::Panel);
  pageLabel->setFrameShadow(QFrame::Sunken);
  pageLabel->setMinimumWidth(metric.width(" P88 8888x8888 888dpi ")); 
  statusBar->addPermanentWidget(pageLabel);
  mouseLabel = new QLabel(statusBar);
  mouseLabel->setFont(font);
  mouseLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
  mouseLabel->setFrameStyle(QFrame::Panel);
  mouseLabel->setFrameShadow(QFrame::Sunken);
  mouseLabel->setMinimumWidth(metric.width(" x=888 y=888 "));
  statusBar->addPermanentWidget(mouseLabel);
  setStatusBar(statusBar);

  // Create toolbar  
  toolBar = new QToolBar(this);
  toolBar->setObjectName("toolbar");
  toolBar->setAllowedAreas(Qt::TopToolBarArea|Qt::BottomToolBarArea);
  addToolBar(toolBar);

  // Create mode combo box
  modeCombo = new QComboBox(toolBar);
  fillModeCombo(modeCombo);
  connect(modeCombo, SIGNAL(activated(int)),
          this, SLOT(modeComboActivated(int)) );
  connect(modeCombo, SIGNAL(activated(int)),
          this, SLOT(updateActionsLater()) );

  // Create zoom combo box
  zoomCombo = new QComboBox(toolBar);
  zoomCombo->setEditable(true);
  zoomCombo->setInsertPolicy(QComboBox::NoInsert);
  fillZoomCombo(zoomCombo);
  connect(zoomCombo, SIGNAL(activated(int)),
          this, SLOT(zoomComboActivated(int)) );
  connect(zoomCombo->lineEdit(), SIGNAL(editingFinished()),
          this, SLOT(zoomComboEdited()) );

  // Create page combo box
  pageCombo = new QComboBox(toolBar);
  pageCombo->setSizeAdjustPolicy(QComboBox::AdjustToContents);
  pageCombo->setMinimumWidth(80);
  pageCombo->setEditable(true);
  pageCombo->setInsertPolicy(QComboBox::NoInsert);
  connect(pageCombo, SIGNAL(activated(int)),
          this, SLOT(pageComboActivated(int)));
  connect(pageCombo->lineEdit(), SIGNAL(editingFinished()),
          this, SLOT(pageComboEdited()) );
  
  // Create sidebar  
  thumbnailWidget = new QDjViewThumbnails(this);
  thumbnailDock = new QDockWidget(this);
  thumbnailDock->setObjectName("thumbnailDock");
  thumbnailDock->setWindowTitle(tr("&Thumbnails"));
  thumbnailDock->setWidget(thumbnailWidget);
  thumbnailDock->installEventFilter(this);
  addDockWidget(Qt::LeftDockWidgetArea, thumbnailDock);
  outlineWidget = new QDjViewOutline(this);
  outlineDock = new QDockWidget(this);
  outlineDock->setObjectName("outlineDock");
  outlineDock->setWindowTitle(tr("&Outline")); 
  outlineDock->setWidget(outlineWidget);
  outlineDock->installEventFilter(this);
  addDockWidget(Qt::LeftDockWidgetArea, outlineDock);
  findWidget = new QDjViewFind(this);
  findDock = new QDockWidget(this);
  findDock->setObjectName("findDock");
  findDock->setWindowTitle(tr("&Find")); 
  findDock->setWidget(findWidget);
  findDock->installEventFilter(this);
  addDockWidget(Qt::LeftDockWidgetArea, findDock);

  // Create escape shortcut for sidebar
  shortcutEscape = new QShortcut(QKeySequence("Esc"), this);
  connect(shortcutEscape, SIGNAL(activated()), this, SLOT(performEscape()));

  // Create undoTimer.
  undoTimer = new QTimer(this);
  undoTimer->setSingleShot(true);
  connect(undoTimer,SIGNAL(timeout()), this, SLOT(saveUndoData()));

  // Actions
  createActions();
  createMenus();
  createWhatsThis();
  enableContextMenu(true);
  
  // Preferences
  connect(prefs, SIGNAL(updated()), this, SLOT(preferencesChanged()));
  applyPreferences();
  updateActions();

  // Options set so far have default priority
  widget->reduceOptionsToPriority(QDjVuWidget::PRIORITY_DEFAULT);
}


void 
QDjView::closeDocument()
{
  here.clear();
  undoList.clear();
  redoList.clear();
  layout->setCurrentWidget(splash);
  QDjVuDocument *doc = document;
  if (doc)
    {
      doc->ref();
      disconnect(doc, 0, this, 0);
      disconnect(doc, 0, errorDialog, 0);
      printingAllowed = true;
      savingAllowed = true;
    }
  widget->setDocument(0);
  documentPages.clear();
  documentFileName.clear();
  hasNumericalPageTitle = true;
  documentUrl.clear();
  textLabelTimer->stop();
  document = 0;
  if (doc)
    {
      emit documentClosed(doc);
      doc->deref();
    }
}


/*! Open a document represented by a \a QDjVuDocument object. */

void 
QDjView::open(QDjVuDocument *doc, QUrl url)
{
  closeDocument();
  document = doc;
  if (url.isValid())
    documentUrl = url;
  connect(doc,SIGNAL(destroyed(void)), this, SLOT(closeDocument(void)));
  connect(doc,SIGNAL(docinfo(void)), this, SLOT(docinfo(void)));
  widget->setDocument(document);
  disconnect(document, 0, errorDialog, 0);
  layout->setCurrentWidget(widget);
  updateActions();
  docinfo();
  if (doc)
    emit documentOpened(doc);
  // process url options
  if (url.isValid())
    parseDjVuCgiArguments(url);
  widget->reduceOptionsToPriority(QDjVuWidget::PRIORITY_CGI);
  // set focus
  widget->setFocus();
}


/*! Open the djvu document stored in file \a filename. */

bool
QDjView::open(QString filename)
{
  closeDocument();
  QDjVuDocument *doc = new QDjVuDocument(true);
  connect(doc, SIGNAL(error(QString,QString,int)),
          errorDialog, SLOT(error(QString,QString,int)));
  doc->setFileName(&djvuContext, filename);
  if (!doc->isValid())
    {
      delete doc;
      addToErrorDialog(tr("Cannot open file '%1'.").arg(filename));
      raiseErrorDialog(QMessageBox::Critical, tr("Opening DjVu file"));
      return false;
    }
  QFileInfo fileinfo(filename);
  QUrl url = QUrl::fromLocalFile(fileinfo.absoluteFilePath());
  open(doc, url);
  documentFileName = filename;
  addRecent(url);
  setWindowTitle(QString("%1[*] - ").arg(getShortFileName()) + tr("DjView"));
  return true;
}


/*! Open the djvu document available at url \a url.
  Only the \a http: and \a file: protocols are supported. */

bool
QDjView::open(QUrl url)
{
  closeDocument();
  QDjVuHttpDocument *doc = new QDjVuHttpDocument(true);
  connect(doc, SIGNAL(error(QString,QString,int)),
          errorDialog, SLOT(error(QString,QString,int)));
  if (prefs->proxyUrl.isValid() && prefs->proxyUrl.scheme() == "http")
    {
      QUrl proxyUrl = prefs->proxyUrl;
      QString host =  proxyUrl.host();
      int port = proxyUrl.port(8080);
      QString user = proxyUrl.userName();
      QString pass = proxyUrl.password();
      if (!host.isEmpty() && proxyUrl.path().isEmpty() && port >=0 )
        doc->setProxy(host, port, user, pass);
    }
  
  QUrl docurl = removeDjVuCgiArguments(url);
  doc->setUrl(&djvuContext, docurl);
  if (!doc->isValid())
    {
      delete doc;
      addToErrorDialog(tr("Cannot open URL '%1'.").arg(url.toString()));
      raiseErrorDialog(QMessageBox::Critical, tr("Opening DjVu document"));
      return false;
    }
  open(doc, url);
  addRecent(docurl);
  setWindowTitle(QString("%1[*] - ").arg(getShortFileName()) + tr("DjView"));
  return true;
}


/*! Jump to the page numbered \a pageno. */

void 
QDjView::goToPage(int pageno)
{
  int pagenum = documentPages.size();
  if (!pagenum || !document)
    {
      pendingPage = QString("#%1").arg(pageno+1);
    }
  else
    {
      if (pageno>=0 && pageno<pagenum)
        {
          widget->setPage(pageno);
        }
      else
        {
          QString msg = tr("Cannot find page numbered: %1").arg(pageno+1);
          qWarning((const char*)msg.toLocal8Bit());
        }
      updateActionsLater();
    }
}


/*! Jump to the page named \a name. 
  Names starting with the "#" character are interpreted
  using the same rules as hyperlinks. In particular,
  names of the form \a "#+n" and \a "#-n" express
  relative displacement from the current page
  or the page specified by argument \a from. */

void 
QDjView::goToPage(QString name, int from)
{
  int pagenum = documentPages.size();
  if (!pagenum || !document)
    {
      pendingPage = name;
    }
  else
    {
      int pageno = pageNumber(name, from);
      if (pageno >= 0 && pageno < pagenum)
        {
          widget->setPage(pageno);
        }
      else
        {
          QString msg = tr("Cannot find page named: %1").arg(name);
          qWarning((const char*)msg.toLocal8Bit());
        }
      updateActionsLater();
    }
}


/* Jump to the page named \a pagename
   and tries to position the specified point in
   the center of the window. Arguments \a px and \a py
   must be in range 0 to 1 indicating a fraction
   of the page width or height. */

void  
QDjView::goToPosition(QString name, double px, double py)
{
  int pagenum = documentPages.size();
  if (!pagenum || !document)
    {
      pendingPosition.clear();
      pendingPosition << px << py;
      if (! name.isEmpty())
        pendingPage = name;
    }
  else
    {
      int pageno = (name.isEmpty()) ? widget->page() : pageNumber(name);
      if (pageno < 0 || pageno >= pagenum)
        {
          QString msg = tr("Cannot find page named: %1").arg(name);
          qWarning((const char*)msg.toLocal8Bit());
        }
      else 
        {
          QDjVuWidget::Position pos;
          pos.pageNo = pageno;
          pos.inPage = false;
          pos.posView = QPoint(0,0);
          pos.posPage = QPoint(0,0);
          pos.hAnchor = qBound(0, (int)(px*100), 100);
          pos.vAnchor = qBound(0, (int)(py*100), 100);
          widget->setPosition(pos, widget->rect().center());
        }
      updateActionsLater();
    }
}



/*! Add a message to the error dialog. */

void
QDjView::addToErrorDialog(QString message)
{
  errorDialog->error(message, __FILE__, __LINE__);
}


/*! Show the error dialog. */

void
QDjView::raiseErrorDialog(QMessageBox::Icon icon, QString caption)
{
  errorDialog->prepare(icon, caption);
  errorDialog->show();
  errorDialog->raise();
  errorDialog->activateWindow();
}


/*! Show the error dialog and wait until the user clicks "OK". */

int
QDjView::execErrorDialog(QMessageBox::Icon icon, QString caption)
{
  errorDialog->prepare(icon, caption);
  return errorDialog->exec();
}


/*! Set the content of the page box in the status bar. */

void  
QDjView::setPageLabelText(QString s)
{
  QLabel *m = pageLabel;
  m->setText(s);
  m->setMinimumWidth(qMax(m->minimumWidth(), m->sizeHint().width()));
}


/*! Set the content of the mouse box in the status bar. */

void  
QDjView::setMouseLabelText(QString s)
{
  QLabel *m = mouseLabel;
  m->setText(s);
  m->setMinimumWidth(qMax(m->minimumWidth(), m->sizeHint().width()));
}


/*! Display transient message in the status bar.
  This also emits signal \a pluginStatusMessage
  to mirror the message in the browser status bar. */

void  
QDjView::statusMessage(QString message)
{
  statusBar->showMessage(message);
  emit pluginStatusMessage(message);
}


/*! Returns a bitmask of the visible sidebar tabs. */

int
QDjView::visibleSideBarTabs()
{
  int tabs = 0;
  if (! thumbnailDock->isHidden())
    tabs |= 1;
  if (! outlineDock->isHidden())
    tabs |= 2;
  if (! findDock->isHidden())
    tabs |= 4;
  return tabs;
}


/*! Returns a bitmask of the hidden sidebar tabs. */

int
QDjView::hiddenSideBarTabs()
{
  int tabs = 0;
  if (thumbnailDock->isHidden())
    tabs |= 1;
  if (outlineDock->isHidden())
    tabs |= 2;
  if (findDock->isHidden())
    tabs |= 4;
  return tabs;
}


/*! Change the position and the visibility of 
    the sidebars specified by bitmask \a tabs. */

bool  
QDjView::showSideBar(Qt::DockWidgetAreas areas, int tabs)
{
  QList<QDockWidget*> allDocks;
  allDocks << thumbnailDock << outlineDock << findDock;
  // find first tab in desired area
  QDockWidget *first = 0;
  foreach(QDockWidget *w, allDocks)
    if (!first && !w->isHidden() && !w->isFloating())
      if (dockWidgetArea(w) & areas)
        first = w;
  // find relevant docks
  QList<QDockWidget*> docks;
  if (tabs & 1)
    docks << thumbnailDock;
  if (tabs & 2)
    docks << outlineDock;
  if (tabs & 4)
    docks << findDock;
  // hide all
  foreach(QDockWidget *w, docks)
    w->hide();
  if (areas)
    {
      Qt::DockWidgetArea area = Qt::LeftDockWidgetArea;
      if (areas & Qt::LeftDockWidgetArea)
        area = Qt::LeftDockWidgetArea;
      else if (areas & Qt::RightDockWidgetArea)
        area = Qt::RightDockWidgetArea;
      else if (areas & Qt::TopDockWidgetArea)
        area = Qt::TopDockWidgetArea;
      else if (areas & Qt::BottomDockWidgetArea)
        area = Qt::BottomDockWidgetArea;
      foreach(QDockWidget *w, docks)
        {
          w->show();
          if (w->isFloating())
            w->setFloating(false);
          if (! (areas & dockWidgetArea(w)))
            {
              removeDockWidget(w);
              addDockWidget(area, w);
              if (first)
                tabifyDockWidget(first, w);
              else 
                first = w;
            }
          w->show();
          w->raise();
        }
    }
  // Okay
  return true;
}


/*! Change the position and composition of the sidebar.
  String \a args contains comma separated keywords:
  \a left, \a right, \a top, \a bottom, \a yes, or \a no,
  \a outline, \a bookmarks, \a thumbnails, \a search, etc.
  Error messages are added to string list \a errors. */

bool
QDjView::showSideBar(QString args, QStringList &errors)
{
  bool no = false;
  bool ret = true;
  int tabs = 0;
  Qt::DockWidgetAreas areas = 0;
  QString arg;
  foreach(arg, args.split(",", QString::SkipEmptyParts))
    {
      arg = arg.toLower();
      if (arg == "no" || arg == "false")
        no = true;
      else if (arg == "left")
        areas |= Qt::LeftDockWidgetArea;
      else if (arg == "right")
        areas |= Qt::RightDockWidgetArea;
      else if (arg == "top")
        areas |= Qt::TopDockWidgetArea;
      else if (arg == "bottom")
        areas |= Qt::BottomDockWidgetArea;
      else if (arg == "thumbnails" || arg == "thumbnail")
        tabs |= 1;
      else if (arg == "outline" || arg == "bookmarks")
        tabs |= 2;
      else if (arg == "search" || arg == "find")
        tabs |= 4;
      else if (arg != "yes" && arg != "true") {
        errors << tr("Unrecognized sidebar options '%1'.").arg(arg);
        ret = false;
      }
    }
  if (! tabs)
    tabs = ~0;
  if (no)
    areas = 0;
  else if (! areas)
    areas = Qt::AllDockWidgetAreas;
  if (showSideBar(areas, tabs))
    return ret;
  return false;
}


/* Overloaded version of \a showSideBar for convenience. */

bool
QDjView::showSideBar(QString args)
{
  QStringList errors;
  return showSideBar(args, errors);
}


/*! Overloaded version of \a showSideBar for convenience. */

bool
QDjView::showSideBar(bool show)
{
  thumbnailDock->setVisible(show);
  outlineDock->setVisible(show);
  findDock->setVisible(show);
  return true;
}


/*! Pops up a print dialog */
void
QDjView::print()
{
  QDjViewPrintDialog *pd = printDialog;
  if (! pd)
    {
      printDialog = pd = new QDjViewPrintDialog(this);
      pd->setAttribute(Qt::WA_DeleteOnClose);
      pd->setWindowTitle(tr("Print - DjView", "dialog caption"));
    }
  pd->show();
  pd->raise();
}



/*! Pops up a save dialog */
void
QDjView::saveAs()
{
  QDjViewSaveDialog *sd = saveDialog;
  if (! sd)
    {
      saveDialog = sd = new QDjViewSaveDialog(this);
      sd->setAttribute(Qt::WA_DeleteOnClose);
      sd->setWindowTitle(tr("Save - DjView", "dialog caption"));
    }
  sd->show();
  sd->raise();
}


/*! Pops up export dialog */
void
QDjView::exportAs()
{
  QDjViewExportDialog *sd = exportDialog;
  if (! sd)
    {
      exportDialog = sd = new QDjViewExportDialog(this);
      sd->setAttribute(Qt::WA_DeleteOnClose);
      sd->setWindowTitle(tr("Export - DjView", "dialog caption"));
    }
  sd->show();
  sd->raise();
}



/*! Start searching string \a find. 
    String might be terminated with a slash
    followed by letters "w" (words only),
    "W" (not words only), "c" (case sensitive),
    or "C" (case insentitive). */

void
QDjView::find(QString find)
{
  if (! find.isEmpty())
    {
      QRegExp options("/[wWcCrR]*$");
      if (find.contains(options))
        {
          for (int i=find.lastIndexOf("/"); i<find.size(); i++)
            {
              int c = find[i].toAscii();
              if (c == 'c')
                findWidget->setCaseSensitive(true); 
              else if (c == 'C')
                findWidget->setCaseSensitive(false); 
              else if (c == 'w')
                findWidget->setWordOnly(true); 
              else if (c == 'W')
                findWidget->setWordOnly(false); 
              else if (c == 'r')
                findWidget->setRegExpMode(true); 
              else if (c == 'R')
                findWidget->setRegExpMode(false); 
            }
          find = find.remove(options);
        }
      findWidget->setText(find);
    }
  findWidget->findNext();
}






// -----------------------------------
// UTILITIES


/*! \fn QDjView::getDjVuWidget()
  Return the \a QDjVuWidget object managed by this window. */

/*! \fn QDjView::getErrorDialog()
  Return the error dialog for this window. */

/*! \fn QDjView::getDocument()
  Return the currently displayed \a QDjVuDocument. */

/*! \fn QDjView::getDocumentFileName()
  Return the filename of the currently displayed \a QDjVuDocument. */

/*! \fn QDjView::getDocumentUrl()
  Return the url of the currently displayed \a QDjVuDocument. */

/*! \fn QDjView::getViewerMode
  Return the current viewer mode. */



/*! Return an url that encodes the current view and page. */

QUrl
QDjView::getDecoratedUrl()
{
  QUrl url = removeDjVuCgiArguments(documentUrl);
  QPoint center = widget->rect().center();
  QDjVuWidget::Position pos = widget->positionWithClosestAnchor(center);
  int pageNo = pos.pageNo;
  if (url.isValid() && pageNo>=0 && pageNo<pageNum())
    {
      url.addQueryItem("djvuopts", QString::null);
      QList<ddjvu_fileinfo_t> &dp = documentPages;
      if (pageNo>=0 && pageNo<documentPages.size())
        url.addQueryItem("page", QString::fromUtf8(dp[pageNo].id));
      int rotation = widget->rotation();
      if (rotation)
        url.addQueryItem("rotate", QString::number(90 * rotation));
      int zoom = widget->zoomFactor();
      url.addQueryItem("zoom", QString::number(zoom));
      double ha = pos.hAnchor / 100.0;
      double va = pos.vAnchor / 100.0;
      url.addQueryItem("showposition", QString("%1,%2").arg(ha).arg(va));
    }
  return url;
}


/*! Return the base name of the current file. */

QString
QDjView::getShortFileName()
{
  if (! documentFileName.isEmpty())
    return QFileInfo(documentFileName).fileName();
  else if (! documentUrl.isEmpty())
    return documentUrl.path().section('/', -1);
  return QString();
}


/*! Return true if full screen mode is active */
bool
QDjView::getFullScreen()
{
  if (viewerMode == STANDALONE)
    return actionViewFullScreen->isChecked();
  return false;
}


/*! Return the number of pages in the document.
  This function returns zero when called before
  fully decoding the document header. */

int 
QDjView::pageNum(void)
{
  return documentPages.size();
}


/*! Return a name for page \a pageno. */

QString 
QDjView::pageName(int pageno, bool titleonly)
{
  // obtain page title
  if (pageno>=0 && pageno<documentPages.size())
    if ( documentPages[pageno].title )
      return QString::fromUtf8(documentPages[pageno].title);
  if (titleonly)
    return QString();
  // generate a name from the page number
  //// if (hasNumericalPageTitle)
  ////  return QString("$%1").arg(pageno + 1);
  return QString("%1").arg(pageno + 1);
}


/*! Return a page number for the page named \a name.
  Names of the form the "#+n", "#-n" express
  relative displacement from the current page
  or the page specified by argument \a from.
  Names of the form "#$n" or "$n" express an absolute 
  page number starting from 1.  Other names
  are matched against page titles, page numbers,
  page names, and page ids. */

int
QDjView::pageNumber(QString name, int from)
{
  int pagenum = documentPages.size();
  if (pagenum <= 0)
    return -1;
  // First search an exact page id match
  QByteArray utf8Name = name.toUtf8();
  for (int i=0; i<pagenum; i++)
    if (documentPages[i].id && 
        !strcmp(utf8Name, documentPages[i].id))
      return i;
  // Then interpret the syntaxes +n, -n.
  // Also recognizes $n as an ordinal page number (obsolete)
  if (from < 0)
    from = widget->page();
  if (from < pagenum && 
      name.contains(QRegExp("^[-+$]\\d+$")) )
    {
      int num = name.mid(1).toInt();
      if (name[0]=='+')
        num = from + 1 + num;
      else if (name[0]=='-')
        num = from + 1 - num;
      return qBound(1, num, pagenum) - 1;
    }
  // Then search a matching page title starting 
  // from the current page and wrapping around
  for (int i=from; i<pagenum; i++)
    if (documentPages[i].title && 
        ! strcmp(utf8Name, documentPages[i].title))
      return i;
  for (int i=0; i<from; i++)
    if (documentPages[i].title &&
        ! strcmp(utf8Name, documentPages[i].title))
      return i;
  // Then process a number in range [1..pagenum]
  if (name.contains(QRegExp("^\\d+$")))
    return qBound(1, name.toInt(), pagenum) - 1;
  // Otherwise search page names in the unlikely
  // case they are different from the page ids
  for (int i=0; i<pagenum; i++)
    if (documentPages[i].name && 
        !strcmp(utf8Name, documentPages[i].name))
      return i;
  // Give up
  return -1;
}


/*! Create another main window with the same
  contents, zoom and position as the current one. */

QDjView*
QDjView::copyWindow(bool openDocument)
{
  // update preferences
  if (viewerMode == STANDALONE)
    updateSaved(getSavedPrefs());
  // create new window
  QDjView *other = new QDjView(djvuContext, STANDALONE);
  QDjVuWidget *otherWidget = other->widget;
  other->setAttribute(Qt::WA_DeleteOnClose);
  // copy window geometry
  if (! (windowState() & unusualWindowStates))
    {
      other->resize( size() );
      other->toolBar->setVisible(!toolBar->isHidden());
      other->statusBar->setVisible(!statusBar->isHidden());
      other->restoreState(saveState());
    }
  // copy essential properties 
  otherWidget->setDisplayMode( widget->displayMode() );
  otherWidget->setContinuous( widget->continuous() );
  otherWidget->setSideBySide( widget->sideBySide() );
  otherWidget->setCoverPage( widget->coverPage() );
  otherWidget->setRotation( widget->rotation() );
  otherWidget->setZoom( widget->zoom() );
  // copy document
  if (document && openDocument)
    {
      other->open(document);
      other->documentFileName = documentFileName;
      other->documentUrl = documentUrl;
      // copy position
      otherWidget->setPosition( widget->position() );
    }
  return other;
}


/*! Save \a text info the specified file.
  When argument \a filename is omitted, 
  a file dialog is presented to the user. */

bool 
QDjView::saveTextFile(QString text, QString filename)
{
  // obtain filename
  if (filename.isEmpty())
    {
      QString filters;
      filters += tr("Text files", "save filter")+" (*.txt);;";
      filters += tr("All files", "save filter") + " (*)";
      QString caption = tr("Save Text - DjView", "dialog caption");
      filename = QFileDialog::getSaveFileName(this, caption, "", filters);
      if (filename.isEmpty())
        return false;
    }
  // open file
  errno = 0;
  QFile file(filename);
  if (! file.open(QIODevice::WriteOnly|QIODevice::Truncate))
    {
      QString message = file.errorString();
#if HAVE_STRERROR
      if (file.error() == QFile::OpenError && errno > 0)
        message = strerror(errno);
#endif
      QMessageBox::critical(this, 
                            tr("Error - DjView", "dialog caption"),
                            tr("Cannot write file '%1'.\n%2.")
                            .arg(QFileInfo(filename).fileName())
                            .arg(message) );
      file.remove();
      return false;
    }
  // save text in current locale encoding
  QTextStream(&file) << text;
  return true;
}


/*! Save \a image info the specified file
  using a format derived from the filename suffix.
  When argument \a filename is omitted, 
  a file dialog is presented to the user. */

bool 
QDjView::saveImageFile(QImage image, QString filename)
{
  // obtain filename with suitable suffix
  if (filename.isEmpty())
    {
      QString filters;
      QList<QByteArray> formats = QImageWriter::supportedImageFormats(); 
      foreach(QByteArray format, formats)
        filters += tr("%1 files (*.%2);;", "save image filter")
        .arg(QString(format).toUpper())
        .arg(QString(format).toLower());
      filters += tr("All files", "save filter") + " (*)";
      QString caption = tr("Save Image - DjView", "dialog caption");
      filename = QFileInfo(getShortFileName()).baseName();
      if (formats.size())
        filename += "." + QString(formats[0]);
      filename = QFileDialog::getSaveFileName(this, caption, 
                                              filename, filters);
      if (filename.isEmpty())
        return false;
    }
  // suffix
  QString suffix = QFileInfo(filename).suffix();
  if (suffix.isEmpty())
    {
      QMessageBox::critical(this, 
                            tr("Error - DjView", "dialog caption"),
                            tr("Cannot determine file format.\n"
                               "Filename '%1' has no suffix.")
                            .arg(QFileInfo(filename).fileName()) );
      return false;
    }
  // write
  errno = 0;
  QFile file(filename);
  QImageWriter writer(&file, suffix.toLatin1());
  if (! writer.write(image))
    {
      QString message = file.errorString();
      if (writer.error() == QImageWriter::UnsupportedFormatError)
        message = tr("Image format %1 not supported.").arg(suffix.toUpper());
#if HAVE_STRERROR
      else if (file.error() == QFile::OpenError && errno > 0)
        message = strerror(errno);
#endif
      QMessageBox::critical(this,
                            tr("Error - DjView", "dialog caption"),
                            tr("Cannot write file '%1'.\n%2.")
                            .arg(QFileInfo(filename).fileName())
                            .arg(message) );
      file.remove();
      return false;
    }
  return true;
}


/*! Start the preferred browser on \a url. */

bool
QDjView::startBrowser(QUrl url)
{
  // Determine browsers to try
  QStringList browsers;
#ifdef Q_OS_WIN32
  browsers << "firefox.exe";
  browsers << "iexplore.exe";
#endif
#ifdef Q_WS_MAC
  browsers << "open";
#endif
#ifdef Q_OS_UNIX
  browsers << "x-www-browser" << "firefox" << "konqueror";
  const char *varBrowser = ::getenv("BROWSER");
  if (varBrowser && varBrowser[0])
    browsers = QFile::decodeName(varBrowser).split(":") + browsers;
  browsers << "sensible-browser";
  const char *varPath = ::getenv("PATH");
  QStringList path;
  if (varPath && varPath[0])
    path = QFile::decodeName(varPath).split(":");
  path << ".";
#endif
  if (! prefs->browserProgram.isEmpty())
    browsers.prepend(prefs->browserProgram);
  // Try them
  QStringList args(url.toEncoded());
  foreach (QString browser, browsers)
    {
#ifdef Q_OS_UNIX
      int i;
      for (i=0; i<path.size(); i++)
        if (QFileInfo(QDir(path[i]), browser).exists())
          break;
      if (i >= path.size())
        continue;
#endif
      if (QProcess::startDetached(browser, args))
        return true;
    }
  return false;
}



/*! \fn QDjView::documentClosed(QDjVuDocument *doc)
  This signal is emitted when clearing the current document.
  Argument \a doc is the previous document. */

/*! \fn QDjView::documentOpened(QDjVuDocument *doc)
  This signal is emitted when opening a new document \a doc. */
  
/*! \fn QDjView::documentReady(QDjVuDocument *doc)
  This signal is emitted when the document structure 
  for the current document \a doc is known. */

/*! \fn QDjView::pluginStatusMessage(QString message = QString())
  This signal is emitted when a message is displayed
  in the status bar. This is useful for plugins because
  the message must be replicated in the browser status bar. */

/*! \fn QDjView::pluginGetUrl(QUrl url, QString target)
  This signal is emitted when the user clicks an external
  hyperlink while the viewer is in plugin mode. */



// -----------------------------------
// OVERRIDES


void
QDjView::closeEvent(QCloseEvent *event)
{
  // Close document.
  closeDocument();
  // Save options.
  //  after closing the document in order to 
  //  avoid saving document defined settings.
  updateSaved(getSavedPrefs());
  prefs->thumbnailSize = thumbnailWidget->size();
  prefs->thumbnailSmart = thumbnailWidget->smart();
  prefs->searchWordsOnly = findWidget->wordOnly();
  prefs->searchCaseSensitive = findWidget->caseSensitive();
  prefs->searchRegExpMode = findWidget->regExpMode();
  prefs->save();
  // continue closing the window
  event->accept();
}


void 
QDjView::dragEnterEvent(QDragEnterEvent *event)
{
  if (viewerMode == STANDALONE)
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls() && data->urls().size()==1)
        event->accept();
    }
}


void 
QDjView::dragMoveEvent(QDragMoveEvent *event)
{
#ifdef Q_OS_WIN
  QMainWindow::dragMoveEvent(event);
#else
  QRect rect = centralWidget()->geometry();
  if (event->answerRect().intersects(rect))
    event->accept(rect);
  else
    event->ignore();
#endif
}


void 
QDjView::dropEvent(QDropEvent *event)
{
  if (viewerMode == STANDALONE)
    {
      const QMimeData *data = event->mimeData();
      if (data->hasUrls() && data->urls().size()==1)
        if (open(data->urls()[0]))
          {
            event->setDropAction(Qt::CopyAction);
            event->accept();
          }
    }
}


bool 
QDjView::eventFilter(QObject *watched, QEvent *event)
{
  switch(event->type())
    {
    case QEvent::Show:
    case QEvent::Hide:
      if (qobject_cast<QDockWidget*>(watched))
        updateActionsLater();
      break;
    case QEvent::Leave:
      if ((watched == widget->viewport()) ||
          (qobject_cast<QDockWidget*>(watched)) )
        {
          pageLabel->clear();
          mouseLabel->clear();
          textLabel->clear();
          statusMessage();
          return false;
        }
      break;
    case QEvent::StatusTip:
      if (qobject_cast<QMenu*>(watched))
        return QApplication::sendEvent(this, event);
      break;
    default:
      break;
    }
  return false;
}


// -----------------------------------
// PROTECTED SIGNALS


void 
QDjView::info(QString message)
{
  // just for calling qWarning
  qWarning("INFO: %s", (const char*)message.toLocal8Bit());
}


void 
QDjView::error(QString message, QString filename, int lineno)
{
  // just for calling qWarning
  filename = filename.section("/", -1);
  if (filename.isEmpty())
    qWarning("ERROR: %s", (const char*)message.toLocal8Bit());
  else
    qWarning("ERROR (%s:%d): %s", (const char*)filename.toLocal8Bit(), 
             lineno, (const char*)message.toLocal8Bit() );
}


void 
QDjView::errorCondition(int pageno)
{
  QString message;
  if (pageno >= 0)
    message = tr("Cannot decode page %1.").arg(pageno+1);
  else
    message = tr("Cannot decode document.");
  addToErrorDialog(message);
  raiseErrorDialog(QMessageBox::Warning, tr("Decoding DjVu document"));
}


void 
QDjView::docinfo()
{
  if (document && documentPages.size()==0 &&
      ddjvu_document_decoding_status(*document)==DDJVU_JOB_OK)
    {
      // Obtain information about pages.
      int n = ddjvu_document_get_pagenum(*document);
      int m = ddjvu_document_get_filenum(*document);
      for (int i=0; i<m; i++)
        {
          ddjvu_fileinfo_t info;
          if (ddjvu_document_get_fileinfo(*document, i, &info)!=DDJVU_JOB_OK)
            qWarning("Internal(docinfo): ddjvu_document_get_fileinfo fails.");
          if (info.title && info.name && !strcmp(info.title, info.name))
            info.title = 0;  // clear title if equal to name.
          if (info.type == 'P')
            documentPages << info;
        }
      if (documentPages.size() != n)
        qWarning("Internal(docinfo): inconsistent number of pages.");

      // Check for numerical title
      hasNumericalPageTitle = false;
      QRegExp allNumbers("\\d+");
      for (int i=0; i<n; i++)
        if (documentPages[i].title &&
            allNumbers.exactMatch(QString::fromUtf8(documentPages[i].title)) )
          hasNumericalPageTitle = true;
      
      // Fill page combo
      fillPageCombo(pageCombo);
      
      // Continue processing a bit later
      QTimer::singleShot(0, this, SLOT(docinfoExtra()));
    }
}

void 
QDjView::docinfoExtra()
{
  if (document && documentPages.size()>0)
    {
      performPending();
      updateActionsLater();
      emit documentReady(document);
    }
}


/*! Set options that could not be set before fully decoding
  the document header. For instance, calling \a QDjView::goToPage
  before decoding the djvu document header simply stores
  the page number in a well known variable. Function \a performPending
  peforms the page change as soon as the document information
  is available. */


void
QDjView::performPending()
{
  if (! documentPages.isEmpty())
    {
      if (! pendingPosition.isEmpty())
        {
          if (pendingPosition.size() == 2)
            goToPosition(pendingPage, pendingPosition[0], pendingPosition[1]);
          pendingPosition.clear();
          pendingPage.clear();
        }
      if (! pendingPage.isNull())
        {
          goToPage(pendingPage);
          pendingPage.clear();
        }
      if (pendingHilite.size() > 0)
        {
          StringPair pair;
          foreach(pair, pendingHilite)
            {
              int x, y, w, h;
              QColor color = Qt::blue;
              int pageno = widget->page();
              if (! pair.first.isEmpty())
                pageno = pageNumber(pair.first);
              if (pageno >= 0 && pageno < pageNum() &&
                  parse_highlight(pair.second, x, y, w, h, color) &&
                  w > 0 && h > 0 )
                {
                  color.setAlpha(96);
                  widget->addHighlight(pageno, x, y, w, h, color);
                }
        }
          pendingHilite.clear();
    }
      if (pendingFind.size() > 0)
        {
          find(pendingFind);
          pendingFind.clear();
        }
    }
  performPendingScheduled = false;
}


/*! Schedule a call to \a QDjView::performPending(). */

void
QDjView::performPendingLater()
{
  if (! performPendingScheduled)
    {
      performPendingScheduled = true;
      QTimer::singleShot(0, this, SLOT(performPending()));
    }
}


void 
QDjView::pointerPosition(const Position &pos, const PageInfo &info)
{
  // setup page label
  QString p = "";
  QString m = "";
  if (pos.inPage || !info.segment.isEmpty())
    {
      p = tr(" P%1 %2x%3 %4dpi ")
        .arg(pos.pageNo+1) 
        .arg(info.width).arg(info.height).arg(info.dpi);
      if (info.segment.isEmpty())
        m = tr(" x=%1 y=%2 ")
          .arg(pos.posPage.x())
          .arg(pos.posPage.y());
      else
        m = QString(" %3x%4+%1+%2 ")
          .arg(info.segment.left())
          .arg(info.segment.top())
          .arg(info.segment.width())
          .arg(info.segment.height());
    }
  setPageLabelText(p);
  setMouseLabelText(m);
  if (textLabel->isVisible())
    {
      textLabelRect = info.selected;
      textLabelTimer->start(25);
    }
}


void 
QDjView::updateTextLabel()
{
  textLabel->clear();
  textLabel->setWordWrap(false);
  textLabel->setTextFormat(Qt::PlainText);
  if (textLabel->isVisible())
    {
      QString text;
      QFontMetrics m(textLabel->font());
      QString lb = QString::fromUtf8(" \302\253 ");
      QString rb = QString::fromUtf8(" \302\273 ");
      int w = textLabel->width() - m.width(lb+rb);
      if (! textLabelRect.isEmpty())
        {
          text = widget->getTextForRect(textLabelRect);
          text = text.replace(QRegExp("\\s+"), " ");
          text = m.elidedText(text, Qt::ElideMiddle, w);
        }
      else
        {
          QString results[3];
          if (widget->getTextForPointer(results))
            {
              if (results[0].size() || results[2].size())
                results[1] = "[" + results[1] + "]";
              int r1 = m.width(results[1]);
              int r2 = m.width(results[2]);
              int r0 = qMax(0, qMax( (w-r1)/2, (w-r1-r2) ));
              text = m.elidedText(results[0], Qt::ElideLeft, r0) + results[1];
              text = m.elidedText(text+results[2], Qt::ElideRight, w);
            }
        }
      text = text.trimmed();
      if (text.size())
        textLabel->setText(lb + text + rb);
    }
}


void 
QDjView::pointerEnter(const Position&, miniexp_t)
{
  // Display information message about hyperlink
  QString link = widget->linkUrl();
  if (link.isEmpty())
    return;
  QString target = widget->linkTarget();
  if (target=="_self" || target=="_page")
    target.clear();
  QString message;
  if (link.startsWith("#") &&
      link.contains(QRegExp("^#[-+]\\d+$")) )
    {
      int n = link.mid(2).toInt();
      if (link[1]=='+')
        message = (n==1) ? tr("Go: 1 page forward.") 
          : tr("Go: %n pages forward.", 0, n);
      else
        message = (n==1) ? tr("Go: 1 page backward.")
          : tr("Go: %n pages backward.", 0, n);
    }
  else if (link.startsWith("#$"))
    message = tr("Go: page %1.").arg(link.mid(2));
  else if (link.startsWith("#"))
    message = tr("Go: page %1.").arg(link.mid(1));
  else
    message = tr("Link: %1").arg(link);
  if (!target.isEmpty())
    message = message + tr(" (in other window.)");
  
  statusMessage(message);
}


void 
QDjView::pointerLeave(const Position&, miniexp_t)
{
  statusMessage();
}


void 
QDjView::pointerClick(const Position &pos, miniexp_t)
{
  goToLink(widget->linkUrl(), widget->linkTarget(), pos.pageNo);
}


void  
QDjView::goToLink(QString link, QString target, int fromPage)
{
  bool inPlace = target.isEmpty() || target=="_self" || target=="_page";
  QUrl url = documentUrl;
  // Internal link
  if (link.startsWith("#"))
    {
      QString name = link.mid(1);
      if (inPlace)
        {
          goToPage(name, fromPage);
          return;
        }
      if (viewerMode == STANDALONE)
        {
          QDjView *other = copyWindow();
          other->goToPage(name, fromPage);
          other->show();
          return;
        }
      // Construct url
      QPair<QString,QString> pair;
      QList<QPair<QString, QString> > query;
      foreach(pair, url.queryItems())
        if (pair.first.toLower() != "djvuopts")
          query << pair;
        else
          break;
      url.setQueryItems(query);
      url.addQueryItem("djvuopts", "");
      int pageno = pageNumber(name, fromPage);
      if (pageno>=0 && pageno<=documentPages.size())
        url.addQueryItem("page", QString::fromUtf8(documentPages[pageno].id));
    }
  else
    {
      // Resolve url
      QUrl linkUrl(link);
      QList<QPair<QString, QString> > empty;
      url.setQueryItems(empty);
      url = url.resolved(linkUrl);
      url.setQueryItems(linkUrl.queryItems());
    }
  // Check url
  if (! url.isValid() || url.isRelative())
    {
      QString msg = tr("Cannot resolve link '%1'").arg(link);
      qWarning(msg.toLocal8Bit());
      return;
    }
  // Signal browser
  if (viewerMode != STANDALONE)
    {
      emit pluginGetUrl(url, target);
      return;
    }
  // Standalone can open djvu documents
  QFileInfo file = url.toLocalFile();
  QString suffix = file.suffix().toLower();
  if (file.exists() && (suffix=="djvu" || suffix=="djv"))
    {
      if (inPlace)
        open(url);
      else
        {
          QDjView *other = copyWindow();
          other->open(url);
          other->show();
        }
      return;
    }
  // Open a browser
  if (! startBrowser(url))
    {
      QString msg = tr("Cannot spawn a browser for url '%1'").arg(link);
      qWarning((const char*)msg.toLocal8Bit());
    }
}


void 
QDjView::pointerSelect(const QPoint &pointerPos, const QRect &rect)
{
  // Collect text
  QString text=widget->getTextForRect(rect);
  int l = text.size();
  int w = rect.width();
  int h = rect.height();
  QString s = tr("%n characters", 0, l);
#ifdef Q_WS_X11
  QApplication::clipboard()->setText(text, QClipboard::Selection);
#endif
  // Prepare menu
  QMenu *menu = new QMenu(this);
  QAction *copyText = menu->addAction(tr("Copy text (%1)").arg(s));
  QAction *saveText = menu->addAction(tr("Save text as..."));
  copyText->setEnabled(l>0);
  saveText->setEnabled(l>0);
  copyText->setStatusTip(tr("Copy text into the clipboard."));
  saveText->setStatusTip(tr("Save text into a file."));
  menu->addSeparator();
  QString copyImageString = tr("Copy image (%1x%2 pixels)").arg(w).arg(h);
  QAction *copyImage = menu->addAction(copyImageString);
  QAction *saveImage = menu->addAction(tr("Save image as..."));
  copyImage->setStatusTip(tr("Copy image into the clipboard."));
  saveImage->setStatusTip(tr("Save image into a file."));
  menu->addSeparator();
  QAction *zoom = menu->addAction(tr("Zoom to rectangle"));
  zoom->setStatusTip(tr("Zoom the selection to fit the window."));
  QAction *copyUrl = 0;
  QAction *copyMaparea = 0;
  if (prefs->advancedFeatures)
    {
      menu->addSeparator();
      copyUrl = menu->addAction(tr("Copy URL"));
      copyUrl->setStatusTip(tr("Save into the clipboard an URL that "
                               "highlights the selection."));
      copyMaparea = menu->addAction(tr("Copy Maparea"));
      copyMaparea->setStatusTip(tr("Save into the clipboard a maparea "
                                   "annotation expression for program "
                                   "djvused."));
    }
  
  // Make sure that status tips work (hack)
  menu->installEventFilter(this);

  // Execute menu
  QAction *action = menu->exec(pointerPos-QPoint(5,5), copyText);
  if (action == zoom)
    widget->zoomRect(rect);
  else if (action == copyText)
    QApplication::clipboard()->setText(text);
  else if (action == saveText)
    saveTextFile(text);
  else if (action == copyImage)
    QApplication::clipboard()->setImage(widget->getImageForRect(rect));
  else if (action == saveImage)
    saveImageFile(widget->getImageForRect(rect));
  else if (action && action == copyMaparea)
    {
      Position pos = widget->position(pointerPos);
      QRect seg = widget->getSegmentForRect(rect, pos.pageNo);
      if (! rect.isEmpty())
        {
          QString s = QString("(maparea \"url\"\n"
                              "         \"comment\"\n"
                              "         (rect %1 %2 %3 %4))")
                        .arg(seg.left()).arg(seg.top())
                        .arg(seg.width()).arg(seg.height());
          QApplication::clipboard()->setText(s);
        }
    }
  else if (action && action == copyUrl)
    {
      QUrl url = getDecoratedUrl();
      Position pos = widget->position(pointerPos);
      QRect seg = widget->getSegmentForRect(rect, pos.pageNo);
      if (url.isValid() && pos.pageNo>=0 && pos.pageNo<pageNum())
        {
          if (! url.hasQueryItem("djvuopts"))
            url.addQueryItem("djvuopts", QString::null);
          QList<ddjvu_fileinfo_t> &dp = documentPages;
          if (! url.hasQueryItem("page"))
            if (pos.pageNo>=0 && pos.pageNo<documentPages.size())
              url.addQueryItem("page", QString::fromUtf8(dp[pos.pageNo].id));
          if (! rect.isEmpty())
            url.addQueryItem("highlight", QString("%1,%2,%3,%4")
                             .arg(seg.left()).arg(seg.top())
                             .arg(seg.width()).arg(seg.height()) );
          QApplication::clipboard()->setText(url.toString());
        }
    }
  
  // Cancel select mode.
  updateActionsLater();
  if (actionSelect->isChecked())
    {
      actionSelect->setChecked(false);
      performSelect(false);
    }
}


/*! Schedule a call to \a QDjView::updateActions(). */

void
QDjView::updateActionsLater()
{
  if (! updateActionsScheduled)
    {
      updateActionsScheduled = true;
      QTimer::singleShot(0, this, SLOT(updateActions()));
    }
}


void
QDjView::modeComboActivated(int index)
{
  int mode = modeCombo->itemData(index).toInt();
  widget->setDisplayMode((QDjVuWidget::DisplayMode)mode);
}


void
QDjView::zoomComboActivated(int index)
{
  int zoom = zoomCombo->itemData(index).toInt();
  int oldzoom = widget->zoom();
  widget->setZoom(zoom);
  if (zoom != oldzoom)
    widget->setFocus();
}


void 
QDjView::zoomComboEdited(void)
{
  bool okay;
  QString text = zoomCombo->lineEdit()->text();
  int zoom = text.replace(QRegExp("\\s*%?$"),"").trimmed().toInt(&okay);
  int oldzoom = widget->zoom();
  if (okay && zoom>0)
    widget->setZoom(zoom);
  updateActionsLater();
  if (zoom != oldzoom)
    widget->setFocus();
}


void 
QDjView::pageComboActivated(int index)
{
  int oldpage = widget->page();
  goToPage(pageCombo->itemData(index).toInt());
  updateActionsLater();
  if (oldpage != widget->page())
    widget->setFocus();
}


void 
QDjView::pageComboEdited(void)
{
  int oldpage = widget->page();
  goToPage(pageCombo->lineEdit()->text().trimmed());
  updateActionsLater();
  if (oldpage != widget->page())
    widget->setFocus();
}




// -----------------------------------
// SIGNALS IMPLEMENTING ACTIONS



void
QDjView::performAbout(void)
{
  QString html = 
    tr("<html>"
       "<h2>DjVuLibre DjView %1</h2>"
       "<p>"
       "Viewer for DjVu documents<br>"
       "<a href=http://djvulibre.djvuzone.org>"
       "http://djvulibre.djvuzone.org</a><br>"
       "Copyright \251 2006-- L\351on Bottou."
       "</p>"
       "<p align=justify><small>"
       "This program is free software. "
       "You can redistribute or modify it under the terms of the "
       "GNU General Public License as published "
       "by the Free Software Foundation. "
       "This program is distributed <i>without any warranty</i>. "
       "See the GNU General Public License for more details."
       "</small></p>"
       "</html>")
    .arg(QDjViewPrefs::versionString());

  QMessageBox::about(this, tr("About DjView"), html);
}


void
QDjView::performNew(void)
{
  if (viewerMode != STANDALONE)
    return;
  QDjView *other = copyWindow(false);
  other->show();
}


void
QDjView::performOpen(void)
{
  if (viewerMode != STANDALONE)
    return;
  QString filters;
  filters += tr("DjVu files") + " (*.djvu *.djv);;";
  filters += tr("All files") + " (*)";
  QString caption = tr("Open - DjView", "dialog caption");
  QString dirname = QDir::currentPath();
  QDir dir = QFileInfo(documentFileName).absoluteDir();
  if (dir.exists() && !documentFileName.isEmpty())
    dirname = dir.absolutePath();
  QString fname;
  fname = QFileDialog::getOpenFileName(this, caption, dirname, filters);
  if (! fname.isEmpty())
    open(fname);
}


void
QDjView::performOpenLocation(void)
{
  if (viewerMode != STANDALONE)
    return;
  QString caption = tr("Open Location - DjView", "dialog caption");
  QString label = tr("Enter the URL of a DjVu document.");
  QString text = "http://";
  bool ok;
  QUrl url  = QInputDialog::getText(this, caption, label, 
                                    QLineEdit::Normal, text, &ok);
  if (ok && url.isValid())
	open(url);
}


void 
QDjView::performInformation(void)
{
  if (! documentPages.size())
    return;
  if (! infoDialog)
    infoDialog = new QDjViewInfoDialog(this);
  infoDialog->setWindowTitle(tr("Information - DjView", "dialog caption"));
  infoDialog->setPage(widget->page());
  infoDialog->refresh();
  infoDialog->raise();
  infoDialog->show();
}


void 
QDjView::performMetadata(void)
{
  if (! documentPages.size())
    return;
  if (! metaDialog)
    metaDialog = new QDjViewMetaDialog(this);
  metaDialog->setWindowTitle(tr("Metadata - DjView", "dialog caption"));
  metaDialog->setPage(widget->page());
  metaDialog->refresh();
  metaDialog->raise();
  metaDialog->show();
}


void
QDjView::performPreferences(void)
{
  QDjViewPrefsDialog *dialog = QDjViewPrefsDialog::instance();
  updateSaved(getSavedPrefs());
  dialog->load(this);
  dialog->show();
  dialog->raise();
}


void 
QDjView::performRotation(void)
{
  QAction *action = qobject_cast<QAction*>(sender());
  int rotation = action->data().toInt();
  widget->setRotation(rotation);
}


void 
QDjView::performZoom(void)
{
  QAction *action = qobject_cast<QAction*>(sender());
  int zoom = action->data().toInt();
  widget->setZoom(zoom);
}


void 
QDjView::performSelect(bool checked)
{
  if (checked)
    widget->setModifiersForSelect(Qt::NoModifier);
  else
    widget->setModifiersForSelect(prefs->modifiersForSelect);
}


void 
QDjView::performViewFullScreen(bool checked)
{
  if (viewerMode != STANDALONE)
    return;
  if (checked)
    {
      fsSavedNormal.remember = true;
      updateSaved(&fsSavedNormal);
      updateSaved(&prefs->forStandalone);
      Qt::WindowStates wstate = windowState();
      fsWindowState = wstate;
      wstate &= ~unusualWindowStates;
      wstate |= Qt::WindowFullScreen;
      setWindowState(wstate);
      applySaved(&fsSavedFullScreen);
      // Make sure full screen action remains accessible (F11)
      if (! actions().contains(actionViewFullScreen))
        addAction(actionViewFullScreen);
    }
  else
    {
      fsSavedFullScreen.remember = true;
      updateSaved(&fsSavedFullScreen);
      updateSaved(&prefs->forFullScreen);
      Qt::WindowStates wstate = windowState();
      wstate &= ~unusualWindowStates;
      wstate |= fsWindowState & unusualWindowStates;
      wstate &= ~Qt::WindowFullScreen;
      setWindowState(wstate);
      applySaved(&fsSavedNormal);
      // Demote full screen action
      if (actions().contains(actionViewFullScreen))
        removeAction(actionViewFullScreen);
    }
}


void 
QDjView::performFind()
{
  findDock->show();
  findDock->raise();
  findWidget->takeFocus(Qt::ShortcutFocusReason);
  findWidget->findNext();
  findWidget->selectAll();
}


void 
QDjView::performEscape()
{
  if (actionViewSideBar->isChecked())
    actionViewSideBar->activate(QAction::Trigger);
  else if (actionViewFullScreen->isChecked())
    actionViewFullScreen->activate(QAction::Trigger);
}


void 
QDjView::addRecent(QUrl url)
{
  QString name = url.toString();
  prefs->recentFiles.removeAll(name);
  prefs->recentFiles.prepend(name);
  while(prefs->recentFiles.size() > 6)
    prefs->recentFiles.pop_back();
}


void 
QDjView::fillRecent()
{
  if (recentMenu)
    {
      int n = prefs->recentFiles.size();
      recentMenu->clear();
      for (int i=0; i<n; i++)
        {
          QUrl url = prefs->recentFiles[i];
          QString base = url.path().section("/",-1);
          QString name = url.toLocalFile();
          if (name.isEmpty())
            name = url.toString();
          QFontMetrics metrics = QFont();
          name = metrics.elidedText(name, Qt::ElideMiddle, 400);
          name = QString("%1 [%2]").arg(base).arg(name);
          QAction *action = recentMenu->addAction(name);
          action->setData(url);
          connect(action,SIGNAL(triggered()), this, SLOT(openRecent()));
        }
      recentMenu->addSeparator();
    }
  QAction *action = recentMenu->addAction(tr("&Clear History"));
  connect(action, SIGNAL(triggered()), this, SLOT(clearRecent()));
}


void 
QDjView::openRecent()
{
  QAction *action = qobject_cast<QAction*>(sender());
  if (action && viewerMode == STANDALONE)
    {
      QUrl url = action->data().toUrl();
      QFileInfo file = url.toLocalFile();
      if (file.exists())
        open(file.absoluteFilePath());
      else
        open(url);
    }
}


void 
QDjView::clearRecent()
{
  prefs->recentFiles.clear();
  prefs->save();
}


QDjView::UndoRedo::UndoRedo()
  : valid(false)
{
}


void
QDjView::UndoRedo::clear()
{
  valid = false;
}


void
QDjView::UndoRedo::set(QDjView *djview)
{
  QDjVuWidget *djvu = djview->getDjVuWidget();
  rotation = djvu->rotation();
  zoom = djvu->zoom();
  hotSpot = djvu->hotSpot();
  position = djvu->position(hotSpot);
  valid = true;
}


void 
QDjView::UndoRedo::apply(QDjView *djview)
{
  if (valid)
    {
      QDjVuWidget *djvu = djview->getDjVuWidget();
      djvu->setZoom(zoom);
      djvu->setRotation(rotation);
      djvu->setPosition(position, hotSpot);
    }
}


bool 
QDjView::UndoRedo::changed(const QDjVuWidget *djvu) const
{
  if (valid)
    {
      if (zoom != djvu->zoom() || 
          rotation != djvu->rotation())
        return true;
      Position curpos = djvu->position(hotSpot);
      if (curpos.pageNo != position.pageNo ||
          curpos.inPage != position.inPage)
        return true;
      if (curpos.inPage)
        return (curpos.posPage != position.posPage);
      else
        return (curpos.posView != position.posView);
    }
  return false;
}


void
QDjView::saveUndoData()
{
  if (QApplication::mouseButtons() == Qt::NoButton
      && widget->pageSizeKnown(widget->page()) )
    {
      if (here.changed(widget))
        {
          undoList.prepend(here);
          while (undoList.size() > 1024)
            undoList.removeLast();
          redoList.clear();
        }
      here.set(this);
      actionBack->setEnabled(undoList.size() > 0);
      actionForw->setEnabled(redoList.size() > 0);
    }
  else
    {
      undoTimer->stop();
      undoTimer->start(250);
    }
}


void 
QDjView::performUndo()
{
  if (undoList.size() > 0)
    {
      UndoRedo target = undoList.takeFirst();
      UndoRedo saved;
      saved.set(this);
      target.apply(this);
      here.clear();
      redoList.prepend(saved);
    }
}


void 
QDjView::performRedo()
{
  if (redoList.size() > 0)
    {
      UndoRedo target = redoList.takeFirst();
      UndoRedo saved;
      saved.set(this);
      target.apply(this);
      here.clear();
      undoList.prepend(saved);
    }
}


void 
QDjView::performCopyUrl()
{
  QUrl url = getDecoratedUrl();
  if (url.isValid())
    QApplication::clipboard()->setText(url.toString());
}


static QByteArray *qstring_puts_data = 0;

static int 
qstring_puts(const char *s)
{
  if (qstring_puts_data)
    (*qstring_puts_data) += s;
  return strlen(s);
}

static QString
miniexp_to_string(miniexp_t expr, int width=40, bool octal=false)
{
  QByteArray buffer;
  qstring_puts_data = &buffer;
  int (*saved_puts)(const char*) = minilisp_puts;
  int saved_print_7bits = minilisp_print_7bits;
  minilisp_puts = qstring_puts;
  minilisp_print_7bits = (octal) ? 1 : 0;
  miniexp_pprint(expr, width);
  minilisp_print_7bits = saved_print_7bits;
  minilisp_puts = saved_puts;
  return QString::fromUtf8(buffer.data());
}


void 
QDjView::performCopyOutline()
{
  if (document)
    {
      QString s;
      minivar_t expr = document->getDocumentOutline();
      if (miniexp_consp(expr))
        s += QString("# This is the existing outline.\n");
      else {
        s += QString("# This is an outline template with all pages.\n");
        expr = miniexp_cons(miniexp_symbol("bookmarks"),miniexp_nil);
        for (int i=0; i<documentPages.size(); i++)
          {
            minivar_t p = miniexp_nil;
            QByteArray ref = documentPages[i].id;
            p = miniexp_cons(miniexp_string(ref.prepend("#").constData()),p);
            QString name = QString("Page %1").arg(pageName(i));
            p = miniexp_cons(miniexp_string(name.toUtf8().constData()),p);
            expr = miniexp_cons(p,expr);
          }
        expr = miniexp_reverse(expr);
      }
      s += QString("# Edit it and store it with command:\n"
                   "#   $ djvused foo.djvu -f thisfile -s\n"
                   "# The following line is the djvused command\n"
                   "# to set the outline and the rest is the outline\n"
                   "set-outline\n\n");
      s += miniexp_to_string(expr);
      // copy to clipboard
      QApplication::clipboard()->setText(s);
    }
}


void 
QDjView::performCopyAnnotation()
{
  int pageNo = widget->page();
  if (document && pageNo>=0 && pageNo<pageNum())
    {
      QString s;
      miniexp_t expr = document->getPageAnnotations(pageNo);
      if (expr == miniexp_nil || expr == miniexp_dummy)
        s += QString("# There were no annotations for page %1.\n");
      else
        s += QString("# These are the annotation for page %1.\n");
      s += QString("# Edit this file and store it with command:\n"
                   "#   $ djvused foo.djvu -f thisfile -s\n"
                   "# Tip: select an area in djview4 and use 'copy maparea'.\n"
                   "# The following line is the djvused command to set\n"
                   "# the annotation and the rest are the annotations\n"
                   "select %2; set-ant\n\n");
      s = s.arg(pageNo+1).arg(pageNo+1);
      while (miniexp_consp(expr))
        {
          s += miniexp_to_string(miniexp_car(expr));
          expr = miniexp_cdr(expr);
        }
      // copy to clipboard
      QApplication::clipboard()->setText(s);
    }
}



/* -------------------------------------------------------------
   Local Variables:
   c++-font-lock-extra-types: ( "\\sw+_t" "[A-Z]\\sw*[a-z]\\sw*" )
   End:
   ------------------------------------------------------------- */
