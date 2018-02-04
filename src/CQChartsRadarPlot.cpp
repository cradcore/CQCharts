#include <CQChartsRadarPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsKey.h>
#include <CQChartsUtil.h>
#include <CQCharts.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsTextBoxObj.h>
#include <CQChartsFillObj.h>
#include <CQStrParse.h>
#include <QPainter>

CQChartsRadarPlotType::
CQChartsRadarPlotType()
{
}

void
CQChartsRadarPlotType::
addParameters()
{
  addColumnParameter ("name" , "Name" , "nameColumn"  , "optional");
  addColumnsParameter("value", "Value", "valueColumns", "", "1");
}

CQChartsPlot *
CQChartsRadarPlotType::
create(CQChartsView *view, const ModelP &model) const
{
  return new CQChartsRadarPlot(view, model);
}

//------

CQChartsRadarPlot::
CQChartsRadarPlot(CQChartsView *view, const ModelP &model) :
 CQChartsPlot(view, view->charts()->plotType("radar"), model)
{
  gridColor_ = CQChartsPaletteColor(CQChartsPaletteColor::Type::THEME_VALUE, 0.5);

  textBoxObj_ = new CQChartsTextBoxObj(this);

  textBoxObj_->setBackgroundColor(CQChartsPaletteColor(CQChartsPaletteColor::Type::PALETTE));
  textBoxObj_->setBackgroundAlpha(0.5);

  setBorder(true);
  setFilled(true);

  CQChartsPaletteColor textColor(CQChartsPaletteColor::Type::THEME_VALUE, 1);

  textBoxObj_->setTextColor(textColor);

  addKey();

  addTitle();
}

CQChartsRadarPlot::
~CQChartsRadarPlot()
{
  delete textBoxObj_;
}

//------

QString
CQChartsRadarPlot::
valueColumnsStr() const
{
  return CQChartsUtil::toString(valueColumns());
}

bool
CQChartsRadarPlot::
setValueColumnsStr(const QString &s)
{
  std::vector<int> valueColumns;

  if (! CQChartsUtil::fromString(s, valueColumns))
    return false;

  setValueColumns(valueColumns);

  return true;
}

//------

bool
CQChartsRadarPlot::
isFilled() const
{
  return textBoxObj_->isBackground();
}

void
CQChartsRadarPlot::
setFilled(bool b)
{
  textBoxObj_->setBackground(b);

  update();
}

QString
CQChartsRadarPlot::
fillColorStr() const
{
  return textBoxObj_->backgroundColorStr();
}

void
CQChartsRadarPlot::
setFillColorStr(const QString &s)
{
  textBoxObj_->setBackgroundColorStr(s);

  update();
}

QColor
CQChartsRadarPlot::
interpFillColor(int i, int n)
{
  return textBoxObj_->interpBackgroundColor(i, n);
}

double
CQChartsRadarPlot::
fillAlpha() const
{
  return textBoxObj_->backgroundAlpha();
}

void
CQChartsRadarPlot::
setFillAlpha(double r)
{
  textBoxObj_->setBackgroundAlpha(r);

  update();
}

CQChartsRadarPlot::Pattern
CQChartsRadarPlot::
fillPattern() const
{
  return (Pattern) textBoxObj_->backgroundPattern();
}

void
CQChartsRadarPlot::
setFillPattern(Pattern pattern)
{
  textBoxObj_->setBackgroundPattern((CQChartsBoxObj::Pattern) pattern);

  update();
}

//---

bool
CQChartsRadarPlot::
isBorder() const
{
  return textBoxObj_->isBorder();
}

void
CQChartsRadarPlot::
setBorder(bool b)
{
  textBoxObj_->setBorder(b);

  update();
}

QString
CQChartsRadarPlot::
borderColorStr() const
{
  return textBoxObj_->borderColorStr();
}

void
CQChartsRadarPlot::
setBorderColorStr(const QString &str)
{
  textBoxObj_->setBorderColorStr(str);

  update();
}

QColor
CQChartsRadarPlot::
interpBorderColor(int i, int n) const
{
  return textBoxObj_->interpBorderColor(i, n);
}

