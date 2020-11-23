#include <CQChartsPointPlot.h>
#include <CQChartsDataLabel.h>
#include <CQChartsAxisRug.h>
#include <CQChartsAxis.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelUtil.h>
#include <CQChartsVariant.h>
#include <CQChartsFitData.h>
#include <CQChartsGrahamHull.h>
#include <CQChartsWidgetUtil.h>
#include <CQChartsTip.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>

CQChartsPointPlotType::
CQChartsPointPlotType() :
 CQChartsGroupPlotType()
{
}

void
CQChartsPointPlotType::
addMappingParameters()
{
  // custom columns/map
  startParameterGroup("Symbols");

  addColumnParameter("symbolType", "Symbol Type", "symbolTypeColumn").
    setTip("Custom Symbol Type").setMapped().
    setMapMinMax(CQChartsSymbol::minFillValue(), CQChartsSymbol::maxFillValue());

  addColumnParameter("symbolSize", "Symbol Size", "symbolSizeColumn").
    setTip("Custom Symbol Size").setMapped().
    setMapMinMax(CQChartsSymbolSize::minValue(), CQChartsSymbolSize::maxValue());

  addColumnParameter("fontSize", "Font Size", "fontSizeColumn").
    setTip("Custom Font Size for Label").setMapped().
    setMapMinMax(CQChartsFontSize::minValue(), CQChartsFontSize::maxValue());

  endParameterGroup();
}

//---

CQChartsPointPlot::
CQChartsPointPlot(CQChartsView *view, CQChartsPlotType *plotType, const ModelP &model) :
 CQChartsGroupPlot(view, plotType, model),
 CQChartsObjBestFitShapeData<CQChartsPointPlot>(this),
 CQChartsObjHullShapeData   <CQChartsPointPlot>(this),
 CQChartsObjStatsLineData   <CQChartsPointPlot>(this)
{
}

CQChartsPointPlot::
~CQChartsPointPlot()
{
  term();
}

//---

void
CQChartsPointPlot::
init()
{
  CQChartsGroupPlot::init();

  //---

  NoUpdate noUpdate(this);

  // create a data label (shared state for all data labels)
  dataLabel_ = new CQChartsDataLabel(this);

  dataLabel_->setClip(false);
  dataLabel_->setMoveClipped(false);
  dataLabel_->setSendSignal(true);

  connect(dataLabel_, SIGNAL(dataChanged()), this, SLOT(dataLabelChanged()));

  //---

  xRug_ = std::make_unique<CQChartsAxisRug>(this, Qt::Horizontal);
  yRug_ = std::make_unique<CQChartsAxisRug>(this, Qt::Vertical);

  xRug_->setVisible(false);
  yRug_->setVisible(false);

  //---

  // set mapped range
  setSymbolTypeMapped(true);
  setSymbolTypeMapMin(CQChartsSymbol::minFillValue());
  setSymbolTypeMapMax(CQChartsSymbol::maxFillValue());

  setSymbolSizeMapped(true);
  setSymbolSizeMapMin(CQChartsSymbolSize::minValue());
  setSymbolSizeMapMax(CQChartsSymbolSize::maxValue());

  setFontSizeMapped(true);
  setFontSizeMapMin(CQChartsFontSize::minValue());
  setFontSizeMapMax(CQChartsFontSize::maxValue());

  //---

  // best fit
  setBestFit(false);
  setBestFitStrokeDash(CQChartsLineDash(CQChartsLineDash::Lengths({2, 2}), 0));
  setBestFitFillColor(Color(Color::Type::PALETTE));
  setBestFitFillAlpha(Alpha(0.5));

  // hull
  setHullFillColor(Color(Color::Type::PALETTE));

  // stats
  setStatsLines(false);
  setStatsLinesDash(CQChartsLineDash(CQChartsLineDash::Lengths({2, 2}), 0));
}

void
CQChartsPointPlot::
term()
{
  clearFitData ();
  clearHullData();

  delete dataLabel_;
}

//---

void
CQChartsPointPlot::
clearFitData()
{
  for (const auto &gfit : groupFitData_)
    delete gfit.second;

  groupFitData_.clear();
}

