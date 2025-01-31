#include <CQChartsCreatePlotDlg.h>
#include <CQChartsPlotType.h>
#include <CQChartsWindow.h>
#include <CQChartsView.h>
#include <CQChartsPlot.h>
#include <CQChartsPlotParameter.h>
#include <CQChartsAxis.h>
#include <CQChartsVariant.h>
#include <CQChartsModelUtil.h>
#include <CQChartsColumnEdit.h>
#include <CQChartsModelViewHolder.h>
#include <CQChartsModelData.h>
#include <CQChartsModelDetailsWidget.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelChooser.h>
#include <CQChartsAnalyzeModel.h>
#include <CQChartsPreviewPlot.h>
#include <CQChartsPlotParameterEdit.h>
#include <CQChartsWidgetUtil.h>
#include <CQChartsOptRealEdit.h>
#include <CQChartsLineEdit.h>
#include <CQCharts.h>

#include <CQSummaryModel.h>
#include <CQTabSplit.h>
#include <CQIntegerSpin.h>
#include <CQRealSpin.h>
#include <CQUtil.h>
#include <CQCustomCombo.h>
#include <CQGroupBox.h>
#include <CQIconButton.h>
#include <CQIconLabel.h>
#include <CQTabWidget.h>
#include <CQTclUtil.h>
#include <CQPixmapCache.h>

#include <QGridLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QStackedWidget>
#include <QTextEdit>
#include <QCheckBox>
#include <cassert>

CQChartsCreatePlotDlg::
CQChartsCreatePlotDlg(CQCharts *charts, CQChartsModelData *modelData) :
 QDialog(nullptr), charts_(charts), modelData_(modelData)
{
  assert(modelData_);

  model_ = modelData_->currentModel();

  setModal(false);

  init();
}

CQChartsCreatePlotDlg::
~CQChartsCreatePlotDlg()
{
}

void
CQChartsCreatePlotDlg::
init()
{
  setObjectName("plotDlg");

  setWindowTitle(QString("Create Plot (Model %1)").arg(modelData_->ind()));
//setWindowIcon(QIcon()); TODO

  //----

  auto *layout = CQUtil::makeLayout<QVBoxLayout>(this, 0, 2);

  //----

  auto *modelChooserFrame  = CQUtil::makeWidget<QFrame>("modelChooserFrame");
  auto *modelChooserLayout = CQUtil::makeLayout<QHBoxLayout>(modelChooserFrame, 2, 2);

  modelChooser_ = new CQChartsModelChooser(charts_);

  connect(modelChooser_, SIGNAL(currentModelChanged()), this, SLOT(modelChangeSlot()));

  modelChooserLayout->addWidget (CQUtil::makeLabelWidget<QLabel>("Model", "modelLabel"));
  modelChooserLayout->addWidget (modelChooser_);
  modelChooserLayout->addStretch(1);

  layout->addWidget(modelChooserFrame);

  //----

  auto *area = CQUtil::makeWidget<CQTabSplit>("area");

  area->setOrientation(Qt::Vertical);
  area->setGrouped(true);
  area->setState(CQTabSplit::State::VSPLIT);

  layout->addWidget(area);

  //----

  // add data frame
  auto *dataFrame = createDataFrame();

  area->addWidget(dataFrame, "Plot Data");

  //----

  auto *dataArea = CQUtil::makeWidget<CQTabSplit>("dataArea");

  dataArea->setOrientation(Qt::Vertical);
  dataArea->setGrouped(true);
  dataArea->setState(CQTabSplit::State::TAB);

  area->addWidget(dataArea, "Data/Preview");

  //---

  // add summary frame
  auto *summaryFrame = createSummaryFrame();

  dataArea->addWidget(summaryFrame, "Model Data");

  //----

  // add details frame
  auto *detailsFrame = createDetailsFrame();

  dataArea->addWidget(detailsFrame, "Model Details");

  //----

  // add preview frame
  auto *previewFrame = createPreviewFrame();

  dataArea->addWidget(previewFrame, "Plot Preview");

  //----

  //auto *sep1 = createSep("sep1");

  //layout->addWidget(sep1);

  //----

  msgLabel_ = CQUtil::makeLabelWidget<QLabel>("", "msgLabel");

  layout->addWidget(msgLabel_);

  //----

  //auto *sep2 = createSep("sep2");

  //layout->addWidget(sep2);

  //-------

  // OK, Apply, Cancel Buttons
  auto *buttons = new CQChartsDialogButtons(this);

  buttons->connect(this, SLOT(okSlot()), SLOT(applySlot()), SLOT(cancelSlot()));

  buttons->setToolTips("Create plot and close dialog",
                       "Create plot and keep dialog open",
                       "Close dialog without creating plot");

  okButton_    = buttons->okButton();
  applyButton_ = buttons->applyButton();

  layout->addWidget(buttons);

  //--

  connectSlots(true);

  initialized_ = true;

  //---

  updateModelData();

  validateSlot();
}

QFrame *
CQChartsCreatePlotDlg::
createDataFrame()
{
  auto *dataFrame  = CQUtil::makeWidget<QFrame>("data");
  auto *dataLayout = CQUtil::makeLayout<QVBoxLayout>(dataFrame, 2, 2);

  //----

  auto *typeLayout = CQUtil::makeLayout<QHBoxLayout>(dataLayout, 2, 2);

  // create type combo
  auto *typeComboFrame = createTypeCombo();

  typeLayout->addWidget(typeComboFrame);

  //--

  // advanced controls
  auto *advancedCheck = CQUtil::makeLabelWidget<QCheckBox>("Advanced", "advancedCheck");

  advancedCheck->setToolTip("Show advanced plot options");

  connect(advancedCheck, SIGNAL(stateChanged(int)), this, SLOT(advancedSlot(int)));

  typeLayout->addWidget(advancedCheck);

  // auto analyze model data
  auto *autoAnalyzeCheck =
    CQUtil::makeLabelWidget<QCheckBox>("Auto Analyze", "autoAnalyzeCheck");

  autoAnalyzeCheck->setChecked(isAutoAnalyzeModel());
  autoAnalyzeCheck->setToolTip("Automatically set columns from model data");

  connect(autoAnalyzeCheck, SIGNAL(stateChanged(int)), this, SLOT(autoAnalyzeSlot(int)));

  typeLayout->addWidget(autoAnalyzeCheck);

  //----

  auto *dataTab = CQUtil::makeWidget<CQTabWidget>("dataTab");

  auto *typeFrame = createTypeDataFrame();

  dataTab->addTab(typeFrame, "Type");

  auto *genFrame = createGeneralDataFrame();

  dataTab->addTab(genFrame, "General");

  //---

  dataLayout->addWidget(dataTab);

  return dataFrame;
}

QFrame *
CQChartsCreatePlotDlg::
createTypeDataFrame()
{
  auto *typeFrame  = CQUtil::makeWidget<QFrame>("type");
  auto *typeLayout = CQUtil::makeLayout<QVBoxLayout>(typeFrame, 2, 2);

  //----

  // get plot types
  CQCharts::PlotTypes plotTypes;

  sortedPlotTypes(plotTypes);

  auto *typeTab = CQUtil::makeWidget<CQTabWidget>("typeTab");

  typeLayout->addWidget(typeTab);

  // plot type widgets in stack (one per type)
  auto *plotDataFrame  = CQUtil::makeWidget<QFrame>("plotDataFrame");
  auto *plotDataLayout = CQUtil::makeLayout<QVBoxLayout>(plotDataFrame, 2, 2);

  plotDataStack_ = CQUtil::makeWidget<QStackedWidget>("stack");

  plotDataLayout->addWidget(plotDataStack_);

  basicPlotDataStack_    = CQUtil::makeWidget<QStackedWidget>("basicStack");
  advancedPlotDataStack_ = CQUtil::makeWidget<QStackedWidget>("advancedStack");

  for (std::size_t i = 0; i < plotTypes.size(); ++i)
    addPlotWidgets(plotTypes[i], int(i));

  plotDataStack_->addWidget(basicPlotDataStack_   );
  plotDataStack_->addWidget(advancedPlotDataStack_);

  typeTab->addTab(plotDataFrame, "Input Data");

  //---

  // plot type description
  descText_ = CQUtil::makeWidget<QTextEdit>("descText");

  typeTab->addTab(descText_, "Description");

  //----

  // where filter edit
  whereFrame_ = CQUtil::makeWidget<QFrame>("whereFrame");

  auto *whereLayout = CQUtil::makeLayout<QHBoxLayout>(whereFrame_, 0, 2);

  typeLayout->addWidget(whereFrame_);

  auto *whereLabel = CQUtil::makeLabelWidget<QLabel>("Where", "whereLabel");

  whereLayout->addWidget(whereLabel);

  whereEdit_ = CQUtil::makeWidget<CQChartsLineEdit>("whereEdit");

  whereEdit_->setToolTip("Filter for input data");

  whereLayout->addWidget(whereEdit_);

  whereLayout->addStretch(1);

  whereFrame_->setVisible(isAdvanced());

  //---

  return typeFrame;
}

QFrame *
CQChartsCreatePlotDlg::
createTypeCombo()
{
  // type combo
  auto *typeComboFrame  = CQUtil::makeWidget<QFrame>("typeComboFrame");
  auto *typeComboLayout = CQUtil::makeLayout<QHBoxLayout>(typeComboFrame, 0, 2);

  auto *typeLabel = CQUtil::makeLabelWidget<QLabel>("Type", "typeLabel");

  typeComboLayout->addWidget(typeLabel);

  auto *typeCombo = CQUtil::makeWidget<CQCustomCombo>("typeCombo");

  typeComboLayout->addWidget(typeCombo);

  typeCombo->setToolTip("Plot Type");

  //---

  // get plot types
  CQCharts::PlotTypes plotTypes;

  sortedPlotTypes(plotTypes);

  //---

  QStringList items;

  bool plotTypeHier = false;
  auto plotTypeCat  = PlotType::Category::NONE;

  QString title;

  for (auto &plotType : plotTypes) {
    bool hier = plotType->isHierarchical();
    auto cat  = plotType->category();

    if (hier != plotTypeHier) {
      if (hier)
        typeCombo->addTitle("Hierarchical");

      plotTypeHier = hier;
    }

    if (cat != plotTypeCat) {
      if      (cat == PlotType::Category::ONE_D)
        typeCombo->addTitle("1D");
      else if (cat == PlotType::Category::TWO_D)
        typeCombo->addTitle("2D");
      else if (cat == PlotType::Category::CONNECTION)
        typeCombo->addTitle("Connection");
      else
        typeCombo->addTitle("Other");

      plotTypeCat = cat;
    }

    typeCombo->addItem(plotType->desc());
  }

  connect(typeCombo, SIGNAL(currentIndexChanged(const QString &)),
          this, SLOT(comboSlot(const QString &)));

  //---

  typeComboLayout->addStretch(1);

  return typeComboFrame;
}

