#include <CQChartsParallelPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsUtil.h>
#include <CQChartsTip.h>
#include <CQCharts.h>
#include <CQChartsDisplayRange.h>
#include <QPainter>

CQChartsParallelPlotType::
CQChartsParallelPlotType()
{
}

void
CQChartsParallelPlotType::
addParameters()
{
  addColumnParameter ("x", "X", "xColumn" , 0  ).setRequired();
  addColumnsParameter("y", "Y", "yColumns", "1").setRequired();

  CQChartsPlotType::addParameters();
}

CQChartsPlot *
CQChartsParallelPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsParallelPlot(view, model);
}

//---

CQChartsParallelPlot::
CQChartsParallelPlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("parallel"), model)
{
  pointData_.visible = true;

  //addKey(); TODO

  addTitle();
}

CQChartsParallelPlot::
~CQChartsParallelPlot()
{
  for (auto &yAxis : yAxes_)
    delete yAxis;
}

QString
CQChartsParallelPlot::
yColumnsStr() const
{
  return CQChartsColumn::columnsToString(yColumns_);
}

bool
CQChartsParallelPlot::
setYColumnsStr(const QString &s)
{
  Columns yColumns;

  if (! CQChartsColumn::stringToColumns(s, yColumns))
    return false;

  setYColumns(yColumns);

  return true;
}

const CQChartsColor &
CQChartsParallelPlot::
pointsColor() const
{
  return pointData_.stroke.color;
}

void
CQChartsParallelPlot::
setPointsColor(const CQChartsColor &c)
{
  pointData_.stroke.color = c;

  update();
}

QColor
CQChartsParallelPlot::
interpPointsColor(int i, int n) const
{
  return pointsColor().interpColor(this, i, n);
}

const CQChartsColor &
CQChartsParallelPlot::
linesColor() const
{
  return lineData_.color;
}

void
CQChartsParallelPlot::
setLinesColor(const CQChartsColor &c)
{
  lineData_.color = c;

  update();
}

QColor
CQChartsParallelPlot::
interpLinesColor(int i, int n) const
{
  return linesColor().interpColor(this, i, n);
}

QString
CQChartsParallelPlot::
symbolName() const
{
  return CQChartsPlotSymbolMgr::typeToName(pointData_.type);
}

void
CQChartsParallelPlot::
setSymbolName(const QString &s)
{
  CQChartsPlotSymbol::Type type = CQChartsPlotSymbolMgr::nameToType(s);

  if (type != CQChartsPlotSymbol::Type::NONE) {
    pointData_.type = type;

    update();
  }
}

//------

void
CQChartsParallelPlot::
addProperties()
{
  CQChartsPlot::addProperties();

  addProperty("columns", this, "xColumn" , "x"   );
  addProperty("columns", this, "yColumn" , "y"   );
  addProperty("columns", this, "yColumns", "yset");

  addProperty("points" , this, "points"      , "visible");
  addProperty("points" , this, "pointsColor" , "color"  );
  addProperty("points" , this, "symbolName"  , "symbol" );
  addProperty("points" , this, "symbolSize"  , "size"   );
  addProperty("points" , this, "symbolFilled", "filled" );

  addProperty("lines", this, "lines"     , "visible");
  addProperty("lines", this, "linesColor", "color"  );
  addProperty("lines", this, "linesWidth", "width"  );
}

