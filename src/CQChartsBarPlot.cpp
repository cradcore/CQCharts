#include <CQChartsBarPlot.h>
#include <CQChartsAxis.h>
#include <CQPropertyViewItem.h>

CQChartsBarPlot::
CQChartsBarPlot(CQChartsView *view, CQChartsPlotType *plotType, const ModelP &model) :
 CQChartsGroupPlot(view, plotType, model),
 CQChartsObjBarShapeData<CQChartsBarPlot>(this)
{
  NoUpdate noUpdate(this);

  setBarFilled   (true);
  setBarFillColor(CQChartsColor(CQChartsColor::Type::PALETTE));

  setBarBorder(true);

  //---

  setLayerActive(CQChartsLayer::Type::FG_PLOT, true);

  addAxes();

  addKey();

  addTitle();
}

CQChartsBarPlot::
~CQChartsBarPlot()
{
}

//---

void
CQChartsBarPlot::
setValueColumns(const CQChartsColumns &c)
{
  CQChartsUtil::testAndSet(valueColumns_, c, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsBarPlot::
addProperties()
{
  auto addProp = [&](const QString &path, const QString &name, const QString &alias,
                     const QString &desc) {
    return &(this->addProperty(path, this, name, alias)->setDesc(desc));
  };

  //---

  // columns
  addProp("columns", "valueColumns", "values", "Value columns");

  // options
  addProp("options", "horizontal", "", "Draw bars horizontally");

  // margins
  addProp("margins", "margin"     , "bar"  , "Bar margin");
  addProp("margins", "groupMargin", "group", "Grouped bar margin");

  // fill
  addProp("fill", "barFilled", "visible", "Bar fill visible");

  addFillProperties("fill", "barFill", "Bar");

  // stroke
  addProp("stroke", "barBorder", "visible", "Bar stroke visible");

  addLineProperties("stroke", "barBorder", "Bar");

  addProp("stroke", "barCornerSize", "cornerSize", "Bar corner size");

  // color map
  addColorMapProperties();
}

//---

void
CQChartsBarPlot::
setHorizontal(bool b)
{
  CQChartsUtil::testAndSet(horizontal_, b, [&]() {
    CQChartsAxis::swap(xAxis(), yAxis());

    updateRangeAndObjs();
  } );
}

//---

void
CQChartsBarPlot::
setMargin(const CQChartsLength &l)
{
  CQChartsUtil::testAndSet(margin_, l, [&]() { drawObjs(); } );
}

void
CQChartsBarPlot::
setGroupMargin(const CQChartsLength &l)
{
  CQChartsUtil::testAndSet(groupMargin_, l, [&]() { drawObjs(); } );
}

//---

bool
CQChartsBarPlot::
probe(ProbeData &probeData) const
{
  const CQChartsGeom::Range &dataRange = this->dataRange();

  if (! dataRange.isSet())
    return false;

  if (! isHorizontal()) {
    probeData.direction = Qt::Vertical;

    if (probeData.p.x < dataRange.xmin() + 0.5)
      probeData.p.x = dataRange.xmin() + 0.5;

    if (probeData.p.x > dataRange.xmax() - 0.5)
      probeData.p.x = dataRange.xmax() - 0.5;

    probeData.p.x = std::round(probeData.p.x);
  }
  else {
    probeData.direction = Qt::Horizontal;

    if (probeData.p.y < dataRange.ymin() + 0.5)
      probeData.p.y = dataRange.ymin() + 0.5;

    if (probeData.p.y > dataRange.ymax() - 0.5)
      probeData.p.y = dataRange.ymax() - 0.5;

    probeData.p.y = std::round(probeData.p.y);
  }

  return true;
}
