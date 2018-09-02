#include <CQChartsImagePlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <QPainter>

CQChartsImagePlotType::
CQChartsImagePlotType()
{
}

void
CQChartsImagePlotType::
addParameters()
{
  CQChartsPlotType::addParameters();
}

QString
CQChartsImagePlotType::
description() const
{
  return "<h2>Summary</h2>\n"
         "<p>Draw 2d grid of 'pixels' from values in 2d table.</p>\n";
}

CQChartsPlot *
CQChartsImagePlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsImagePlot(view, model);
}

//------

CQChartsImagePlot::
CQChartsImagePlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("image"), model),
 CQChartsPlotTextData<CQChartsImagePlot>(this)
{
  addTitle();
}

void
CQChartsImagePlot::
addProperties()
{
  CQChartsPlot::addProperties();

  addProperty("labels", this, "xLabels"   , "x"    );
  addProperty("labels", this, "yLabels"   , "y"    );
  addProperty("labels", this, "cellLabels", "cell" );

  addTextProperties("label", "text");
}

//------

void
CQChartsImagePlot::
setMinValue(double r)
{
  CQChartsUtil::testAndSet(minValue_, r, [&]() { invalidateLayers(); } );
}

void
CQChartsImagePlot::
setMaxValue(double r)
{
  CQChartsUtil::testAndSet(maxValue_, r, [&]() { invalidateLayers(); } );
}

//---

void
CQChartsImagePlot::
setXLabels(bool b)
{
  CQChartsUtil::testAndSet(xLabels_, b, [&]() { invalidateLayers(); } );
}

void
CQChartsImagePlot::
setYLabels(bool b)
{
  CQChartsUtil::testAndSet(yLabels_, b, [&]() { invalidateLayers(); } );
}

void
CQChartsImagePlot::
setCellLabels(bool b)
{
  CQChartsUtil::testAndSet(cellLabels_, b, [&]() { invalidateLayers(); } );
}

//---

void
CQChartsImagePlot::
calcRange()
{
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsImagePlot *plot) :
     plot_(plot) {
    }

    State visit(QAbstractItemModel *, const QModelIndex &parent, int row) override {
      for (int col = 0; col < numCols(); ++col) {
        bool ok;

        double value = plot_->modelReal(row, col, parent, ok);

        if (! valueSet_) {
          minValue_ = value;
          maxValue_ = value;

          valueSet_ = true;
        }
        else {
          minValue_ = std::min(minValue_, value);
          maxValue_ = std::max(maxValue_, value);
        }
      }

      return State::OK;
    }

    double minValue() const { return minValue_; }
    double maxValue() const { return maxValue_; }

   private:
    CQChartsImagePlot *plot_     { nullptr };
    bool               valueSet_ { false };
    double             minValue_ { 0.0 };
    double             maxValue_ { 0.0 };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  // set value range
  minValue_ = visitor.minValue();

  if (xmin())
    minValue_ = *xmin();

  maxValue_ = visitor.maxValue();

  if (xmax())
    maxValue_ = *xmax();

  //---

  dataRange_.reset();

  nr_ = visitor.numRows();
  nc_ = visitor.numCols();

  dataRange_.updateRange(  0,   0);
  dataRange_.updateRange(nc_, nr_);
}

bool
CQChartsImagePlot::
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

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsImagePlot *plot) :
     plot_(plot) {
    }

    State visit(QAbstractItemModel *, const QModelIndex &parent, int row) override {
      x_ = 0.0;

      for (int col = 0; col < numCols(); ++col) {
        QModelIndex ind = plot_->modelIndex(row, col, parent);

        bool ok;

        double value = plot_->modelReal(row, col, parent, ok);

        //---

        plot_->addImageObj(row, col, x_, y_, dx_, dy_, value, ind);

        //---

        x_ += dx_;
      }

      y_ += dy_;

      return State::OK;
    }

   private:
    CQChartsImagePlot *plot_ { nullptr };
    double             x_    { 0.0 };
    double             y_    { 0.0 };
    double             dx_   { 1.0 };
    double             dy_   { 1.0 };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  return true;
}