void
CQChartsParallelPlot::
updateRange(bool apply)
{
  QAbstractItemModel *model = this->model();

  if (! model)
    return;

  for (int j = 0; j < numSets(); ++j) {
    CQChartsAxis *axis = new CQChartsAxis(this, CQChartsAxis::Direction::VERTICAL, 0, 1);

    yAxes_.push_back(axis);
  }

  //---

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsParallelPlot *plot) :
     plot_(plot) {
      ns_ = plot_->numSets();

      for (int i = 0; i < ns_; ++i)
        yRanges_.emplace_back();
    }

    State visit(QAbstractItemModel *model, const QModelIndex &parent, int row) override {
      for (int i = 0; i < ns_; ++i) {
        CQChartsGeom::Range &range = yRanges_[i];

        const CQChartsColumn &yColumn = plot_->getSetColumn(i);

        //---

        bool ok;

        double x = 0;
        double y = plot_->modelReal(model, row, yColumn, parent, ok);

        if (! ok)
          y = i;

        if (CQChartsUtil::isNaN(y))
          continue;

        range.updateRange(x, y);
      }

      return State::OK;
    }

    const Ranges &yRanges() const { return yRanges_; }

   private:
    CQChartsParallelPlot *plot_ { nullptr };
    int                   ns_   { 0 };
    Ranges                yRanges_;
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  yRanges_ = visitor.yRanges();

  //---

  for (int j = 0; j < numSets(); ++j) {
    CQChartsGeom::Range &range = yRanges_[j];

    range.updateRange(          - 0.5, range.ymin());
    range.updateRange(numSets() - 0.5, range.ymax());
  }

  dataRange_.updateRange(          - 0.5, 0);
  dataRange_.updateRange(numSets() - 0.5, 1);

  //---

  for (int j = 0; j < numSets(); ++j) {
    const CQChartsGeom::Range &range = yRange(j);

    const CQChartsColumn &yColumn = getSetColumn(j);

    bool ok;

    QString name = modelHeaderString(model, yColumn, ok);

    setDataRange(range);

    yAxes_[j]->setRange(dataRange_.ymin(), dataRange_.ymax());
    yAxes_[j]->setLabel(name);
  }

  //---

  displayRange_->setWindowRange(-0.5, 0, numSets() - 0.5, 1);

  //---

  if (apply)
    applyDataRange();
}

bool
CQChartsParallelPlot::
initObjs()
{
  if (! dataRange_.isSet()) {
    updateRange();

    if (! dataRange_.isSet())
      return false;
  }

  //---

  if (! plotObjs_.empty())
    return false;

  //---

  QAbstractItemModel *model = this->model();

  if (! model)
    return false;

  //---

  using Polygons = std::vector<QPolygonF>;
  using Indices  = std::vector<QModelIndex>;

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsParallelPlot *plot) :
     plot_(plot) {
      ns_ = plot_->numSets();
    }

    State visit(QAbstractItemModel *model, const QModelIndex &parent, int row) override {
      QPolygonF poly;

      QModelIndex xind = model->index(row, plot_->xColumn().column(), parent);

      xinds_.push_back(xind);

      //---

      for (int i = 0; i < ns_; ++i) {
        const CQChartsColumn &yColumn = plot_->getSetColumn(i);

        //---

        bool ok;

        double x = i;
        double y = plot_->modelReal(model, row, yColumn, parent, ok);

        if (! ok)
          y = i;

        if (CQChartsUtil::isNaN(y))
          continue;

        poly << QPointF(x, y);
      }

      polys_.push_back(poly);

      return State::OK;
    }

    const Polygons &polys() const { return polys_; }

    const Indices &xinds() const { return xinds_; }

   private:
    CQChartsParallelPlot *plot_ { nullptr };
    int                   ns_   { 0 };
    Polygons              polys_;
    Indices               xinds_;
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  const Polygons &polys = visitor.polys();
  const Indices  &xinds = visitor.xinds();

  //---

  // TODO: use actual symbol size
  double sw = (dataRange_.xmax() - dataRange_.xmin())/100.0;
  double sh = (dataRange_.ymax() - dataRange_.ymin())/100.0;

  int n = polys.size();

  for (int i = 0; i < n; ++i) {
    const QPolygonF   &poly = polys[i];
    const QModelIndex &xind = xinds[i];

    QModelIndex xind1 = normalizeIndex(xind);

    //---

    bool ok;

    QString xname = modelString(model, xind.row(), xind.column(), xind.parent(), ok);

    CQChartsGeom::BBox bbox(-0.5, 0, numSets() - 0.5, 1);

    CQChartsParallelLineObj *lineObj =
      new CQChartsParallelLineObj(this, bbox, poly, xind1, i, n);

    addPlotObject(lineObj);

    //---

    int nl = poly.count();

    for (int j = 0; j < nl; ++j) {
      const CQChartsColumn &yColumn = getSetColumn(j);

      QModelIndex yind  = model->index(i, yColumn.column(), xind.parent());
      QModelIndex yind1 = normalizeIndex(yind);

      //---

      const CQChartsGeom::Range &range = yRange(j);

      const QPointF &p = poly[j];

      double y1 = (p.y() - range.ymin())/(range.ymax() - range.ymin());

      CQChartsGeom::BBox bbox(j - sw/2, y1 - sh/2, j + sw/2, y1 + sh/2);

      CQChartsParallelPointObj *pointObj =
        new CQChartsParallelPointObj(this, bbox, j, y1, yind1, i, n, j, nl);

      bool ok;

      QString yname = modelHeaderString(model, yColumn, ok);

      QString id = QString("%1:%2=%3").arg(xname).arg(yname).arg(p.y());

      pointObj->setId(id);

      addPlotObject(pointObj);
    }
  }

  //---

  return true;
}

