#include <CQChartsViewToolBar.h>
#include <CQChartsWindow.h>
#include <CQChartsView.h>
#include <CQChartsPlot.h>
#include <CQChartsModelData.h>
#include <CQCharts.h>

#include <CQChartsCreatePlotDlg.h>
#include <CQChartsCmdLine.h>
#include <CQChartsHelpDlg.h>
#include <CQChartsWidgetUtil.h>

#include <CQPixmapCache.h>
#include <CQIconButton.h>
#include <CQIconCombo.h>
#include <CQAppOptions.h>
#include <CQUtil.h>

#include <QStackedWidget>
#include <QPushButton>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QHBoxLayout>

CQChartsViewToolBar::
CQChartsViewToolBar(CQChartsWindow *window) :
 QFrame(window), window_(window)
{
  setObjectName("toolbar");

  setAutoFillBackground(true);

  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  //---

  auto *layout = CQUtil::makeLayout<QHBoxLayout>(this, 0, 2);

  //---

  auto createIconButton = [&](const QString &name, const QString &iconName, const QString &tip,
                              const char *receiver, bool checkable=false, bool checked=false) {
    auto *button = CQUtil::makeWidget<CQIconButton>(name);

    button->setSize(CQIconButton::Size::LARGE);
    button->setIcon(iconName);

    if (checkable) {
      button->setCheckable(true);
      button->setChecked(checked);
    }

    button->setFocusPolicy(Qt::NoFocus);

    connect(button, SIGNAL(clicked(bool)), this, receiver);

    button->setToolTip(tip);

    button->setFocusPolicy(Qt::NoFocus);

    return button;
  };

  auto createCheckedButton = [&](const QString &name, const QString &iconName, const QString &tip,
                                 const char *receiver, bool checked=false) {
    return createIconButton(name, iconName, tip, receiver, true, checked);
  };

  //---

  modeCombo_ = CQUtil::makeWidget<CQIconCombo>("modeCombo");

  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("SELECT"  ), "Select"  );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("ZOOM_IN" ), "Zoom In" );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("ZOOM_OUT"), "Zoom Out");
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("PAN"     ), "Pan"     );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("PROBE"   ), "Probe"   );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("QUERY"   ), "Query"   );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("EDIT"    ), "Edit"    );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("REGION"  ), "Region"  );
  modeCombo_->addItem(CQPixmapCacheInst->getLightDarkIcon("RULER"   ), "Ruler"   );

  modeCombo_->setFocusPolicy(Qt::NoFocus);

  connect(modeCombo_, SIGNAL(currentIndexChanged(int)), this, SLOT(modeSlot(int)));

  layout->addWidget(modeCombo_);

  //---

  controlsStack_ = CQUtil::makeWidget<QStackedWidget>("controls");

  controlsStack_->layout()->setMargin(0); controlsStack_->layout()->setSpacing(0);

  layout->addWidget(controlsStack_);

  auto *selectControls  = CQUtil::makeWidget<QFrame>("select");
  auto *zoomInControls  = CQUtil::makeWidget<QFrame>("zoom_in");
  auto *zoomOutControls = CQUtil::makeWidget<QFrame>("zoom_out");
  auto *panControls     = CQUtil::makeWidget<QFrame>("pan");
  auto *probeControls   = CQUtil::makeWidget<QFrame>("probe");
  auto *queryControls   = CQUtil::makeWidget<QFrame>("query");
  auto *editControls    = CQUtil::makeWidget<QFrame>("edit");
  auto *regionControls  = CQUtil::makeWidget<QFrame>("region");
  auto *rulerControls   = CQUtil::makeWidget<QFrame>("ruler");

  controlsStack_->addWidget(selectControls);
  controlsStack_->addWidget(zoomInControls);
  controlsStack_->addWidget(zoomOutControls);
  controlsStack_->addWidget(panControls);
  controlsStack_->addWidget(probeControls);
  controlsStack_->addWidget(queryControls);
  controlsStack_->addWidget(editControls);
  controlsStack_->addWidget(regionControls);
  controlsStack_->addWidget(rulerControls);

  layout->addStretch(1);

  //-----

  auto makePushButton = [&](const QString &label, const QString &name, const QString &tip,
                            const char *slotName) {
    auto *button = CQUtil::makeLabelWidget<QPushButton>(label, name);

    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(tip);

    connect(button, SIGNAL(clicked()), this, slotName);

    return button;
  };

  auto makeRadioButton = [&](const QString &label, const QString &name, const QString &tip) {
    auto *button = CQUtil::makeLabelWidget<QRadioButton>(label, name);

    button->setFocusPolicy(Qt::NoFocus);
    button->setToolTip(tip);

    return button;
  };

  auto makeCheckBox = [&](const QString &label, const QString &name, bool isChecked,
                          const QString &tip, const char *slotName) {
    auto *check = CQUtil::makeLabelWidget<QCheckBox>(label, name);

    check->setFocusPolicy(Qt::NoFocus);
    check->setToolTip(tip);

    check->setChecked(isChecked);

    connect(check, SIGNAL(stateChanged(int)), this, slotName);

    return check;
  };

  //---

  // TODO: add smart
  auto *selectControlsLayout = CQUtil::makeLayout<QHBoxLayout>(selectControls, 0, 2);

  selectPointButton_ = makeRadioButton("Point", "point", "Select objects at point");
  selectRectButton_  = makeRadioButton("Rect" , "rect" ,
                         "Select objects inside/touching rectangle");

  if (view()->selectMode() == CQChartsView::SelectMode::POINT)
    selectPointButton_->setChecked(true);
  else
    selectRectButton_ ->setChecked(true);

  auto *selectButtonGroup =
    CQUtil::makeButtonGroup(this, {selectPointButton_, selectRectButton_});

  selectControlsLayout->addWidget(selectPointButton_);
  selectControlsLayout->addWidget(selectRectButton_ );

  connect(selectButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(selectButtonClicked(int)));

  //--

  selectInsideCheck_ = makeCheckBox("Inside", "selectInside", view()->isSelectInside(),
    "Rectangle intersect inside (checked) or touching (unchecked)",
    SLOT(selectInsideSlot(int)));

  selectControlsLayout->addWidget(selectInsideCheck_);

  //--

  selectControlsLayout->addWidget(CQChartsWidgetUtil::createHSpacer(1));

  //---

  auto *selecyKeyLabel = CQUtil::makeLabelWidget<QLabel>("Key", "selecyKeyLabel");

  selectKeyCombo_ = CQUtil::makeWidget<QComboBox>("selectKeyCombo");

  selectKeyCombo_->addItem("Show");
  selectKeyCombo_->addItem("Select");

  connect(selectKeyCombo_, SIGNAL(currentIndexChanged(int)),
          this, SLOT(selectKeyComboSlot(int)));

  selectControlsLayout->addWidget(selecyKeyLabel);
  selectControlsLayout->addWidget(selectKeyCombo_);

  //--

  selectControlsLayout->addStretch(1);

  //-----

  auto *zoomInControlsLayout = CQUtil::makeLayout<QHBoxLayout>(zoomInControls, 0, 2);

  auto *zoomResetButton = makePushButton("Reset", "reset", "Reset Zoom", SLOT(zoomFullSlot()));

  zoomInControlsLayout->addWidget(zoomResetButton);

  zoomInControlsLayout->addStretch(1);

  //-----

  auto *panControlsLayout = CQUtil::makeLayout<QHBoxLayout>(panControls, 0, 2);

  auto *panButton = makePushButton("Reset", "reset", "Reset Pan", SLOT(panResetSlot()));

  panControlsLayout->addWidget(panButton);

  panControlsLayout->addStretch(1);

  //-----