void
CQChartsImagePlot::
addImageObj(int row, int col, double x, double y, double dx, double dy, double value,
            const QModelIndex &ind)
{
  QModelIndex ind1 = normalizeIndex(ind);

  CQChartsGeom::BBox bbox(x, y, x + dx, y + dy);

  CQChartsImageObj *imageObj = new CQChartsImageObj(this, bbox, row, col, value, ind1);

  addPlotObject(imageObj);
}

//------

bool
CQChartsImagePlot::
hasForeground() const
{
  if (! isXLabels() && ! isYLabels())
    return false;

  if (! isLayerActive(CQChartsLayer::Type::FOREGROUND))
    return false;

  return true;
}

void
CQChartsImagePlot::
drawForeground(QPainter *painter)
{
  if (isXLabels())
    drawXLabels(painter);

  if (isYLabels())
    drawYLabels(painter);
}

void
CQChartsImagePlot::
drawXLabels(QPainter *painter)
{
  view()->setPlotPainterFont(this, painter, textFont());

  //---

  QPen tpen;

  QColor tc = interpTextColor(0, 1);

  setPen(tpen, true, tc, textAlpha(), CQChartsLength("0px"), CQChartsLineDash());

  painter->setPen(tpen);

  //---

  CQChartsTextOptions textOptions;

//textOptions.contrast  = isTextContrast();
//textOptions.formatted = isTextFormatted();
//textOptions.scaled    = isTextScaled();
  textOptions.align     = Qt::AlignRight;
  textOptions.angle     = 90;

  QFontMetricsF fm(painter->font());

  double tw = 0.0;
//double th = fm.height();
  double tm = 4;

  for (int col = 0; col < nc_; ++col) {
    bool ok;

    QString name = modelHeaderString(col, Qt::Horizontal, ok);
    if (! name.length()) continue;

    tw = std::max(tw, fm.width(name));
  }

  for (int col = 0; col < nc_; ++col) {
    bool ok;

    QString name = modelHeaderString(col, Qt::Horizontal, ok);
    if (! name.length()) continue;

    double tw1 = fm.width(name);

    QPointF p(col + 0.5, 0);

    QPointF p1 = windowToPixel(p);

    QRectF trect(p1.x() - tw/2, p1.y() - tw1 - tm, tw, tw1);

    drawTextInBox(painter, trect, name, tpen, textOptions);
  }
}

void
CQChartsImagePlot::
drawYLabels(QPainter *painter)
{
  view()->setPlotPainterFont(this, painter, textFont());

  //---

  QPen tpen;

  QColor tc = interpTextColor(0, 1);

  setPen(tpen, true, tc, textAlpha(), CQChartsLength("0px"), CQChartsLineDash());

  painter->setPen(tpen);

  //---

  CQChartsTextOptions textOptions;

//textOptions.contrast  = isTextContrast();
//textOptions.formatted = isTextFormatted();
//textOptions.scaled    = isTextScaled();
  textOptions.align     = Qt::AlignRight;

  QFontMetricsF fm(painter->font());

  double tw = 0.0;
  double th = fm.height();
  double tm = 4;

  for (int row = 0; row < nr_; ++row) {
    bool ok;

    QString name = modelHeaderString(row, Qt::Vertical, ok);
    if (! name.length()) continue;

    tw = std::max(tw, fm.width(name));
  }

  for (int row = 0; row < nr_; ++row) {
    bool ok;

    QString name = modelHeaderString(row, Qt::Vertical, ok);
    if (! name.length()) continue;

    QPointF p(0, row + 0.5);

    QPointF p1 = windowToPixel(p);

    QRectF trect(p1.x() - tw - tm, p1.y() - th/2.0, tw, th);

    drawTextInBox(painter, trect, name, tpen, textOptions);
  }
}

//------