int
CQChartsParallelPlot::
numSets() const
{
  if (yColumns_.empty())
    return 1;

  return yColumns_.size();
}

const CQChartsColumn &
CQChartsParallelPlot::
getSetColumn(int i) const
{
  if (! yColumns_.empty())
    return yColumns_[i];
  else
    return yColumn_;
}

bool
CQChartsParallelPlot::
probe(ProbeData &probeData) const
{
  int n = numSets();

  int x = std::round(probeData.x);

  x = std::max(x, 0    );
  x = std::min(x, n - 1);

  const CQChartsGeom::Range &range = yRanges_[x];

  probeData.x = x;

  probeData.yvals.emplace_back(probeData.y,
    QString("%1").arg(probeData.y*range.ysize() + range.ymin()));

  return true;
}

void
CQChartsParallelPlot::
draw(QPainter *painter)
{
  initPlotObjs();

  //---

  drawBackground(painter);

  //---

  QFontMetricsF fm(view()->font());

  displayRange_->setWindowRange(dataRange_.xmin(), 0, dataRange_.xmax(), 1);

  drawObjs(painter, Layer::MID);

  for (int j = 0; j < numSets(); ++j) {
    const CQChartsGeom::Range &range = yRange(j);

    dataRange_ = range;
    //setDataRange(range); // will clear objects

    displayRange_->setWindowRange(-0.5, dataRange_.ymin(), numSets() - 0.5, dataRange_.ymax());

    yAxes_[j]->setPos(j);

    yAxes_[j]->draw(this, painter);

    QString label = yAxes_[j]->label();

    double px, py;

    windowToPixel(j, dataRange_.ymax(), px, py);

    painter->setPen(yAxes_[j]->interpTickLabelColor(0, 1));

    painter->drawText(QPointF(px - fm.width(label)/2.0, py - fm.height()), label);
  }

  displayRange_->setWindowRange(-0.5, 0, numSets() - 0.5, 1);

  //---

  drawTitle(painter);
}

//------

CQChartsParallelLineObj::
CQChartsParallelLineObj(CQChartsParallelPlot *plot, const CQChartsGeom::BBox &rect,
                        const QPolygonF &poly, const QModelIndex &ind, int i, int n) :
 CQChartsPlotObj(plot, rect), plot_(plot), poly_(poly), ind_(ind), i_(i), n_(n)
{
}

QString
CQChartsParallelLineObj::
calcId() const
{
  bool ok;

  QString xname =
    plot_->modelString(plot_->model(), ind_.row(), plot_->xColumn(), ind_.parent(), ok);

  return xname;
}

QString
CQChartsParallelLineObj::
calcTipId() const
{
  bool ok;

  QString xname =
    plot_->modelString(plot_->model(), ind_.row(), plot_->xColumn(), ind_.parent(), ok);

  CQChartsTableTip tableTip;

  tableTip.addBoldLine(xname);

  int nl = poly_.count();

  for (int j = 0; j < nl; ++j) {
    const CQChartsColumn &yColumn = plot_->getSetColumn(j);

    bool ok;

    QString yname = plot_->modelHeaderString(plot_->model(), yColumn, ok);

    tableTip.addTableRow(yname, poly_[j].y());
  }

  return tableTip.str();
}

bool
CQChartsParallelLineObj::
visible() const
{
  if (! plot_->isLines())
    return false;

  return isVisible();
}

bool
CQChartsParallelLineObj::
inside(const CQChartsGeom::Point &p) const
{
  if (! visible())
    return false;

  QPolygonF poly;

  for (int i = 0; i < poly_.count(); ++i) {
    const CQChartsGeom::Range &range = plot_->yRange(i);

    double x = poly_[i].x();
    double y = (poly_[i].y() - range.ymin())/range.ysize();

    poly << QPointF(x, y);
  }

  for (int i = 1; i < poly.count(); ++i) {
    double x1 = poly[i - 1].x();
    double y1 = poly[i - 1].y();
    double x2 = poly[i    ].x();
    double y2 = poly[i    ].y();

    double d;

    CQChartsGeom::Point pl1(x1, y1);
    CQChartsGeom::Point pl2(x2, y2);

    if (CQChartsUtil::PointLineDistance(p, pl1, pl2, &d) && d < 1E-3)
      return true;
  }

  return false;
}

// TODO : interpY