void
CQChartsCreatePlotDlg::
modelChangeSlot()
{
  updateModelData();

  validateSlot();
}

void
CQChartsCreatePlotDlg::
updateModelData()
{
  connectSlots(false);

  //---

  modelData_ = charts_->currentModelData();
  assert(modelData_);

  model_ = modelData_->currentModel();

  //---

  if (! CQChartsModelUtil::isHierarchical(model_.data()))
    (void) modelData_->addSummaryModel();

  //---

  // update summary model
  auto *summaryModel = modelData_->summaryModel();

  int nr = model_.data()->rowCount();
  int nc = model_.data()->columnCount();

  summaryEditData_.enabledCheck->setChecked(modelData_->isSummaryEnabled());

  summaryEditData_.maxRows->setRange(1, nr);
  summaryEditData_.maxRows->setToolTip(QString("Set Preview Row Count (1 -> %1)").arg(nr));

  if (summaryModel)
    summaryEditData_.maxRows->setValue(summaryModel->maxRows());
  else
    summaryEditData_.maxRows->setValue(nr);

  if (summaryModel) {
    if      (summaryModel->mode() == CQSummaryModel::Mode::RANDOM)
      summaryEditData_.typeCombo->setCurrentIndex(1);
    else if (summaryModel->mode() == CQSummaryModel::Mode::SORTED)
      summaryEditData_.typeCombo->setCurrentIndex(2);
    else if (summaryModel->mode() == CQSummaryModel::Mode::PAGED)
      summaryEditData_.typeCombo->setCurrentIndex(3);
  }

  summaryEditData_.sortedColEdit->setRange(0, nc - 1);
  summaryEditData_.sortedColEdit->setToolTip(
    QString("Set Preview Sort Column (0 -> %1)").arg(nc - 1));

  if (summaryModel)
    summaryEditData_.sortedColEdit->setValue(summaryModel->sortColumn());

  summaryEditData_.pageSizeEdit->setRange(1, nr);
  summaryEditData_.pageSizeEdit->setToolTip(QString("Set Preview Page Size (1 -> %1)").arg(nr));

  if (summaryModel)
    summaryEditData_.pageSizeEdit->setValue(summaryModel->pageSize());

  int np = (nr + summaryEditData_.pageSizeEdit->value() - 1)/
           summaryEditData_.pageSizeEdit->value();

  summaryEditData_.currentPageEdit->setRange(0, np - 1);
  summaryEditData_.currentPageEdit->setToolTip(
    QString("Set Preview Page Count (0 -> %1)").arg(np - 1));

  if (summaryModel)
    summaryEditData_.currentPageEdit->setValue(summaryModel->currentPage());

  //--

  if (modelData_->isSummaryEnabled() && summaryModel) {
    ModelP summaryModelP = modelData_->summaryModelP();

    summaryEditData_.modelView->setModel(summaryModelP,
      CQChartsModelUtil::isHierarchical(summaryModel));
  }
  else {
    summaryEditData_.modelView->setModel(model_,
      CQChartsModelUtil::isHierarchical(model_.data()));
  }

  summaryEditData_.maxRows        ->setEnabled(summaryModel);
  summaryEditData_.typeCombo      ->setEnabled(summaryModel);
  summaryEditData_.typeStack      ->setEnabled(summaryModel);
  summaryEditData_.sortedColEdit  ->setEnabled(summaryModel);
  summaryEditData_.pageSizeEdit   ->setEnabled(summaryModel);
  summaryEditData_.currentPageEdit->setEnabled(summaryModel);

  //---

  // update model details
  detailsWidget_->setModelData(modelData_);

  //---

  connectSlots(true);
}

void
CQChartsCreatePlotDlg::
connectSlots(bool b)
{
  CQChartsWidgetUtil::connectDisconnect(b,
    modelData_, SIGNAL(dataChanged()), this, SLOT(updateModelData()));
  CQChartsWidgetUtil::connectDisconnect(b,
    modelData_, SIGNAL(deleted()), this, SLOT(cancelSlot()));

  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.enabledCheck, SIGNAL(stateChanged(int)),
    this, SLOT(summaryEnabledSlot()));
  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.maxRows, SIGNAL(valueChanged(int)),
    this, SLOT(updatePreviewSlot()));
  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.typeCombo, SIGNAL(currentIndexChanged(int)),
    this, SLOT(updateSummaryTypeSlot()));
  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.sortedColEdit, SIGNAL(valueChanged(int)),
    this, SLOT(updatePreviewSlot()));
  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.pageSizeEdit, SIGNAL(valueChanged(int)),
    this, SLOT(updatePreviewSlot()));
  CQChartsWidgetUtil::connectDisconnect(b,
    summaryEditData_.currentPageEdit, SIGNAL(valueChanged(int)),
    this, SLOT(updatePreviewSlot()));
}

void
CQChartsCreatePlotDlg::
sortedPlotTypes(CQCharts::PlotTypes &plotTypes1)
{
  CQCharts::PlotTypes plotTypes;

  this->charts()->getPlotTypes(plotTypes);

  // create ordered list of types (1D, 2D, Category, Other, Hierarchical)

  using DimPlotTypesMap     = std::map<int, CQCharts::PlotTypes>;
  using HierDimPlotsTypeMap = std::map<bool, DimPlotTypesMap>;

  HierDimPlotsTypeMap hierDimPlotsTypeMap;

  for (auto &plotType : plotTypes) {
    auto cat = plotType->category();

    int cat1 = 999;

    if (cat != PlotType::Category::NONE)
      cat1 = int(cat);

    hierDimPlotsTypeMap[plotType->isHierarchical()][cat1].push_back(plotType);
  }

  //---

  plotTypes1.clear();

  for (auto &p1 : hierDimPlotsTypeMap) {
    const auto &catPlotTypesMap = p1.second;

    for (auto &p2 : catPlotTypesMap) {
      const auto &plotTypes = p2.second;

      for (auto &plotType : plotTypes)
        plotTypes1.push_back(plotType);
    }
  }
}

QFrame *
CQChartsCreatePlotDlg::
createGeneralDataFrame()
{
  auto createButton = [&](const QString &name, const QString &iconName, const QString &tip,
                          const char *receiver) {
    auto *button = CQUtil::makeWidget<CQIconButton>(name);

    button->setIcon(iconName);

    connect(button, SIGNAL(clicked()), this, receiver);

    button->setToolTip(tip);

    return button;
  };

  //--

  auto *genFrame  = CQUtil::makeWidget<QFrame>("general");
  auto *genLayout = CQUtil::makeLayout<QGridLayout>(genFrame, 2, 2);

  //----

  int row = 0, column = 0;

  viewEdit_ = addStringEdit(genLayout, row, column, "View Name", "viewEdit", "View Name");

  viewEdit_->setToolTip("View to add plot to. If empty create new view");

  //----

  ++row; column = 0;

  auto *positionGroup       = CQUtil::makeLabelWidget<CQGroupBox>("Position", "positionGroup");
  auto *positionGroupLayout = CQUtil::makeLayout<QVBoxLayout>(positionGroup, 2, 2);

  genLayout->addWidget(positionGroup, row, column, 1, 4);

  //--

  autoRangeEdit_ = CQUtil::makeLabelWidget<QCheckBox>("Auto Place", "autoPlace");

  autoRangeEdit_->setChecked(true);
  autoRangeEdit_->setToolTip("Automatically place plots in view");

  positionGroupLayout->addWidget(autoRangeEdit_);

  connect(autoRangeEdit_, SIGNAL(stateChanged(int)), this, SLOT(autoPlaceSlot(int)));

  auto *rangeEditLayout = CQUtil::makeLayout<QHBoxLayout>(positionGroupLayout, 2, 2);

  posEdit_ = addStringEdit(rangeEditLayout, row, column, "Range", "range", "Plot Range");

  posEdit_->setText("0 0 1 1");
  posEdit_->setEnabled(false);
  posEdit_->setToolTip("Position and size of plot in view (values in range 0->1)");

  //----

  ++row; column = 0;

  titleEdit_ = addStringEdit(genLayout, row, column, "Plot Title", "title", "Title");

  titleEdit_->setToolTip("Plot Title");

  connect(titleEdit_, SIGNAL(textChanged(const QString &)), this, SLOT(validateSlot()));

  //----

  ++row; column = 0;

  xLabelEdit_ = addStringEdit(genLayout, row, column, "X Label", "xlabel", "Label");

  xLabelEdit_->setToolTip("X Axis Label");

  connect(xLabelEdit_, SIGNAL(textChanged(const QString &)), this, SLOT(validateSlot()));

  yLabelEdit_ = addStringEdit(genLayout, row, column, "Y Label", "ylabel", "Label");

  yLabelEdit_->setToolTip("Y Axis Label");

  connect(yLabelEdit_, SIGNAL(textChanged(const QString &)), this, SLOT(validateSlot()));

  //----

  ++row; column = 0;

  auto *rangeGroup       = CQUtil::makeLabelWidget<CQGroupBox>("Range", "rangeGroup");
  auto *rangeGroupLayout = CQUtil::makeLayout<QVBoxLayout>(rangeGroup, 2, 2);

  genLayout->addWidget(rangeGroup, row, column, 1, 4);

  //--

  auto *xRangeFrame       = CQUtil::makeWidget<QFrame>("xRangeFrame");
  auto *xRangeFrameLayout = CQUtil::makeLayout<QHBoxLayout>(xRangeFrame, 0, 2);

  rangeGroupLayout->addWidget(xRangeFrame);

  //--

  rangeEditData_.xminEdit =
    addRealEdit(xRangeFrameLayout, row, column, "X Min", "xmin", "Custom X Axis Minimum Value");
  rangeEditData_.xminButton =
    createButton("xminButton", "REFRESH", "Get minimum value for x column", SLOT(xminSlot()));

  xRangeFrameLayout->addWidget(rangeEditData_.xminButton);

  rangeEditData_.yminEdit =
    addRealEdit(xRangeFrameLayout, row, column, "Y Min", "ymin", "Custom Y Axis Minimum Value");
  rangeEditData_.yminButton =
    createButton("yminButton", "REFRESH", "Get minimum value for y column", SLOT(yminSlot()));

  xRangeFrameLayout->addWidget(rangeEditData_.yminButton);

  connect(rangeEditData_.xminEdit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));
  connect(rangeEditData_.yminEdit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));

  //--

  auto *yRangeFrame       = CQUtil::makeWidget<QFrame>("yRangeFrame");
  auto *yRangeFrameLayout = CQUtil::makeLayout<QHBoxLayout>(yRangeFrame, 0, 2);

  rangeGroupLayout->addWidget(yRangeFrame);

  //--

  rangeEditData_.xmaxEdit =
    addRealEdit(yRangeFrameLayout, row, column, "X Max", "xmax", "Custom X Axis Maximum Value");
  rangeEditData_.xmaxButton =
    createButton("xmaxButton", "REFRESH", "Get maximum value for x column", SLOT(xmaxSlot()));

  yRangeFrameLayout->addWidget(rangeEditData_.xmaxButton);

  rangeEditData_.ymaxEdit =
    addRealEdit(yRangeFrameLayout, row, column, "Y Max", "ymax", "Custom Y Axis Maximum Value");
  rangeEditData_.ymaxButton =
    createButton("ymaxButton", "REFRESH", "Get maximum value for y column", SLOT(ymaxSlot()));

  yRangeFrameLayout->addWidget(rangeEditData_.ymaxButton);

  connect(rangeEditData_.xmaxEdit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));
  connect(rangeEditData_.ymaxEdit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));

  //----

  ++row; column = 0;

  auto *xyFrame       = CQUtil::makeWidget<QFrame>("xyFrame");
  auto *xyFrameLayout = CQUtil::makeLayout<QHBoxLayout>(xyFrame, 0, 2);

  genLayout->addWidget(xyFrame, row, column, 1, 5);

  xintegralCheck_ = CQUtil::makeLabelWidget<QCheckBox>("X Integral", "xintegralCheck");
  yintegralCheck_ = CQUtil::makeLabelWidget<QCheckBox>("Y Integral", "yintegralCheck");

  xintegralCheck_->setToolTip("X values are Integral");
  yintegralCheck_->setToolTip("Y values are Integral");

  connect(xintegralCheck_, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));
  connect(yintegralCheck_, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));

  xyFrameLayout->addWidget(xintegralCheck_);
  xyFrameLayout->addWidget(yintegralCheck_);

  //--

  auto *xySpacer = CQChartsWidgetUtil::createHSpacer(1, "xySpacer");

  xyFrameLayout->addWidget(xySpacer);

  //--

  xlogCheck_ = CQUtil::makeLabelWidget<QCheckBox>("X Log", "xlogCheck");
  ylogCheck_ = CQUtil::makeLabelWidget<QCheckBox>("Y Log", "ylogCheck");

  xlogCheck_->setToolTip("Use log scale for X Axis");
  ylogCheck_->setToolTip("Use log scale for Y Axis");

  connect(xlogCheck_, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));
  connect(ylogCheck_, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));

  xyFrameLayout->addWidget(xlogCheck_);
  xyFrameLayout->addWidget(ylogCheck_);

  //--

  xyFrameLayout->addStretch(1);

  //----

  ++row;

  genLayout->setRowStretch(row, 1);

  //---

  return genFrame;
}

