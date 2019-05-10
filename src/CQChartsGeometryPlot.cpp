#include <CQChartsGeometryPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelUtil.h>
#include <CQChartsPolygonList.h>
#include <CQChartsPath.h>
#include <CQChartsDataLabel.h>
#include <CQCharts.h>
#include <CQChartsTip.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <QPainter>

CQChartsGeometryPlotType::
CQChartsGeometryPlotType()
{
}

void
CQChartsGeometryPlotType::
addParameters()
{
  startParameterGroup("Geometry");

  addColumnParameter("geometry", "Geometry", "geometryColumn").
   setRequired().setTip("Polygon List, Polygon, Rect or Path geometry");
  addColumnParameter("value", "Value", "valueColumn").setRequired();

  addColumnParameter("name" , "Name" , "nameColumn" );
  addColumnParameter("style", "Style", "styleColumn");

  endParameterGroup();

  //---

  CQChartsPlotType::addParameters();
}

QString
CQChartsGeometryPlotType::
description() const
{
  return "<h2>Summary</h2>\n"
         "<p>Draw polygon list, polygon, rect or path shapes.</p>\n";
}

bool
CQChartsGeometryPlotType::
isColumnForParameter(CQChartsModelColumnDetails *columnDetails,
                     CQChartsPlotParameter *parameter) const
{
  if (parameter->name() == "geometry") {
    if (columnDetails->type() == CQChartsPlot::ColumnType::RECT ||
        columnDetails->type() == CQChartsPlot::ColumnType::POLYGON ||
        columnDetails->type() == CQChartsPlot::ColumnType::POLYGON_LIST ||
        columnDetails->type() == CQChartsPlot::ColumnType::PATH)
      return true;

    return false;
  }

  return CQChartsPlotType::isColumnForParameter(columnDetails, parameter);
}

CQChartsPlot *
CQChartsGeometryPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsGeometryPlot(view, model);
}

//------

CQChartsGeometryPlot::
CQChartsGeometryPlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("geometry"), model),
 CQChartsObjShapeData<CQChartsGeometryPlot>(this)
{
  NoUpdate noUpdate(this);

  dataLabel_ = new CQChartsDataLabel(this);

  setFillColor(CQChartsColor(CQChartsColor::Type::PALETTE));

  setFilled(true);
  setBorder(true);

  setLayerActive(CQChartsLayer::Type::FG_PLOT, true);

  addAxes();

  addTitle();
}

CQChartsGeometryPlot::
~CQChartsGeometryPlot()
{
  delete dataLabel_;
}

//---

