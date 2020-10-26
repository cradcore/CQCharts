#include <CQChartsStripPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsTip.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsUtil.h>
#include <CQChartsVariant.h>
#include <CQCharts.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsScriptPaintDevice.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <QMenu>

CQChartsStripPlotType::
CQChartsStripPlotType()
{
}

void
CQChartsStripPlotType::
addParameters()
{
  startParameterGroup("Strip");

  // value, name columns
  addColumnParameter("value", "Value", "valueColumn").
    setNumeric().setRequired().setTip("Value column");

  addColumnParameter("name", "Name", "nameColumn").
    setBasic().setTip("Name column");

  addColumnParameter("position", "Position", "positionColumn").
    setNumeric().setTip("Position column");

  endParameterGroup();

  //---

  // group data
  CQChartsGroupPlotType::addParameters();
}

QString
CQChartsStripPlotType::
description() const
{
  return CQChartsHtml().
   h2("Strip Plot").
    h3("Summary").
     p("TODO");
}

void
CQChartsStripPlotType::
analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData)
{
  auto *details = modelData->details();
  if (! details) return;

  CQChartsColumn valueColumn;

  int nc = details->numColumns();

  for (int i = 0; i < nc; ++i) {
    auto *columnDetails = details->columnDetails(Column(i));

    if (columnDetails && columnDetails->isNumeric()) {
      valueColumn = columnDetails->column();
      break;
    }
  }

  if (valueColumn.isValid())
    analyzeModelData.parameterNameColumn["value"] = valueColumn;
}

CQChartsPlot *
CQChartsStripPlotType::
create(View *view, const ModelP &model) const
{
  return new CQChartsStripPlot(view, model);
}

//------

CQChartsStripPlot::
CQChartsStripPlot(View *view, const ModelP &model) :
 CQChartsGroupPlot(view, view->charts()->plotType("strip"), model),
 CQChartsObjPointData<CQChartsStripPlot>(this)
{
}

CQChartsStripPlot::
~CQChartsStripPlot()
{
  term();
}

//---

void
CQChartsStripPlot::
init()
{
  CQChartsGroupPlot::init();

  //---

  NoUpdate noUpdate(this);

  //---

  setSymbolType(Symbol::Type::CIRCLE);
  setSymbolSize(Length("4px"));
  setSymbolFilled(true);
  setSymbolFillColor(Color(Color::Type::PALETTE));

  //---

  addAxes();

  addKey();

  addTitle();
}

void
CQChartsStripPlot::
term()
{
}

//------