QFrame *
CQChartsCreatePlotDlg::
createSummaryFrame()
{
  auto *summaryFrame = CQUtil::makeWidget<QFrame>("summary");

  summaryFrame->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
  summaryFrame->setAutoFillBackground(true);

  auto *summaryLayout = CQUtil::makeLayout<QVBoxLayout>(summaryFrame, 0, 2);

  //--

  auto *summaryControl = CQUtil::makeWidget<QFrame>("summaryControl");

  summaryControl->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

  auto *summaryControlLayout = CQUtil::makeLayout<QHBoxLayout>(summaryControl, 2, 2);

  //---

  summaryEditData_.enabledCheck =
    CQUtil::makeLabelWidget<QCheckBox>("Summary", "summaryEnabled");

  summaryEditData_.enabledCheck->setToolTip("Enable summary of model data");

  summaryControlLayout->addWidget(summaryEditData_.enabledCheck);

  //---

  summaryEditData_.maxRows = CQUtil::makeWidget<CQIntegerSpin>("summaryMaxRows");

  summaryControlLayout->addWidget(CQUtil::makeLabelWidget<QLabel>("Max Rows", "maxRows"));
  summaryControlLayout->addWidget(summaryEditData_.maxRows);

  //---

  summaryEditData_.typeCombo = CQUtil::makeWidget<QComboBox>("summaryTypeCombo");

  summaryEditData_.typeCombo->addItems(
    QStringList() << "Normal" << "Random" << "Sorted" << "Paged");

  summaryEditData_.typeCombo->setToolTip("Summary data selection type");

  summaryControlLayout->addWidget(summaryEditData_.typeCombo);

  //----

  summaryEditData_.typeStack = CQUtil::makeWidget<QStackedWidget>("summaryTypeStack");

  summaryControlLayout->addWidget(summaryEditData_.typeStack);

  //---

  auto *normalTypeFrame = CQUtil::makeWidget<QFrame>("normalTypeFrame");

  summaryEditData_.typeStack->addWidget(normalTypeFrame);

  //---

  auto *randomTypeFrame = CQUtil::makeWidget<QFrame>("randomTypeFrame");

  summaryEditData_.typeStack->addWidget(randomTypeFrame);

  //---

  auto *sortedTypeFrame = CQUtil::makeWidget<QFrame>("sortedTypeFrame");

  summaryEditData_.typeStack->addWidget(sortedTypeFrame);

  auto *sortedTypeLayout = CQUtil::makeLayout<QHBoxLayout>(sortedTypeFrame, 0, 2);

  //--

  summaryEditData_.sortedColEdit = CQUtil::makeWidget<CQIntegerSpin>("summarySortedColEdit");

  sortedTypeLayout->addWidget(CQUtil::makeLabelWidget<QLabel>("Sort Column", "sortCol"));
  sortedTypeLayout->addWidget(summaryEditData_.sortedColEdit);

  sortedTypeLayout->addStretch(1);

  //---

  auto *pageSizeTypeFrame = CQUtil::makeWidget<QFrame>("pageSizeTypeFrame");

  summaryEditData_.typeStack->addWidget(pageSizeTypeFrame);

  auto *pageSizeTypeLayout = CQUtil::makeLayout<QHBoxLayout>(pageSizeTypeFrame, 0, 2);

  //--

  summaryEditData_.pageSizeEdit = CQUtil::makeWidget<CQIntegerSpin>("summaryPageSizeEdit");

  pageSizeTypeLayout->addWidget(CQUtil::makeLabelWidget<QLabel>("Page Size", "pointSize"));
  pageSizeTypeLayout->addWidget(summaryEditData_.pageSizeEdit);

  //--

  summaryEditData_.currentPageEdit = CQUtil::makeWidget<CQIntegerSpin>("summaryCurrentPageEdit");

  pageSizeTypeLayout->addWidget(CQUtil::makeLabelWidget<QLabel>("Current Page", "currentPage"));
  pageSizeTypeLayout->addWidget(summaryEditData_.currentPageEdit);

  pageSizeTypeLayout->addStretch(1);

  //---

  summaryControlLayout->addStretch(1);

  summaryLayout->addWidget(summaryControl);

  //--

  summaryEditData_.modelView = new CQChartsModelViewHolder(charts_);

  summaryLayout->addWidget(summaryEditData_.modelView);

  //--

  return summaryFrame;
}

QFrame *
CQChartsCreatePlotDlg::
createDetailsFrame()
{
  auto *detailsFrame  = CQUtil::makeWidget<QFrame>("details");
  auto *detailsLayout = CQUtil::makeLayout<QVBoxLayout>(detailsFrame, 0, 2);

  detailsFrame->setAutoFillBackground(true);

  detailsWidget_ = new CQChartsModelDetailsWidget(charts_);

  detailsLayout->addWidget(detailsWidget_);

  return detailsFrame;
}

QFrame *
CQChartsCreatePlotDlg::
createPreviewFrame()
{
  auto *previewFrame  = CQUtil::makeWidget<QFrame>("preview");
  auto *previewLayout = CQUtil::makeLayout<QVBoxLayout>(previewFrame, 0, 2);

  previewFrame->setAutoFillBackground(true);

  //---

  previewPlot_ = new CQChartsPreviewPlot(charts_);

  connect(previewPlot_, SIGNAL(enableStateChanged()), this, SLOT(previewEnabledSlot()));

  previewLayout->addWidget(previewPlot_);

  //--

  return previewFrame;
}

void
CQChartsCreatePlotDlg::
addPlotWidgets(PlotType *type, int ind)
{
  // add widgets for type
  auto *basicFrame = CQUtil::makeWidget<QFrame>(type->name() + "_frame");

  basicPlotDataStack_->addWidget(basicFrame);

  // add basic parameter edits
  auto *basicFrameLayout = CQUtil::makeLayout<QGridLayout>(basicFrame, 2, 2);

  int basicRow = 0;

  auto &basicPlotData = basicTypePlotData_[type->name()];

  addParameterEdits(type, basicPlotData, basicFrameLayout, basicRow, /*basic*/true);

  basicPlotData.ind = ind;

  //---

  // add widgets for type
  auto *advancedFrame = CQUtil::makeWidget<QFrame>(type->name() + "_frame");

  advancedPlotDataStack_->addWidget(advancedFrame);

  // add advanced parameter edits
  auto *advancedFrameLayout = CQUtil::makeLayout<QGridLayout>(advancedFrame, 2, 2);

  int advancedRow = 0;

  auto &advancedPlotData = advancedTypePlotData_[type->name()];

  addParameterEdits(type, advancedPlotData, advancedFrameLayout, advancedRow, /*basic*/false);

  advancedPlotData.ind = ind;

  //---

  stackIndexPlotType_[basicPlotData.ind] = type;
}