CQChartsFitData *
CQChartsPointPlot::
getBestFit(int ind, bool &created) const
{
  // get best fit for group or set id
  created = false;

  auto *th = const_cast<CQChartsPointPlot *>(this);

  auto ph = th->groupFitData_.find(ind);

  if (ph == th->groupFitData_.end()) {
    ph = th->groupFitData_.insert(ph, GroupFitData::value_type(ind, new BestFit));

    created = true;
  }

  return (*ph).second;
}

//---

void
CQChartsPointPlot::
clearHullData()
{
  for (const auto &ghull : groupHull_)
    delete ghull.second;

  groupHull_.clear();
}

CQChartsGrahamHull *
CQChartsPointPlot::
getHull(int ind, bool &created) const
{
  // get hull for group or set id
  created = false;

  auto *th = const_cast<CQChartsPointPlot *>(this);

  auto ph = th->groupHull_.find(ind);

  if (ph == th->groupHull_.end()) {
    ph = th->groupHull_.insert(ph, GroupHull::value_type(ind, new Hull));

    created = true;
  }

  return (*ph).second;
}

//---

void
CQChartsPointPlot::
addPointProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  // columns
  addProp("columns", "symbolTypeColumn", "symbolType", "Symbol type column");
  addProp("columns", "symbolSizeColumn", "symbolSize", "Symbol size column");
  addProp("columns", "fontSizeColumn"  , "fontSize"  , "Font size column");

  //---

  // mapping for columns
  addProp("mapping/symbolType", "symbolTypeMapped", "enabled", "Symbol type values mapped");
  addProp("mapping/symbolType", "symbolTypeMapMin", "min"    , "Symbol type map min value");
  addProp("mapping/symbolType", "symbolTypeMapMax", "max"    , "Symbol type map max value");

  addProp("mapping/symbolSize", "symbolSizeMapped"  , "enabled", "Symbol size values mapped");
  addProp("mapping/symbolSize", "symbolSizeMapMin"  , "min"    , "Symbol size map min value");
  addProp("mapping/symbolSize", "symbolSizeMapMax"  , "max"    , "Symbol size map max value");
  addProp("mapping/symbolSize", "symbolSizeMapUnits", "units"  , "Symbol size map units");

  addProp("mapping/fontSize", "fontSizeMapped"  , "enabled", "Font size value mapped");
  addProp("mapping/fontSize", "fontSizeMapMin"  , "min"    , "Font size map min value");
  addProp("mapping/fontSize", "fontSizeMapMax"  , "max"    , "Font size map max value");
  addProp("mapping/fontSize", "fontSizeMapUnits", "units"  , "Font size map units");
}

void
CQChartsPointPlot::
addBestFitProperties(bool hasLayer)
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  // best fit line and deviation fill
  addProp("bestFit", "bestFit"         , "visible"  , "Show best fit overlay");
  addProp("bestFit", "bestFitOutliers" , "outliers" , "Best fit include outliers");
  addProp("bestFit", "bestFitOrder"    , "order"    , "Best fit curve order");
  addProp("bestFit", "bestFitDeviation", "deviation", "Best fit standard deviation");

  if (hasLayer)
    addProp("bestFit", "bestFitLayer", "layer", "Best fit draw layer");

  addFillProperties("bestFit/fill"  , "bestFitFill"  , "Best fit");
  addLineProperties("bestFit/stroke", "bestFitStroke", "Best fit");
}

void
CQChartsPointPlot::
addHullProperties(bool hasLayer)
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  // convex hull shape
  addProp("hull", "hull", "visible", "Show convex hull overlay");

  if (hasLayer)
    addProp("hull", "hullLayer", "layer", "Convex hull draw layer");

  addFillProperties("hull/fill"  , "hullFill"  , "Convex hull");
  addLineProperties("hull/stroke", "hullStroke", "Convex hull");
}

void
CQChartsPointPlot::
addStatsProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  // stats
  addProp("statsData", "statsLines", "visible", "Statistic lines visible");

  addLineProperties("statsData/stroke", "statsLines", "Statistic lines");
}

void
CQChartsPointPlot::
addRugProperties()
{
  xRug_->addProperties("rug/x", "X Rug");
  yRug_->addProperties("rug/y", "Y Rug");
}

void
CQChartsPointPlot::
getPropertyNames(QStringList &names, bool hidden) const
{
  CQChartsPlot::getPropertyNames(names, hidden);

  propertyModel()->objectNames(dataLabel(), names, hidden);
}

//---