#if 0
  auto *queryControlsLayout = CQUtil::makeLayout<QHBoxLayout>(queryControls, 0, 2);
#endif

  //-----

  auto *editControlsLayout = CQUtil::makeLayout<QHBoxLayout>(editControls, 0, 2);

  auto *flipHButton = makePushButton("Flip H", "fliph", "Flip Horizontal", SLOT(flipHSlot()));
  auto *flipVButton = makePushButton("Flip V", "flipv", "Flip Vertical"  , SLOT(flipVSlot()));

  editControlsLayout->addWidget(flipHButton);
  editControlsLayout->addWidget(flipVButton);

  editControlsLayout->addStretch(1);

  //-----

  auto *regionControlsLayout = CQUtil::makeLayout<QHBoxLayout>(regionControls, 0, 2);

  regionPointButton_ = makeRadioButton("Point", "point", "Define region at point");
  regionRectButton_  = makeRadioButton("Rect" , "rect" , "Define region rectangle");

  if (view()->regionMode() == CQChartsView::RegionMode::POINT)
    regionPointButton_->setChecked(true);
  else
    regionRectButton_ ->setChecked(true);

  auto *regionButtonGroup =
    CQUtil::makeButtonGroup(this, {regionPointButton_, regionRectButton_});

  regionControlsLayout->addWidget(regionPointButton_);
  regionControlsLayout->addWidget(regionRectButton_ );

  connect(regionButtonGroup, SIGNAL(buttonClicked(int)), this, SLOT(regionButtonClicked(int)));

  connect(view(), SIGNAL(regionModeChanged()), this, SLOT(updateState()));

  regionControlsLayout->addStretch(1);

  //-----

  auto *rulerControlsLayout = CQUtil::makeLayout<QHBoxLayout>(rulerControls, 0, 2);

  auto *rulerClearButton = makePushButton("Clear", "clear", "Clear Ruler", SLOT(clearRulerSlot()));

  rulerControlsLayout->addWidget(rulerClearButton);

  rulerControlsLayout->addStretch(1);

  //-----

  appOptionssButton_ = createIconButton("options", "SETTINGS", "Show/Hide App Options",
                                        SLOT(appOptionsSlot()));

  layout->addWidget(appOptionssButton_);

  viewSettingsButton_ = createCheckedButton("settings", "OPTIONS", "Show/Hide View Settings",
                                            SLOT(viewSettingsSlot(bool)), true);

  layout->addWidget(viewSettingsButton_);

  //-----

  layout->addWidget(CQChartsWidgetUtil::createHSpacer(1));

  //---

  bool showTable = (window_ && window_->isDataTable());

  viewTableButton_ = createCheckedButton("table", "TABLE", "Show/Hide View Table",
                                         SLOT(viewTableSlot(bool)), showTable);

  layout->addWidget(viewTableButton_);

  //-----

  manageModelsDlgButton_ =
    createIconButton("modelDlg", "MODELS", "Manage Models", SLOT(manageModelsSlot()));
  createPlotDlgButton_ =
    createIconButton("plotDlg" , "CHARTS", "Add Plot", SLOT(addPlotSlot()));

  layout->addWidget(manageModelsDlgButton_);
  layout->addWidget(createPlotDlgButton_);

  autoFitButton_ = createIconButton("fit", "ZOOM_FIT", "Zoom Fit", SLOT(autoFitSlot()));

  layout->addWidget(autoFitButton_);

  //---

  leftButton_  = createIconButton("left" , "LEFT"    , "Scroll Left" , SLOT(leftSlot()));
  rightButton_ = createIconButton("right", "RIGHT"   , "Scroll Right", SLOT(rightSlot()));

  layout->addWidget(leftButton_);
  layout->addWidget(rightButton_);

  //---

  consoleButton_ = createIconButton("console", "CONSOLE", "Show/Hide Console",
                                    SLOT(consoleSlot()));

  layout->addWidget(consoleButton_);

  //---

  auto *helpButton = createIconButton("help", "INFO", "Help" , SLOT(helpSlot()));

  layout->addWidget(helpButton);

  //---

  connect(view(), SIGNAL(plotAdded(const QString &)), this, SLOT(updateState()));

  connect(view(), SIGNAL(scrollDataChanged()), this, SLOT(updateState()));

  //---

  updateState();
}