void
CQChartsCreatePlotDlg::
addParameterEdits(PlotType *type, PlotData &plotData, QGridLayout *layout, int &row, bool isBasic)
{
  using GroupTab    = std::map<int, CQTabWidget*>;
  using ChildGroups = std::vector<PlotParameterGroup *>;

  GroupTab    groupTab;
  ChildGroups childGroups;

  for (const auto &p : type->parameterGroups()) {
    auto *parameterGroup = p.second;

    if (parameterGroup->isHidden())
      continue;

    if (isBasic && (parameterGroup->type() == PlotParameterGroup::SECONDARY))
      continue;

    if (parameterGroup->parentGroupId() >= 0) {
      childGroups.push_back(parameterGroup);
      continue;
    }

    //---

    // get parameters and child parameter groups
    auto parameters = type->groupParameters(parameterGroup->groupId());

    auto parameterGroups = type->groupParameterGroups(parameterGroup->groupId());

    assert(! parameters.empty() || ! parameterGroups.empty());

    //---

    // get tab widget
    auto pg = groupTab.find(-1);

    if (pg == groupTab.end()) {
      auto *parameterGroupTab = CQUtil::makeWidget<CQTabWidget>("parameterGroupTab");

      layout->addWidget(parameterGroupTab, row, 0, 1, 5);

      ++row;

      pg = groupTab.insert(pg, GroupTab::value_type(-1, parameterGroupTab));
    }

    auto *parameterGroupTab = (*pg).second;

    //---

    // create frame for widgets and add to tab widget
    auto *parameterGroupFrame  = CQUtil::makeWidget<QFrame>(parameterGroup->name());
    auto *parameterGroupLayout = CQUtil::makeLayout<QGridLayout>(parameterGroupFrame, 2, 2);

    int row1 = 0;

    //---

    // process child parameter groups
    ChildGroups childGroups1;

    for (const auto &parameterGroup1 : parameterGroups) {
      if (parameterGroup1->isHidden())
        continue;

      if (isBasic && (parameterGroup1->type() == PlotParameterGroup::SECONDARY))
        continue;

      childGroups1.push_back(parameterGroup1);
    }

    auto ng1 = childGroups1.size();

    for (const auto &parameterGroup1 : childGroups1) {
      // get parameters and child parameter groups
      auto parameters1 = type->groupParameters(parameterGroup1->groupId());

      auto parameterGroups1 = type->groupParameterGroups(parameterGroup1->groupId());

      assert(! parameters1.empty() || ! parameterGroups1.empty());

      //---

      CQTabWidget *parameterGroupTab1 = nullptr;

      if (ng1 > 1) {
        // get tab widget
        auto pg = groupTab.find(parameterGroup->groupId());

        if (pg == groupTab.end()) {
          auto *parameterGroupTab1 = CQUtil::makeWidget<CQTabWidget>("parameterGroupTab");

          parameterGroupLayout->addWidget(parameterGroupTab1, row1, 0);

          ++row1;

          pg = groupTab.insert(pg,
            GroupTab::value_type(parameterGroup->groupId(), parameterGroupTab1));
        }

        parameterGroupTab1 = (*pg).second;
      }

      //---

      int row2 = 0;

      auto *parameterGroupFrame1  = CQUtil::makeWidget<QFrame>(parameterGroup1->name());
      auto *parameterGroupLayout1 = CQUtil::makeLayout<QGridLayout>(parameterGroupFrame1, 2, 2);

      if (! parameters1.empty()) {
        addParameterEdits(parameters1, plotData, parameterGroupLayout1, row2, isBasic);

        if (row2 > 0)
          parameterGroupLayout1->setRowStretch(row2, 1);
      }

      if (row2 > 0) {
        if (parameterGroupTab1)
          parameterGroupTab1->addTab(parameterGroupFrame1, parameterGroup1->name());
        else {
          parameterGroupLayout->addWidget(parameterGroupFrame1, row1, 0);

          ++row1;
        }
      }
      else
        delete parameterGroupFrame1;

      //---

      ++row1;
    }

    //---

    if (! parameters.empty()) {
      addParameterEdits(parameters, plotData, parameterGroupLayout, row1, isBasic);

      if (row1 > 0)
        parameterGroupLayout->setRowStretch(row1, 1);
    }

    //---

    if (row1 > 0)
      parameterGroupTab->addTab(parameterGroupFrame, parameterGroup->name());
    else
      delete parameterGroupFrame;
  }

  //---

  auto parameters = type->nonGroupParameters();

  if (! parameters.empty())
    addParameterEdits(parameters, plotData, layout, row, isBasic);

  //---

  layout->setRowStretch(row, 1);
}

void
CQChartsCreatePlotDlg::
addParameterEdits(const PlotType::Parameters &parameters, PlotData &plotData,
                  QGridLayout *layout, int &row, bool isBasic)
{
  // add column edits first
  int nstr  = 0;
  int nreal = 0;
  int nint  = 0;
  int nenum = 0;
  int nbool = 0;

  for (const auto &parameter : parameters) {
    if (parameter->isHidden())
      continue;

    if (isBasic && (! parameter->isRequired() && ! parameter->isBasic()))
      continue;

    plotData.names.insert(parameter->name());

    if      (parameter->type() == PlotParameter::Type::COLUMN ||
             parameter->type() == PlotParameter::Type::COLUMN_LIST)
      addParameterEdit(plotData, layout, row, parameter, isBasic);
    else if (parameter->type() == PlotParameter::Type::STRING)
      ++nstr;
    else if (parameter->type() == PlotParameter::Type::REAL)
      ++nreal;
    else if (parameter->type() == PlotParameter::Type::INTEGER)
      ++nint;
    else if (parameter->type() == PlotParameter::Type::ENUM)
      ++nenum;
    else if (parameter->type() == PlotParameter::Type::BOOLEAN)
      ++nbool;
    else
      assert(false);
  }

  // add string and real edits
  if (nstr > 0 || nreal > 0 || nint > 0) {
    auto *strLayout = CQUtil::makeLayout<QHBoxLayout>(0, 2);

    for (const auto &parameter : parameters) {
      if (parameter->isHidden())
        continue;

      if (isBasic && (! parameter->isRequired() && ! parameter->isBasic()))
        continue;

      if (parameter->type() == PlotParameter::Type::STRING ||
          parameter->type() == PlotParameter::Type::REAL ||
          parameter->type() == PlotParameter::Type::INTEGER)
        addParameterEdit(plotData, strLayout, parameter, isBasic);
    }

    strLayout->addStretch(1);

    if (! isBasic)
      layout->addLayout(strLayout, row, 0, 1, 5);
    else
      layout->addLayout(strLayout, row, 0, 1, 3);

    ++row;
  }

  // add enum edits
  if (nenum > 0) {
    auto *enumLayout = CQUtil::makeLayout<QHBoxLayout>(0, 2);

    for (const auto &parameter : parameters) {
      if (parameter->isHidden())
        continue;

      if (isBasic && (! parameter->isRequired() && ! parameter->isBasic()))
        continue;

      if (parameter->type() == PlotParameter::Type::ENUM)
        addParameterEdit(plotData, enumLayout, parameter, isBasic);
    }

    enumLayout->addStretch(1);

    if (! isBasic)
      layout->addLayout(enumLayout, row, 0, 1, 5);
    else
      layout->addLayout(enumLayout, row, 0, 1, 3);

    ++row;
  }

  // add bool edits
  if (nbool > 0) {
    auto *boolLayout = CQUtil::makeLayout<QHBoxLayout>(0, 2);

    for (const auto &parameter : parameters) {
      if (parameter->isHidden())
        continue;

      if (isBasic && (! parameter->isRequired() && ! parameter->isBasic()))
        continue;

      if (parameter->type() == PlotParameter::Type::BOOLEAN)
        addParameterEdit(plotData, boolLayout, parameter, isBasic);
    }

    boolLayout->addStretch(1);

    if (! isBasic)
      layout->addLayout(boolLayout, row, 0, 1, 5);
    else
      layout->addLayout(boolLayout, row, 0, 1, 3);

    ++row;
  }

  if (isBasic)
    layout->setColumnStretch(2, 1);
}

void
CQChartsCreatePlotDlg::
addParameterEdit(PlotData &plotData, QGridLayout *layout, int &row,
                 PlotParameter *parameter, bool isBasic)
{
  if      (parameter->type() == PlotParameter::Type::COLUMN)
    addParameterColumnEdit(plotData, layout, row, parameter, isBasic);
  else if (parameter->type() == PlotParameter::Type::COLUMN_LIST)
    addParameterColumnsEdit(plotData, layout, row, parameter, isBasic);
  else
    assert(false);
}

void
CQChartsCreatePlotDlg::
addParameterEdit(PlotData &plotData, QHBoxLayout *layout,
                 PlotParameter *parameter, bool /*isBasic*/)
{
  if      (parameter->type() == PlotParameter::Type::STRING)
    addParameterStringEdit(plotData, layout, parameter);
  else if (parameter->type() == PlotParameter::Type::REAL)
    addParameterRealEdit(plotData, layout, parameter);
  else if (parameter->type() == PlotParameter::Type::INTEGER)
    addParameterIntEdit(plotData, layout, parameter);
  else if (parameter->type() == PlotParameter::Type::ENUM)
    addParameterEnumEdit(plotData, layout, parameter);
  else if (parameter->type() == PlotParameter::Type::BOOLEAN)
    addParameterBoolEdit(plotData, layout, parameter);
  else
    assert(false);
}