const CQChartsColumn &
CQChartsPointPlot::
symbolTypeColumn() const
{
  return symbolTypeData_.column;
}

void
CQChartsPointPlot::
setSymbolTypeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(symbolTypeData_.column, c, [&]() { updateObjs(); } );
}

bool
CQChartsPointPlot::
isSymbolTypeMapped() const
{
  return symbolTypeData_.mapped;
}

void
CQChartsPointPlot::
setSymbolTypeMapped(bool b)
{
  CQChartsUtil::testAndSet(symbolTypeData_.mapped, b, [&]() { updateObjs(); } );
}

int
CQChartsPointPlot::
symbolTypeMapMin() const
{
  return symbolTypeData_.map_min;
}

void
CQChartsPointPlot::
setSymbolTypeMapMin(int i)
{
  CQChartsUtil::testAndSet(symbolTypeData_.map_min, i, [&]() { updateObjs(); } );
}

int
CQChartsPointPlot::
symbolTypeMapMax() const
{
  return symbolTypeData_.map_max;
}

void
CQChartsPointPlot::
setSymbolTypeMapMax(int i)
{
  CQChartsUtil::testAndSet(symbolTypeData_.map_max, i, [&]() { updateObjs(); } );
}

//---

const CQChartsColumn &
CQChartsPointPlot::
symbolSizeColumn() const
{
  return symbolSizeData_.column;
}

void
CQChartsPointPlot::
setSymbolSizeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(symbolSizeData_.column, c, [&]() { updateRangeAndObjs(); } );
}

bool
CQChartsPointPlot::
isSymbolSizeMapped() const
{
  return symbolSizeData_.mapped;
}

void
CQChartsPointPlot::
setSymbolSizeMapped(bool b)
{
  CQChartsUtil::testAndSet(symbolSizeData_.mapped, b, [&]() { updateRangeAndObjs(); } );
}

double
CQChartsPointPlot::
symbolSizeMapMin() const
{
  return symbolSizeData_.map_min;
}

void
CQChartsPointPlot::
setSymbolSizeMapMin(double r)
{
  CQChartsUtil::testAndSet(symbolSizeData_.map_min, r, [&]() { updateRangeAndObjs(); } );
}

double
CQChartsPointPlot::
symbolSizeMapMax() const
{
  return symbolSizeData_.map_max;
}

void
CQChartsPointPlot::
setSymbolSizeMapMax(double r)
{
  CQChartsUtil::testAndSet(symbolSizeData_.map_max, r, [&]() { updateRangeAndObjs(); } );
}

const QString &
CQChartsPointPlot::
symbolSizeMapUnits() const
{
  return symbolSizeData_.units;
}

void
CQChartsPointPlot::
setSymbolSizeMapUnits(const QString &s)
{
  CQChartsUtil::testAndSet(symbolSizeData_.units, s, [&]() { updateRangeAndObjs(); } );
}

//---

const CQChartsColumn &
CQChartsPointPlot::
fontSizeColumn() const
{
  return fontSizeData_.column;
}

void
CQChartsPointPlot::
setFontSizeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(fontSizeData_.column, c, [&]() { updateRangeAndObjs(); } );
}

bool
CQChartsPointPlot::
isFontSizeMapped() const
{
  return fontSizeData_.mapped;
}

void
CQChartsPointPlot::
setFontSizeMapped(bool b)
{
  CQChartsUtil::testAndSet(fontSizeData_.mapped, b, [&]() { updateRangeAndObjs(); } );
}

double
CQChartsPointPlot::
fontSizeMapMin() const
{
  return fontSizeData_.map_min;
}

void
CQChartsPointPlot::
setFontSizeMapMin(double r)
{
  CQChartsUtil::testAndSet(fontSizeData_.map_min, r, [&]() { updateRangeAndObjs(); } );
}

double
CQChartsPointPlot::
fontSizeMapMax() const
{
  return fontSizeData_.map_max;
}

void
CQChartsPointPlot::
setFontSizeMapMax(double r)
{
  CQChartsUtil::testAndSet(fontSizeData_.map_max, r, [&]() { updateRangeAndObjs(); } );
}

const QString &
CQChartsPointPlot::
fontSizeMapUnits() const
{
  return fontSizeData_.units;
}