void
CQChartsStripPlot::
setValueColumn(const Column &c)
{
  CQChartsUtil::testAndSet(valueColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsStripPlot::
setNameColumn(const Column &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsStripPlot::
setPositionColumn(const Column &c)
{
  CQChartsUtil::testAndSet(positionColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsStripPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc, bool hidden=false) {
    auto *item = this->addProperty(path, this, name, alias);
    item->setDesc(desc);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  //---

  addBaseProperties();

  // columns
  addProp("columns", "valueColumn"   , "value"   , "Value column");
  addProp("columns", "nameColumn"    , "name"    , "Name column");
  addProp("columns", "positionColumn", "position", "Position column");

  //---

  // options
  addProp("options", "margin", "margin", "Margin");

  //---

  // symbol
  addSymbolProperties("symbol", "", "");

  //---

  // color map
  addColorMapProperties();
}

//---

CQChartsGeom::Range
CQChartsStripPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsStripPlot::calcRange");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsStripPlot *>(this);

  th->clearErrors();

  //---

  th->namePos_   .clear();
  th->posValues_ .clear();
  th->posYValues_.clear();

  th->posRange_ = IMinMax();

  th->xAxis()->clearTickLabels();

  th->xAxis()->setValueType(CQChartsAxisValueType::Type::INTEGER);

  //---

  // check columns
  bool columnsValid = true;

  // value column required
  // name, position columns optional

  if (! checkColumn(valueColumn(), "Value", /*required*/true))
    columnsValid = false;

  if (! checkColumn(nameColumn(), "Name")) columnsValid = false;

  if (! checkColumn(positionColumn(), "Position")) columnsValid = false;

  if (! columnsValid)
    return Range();

  //---

  Range dataRange;

  //---

  // init grouping
  initGroupData(Columns(), nameColumn());

  //---

  // process model data
  class StripVisitor : public ModelVisitor {
   public:
    StripVisitor(const CQChartsStripPlot *plot, Range &dataRange) :
     plot_(plot), dataRange_(dataRange) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      plot_->calcRowRange(data, dataRange_);

      return State::OK;
    }

   private:
    const CQChartsStripPlot* plot_ { nullptr };
    Range&                   dataRange_;
  };

  StripVisitor stripVisitor(this, dataRange);

  visitModel(stripVisitor);

  //---

  for (auto &pgv : th->posValues_) {
    auto &groupValuesData = pgv.second;

    auto &values = groupValuesData.values;

    std::sort(values.begin(), values.end(), [](double lhs, double rhs) { return (lhs < rhs); });
  }

  //---

  // bucket values by y
  for (auto &pgv : th->posValues_) {
    int   pos             = pgv.first;
    auto &groupValuesData = pgv.second;

    const auto &values = groupValuesData.values;

    auto min = *values.begin ();
    auto max = *values.rbegin();

    auto pgyv = th->posYValues_.find(pos);

    if (pgyv == th->posYValues_.end())
      pgyv = th->posYValues_.insert(pgyv, PosYValues::value_type(pos, YValues()));

    auto &yvalues = (*pgyv).second;

    for (const auto &value : values) {
      int iy = (int) CMathUtil::map(value, min, max, 0, 100);

      auto pyv = yvalues.find(iy);

      if (pyv == yvalues.end())
        pyv = yvalues.insert(pyv, YValues::value_type(iy, ValuesData()));

      auto &valuesData = (*pyv).second;

      ++valuesData.n;

      groupValuesData.maxN = std::max(groupValuesData.maxN, valuesData.n);
    }
  }

  return dataRange;
}

void
CQChartsStripPlot::
calcRowRange(const ModelVisitor::VisitData &data, Range &range) const
{
  auto *th = const_cast<CQChartsStripPlot *>(this);

  //---

  // get value (required)
  ModelIndex valueInd(th, data.row, valueColumn(), data.parent);

  bool ok;

  auto value = modelReal(valueInd, ok);
  if (! ok) th->addDataError(valueInd, "Invalid value");

  //---

  // get optional name
  QString name;

  if (nameColumn().isValid()) {
    ModelIndex nameInd(th, data.row, nameColumn(), data.parent);

    name = modelString(nameInd, ok);
    if (! ok) th->addDataError(nameInd, "Invalid name");
  }

  //---

  // get optional position
  using OptInt = CQChartsOptInt;

  OptInt position;

  if (positionColumn().isValid()) {
    ModelIndex positionInd(th, data.row, positionColumn(), data.parent);

    position = OptInt(modelInteger(positionInd, ok));
    if (! ok) { th->addDataError(positionInd, "Invalid position"); position = OptInt(); }
  }

  //---

  if (! position.isSet()) {
    auto pn = th->namePos_.find(name);

    if (pn == th->namePos_.end()) {
      int n = th->namePos_.size();

      pn = th->namePos_.insert(pn, NamePos::value_type(name, n));
    }

    position = OptInt((*pn).second);
  }

  int pos = position.integer();

  th->posRange_.add(pos);

  //--

  auto pgv = th->posValues_.find(pos);

  if (pgv == th->posValues_.end())
    pgv = th->posValues_.insert(pgv, PosValues::value_type(pos, PosValuesData()));

  auto &groupValuesData = (*pgv).second;

  auto &values = groupValuesData.values;

  values.push_back(value);

  //---

  if (name != "")
    th->xAxis()->setTickLabel(pos, name);

  //---

  range.updateRange(Point(pos - 0.5, 0.0  ));
  range.updateRange(Point(pos + 0.5, value));
}

//---

bool
CQChartsStripPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsStripPlot::createObjs");

  NoUpdate noUpdate(this);

  //---

  auto *th = const_cast<CQChartsStripPlot *>(this);

  for (auto &pgyv : th->posYValues_) {
    auto &yvalues = pgyv.second;

    for (auto &pyv : yvalues) {
      auto &valuesData = pyv.second;

      valuesData.values.clear();
    }
  }

  //---

  // process model data
  class StripVisitor : public ModelVisitor {
   public:
    StripVisitor(const CQChartsStripPlot *plot, PlotObjs &objs) :
     plot_(plot), objs_(objs) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      plot_->addRowObj(data, objs_);

      return State::OK;
    }

   private:
    const CQChartsStripPlot *plot_ { nullptr };
    PlotObjs&                objs_;
  };

  StripVisitor stripVisitor(this, objs);

  visitModel(stripVisitor);

  return true;
}