void
CQChartsCreatePlotDlg::
addParameterColumnEdit(PlotData &plotData, QGridLayout *layout, int &row,
                       PlotParameter *parameter, bool isBasic)
{
  auto objName = parameter->name() + "Column";

  auto *label      = CQUtil::makeLabelWidget<QLabel>(parameter->desc(), objName + "Label");
  auto *columnEdit = new CQChartsColumnParameterEdit(parameter, isBasic);

  columnEdit->setModelData(modelData_);

  plotData.columnEdits[parameter->name()] = columnEdit;

  connect(columnEdit, SIGNAL(columnChanged()), this, SLOT(validateSlot()));

  //---

  layout->addWidget(label     , row, 0);
  layout->addWidget(columnEdit, row, 1);

  //---

  if (! isBasic) {
    int col = 2;

    addFormatEdit(plotData, parameter, layout, "Column", row, col);
  }

  //---

  ++row;

  //---

  if (! isBasic) {
    // add mapping widgets
    if (parameter->isMapped()) {
      MapEditData mapEditData;

      int col = 1;

      auto *mapLayout = CQUtil::makeLayout<QHBoxLayout>(0, 2);

      //---

      // add map check
      mapEditData.mappedCheck = CQUtil::makeLabelWidget<QCheckBox>("Mapped", "mapped");

      connect(mapEditData.mappedCheck, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));

      mapEditData.mappedCheck->
        setToolTip("Remap column values from actual range to specific range");

      //---

      // add map min/max
      mapEditData.mapMinSpin = CQUtil::makeWidget<CQRealSpin>("mapMin");
      mapEditData.mapMaxSpin = CQUtil::makeWidget<CQRealSpin>("mapMax");

      connect(mapEditData.mapMinSpin, SIGNAL(valueChanged(double)), this, SLOT(validateSlot()));
      connect(mapEditData.mapMaxSpin, SIGNAL(valueChanged(double)), this, SLOT(validateSlot()));

      mapEditData.mapMinSpin->setToolTip("Min value for mapped values");
      mapEditData.mapMaxSpin->setToolTip("Max value for mapped values");

      //--

      mapLayout->addWidget(mapEditData.mappedCheck);
      mapLayout->addWidget(mapEditData.mapMinSpin);
      mapLayout->addWidget(mapEditData.mapMaxSpin);
      mapLayout->addStretch(1);

      plotData.mappedEdits[parameter->name()] = mapEditData;

      layout->addLayout(mapLayout, row, col, 1, 2);

      ++row;

      //---

      mapEditData.mappedCheck->setChecked(false);

      mapEditData.mapMinSpin->setValue(parameter->attributes().mapMin());
      mapEditData.mapMaxSpin->setValue(parameter->attributes().mapMax());
    }
  }
}

void
CQChartsCreatePlotDlg::
addParameterColumnsEdit(PlotData &plotData, QGridLayout *layout, int &row,
                        PlotParameter *parameter, bool isBasic)
{
  auto objName = parameter->name() + "Columns";

  auto *label       = CQUtil::makeLabelWidget<QLabel>(parameter->desc(), objName + "Label");
  auto *columnsEdit = new CQChartsColumnsParameterEdit(parameter, isBasic);

  columnsEdit->setModelData(modelData_);

  plotData.columnsEdits[parameter->name()] = columnsEdit;

  connect(columnsEdit, SIGNAL(columnsChanged()), this, SLOT(validateSlot()));

  //---

  layout->addWidget(label      , row, 0);
  layout->addWidget(columnsEdit, row, 1);

  //---

  if (! isBasic) {
    int col = 2;

    addFormatEdit(plotData, parameter, layout, "Columns", row, col);
  }

  //---

  ++row;
}

void
CQChartsCreatePlotDlg::
addParameterStringEdit(PlotData &plotData, QHBoxLayout *layout, PlotParameter *parameter)
{
  auto *editLayout = CQUtil::makeLayout<QHBoxLayout>(layout, 0, 2);

  auto *label = CQUtil::makeLabelWidget<QLabel>(parameter->desc(), parameter->name() + "_label");
  auto *edit  = new CQChartsStringParameterEdit(parameter);

  editLayout->addWidget(label);
  editLayout->addWidget(edit);

  plotData.stringEdits[parameter->name()] = edit;

  //---

  connect(edit, SIGNAL(textChanged(const QString &)), this, SLOT(validateSlot()));
}

void
CQChartsCreatePlotDlg::
addParameterRealEdit(PlotData &plotData, QHBoxLayout *layout, PlotParameter *parameter)
{
  auto *editLayout = CQUtil::makeLayout<QHBoxLayout>(layout, 0, 2);

  auto *label = CQUtil::makeLabelWidget<QLabel>(parameter->desc(), parameter->name() + "_label");
  auto *edit  = new CQChartsRealParameterEdit(parameter);

  editLayout->addWidget(label);
  editLayout->addWidget(edit);

  plotData.realEdits[parameter->name()] = edit;

  //---

  connect(edit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));
}

void
CQChartsCreatePlotDlg::
addParameterIntEdit(PlotData &plotData, QHBoxLayout *layout, PlotParameter *parameter)
{
  auto *editLayout = CQUtil::makeLayout<QHBoxLayout>(layout, 0, 2);

  auto *label = CQUtil::makeLabelWidget<QLabel>(parameter->desc(), parameter->name() + "_label");
  auto *edit  = new CQChartsIntParameterEdit(parameter);

  editLayout->addWidget(label);
  editLayout->addWidget(edit);

  plotData.intEdits[parameter->name()] = edit;

  //---

  connect(edit, SIGNAL(valueChanged()), this, SLOT(validateSlot()));
}

void
CQChartsCreatePlotDlg::
addParameterEnumEdit(PlotData &plotData, QHBoxLayout *layout, PlotParameter *parameter)
{
  auto *eparameter = dynamic_cast<CQChartsEnumParameter *>(parameter);
  assert(eparameter);

  //---

  auto *editLayout = CQUtil::makeLayout<QHBoxLayout>(layout, 0, 2);

  auto *label = CQUtil::makeLabelWidget<QLabel>(eparameter->desc(), eparameter->name() + "_label");
  auto *combo = new CQChartsEnumParameterEdit(eparameter);

  editLayout->addWidget(label);
  editLayout->addWidget(combo);

  plotData.enumEdits[eparameter->name()] = combo;

  //---

  connect(combo, SIGNAL(currentIndexChanged(int)), this, SLOT(validateSlot()));
}

void
CQChartsCreatePlotDlg::
addParameterBoolEdit(PlotData &plotData, QHBoxLayout *layout, PlotParameter *parameter)
{
  auto *edit = new CQChartsBoolParameterEdit(parameter);

  layout->addWidget(edit);

  plotData.boolEdits[parameter->name()] = edit;

  connect(edit, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));
}

CQChartsOptRealEdit *
CQChartsCreatePlotDlg::
addRealEdit(QLayout *layout, int &row, int &column, const QString &name,
            const QString &objName, const QString &placeholderText) const
{
  auto *label = CQUtil::makeLabelWidget<QLabel>(name, objName + "Label");
  auto *edit  = CQUtil::makeWidget<CQChartsOptRealEdit>(objName + "Edit");

  edit->setPlaceholderText(placeholderText);

  auto *grid    = qobject_cast<QGridLayout *>(layout);
  auto *blayout = qobject_cast<QBoxLayout  *>(layout);

  if      (grid) {
    if (label) {
      grid->addWidget(label, row, column); ++column;
    }

    grid->addWidget(edit, row, column); ++column;
  }
  else if (blayout) {
    if (label)
      blayout->addWidget(label);

    blayout->addWidget(edit );
  }

  return edit;
}

CQChartsLineEdit *
CQChartsCreatePlotDlg::
addStringEdit(QLayout *layout, int &row, int &column, const QString &name,
              const QString &objName, const QString &placeholderText) const
{
  auto *label = (name != "" ? CQUtil::makeLabelWidget<QLabel>(name, objName + "Label") : nullptr);
  auto *edit  = CQUtil::makeWidget<CQChartsLineEdit>(objName + "Edit");

  edit->setPlaceholderText(placeholderText);

  auto *grid    = qobject_cast<QGridLayout *>(layout);
  auto *blayout = qobject_cast<QBoxLayout  *>(layout);

  if      (grid) {
    if (label) {
      grid->addWidget(label, row, column); ++column;
    }

    grid->addWidget(edit, row, column); ++column;
  }
  else if (blayout) {
    if (label)
      blayout->addWidget(label);

    blayout->addWidget(edit);
  }

  return edit;
}

//---

void
CQChartsCreatePlotDlg::
addFormatEdit(PlotData &plotData, PlotParameter *parameter, QGridLayout *layout,
              const QString &name, int &row, int &col)
{
  FormatEditData formatEditData;

  formatEditData.formatEdit =
    addStringEdit(layout, row, col, "", parameter->name() + "Format", name + " Format");

  formatEditData.formatEdit->setToolTip(name + " format");

  connect(formatEditData.formatEdit, SIGNAL(textChanged(const QString &)),
          this, SLOT(validateSlot()));

  //--

  // add format update button
  formatEditData.formatUpdate = CQUtil::makeWidget<CQIconButton>("formatUpdate");

  formatEditData.formatUpdate->setIcon("REFRESH");

  connect(formatEditData.formatUpdate, SIGNAL(clicked()),
          this, SLOT(updateFormatSlot()));

  formatEditData.formatUpdate->setToolTip("Get current " + name + " format");

  layout->addWidget(formatEditData.formatUpdate, row, col); ++col;

  plotData.formatEdits[parameter->name()] = formatEditData;

  //---

  // add attributes tip label
  auto *attributesLabel = CQUtil::makeWidget<CQIconLabel>("attributesLabel");

  attributesLabel->setIcon("INFO");
  attributesLabel->setToolTip(parameter->attributes().summary());

  layout->addWidget(attributesLabel, row, col); ++col;
}

//---

void
CQChartsCreatePlotDlg::
comboSlot(const QString &desc)
{
  CQCharts::PlotTypes plotTypes;

  sortedPlotTypes(plotTypes);

  int ind = -1;

  int ind1 = 0;

  for (auto &plotType : plotTypes) {
    if (plotType->desc() == desc) {
      ind = ind1;
      break;
    }

    ++ind1;
  }

  if (ind >= 0) {
    basicPlotDataStack_   ->setCurrentIndex(ind);
    advancedPlotDataStack_->setCurrentIndex(ind);

    validateSlot();
  }
}

void
CQChartsCreatePlotDlg::
advancedSlot(int state)
{
  advanced_ = (state != 0);

  plotDataStack_->setCurrentIndex(advanced_ ? 1 : 0);

  whereFrame_->setVisible(advanced_);

  validateSlot();
}

void
CQChartsCreatePlotDlg::
autoAnalyzeSlot(int state)
{
  setAutoAnalyzeModel(state);

  validateSlot();
}

void
CQChartsCreatePlotDlg::
xminSlot()
{
  setXYMin("xmin");
}