double
CQChartsRadarPlot::
borderAlpha() const
{
  return textBoxObj_->borderAlpha();
}

void
CQChartsRadarPlot::
setBorderAlpha(double a)
{
  textBoxObj_->setBorderAlpha(a);

  update();
}

double
CQChartsRadarPlot::
borderWidth() const
{
  return textBoxObj_->borderWidth();
}

void
CQChartsRadarPlot::
setBorderWidth(double r)
{
  textBoxObj_->setBorderWidth(r);

  update();
}

//---

const QFont &
CQChartsRadarPlot::
textFont() const
{
  return textBoxObj_->textFont();
}

void
CQChartsRadarPlot::
setTextFont(const QFont &f)
{
  textBoxObj_->setTextFont(f);

  update();
}

QString
CQChartsRadarPlot::
textColorStr() const
{
  return textBoxObj_->textColorStr();
}

void
CQChartsRadarPlot::
setTextColorStr(const QString &s)
{
  textBoxObj_->setTextColorStr(s);

  update();
}

QColor
CQChartsRadarPlot::
interpTextColor(int i, int n) const
{
  return textBoxObj_->interpTextColor(i, n);
}

//----

void
CQChartsRadarPlot::
addProperties()
{
  CQChartsPlot::addProperties();

  addProperty("columns", this, "nameColumn"  , "name"     );
  addProperty("columns", this, "valueColumn" , "value"    );
  addProperty("columns", this, "valueColumns", "valuesSet");

  addProperty("", this, "angleStart" );
  addProperty("", this, "angleExtent");

  addProperty("grid", this, "gridColor", "color");
  addProperty("grid", this, "gridAlpha", "alpha");

  addProperty("stroke", this, "border"     , "visible");
  addProperty("stroke", this, "borderColor", "color"  );
  addProperty("stroke", this, "borderAlpha", "alpha"  );
  addProperty("stroke", this, "borderWidth", "width"  );

  addProperty("fill", this, "filled"     , "visible");
  addProperty("fill", this, "fillColor"  , "color"  );
  addProperty("fill", this, "fillAlpha"  , "alpha"  );
  addProperty("fill", this, "fillPattern", "pattern");

  addProperty("text", this, "textColor", "color");
  addProperty("text", this, "textFont" , "font" );
}

void
CQChartsRadarPlot::
updateRange(bool apply)
{
  QAbstractItemModel *model = this->model();

  if (! model)
    return;

  //---

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsRadarPlot *plot) :
     plot_(plot) {
      nv_ = plot_->numValueColumns();
    }

    State visit(QAbstractItemModel *model, const QModelIndex &parent, int row) override {
      for (int iv = 0; iv < nv_; ++iv) {
        int column = plot_->valueColumn(iv);

        QModelIndex valueInd = model->index(row, column, parent);

        bool ok;

        double value = CQChartsUtil::modelReal(model, valueInd, ok);

        valueDatas_[iv].add(value);
      }

      return State::OK;
    }

    const ValueDatas &valueDatas() const { return valueDatas_; }

   private:
    CQChartsRadarPlot *plot_ { nullptr };
    int                nv_   { 0 };
    ValueDatas         valueDatas_;
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  valueDatas_ = visitor.valueDatas();

  //---

  valueRadius_ = 0.0;

  int nv = numValueColumns();

  for (int iv = 0; iv < nv; ++iv)
    valueRadius_ = std::max(valueRadius_, valueDatas_[iv].max()/valueDatas_[iv].sum());

  //---

  double r = valueRadius_;

  dataRange_.updateRange(-r, -r);
  dataRange_.updateRange( r,  r);

  if (isEqualScale()) {
    double aspect = this->aspect();

    dataRange_.equalScale(aspect);
  }

  //---

  if (apply)
    applyDataRange();
}

bool
CQChartsRadarPlot::
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

  // process model data
  class RadarPlotVisitor : public ModelVisitor {
   public:
    RadarPlotVisitor(CQChartsRadarPlot *plot) :
     plot_(plot) {
    }

    State visit(QAbstractItemModel *model, const QModelIndex &ind, int row) override {
      plot_->addRow(model, ind, row, numRows());

      return State::OK;
    }

   private:
    CQChartsRadarPlot *plot_ { nullptr };
  };

  RadarPlotVisitor radarPlotVisitor(this);

  visitModel(radarPlotVisitor);

  //---

  resetKeyItems();

  //---

  return true;
}