CQChartsView *
CQChartsViewToolBar::
view() const
{
  return window()->view();
}

void
CQChartsViewToolBar::
updateState()
{
  bool scrolled = view()->isScrolled();

  int numScrolled = (scrolled ? view()->numPlots() : 1);

  leftButton_ ->setVisible(numScrolled > 1);
  rightButton_->setVisible(numScrolled > 1);

  if (numScrolled > 1) {
    leftButton_ ->setEnabled(view()->scrollPage() > 0);
    rightButton_->setEnabled(view()->scrollPage() < view()->scrollNumPages() - 1);
  }

  if      (view()->regionMode() == CQChartsView::RegionMode::POINT)
    regionPointButton_->setChecked(true);
  else if (view()->regionMode() == CQChartsView::RegionMode::RECT)
    regionRectButton_->setChecked(true);
}

void
CQChartsViewToolBar::
modeSlot(int ind)
{
  if      (ind == 0) view()->setMode(CQChartsView::Mode::SELECT);
  else if (ind == 1) view()->setMode(CQChartsView::Mode::ZOOM_IN);
  else if (ind == 2) view()->setMode(CQChartsView::Mode::ZOOM_OUT);
  else if (ind == 3) view()->setMode(CQChartsView::Mode::PAN);
  else if (ind == 4) view()->setMode(CQChartsView::Mode::PROBE);
  else if (ind == 5) view()->setMode(CQChartsView::Mode::QUERY);
  else if (ind == 6) view()->setMode(CQChartsView::Mode::EDIT);
  else if (ind == 7) view()->setMode(CQChartsView::Mode::REGION);
  else if (ind == 8) view()->setMode(CQChartsView::Mode::RULER);

  updateMode();
}

//---

void
CQChartsViewToolBar::
selectButtonClicked(int ind)
{
  if (ind == 0)
    view()->setSelectMode(CQChartsView::SelectMode::POINT);
  else
    view()->setSelectMode(CQChartsView::SelectMode::RECT);
}

void
CQChartsViewToolBar::
selectInsideSlot(int state)
{
  view()->setSelectInside(state);
}

void
CQChartsViewToolBar::
selectKeyComboSlot(int ind)
{
  if      (ind == 0)
    view()->setKeyBehavior(CQChartsView::KeyBehavior(CQChartsView::KeyBehavior::Type::SHOW));
  else if (ind == 1)
    view()->setKeyBehavior(CQChartsView::KeyBehavior(CQChartsView::KeyBehavior::Type::SELECT));
}