void
CQChartsCreatePlotDlg::
yminSlot()
{
  setXYMin("ymin");
}

void
CQChartsCreatePlotDlg::
xmaxSlot()
{
  setXYMin("xmax");
}

void
CQChartsCreatePlotDlg::
ymaxSlot()
{
  setXYMin("ymax");
}

void
CQChartsCreatePlotDlg::
setXYMin(const QString &id)
{
  if (! model_)
    return;

  auto *type = getPlotType();
  if (! type) return;

  auto &plotData = (isAdvanced() ? advancedTypePlotData_[type->name()] :
                                   basicTypePlotData_   [type->name()]);

  Column column;

  auto colName = (id == "xmin" || id == "xmax" ? type->xColumnName() : type->yColumnName());
  if (colName == "") return;

  for (const auto &parameter : type->parameters()) {
    if (parameter->isHidden())
      continue;

    if (plotData.names.find(parameter->name()) == plotData.names.end())
      continue;

    if (parameter->name() != colName)
      continue;

    if      (parameter->type() == PlotParameter::Type::COLUMN) {
      auto column1 = Column::fromVariant(parameter->defValue());

      QString      columnTypeStr;
      MapValueData mapValueData;

      if (! parseParameterColumnEdit(parameter, plotData, column1, columnTypeStr, mapValueData))
        return;

      column = column1;
    }
    else if (parameter->type() == PlotParameter::Type::COLUMN_LIST) {
      auto columns = Columns::fromVariant(parameter->defValue());

      QString columnTypeStr;

      if (! parseParameterColumnsEdit(parameter, plotData, columns, columnTypeStr))
        return;

      column = columns.column();
    }

    break;
  }

  if (! column.isValid())
    return;

  if (! modelData_)
    return;

  auto *details = modelData_->details();
  if (! details) return;

  if (column.type() != Column::Type::DATA &&
      column.type() != Column::Type::DATA_INDEX)
    return;

  auto *columnDetails = details->columnDetails(column);
  if (! columnDetails) return;

  if      (id == "xmin") {
    auto xmin = columnDetails->minValue();

    rangeEditData_.xminEdit->setValue(CQChartsOptReal(xmin));
  }
  else if (id == "ymin") {
    auto ymin = columnDetails->minValue();

    rangeEditData_.yminEdit->setValue(CQChartsOptReal(ymin));
  }
  else if (id == "xmax") {
    auto xmax = columnDetails->maxValue();

    rangeEditData_.xmaxEdit->setValue(CQChartsOptReal(xmax));
  }
  else if (id == "ymax") {
    auto ymax = columnDetails->maxValue();

    rangeEditData_.ymaxEdit->setValue(CQChartsOptReal(ymax));
  }
}

//---

void
CQChartsCreatePlotDlg::
autoPlaceSlot(int state)
{
  posEdit_->setEnabled(! state);
}

void
CQChartsCreatePlotDlg::
validateSlot()
{
  if (! initialized_)
    return;

  auto *type = getPlotType();
  if (! type) return;

  //---

  if (isAutoAnalyzeModel()) {
    bool isInitialized = false;

    if (isAdvanced())
      isInitialized = advancedTypeInitialzed_[type->description()];
    else
      isInitialized = basicTypeInitialzed_[type->description()];

    if (! isInitialized) {
      auto &plotData = (isAdvanced() ? advancedTypePlotData_[type->name()] :
                                       basicTypePlotData_   [type->name()]);

      CQChartsAnalyzeModel analyzeModel(charts_, modelData_);

      analyzeModel.analyzeType(type);

      const auto &analyzeModelData = analyzeModel.analyzeModelData(type);

      //---

      // init column edits
      for (const auto &nc : analyzeModelData.parameterNameColumn) {
        auto pe = plotData.columnEdits.find(nc.first);

        auto column = nc.second;

        if (pe != plotData.columnEdits.end()) {
          auto *edit = (*pe).second;

          CQChartsWidgetUtil::AutoDisconnect autoDisconnect(
            edit, SIGNAL(columnChanged()), this, SLOT(validateSlot()));

          edit->setColumn(column);
        }
        else {
          auto pe = plotData.columnsEdits.find(nc.first);

          if (pe != plotData.columnsEdits.end()) {
            auto *edit = (*pe).second;

            CQChartsWidgetUtil::AutoDisconnect autoDisconnect(
              edit, SIGNAL(columnsChanged()), this, SLOT(validateSlot()));

            edit->setColumns(Columns(column));
          }
        }
      }

      // init columns edits
      for (const auto &nc : analyzeModelData.parameterNameColumns) {
        auto pe = plotData.columnsEdits.find(nc.first);

        if (pe != plotData.columnsEdits.end()) {
          auto *edit = (*pe).second;

          CQChartsWidgetUtil::AutoDisconnect autoDisconnect(
            edit, SIGNAL(columnsChanged()), this, SLOT(validateSlot()));

          edit->setColumns(nc.second);
        }
      }

      // init bool edits
      for (const auto &nc : analyzeModelData.parameterNameBool) {
        auto pe = plotData.boolEdits.find(nc.first);

        if (pe != plotData.boolEdits.end()) {
          auto *edit = (*pe).second;

          CQChartsWidgetUtil::AutoDisconnect autoDisconnect(
            edit, SIGNAL(stateChanged(int)), this, SLOT(validateSlot()));

          edit->setChecked(nc.second);
        }
      }

      if (isAdvanced())
        advancedTypeInitialzed_[type->description()] = true;
      else
        basicTypeInitialzed_[type->description()] = true;
    }
  }

  //---

  // set description
  descText_->setText(type->description());

  // can only set x/y range if plot supports custom range
  rangeEditData_.xminEdit->setEnabled(type->customXRange());
  rangeEditData_.yminEdit->setEnabled(type->customYRange());
  rangeEditData_.xmaxEdit->setEnabled(type->customXRange());
  rangeEditData_.ymaxEdit->setEnabled(type->customYRange());

  // can only set range if plot type supplies which column refers to these axes
  auto xcolName = type->xColumnName();
  auto ycolName = type->yColumnName();

  rangeEditData_.xminButton->setEnabled(xcolName != "");
  rangeEditData_.yminButton->setEnabled(ycolName != "");
  rangeEditData_.xmaxButton->setEnabled(xcolName != "");
  rangeEditData_.ymaxButton->setEnabled(ycolName != "");

  titleEdit_ ->setEnabled(type->hasTitle());
  xLabelEdit_->setEnabled(type->hasXAxis());
  yLabelEdit_->setEnabled(type->hasYAxis());

  xintegralCheck_->setEnabled(type->hasXAxis() && type->allowXAxisIntegral());
  yintegralCheck_->setEnabled(type->hasYAxis() && type->allowYAxisIntegral());

  xlogCheck_->setEnabled(type->allowXLog());
  ylogCheck_->setEnabled(type->allowYLog());

  //---

  QStringList msgs;

  bool valid = validate(msgs);

  okButton_   ->setEnabled(valid);
  applyButton_->setEnabled(valid);

  if (! valid) {
    msgLabel_->setText(msgs.at(0));
    msgLabel_->setToolTip(msgs.join("\n"));
  }
  else {
    msgLabel_->setText(" ");
    msgLabel_->setToolTip("");
  }

  msgLabel_->setFixedHeight(msgLabel_->sizeHint().height());

  //---

  updatePreviewPlot(valid);
}

void
CQChartsCreatePlotDlg::
updatePreviewPlot(bool valid)
{
  if (valid && previewPlot_->isEnabled()) {
    // get current plot type
    auto *type = getPlotType();
    assert(type);

    // get model to use (current or summary)
    ModelP previewModel;

    auto *summaryModel = modelData_->summaryModel();

    if (modelData_->isSummaryEnabled() && summaryModel)
      previewModel = modelData_->summaryModelP();
    else
      previewModel = model_;

    // update plot
    previewPlot_->updatePlot(previewModel, type, isAdvanced() ? whereEdit_->text() : "");

    applyPlot(previewPlot_->plot(), /*preview*/true);
  }
  else {
    previewPlot_->resetPlot();
  }
}

void
CQChartsCreatePlotDlg::
updateFormatSlot()
{
  auto *formatUpdate = qobject_cast<QToolButton *>(sender());
  if (! formatUpdate) return;

  auto *type = getPlotType();
  if (! type) return;

  auto &plotData = (isAdvanced() ? advancedTypePlotData_[type->name()] :
                                   basicTypePlotData_   [type->name()]);

  QString           parameterName;
  CQChartsLineEdit *formatEdit { nullptr };

  for (const auto &fe : plotData.formatEdits) {
    const auto &formatEditData = fe.second;

    if (formatUpdate == formatEditData.formatUpdate) {
      parameterName = fe.first;
      formatEdit    = formatEditData.formatEdit;
      break;
    }
  }

  if (! parameterName.length())
    return;

  Column column;

  auto pce = plotData.columnEdits.find(parameterName);

  if (pce != plotData.columnEdits.end()) {
    Column defColumn;

    if (isAdvanced()) {
      if (! columnLineEditValue((*pce).second, column, defColumn))
        return;
    }
    else {
    }
  }
  else {
    auto pce = plotData.columnsEdits.find(parameterName);

    if (pce != plotData.columnsEdits.end()) {
      Columns columns;
      Columns defColumns;

      if (! columnsLineEditValue((*pce).second, columns, defColumns))
        return;

      column = columns.column();
    }
  }

  QString typeStr;

  auto *summaryModel = modelData_->summaryModel();

  if (modelData_->isSummaryEnabled() && summaryModel) {
    if (! CQChartsModelUtil::columnTypeStr(charts_, summaryModel, column, typeStr))
      return;
  }
  else {
    if (! CQChartsModelUtil::columnTypeStr(charts_, model_.data(), column, typeStr))
      return;
  }

  if (formatEdit)
    formatEdit->setText(typeStr);
}