void
CQChartsRadarPlot::
addRow(QAbstractItemModel *model, const QModelIndex &parent, int row, int nr)
{
  bool hidden = isSetHidden(row);

  if (hidden)
    return;

  //---

  QModelIndex nameInd = model->index(row, nameColumn(), parent);

  QModelIndex nameInd1 = normalizeIndex(nameInd);

  bool ok;

  QString name = CQChartsUtil::modelString(model, nameInd, ok);

  //---

  int nv = valueColumns().size();

  double alen = std::min(std::max(angleExtent(), -360.0), 360.0);

  double da = (nv > 0 ? alen/nv : 0.0);

  //---

  QPolygonF poly;

  double a = angleStart();

  for (int iv = 0; iv < nv; ++iv) {
    QModelIndex valueInd = model->index(row, valueColumns()[iv], parent);

    bool ok1;

    double value = CQChartsUtil::modelReal(model, valueInd, ok1);

    double scale = valueDatas_[iv].sum();

    double ra = CQChartsUtil::Deg2Rad(a);

    double x = value*cos(ra)/scale;
    double y = value*sin(ra)/scale;

    poly << QPointF(x, y);

    a -= da;
  }

  //---

  CQChartsGeom::BBox bbox(-1, -1, 1, 1);

  CQChartsRadarObj *radarObj =
    new CQChartsRadarObj(this, bbox, name, poly, nameInd1, row, nr);

  addPlotObject(radarObj);
}

void
CQChartsRadarPlot::
addKeyItems(CQChartsPlotKey *key)
{
  QAbstractItemModel *model = this->model();

  if (! model)
    return;

  //---

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(CQChartsRadarPlot *plot, CQChartsPlotKey *key) :
     plot_(plot), key_(key) {
    }

    State visit(QAbstractItemModel *model, const QModelIndex &parent, int row) override {
      QModelIndex nameInd = model->index(row, plot_->nameColumn(), parent);

      bool ok;

      QString name = CQChartsUtil::modelString(model, nameInd, ok);

      //---

      CQChartsKeyColorBox *color = new CQChartsKeyColorBox(plot_, row, numRows());
      CQChartsKeyText     *text  = new CQChartsKeyText(plot_, name);

      color->setClickHide(true);

      key_->addItem(color, row, 0);
      key_->addItem(text , row, 1);

      return State::OK;
    }

   private:
    CQChartsRadarPlot *plot_ { nullptr };
    CQChartsPlotKey   *key_  { nullptr };
  };

  RowVisitor visitor(this, key);

  visitModel(visitor);

  //---

  key->plot()->updateKeyPosition(/*force*/true);
}

void
CQChartsRadarPlot::
handleResize()
{
  dataRange_.reset();

  CQChartsPlot::handleResize();
}

