#include <CQChartsWheelPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsTip.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsScriptPaintDevice.h>
#include <CQChartsHtml.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

CQChartsWheelPlotType::
CQChartsWheelPlotType()
{
}

void
CQChartsWheelPlotType::
addParameters()
{
  startParameterGroup("Wheel");

  // columns
  addColumnParameter ("x", "X", "xColumn").
    setRequired().setMonotonic().setNumeric().setTip("X Value Column");
  addColumnsParameter("y", "Y", "yColumn").
    setRequired().setNumeric().setTip("Y Value Column");

  addColumnsParameter("min", "Min", "minColumn").
    setNumeric().setTip("Min Value Column");
  addColumnsParameter("max", "Max", "maxColumn").
    setNumeric().setTip("Max Value Column");

  addColumnsParameter("innerBar", "InnerBar", "innerBarColumn").
    setTip("Inner Fill Column");
  addColumnsParameter("outerBar", "OuterBar", "outerBarColumn").
    setNumeric().setTip("Outer Bar Column");
  addColumnsParameter("outerBubble", "OuterBubble", "outerBubbleColumn").
    setNumeric().setTip("Outer Bubble Column");

  endParameterGroup();

  //---

  CQChartsPlotType::addParameters();
}

QString
CQChartsWheelPlotType::
description() const
{
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Wheel Plot").
    h3("Summary").
     p("Draws data in wheel.").
    h3("Limitations").
     p("None.").
    h3("Example").
     p(IMG("images/wheel.png"));
}

void
CQChartsWheelPlotType::
analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData)
{
  auto px   = analyzeModelData.parameterNameColumn.find("x");
  bool hasX = (px != analyzeModelData.parameterNameColumn.end());

  auto *details = modelData->details();
  if (! details) return;

  // set x column
  Column xColumn;

  int nc = details->numColumns();

  for (int c = 0; c < nc; ++c) {
    if (! xColumn.isValid()) {
      auto *columnDetails = details->columnDetails(Column(c));
      if (! columnDetails) continue;

      if      (columnDetails->isMonotonic())
        xColumn = columnDetails->column();
      else if (columnDetails->type() == ColumnType::TIME)
        xColumn = columnDetails->column();
      else if (! hasX && columnDetails->isNumeric())
        xColumn = columnDetails->column();
    }
  }

  if      (xColumn.isValid())
    analyzeModelData.parameterNameColumn["x"] = xColumn;
  else if (hasX)
    xColumn = (*px).second;

  // set y column
  Column yColumn;

  for (int c = 0; c < nc; ++c) {
    if (c == xColumn.column())
      continue;

    auto *columnDetails = details->columnDetails(Column(c));
    if (! columnDetails) continue;

    if (columnDetails->isNumeric()) {
      yColumn = columnDetails->column();
      break;
    }
  }

  analyzeModelData.parameterNameColumn["y"] = yColumn;
}

CQChartsPlot *
CQChartsWheelPlotType::
create(View *view, const ModelP &model) const
{
  return new CQChartsWheelPlot(view, model);
}

//------

CQChartsWheelPlot::
CQChartsWheelPlot(View *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("wheel"), model)
{
  setGridFont(CQChartsFont().decFontSize(6));
  setTextFont(CQChartsFont().decFontSize(4));
}

CQChartsWheelPlot::
~CQChartsWheelPlot()
{
}

//---

void
CQChartsWheelPlot::
init()
{
  CQChartsPlot::init();

  //---

  NoUpdate noUpdate(this);

  addTitle();
}

void
CQChartsWheelPlot::
term()
{
}

//------