//---

void
CQChartsViewToolBar::
zoomFullSlot()
{
  auto *plot = view()->currentPlot(/*remap*/true);
  if (! plot) return;

  plot->zoomFull();
}

//---

void
CQChartsViewToolBar::
panResetSlot()
{
}

//---

void
CQChartsViewToolBar::
flipHSlot()
{
  auto *plot = view()->currentPlot(/*remap*/true);
  if (! plot) return;

  plot->flipSelected(Qt::Horizontal);
}

void
CQChartsViewToolBar::
flipVSlot()
{
  auto *plot = view()->currentPlot(/*remap*/true);
  if (! plot) return;

  plot->flipSelected(Qt::Vertical);
}

//---

void
CQChartsViewToolBar::
regionButtonClicked(int ind)
{
  if (ind == 0)
    view()->setRegionMode(CQChartsView::RegionMode::POINT);
  else
    view()->setRegionMode(CQChartsView::RegionMode::RECT);
}

void
CQChartsViewToolBar::
clearRulerSlot()
{
  view()->clearRulerSlot();
}

//---

void
CQChartsViewToolBar::
updateMode()
{
  if      (view()->mode() == CQChartsView::Mode::SELECT) {
    modeCombo_    ->setCurrentIndex(0);
    controlsStack_->setCurrentIndex(0);

    if (view()->selectMode() == CQChartsView::SelectMode::POINT)
      selectPointButton_->setChecked(true);
    else
      selectRectButton_->setChecked(true);
  }
  else if (view()->mode() == CQChartsView::Mode::ZOOM_IN) {
    modeCombo_    ->setCurrentIndex(1);
    controlsStack_->setCurrentIndex(1);
  }
  else if (view()->mode() == CQChartsView::Mode::ZOOM_OUT) {
    modeCombo_    ->setCurrentIndex(2);
    controlsStack_->setCurrentIndex(2);
  }
  else if (view()->mode() == CQChartsView::Mode::PAN) {
    modeCombo_    ->setCurrentIndex(3);
    controlsStack_->setCurrentIndex(3);
  }
  else if (view()->mode() == CQChartsView::Mode::PROBE) {
    modeCombo_    ->setCurrentIndex(4);
    controlsStack_->setCurrentIndex(4);
  }
  else if (view()->mode() == CQChartsView::Mode::QUERY) {
    modeCombo_    ->setCurrentIndex(5);
    controlsStack_->setCurrentIndex(5);
  }
  else if (view()->mode() == CQChartsView::Mode::EDIT) {
    modeCombo_    ->setCurrentIndex(6);
    controlsStack_->setCurrentIndex(6);
  }
  else if (view()->mode() == CQChartsView::Mode::REGION) {
    modeCombo_    ->setCurrentIndex(7);
    controlsStack_->setCurrentIndex(7);
  }
  else if (view()->mode() == CQChartsView::Mode::RULER) {
    modeCombo_    ->setCurrentIndex(8);
    controlsStack_->setCurrentIndex(8);
  }
}

void
CQChartsViewToolBar::
appOptionsSlot()
{
  CQAppOptions::show();
}

void
CQChartsViewToolBar::
viewSettingsSlot(bool b)
{
  if (window())
    window()->setViewSettings(b);
}

void
CQChartsViewToolBar::
viewTableSlot(bool b)
{
  if (window())
    window()->setDataTable(b);
}

void
CQChartsViewToolBar::
manageModelsSlot()
{
  auto *charts = view()->charts();

  (void) charts->manageModelsDlg();
}

void
CQChartsViewToolBar::
addPlotSlot()
{
  auto *charts = view()->charts();

  auto *modelData = charts->currentModelData();

  if (! modelData)
    return;

  auto *createPlotDlg = charts->createPlotDlg(modelData);

  createPlotDlg->setViewName(view()->id());
}

void
CQChartsViewToolBar::
autoFitSlot()
{
  view()->fitSlot();
}

void
CQChartsViewToolBar::
leftSlot()
{
  view()->scrollLeft();
}

void
CQChartsViewToolBar::
rightSlot()
{
  view()->scrollRight();
}

void
CQChartsViewToolBar::
consoleSlot()
{
  auto *charts = view()->charts();

  CQChartsCmdLineDlgMgrInst->showDialog(charts);
}

void
CQChartsViewToolBar::
helpSlot()
{
  auto *charts = view()->charts();

  CQChartsHelpDlgMgrInst->showDialog(charts);
}

QSize
CQChartsViewToolBar::
sizeHint() const
{
  int w = autoFitButton_->sizeHint().width ();
  int h = autoFitButton_->sizeHint().height();

  return QSize(4*w + 6, h);
}