void
CQChartsRadarPlot::
drawBackground(QPainter *painter)
{
  CQChartsPlot::drawBackground(painter);

  //---

  int nv = valueColumns().size();

  if (! nv)
    return;

  double alen = std::min(std::max(angleExtent(), -360.0), 360.0);

  double da = alen/nv;

  //---

  // draw grid spokes
  QColor gridColor1 = interpGridColor(0, 1);

  gridColor1.setAlphaF(gridAlpha());

  QPen gpen1(gridColor1);

  painter->setPen(gpen1);

  double px1, py1;

  windowToPixel(0.0, 0.0, px1, py1);

  double a = angleStart();

  for (int iv = 0; iv < nv; ++iv) {
    double ra = CQChartsUtil::Deg2Rad(a);

    double x = valueRadius_*cos(ra);
    double y = valueRadius_*sin(ra);

    double px2, py2;

    windowToPixel(x, y, px2, py2);

    painter->drawLine(QPointF(px1, py1), QPointF(px2, py2));

    a -= da;
  }

  //---

  // draw grid polygons
  QColor gridColor2 = interpGridColor(0, 1);

  QPen gpen2(gridColor2);

  //---

  QColor textColor = interpTextColor(0, 1);

  QPen tpen(textColor);

  painter->setFont(textFont());

  int    nl = 5;
  double dr = valueRadius_/nl;

  for (int i = 0; i <= nl; ++i) {
    double r = dr*i;

    double a = angleStart();

    QPolygonF poly;

    for (int iv = 0; iv < nv; ++iv) {
      double ra = CQChartsUtil::Deg2Rad(a);

      double x = r*cos(ra);
      double y = r*sin(ra);

      double px, py;

      windowToPixel(x, y, px, py);

      poly << QPointF(px, py);

      //---

      if (i == nl) {
        painter->setPen(tpen);

        //---

        bool ok;

        QString name = CQChartsUtil::modelHeaderString(model(), valueColumns()[iv], ok);

        Qt::Alignment align = 0;

        if      (CQChartsUtil::isZero(x)) align |= Qt::AlignHCenter;
        else if (x > 0)                   align |= Qt::AlignLeft;
        else if (x < 0)                   align |= Qt::AlignRight;

        if      (CQChartsUtil::isZero(y)) align |= Qt::AlignVCenter;
        else if (y > 0)                   align |= Qt::AlignBottom;
        else if (y < 0)                   align |= Qt::AlignTop;

        CQChartsDrawUtil::drawAlignedText(painter, px, py, name, align, 2, 2);
      }

      //---

      a -= da;
    }

    poly << poly[0];

    painter->setPen(gpen2);

    painter->drawPolygon(poly);
  }
}

void
CQChartsRadarPlot::
draw(QPainter *painter)
{
  initPlotObjs();

  //---

  drawParts(painter);
}

//------

CQChartsRadarObj::
CQChartsRadarObj(CQChartsRadarPlot *plot, const CQChartsGeom::BBox &rect, const QString &name,
                 const QPolygonF &poly, const QModelIndex &ind, int i, int n) :
 CQChartsPlotObj(plot, rect), plot_(plot), name_(name), poly_(poly), ind_(ind), i_(i), n_(n)
{
}

QString
CQChartsRadarObj::
calcId() const
{
  return QString("%1").arg(name_);
}

bool
CQChartsRadarObj::
inside(const CQChartsGeom::Point &p) const
{
  if (! visible())
    return false;

  return poly_.containsPoint(CQChartsUtil::toQPoint(p), Qt::OddEvenFill);
}

void
CQChartsRadarObj::
addSelectIndex()
{
  plot_->addSelectIndex(ind_);
}

bool
CQChartsRadarObj::
isIndex(const QModelIndex &ind) const
{
  return (ind == ind_);
}

void
CQChartsRadarObj::
draw(QPainter *painter, const CQChartsPlot::Layer &)
{
  if (! poly_.size())
    return;

  QPolygonF ppoly;

  for (int i = 0; i < poly_.size(); ++i) {
    double x = poly_[i].x();
    double y = poly_[i].y();

    double px, py;

    plot_->windowToPixel(x, y, px, py);

    ppoly << QPointF(px, py);
  }

  ppoly << ppoly[0];

  //---

  // calc stroke and brush
  QBrush brush;

  if (plot_->isFilled()) {
    QColor c = plot_->interpFillColor(i_, n_);

    c.setAlphaF(plot_->fillAlpha());

    brush.setColor(c);
    brush.setStyle(CQChartsFillObj::patternToStyle(
      (CQChartsFillObj::Pattern) plot_->fillPattern()));
  }
  else {
    brush.setStyle(Qt::NoBrush);
  }

  QPen pen;

  if (plot_->isBorder()) {
    QColor bc = plot_->interpBorderColor(0, 1);

    bc.setAlphaF(plot_->borderAlpha());

    pen.setColor (bc);
    pen.setWidthF(plot_->borderWidth());
  }
  else {
    pen.setStyle(Qt::NoPen);
  }

  plot_->updateObjPenBrushState(this, pen, brush);

  painter->setPen  (pen);
  painter->setBrush(brush);

  painter->drawPolygon(ppoly);
}