void
CQChartsWheelPlot::
setXColumn(const Column &c)
{
  CQChartsUtil::testAndSet(xColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setYColumn(const Column &c)
{
  CQChartsUtil::testAndSet(yColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setMinColumn(const Column &c)
{
  CQChartsUtil::testAndSet(minColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setMaxColumn(const Column &c)
{
  CQChartsUtil::testAndSet(maxColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setInnerBarColumn(const Column &c)
{
  CQChartsUtil::testAndSet(innerBarColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setOuterBarColumn(const Column &c)
{
  CQChartsUtil::testAndSet(outerBarColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setOuterBubbleColumn(const Column &c)
{
  CQChartsUtil::testAndSet(outerBubbleColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//------

void
CQChartsWheelPlot::
setInnerRadius(double r)
{
  CQChartsUtil::testAndSet(innerRadius_, r, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsWheelPlot::
setOuterRadius(double r)
{
  CQChartsUtil::testAndSet(outerRadius_, r, [&]() { updateRangeAndObjs(); } );
}

//------

void
CQChartsWheelPlot::
setHotTemp(double t)
{
  CQChartsUtil::testAndSet(hotTemp_, t, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setColdTemp(double t)
{
  CQChartsUtil::testAndSet(coldTemp_, t, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setTempPalette(const PaletteName &s)
{
  CQChartsUtil::testAndSet(tempPalette_, s, [&]() { drawObjs(); } );
}

//------

void
CQChartsWheelPlot::
setLineColor(const Color &c)
{
  CQChartsUtil::testAndSet(lineColor_, c, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setLineAlpha(const Alpha &a)
{
  CQChartsUtil::testAndSet(lineAlpha_, a, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setLineWidth(const Length &w)
{
  CQChartsUtil::testAndSet(lineWidth_, w, [&]() { drawObjs(); } );
}

//------

void
CQChartsWheelPlot::
setGridColor(const Color &c)
{
  CQChartsUtil::testAndSet(gridColor_, c, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setGridAlpha(const Alpha &a)
{
  CQChartsUtil::testAndSet(gridAlpha_, a, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setGridFont(const Font &f)
{
  CQChartsUtil::testAndSet(gridFont_, f, [&]() { drawObjs(); } );
}

//------

void
CQChartsWheelPlot::
setTextColor(const Color &c)
{
  CQChartsUtil::testAndSet(textColor_, c, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setTextFont(const Font &f)
{
  CQChartsUtil::testAndSet(textFont_, f, [&]() { drawObjs(); } );
}

//------

void
CQChartsWheelPlot::
setInnerBarColor(const Color &c)
{
  CQChartsUtil::testAndSet(innerBarColor_, c, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setOuterBarColor(const Color &c)
{
  CQChartsUtil::testAndSet(outerBarColor_, c, [&]() { drawObjs(); } );
}

void
CQChartsWheelPlot::
setOuterBubbleColor(const Color &c)
{
  CQChartsUtil::testAndSet(outerBubbleColor_, c, [&]() { drawObjs(); } );
}

//------

void
CQChartsWheelPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  addBaseProperties();

  // columns
  addProp("columns", "xColumn"  , "x"  , "X value column");
  addProp("columns", "yColumn"  , "y"  , "Y value column");
  addProp("columns", "minColumn", "min", "Min value column");
  addProp("columns", "maxColumn", "max", "Max value column");

  // options
  addProp("options", "innerRadius", "innerRadius", "Inner Radius");
  addProp("options", "outerRadius", "outerRadius", "Outer Radius");

  addProp("options", "hotTemp"    , "hotTemp"    , "Hot Temperature");
  addProp("options", "coldTemp"   , "coldTemp"   , "Cold Temperature");
  addProp("options", "tempPalette", "tempPalette", "Temperature Palette");

  addProp("options", "lineColor", "lineColor", "Line Color");
  addProp("options", "lineAlpha", "lineAlpha", "Line Alpha");
  addProp("options", "lineWidth", "lineWidth", "Line Width");

  addProp("options", "gridColor", "gridColor", "Grid Color");
  addProp("options", "gridAlpha", "gridAlpha", "Grid Alpha");
  addProp("options", "gridFont" , "gridFont" , "Grid Font");

  addProp("options", "textColor", "textColor", "Text Color");
  addProp("options", "textFont" , "textFont" , "Text Font");

  addProp("options", "innerBarColor"   , "innerBarColor"   , "Inner Bar Color");
  addProp("options", "outerBarColor"   , "outerBarColor"   , "Outer Bar Color");
  addProp("options", "outerBubbleColor", "outerBubbleColor", "Outer Bubble Color");
}

CQChartsGeom::Range
CQChartsWheelPlot::
calcRange() const
{
  Range dataRange;

  dataRange.updateRange(-1.25, -1.25);
  dataRange.updateRange( 1.25,  1.25);

  // adjust for equal scale
  if (isEqualScale()) {
    double aspect = this->aspect();

    dataRange.equalScale(aspect);
  }

  return dataRange;
}

//------

bool
CQChartsWheelPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsWheelPlot::createObjs");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsWheelPlot *>(this);

  //--

  th->clearErrors();

  th->moveObj_ = nullptr;

  //---

  // check columns
  bool columnsValid = true;

  // value column required, count column optional
  if (! checkColumn(xColumn(), "X", /*required*/true))
    columnsValid = false;

  if (! checkColumn(yColumn(), "Y", /*required*/true))
    columnsValid = false;

  if (! checkColumn(minColumn(), "Min", /*required*/false))
    columnsValid = false;

  if (! checkColumn(maxColumn(), "Max", /*required*/false))
    columnsValid = false;

  if (! checkColumn(innerBarColumn(), "Inner Bar", /*required*/false))
    columnsValid = false;

  if (! checkColumn(outerBarColumn(), "Outer Bar", /*required*/false))
    columnsValid = false;

  if (! checkColumn(outerBubbleColumn(), "Outer Bubble", /*required*/false))
    columnsValid = false;

  if (! columnsValid)
    return false;

  //---

  class WheelModelVisitor : public ModelVisitor {
   public:
    WheelModelVisitor(const CQChartsWheelPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      auto *plot = const_cast<CQChartsWheelPlot *>(plot_);

      ModelIndex xModelInd(plot, data.row, plot_->xColumn(), data.parent);
      ModelIndex yModelInd(plot, data.row, plot_->yColumn(), data.parent);

      bool ok1;
      auto x = plot_->modelReal(xModelInd, ok1);
      if (! ok1) return State::SKIP;

      bool ok2;
      auto y = plot_->modelReal(yModelInd, ok2);
      if (! ok2) return State::SKIP;

      xMinMax_.add(x);
      yMinMax_.add(y);

      PointData pointData;

      pointData.point = Point(x, y);

      if (plot_->minColumn().isValid()) {
        ModelIndex minModelInd(plot, data.row, plot_->minColumn(), data.parent);

        bool ok;
        auto min = plot_->modelReal(minModelInd, ok);

        if (ok) {
          pointData.min = OptReal(min);

          yMinMax_.add(min);
        }
      }

      if (plot_->maxColumn().isValid()) {
        ModelIndex maxModelInd(plot, data.row, plot_->maxColumn(), data.parent);

        bool ok;
        auto max = plot_->modelReal(maxModelInd, ok);

        if (ok) {
          pointData.max = OptReal(max);

          yMinMax_.add(max);
        }
      }

      if (plot_->innerBarColumn().isValid()) {
        ModelIndex innerBarModelInd(plot, data.row, plot_->innerBarColumn(), data.parent);

        bool ok;
        auto innerBar = plot_->modelValue(innerBarModelInd, ok);

        if (ok)
          pointData.innerBar = innerBar;
      }

      if (plot_->outerBarColumn().isValid()) {
        ModelIndex outerBarModelInd(plot, data.row, plot_->outerBarColumn(), data.parent);

        bool ok;
        auto outerBar = plot_->modelReal(outerBarModelInd, ok);

        if (ok) {
          pointData.outerBar = OptReal(outerBar);

          outerBarMinMax_.add(outerBar);
        }
      }

      if (plot_->outerBubbleColumn().isValid()) {
        ModelIndex outerBubbleModelInd(plot, data.row, plot_->outerBubbleColumn(), data.parent);

        bool ok;
        auto outerBubble = plot_->modelReal(outerBubbleModelInd, ok);

        if (ok && outerBubble > 0.0) {
          pointData.outerBubble = OptReal(outerBubble);

          outerBubbleMinMax_.add(outerBubble);
        }
      }

      addPointData(pointData);

      return State::OK;
    }

    //---

    double xmin() const { return xMinMax_.min(0.0); }
    double xmax() const { return xMinMax_.max(1.0); }

    double ymin() const { return yMinMax_.min(0.0); }
    double ymax() const { return yMinMax_.max(1.0); }

    double outerBarMin() const { return outerBarMinMax_.min(0.0); }
    double outerBarMax() const { return outerBarMinMax_.max(1.0); }

    double outerBubbleMin() const { return outerBubbleMinMax_.min(0.0); }
    double outerBubbleMax() const { return outerBubbleMinMax_.max(1.0); }

    //---

    const PointDatas &pointDatas() const { return pointDatas_; }

    void addPointData(const PointData &pointData) { pointDatas_.push_back(pointData); }

   private:
    const CQChartsWheelPlot* plot_ { nullptr };
    RMinMax                  xMinMax_;
    RMinMax                  yMinMax_;
    RMinMax                  outerBarMinMax_;
    RMinMax                  outerBubbleMinMax_;
    PointDatas               pointDatas_;
  };

  WheelModelVisitor visitor(this);

  visitModel(visitor);

  xmin_ = visitor.xmin();
  xmax_ = visitor.xmax();

  ymin_ = visitor.ymin();
  ymax_ = visitor.ymax();

  outerBarMin_ = visitor.outerBarMin();
  outerBarMax_ = visitor.outerBarMax();

  outerBubbleMin_ = visitor.outerBubbleMin();
  outerBubbleMax_ = visitor.outerBubbleMax();

  //---

  Polygon poly;

  int i = 0;
  int n = visitor.pointDatas().size();

  da_ = 360.0/n;
  dx_ = (xmax_ - xmin_)/n;

  for (const auto &pointData : visitor.pointDatas()) {
    auto pp = pointToPolarPoint(pointData.point);

    poly.addPoint(pp.point);

    //---

    auto w = 0.1;
    auto h = 0.1;

    BBox rect(pp.point.x - w/2, pp.point.y - h/2, pp.point.x + w/2, pp.point.y + h/2);

    QModelIndex ind;
    ColorInd    iv(i, n);

    auto *pointObj = th->createPointObj(rect, pointData, ind, iv);

    objs.push_back(pointObj);

    if (! moveObj_)
      moveObj_ = pointObj;

    //---

    if (pointData.innerBar.isValid()) {
      auto w = 0.1;
      auto h = 0.1;

      PointData pointData1 = pointData;
      PointData pointData2 = pointData;

      pointData1.point = Point(pointData.point.x, ymin_);
      pointData2.point = Point(pointData.point.x, ymax_);

      auto pp1 = pointToPolarPoint(pointData1.point);
      auto pp2 = pointToPolarPoint(pointData2.point);

      BBox rect(std::min(pp1.point.x, pp2.point.x) - w/2, std::min(pp1.point.y, pp2.point.y) - h/2,
                std::max(pp1.point.x, pp2.point.x) + w/2, std::max(pp1.point.y, pp2.point.y) + h/2);

      QModelIndex ind;
      ColorInd    iv(i, n);

      auto *barObj = th->createInnerBarObj(rect, pointData1, ind, iv);

      objs.push_back(barObj);
    }

    //---

    if (pointData.outerBar.isSet()) {
      auto w = 0.1;
      auto h = 0.1;

      PointData pointData1 = pointData;

      pointData1.point = Point(pointData.point.x, ymax_);

      auto pp1 = pointToPolarPoint(pointData1.point);

      BBox rect(pp1.point.x - w/2, pp1.point.y - h/2, pp1.point.x + w/2, pp1.point.y + h/2);

      QModelIndex ind;
      ColorInd    iv(i, n);

      auto *barObj = th->createOuterBarObj(rect, pointData1, ind, iv);

      objs.push_back(barObj);
    }

    //---

    if (pointData.outerBubble.isSet()) {
      auto w = 0.1;
      auto h = 0.1;

      PointData pointData1 = pointData;

      pointData1.point = Point(pointData.point.x, ymax_);

      auto pp1 = pointToPolarPoint(pointData1.point);

      BBox rect(pp1.point.x - w/2, pp1.point.y - h/2, pp1.point.x + w/2, pp1.point.y + h/2);

      QModelIndex ind;
      ColorInd    iv(i, n);

      auto *bubbleObj = th->createOuterBubbleObj(rect, pointData1, ind, iv);

      objs.push_back(bubbleObj);
    }

    //---

    ++i;
  }

  //---

  if (poly.size() > 0)
    poly.addPoint(poly.point(0));

  BBox rect(-1, -1, 1, 1);

  auto *lineObj = th->createLineObj(rect, poly, QModelIndex(), ColorInd(0, 1));

  objs.push_back(lineObj);

  //---

  return true;
}

//---

bool
CQChartsWheelPlot::
selectMove(const Point &p, Constraints constraints, bool first)
{
  auto *oldMoveObj = moveObj_;

  movePoint_.polar = polarFromPoint(p);

  moveObj_ = nullptr;

  double a  = CMathUtil::normalizeAngle(movePoint_.polar.a);
  double da = 0.0;

  for (const auto &plotObj : plotObjs_) {
    auto *pointObj = dynamic_cast<CQChartsPointObj *>(plotObj);
    if (! pointObj) continue;

    auto pp = pointToPolarPoint(pointObj->point());

    double a1  = CMathUtil::normalizeAngle(pp.polar.a);
    double da1 = std::abs(a1 - a);

    if (! moveObj_ || da1 < da) {
      moveObj_ = pointObj;
      da       = da1;
    }
  }

  bool rc = CQChartsPlot::selectMove(p, constraints, first);

  if (moveObj_ != oldMoveObj) {
    moveTip_ = (moveObj_ ? moveObj_->calcTipId() : "");

    invalidateOverlay();
  }

  return rc;
}

//---

CQChartsPolarPoint
CQChartsWheelPlot::
pointToPolarPoint(const Point &point) const
{
  CQChartsPolarPoint pp;

  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  pp.polar.a = -CMathUtil::map(point.x, xmin_, xmax_, 0.0,   360.0) + 90.0;
  pp.polar.r =  CMathUtil::map(point.y, ymin_, ymax_, 0.0, ro - ri);

  pp.point = polarToPoint(pp.polar);

  return pp;
}

CQChartsGeom::Point
CQChartsWheelPlot::
polarToPoint(const Polar &polar) const
{
  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);

  double ar = CMathUtil::Deg2Rad(polar.a);

  double x = (ri + polar.r)*std::cos(ar);
  double y = (ri + polar.r)*std::sin(ar);

  return Point(x, y);
}

CQChartsGeom::Point
CQChartsWheelPlot::
pointFromPolar(const Point &point) const
{
  auto p = polarFromPoint(point);

  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  double x = CMathUtil::map(90.0 - p.a, 0.0,   360.0, xmin_, xmax_);
  double y = CMathUtil::map(p.r       , 0.0, ro - ri, ymin_, ymax_);

  return Point(x, y);
}

CQChartsPolar
CQChartsWheelPlot::
polarFromPoint(const Point &point) const
{
  double a = std::atan2(point.y, point.x);

  if (a < 0)
    a = 2*M_PI + a;

  Polar polar;

  polar.a = CMathUtil::Rad2Deg(a);

  double r = std::hypot(point.x, point.y);

  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  polar.r = std::min(std::max(r - ri, 0.0), ro - ri);

  return polar;
}

double
CQChartsWheelPlot::
outerBarRadius(double value) const
{
  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  double gap = 0.01;

  return CMathUtil::map(value, outerBarMin_, outerBarMax_, ro - ri + gap, ro - ri + 0.1 + gap);
}

double
CQChartsWheelPlot::
outerBubbleRadius(double value) const
{
  return CMathUtil::map(value, 0.0, outerBubbleMax_, 0.0, 0.1);
}

//---

bool
CQChartsWheelPlot::
hasBackground() const
{
  return false;
}

void
CQChartsWheelPlot::
execDrawBackground(PaintDevice *) const
{
}

bool
CQChartsWheelPlot::
hasForeground() const
{
  return true;
}

void
CQChartsWheelPlot::
execDrawForeground(PaintDevice *device) const
{
  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  //---

  auto lc = interpColor(gridColor(), ColorInd());

  PenBrush penBrush;

  setPenBrush(penBrush,
    PenData(true, lc, gridAlpha()), BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw radial grid
  device->drawEllipse(BBox(-ri, -ri, ri, ri));
  device->drawEllipse(BBox(-ro, -ro, ro, ro));

  //---

  view()->setPlotPainterFont(this, device, gridFont());

  // draw month grid
  static const char *monthNames[] = {
   "January", "February", "March"    , "April"  , "May"     , "June"    ,
   "July"   , "August"  , "September", "October", "November", "December"
  };

  double da = 360.0/12;

  double a = 90.0;

  for (int i = 0; i < 12; ++i) {
    auto pp1 = polarToPoint(Polar(a, 0.0          ));
    auto pp2 = polarToPoint(Polar(a, ro - ri + 0.2));

    device->drawLine(pp1, pp2);

    CQChartsTextOptions textOptions;

    textOptions.angle = CQChartsAngle(a - 90);

    CQChartsDrawUtil::drawTextAtPoint(device, pp2, monthNames[i], textOptions, /*centered*/true);

    a -= da;
  }

  //---

  // draw y value axis
  CInterval interval;

  interval.setStart(ymin_);
  interval.setEnd  (ymax_);

  double y  = interval.calcStart();
  double dy = interval.calcIncrement();
  int    n  = interval.calcNumMajor();

  for (int i = 0; i < n; ++i) {
    if (y >= ymin_ && y <= ymax_) {
      auto r = CMathUtil::map(y, ymin_, ymax_, 0.0, ro - ri);

      auto pp = polarToPoint(Polar(90.0, r));

      CQChartsTextOptions textOptions;

      QString text = QString("%1").arg(y);

      CQChartsDrawUtil::drawTextAtPoint(device, pp, text, textOptions, /*centered*/true);
    }

    y += dy;
  }
}

bool
CQChartsWheelPlot::
hasOverlay() const
{
  return true;
}

void
CQChartsWheelPlot::
execDrawOverlay(PaintDevice *device) const
{
  if (! moveObj_)
    return;

  double ri = std::min(std::max(innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(outerRadius(), 0.0), 1.0);

  //---

  PenBrush penBrush;

  // draw mouse position
  auto lc = interpColor(lineColor(), ColorInd());

  setPenBrush(penBrush,
    PenData(false), BrushData(true, lc, lineAlpha()));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  auto pp = pointToPolarPoint(moveObj_->point());

  auto p1 = polarToPoint(Polar(pp.polar.a - da_/2.0, 0.0));
  auto p2 = polarToPoint(Polar(pp.polar.a + da_/2.0, 0.0));
  auto p3 = polarToPoint(Polar(pp.polar.a - da_/2.0, ro - ri + 0.2));
  auto p4 = polarToPoint(Polar(pp.polar.a + da_/2.0, ro - ri + 0.2));

  Polygon poly;

  poly.addPoint(p2);
  poly.addPoint(p4);
  poly.addPoint(p3);
  poly.addPoint(p1);

  device->drawPolygon(poly);

  //---

  // draw mouse object details in center
  view()->setPlotPainterFont(this, device, textFont());

  auto tc = interpColor(textColor(), ColorInd());

  setPenBrush(penBrush,
    PenData(true, tc), BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  CQChartsTextOptions textOptions;

  textOptions.html      = true;
  textOptions.alignHtml = false;

  double ts = 2*ri - 0.2;
  double tx = 0.1;

  //CQChartsDrawUtil::drawTextAtPoint(device, Point(tx, 0.0),
  //                                  moveTip_, textOptions, /*centered*/true);

  BBox rect(Point(tx - ts/2.0, -ts/2.0), Point(tx + ts/2.0, ts/2.0));

  CQChartsDrawUtil::drawTextInBox(device, rect, moveTip_, textOptions);

  //device->drawRect(rect); // DEBUG

  //---

  auto *details = columnDetails(innerBarColumn());

  if (details) {
    QString name = moveObj_->pointData().innerBar.toString();

    CQChartsImage image;

    if (! details->namedImage(name, image))
      std::cerr << "Failed to find image for '" << name.toStdString() << "'\n";

    if (image.isValid()) {
      double is = 0.1;
      double di = ri/std::sqrt(2.0) - is;
      double xc = -di;
      double yc =  di;

      BBox irect(Point(xc - is/2.0, yc - is/2.0), Point(xc + is/2.0, yc + is/2.0));

      device->drawImageInRect(irect, image);
    }
  }
}

//---

CQChartsPointObj *
CQChartsWheelPlot::
createPointObj(const BBox &rect, const PointData &pointData,
               const QModelIndex &ind, const ColorInd &iv)
{
  return new CQChartsPointObj(this, rect, pointData, ind, iv);
}

CQChartsLineObj *
CQChartsWheelPlot::
createLineObj(const BBox &rect, const Polygon &poly, const QModelIndex &ind, const ColorInd &iv)
{
  return new CQChartsLineObj(this, rect, poly, ind, iv);
}

CQChartsInnerBarObj *
CQChartsWheelPlot::
createInnerBarObj(const BBox &rect, const PointData &pointData,
                  const QModelIndex &ind, const ColorInd &iv)
{
  return new CQChartsInnerBarObj(this, rect, pointData, ind, iv);
}

CQChartsOuterBarObj *
CQChartsWheelPlot::
createOuterBarObj(const BBox &rect, const PointData &pointData,
                  const QModelIndex &ind, const ColorInd &iv)
{
  return new CQChartsOuterBarObj(this, rect, pointData, ind, iv);
}

CQChartsOuterBubbleObj *
CQChartsWheelPlot::
createOuterBubbleObj(const BBox &rect, const PointData &pointData,
                     const QModelIndex &ind, const ColorInd &iv)
{
  return new CQChartsOuterBubbleObj(this, rect, pointData, ind, iv);
}

//------

CQChartsPointObj::
CQChartsPointObj(const CQChartsWheelPlot *plot, const BBox &rect, const PointData &pointData,
                 const QModelIndex &ind, const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsWheelPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), pointData_(pointData)
{
  if (ind.isValid())
    setModelInd(ind);

  setSelectable(false);

  setPriority(10);
}

QString
CQChartsPointObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(is_.i);
}

QString
CQChartsPointObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow(plot_->columnHeaderName(plot_->xColumn()),
    plot_->columnStr(plot_->xColumn(), point().x));
  tableTip.addTableRow(plot_->columnHeaderName(plot_->yColumn()),
    plot_->columnStr(plot_->yColumn(), point().y));

  if (pointData_.min.isSet())
    tableTip.addTableRow(plot_->columnHeaderName(plot_->minColumn()),
      plot_->columnStr(plot_->minColumn(), pointData_.min.real()));

  if (pointData_.max.isSet())
    tableTip.addTableRow(plot_->columnHeaderName(plot_->maxColumn()),
      plot_->columnStr(plot_->maxColumn(), pointData_.max.real()));

  if (pointData_.innerBar.isValid())
    tableTip.addTableRow(plot_->columnHeaderName(plot_->innerBarColumn()),
                         pointData_.innerBar.toString());

  if (pointData_.outerBar.isSet())
    tableTip.addTableRow(plot_->columnHeaderName(plot_->outerBarColumn()),
      plot_->columnStr(plot_->outerBarColumn(), pointData_.outerBar.real()));

  //---
  if (pointData_.outerBubble.isSet())
    tableTip.addTableRow(plot_->columnHeaderName(plot_->outerBubbleColumn()),
      plot_->columnStr(plot_->outerBubbleColumn(), pointData_.outerBubble.real()));

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

void
CQChartsPointObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "point")->setDesc("Point");
}

//---

void
CQChartsPointObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->xColumn());
  addColumnSelectIndex(inds, plot_->yColumn());
}

void
CQChartsPointObj::
draw(PaintDevice *device) const
{
  auto pp = plot_->pointToPolarPoint(point());

  //---

  bool updateState = device->isInteractive();

  device->setColorNames();

  //---

  auto normalizeTemp = [&](double t) {
    double nt = CMathUtil::map(t, plot_->coldTemp(), plot_->hotTemp(), 0.0, 1.0);
    return std::min(std::max(nt, 0.0), 1.0);
  };

  if (pointData_.min.isSet() && pointData_.max.isSet()) {
    double min = pointData_.min.real();
    double max = pointData_.max.real();

    //---

    auto pp1 = plot_->pointToPolarPoint(Point(point().x - plot_->dx()/2.0, min));
    auto pp2 = plot_->pointToPolarPoint(Point(point().x + plot_->dx()/2.0, min));
    auto pp3 = plot_->pointToPolarPoint(Point(point().x - plot_->dx()/2.0, max));
    auto pp4 = plot_->pointToPolarPoint(Point(point().x + plot_->dx()/2.0, max));

    //---

    //CQChartsFillPattern fillPattern(CQChartsFillPattern::Type::LGRADIENT);

    double pmin = normalizeTemp(min);
    double pmax = normalizeTemp(max);

    auto palette = plot_->tempPalette();
    palette.setMin(pmin); palette.setMax(pmax);

    CQChartsFillPattern fillPattern(CQChartsFillPattern::Type::PALETTE);

    fillPattern.setPalette(palette);

    //fillPattern.setAltColor(c2);

    fillPattern.setAngle(CQChartsAngle(pp.polar.a));

    Polygon poly;

    poly.addPoint(pp2.point);
    poly.addPoint(pp4.point);
    poly.addPoint(pp3.point);
    poly.addPoint(pp1.point);

    PenBrush penBrush;

    plot_->setPenBrush(penBrush,
      PenData(false), BrushData(true, QColor(), Alpha(), fillPattern));

    if (updateState)
      plot_->updateObjPenBrushState(this, penBrush);

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->drawPolygon(poly);
  }

  //---

  device->resetColorNames();
}

void
CQChartsPointObj::
writeScriptData(ScriptPaintDevice *device) const
{
  CQChartsPlotObj::writeScriptData(device);
}

//------

CQChartsLineObj::
CQChartsLineObj(const CQChartsWheelPlot *plot, const BBox &rect, const Polygon &poly,
                const QModelIndex &ind, const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsWheelPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), poly_(poly)
{
  if (ind.isValid())
    setModelInd(ind);

  setSelectable(false);

  setPriority(20);
}

QString
CQChartsLineObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(is_.i);
}

QString
CQChartsLineObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Line", "");

  plot()->addTipColumns(tableTip, modelInd());

  return tableTip.str();
}

//---

void
CQChartsLineObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = path + "/" + propertyId();

  CQChartsPlotObj::addProperties(model, path1);
}

//---

void
CQChartsLineObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->xColumn());
  addColumnSelectIndex(inds, plot_->yColumn());
}

void
CQChartsLineObj::
draw(PaintDevice *device) const
{
  // calc stroke and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawPolyline(poly_);

  device->resetColorNames();
}

void
CQChartsLineObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto lc = plot_->interpColor(plot_->lineColor(), ColorInd());

  plot_->setPenBrush(penBrush,
    PenData(true, lc, plot_->lineAlpha(), plot_->lineWidth()), BrushData(false));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsLineObj::
writeScriptData(ScriptPaintDevice *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);
}

//------

CQChartsInnerBarObj::
CQChartsInnerBarObj(const CQChartsWheelPlot *plot, const BBox &rect, const PointData &pointData,
                    const QModelIndex &ind, const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsWheelPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), pointData_(pointData)
{
  if (ind.isValid())
    setModelInd(ind);

  setSelectable(false);

  setPriority(-10);
}

QString
CQChartsInnerBarObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(is_.i);
}

QString
CQChartsInnerBarObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Bar", "");

  plot()->addTipColumns(tableTip, modelInd());

  return tableTip.str();
}

//---

void
CQChartsInnerBarObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "point")->setDesc("Point");
}

//---

void
CQChartsInnerBarObj::
getObjSelectIndices(Indices &) const
{
}

void
CQChartsInnerBarObj::
draw(PaintDevice *device) const
{
  // calc stroke and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  //---

  auto pp1 = plot_->pointToPolarPoint(Point(point().x - plot_->dx()/2.0, plot_->ymax()));
  auto pp2 = plot_->pointToPolarPoint(Point(point().x + plot_->dx()/2.0, plot_->ymax()));

  double ri = std::min(std::max(plot_->innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(plot_->outerRadius(), 0.0), 1.0);

  CQChartsPolarPoint::Polar pp3(pp1.polar.a, 0.0);
  CQChartsPolarPoint::Polar pp4(pp2.polar.a, 0.0);
  CQChartsPolarPoint::Polar pp5(pp1.polar.a, ro - ri);
  CQChartsPolarPoint::Polar pp6(pp2.polar.a, ro - ri);

  auto p1 = plot_->polarToPoint(pp3);
  auto p2 = plot_->polarToPoint(pp4);
  auto p3 = plot_->polarToPoint(pp5);
  auto p4 = plot_->polarToPoint(pp6);

  //---

  Polygon poly;

  poly.addPoint(p2);
  poly.addPoint(p4);
  poly.addPoint(p3);
  poly.addPoint(p1);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawPolygon(poly);

  //---

  device->resetColorNames();
}

void
CQChartsInnerBarObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto *details = plot_->columnDetails(plot_->innerBarColumn());

  auto fc = plot_->interpColor(plot_->innerBarColor(), ColorInd());

  if (details) {
    Color c;

    QString name = pointData_.innerBar.toString();

    if (! details->namedColor(name, c))
      std::cerr << "Failed to find color for '" << name.toStdString() << "'\n";

    fc = c.color();
  }

  plot_->setPenBrush(penBrush,
    PenData(false), BrushData(true, fc, Alpha(0.3)));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsInnerBarObj::
writeScriptData(ScriptPaintDevice *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);
}

//------

CQChartsOuterBarObj::
CQChartsOuterBarObj(const CQChartsWheelPlot *plot, const BBox &rect, const PointData &pointData,
                    const QModelIndex &ind, const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsWheelPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), pointData_(pointData)
{
  if (ind.isValid())
    setModelInd(ind);

  setSelectable(false);

  setPriority(-10);
}

QString
CQChartsOuterBarObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(is_.i);
}

QString
CQChartsOuterBarObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Bar", "");

  plot()->addTipColumns(tableTip, modelInd());

  return tableTip.str();
}

//---

void
CQChartsOuterBarObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "point")->setDesc("Point");
}

//---

void
CQChartsOuterBarObj::
getObjSelectIndices(Indices &) const
{
}

void
CQChartsOuterBarObj::
draw(PaintDevice *device) const
{
  // calc stroke and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  //---

  double ri = std::min(std::max(plot_->innerRadius(), 0.0), 1.0);
  double ro = std::min(std::max(plot_->outerRadius(), 0.0), 1.0);

  auto tp1 = plot_->pointToPolarPoint(Point(point().x - plot_->dx()/2.0, plot_->ymax()));
  auto tp2 = plot_->pointToPolarPoint(Point(point().x + plot_->dx()/2.0, plot_->ymax()));

  double r = plot_->outerBarRadius(pointData_.outerBar.real());

  double gap = 0.01;

  CQChartsPolarPoint::Polar pp1(tp1.polar.a, ro - ri + gap);
  CQChartsPolarPoint::Polar pp2(tp2.polar.a, ro - ri + gap);
  CQChartsPolarPoint::Polar pp3(tp1.polar.a, r);
  CQChartsPolarPoint::Polar pp4(tp2.polar.a, r);

  auto p1 = plot_->polarToPoint(pp1);
  auto p2 = plot_->polarToPoint(pp2);
  auto p3 = plot_->polarToPoint(pp3);
  auto p4 = plot_->polarToPoint(pp4);

  //---

  Polygon poly;

  poly.addPoint(p2);
  poly.addPoint(p4);
  poly.addPoint(p3);
  poly.addPoint(p1);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawPolygon(poly);

  //---

  device->resetColorNames();
}

void
CQChartsOuterBarObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto fc = plot_->interpColor(plot_->outerBarColor(), ColorInd());

  plot_->setPenBrush(penBrush,
    PenData(false), BrushData(true, fc, Alpha(0.3)));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsOuterBarObj::
writeScriptData(ScriptPaintDevice *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);
}

//------

CQChartsOuterBubbleObj::
CQChartsOuterBubbleObj(const CQChartsWheelPlot *plot, const BBox &rect, const PointData &pointData,
                       const QModelIndex &ind, const ColorInd &is) :
 CQChartsPlotObj(const_cast<CQChartsWheelPlot *>(plot), rect, is, ColorInd(), ColorInd()),
 plot_(plot), pointData_(pointData)
{
  if (ind.isValid())
    setModelInd(ind);

  setSelectable(false);

  setPriority(10);
}

QString
CQChartsOuterBubbleObj::
calcId() const
{
  return QString("%1:%2").arg(typeName()).arg(is_.i);
}

QString
CQChartsOuterBubbleObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Bubble", "");

  plot()->addTipColumns(tableTip, modelInd());

  return tableTip.str();
}

//---

void
CQChartsOuterBubbleObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  auto path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  CQChartsPlotObj::addProperties(model, path1);

  model->addProperty(path1, this, "point")->setDesc("Point");
}

//---

void
CQChartsOuterBubbleObj::
getObjSelectIndices(Indices &) const
{
}

void
CQChartsOuterBubbleObj::
draw(PaintDevice *device) const
{
  // calc stroke and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  //---

  double r = plot_->outerBubbleRadius(pointData_.outerBubble.real());

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  auto pp = plot_->pointToPolarPoint(point());

  double ri = std::min(std::max(plot_->innerRadius(), 0.0), 1.0);

  CQChartsPolarPoint::Polar pp1(pp.polar.a, 1.15 - ri);

  auto p = plot_->polarToPoint(pp1);

  BBox bbox(p.x - r, p.y - r, p.x + r, p.y + r);

  device->drawEllipse(bbox);

  //---

  device->resetColorNames();
}

void
CQChartsOuterBubbleObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto fc = plot_->interpColor(plot_->outerBubbleColor(), ColorInd());

  plot_->setPenBrush(penBrush,
    PenData(true, fc), BrushData(true, fc, Alpha(0.3)));

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush);
}

void
CQChartsOuterBubbleObj::
writeScriptData(ScriptPaintDevice *device) const
{
  calcPenBrush(penBrush_, /*updateState*/ false);

  CQChartsPlotObj::writeScriptData(device);
}
