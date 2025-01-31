#include <CQChartsModelDataWidget.h>
#include <CQChartsModelDetailsWidget.h>

#ifdef CQCHARTS_MODEL_VIEW
#include <CQChartsModelView.h>
#else
#include <CQChartsTable.h>
#include <CQChartsTree.h>
#endif

#include <CQChartsModelData.h>
#include <CQChartsModelDetails.h>
#include <CQChartsLineEdit.h>
#include <CQCharts.h>

#include <CQUtil.h>
#include <CQTabWidget.h>

#include <QVBoxLayout>
#include <QStackedWidget>
#include <set>

CQChartsModelDataWidget::
CQChartsModelDataWidget(CQCharts *charts, ModelData *modelData) :
 QFrame(nullptr), charts_(charts), modelData_(modelData)
{
  setObjectName("modelData");

  init();
}

CQChartsModelDataWidget::
~CQChartsModelDataWidget()
{
}

void
CQChartsModelDataWidget::
init()
{
  auto *layout = CQUtil::makeLayout<QVBoxLayout>(this, 0, 0);

  //--

  // model/details tab
  auto *tableTab = CQUtil::makeWidget<CQTabWidget>("tableTab");

  layout->addWidget(tableTab);

  //---

  // model frame
  auto *viewFrame  = CQUtil::makeWidget<QFrame>("view");
  auto *viewLayout = CQUtil::makeLayout<QVBoxLayout>(viewFrame, 2, 2);

  tableTab->addTab(viewFrame, "Model");

  //---

  // details frame
  auto *detailsFrame  = CQUtil::makeWidget<QFrame>("details");
  auto *detailsLayout = CQUtil::makeLayout<QVBoxLayout>(detailsFrame, 2, 2);

  tableTab->addTab(detailsFrame, "Details");

  //---

  // model filter
  auto *filterEdit = CQUtil::makeWidget<CQChartsLineEdit>("filter");

  filterEdit->setToolTip("Filter Model by Tcl Expression\n"
                         "Use column name variable or column(<col>)\n"
                         "Use row(<row>), cell(<row>,<col>), header(<col>)\n"
                         "Use \"selected\" or \"non-selected\" for selected items");

  viewLayout->addWidget(filterEdit);

  connect(filterEdit, SIGNAL(returnPressed()), this, SLOT(filterSlot()));

  connect(filterEdit, SIGNAL(textChanged(const QString &)),
          this, SIGNAL(filterTextChanged(const QString &)));

  //---

#ifdef CQCHARTS_MODEL_VIEW
  assert(! view_);

  view_ = new CQChartsModelView(charts_);

  viewLayout->addWidget(view_);

  connect(view_, SIGNAL(columnClicked(int)), this, SLOT(columnClicked(int)));
  connect(view_, SIGNAL(selectionHasChanged()), this, SLOT(selectionChanged()));
#else
  assert(! stack_);

  // table/tree stack
  stack_ = CQUtil::makeWidget<QStackedWidget>("stack");

  viewLayout->addWidget(stack_);

  //---

  // tree (hier data)
  tree_ = new CQChartsTree(charts_);

  tree_->setSelectionBehavior(QAbstractItemView::SelectItems);

  stack_->addWidget(tree_);

  connect(tree_, SIGNAL(columnClicked(int)), this, SLOT(columnClicked(int)));
  connect(tree_, SIGNAL(selectionHasChanged()), this, SLOT(selectionChanged()));

  //---

  // table (flat data)
  table_ = new CQChartsTable(charts_);

  table_->setSelectionBehavior(QAbstractItemView::SelectItems);

  stack_->addWidget(table_);

  connect(table_, SIGNAL(columnClicked(int)), this, SLOT(columnClicked(int)));
  connect(table_, SIGNAL(selectionHasChanged()), this, SLOT(selectionChanged()));
#endif

  //------

  // details
  detailsWidget_ = new CQChartsModelDetailsWidget(charts_);

  detailsLayout->addWidget(detailsWidget_);

  //---

  if (modelData_)
    connect(modelData_, SIGNAL(currentModelChanged()), this, SLOT(reloadModelSlot()));

  reloadModel();

  setDetails();
}

void
CQChartsModelDataWidget::
setModelData(ModelData *modelData)
{
  if (modelData_)
    disconnect(modelData_, SIGNAL(currentModelChanged()), this, SLOT(reloadModelSlot()));

  modelData_ = modelData;
  charts_    = (modelData_ ? modelData_->charts() : nullptr);

  if (modelData_)
    connect(modelData_, SIGNAL(currentModelChanged()), this, SLOT(reloadModelSlot()));

  reloadModel();

  setDetails();
}

void
CQChartsModelDataWidget::
filterSlot()
{
  auto *filterEdit = qobject_cast<CQChartsLineEdit *>(sender());

#ifdef CQCHARTS_MODEL_VIEW
  view_->setFilter(filterEdit->text());
#else
  if (stack_->currentIndex() == 0) {
    if (tree_)
      tree_->setFilter(filterEdit->text());
  }
  else {
    if (table_)
      table_->setFilter(filterEdit->text());
  }
#endif
}

void
CQChartsModelDataWidget::
columnClicked(int column)
{
  if (modelData_)
    modelData_->setCurrentColumn(column);
}

void
CQChartsModelDataWidget::
selectionChanged()
{
  std::set<int> columns;

#ifdef CQCHARTS_MODEL_VIEW
  auto *sm = view_->selectionModel();

  auto inds = sm->selectedIndexes();

  for (int i = 0; i < inds.length(); ++i)
    columns.insert(inds[i].column());
#else
  if      (tree_) {
  }
  else if (table_) {
    auto *sm = table_->selectionModel();

    if      (table_->selectionBehavior() == QAbstractItemView::SelectColumns) {
      auto inds = sm->selectedColumns();

      if (inds.size() >= 1) {
        int column = inds[0].column();

        columns.insert(column);
      }
    }
    else if (table_->selectionBehavior() == QAbstractItemView::SelectItems) {
      auto inds = sm->selectedIndexes();

      for (int i = 0; i < inds.length(); ++i)
        columns.insert(inds[i].column());
    }
  }
#endif

  if (columns.size() >= 1) {
    int column = *columns.begin();

    modelData_->setCurrentColumn(column);
  }
}

void
CQChartsModelDataWidget::
reloadModelSlot()
{
  reloadModel();
}

void
CQChartsModelDataWidget::
reloadModel()
{
  if (! modelData_)
    return;

#ifdef CQCHARTS_MODEL_VIEW
  view_->setModelP(modelData_->currentModel(isProxy()));
#else
  if (modelData_->details()->isHierarchical()) {
    if (tree_)
      tree_->setModelP(modelData_->currentModel(isProxy()));

    stack_->setCurrentIndex(0);
  }
  else {
    if (table_)
      table_->setModelP(modelData_->currentModel(isProxy()));

    stack_->setCurrentIndex(1);
  }
#endif
}

void
CQChartsModelDataWidget::
setDetails()
{
  if (! modelData_)
    return;

  //---

  const ModelData *modelData1 = nullptr;

#ifdef CQCHARTS_MODEL_VIEW
  modelData1 = charts_->getModelData(view_->modelP());
#else
  if (stack_->currentIndex() == 0) {
    if (tree_)
      modelData1 = charts_->getModelData(tree_->modelP());
  }
  else {
    if (table_)
      modelData1 = charts_->getModelData(table_->modelP());
  }
#endif

  if (! modelData1)
    modelData1 = modelData_;

  //---

  detailsWidget_->setModelData(const_cast<ModelData *>(modelData1));
}