void
CQChartsPointPlot::
setFontSizeMapUnits(const QString &s)
{
  CQChartsUtil::testAndSet(fontSizeData_.units, s, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsPointPlot::
initSymbolTypeData() const
{
  auto *th = const_cast<CQChartsPointPlot *>(this);

  CQChartsPlot::initSymbolTypeData(th->symbolTypeData_);
}

bool
CQChartsPointPlot::
columnSymbolType(int row, const QModelIndex &parent, CQChartsSymbol &symbolType) const
{
  return CQChartsPlot::columnSymbolType(row, parent, symbolTypeData_, symbolType);
}

//------

void
CQChartsPointPlot::
initSymbolSizeData() const
{
  auto *th = const_cast<CQChartsPointPlot *>(this);

  CQChartsPlot::initSymbolSizeData(th->symbolSizeData_);
}

bool
CQChartsPointPlot::
columnSymbolSize(int row, const QModelIndex &parent, Length &symbolSize) const
{
  return CQChartsPlot::columnSymbolSize(row, parent, symbolSizeData_, symbolSize);
}

//------

void
CQChartsPointPlot::
initFontSizeData() const
{
  auto *th = const_cast<CQChartsPointPlot *>(this);

  CQChartsPlot::initFontSizeData(th->fontSizeData_);
}

bool
CQChartsPointPlot::
columnFontSize(int row, const QModelIndex &parent, Length &fontSize) const
{
  return CQChartsPlot::columnFontSize(row, parent, fontSizeData_, fontSize);
}

//---

bool
CQChartsPointPlot::
isPointLabels() const
{
  return dataLabel()->isVisible();
}

void
CQChartsPointPlot::
setPointLabels(bool b)
{
  if (b != isPointLabels()) {
    dataLabel()->setVisible(b); updateRangeAndObjs();
  }
}

void
CQChartsPointPlot::
setDataLabelFont(const CQChartsFont &font)
{
  NoUpdate noUpdate(this);

  auto *dataLabel = this->dataLabel();

  CQChartsWidgetUtil::AutoDisconnect dataChangeDisconnect(
    dataLabel, SIGNAL(dataChanged()), this, SLOT(dataLabelChanged()));

  dataLabel->setTextFont(font);
}

//---

void
CQChartsPointPlot::
dataLabelChanged()
{
  // TODO: not enough info to optimize behavior so reload all objects
  updateObjs();
}

//---

void
CQChartsPointPlot::
setBestFit(bool b)
{
  CQChartsUtil::testAndSet(bestFitData_.visible, b, [&]() { resetBestFit(); updateObjs(); } );
}

void
CQChartsPointPlot::
setBestFitOutliers(bool b)
{
  CQChartsUtil::testAndSet(bestFitData_.includeOutliers, b, [&]() { resetBestFit(); drawObjs(); } );
}

void
CQChartsPointPlot::
setBestFitDeviation(bool b)
{
  CQChartsUtil::testAndSet(bestFitData_.showDeviation, b, [&]() { resetBestFit(); drawObjs(); } );
}

void
CQChartsPointPlot::
setBestFitOrder(int o)
{
  CQChartsUtil::testAndSet(bestFitData_.order, o, [&]() { resetBestFit(); drawObjs(); } );
}

void
CQChartsPointPlot::
setBestFitLayer(const DrawLayer &l)
{
  CQChartsUtil::testAndSet(bestFitData_.layer, l, [&]() {
    for (const auto &plotObj : plotObjs_) {
      auto *bestFitObj = dynamic_cast<CQChartsPointBestFitObj *>(plotObj);

      if (bestFitObj)
        bestFitObj->setDrawLayer((CQChartsPlotObj::DrawLayer) bestFitLayer());
    }

    drawObjs();
  } );
}

//---

void
CQChartsPointPlot::
setHull(bool b)
{
  CQChartsUtil::testAndSet(hullData_.visible, b, [&]() { updateObjs(); } );
}

void
CQChartsPointPlot::
setHullLayer(const DrawLayer &l)
{
  CQChartsUtil::testAndSet(hullData_.layer, l, [&]() {
    for (const auto &plotObj : plotObjs_) {
      auto *hullObj = dynamic_cast<CQChartsPointHullObj *>(plotObj);

      if (hullObj)
        hullObj->setDrawLayer((CQChartsPlotObj::DrawLayer) hullLayer());
    }

    drawObjs();
  } );
}

//---

CQChartsPointBestFitObj *
CQChartsPointPlot::
createBestFitObj(int groupInd, const QString &name, const ColorInd &ig, const ColorInd &is,
                 const BBox &rect) const
{
  return new CQChartsPointBestFitObj(this, groupInd, name, ig, is, rect);
}

void
CQChartsPointPlot::
drawBestFit(PaintDevice *device, const BestFit *fitData, const ColorInd &ic) const
{
  // calc fit shape at each pixel
  Polygon bpoly, poly, tpoly;

  auto pl = windowToPixel(Point(fitData->xmin(), 0));
  auto pr = windowToPixel(Point(fitData->xmax(), 0));

  double dx = std::max((pr.x - pl.x)/100, 1.0);

  for (double px = pl.x; px <= pr.x; px += dx) {
    if (isInterrupt())
      return;

    auto p1 = pixelToWindow(Point(px, 0.0));

    double y2 = fitData->interp(p1.x);

    auto p2 = Point(p1.x, y2);

    poly.addPoint(p2);

    // deviation curve above/below
    if (isBestFitDeviation()) {
      p2 = Point(p1.x, y2 - fitData->deviation());

      bpoly.addPoint(p2);

      p2 = Point(p1.x, y2 + fitData->deviation());

      tpoly.addPoint(p2);
    }
  }

  //---

  if (poly.size()) {
    // calc pen and brush
    PenBrush penBrush;

    auto strokeColor = interpBestFitStrokeColor(ic);
    auto fillColor   = interpBestFitFillColor  (ic);

    setPenBrush(penBrush,
      PenData  (isBestFitStroked(), strokeColor, bestFitStrokeAlpha(),
                bestFitStrokeWidth(), bestFitStrokeDash()),
      BrushData(isBestFitFilled(), fillColor, bestFitFillAlpha(),
                bestFitFillPattern()));

    updateObjPenBrushState(this, ic, penBrush, CQChartsPlot::DrawType::LINE);

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    //---

    // draw fit deviation shape
    if (isBestFitDeviation()) {
      Polygon dpoly;

      for (int i = 0; i < bpoly.size(); ++i) {
        if (isInterrupt())
          return;

        auto p = bpoly.point(i);

        dpoly.addPoint(p);
      }

      for (int i = tpoly.size() - 1; i >= 0; --i) {
        if (isInterrupt())
          return;

        auto p = tpoly.point(i);

        dpoly.addPoint(p);
      }

      device->drawPolygon(dpoly);
    }

    //---

    // draw fit line
    auto path = CQChartsDrawUtil::polygonToPath(poly, /*closed*/false);

    device->strokePath(path, penBrush.pen);
  }
}

void
CQChartsPointPlot::
initGroupBestFit(BestFit *fitData, int ind, const QVariant &var, bool isGroup) const
{
  // init best fit data
  if (! fitData->isFitted()) {
    auto points = indPoints(var, isGroup);

    if (! isBestFitOutliers()) {
      initGroupStats(ind, var, isGroup);

      //---

      auto ps = groupStatData_.find(ind);
      assert(ps != groupStatData_.end());

      const auto &statData = (*ps).second;

      //---

      Polygon poly;

      for (const auto &p : points) {
        if (! statData.xstat.isOutlier(p.x) && ! statData.ystat.isOutlier(p.y))
          poly.addPoint(p);
      }

      //---

      fitData->calc(poly, bestFitOrder());
    }
    else {
      fitData->calc(points, bestFitOrder());
    }
  }
}

void
CQChartsPointPlot::
initGroupStats(int ind, const QVariant &var, bool isGroup) const
{
  // init stats data
  auto *th = const_cast<CQChartsPointPlot *>(this);

  auto &statData = th->groupStatData_[ind];

  if (! statData.xstat.set || ! statData.ystat.set) {
    auto points = indPoints(var, isGroup);

    std::vector<double> x, y;

    for (std::size_t i = 0; i < points.size(); ++i) {
      x.push_back(points[i].x);
      y.push_back(points[i].y);
    }

    std::sort(x.begin(), x.end());
    std::sort(y.begin(), y.end());

    statData.xstat.calcStatValues(x);
    statData.ystat.calcStatValues(y);
  }
}

//---

CQChartsPointHullObj *
CQChartsPointPlot::
createHullObj(int groupInd, const QString &name, const ColorInd &ig, const ColorInd &is,
              const BBox &rect) const
{
  return new CQChartsPointHullObj(this, groupInd, name, ig, is, rect);
}

CQChartsPointPlot::Points
CQChartsPointPlot::
indPoints(const QVariant &var, int isGroup) const
{
  Points points;

  if (isGroup) {
    int groupInd = var.toInt();

    auto p = groupPoints_.find(groupInd);
    if (p == groupPoints_.end()) return points;

    points = (*p).second;
  }
  else {
    const auto &nameValues = (*groupNameValues_.begin()).second;

    auto pn = nameValues.find(var.toString());
    if (pn == nameValues.end()) return points;

    const auto &values = (*pn).second.values;

    for (const auto &v : values)
      points.push_back(v.p);
  }

  return points;
}

//---

void
CQChartsPointPlot::
setStatsLinesSlot(bool b)
{
  if (b != isStatsLines()) {
    setStatsLines(b);

    drawObjs();
  }
}

//---

bool
CQChartsPointPlot::
isXRug() const
{
  return (xRug_ && xRug_->isVisible());
}

void
CQChartsPointPlot::
setXRug(bool b)
{
  if (b != isXRug()) { xRug_->setVisible(b); drawObjs(); }
}

const CQChartsAxisRug::Side &
CQChartsPointPlot::
xRugSide() const
{
  return xRug_->side();
}

void
CQChartsPointPlot::
setXRugSide(const CQChartsAxisRug::Side &s)
{
  if (s != xRugSide()) { xRug_->setSide(s); drawObjs(); }
}

const CQChartsSymbol &
CQChartsPointPlot::
xRugSymbolType() const
{
  return xRug_->symbolType();
}

void
CQChartsPointPlot::
setXRugSymbolType(const CQChartsSymbol &s)
{
  if (s != xRugSymbolType()) { xRug_->setSymbolType(s); drawObjs(); }
}

const CQChartsLength &
CQChartsPointPlot::
xRugSymbolSize() const
{
  return xRug_->symbolSize();
}

void
CQChartsPointPlot::
setXRugSymbolSize(const CQChartsLength &l)
{
  if (l != xRugSymbolSize()) { xRug_->setSymbolSize(l); drawObjs(); }
}

//---

bool
CQChartsPointPlot::
isYRug() const
{
  return (yRug_ && yRug_->isVisible());
}

void
CQChartsPointPlot::
setYRug(bool b)
{
  if (b != isYRug()) { yRug_->setVisible(b); drawObjs(); }
}

const CQChartsAxisRug::Side &
CQChartsPointPlot::
yRugSide() const
{
  return yRug_->side();
}

void
CQChartsPointPlot::
setYRugSide(const CQChartsAxisRug::Side &s)
{
  if (s != yRugSide()) { yRug_->setSide(s); drawObjs(); }
}

const CQChartsSymbol &
CQChartsPointPlot::
yRugSymbolType() const
{
  return yRug_->symbolType();
}

void
CQChartsPointPlot::
setYRugSymbolType(const CQChartsSymbol &s)
{
  if (s != yRugSymbolType()) { yRug_->setSymbolType(s); drawObjs(); }
}

const CQChartsLength &
CQChartsPointPlot::
yRugSymbolSize() const
{
  return yRug_->symbolSize();
}

void
CQChartsPointPlot::
setYRugSymbolSize(const CQChartsLength &l)
{
  if (l != yRugSymbolSize()) { yRug_->setSymbolSize(l); drawObjs(); }
}

//---

void
CQChartsPointPlot::
write(std::ostream &os, const QString &plotVarName, const QString &modelVarName,
      const QString &viewVarName) const
{
  CQChartsPlot::write(os, plotVarName, modelVarName, viewVarName);

  dataLabel_->write(os, plotVarName);
}

//------

CQChartsPointBestFitObj::
CQChartsPointBestFitObj(const CQChartsPointPlot *plot, int groupInd, const QString &name,
                        const ColorInd &ig, const ColorInd &is, const BBox &rect) :
 CQChartsPlotObj(const_cast<CQChartsPointPlot *>(plot), rect, is,
                 ig, ColorInd()), plot_(plot), groupInd_(groupInd), name_(name)
{
  setDetailHint(DetailHint::MAJOR);
}

QString
CQChartsPointBestFitObj::
calcId() const
{
  return QString("%1:%2:%3").arg(typeName()).arg(ig().i).arg(is().i);
}

QString
CQChartsPointBestFitObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  auto *bestFit = getBestFit();

  auto dev = bestFit->deviation();

  QString groupName;

  if (name_ == "")
    groupName = plot_->groupIndName(groupInd_);
  else {
    ColorInd ind;

    groupName = plot_->singleGroupName(ind);

    if (groupName == "")
      groupName = name_;
  }

  tableTip.addBoldLine("Best Fit");
  tableTip.addTableRow("Group"    , groupName);
  tableTip.addTableRow("Deviation", dev);

  return tableTip.str();
}

void
CQChartsPointBestFitObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  CQChartsPlotObj::addProperties(model, path);
}