bool
CQChartsCreatePlotDlg::
validate(QStringList &msgs)
{
  msgs.clear();

  CQChartsModelData *modelData = nullptr;

  auto *summaryModel = modelData_->summaryModel();

  if (modelData_->isSummaryEnabled() && summaryModel)
    modelData = modelData_->summaryModelData();
  else
    modelData = modelData_;

  if (! modelData) { msgs << "no model data"; return false; }

  auto *details = modelData->details();

  //---

  // create plot for typename of current tab
  auto *type = getPlotType();

  if (! type) {
    msgs << "invalid plot type";
    return false;
  }

  // set plot property for widgets for plot parameters
  auto &plotData = (isAdvanced() ? advancedTypePlotData_[type->name()] :
                                   basicTypePlotData_   [type->name()]);

  bool rc = true;

  int num_valid = 0;
  int num_cols  = 0;

  for (const auto &parameter : type->parameters()) {
    if (parameter->isHidden())
      continue;

    if (plotData.names.find(parameter->name()) == plotData.names.end())
      continue;

    if      (parameter->type() == PlotParameter::Type::COLUMN) {
      ++num_cols;

      auto column = Column::fromVariant(parameter->defValue());

      QString      columnTypeStr;
      MapValueData mapValueData;

      if (! parseParameterColumnEdit(parameter, plotData, column, columnTypeStr, mapValueData)) {
        if (parameter->isRequired()) {
          msgs << QString("missing required column value (%1)").arg(parameter->name());
          rc = false;
        }

        continue;
      }

      if (! column.isValid()) {
        msgs << QString("invalid column value (%1)").arg(parameter->name());
        rc = false;
        continue;
      }

      bool rc1 = true;

      if (column.type() == Column::Type::DATA ||
          column.type() == Column::Type::DATA_INDEX) {
        auto *columnDetails = details->columnDetails(column);
        assert(columnDetails);

        if (parameter->isMonotonic()) {
          if (! columnDetails->isMonotonic()) {
            msgs << QString("non-monotonic column (%1)").arg(parameter->name());
            rc1 = false;
          }
        }

        if      (parameter->isNumericColumn()) {
          if (columnDetails->type() != CQBaseModelType::INTEGER &&
              columnDetails->type() != CQBaseModelType::REAL &&
              columnDetails->type() != CQBaseModelType::TIME) {
            msgs << QString("non-numeric column (%1)").arg(parameter->name());
            rc1 = false;
          }
        }
        else if (parameter->isStringColumn()) {
#if 0
          if (columnDetails->type() != CQBaseModelType::STRING) {
            msgs << QString("non-string column (%1)").arg(parameter->name());
            rc1 = false;
          }
#endif
        }
        else if (parameter->isBoolColumn()) {
          if (columnDetails->type() != CQBaseModelType::BOOLEAN) {
            msgs << QString("non-bool column (%1)").arg(parameter->name());
            rc1 = false;
          }
        }
        else if (parameter->isColorColumn()) {
          if (columnDetails->type() != CQBaseModelType::COLOR) {
            msgs << QString("non-color column (%1)").arg(parameter->name());
            rc1 = false;
          }
        }
      }

      if (rc1)
        ++num_valid;
      else
        rc = rc1;
    }
    else if (parameter->type() == PlotParameter::Type::COLUMN_LIST) {
      ++num_cols;

      auto columns = Columns::fromVariant(parameter->defValue());

      QString columnTypeStr;

      if (! parseParameterColumnsEdit(parameter, plotData, columns, columnTypeStr)) {
        if (parameter->isRequired()) {
          msgs << QString("missing required columns value (%1)").arg(parameter->name());
          rc = false;
        }

        continue;
      }

      if (columns.isValid()) {
        ++num_valid;
      }
    }
  }

#if 0
  if (num_valid == 0 && num_cols > 0) {
    msgs << "no columns specified";
    rc = false;
  }
#endif

  return rc;
}

void
CQChartsCreatePlotDlg::
previewEnabledSlot()
{
  validateSlot();
}

void
CQChartsCreatePlotDlg::
summaryEnabledSlot()
{
  if (modelData_)
    modelData_->setSummaryEnabled(summaryEditData_.enabledCheck->isChecked());

  validateSlot();
}

void
CQChartsCreatePlotDlg::
updateSummaryTypeSlot()
{
  summaryEditData_.typeStack->setCurrentIndex(summaryEditData_.typeCombo->currentIndex());

  updatePreviewSlot();
}

void
CQChartsCreatePlotDlg::
updatePreviewSlot()
{
  auto *summaryModel = modelData_->summaryModel();

  if (modelData_->isSummaryEnabled() && summaryModel) {
    CQChartsWidgetUtil::AutoDisconnect autoDisconnect(
      modelData_, SIGNAL(dataChanged()), this, SLOT(updateModelData()));

    int n = summaryEditData_.maxRows->value();
    if (n <= 0) return;

    bool random = (summaryEditData_.typeCombo->currentText() == "Random");
    bool sorted = (summaryEditData_.typeCombo->currentText() == "Sorted");
    bool paged  = (summaryEditData_.typeCombo->currentText() == "Paged" );

    if (n != summaryModel->maxRows()) {
      summaryModel->setMode(CQSummaryModel::Mode::NORMAL);
      summaryModel->setMaxRows(n);
    }

    // random n rows
    if      (random) {
      summaryModel->setRandomMode(true);
    }
    // first n rows when sorted by column
    else if (sorted) {
      int sortCol = summaryEditData_.sortedColEdit->value();

      summaryModel->setSortColumn(sortCol);
      summaryModel->setSortMode(true);
    }
    // nth page of specified page size
    else if (paged) {
      int ps = summaryEditData_.pageSizeEdit   ->value();
      int np = summaryEditData_.currentPageEdit->value();

      int nr = model_.data()->rowCount();

      int np1 = (nr + ps - 1)/ps;

      np = std::min(np, np1 - 1);

      summaryEditData_.currentPageEdit->setRange(0, np1 - 1);
      summaryEditData_.currentPageEdit->setToolTip(
        QString("Set Preview Page Count (0 -> %1)").arg(np1 - 1));

      summaryModel->setPageSize(ps);
      summaryModel->setCurrentPage(np);
      summaryModel->setPagedMode(true);
    }
    else {
      summaryModel->setMode(CQSummaryModel::Mode::NORMAL);
    }
  }

  validateSlot();
}

void
CQChartsCreatePlotDlg::
okSlot()
{
  if (applySlot())
    accept();
}

bool
CQChartsCreatePlotDlg::
applySlot()
{
  plot_ = nullptr;

  // create plot for typename of current tab
  auto *type = getPlotType();
  if (! type) return false;

  //---

  // get or create view
  auto viewId = viewEdit_->text();

  auto *view = charts()->getView(viewId);

  if (! view) {
    view = charts()->addView();

    auto *window = charts()->createWindow(view);

    window->show();

    viewEdit_->setText(view->id());
  }

  //---

  // create plot
  plot_ = type->createAndInit(view, model_);

  if (isAdvanced())
    plot_->setFilterStr(whereEdit_->text());

  //---

  applyPlot(plot_);

  //---

  double vr = CQChartsView::viewportRange();

  CQChartsGeom::BBox bbox(0, 0, vr, vr);

  if (! autoRangeEdit_->isChecked()) {
    double xmin = 0.0, ymin = 0.0, xmax = 1.0, ymax = 1.0;

    parsePosition(xmin, ymin, xmax, ymax);

    bbox = CQChartsGeom::BBox(vr*xmin, vr*ymin, vr*xmax, vr*ymax);
  }

  //---

  int n = view->numPlots();

  plot_->setId(QString("Chart.%1").arg(n + 1));

  //---

  view->addPlot(plot_, bbox);

  //---

  if (autoRangeEdit_->isChecked()) {
    view->autoPlacePlots();
  }

  return true;
}

CQChartsPlotType *
CQChartsCreatePlotDlg::
getPlotType() const
{
  int ind = basicPlotDataStack_->currentIndex();

  auto p = stackIndexPlotType_.find(ind);

  if (p == stackIndexPlotType_.end())
    return nullptr;

  auto *type = (*p).second;

  return type;
}