void
CQChartsGeometryPlot::
setNameColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsGeometryPlot::
setGeometryColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(geometryColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsGeometryPlot::
setValueColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(valueColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsGeometryPlot::
setStyleColumn(const CQChartsColumn &c)
{
  CQChartsUtil::testAndSet(styleColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsGeometryPlot::
setColorByValue(bool b)
{
  CQChartsUtil::testAndSet(colorByValue_, b, [&]() { drawObjs(); } );
}

//---

double
CQChartsGeometryPlot::
minValue() const
{
  return minValue_.value_or(valueRange_.min(0.0));
}

void
CQChartsGeometryPlot::
setMinValue(double r)
{
  if (! minValue_ || r != minValue_.value()) {
    minValue_ = r; drawObjs();
  }
}

double
CQChartsGeometryPlot::
maxValue() const
{
  return maxValue_.value_or(valueRange_.max(1.0));
}

void
CQChartsGeometryPlot::
setMaxValue(double r)
{
  if (! maxValue_ || r != maxValue_.value()) {
    maxValue_ = r; drawObjs();
  }
}

//---

void
CQChartsGeometryPlot::
addProperties()
{
  CQChartsPlot::addProperties();

  // columns
  addProperty("columns", this, "nameColumn"    , "name"    )->setDesc("Name column");
  addProperty("columns", this, "geometryColumn", "geometry")->setDesc("Geometry column");
  addProperty("columns", this, "valueColumn"   , "value"   )->setDesc("Value column");
  addProperty("columns", this, "styleColumn"   , "style"   )->setDesc("Style column");

  // color
  addProperty("color", this, "colorByValue", "colorByValue")->setDesc("Color shapes by value");

  // fill
  addProperty("fill", this, "filled", "visible")->setDesc("Fill visible");

  addFillProperties("fill", "fill", "");

  // stroke
  addProperty("stroke", this, "border", "visible")->setDesc("Stroke visible");

  addLineProperties("stroke", "border", "");

  // data label
  dataLabel_->addPathProperties("dataLabel");

  // value
  addProperty("value", this, "minValue", "min")->setDesc("Min value for color map");
  addProperty("value", this, "maxValue", "max")->setDesc("Max value for color map");
}

void
CQChartsGeometryPlot::
getPropertyNames(QStringList &names, bool hidden) const
{
  CQChartsPlot::getPropertyNames(names, hidden);

  propertyModel()->objectNames(dataLabel_, names, hidden);
}

//---

CQChartsGeom::Range
CQChartsGeometryPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsGeometryPlot::calcRange");

  CQChartsGeom::Range dataRange;

  QAbstractItemModel *model = this->model().data();

  if (! model)
    return dataRange;

  //---

  CQChartsGeometryPlot *th = const_cast<CQChartsGeometryPlot *>(this);

  th->geometries_.clear();

  th->valueRange_ = CQChartsGeom::RMinMax();

  //---

  th->geometryColumnType_ = columnValueType(geometryColumn());
  th->colorColumnType_    = columnValueType(colorColumn());
  th->styleColumnType_    = columnValueType(styleColumn());

  //---

  // process model data
  class GeometryPlotVisitor : public ModelVisitor {
   public:
    GeometryPlotVisitor(const CQChartsGeometryPlot *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *model, const VisitData &data) override {
      plot_->addRow(model, data, dataRange_);

      return State::OK;
    }

    const CQChartsGeom::Range &dataRange() const { return dataRange_; }

   private:
    const CQChartsGeometryPlot *plot_ { nullptr };
    CQChartsGeom::Range         dataRange_;
  };

  GeometryPlotVisitor geometryPlotVisitor(this);

  visitModel(geometryPlotVisitor);

  return geometryPlotVisitor.dataRange();
}

void
CQChartsGeometryPlot::
addRow(const QAbstractItemModel *model, const ModelVisitor::VisitData &data,
       CQChartsGeom::Range &dataRange) const
{
  CQChartsGeometryPlot *th = const_cast<CQChartsGeometryPlot *>(this);

  //---

  Geometry geometry;

  //---

  // get geometry name
  bool ok1;

  geometry.name = modelString(data.row, nameColumn(), data.parent, ok1);

  //---

  // decode geometry column value into polygons
  if (geometryColumnType_ == ColumnType::RECT ||
      geometryColumnType_ == ColumnType::POLYGON ||
      geometryColumnType_ == ColumnType::POLYGON_LIST ||
      geometryColumnType_ == ColumnType::PATH) {
    bool ok2;

    QVariant var = modelValue(data.row, geometryColumn(), data.parent, ok2);

    bool converted;

    QVariant rvar =
      CQChartsModelUtil::columnUserData(charts(), model, geometryColumn(), var, converted);

    QPolygonF poly;

    if      (geometryColumnType_ == ColumnType::RECT) {
      QRectF r = rvar.value<QRectF>();

      poly = QPolygonF(r);

      geometry.polygons.push_back(poly);
    }
    else if (geometryColumnType_ == ColumnType::POLYGON) {
      poly = rvar.value<QPolygonF>();

      geometry.polygons.push_back(poly);
    }
    else if (geometryColumnType_ == ColumnType::POLYGON_LIST) {
      CQChartsPolygonList polyList = rvar.value<CQChartsPolygonList>();

      for (const auto &poly : polyList.polygons())
        geometry.polygons.push_back(poly);
    }
    else if (geometryColumnType_ == ColumnType::PATH) {
      CQChartsPath path = rvar.value<CQChartsPath>();

      poly = path.path().toFillPolygon();

      geometry.polygons.push_back(poly);
    }
    else {
      assert(false);
    }
  }
  else {
    bool ok2;

    QString geomStr = modelString(data.row, geometryColumn(), data.parent, ok2);

    if (! decodeGeometry(geomStr, geometry.polygons)) {
      if (! isPreview())
        charts()->errorMsg("Invalid geometry '" + geomStr + "' for '" + geometry.name + "'");
      return;
    }
  }

  //---

  // update range from polygons
  for (auto &poly : geometry.polygons) {
    for (int j = 0; j < poly.count(); ++j) {
      const QPointF &p = poly[j];

      dataRange.updateRange(p.x(), p.y());

      geometry.bbox.add(p.x(), p.y());
    }
  }

  //---

  // get geometry associated value
  bool ok3;

  geometry.value = modelReal(data.row, valueColumn(), data.parent, ok3);

  if (! ok3)
    geometry.value = data.row;

  if (CMathUtil::isNaN(geometry.value))
    return;

  // update value range
  th->valueRange_.add(geometry.value);

  //---

  // get geometry custom color
  if (colorColumn().isValid()) {
    if (colorColumnType_ == ColumnType::COLOR) {
      CQChartsColor c;

      if (columnColor(data.row, data.parent, c))
        geometry.color = c;
    }
    else {
      bool ok4;

      QString str = modelString(data.row, colorColumn(), data.parent, ok4);

      if (ok4)
        geometry.color = CQChartsColor(str);
    }
  }

  //---

  // get geometry custom style
  if (styleColumn().isValid()) {
    bool ok4;

    if (styleColumnType_ == ColumnType::STYLE) {
      QString str = modelString(data.row, styleColumn(), data.parent, ok4);

      geometry.style = CQChartsStyle(str);
    }
  }

  //---

  // save model index for geometry
  QModelIndex geomInd  = modelIndex(data.row, geometryColumn(), data.parent);
  QModelIndex geomInd1 = normalizeIndex(geomInd);

  geometry.ind = geomInd1;

  //---

  // add to list
  th->geometries_.push_back(geometry);
}

bool
CQChartsGeometryPlot::
decodeGeometry(const QString &geomStr, Polygons &polygons) const
{
  if (geomStr.simplified() == "")
    return true;

  // count leading braces
  int n = CQChartsUtil::countLeadingBraces(geomStr);

  //---

  // no braces - single polygon x1 y1 x2 y2 ...
  if      (n == 0) {
    QPolygonF poly;

    if (! CQChartsUtil::stringToPolygon("{{" + geomStr + "}}", poly))
      return false;

    polygons.push_back(poly);
  }
  // single brace - single polygon {x1 y1} {x2 y2} ...
  else if (n == 1) {
    QPolygonF poly;

    if (! CQChartsUtil::stringToPolygon("{" + geomStr + "}", poly))
      return false;

    polygons.push_back(poly);
  }
  // two braces - single polygon {{x1 y1} {x2 y2} ...}
  else if (n == 2) {
    QPolygonF poly;

    if (! CQChartsUtil::stringToPolygon(geomStr, poly))
      return false;

    polygons.push_back(poly);
  }
  // three braces - list of polygons {{{x1 y1} {x2 y2} ...} ... }
  else if (n == 3) {
    if (! CQChartsUtil::stringToPolygons(geomStr, polygons))
      return false;
  }
  else {
    return false;
  }

  return true;
}

bool
CQChartsGeometryPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsGeometryPlot::createObjs");

  CQChartsGeometryPlot *th = const_cast<CQChartsGeometryPlot *>(this);

  NoUpdate noUpdate(th);

  //---

  int n = geometries_.size();

  for (int i = 0; i < n; ++i) {
    const Geometry &geometry = geometries_[i];

    CQChartsGeom::BBox bbox = geometry.bbox;

    bool hasValue = valueColumn().isValid();

    ColorInd iv(i, n);

    CQChartsGeometryObj *geomObj =
      new CQChartsGeometryObj(this, bbox, geometry.polygons, geometry.ind, iv, hasValue);

    geomObj->setName (geometry.name);
    geomObj->setValue(geometry.value);
    geomObj->setColor(geometry.color);
    geomObj->setStyle(geometry.style);

    objs.push_back(geomObj);
  }

  //---

  return true;
}

//------

CQChartsGeometryObj::
CQChartsGeometryObj(const CQChartsGeometryPlot *plot, const CQChartsGeom::BBox &rect,
                    const Polygons &polygons, const QModelIndex &ind, const ColorInd &iv,
                    bool hasValue) :
 CQChartsPlotObj(const_cast<CQChartsGeometryPlot *>(plot), rect, ColorInd(), ColorInd(), iv),
 plot_(plot), polygons_(polygons), ind_(ind), hasValue_(hasValue)
{
}

QString
CQChartsGeometryObj::
calcId() const
{
  if (! plot_->valueColumn().isValid())
    return name();

  return QString("%1:%2:%3").arg(typeName()).arg(name()).arg(value());
}

QString
CQChartsGeometryObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Name" , name ());
  tableTip.addTableRow("Value", value());

  return tableTip.str();
}

bool
CQChartsGeometryObj::
inside(const CQChartsGeom::Point &p) const
{
  QPointF p1 = CQChartsUtil::toQPoint(p);

  for (const auto &poly : polygons_) {
    if (poly.containsPoint(p1, Qt::OddEvenFill))
      return true;
  }

  return false;
}

void
CQChartsGeometryObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, plot_->nameColumn    ());
  addColumnSelectIndex(inds, plot_->geometryColumn());
  addColumnSelectIndex(inds, plot_->valueColumn   ());
  addColumnSelectIndex(inds, plot_->styleColumn   ());
}