void
CQChartsPointBestFitObj::
draw(PaintDevice *device) const
{
  auto *bestFit = getBestFit();

  if (name_ == "")
    plot_->drawBestFit(device, bestFit, ig_);
  else
    plot_->drawBestFit(device, bestFit, is_);
}

CQChartsFitData *
CQChartsPointBestFitObj::
getBestFit() const
{
  bool created;

  BestFit *fitData = nullptr;

  if (name_ == "") {
    // get fit data for group (add if needed)
    fitData = plot_->getBestFit(groupInd_, created);

    if (created)
      plot_->initGroupBestFit(fitData, ig_.i, QVariant(groupInd_), /*isGroup*/true);
  }
  else {
    // get fit data for set (add if needed)
    fitData = plot_->getBestFit(is_.i, created);

    if (created)
      plot_->initGroupBestFit(fitData, is_.i, QVariant(name_), /*isGroup*/false);
  }

  return fitData;
}

//------

CQChartsPointHullObj::
CQChartsPointHullObj(const CQChartsPointPlot *plot, int groupInd, const QString &name,
                     const ColorInd &ig, const ColorInd &is, const BBox &rect) :
 CQChartsPlotObj(const_cast<CQChartsPointPlot *>(plot), rect, is,
                 ig, ColorInd()), plot_(plot), groupInd_(groupInd), name_(name)
{
  setDetailHint(DetailHint::MAJOR);
}