void
CQChartsCreatePlotDlg::
applyPlot(Plot *plot, bool preview)
{
  ModelP model = plot->model();

  auto *type = plot->type();

  // set plot property for widgets for plot parameters
  auto &plotData = (isAdvanced() ? advancedTypePlotData_[type->name()] :
                                   basicTypePlotData_   [type->name()]);

  for (const auto &parameter : type->parameters()) {
    if (parameter->isHidden())
      continue;

    if (plotData.names.find(parameter->name()) == plotData.names.end())
      continue;

    if      (parameter->type() == PlotParameter::Type::COLUMN) {
      auto column = Column::fromVariant(parameter->defValue());

      QString      columnTypeStr;
      MapValueData mapValueData;

      if (parseParameterColumnEdit(parameter, plotData, column, columnTypeStr, mapValueData)) {
        if (! plot->setParameter(parameter, column.toString()))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");

        if (columnTypeStr.length())
          CQChartsModelUtil::setColumnTypeStr(charts_, model.data(), column, columnTypeStr);

        if (parameter->isMapped()) {
          QString mappedPropName, mapMinPropName, mapMaxPropName;

          if (parameter->mapPropNames(mappedPropName, mapMinPropName, mapMaxPropName)) {
            if (! CQUtil::setProperty(plot, mappedPropName,
                                      CQChartsVariant::fromBool(mapValueData.mapped)))
              charts()->errorMsg("Failed to set parameter '" + mappedPropName + "'");

            if (! CQUtil::setProperty(plot, mapMinPropName,
                                      CQChartsVariant::fromReal(mapValueData.min)))
              charts()->errorMsg("Failed to set parameter '" + mapMinPropName + "'");

            if (! CQUtil::setProperty(plot, mapMaxPropName,
                                      CQChartsVariant::fromReal(mapValueData.max)))
              charts()->errorMsg("Failed to set parameter '" + mapMaxPropName + "'");
          }
          else {
            charts()->errorMsg("Invalid column parameter name '" + parameter->propName() + "'");
          }
        }
      }
      else {
        if (parameter->isRequired()) {
          if (column.isValid())
            plot->setParameter(parameter, column.toString());
        }
        else {
          plot->setParameter(parameter, CQChartsVariant::fromInt(-1));
        }
      }
    }
    else if (parameter->type() == PlotParameter::Type::COLUMN_LIST) {
      auto columns = Columns::fromVariant(parameter->defValue());

      QString columnTypeStr;

      if (parseParameterColumnsEdit(parameter, plotData, columns, columnTypeStr)) {
        auto s = columns.toString();

        if (! plot->setParameter(parameter, CQChartsVariant::fromString(s)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");

        if (columnTypeStr.length() && columns.isValid())
          CQChartsModelUtil::setColumnTypeStr(charts_, model.data(), columns.column(),
                                              columnTypeStr);
      }
      else {
        if (parameter->isRequired()) {
          if (columns.isValid())
            plot->setParameter(parameter, columns.toString());
        }
        else {
          plot->setParameter(parameter, QString());
        }
      }
    }
    else if (parameter->type() == PlotParameter::Type::STRING) {
      bool ok;
      auto defStr = CQChartsVariant::toString(parameter->defValue(), ok);

      auto str = defStr;

      if (parseParameterStringEdit(parameter, plotData, str)) {
        if (! plot->setParameter(parameter, CQChartsVariant::fromString(str)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");
      }
      else {
        if (parameter->isRequired()) {
          if (ok)
            plot->setParameter(parameter, CQChartsVariant::fromString(defStr));
        }
        else {
          plot->setParameter(parameter, QString());
        }
      }
    }
    else if (parameter->type() == PlotParameter::Type::REAL) {
      bool ok;
      double defValue = CQChartsVariant::toReal(parameter->defValue(), ok);

      double r = defValue;

      if (parseParameterRealEdit(parameter, plotData, r)) {
        if (! plot->setParameter(parameter, CQChartsVariant::fromReal(r)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");
      }
      else {
        if (parameter->isRequired()) {
          if (ok)
            plot->setParameter(parameter, CQChartsVariant::fromReal(defValue));
        }
      }
    }
    else if (parameter->type() == PlotParameter::Type::INTEGER) {
      bool ok;
      auto defValue = CQChartsVariant::toInt(parameter->defValue(), ok);

      int i = int(defValue);

      if (parseParameterIntEdit(parameter, plotData, i)) {
        if (! plot->setParameter(parameter, CQChartsVariant::fromInt(i)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");
      }
      else {
        if (parameter->isRequired()) {
          if (ok)
            plot->setParameter(parameter, CQChartsVariant::fromInt(defValue));
        }
      }
    }
    else if (parameter->type() == PlotParameter::Type::ENUM) {
      bool ok;
      auto defValue = CQChartsVariant::toInt(parameter->defValue(), ok);

      int i = int(defValue);

      if (parseParameterEnumEdit(parameter, plotData, i)) {
        if (! plot->setParameter(parameter, CQChartsVariant::fromInt(i)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");
      }
      else {
        if (preview)
          plot->setParameter(parameter, CQChartsVariant::fromInt(defValue));
      }
    }
    else if (parameter->type() == PlotParameter::Type::BOOLEAN) {
      bool ok;
      bool defValue = CQChartsVariant::toBool(parameter->defValue(), ok);

      bool b = defValue;

      if (parseParameterBoolEdit(parameter, plotData, b)) {
        if (! plot->setParameter(parameter, CQChartsVariant::fromBool(b)))
          charts()->errorMsg("Failed to set parameter '" + parameter->propName() + "'");
      }
      else {
        if (preview)
          plot->setParameter(parameter, CQChartsVariant::fromBool(defValue));
      }
    }
    else
      assert(false);
  }

  //---

  // set title
  if (titleEdit_->text().length())
    plot->setTitleStr(titleEdit_->text());

  // set axis labels
  if (xLabelEdit_->text().length())
    plot->setXLabel(xLabelEdit_->text());

  if (yLabelEdit_->text().length())
    plot->setYLabel(yLabelEdit_->text());

  // set axis integral
  if (plot->xAxis())
    plot->xAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::REAL));

  if (plot->yAxis())
    plot->yAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::REAL));

  if (plot->xAxis() && type->allowXAxisIntegral()) {
    if (xintegralCheck_->isChecked())
      plot->xAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::INTEGER));
  }

  if (plot->yAxis() && type->allowYAxisIntegral()) {
    if (yintegralCheck_->isChecked())
      plot->yAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::INTEGER));
  }

  // set axis log
  plot->setLogX(xlogCheck_->isChecked());
  plot->setLogY(ylogCheck_->isChecked());

  //---

  // set range
  auto xmin = rangeEditData_.xminEdit->value();
  auto ymin = rangeEditData_.yminEdit->value();
  auto xmax = rangeEditData_.xmaxEdit->value();
  auto ymax = rangeEditData_.ymaxEdit->value();

  plot->setXMin(xmin);
  plot->setYMin(ymin);
  plot->setXMax(xmax);
  plot->setYMax(ymax);
}

void
CQChartsCreatePlotDlg::
cancelSlot()
{
  plot_ = nullptr;

  reject();
}

bool
CQChartsCreatePlotDlg::
parsePosition(double &xmin, double &ymin, double &xmax, double &ymax) const
{
  auto posStr = posEdit_->text();

  QStringList posStrs;

  if (! CQTcl::splitList(posStr, posStrs))
    return false;

  if (posStrs.length() == 4) {
    bool ok1; xmin = CQChartsUtil::toReal(posStrs[0], ok1); if (! ok1) xmin = 0.0;
    bool ok2; ymin = CQChartsUtil::toReal(posStrs[1], ok2); if (! ok2) ymin = 0.0;
    bool ok3; xmax = CQChartsUtil::toReal(posStrs[2], ok3); if (! ok3) xmax = 1.0;
    bool ok4; ymax = CQChartsUtil::toReal(posStrs[3], ok4); if (! ok4) ymax = 1.0;

    xmin = CMathUtil::clamp(xmin, 0.0, 1.0);
    ymin = CMathUtil::clamp(ymin, 0.0, 1.0);
    xmax = CMathUtil::clamp(xmax, 0.0, 1.0);
    ymax = CMathUtil::clamp(ymax, 0.0, 1.0);

    if (xmin > xmax) std::swap(xmin, xmax);
    if (ymin > ymax) std::swap(ymin, ymax);

    if (xmin == xmax) {
      if (xmin > 0.0) xmin = 0.0;
      else            xmax = 1.0;
    }

    if (ymin == ymax) {
      if (ymin > 0.0) ymin = 0.0;
      else            ymax = 1.0;
    }

    return (ok1 && ok2 && ok3 && ok4);
  }

  return false;
}

bool
CQChartsCreatePlotDlg::
parseParameterColumnEdit(PlotParameter *parameter, const PlotData &plotData,
                         Column &column, QString &columnType, MapValueData &mapValueData)
{
  // get format value
  if (isAdvanced()) {
    auto pf = plotData.formatEdits.find(parameter->name());
    assert(pf != plotData.formatEdits.end());

    const auto &formatEditData = (*pf).second;

    auto format = formatEditData.formatEdit->text();

    if (format != "")
      columnType = format;
  }

  //---

  // get column value
  auto defColumn = Column::fromVariant(parameter->defValue());

  auto pe = plotData.columnEdits.find(parameter->name());
  assert(pe != plotData.columnEdits.end());

  if (! columnLineEditValue((*pe).second, column, defColumn))
    return false;

  //---

  // get mapped values
  if (isAdvanced()) {
    auto pm = plotData.mappedEdits.find(parameter->name());

    if (pm != plotData.mappedEdits.end()) {
      const auto &mapEditData = (*pm).second;

      mapValueData.mapped = mapEditData.mappedCheck->isChecked();
      mapValueData.min    = mapEditData.mapMinSpin ->value();
      mapValueData.max    = mapEditData.mapMaxSpin ->value();
    }
    else {
      mapValueData.mapped = false;
    }
  }
  else {
    mapValueData.mapped = false;
  }

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterColumnsEdit(PlotParameter *parameter, const PlotData &plotData,
                          Columns &columns, QString &columnType)
{
  // get format value
  if (isAdvanced()) {
    auto pf = plotData.formatEdits.find(parameter->name());
    if (pf == plotData.formatEdits.end()) return true;

    const auto &formatEditData = (*pf).second;

    auto format = formatEditData.formatEdit->text();

    if (format != "")
      columnType = format;
  }

  //---

  // get columns value
  auto defColumns = Columns::fromVariant(parameter->defValue());

  auto pe = plotData.columnsEdits.find(parameter->name());
  assert(pe != plotData.columnsEdits.end());

  if (! columnsLineEditValue((*pe).second, columns, defColumns))
    return false;

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterStringEdit(PlotParameter *parameter, const PlotData &plotData, QString &str)
{
  auto p = plotData.stringEdits.find(parameter->name());
  assert(p != plotData.stringEdits.end());

  str = (*p).second->text();

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterRealEdit(PlotParameter *parameter, const PlotData &plotData, double &r)
{
  auto p = plotData.realEdits.find(parameter->name());
  assert(p != plotData.realEdits.end());

  r = (*p).second->value();

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterIntEdit(PlotParameter *parameter, const PlotData &plotData, int &i)
{
  auto p = plotData.intEdits.find(parameter->name());
  assert(p != plotData.intEdits.end());

  i = (*p).second->value();

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterEnumEdit(PlotParameter *parameter, const PlotData &plotData, int &i)
{
  auto *eparameter = dynamic_cast<CQChartsEnumParameter *>(parameter);
  assert(eparameter);

  auto p = plotData.enumEdits.find(eparameter->name());
  assert(p != plotData.enumEdits.end());

  i = (*p).second->currentValue();

  return true;
}

bool
CQChartsCreatePlotDlg::
parseParameterBoolEdit(PlotParameter *parameter, const PlotData &plotData, bool &b)
{
  auto p = plotData.boolEdits.find(parameter->name());
  assert(p != plotData.boolEdits.end());

  b = (*p).second->isChecked();

  return true;
}

bool
CQChartsCreatePlotDlg::
columnLineEditValue(CQChartsColumnParameterEdit *edit, Column &column,
                    const Column &defColumn) const
{
  column = edit->getColumn();

  if (! column.isValid())
    column = defColumn;

  return column.isValid();
}

bool
CQChartsCreatePlotDlg::
columnsLineEditValue(CQChartsColumnsLineEdit *le, Columns &columns,
                     const Columns &defColumns) const
{
  columns = le->columns();

  if (! columns.isValid())
    columns = defColumns;

  return columns.isValid();
}

void
CQChartsCreatePlotDlg::
setViewName(const QString &viewName)
{
  if (viewEdit_)
    viewEdit_->setText(viewName);
}

int
CQChartsCreatePlotDlg::
exec()
{
  plot_ = nullptr;

  return QDialog::exec();
}