void
CQChartsParallelLineObj::
getSelectIndices(Indices &inds) const
{
  addSelectIndex(inds, ind_.row(), plot_->xColumn(), ind_.parent());
  addSelectIndex(inds, ind_.row(), plot_->yColumn(), ind_.parent());
}

void
CQChartsParallelLineObj::
draw(QPainter *painter, const CQChartsPlot::Layer &)
{
  if (! visible())
    return;

  QPolygonF poly;

  for (int i = 0; i < poly_.count(); ++i) {
    const CQChartsGeom::Range &range = plot_->yRange(i);

    double x = poly_[i].x();
    double y = (poly_[i].y() - range.ymin())/range.ysize();

    poly << QPointF(x, y);
  }

  QColor lc = plot_->interpPaletteColor(i_, n_, /*scale*/false);
  double lw = plot_->lengthPixelWidth(plot_->linesWidth());

  QBrush brush(Qt::NoBrush);
  QPen   pen(lc);

  if (lw > 0)
    pen.setWidthF(lw);

  plot_->updateObjPenBrushState(this, pen, brush);

#if 0
  if (isInside()) {
    if (lw <= 0)
      lw = 1;

    lw *= 3;
  }
#endif

  painter->setPen(pen);

  for (int i = 1; i < poly.count(); ++i) {
    double x1 = poly[i - 1].x();
    double y1 = poly[i - 1].y();
    double x2 = poly[i    ].x();
    double y2 = poly[i    ].y();

    double px1, py1, px2, py2;

    plot_->windowToPixel(x1, y1, px1, py1);
    plot_->windowToPixel(x2, y2, px2, py2);

    painter->drawLine(QPointF(px1, py1), QPointF(px2, py2));
  }
}

//------

CQChartsParallelPointObj::
CQChartsParallelPointObj(CQChartsParallelPlot *plot, const CQChartsGeom::BBox &rect,
                         double x, double y, const QModelIndex &ind, int iset, int nset,
                         int i, int n) :
 CQChartsPlotObj(plot, rect), plot_(plot), x_(x), y_(y), ind_(ind), iset_(iset), nset_(nset),
 i_(i), n_(n)
{
}

QString
CQChartsParallelPointObj::
calcId() const
{
  bool ok;

  QString xname =
    plot_->modelString(plot_->model(), ind_.row(), plot_->xColumn(), ind_.parent(), ok);

  const CQChartsColumn &yColumn = plot_->getSetColumn(i_);

  QString yname = plot_->modelHeaderString(plot_->model(), yColumn, ok);

  return QString("%1:%2=%3").arg(xname).arg(yname).arg(y_);
}

QString
CQChartsParallelPointObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  bool ok;

  QString xname =
    plot_->modelString(plot_->model(), ind_.row(), plot_->xColumn(), ind_.parent(), ok);

  tableTip.addBoldLine(xname);

  const CQChartsColumn &yColumn = plot_->getSetColumn(i_);

  QString yname = plot_->modelHeaderString(plot_->model(), yColumn, ok);

  tableTip.addTableRow(yname, y_);

  return tableTip.str();
}

bool
CQChartsParallelPointObj::
visible() const
{
  if (! plot_->isPoints())
    return false;

  return isVisible();
}

bool
CQChartsParallelPointObj::
inside(const CQChartsGeom::Point &p) const
{
  if (! visible())
    return false;

  double px, py;

  plot_->windowToPixel(x_, y_, px, py);

  double s = plot_->symbolSize();

  CQChartsGeom::BBox pbbox(px - s, py - s, px + s, py + s);

  CQChartsGeom::Point pp;

  plot_->windowToPixel(p, pp);

  return pbbox.inside(pp);
}

void
CQChartsParallelPointObj::
getSelectIndices(Indices &inds) const
{
  addSelectIndex(inds, ind_);
}

void
CQChartsParallelPointObj::
draw(QPainter *painter, const CQChartsPlot::Layer &)
{
  if (! visible())
    return;

  CQChartsPlotSymbol::Type symbol = plot_->symbolType();
  bool                     filled = plot_->isSymbolFilled();
  double                   s      = plot_->symbolSize();

  QColor c = plot_->interpPaletteColor(i_, n_, /*scale*/false);

  CQChartsGeom::Point pp(x_, y_);

  double px, py;

  plot_->windowToPixel(pp.x, pp.y, px, py);

  if (isInside() || isSelected())
    s *= 2;

  plot_->drawSymbol(painter, QPointF(px, py), symbol, s, true, c, 1, filled, c);
}