QString
CQChartsPointHullObj::
calcId() const
{
  return QString("%1:%2:%3").arg(typeName()).arg(ig().i).arg(is().i);
}

QString
CQChartsPointHullObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  auto *hull = getHull();

  auto area = hull->area();

  QString groupName;

  if (name_ == "")
    groupName = plot_->groupIndName(groupInd_);
  else {
    ColorInd ind;

    groupName = plot_->singleGroupName(ind);

    if (groupName == "")
      groupName = name_;
  }

  tableTip.addBoldLine("Convex Hull");
  tableTip.addTableRow("Group", groupName);
  tableTip.addTableRow("Area" , area);

  return tableTip.str();
}

void
CQChartsPointHullObj::
addProperties(CQPropertyViewModel *model, const QString &path)
{
  CQChartsPlotObj::addProperties(model, path);
}

void
CQChartsPointHullObj::
draw(PaintDevice *device) const
{
  // draw hull
  auto drawHullData = [&](const Hull *hull, const ColorInd &ic) {
    // set pen/brush
    PenBrush penBrush;

    plot_->setPenBrush(penBrush, plot_->hullPenData(ic), plot_->hullBrushData(ic));

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    hull->draw(device);
  };

  //---

  auto *hull = getHull();

  if (name_ != "")
    drawHullData(hull, is_);
  else
    drawHullData(hull, ig_);
}

CQChartsGrahamHull *
CQChartsPointHullObj::
getHull() const
{
  using Points = std::vector<Point>;

  auto addHullPoints = [&](Hull *hull, const Points &points) {
    for (const auto &p : points)
      hull->addPoint(p);
  };

  //---

  bool created;

  Hull *hull = nullptr;

  if (name_ == "") {
    // get hull for group (add if needed)
    hull = plot_->getHull(groupInd_, created);

    if (created) {
      const auto &points = plot_->indPoints(QVariant(groupInd_), /*isGroup*/true);

      addHullPoints(hull, points);
    }
  }
  else {
    // get hull for set (add if needed)
    hull = plot_->getHull(is_.i, created);

    if (created) {
      const auto &points = plot_->indPoints(QVariant(name_), /*isGroup*/false);

      addHullPoints(hull, points);
    }
  }

  return hull;
}