void
CQChartsStripPlot::
addRowObj(const ModelVisitor::VisitData &data, PlotObjs &objs) const
{
  auto *th = const_cast<CQChartsStripPlot *>(this);

  //---

  // get value required
  ModelIndex valueInd(th, data.row, valueColumn(), data.parent);

  bool ok;

  auto value = modelReal(valueInd, ok);
  if (! ok) return;

  //---

  // get optional name
  QString name;

  if (nameColumn().isValid()) {
    ModelIndex nameInd(th, data.row, nameColumn(), data.parent);

    name = modelString(nameInd, ok);
    if (! ok) name = "";
  }

  //--

  // get optional position
  using OptInt = CQChartsOptInt;

  OptInt position;

  if (positionColumn().isValid()) {
    ModelIndex positionInd(th, data.row, positionColumn(), data.parent);

    position = OptInt(modelInteger(positionInd, ok));
    if (! ok) { th->addDataError(positionInd, "Invalid position"); position = OptInt(); }
  }

  //---

  if (! position.isSet()) {
    auto pn = namePos_.find(name);
    assert(pn != namePos_.end());

    position = OptInt((*pn).second);
  }

  int pos = position.integer();

  int ig = pos - posRange_.min();
  int ng = posRange_.max() - posRange_.min() + 1;

  //---

  // get group
//int ns = numGroups();
  int is = rowGroupInd(valueInd);

  if (is < 0)
    is = 0;

  //---

  auto pgv = posValues_.find(pos);
  assert(pgv != posValues_.end());

  auto &groupValuesData = (*pgv).second;

  auto &values = groupValuesData.values;

  auto min = *values.begin ();
  auto max = *values.rbegin();

  int iy = (int) CMathUtil::map(value, min, max, 0, 100);

  auto pgyv = th->posYValues_.find(pos);
  assert(pgyv != posYValues_.end());

  auto &yvalues = (*pgyv).second;

  auto pyv = yvalues.find(iy);
  assert(pyv != yvalues.end());

  auto &valuesData = (*pyv).second;

  int iv = valuesData.values.size();
  int nv = valuesData.n;

#if 0
  double delta = (groupValuesData.maxN > 0 ? 1.0/groupValuesData.maxN : 0.0);

  double dv = 0.0;

  if (iv > 0) {
    int iv1 = iv - 1;

    int lr1 = iv1 & 1;        // 0 left, 1 right
    int nv1 = (iv1 >> 1) + 1; // index

    dv = nv1*delta*(lr1 ? -1 : 1);
  }
#else
  double m = std::min(std::max(this->margin(), 0.0), 0.5);
  double w = 0.5 - m;

  double dv = CMathUtil::map(iv, 0, nv - 1, -w, w);
#endif

  valuesData.values.push_back(value);

  double dpos = pos + dv;

  //---

  double osx, osy;

  plotSymbolSize(symbolSize(), osx, osy);

  BBox bbox(dpos - osx, value - osy, dpos + osx, value + osy);

  Point p(dpos, value);

  auto ind = modelIndex(normalizeIndex(valueInd));

  auto *obj = createPointObj(bbox, is, p, ind, ColorInd(ig, ng), ColorInd(iv, nv));

  objs.push_back(obj);
}

void
CQChartsStripPlot::
addKeyItems(PlotKey *)
{
}

//------

bool
CQChartsStripPlot::
addMenuItems(QMenu *)
{
  return true;
}

//----

CQChartsStripPointObj *
CQChartsStripPlot::
createPointObj(const BBox &rect, int groupInd, const Point &p, const QModelIndex &ind,
               const ColorInd &ig, const ColorInd &iv) const
{
  return new CQChartsStripPointObj(this, rect, groupInd, p, ind, ig, iv);
}

//------

CQChartsStripPointObj::
CQChartsStripPointObj(const Plot *plot, const BBox &rect, int groupInd, const Point &p,
                      const QModelIndex &ind, const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, ColorInd(), ig, iv), plot_(plot),
 groupInd_(groupInd), p_(p)
{
  setModelInd(ind);
}

QString
CQChartsStripPointObj::
calcId() const
{
  return QString("%1:%2:%3").arg(typeName()).arg(ig_.i).arg(iv_.i);
}

QString
CQChartsStripPointObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  auto groupName = plot_->groupIndName(groupInd_);

  tableTip.addTableRow("Group", groupName);
  tableTip.addTableRow("Ind"  , iv_.i);

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

bool
CQChartsStripPointObj::
inside(const Point &p) const
{
  auto p1 = plot_->windowToPixel(Point(p_.x, p_.y));

  BBox pbbox(p1.x - 4, p1.y - 4, p1.x + 4, p1.y + 4);

  auto pp = plot_->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsStripPointObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, CQChartsColumn(modelInd().column()));
}

void
CQChartsStripPointObj::
draw(PaintDevice *device) const
{
  auto symbolType = plot_->symbolType();
  auto symbolSize = plot_->symbolSize();

  //---

  // get color index
  auto colorInd = this->calcColorInd();

  //---

  // calc stroke and brush
  PenBrush penBrush;

  plot_->setSymbolPenBrush(penBrush, colorInd);

  plot_->updateObjPenBrushState(this, penBrush, CQChartsPlot::DrawType::SYMBOL);

  //---

  // draw symbol
  plot_->drawSymbol(device, p_, symbolType, symbolSize, penBrush);
}