CQChartsGeom::BBox
CQChartsImagePlot::
annotationBBox() const
{
  QFont font = view()->plotFont(this, textFont());

  QFontMetricsF fm(font);

  CQChartsGeom::BBox bbox;

  double tm = 4;

  if (isXLabels()) {
    double tw = 0.0;

    for (int col = 0; col < nc_; ++col) {
      bool ok;

      QString name = modelHeaderString(col, Qt::Horizontal, ok);
      if (! name.length()) continue;

      tw = std::max(tw, fm.width(name));
    }

    double tw1 = pixelToWindowHeight(tw + tm);

    CQChartsGeom::BBox tbbox(0, -tw1, nc_, 0);

    bbox += tbbox;
  }

  if (isYLabels()) {
    double tw = 0.0;

    for (int row = 0; row < nr_; ++row) {
      bool ok;

      QString name = modelHeaderString(row, Qt::Vertical, ok);
      if (! name.length()) continue;

      tw = std::max(tw, fm.width(name));
    }

    double tw1 = pixelToWindowWidth(tw + tm);

    CQChartsGeom::BBox tbbox(-tw1, 0, 0, nr_);

    bbox += tbbox;
  }

  return bbox;
}

//------

CQChartsImageObj::
CQChartsImageObj(CQChartsImagePlot *plot, const CQChartsGeom::BBox &rect,
                 int row, int col, double value, const QModelIndex &ind) :
 CQChartsPlotObj(plot, rect), plot_(plot), row_(row), col_(col), value_(value), ind_(ind)
{
}

QString
CQChartsImageObj::
calcId() const
{
  return QString("image:%1:%2").arg(row_).arg(col_);
}

QString
CQChartsImageObj::
calcTipId() const
{
  bool ok;

  QString xname = plot_->modelHeaderString(ind_.column(), ok);
  QString yname = plot_->modelHeaderString(ind_.row(), Qt::Vertical, ok);

  QString tipStr;

  if (xname.length())
    tipStr += xname + " ";

  if (yname.length())
    tipStr += yname + " ";

  tipStr += QString("%1").arg(value_);

  return tipStr.simplified();
}

void
CQChartsImageObj::
getSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, ind_.column());
}

void
CQChartsImageObj::
addColumnSelectIndex(Indices &inds, const CQChartsColumn &column) const
{
  if (column.isValid())
    addSelectIndex(inds, ind_.row(), column, ind_.parent());
}

void
CQChartsImageObj::
draw(QPainter *painter)
{
  CQChartsGeom::BBox prect;

  plot_->windowToPixel(rect(), prect);

  QRectF qrect = CQChartsUtil::toQRect(prect);

  double v = CMathUtil::norm(value_, plot_->minValue(), plot_->maxValue());

  //---

  QColor c = plot_->interpPaletteColor(v);

  QPen   pen;
  QBrush brush;

  plot_->setBrush(brush, true, c, 1.0, CQChartsFillPattern());

  plot_->updateObjPenBrushState(this, pen, brush);

  //---

  painter->fillRect(qrect, brush);

  if (plot_->isCellLabels()) {
    // set font
    plot_->view()->setPlotPainterFont(plot_, painter, plot_->textFont());

    //---

    // set pen
    QPen   tpen;
    QBrush tbrush;

    QColor tc = plot_->interpTextColor(0, 1);

    plot_->setPen(tpen, true, tc, plot_->textAlpha(), CQChartsLength("0px"), CQChartsLineDash());

    plot_->updateObjPenBrushState(this, tpen, tbrush);

    painter->setPen(tpen);

    //---

    QString valueStr = CQChartsUtil::toString(value_);

    CQChartsTextOptions textOptions;

    //textOptions.contrast  = plot_->isTextContrast();
    //textOptions.formatted = plot_->isTextFormatted();
    //textOptions.scaled    = plot_->isTextScaled();
    //textOptions.align     = plot_->textAlign();

    plot_->drawTextInBox(painter, qrect, valueStr, tpen, textOptions);
  }
}