void
CQChartsGeometryObj::
addColumnSelectIndex(Indices &inds, const CQChartsColumn &column) const
{
  if (column.isValid())
    addSelectIndex(inds, ind_.row(), column, ind_.parent());
}

void
CQChartsGeometryObj::
draw(QPainter *painter)
{
  ppolygons_.clear();

  for (const auto &poly : polygons_) {
    QPolygonF ppoly;

    for (int i = 0; i < poly.count(); ++i) {
      QPointF p1 = plot_->windowToPixel(poly[i]);

      ppoly << p1;
    }

    ppolygons_.push_back(ppoly);
  }

  //---

  // set polygon pen/brush
  QPen   pen;
  QBrush brush;

  QColor fc;

  double dv = (value() - plot_->minValue())/(plot_->maxValue() - plot_->minValue());

  ColorInd colorInd = calcColorInd();

  if (color().isValid()) {
    if (! hasValue_ || ! plot_->isColorByValue())
      fc = plot_->interpColor(color(), colorInd);
    else
      fc = plot_->interpColor(color(), ColorInd(dv));
  }
  else {
    if (! hasValue_ || ! plot_->isColorByValue())
      fc = plot_->interpFillColor(colorInd);
    else
      fc = plot_->interpPaletteColor(ColorInd(dv));
  }

  QColor bc = plot_->interpBorderColor(colorInd);

  plot_->setPenBrush(pen, brush,
    plot_->isBorder(), bc, plot_->borderAlpha(), plot_->borderWidth(), plot_->borderDash(),
    plot_->isFilled(), fc, plot_->fillAlpha(), plot_->fillPattern());

  if (style().isValid()) {
    pen   = style().pen  ();
    brush = style().brush();
  }

  plot_->updateObjPenBrushState(this, pen, brush);

  //---

  painter->setPen  (pen);
  painter->setBrush(brush);

  for (const auto &ppoly : ppolygons_) {
    QPainterPath path;

    int i = 0;

    for (const auto &ppoint : ppoly) {
      if (i == 0)
        path.moveTo(ppoint);
      else
        path.lineTo(ppoint);

      ++i;
    }

    path.closeSubpath();

    painter->drawPath(path);
  }
}

void
CQChartsGeometryObj::
drawFg(QPainter *painter) const
{
  CQChartsGeom::BBox prect = plot_->windowToPixel(rect());

  QRectF qrect = CQChartsUtil::toQRect(prect);

  plot_->dataLabel()->draw(painter, qrect, name());
}
