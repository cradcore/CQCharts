#include <CQChartsXYPlot.h>
#include <CQChartsView.h>
#include <CQChartsAxis.h>
#include <CQChartsAxisRug.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsAnalyzeModelData.h>
#include <CQChartsModelUtil.h>
#include <CQChartsUtil.h>
#include <CQChartsArrow.h>
#include <CQChartsSmooth.h>
#include <CQChartsDataLabel.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsGrahamHull.h>
#include <CQChartsTip.h>
#include <CQChartsHtml.h>
#include <CQChartsVariant.h>
#include <CQChartsWidgetUtil.h>
#include <CQChartsTextPlacer.h>
#include <CQChartsPlotParameterEdit.h>
#include <CQCharts.h>

#include <CQUtil.h>
//#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQPerfMonitor.h>

#include <QMenu>

CQChartsXYPlotType::
CQChartsXYPlotType()
{
}

void
CQChartsXYPlotType::
addParameters()
{
  startParameterGroup("XY");

  // columns
  addColumnParameter ("x", "X", "xColumn").
    setRequired().setMonotonic().setNumericColumn().
    setPropPath("columns.x").setTip("X Value Column");
  addColumnsParameter("y", "Y", "yColumns").
    setRequired().setNumericColumn().setPropPath("columns.y").setTip("Y Value Column(s)");

  addColumnParameter("label", "Label", "labelColumn").
    setStringColumn().setPropPath("columns.label").setTip("Optional Label Column");

  //---

  // options
  addBoolParameter("lines" , "Lines" , "lines", true).setBasic().setTip("Draw Lines");
  addBoolParameter("points", "Points", "points"     ).setBasic().setTip("Draw Points");

  addBoolParameter("columnSeries", "Column Series", "columnSeries").setTip("Columns are series");

  addBoolParameter("bivariate" , "Bivariate" , "bivariateLines" ).setTip("Draw Bivariate Lines");
  addBoolParameter("stacked"   , "Stacked"   , "stacked"        ).setTip("Stack Points");
  addBoolParameter("cumulative", "Cumulative", "cumulative"     ).setTip("Cumulate Values");
  addBoolParameter("fillUnder" , "Fill Under", "fillUnderFilled").setTip("Fill Under Curve");
  addBoolParameter("impulse"   , "Impulse"   , "impulseLines"   ).setTip("Draw Impulse Line");

  addMiscParameters();

  addBoolParameter("movingAverage", "Moving Average", "movingAverage").
    setTip("Show moving average");

  endParameterGroup();

  //---

  addMappingParameters();

  //---

  // vector columns
  startParameterGroup("Vectors");

  addColumnParameter("vectorX", "Vector X", "vectorXColumn").setNumericColumn();
  addColumnParameter("vectorY", "Vector Y", "vectorYColumn").setNumericColumn();

  addBoolParameter("vectors", "Vectors", "vectors");

  endParameterGroup();

  //---

  CQChartsPointPlotType::addParameters();
}

QString
CQChartsXYPlotType::
description() const
{
  auto B   = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("XY Plot Type</h2>").
    h3("Summary").
     p("Draws points at x and y coordinate pairs and optionally connects them with a "
       "continuous line. The connecting lines can be straight or rounded.").
     p("Ideally the x coordinates should be monotonic but this is not required.").
    h3("Columns").
     p("The x and y values come from the values in the " + B("X") + " and " + B("Y") + " columns. "
       "Multiple " + B("Y") + " columns can be specified to create a stack of lines.").
     p("An optional " + B("SymbolType") + " column can be specified to supply the type of the "
       "symbol drawn at the point. An optional " + B("SymbolSize") + " column can be specified "
       "to supply the size of the symbol drawn at the point. An optional " + B("Color") + " "
       "column can be specified to supply the fill color of the symbol drawn at the point.").
     p("An optional point label can be specified using the " + B("Label") + " column. The "
       "font size of the label can be specified using the " + B("FontSize") + " column.").
     p("Optional " + B("VectorX") + " and " + B("VectorY") + " columns can be specified to draw "
       "a vector at the point.").
    h3("Options").
     p("The " + B("Lines") + " option determines whether the points are connected with a line. "
       "The default line style can be separately customized.").
     p("The " + B("Points") + " option determines whether the points are drawn. The default point "
       "symbol can be separately customized.").
     p("Enabling the " + B("Bivariate") + " option fills the area between adjacent sets of x, y "
       "coordinates (two or more y column values should be specified). The bivariate line "
       "style can be separately customized.").
     p("Enabling the " + B("Stacked") + " option stacks the y values on top of each other "
       "so the next set of y values adds onto the previous set of y values.").
     p("Enabling the " + B("Cumulative") + " option treats the y values as an increment "
       "from the previous y value (in each set).").
     p("Enabling the " + B("FillUnder") + " option fills the area under the plot. The "
       "fill under style (fill/stroke) can be separately customized.").
     p("Enabling the " + B("Impulse") + " option draws a line from zero to the "
       "points y value. The impulse line style can be separately customized.").
     p("Enabling the " + B("Best Fit") + " option draws a best fit line between the points.").
     p("The " + B("Vectors") + " option detemines whether the vector specified by the "
       "" + B("VectorX") + " and " + B("VectorY") + " columns are drawn.").
    h3("Customization").
     p("The area under the curve can be filled.").
     p("Impulse lines (from point to y minimum) curve can be added.").
     p("Bivariate lines (between pairs of y values) curve can be added.").
     p("The points can have an overlaid best fit line, statistic lines.").
    h3("Limitations").
     p("None").
    h3("Example").
     p(IMG("images/xychart.png"));
}

void
CQChartsXYPlotType::
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

  Columns yColumns;

  for (int c = 0; c < nc; ++c) {
    if (c == xColumn.column())
      continue;

    auto *columnDetails = details->columnDetails(Column(c));
    if (! columnDetails) continue;

    if (columnDetails->isNumeric())
      yColumns.addColumn(columnDetails->column());
  }

  if (yColumns.count())
    analyzeModelData.parameterNameColumns["y"] = yColumns;
}

CQChartsPlot *
CQChartsXYPlotType::
create(View *view, const ModelP &model) const
{
  return new CQChartsXYPlot(view, model);
}

//---

CQChartsXYPlot::
CQChartsXYPlot(View *view, const ModelP &model) :
 CQChartsPointPlot(view, view->charts()->plotType("xy"), model),
 CQChartsObjLineData             <CQChartsXYPlot>(this, &xyInvalidator_),
 CQChartsObjPointData            <CQChartsXYPlot>(this, &xyInvalidator_),
 CQChartsObjImpulseShapeData     <CQChartsXYPlot>(this, &xyInvalidator_),
 CQChartsObjBivariateLineData    <CQChartsXYPlot>(this, &xyInvalidator_),
 CQChartsObjFillUnderFillData    <CQChartsXYPlot>(this, &xyInvalidator_),
 CQChartsObjMovingAverageLineData<CQChartsXYPlot>(this, &xyInvalidator_),
 xyInvalidator_(this)
{
}

CQChartsXYPlot::
~CQChartsXYPlot()
{
  term();
}

//---

void
CQChartsXYPlot::
init()
{
  CQChartsPointPlot::init();

  //---

  NoUpdate noUpdate(this);

  setLayerActive(Layer::Type::FG_PLOT, true);

  //---

  setSymbol(Symbol::circle());

  setLines (true);
  setPoints(false);

  setLinesColor(Color::makePalette());
  setLinesWidth(Length::pixel(3));

  setImpulseVisible(false);
  setImpulseLines(true);
  setImpulseStrokeColor(Color::makePalette());
  setImpulseFillColor(Color::makePalette());

  setBivariateLines(false);
  setBivariateLinesColor(Color::makePalette());

  //---

  // arrow object
  arrowObj_ = std::make_unique<Arrow>(this);

  arrowObj_->setVisible(false);

  connect(arrowObj_.get(), SIGNAL(dataChanged()), this, SLOT(updateSlot()));

  //---

  setFillUnderFilled   (false);
  setFillUnderFillColor(Color::makePalette());
  setFillUnderFillAlpha(Alpha(0.5));

  //---

  // moving average
  setMovingAverageLinesWidth(Length::pixel(4));
  setMovingAverageLinesColor(Color::makePalette());

  //---

  addAxes();

  addKey();

  addTitle();

  //---

  addColorMapKey();

  //---

  connect(yAxis(), SIGNAL(includeZeroChanged()), this, SLOT(yAxisIncludeZeroSlot()));
}

void
CQChartsXYPlot::
term()
{
}

//---

void
CQChartsXYPlot::
setXColumn(const Column &c)
{
  CQChartsUtil::testAndSet(xColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged(); } );
}

void
CQChartsXYPlot::
setYColumns(const Columns &c)
{
  CQChartsUtil::testAndSet(yColumns_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged(); } );
}

void
CQChartsXYPlot::
setLabelColumn(const Column &c)
{
  CQChartsUtil::testAndSet(labelColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged(); } );
}

//---

void
CQChartsXYPlot::
setVectorXColumn(const Column &c)
{
  CQChartsUtil::testAndSet(vectorXColumn_, c, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsXYPlot::
setVectorYColumn(const Column &c)
{
  CQChartsUtil::testAndSet(vectorYColumn_, c, [&]() { updateRangeAndObjs(); } );
}

//---

QString
CQChartsXYPlot::
columnValueToString(const Column &column, const QVariant &var) const
{
  bool ok;

  if (column == xColumn() && CQChartsVariant::isReal(var))
    return xStr(CQChartsVariant::toReal(var, ok));

  if (yColumns().hasColumn(column) && CQChartsVariant::isReal(var))
    return yStr(CQChartsVariant::toReal(var, ok));

  return var.toString();
}

void
CQChartsXYPlot::
setMapXColumn(bool b)
{
  CQChartsUtil::testAndSet(mapXColumn_, b, [&]() { updateRangeAndObjs(); } );
}

bool
CQChartsXYPlot::
calcMapXColumn() const
{
  if (mapXColumn_)
    return true;

  return (xColumnType_ == CQBaseModelType::STRING);
}

//---

CQChartsColumn
CQChartsXYPlot::
getNamedColumn(const QString &name) const
{
  Column c;
  if      (name == "x"       ) c = this->xColumn();
  else if (name == "label"   ) c = this->labelColumn();
  else if (name == "vector_x") c = this->vectorXColumn();
  else if (name == "vector_y") c = this->vectorYColumn();
  else                         c = CQChartsPointPlot::getNamedColumn(name);

  return c;
}

void
CQChartsXYPlot::
setNamedColumn(const QString &name, const Column &c)
{
  if      (name == "x"       ) this->setXColumn(c);
  else if (name == "label"   ) this->setLabelColumn(c);
  else if (name == "vector_x") this->setVectorXColumn(c);
  else if (name == "vector_y") this->setVectorYColumn(c);
  else                         CQChartsPointPlot::setNamedColumn(name, c);
}

CQChartsColumns
CQChartsXYPlot::
getNamedColumns(const QString &name) const
{
  Columns c;
  if (name == "y") c = this->yColumns();
  else             c = CQChartsPointPlot::getNamedColumns(name);

  return c;
}

void
CQChartsXYPlot::
setNamedColumns(const QString &name, const Columns &c)
{
  if (name == "y") this->setYColumns(c);
  else             CQChartsPointPlot::setNamedColumns(name, c);
}

//---

void
CQChartsXYPlot::
yAxisIncludeZeroSlot()
{
  updateRangeAndObjs();
}

//---

void
CQChartsXYPlot::
resetBestFit()
{
  for (const auto &plotObj : plotObjs_) {
    auto *polyObj = dynamic_cast<PolylineObj *>(plotObj);

    if (polyObj)
      polyObj->resetBestFit();
  }
}

//---

void
CQChartsXYPlot::
setStacked(bool b)
{
  CQChartsUtil::testAndSet(stacked_, b, [&]() {
    updateRangeAndObjs(); emit customDataChanged(); } );
}

void
CQChartsXYPlot::
setCumulative(bool b)
{
  CQChartsUtil::testAndSet(cumulative_, b, [&]() { updateRangeAndObjs(); } );
}

//---

bool
CQChartsXYPlot::
isVectors() const
{
  return arrowObj_->isVisible();
}

void
CQChartsXYPlot::
setVectors(bool b)
{
  if (b != isVectors()) {
    CQChartsWidgetUtil::AutoDisconnect arrowDisconnect(
      arrowObj_.get(), SIGNAL(dataChanged()), this, SLOT(updateSlot()));

    arrowObj_->setVisible(b);

    updateObjs();
  }
}

//---

bool
CQChartsXYPlot::
calcImpulseVisible() const
{
  return (! isStacked() && isImpulseVisible());
}

void
CQChartsXYPlot::
setImpulseVisible(bool b)
{
  CQChartsUtil::testAndSet(impulseData_.visible, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsXYPlot::
setImpulseLines(bool b)
{
  CQChartsUtil::testAndSet(impulseData_.lines, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsXYPlot::
setImpulseWidth(const Length &w)
{
  CQChartsUtil::testAndSet(impulseData_.width, w, [&]() { updateRangeAndObjs(); } );
}

double
CQChartsXYPlot::
calcImpulsePixelWidth(double b) const
{
  double lw;

  if (isImpulseLines())
    lw = std::max(lengthPixelWidth(impulseStrokeWidth()), 2*b);
  else
    lw = std::max(lengthPixelWidth(impulseWidth()), 2*b);

  return lw;
}

//---

void
CQChartsXYPlot::
setLayers(int i)
{
  CQChartsUtil::testAndSet(layers_, i, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsXYPlot::
setPointsSlot(bool b)
{
  setPoints(b);
}

void
CQChartsXYPlot::
setPointDelta(int i)
{
  CQChartsUtil::testAndSet(pointDelta_, i, [&]() { updateObjs(); } );
}

void
CQChartsXYPlot::
setPointCount(int i)
{
  CQChartsUtil::testAndSet(pointCount_, i, [&]() { updateObjs(); } );
}

void
CQChartsXYPlot::
setPointStart(int i)
{
  CQChartsUtil::testAndSet(pointStart_, i, [&]() { updateObjs(); } );
}

//---

void
CQChartsXYPlot::
setLinesSlot(bool b)
{
  setLines(b);
}

void
CQChartsXYPlot::
setLinesSelectable(bool b)
{
  CQChartsUtil::testAndSet(linesSelectable_, b, [&]() { drawObjs(); } );
}

void
CQChartsXYPlot::
setRoundedLines(bool b)
{
  CQChartsUtil::testAndSet(roundedLines_, b, [&]() { drawObjs(); } );
}

void
CQChartsXYPlot::
setLineLabel(bool b)
{
  CQChartsUtil::testAndSet(lineLabel_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsXYPlot::
setKeyLine(bool b)
{
  CQChartsUtil::testAndSet(keyLine_, b, [&]() { drawObjs(); } );
}

//---

void
CQChartsXYPlot::
setFillUnderSelectable(bool b)
{
  CQChartsUtil::testAndSet(fillUnderData_.selectable, b, [&]() { drawObjs(); } );
}

void
CQChartsXYPlot::
setFillUnderPos(const CQChartsFillUnderPos &pos)
{
  CQChartsUtil::testAndSet(fillUnderData_.pos, pos, [&]() { updateObjs(); } );
}

void
CQChartsXYPlot::
setFillUnderSide(const CQChartsFillUnderSide &s)
{
  CQChartsUtil::testAndSet(fillUnderData_.side, s, [&]() { updateObjs(); } );
}

//---

bool
CQChartsXYPlot::
canBivariateLines() const
{
  if (! isBivariateLines())
    return false;

  int ns = numSets();

  return (ns >= 2);
}

//---

void
CQChartsXYPlot::
addProperties()
{
  auto addArrowProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc) {
    return &(this->addProperty(path, arrowObj_.get(), name, alias)->setDesc(desc));
  };

  auto addArrowStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                               const QString &desc) {
    auto *item = addArrowProp(path, name, alias, desc);
    CQCharts::setItemIsStyle(item);
    return item;
  };

  //---

  addBaseProperties();

  // columns
  addProp("columns", "xColumn"    , "x"    , "X value column" );
  addProp("columns", "yColumns"   , "y"    , "Y value column(s)");
  addProp("columns", "labelColumn", "label", "Label column");

  addProp("columns", "vectorXColumn", "vectorX", "Vector x column");
  addProp("columns", "vectorYColumn", "vectorY", "Vector y column");

  addProp("columns", "mapXColumn", "mapX", "Force map x column value to unique id");

  // bivariate
  addProp("bivariate", "bivariateLines", "visible", "Bivariate lines visible");

  addLineProperties("bivariate/stroke", "bivariateLines", "Bivariate");

  // stacked
  addProp("stacked", "stacked", "enabled", "Stack y values");

  // cumulative
  addProp("cumulative", "cumulative", "enabled", "Cumulative values");

  // column series
  addProp("options", "columnSeries", "timeSeries", "Are y column values a series");

  // horizon
  addProp("horizon", "layers", "layers", "Horizon layers");

  // points
  addProp("points", "points"         , "visible", "Point symbol visible");
  addProp("points", "pointLineSelect", "lineSelect", "Select point selects line");
  addProp("points", "pointCount"     , "count", "Number of points to show");
  addProp("points", "pointDelta"     , "delta", "Show points delta index");
  addProp("points", "pointStart"     , "start", "Show points start index");

  addSymbolProperties("points/symbol", "", "Points");

  // data labels
  addProp("points", "adjustText", "adjustText", "Adjust text placement");

  dataLabel()->addPathProperties("points/labels", "Labels");

  // lines
  addProp("lines", "lines"          , "visible"   , "Lines visible");
  addProp("lines", "linesSelectable", "selectable", "Lines selectable");
  addProp("lines", "roundedLines"   , "rounded"   , "Smooth lines");
  addProp("lines", "lineLabel"      , "label"     , "Draw label at end of line");

  addLineProperties("lines/stroke", "lines", "Lines");

  //---

  // moving average
  auto movingAveragePropPath = QString("overlays/movingAverage");

  addProp(movingAveragePropPath, "movingAverage"   , "visible", "Show moving average");
  addProp(movingAveragePropPath, "numMovingAverage", "size"   , "Moving average window size");

  addLineProperties(movingAveragePropPath, "movingAverageLines", "Lines");

  //---

  addSplitGroupsProperties();

  //---

  addProp("key", "keyLine", "drawLine", "Draw lines on key");

  //---

  // best fit line and deviation fill
  addBestFitProperties(/*hasLayer*/false);

  // convex hull shape
  addHullProperties(/*hasLayer*/false);

  // stats
  addStatsProperties();

  //---

  auto axisAnnotationsPath = QString("axisAnnotations");

  // rug axis
  addRugProperties(axisAnnotationsPath);

  //---

  // fill under
  addProp("fillUnder", "fillUnderFilled"    , "visible"   , "Fill under polygon visible");
  addProp("fillUnder", "fillUnderSelectable", "selectable", "Fill under polygon selectable");
  addProp("fillUnder", "fillUnderPos"       , "position"  , "Fill under base position");
  addProp("fillUnder", "fillUnderSide"      , "side"      , "Fill under line side");

  addFillProperties("fillUnder/fill", "fillUnderFill", "Fill under");

  // impulse
  addProp("impulse", "impulseVisible", "visible", "Impulse lines visible");
  addProp("impulse", "impulseLines"  , "lines"  , "Impulse is line");
  addProp("impulse", "impulseWidth"  , "width"  , "Impulse width (non-line)");

  addProp("impulse/stroke", "impulseStroked", "visible", "Impulse stroke visible");

  addLineProperties("impulse/stroke", "impulseStroke", "Node");

  addProp("impulse/fill", "impulseFilled", "visible", "Impulse fill visible");

  addFillProperties("impulse/fill", "impulseFill", "Node");

  //---

  // vectors
  addProp("vectors", "vectors", "visible", "Vectors at points visible");

  auto vectorLinePath = QString("vectors/line");

  addArrowStyleProp(vectorLinePath, "lineWidth", "width", "Vector arrow connecting line width");

  auto vectorFrontHeadPath = QString("vectors/frontHead");

//addArrowProp     (vectorFrontHeadPath, "frontVisible"  , "visible",
//                  "Vector arrow front head visible");
  addArrowProp     (vectorFrontHeadPath, "frontType"     , "type",
                    "Vector arrow front head type");
  addArrowStyleProp(vectorFrontHeadPath, "frontLength"   , "length",
                    "Vector arrow front head length");
  addArrowStyleProp(vectorFrontHeadPath, "frontAngle"    , "angle",
                    "Vector arrow front head angle");
//addArrowStyleProp(vectorFrontHeadPath, "frontBackAngle", "backAngle",
//                  "Vector arrow front head back angle");
//addArrowStyleProp(vectorFrontHeadPath, "frontLineEnds" , "line",
//                  "Vector arrow front head back is drawn using lines");

  auto vectorTailHeadPath = QString("vectors/tailHead");

//addArrowProp     (vectorTailHeadPath, "tailVisible"  , "visible",
//                  "Vector arrow tail head visible");
  addArrowProp     (vectorTailHeadPath, "tailType"     , "type",
                    "Vector arrow tail head type");
  addArrowStyleProp(vectorTailHeadPath, "tailLength"   , "length",
                    "Vector arrow tail head length");
  addArrowStyleProp(vectorTailHeadPath, "tailAngle"    , "angle",
                    "Vector arrow tail head angle");
//addArrowStyleProp(vectorTailHeadPath, "tailBackAngle", "backAngle",
//                  "Vector arrow tail head back angle");
//addArrowStyleProp(vectorTailHeadPath, "tailLineEnds" , "line",
//                  "Vector arrow tail head back is drawn using lines");

  auto vectorFillPath = QString("vectors/fill");

  addArrowStyleProp(vectorFillPath, "filled"   , "visible", "Vector arrow fill visible");
  addArrowStyleProp(vectorFillPath, "fillColor", "color"  , "Vector arrow fill color");
  addArrowStyleProp(vectorFillPath, "fillAlpha", "alpha"  , "Vector arrow fill alpha");

  auto vectorStrokePath = QString("vectors/stroke");

  addArrowStyleProp(vectorStrokePath, "stroked"    , "visible", "Vector stroke visible");
  addArrowStyleProp(vectorStrokePath, "strokeColor", "color"  , "Vector stroke color");
  addArrowStyleProp(vectorStrokePath, "strokeAlpha", "alpha"  , "Vector stroke alpha");
  addArrowStyleProp(vectorStrokePath, "strokeWidth", "width"  , "Vector stroke width");

  //---

  CQChartsPointPlot::addProperties();

  //---

  CQChartsPointPlot::addPointProperties();

  //---

  // color map
  addColorMapProperties();

  // color map key
  addColorMapKeyProperties();
}

//---

QColor
CQChartsXYPlot::
interpColor(const Color &c, const ColorInd &ind) const
{
  if (c.type() != Color::Type::PALETTE)
    return CQChartsPlot::interpColor(c, ind);

  if (isOverlay()) {
    int i = ind.i;
    int n = ind.n;

    if (prevPlot() || nextPlot()) {
      auto *plot1 = prevPlot();
      auto *plot2 = nextPlot();

      while (plot1) { ++n; plot1 = plot1->prevPlot(); }
      while (plot2) { ++n; plot2 = plot2->nextPlot(); }
    }

    //---

    auto *plot1 = prevPlot();

    while (plot1) {
      ++i;

      plot1 = plot1->prevPlot();
    }

    return view()->interpPaletteColor(ColorInd(i, n), c.isScale(), c.isInvert());
  }
  else {
    return view()->interpPaletteColor(ind, c.isScale(), c.isInvert());
  }
}

//---

CQChartsGeom::Range
CQChartsXYPlot::
calcRange() const
{
  CQPerfTrace trace("CQChartsXYPlot::calcRange");

  //---

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsXYPlot *>(this);

  th->clearErrors();

  //---

  // check columns
  bool columnsValid = true;

  if (! checkColumn (xColumn(), "X", th->xColumnType_, /*required*/true))
    columnsValid = false;
  if (! checkColumns(yColumns(), "Y", /*required*/true))
    columnsValid = false;

  if (! checkColumn(labelColumn(), "Label")) columnsValid = false;

  if (! columnsValid)
    return Range(0.0, 0.0, 1.0, 1.0);

  //---

  initGroupData(Columns(), Column());

  //---

  // calc data range (x, y values)
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsXYPlot *plot) :
     plot_(plot) {
      int ns = plot_->numSets();

      sum_.resize(size_t(ns));

      if (plot_->isColumnSeries())
        plot_->headerSeriesData(sx_);
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      ModelIndex ind(plot_, data.row, plot_->xColumn(), data.parent);

      // init group
      int groupInd = plot_->rowGroupInd(ind);

      //---

      lastSum_ = sum_;

      //---

      // get x and y values
      double x; std::vector<double> y; QModelIndex rowInd;

      if (! plot_->rowData(data, x, y, rowInd, plot_->isSkipBad()))
        return State::SKIP;

      auto ny = y.size();

      //---

      auto updateRange = [&](double x, double y) {
        range_               .updateRange(x, y);
        groupRange_[groupInd].updateRange(x, y);
      };

      if      (plot_->isStacked()) {
        // TODO: support stacked and cumulative
        double sum1 = 0.0;

        for (size_t i = 0; i < ny; ++i) {
          if (! CMathUtil::isNaN(y[i]))
            sum1 += y[i];
        }

        updateRange(x, 0.0);
        updateRange(x, sum1);
      }
      else if (plot_->isCumulative()) {
        if (! plot_->isColumnSeries()) {
          for (size_t i = 0; i < ny; ++i) {
            if (! CMathUtil::isNaN(y[i])) {
              double y1 = y[i] + lastSum_[i];

              sum_[i] += y[i];

              updateRange(x, y1);
            }
          }
        }
      }
      else {
        if (! plot_->isColumnSeries()) {
          for (size_t i = 0; i < ny; ++i) {
            if (! CMathUtil::isNaN(y[i]))
              updateRange(x, y[i]);
          }
        }
        else {
          for (size_t i = 0; i < ny; ++i) {
            if (! CMathUtil::isNaN(y[i]))
              updateRange(sx_[i], y[i]);
          }
        }
      }

      return State::OK;
    }

    const Range &range() const { return range_; }

    const GroupRange &groupRange() const { return groupRange_; }

   private:
    using Reals = std::vector<double>;

    const CQChartsXYPlot* plot_ { nullptr };
    Range                 range_;
    GroupRange            groupRange_;
    Reals                 sum_;
    Reals                 lastSum_;
    std::vector<double>   sx_;
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  //---

  Range dataRange;

  if (isSplitGroups()) {
    int ng = numVisibleGroups();

    dataRange = Range(0.0, 0.0, ng, 1.0);

    th->range_      = visitor.range();
    th->groupRange_ = visitor.groupRange();
  }
  else {
    dataRange = visitor.range();
  }

  //---

  // ensure range non-zero
  dataRange.makeNonZero();

  //---

  int nl = this->layers();

  if (nl > 1) {
    th->layerMin_ = dataRange.bottom();
    th->layerMax_ = dataRange.top   ();

    th->layerDelta_ = (layerMax_ - layerMin_)/nl;

    dataRange.setTop(layerMin_ + layerDelta_);
  }

  //---

  return dataRange;
}

void
CQChartsXYPlot::
postCalcRange()
{
  updateAxes();
}

void
CQChartsXYPlot::
updateAxes()
{
  setXValueColumn(xColumn());
  setYValueColumn(yColumns().column());

  xAxis()->setDefLabel("", /*notify*/false);
  yAxis()->setDefLabel("", /*notify*/false);

  //---

  // set x axis column
  Column xAxisColumn;

  if (! isColumnSeries())
    xAxisColumn = xColumn();
  else
    xAxisColumn = Column::makeHHeader(yColumns().getColumn(0).column());

  if (! isSplitGroups()) {
    if (isOverlay()) {
      if (isFirstPlot() || isX1X2())
        xAxis()->setColumn(xAxisColumn);
    }
    else {
      xAxis()->setColumn(xAxisColumn);
    }
  }
  else {
    xAxis()->setColumn(Column());
  }

  //---

  // set x axis name and type
  QString xname;

  (void) xAxisName(xname, "X");

  if (isOverlay()) {
    if (isFirstPlot() || isX1X2())
      xAxis()->setDefLabel(xname, /*notify*/false);
  }
  else {
    xAxis()->setDefLabel(xname, /*notify*/false);
  }

  if (! isSplitGroups()) {
    if (xColumnType_ == ColumnType::TIME)
      xAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::DATE),
                            /*notify*/false);

    if (calcMapXColumn()) {
      xAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::INTEGER),
                            /*notify*/false);

      xAxis()->setMajorIncrement(1);
    }
  }
  else {
    xAxis()->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::INTEGER),
                          /*notify*/false);
  }

  //---

  // set y axis column and name(s)
  QString yname;

  (void) yAxisName(yname, "Y");

  if      (canBivariateLines()) {
    if (isOverlay()) {
      if (isY1Y2()) {
        yAxis()->setDefLabel(yname, /*notify*/false);
      }
    }
    else {
      yAxis()->setDefLabel(yname, /*notify*/false);
    }
  }
  else if (isStacked()) {
  }
  else {
    auto yColumn = yColumns().getColumn(0);

    if (isOverlay()) {
      if (isFirstPlot() || isY1Y2())
        yAxis()->setColumn(yColumn);
    }
    else
      yAxis()->setColumn(yColumn);

    if (isOverlay()) {
      if (isY1Y2()) {
        yAxis()->setDefLabel(yname, /*notify*/false);
      }
    }
    else {
      yAxis()->setDefLabel(yname, /*notify*/false);
    }
  }

  //---

  if (isOverlay() && isFirstPlot())
    setOverlayPlotsAxisNames();

  //---

  // set x axis labels
  xAxis()->clearTickLabels();

  if (! isSplitGroups()) {
    if (calcMapXColumn()) {
      auto *columnDetails = this->columnDetails(xColumn());

      for (int i = 0; columnDetails && i < columnDetails->numUnique(); ++i)
        xAxis()->setTickLabel(i, columnDetails->uniqueValue(i).toString());
    }
  }

  //---

  auto numXAxes = size_t(isSplitGroups() ? numVisibleGroups() : 0);

  CQChartsUtil::makeArraySize(xaxes_, numXAxes, [&]() {
    auto *axis = new CQChartsAxis(this, Qt::Horizontal, 0, 1);

    axis->moveToThread(this->thread());

    axis->setParent(this);
    axis->setPlot  (this);

    axis->setUpdatesEnabled(false);

    return axis;
  });

  auto numYAxes = size_t(isSplitGroups() && ! isSplitSharedY() ? numXAxes : 0);

  CQChartsUtil::makeArraySize(yaxes_, numYAxes, [&]() {
    auto *axis = new CQChartsAxis(this, Qt::Vertical, 0, 1);

    axis->moveToThread(this->thread());

    axis->setParent(this);
    axis->setPlot  (this);

    axis->setUpdatesEnabled(false);

    return axis;
  });

  //---

  auto sm = splitMargin();

  for (size_t i = 0; i < xaxes_.size(); ++i) {
    auto *xaxis = xaxes_[i];

    int ig = unmapVisibleGroup(int(i));

    //---

    if (xColumnType_ == ColumnType::TIME)
      xaxis->setValueType(CQChartsAxisValueType(CQChartsAxisValueType::Type::DATE),
                          /*notify*/false);

    xaxis->setColumn(xAxisColumn);

    auto groupName = groupIndName(ig);

    xaxis->setDefLabel(groupName, /*notify*/false);

    //---

    // set range
    const auto &range = getGroupRange(ig);

    auto xi = double(i);

    double xmin = (i >            0 ? xi + sm       : xi      );
    double xmax = (i < numXAxes - 1 ? xi + 1.0 - sm : xi + 1.0);

    xaxis->setRange(xmin, xmax);

    xaxis->setValueRange(range.xmin(), range.xmax());
  }

  //---

  auto *masterYAxis = yAxis();

  for (size_t i = 0; i < yaxes_.size(); ++i) {
    auto *yaxis = yaxes_[i];

    int ig = unmapVisibleGroup(int(i));

    //---

    // set range
    const auto &range = getGroupRange(ig);

    yaxis->setRange(0.0, 1.0);

    double ymin = range.ymin();

    if (masterYAxis->isIncludeZero())
      ymin = std::min(ymin, 0.0);

    yaxis->setValueRange(ymin, range.ymax());
  }

  //---

  if (isSplitGroups()) {
    double ymin = range_.ymin();

    if (masterYAxis->isIncludeZero())
      ymin = std::min(ymin, 0.0);

    yAxis()->setValueRange(ymin, range_.ymax());
  }
}

bool
CQChartsXYPlot::
xAxisName(QString &name, const QString &def) const
{
  return xColumnName(name, def, false);
}

bool
CQChartsXYPlot::
xColumnName(QString &name, const QString &def, bool tip) const
{
  bool ok;

  name = (tip ? modelHHeaderTip(xColumn(), ok) : modelHHeaderString(xColumn(), ok));

  if (! ok || ! name.length())
    name = def;

  return name.length();
}

bool
CQChartsXYPlot::
yAxisName(QString &name, const QString &def) const
{
  return yColumnName(name, def, false);
}

bool
CQChartsXYPlot::
yColumnName(QString &name, const QString &def, bool tip) const
{
  int ns = numSets();

  if      (canBivariateLines()) {
    name = titleStr();

    if (! name.length()) {
      auto yColumn1 = yColumns().getColumn(0);

      Column yColumn2;

      if (ns > 1)
        yColumn2 = yColumns().getColumn(1);

      bool ok1, ok2;

      auto yname1 = (tip ? modelHHeaderTip(yColumn1, ok1) : modelHHeaderString(yColumn1, ok1));
      auto yname2 = (tip ? modelHHeaderTip(yColumn2, ok2) : modelHHeaderString(yColumn2, ok2));

      name = QString("%1-%2").arg(yname1).arg(yname2);
    }
  }
  else if (isStacked()) {
  }
  else {
    if (! isColumnSeries()) {
      for (int j = 0; j < ns; ++j) {
        bool ok;

        auto yColumn = yColumns().getColumn(j);

        auto name1 = (tip ? modelHHeaderTip(yColumn, ok) : modelHHeaderString(yColumn, ok));
        if (! ok || ! name1.length()) continue;

        if (name.length())
          name += ", ";

        name += name1;
      }
    }
  }

  if (! name.length())
    name = def;

  return name.length();
}

//---

void
CQChartsXYPlot::
setImpulseVisibleSlot(bool b)
{
  setImpulseVisible(b);
}

void
CQChartsXYPlot::
setBivariateLinesSlot(bool b)
{
  setBivariateLines(b);
}

void
CQChartsXYPlot::
setFillUnderFilledSlot(bool b)
{
  setFillUnderFilled(b);
}

//---

void
CQChartsXYPlot::
setMovingAverage(bool b)
{
  CQChartsUtil::testAndSet(movingAverageData_.displayed, b, [&]() {
    drawObjs(); emit customDataChanged(); } );
}

void
CQChartsXYPlot::
setNumMovingAverage(int n)
{
  CQChartsUtil::testAndSet(movingAverageData_.n, n, [&]() { drawObjs(); } );
}

//---

bool
CQChartsXYPlot::
createObjs(PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsXYPlot::createObjs");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsXYPlot *>(this);

  //---

  // calc symbol width (for mouse over of points)
  // TODO: use actual symbol size
  const auto &dataRange = this->dataRange();

  if (dataRange.isSet()) {
    th->symbolWidth_  = (dataRange.xmax() - dataRange.xmin())/100.0;
    th->symbolHeight_ = (dataRange.ymax() - dataRange.ymin())/100.0;
  }

  //---

  th->updateColumnNames();

  //---

  GroupSetIndPoly groupSetIndPoly;

  createGroupSetIndPoly(groupSetIndPoly);

  (void) createGroupSetObjs(groupSetIndPoly, objs);

  //---

  return true;
}

void
CQChartsXYPlot::
updateColumnNames()
{
  // set column header names
  CQChartsPlot::updateColumnNames();

  setColumnHeaderName(labelColumn     (), "Label"     );
  setColumnHeaderName(symbolTypeColumn(), "SymbolType");
  setColumnHeaderName(symbolSizeColumn(), "SymbolSize");
  setColumnHeaderName(fontSizeColumn  (), "FontSize"  );
}

void
CQChartsXYPlot::
createGroupSetIndPoly(GroupSetIndPoly &groupSetIndPoly) const
{
  CQPerfTrace trace("CQChartsXYPlot::createGroupSetIndPoly");

  // create line per set
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsXYPlot *plot) :
     plot_(plot) {
      ns_ = plot_->numSets();

      if (plot_->isColumnSeries())
        plot_->headerSeriesData(sx_);

      int nl = plot_->layers();

      if (nl > 1 && ns_ == 1)
        ns_ = nl;
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      ModelIndex ind(plot_, data.row, plot_->xColumn(), data.parent);

      // get group
      int groupInd = plot_->rowGroupInd(ind);

      auto &setPoly = groupSetPoly_[groupInd];

      if (setPoly.empty())
        setPoly.resize(size_t(ns_));

      //---

      // get x and y values
      double x; std::vector<double> y; QModelIndex rowInd;

      if (! plot_->rowData(data, x, y, rowInd, plot_->isSkipBad()))
        return State::SKIP;

      int ny = int(y.size());

      //---

      int    nl = plot_->layers(); // -1 if not set
      size_t li = 0;

      if (nl > 1 && ny == 1) {
        while (li < size_t(nl - 1) && y[0] > plot_->layerMin() + plot_->layerDelta()) {
          y[0] -= plot_->layerDelta();

          ++li;
        }

        for (size_t i = 0; i < size_t(nl); ++i) {
          double y1 = y[0];

          if      (i < li)
            y1 = plot_->layerMin() + plot_->layerDelta();
          else if (i > li)
            y1 = plot_->layerMin();

          setPoly[i].inds.push_back(rowInd);

          setPoly[i].poly.addPoint(Point(x, y1));
        }
      }
      else {
        if (! plot_->isColumnSeries()) {
          assert(ny == ns_);
        }

        if (! plot_->isColumnSeries()) {
          for (size_t i = 0; i < size_t(ny); ++i) {
            setPoly[i].inds.push_back(rowInd);

            setPoly[i].poly.addPoint(Point(x, y[i]));
          }
        }
        else {
          for (size_t i = 0; i < size_t(ny); ++i) {
            setPoly[size_t(data.row)].inds.push_back(rowInd);

            setPoly[size_t(data.row)].poly.addPoint(Point(sx_[i], y[i]));
          }
        }
      }

      return State::OK;
    }

    // stack lines
    void stack() {
      for (auto &p : groupSetPoly_) {
        auto &setPoly = p.second;

        for (size_t i = 1; i < size_t(ns_); ++i) {
          auto &poly1 = setPoly[i - 1].poly;
          auto &poly2 = setPoly[i    ].poly;

          int np = poly1.size();
          assert(poly2.size() == np);

          for (int j = 0; j < np; ++j) {
            auto p1 = poly1.point(j);

            double y1 = p1.y;

            auto p2 = poly2.point(j);

            double x2 = p2.x, y2 = p2.y;

            if (CMathUtil::isNaN(y1) || CMathUtil::isInf(y1))
              continue;

            poly2.setPoint(j, Point(x2, y2 + y1));
          }
        }
      }
    }

    // cumulate
    void cumulate() {
      for (auto &p : groupSetPoly_) {
        auto &setPoly = p.second;

        for (size_t i = 0; i < size_t(ns_); ++i) {
          auto &poly = setPoly[i].poly;

          int np = poly.size();

          for (int j = 1; j < np; ++j) {
            auto p1 = poly.point(j - 1);
            auto p2 = poly.point(j    );

            double y1 = p1.y;
            double x2 = p2.x, y2 = p2.y;

            if (CMathUtil::isNaN(y1) || CMathUtil::isInf(y1))
              continue;

            poly.setPoint(j, Point(x2, y1 + y2));
          }
        }
      }
    }

    const GroupSetIndPoly &groupSetPoly() const { return groupSetPoly_; }

   private:
    const CQChartsXYPlot* plot_ { nullptr };
    int                   ns_;
    std::vector<double>   sx_;
    GroupSetIndPoly       groupSetPoly_;
  };

  //---

  RowVisitor visitor(this);

  visitModel(visitor);

  if      (isStacked())
    visitor.stack();
  else if (isCumulative())
    visitor.cumulate();

  groupSetIndPoly = visitor.groupSetPoly();
}

bool
CQChartsXYPlot::
createGroupSetObjs(const GroupSetIndPoly &groupSetIndPoly, PlotObjs &objs) const
{
  CQPerfTrace trace("CQChartsXYPlot::createGroupSetObjs");

  //---

  int ns = numSets();

  int ig = 0;
  int ng = int(groupSetIndPoly.size());

  if (ng <= 1 && parentPlot()) {
    ig = parentPlot()->childPlotIndex(this);
    ng = parentPlot()->numChildPlots();
  }

  maxNumPoints_ = 0;

  for (auto &p : groupSetIndPoly) {
    if (isInterrupt())
      return false;

    ColorInd colorInd(ig, ng);

    bool hidden = (ns <= 1 && isSetHidden(ig));

    if (! hidden) {
      int         groupInd = p.first;
      const auto &setPoly  = p.second;

      if      (canBivariateLines()) {
        if (! addBivariateLines(groupInd, setPoly, colorInd, objs))
          return false;
      }
      else {
        if (! addLines(groupInd, setPoly, colorInd, objs))
          return false;
      }
    }

    ++ig;
  }

  return true;
}

bool
CQChartsXYPlot::
addBivariateLines(int groupInd, const SetIndPoly &setPoly,
                  const ColorInd &ig, PlotObjs &objs) const
{
  auto *th = const_cast<CQChartsXYPlot *>(this);

  //---

  double sw = symbolWidth ();
  double sh = symbolHeight();

  PlotObjs pointObjs;

  //---

  // convert lines bivariate lines (line connected each point pair)
  auto ns = setPoly.size();
  if (ns < 1) return false;

  SetIndPoly polygons1, polygons2;

  polygons1.resize(ns - 1);
  polygons2.resize(ns - 1);

  const auto &poly = setPoly[0].poly;

  int np = poly.size();

  for (int ip = 0; ip < np; ++ip) {
    if (isInterrupt())
      return false;

    double x = 0.0;

    // sorted y vals
    std::set<double> sortedYVals;

    for (size_t j = 0; j < ns; ++j) {
      bool hidden = isSetHidden(int(j));

      if (hidden)
        continue;

      //---

      const auto &poly = setPoly[size_t(j)].poly;

      auto p = poly.point(ip);

      if (CMathUtil::isNaN(p.y) || CMathUtil::isInf(p.y))
        continue;

      if (j == 0)
        x = p.x;

      sortedYVals.insert(p.y);
    }

    // need at least 2 values
    if (sortedYVals.size() < 2)
      continue;

    //---

    std::vector<double> yVals;

    for (const auto &y : sortedYVals)
      yVals.push_back(y);

    //---

    QModelIndex parent; // TODO: parent

    ModelIndex xModelInd(th, ip, xColumn(), parent);

    auto xind  = modelIndex(xModelInd);
    auto xind1 = normalizeIndex(xind);

    //---

    // connect each y value to next y value
    double y1  = yVals[0];
    auto   ny1 = yVals.size();

    for (size_t j = 1; j < ny1; ++j) {
      if (isInterrupt())
        return false;

      double y2 = yVals[j];

      BBox bbox(x - sw/2, y1 - sh/2, x + sw/2, y2 + sh/2);

      if (! isFillUnderFilled()) {
        // use vertical line object for each point pair if not fill under
        ColorInd is(int(j - 1), int(ny1 - 1));
        ColorInd iv(ip, np);

        auto gbbox = adjustGroupBBox(groupInd, bbox);

        auto gp1 = adjustGroupPoint(groupInd, Point(x, y1));
        auto gp2 = adjustGroupPoint(groupInd, Point(x, y2));

        auto *lineObj = th->createBiLineObj(groupInd, gbbox, gp1, gp2, xind1, is, ig, iv);

        objs.push_back(lineObj);
      }
      else {
        auto &poly1 = polygons1[j - 1].poly;
        auto &poly2 = polygons2[j - 1].poly;

        // build lower and upper poly line for fill under polygon
        poly1.addPoint(Point(x, y1));
        poly2.addPoint(Point(x, y2));
      }

      y1 = y2;
    }
  }

  //---

  // add lower, upper and polygon objects for fill under
  if (isFillUnderFilled()) {
    auto name = titleStr();

    if (! name.length()) {
      auto yColumn1 = yColumns().getColumn(0);

      Column yColumn2;

      if (yColumns().count() > 1)
        yColumn2 = yColumns().getColumn(1);

      bool ok;

      auto yname1 = modelHHeaderString(yColumn1, ok);
      auto yname2 = modelHHeaderString(yColumn2, ok);

      name = QString("%1-%2").arg(yname1).arg(yname2);
    }

    int ns = numSets();

    for (int j = 1; j < ns; ++j) {
      bool hidden = isSetHidden(j);

      if (hidden)
        continue;

      //---

      QString name1;

      if (ns > 2)
        name1 = QString("%1:%2").arg(name).arg(j);
      else
        name1 = name;

      //---

      auto &poly1 = polygons1[size_t(j - 1)].poly;
      auto &poly2 = polygons2[size_t(j - 1)].poly;

      ColorInd is1(j - 1, ns - 1);

      addPolyLine(poly1, groupInd, is1, ig, name1, pointObjs, objs);
      addPolyLine(poly2, groupInd, is1, ig, name1, pointObjs, objs);

      int len = poly1.size();

      auto fillUnderSideType = fillUnderSide().type();

      if      (fillUnderSideType == FillUnderSide::Type::BOTH) {
        // add upper poly line to lower one (points reversed) to build fill polygon
        for (int k = len - 1; k >= 0; --k)
          poly1.addPoint(poly2.point(k));
      }
      else if (fillUnderSideType == FillUnderSide::Type::ABOVE) {
        Polygon poly3, poly4;
        Point   pa1, pb1;

        bool above1 = true;

        for (int k = 0; k < len; ++k) {
          auto pa2 = poly1.point(k);
          auto pb2 = poly2.point(k);

          bool above2 = (pa2.y > pb2.y);

          if (k > 0 && above1 != above2) {
            Point  pi;
            double mu1, mu2;

            CQChartsUtil::intersectLines(pa1, pa2, pb1, pb2, pi, mu1, mu2);

            poly3.addPoint(pi);
            poly4.addPoint(pi);
          }

          if (above2) {
            poly3.addPoint(pa2);
            poly4.addPoint(pb2);
          }

          pa1 = pa2; pb1 = pb2; above1 = above2;
        }

        len = poly4.size();

        for (int k = len - 1; k >= 0; --k)
          poly3.addPoint(poly4.point(k));

        poly1 = poly3;
      }
      else if (fillUnderSideType == FillUnderSide::Type::BELOW) {
        Polygon poly3, poly4;
        Point   pa1, pb1;

        bool below1 = true;

        for (int k = 0; k < len; ++k) {
          auto pa2 = poly1.point(k);
          auto pb2 = poly2.point(k);

          bool below2 = (pa2.y < pb2.y);

          if (k > 0 && below1 != below2) {
            Point  pi;
            double mu1, mu2;

            CQChartsUtil::intersectLines(pa1, pa2, pb1, pb2, pi, mu1, mu2);

            poly3.addPoint(pi);
            poly4.addPoint(pi);
          }

          if (below2) {
            poly3.addPoint(pa2);
            poly4.addPoint(pb2);
          }

          pa1 = pa2; pb1 = pb2; below1 = below2;
        }

        len = poly4.size();

        for (int k = len - 1; k >= 0; --k)
          poly3.addPoint(poly4.point(k));

        poly1 = poly3;
      }

      addPolygon(poly1, groupInd, is1, ig, name1, objs, /*under*/false);
    }
  }

  return true;
}

bool
CQChartsXYPlot::
addLines(int groupInd, const SetIndPoly &setPoly, const ColorInd &ig, PlotObjs &objs) const
{
  auto *th = const_cast<CQChartsXYPlot *>(this);

  PlotObjs polyLineObjs, pointObjs, labelObjs, impulseLineObjs, polygonObjs;

  //---

  initSymbolTypeData();
  initSymbolSizeData();
  initFontSizeData  ();

  //---

  PlotObjs linePointObjs;

  double sw = symbolWidth ();
  double sh = symbolHeight();

  //---

  const auto &dataRange = this->dataRange();

  //---

  // convert lines into set polygon and set poly lines (more than one if NaNs)
  int ns = numSets();

  //---

  bool isHorizon = false;

  int nl = layers();

  if (nl > 1 && ns == 1) {
    isHorizon = true;
    ns        = nl;
  }

  //---

  for (int is = 0; is < ns; ++is) {
    if (isInterrupt())
      return false;

    bool hidden = (ns > 1 && isSetHidden(is));

    if (hidden)
      continue;

    //---

    // get column name
    QString name;

    if (! isColumnSeries()) {
      auto yColumn = (! isHorizon ? yColumns().getColumn(is) : yColumns().getColumn(0));

      bool ok;

      name = modelHHeaderString(yColumn, ok);
    }

    if (ig.n > 1)
      name = groupIndName(groupInd);

    //---

    const auto &setPoly1 = setPoly[size_t(is)];

    const auto &poly = setPoly1.poly;
    const auto &inds = setPoly1.inds;

    const auto &prevPoly = (is > 0 ? setPoly[size_t(is - 1)].poly : poly);

    //---

    Polygon polyShape, polyLine;

    int np = poly.size();
    assert(prevPoly.size() == np);

    maxNumPoints_ = std::max(maxNumPoints_, np);

    //---

    auto pointStartIndex = [&]() {
      if      (pointStart_ == 0)
        return 0;
      else if (pointStart_ == -1)
        return np - 1;
      else if (pointStart_ == -2)
        return np/2;
      else
        return pointStart_;
    };

    auto validPointIndex = [&](int ip, int np) {
      if      (pointCount_ == 1) {
        return (ip == pointStartIndex());
      }
      else if (pointCount_ > 1) {
        if (ip == 0 || ip == np - 1)
          return true;

        int d = np/(pointCount_ - 1);

        return ((ip % d) == 0);
      }
      else if (pointDelta_ > 1) {
        return ((ip % pointDelta_) == 0);
      }
      else {
        return true;
      }
    };

    //---

    // find first and last valid point
    int ip1 = 0;
    int ip2 = np - 1;

    while (ip1 < np) {
      auto p = poly.point(ip1);

      if (! CMathUtil::isNaN(p.x) && ! CMathUtil::isInf(p.x) &&
          ! CMathUtil::isNaN(p.y) && ! CMathUtil::isInf(p.y))
        break;

      ++ip1;
    }

    while (ip2 >= 0) {
      auto p = poly.point(ip2);

      if (! CMathUtil::isNaN(p.x) && ! CMathUtil::isInf(p.x) &&
          ! CMathUtil::isNaN(p.y) && ! CMathUtil::isInf(p.y))
        break;

      --ip2;
    }

    //---

    // add points to fill under polygon, poly line, and create point objects
    for (int ip = 0; ip < np; ++ip) {
      if (isInterrupt())
        return false;

      //---

      // get polygon point
      auto p = poly.point(ip);

      //---

      // if point is invalid start new poly line if needed and skip
      if (CMathUtil::isNaN(p.x) || CMathUtil::isInf(p.x) ||
          CMathUtil::isNaN(p.y) || CMathUtil::isInf(p.y)) {
        if (polyLine.size()) {
          ColorInd is1(is, ns);

          addPolyLine(polyLine, groupInd, is1, ig, name, linePointObjs, polyLineObjs);

          linePointObjs.clear();

          polyLine = Polygon();
        }

        continue;
      }

      //---

      bool valid = validPointIndex(ip, np);

      if (valid) {
        const auto &xind = inds[size_t(ip)];

        auto xind1 = normalizeIndex(xind);

        //---

        // get symbol size (needed for bounding box)
        Length          symbolSize;
        Qt::Orientation sizeDir { Qt::Horizontal };

        if (symbolSizeColumn().isValid()) {
          if (! columnSymbolSize(xind.row(), xind.parent(), symbolSize, sizeDir))
            symbolSize = Length();
        }

        double sx, sy;

        plotSymbolSize(symbolSize.isValid() ? symbolSize : this->symbolSize(), sx, sy, sizeDir);

        //---

        // create point object
        ColorInd is1(is, ns);
        ColorInd iv1(ip, np);

        auto gp = adjustGroupPoint(groupInd, p);

        BBox gbbox(gp.x - sx, gp.y - sy, gp.x + sx, gp.y + sy);

        auto *pointObj = th->createPointObj(groupInd, gbbox, gp, xind1, is1, ig, iv1);

        if (symbolSize.isValid())
          pointObj->setSymbolSize(symbolSize);

        pointObjs.push_back(pointObj);

        linePointObjs.push_back(pointObj);

        //---

        // set optional symbol
        CQChartsSymbol symbol;

        if (symbolTypeColumn().isValid()) {
          if (! columnSymbolType(xind.row(), xind.parent(), symbol))
            symbol = CQChartsSymbol();
        }

        if (symbol.isValid())
          pointObj->setSymbol(symbol);

        //---

        // set optional font size
        Length          fontSize;
        Qt::Orientation fontSizeDir { Qt::Horizontal };

        if (fontSizeColumn().isValid()) {
          if (! columnFontSize(xind.row(), xind.parent(), fontSize, fontSizeDir))
            fontSize = Length();
        }

        if (fontSize.isValid())
          pointObj->setFontSize(fontSize);

        //---

        // set optional symbol fill color
        Color symbolColor;

        if (colorColumn().isValid()) {
          if (! colorColumnColor(xind.row(), xind.parent(), symbolColor))
            symbolColor = Color();
        }

        if (symbolColor.isValid())
          pointObj->setColor(symbolColor);

        //---

        // set optional point label
        QString pointName;
        Column  pointNameColumn;

        if (labelColumn().isValid()) {
          ModelIndex labelModelInd(th, xind.row(), labelColumn(), xind.parent());

          bool ok;
          pointName = modelString(labelModelInd, ok);
          if (ok) pointNameColumn = labelColumn();
        }

        if (pointNameColumn.isValid() && pointName.length()) {
          BBox bbox(p.x - sw/2, p.y - sh/2, p.x + sw/2, p.y + sh/2);

          auto gbbox = adjustGroupBBox(groupInd, bbox);
          auto gp    = adjustGroupPoint(groupInd, p);

          auto *labelObj = th->createLabelObj(groupInd, gbbox, gp, pointName, xind1, is1, iv1);

          labelObj->setLabelColumn(pointNameColumn);

          labelObjs.push_back(labelObj);

          labelObj->setPointObj(pointObj);
          pointObj->setLabelObj(labelObj);
        }

        //---

        // set optional image
        CQChartsImage image;

        if (imageColumn().isValid()) {
          ModelIndex imageColumnInd(th, xind.row(), imageColumn(), xind.parent());

          bool ok;

          auto imageVar = modelValue(imageColumnInd, ok);

          if (ok)
            image = CQChartsVariant::toImage(imageVar, ok);
        }

        if (image.isValid())
          pointObj->setImage(image);

        //---

        // set vector data
        if (isVectors()) {
          QModelIndex parent; // TODO: parent

          double vx = 0.0, vy = 0.0;

          if (vectorXColumn().isValid()) {
            bool ok;

            ModelIndex vectorXInd(th, xind.row(), vectorXColumn(), parent);

            vx = modelReal(vectorXInd, ok);

            if (! ok)
              th->addDataError(vectorXInd, "Invalid Vector X");
          }

          if (vectorYColumn().isValid()) {
            bool ok;

            ModelIndex vectorYInd(th, xind.row(), vectorYColumn(), parent);

            vy = modelReal(vectorYInd, ok);

            if (! ok)
              th->addDataError(vectorYInd, "Invalid Vector Y");
          }

          pointObj->setVector(Point(vx, vy));
        }

        //---

        // add impulse line (down to or up to zero)
        if (calcImpulseVisible()) {
          double w;

          if (isImpulseLines())
            w = lengthPlotWidth(impulseStrokeWidth());
          else
            w = lengthPlotWidth(impulseWidth());

          double y1 = 0.0;

          if (dataRange.isSet()) {
            y1 = drawRangeYMin(groupInd);

            if (y1 <= 0.0 && drawRangeYMax(groupInd) >= 0.0)
              y1 = 0.0;
          }

          double ys = std::min(p.y, y1);
          double ye = std::max(p.y, y1);

          BBox bbox(p.x - w/2, ys, p.x + w/2, ye);

          auto gbbox = adjustGroupBBox(groupInd, bbox);

          auto gp1 = adjustGroupPoint(groupInd, Point(p.x, ys));
          auto gp2 = adjustGroupPoint(groupInd, Point(p.x, ye));

          auto *impulseObj = th->createImpulseLineObj(groupInd, gbbox, gp1, gp2,
                                                      xind1, is1, ig, iv1);

          impulseLineObjs.push_back(impulseObj);
        }
      }

      //---

      // add point to poly line
      polyLine.addPoint(p);

      //---

      // add point to polygon

      // if first point then add first point of previous polygon
      if (ip == ip1 && dataRange.isSet()) {
        if (isStacked()) {
          double y1 = (is > 0 ? prevPoly.point(ip).y : drawRangeYMin(groupInd));

          if (CMathUtil::isNaN(y1) || CMathUtil::isInf(y1))
            y1 = drawRangeYMin(groupInd);

          polyShape.addPoint(Point(p.x, y1));
        }
        else {
          polyShape.addPoint(calcFillUnderPos(groupInd, p.x, drawRangeYMin(groupInd)));
        }
      }

      polyShape.addPoint(p);

      // if last point then add last point of previous polygon
      if (ip == ip2 && dataRange.isSet()) {
        if (isStacked()) {
          double y1 = (is > 0 ? prevPoly.point(ip).y : drawRangeYMin(groupInd));

          if (CMathUtil::isNaN(y1) || CMathUtil::isInf(y1))
            y1 = drawRangeYMin(groupInd);

          polyShape.addPoint(Point(p.x, y1));
        }
        else {
          polyShape.addPoint(calcFillUnderPos(groupInd, p.x, drawRangeYMin(groupInd)));
        }
      }
    }

    //---

    if (isStacked()) {
      // add points from previous polygon to bottom of polygon
      if (is > 0) {
        for (int ip = np - 2; ip >= 1; --ip) {
          double x1 = prevPoly.point(ip).x;
          double y1 = prevPoly.point(ip).y;

          if (CMathUtil::isNaN(x1) || CMathUtil::isInf(x1))
            x1 = poly.point(ip).x;

          if (CMathUtil::isNaN(y1) || CMathUtil::isInf(y1))
            y1 = 0.0;

          polyShape.addPoint(Point(x1, y1));
        }
      }
    }

    //---

    if (polyLine.size()) {
      ColorInd is1(is, ns);

      addPolyLine(polyLine, groupInd, is1, ig, name, linePointObjs, polyLineObjs);

      linePointObjs.clear();

      //polyLine = Polygon();
    }

    //---

    ColorInd is1(is, ns);

    addPolygon(polyShape, groupInd, is1, ig, name, polygonObjs, /*under*/true);
  }

  //---

  for (auto &obj : polygonObjs)
    objs.push_back(obj);

  for (auto &obj : polyLineObjs)
    objs.push_back(obj);

  for (auto &obj : impulseLineObjs)
    objs.push_back(obj);

  for (auto &obj : pointObjs)
    objs.push_back(obj);

  for (auto &obj : labelObjs)
    objs.push_back(obj);

  return true;
}

//---

bool
CQChartsXYPlot::
calcGroupHidden(int groupInd) const
{
  return (numSets() <= 1 && isSetHidden(groupInd));
}

int
CQChartsXYPlot::
numVisibleGroups() const
{
  int ng = numGroups();
  int nv = ng;

  for (int i = 0; i < ng; ++i) {
    if (calcGroupHidden(i))
      --nv;
  }

  return nv;
}

int
CQChartsXYPlot::
mapVisibleGroup(int groupInd) const
{
  int ng = numGroups();
  int ig = 0;

  for (int i = 0; i < ng; ++i) {
    bool hidden = calcGroupHidden(i);

    if (i == groupInd)
      return (! hidden ? ig : -1);

    if (! hidden)
      ++ig;
  }

  return -1;
}

int
CQChartsXYPlot::
unmapVisibleGroup(int groupInd) const
{
  int ng = numGroups();
  int ig = 0;

  for (int i = 0; i < ng; ++i) {
    bool hidden = calcGroupHidden(i);

    if (! hidden) {
      if (groupInd == ig)
        return i;

      ++ig;
    }
  }

  return -1;
}

//---

bool
CQChartsXYPlot::
headerSeriesData(std::vector<double> &x) const
{
  int nc = yColumns().count();

  for (int i = 0; i < nc; ++i) {
    auto yColumn = yColumns().getColumn(i);

    bool ok;

    auto var = CQChartsModelUtil::modelHeaderValue(model().data(), yColumn, Qt::EditRole, ok);

    if (! ok)
      var = CQChartsModelUtil::modelHeaderValue(model().data(), yColumn, Qt::DisplayRole, ok);

    double r = CQChartsVariant::toReal(var, ok);

    if (! ok)
      r = i;

    x.push_back(r);
  }

  return true;
}

bool
CQChartsXYPlot::
rowData(const ModelVisitor::VisitData &data, double &x, std::vector<double> &y,
        QModelIndex &ind, bool skipBad) const
{
  auto *th = const_cast<CQChartsXYPlot *>(this);

  double defVal = getRowBadValue(data.row);

  //---

  // get x value (must be valid)
  bool ok1 = true;

  ModelIndex xModelInd(th, data.row, xColumn(), data.parent);

  ind = modelIndex(xModelInd);

  if (! calcMapXColumn()) {
    bool ok = modelMappedReal(xModelInd, x, isLogX(), defVal);

    if (! ok) {
      th->addDataError(xModelInd, "Invalid X Value");
      ok1 = false;
    }
  }
  else {
    bool ok;

    auto var = modelValue(xModelInd, ok);

    if (var.isValid()) {
      auto *columnDetails = this->columnDetails(xColumn());

      if (columnDetails)
        x = columnDetails->uniqueId(var);
    }
    else {
      th->addDataError(xModelInd, "Invalid X Value");
      ok1 = false;
    }
  }

  //---

  // get y values (optionally skip bad)
  // TODO: differentiate between bad value and empty value
  bool ok2 = true;

  int nc = yColumns().count();

  for (int i = 0; i < nc; ++i) {
    auto yColumn = yColumns().getColumn(i);

    ModelIndex yModelInd(th, data.row, yColumn, data.parent);

    double y1;

    bool ok = modelMappedReal(yModelInd, y1, isLogY(), defVal);

    if (! ok) {
      y1 = CMathUtil::getNaN();

      if (! skipBad)
        ok2 = false;
    }

    //---

    y.push_back(y1);
  }

  if (nc && y.empty()) {
    th->addDataError(xModelInd, "Invalid Y Value(s)");
    return false;
  }

  return (ok1 && ok2);
}

int
CQChartsXYPlot::
numSets() const
{
  if (isColumnSeries())
    return model()->rowCount();
  else
    return yColumns().count();
}

CQChartsGeom::Point
CQChartsXYPlot::
calcFillUnderPos(int groupInd, double x, double y) const
{
  const auto &pos = fillUnderPos();

  double x1 = x;
  double y1 = y;

  //---

  const auto &dataRange = this->dataRange();

  //---

  if      (pos.xtype() == FillUnderPos::Type::MIN) {
    if (dataRange.isSet())
      x1 = drawRangeXMin(groupInd);
  }
  else if (pos.xtype() == FillUnderPos::Type::MAX) {
    if (dataRange.isSet())
      x1 = dataRange.xmax();
  }
  else if (pos.xtype() == FillUnderPos::Type::POS)
    x1 = pos.xpos();

  if      (pos.ytype() == FillUnderPos::Type::MIN) {
    if (dataRange.isSet())
      y1 = drawRangeYMin(groupInd);
  }
  else if (pos.ytype() == FillUnderPos::Type::MAX) {
    if (dataRange.isSet())
      y1 = drawRangeYMax(groupInd);
  }
  else if (pos.ytype() == FillUnderPos::Type::POS)
    y1 = pos.ypos();

  return Point(x1, y1);
}

CQChartsXYPolylineObj *
CQChartsXYPlot::
addPolyLine(const Polygon &poly, int groupInd, const ColorInd &is,
            const ColorInd &ig, const QString &name, PlotObjs &pointObjs, PlotObjs &objs) const
{
  auto bbox = poly.boundingBox();
  if (! bbox.isSet()) return nullptr;

  auto *th = const_cast<CQChartsXYPlot *>(this);

  auto gbbox = adjustGroupBBox(groupInd, bbox);
  auto gpoly = adjustGroupPoly(groupInd, poly);

  auto *lineObj = th->createPolylineObj(groupInd, gbbox, gpoly, name, is, ig);

  lineObj->setPointObjs(pointObjs);

  for (auto &pointObj : pointObjs) {
    auto *pointObj1 = dynamic_cast<CQChartsXYPointObj *>(pointObj);

    if (pointObj1)
      pointObj1->setLineObj(lineObj);
  }

  objs.push_back(lineObj);

  return lineObj;
}

void
CQChartsXYPlot::
addPolygon(const Polygon &poly, int groupInd, const ColorInd &is,
           const ColorInd &ig, const QString &name, PlotObjs &objs, bool under) const
{
  auto bbox = poly.boundingBox();
  if (! bbox.isSet()) return;

  auto *th = const_cast<CQChartsXYPlot *>(this);

  auto gbbox = adjustGroupBBox(groupInd, bbox);
  auto gpoly = adjustGroupPoly(groupInd, poly);

  auto *polyObj = th->createPolygonObj(groupInd, gbbox, gpoly, name, is, ig, under);

  objs.push_back(polyObj);
}

//---

double
CQChartsXYPlot::
drawRangeXMin(int groupInd, bool adjust) const
{
  if (isSplitGroups()) {
    const auto &range = getGroupRange(groupInd);

    double xmin = range.xmin();

    if (adjust)
      xmin = mapGroupX(range, groupInd, xmin);

    return xmin;
  }
  else {
    const auto &dataRange = this->dataRange();

    return dataRange.xmin();
  }
}

double
CQChartsXYPlot::
drawRangeXMax(int groupInd, bool adjust) const
{
  if (isSplitGroups()) {
    const auto &range = getGroupRange(groupInd);

    double xmax = range.xmax();

    if (adjust)
      xmax = mapGroupX(range, groupInd, xmax);

    return xmax;
  }
  else {
    const auto &dataRange = this->dataRange();

    return dataRange.xmax();
  }
}

double
CQChartsXYPlot::
drawRangeYMin(int groupInd) const
{
  if (isSplitGroups()) {
    double ymin;

    if (! isSplitSharedY()) {
      const auto &range = getGroupRange(groupInd);

      ymin = range.ymin();
    }
    else
      ymin = range_.ymin();

    auto *masterYAxis = yAxis();

    if (masterYAxis->isIncludeZero())
      ymin = std::min(ymin, 0.0);

    return ymin;
  }
  else {
    const auto &dataRange = this->dataRange();

    return dataRange.ymin();
  }
}

double
CQChartsXYPlot::
drawRangeYMax(int groupInd) const
{
  if (isSplitGroups()) {
    if (! isSplitSharedY()) {
      const auto &range = getGroupRange(groupInd);

      return range.ymax();
    }
    else
      return range_.ymax();
  }
  else {
    const auto &dataRange = this->dataRange();

    return dataRange.ymax();
  }
}

//---

CQChartsXYPointObj *
CQChartsXYPlot::
createPointObj(int groupInd, const BBox &rect, const Point &p, const QModelIndex &ind,
               const ColorInd &is, const ColorInd &ig, const ColorInd &iv) const
{
  return new CQChartsXYPointObj(this, groupInd, rect, p, ind, is, ig, iv);
}

CQChartsXYBiLineObj *
CQChartsXYPlot::
createBiLineObj(int groupInd, const BBox &rect, const Point &p1, const Point &p2,
                const QModelIndex &ind, const ColorInd &is, const ColorInd &ig,
                const ColorInd &iv) const
{
  return new CQChartsXYBiLineObj(this, groupInd, rect, p1.x, p1.y, p2.y, ind, is, ig, iv);
}

CQChartsXYLabelObj *
CQChartsXYPlot::
createLabelObj(int groupInd, const BBox &rect, const Point &p, const QString &label,
               const QModelIndex &ind, const ColorInd &is, const ColorInd &iv) const
{
  return new CQChartsXYLabelObj(this, groupInd, rect, p.x, p.y, label, ind, is, iv);
}

CQChartsXYImpulseLineObj *
CQChartsXYPlot::
createImpulseLineObj(int groupInd, const BBox &rect, const Point &p1, const Point &p2,
                     const QModelIndex &ind, const ColorInd &is, const ColorInd &ig,
                     const ColorInd &iv) const
{
  return new CQChartsXYImpulseLineObj(this, groupInd, rect, p1.x, p1.y, p2.y, ind, is, ig, iv);
}

CQChartsXYPolylineObj *
CQChartsXYPlot::
createPolylineObj(int groupInd, const BBox &rect, const Polygon &poly, const QString &name,
                  const ColorInd &is, const ColorInd &ig) const
{
  return new CQChartsXYPolylineObj(this, groupInd, rect, poly, name, is, ig);
}

CQChartsXYPolygonObj *
CQChartsXYPlot::
createPolygonObj(int groupInd, const BBox &rect, const Polygon &poly, const QString &name,
                 const ColorInd &is, const ColorInd &ig, bool under) const
{
  return new CQChartsXYPolygonObj(this, groupInd, rect, poly, name, is, ig, under);
}

//---

QString
CQChartsXYPlot::
valueName(int is, int ns, int irow, bool tip) const
{
  QString name;

  if (ns > 1 && is >= 0) {
    if (! isColumnSeries()) {
      auto yColumn = yColumns().getColumn(is);

      bool ok;

      name = (tip ? modelHHeaderTip(yColumn, ok) : modelHHeaderString(yColumn, ok));
    }
  }

  auto *th = const_cast<CQChartsXYPlot *>(this);

  if (labelColumn().isValid()) {
    QModelIndex parent; // TODO: parent

    ModelIndex labelModelInd(th, irow, labelColumn(), parent);

    bool ok;

    auto name1 = modelString(labelModelInd, ok);

    if (ok)
      return name1;
  }

  return name;
}

void
CQChartsXYPlot::
addKeyItems(PlotKey *key)
{
  // start at next row (vertical) or next column (horizontal) from previous key
  int row = (! key->isHorizontal() ? key->maxRow() : 0);
  int col = (! key->isHorizontal() ? 0 : key->maxCol());

  auto addColorKeyItem = [&](const QString &name, const ColorInd &is, const ColorInd &ig) {
    auto *colorItem = new CQChartsXYColorKeyItem(this, is, ig);
    auto *textItem  = new CQChartsXYTextKeyItem (this, name, is, ig);

    auto *groupItem = new CQChartsGroupKeyItem(this);

    groupItem->addRowItems(colorItem, textItem);

    key->addItem(groupItem, row, col);

    key->nextRowCol(row, col);
  };

  auto addGradientKeyItem = [&](double min, double max) {
    auto *gradientItem = new CQChartsGradientKeyItem(this);

    gradientItem->setMinValue(min);
    gradientItem->setMaxValue(max);

    key->addItem(gradientItem, row, col);

    key->nextRowCol(row, col);
  };

  //---

  int ns = numSets();
  int ng = numGroups();

  //---

  bool isHorizon = false;

  int nl = layers();

  if (nl > 1 && ns == 1) {
    isHorizon = true;
    ns        = nl;
  }

  //---

  if      (canBivariateLines()) {
    auto name = titleStr();

    if (! name.length()) {
      auto yColumn1 = yColumns().getColumn(0);

      Column yColumn2;

      if (ns > 1)
        yColumn2 = yColumns().getColumn(1);

      bool ok;

      auto yname1 = modelHHeaderString(yColumn1, ok);
      auto yname2 = modelHHeaderString(yColumn2, ok);

      name = QString("%1-%2").arg(yname1).arg(yname2);
    }

    ColorInd is, ig;

    addColorKeyItem(name, is, ig);
  }
  else if (isStacked()) {
    for (int i = 0; i < ns; ++i) {
      QString name;

      if (! isColumnSeries()) {
        auto yColumn = (! isHorizon ? yColumns().getColumn(i) : yColumns().getColumn(0));

        bool ok;

        name = modelHHeaderString(yColumn, ok);
      }

      ColorInd is(i, ns), ig;

      addColorKeyItem(name, is, ig);
    }
  }
  else {
    const auto &dataRange = this->dataRange();

    // colored by x axis value
    if      (colorType() == ColorType::X_VALUE) {
      addGradientKeyItem(dataRange.xmin(), dataRange.xmax());
    }
    // colored by y axis value
    else if (colorType() == ColorType::Y_VALUE) {
      addGradientKeyItem(dataRange.ymin(), dataRange.ymax());
    }
    // colored by index
    else if (colorType() == ColorType::INDEX) {
      addGradientKeyItem(0, maxNumPoints_);
    }
    else {
      if      (ns > 1) {
        for (int i = 0; i < ns; ++i) {
          QString name;

          if (! isColumnSeries()) {
            auto yColumn = (! isHorizon ? yColumns().getColumn(i) : yColumns().getColumn(0));

            bool ok;

            name = modelHHeaderString(yColumn, ok);
          }

  #if 0
          if (ns == 1 && (name == "" || name == QString::number(yColumn + 1))) {
            if      (titleStr().length())
              name = titleStr();
            else if (filename().length())
              name = filename();
          }
  #endif
  #if 0
          if (ns == 1 && ! isOverlay() && (titleStr().length() || filename().length())) {
            if      (titleStr().length())
              name = titleStr();
            else if (filename().length())
              name = filename();
          }
  #endif

          if (isHorizon) {
            double y1 = layerMin() + i*layerDelta();
            double y2 = y1 + layerDelta();

            name += QString(" [%1,%2)").arg(y1).arg(y2);
          }

          ColorInd is(i, ns), ig;

          addColorKeyItem(name, is, ig);
        }
      }
      else if (ng > 1) {
        for (int i = 0; i < ng; ++i) {
          auto name = groupIndName(i);

          ColorInd is, ig(i, ng);

          addColorKeyItem(name, is, ig);
        }
      }
      else {
        auto name = groupIndName(0);

        if (name == "")
          (void) yAxisName(name);

        if (name == "")
          name = titleStr();

        ColorInd is, ig;

        if (parentPlot()) {
          int i = parentPlot()->childPlotIndex(this);
          int n = parentPlot()->numChildPlots();

          ig = ColorInd(i, n);
        }

        addColorKeyItem(name, is, ig);
      }
    }
  }

  key->plot()->updateKeyPosition(/*force*/true);
}

//---

bool
CQChartsXYPlot::
probe(ProbeData &probeData) const
{
  InterpValues yvals;

  if (! interpY(probeData.p.x, yvals))
    return false;

  for (const auto &yval : yvals)
    probeData.yvals.emplace_back(yval.y, columnHeaderName(yval.column), "");

  return true;
}

bool
CQChartsXYPlot::
interpY(double x, InterpValues &yvals) const
{
  if (canBivariateLines())
    return false;

  for (const auto &plotObj : plotObjs_) {
    auto *polyObj = dynamic_cast<PolylineObj *>(plotObj);

    if (! polyObj)
      continue;

    Column yColumn;

    if (! isColumnSeries())
      yColumn = yColumns().getColumn(polyObj->is().i);
    else
      yColumn = yColumns().getColumn(0);

    std::vector<double> yvals1;

    polyObj->interpY(x, yvals1);

    for (const auto &y1 : yvals1)
      yvals.emplace_back(y1, yColumn);
  }

  return ! yvals.empty();
}

//------

CQChartsXYPolylineObj *
CQChartsXYPlot::
getGroupObj(int ig) const
{
  for (const auto &plotObj : plotObjs_) {
    auto *polyObj = dynamic_cast<PolylineObj *>(plotObj);

    if (polyObj) {
      if (polyObj->is().n == 1 && polyObj->ig().n > 1 && polyObj->ig().i == ig)
        return polyObj;
    }
  }

  return nullptr;
}

//------

CQChartsGeom::BBox
CQChartsXYPlot::
dataFitBBox() const
{
  auto bbox = CQChartsPlot::dataFitBBox();

  for (const auto &plotObj : plotObjs_) {
    auto *labelObj = dynamic_cast<CQChartsXYLabelObj    *>(plotObj);
    auto *lineObj  = dynamic_cast<CQChartsXYPolylineObj *>(plotObj);

    if      (labelObj)
      bbox += labelObj->rect();
    else if (lineObj)
      bbox += lineObj->fitBBox();
  }

  if (calcImpulseVisible()) {
    double lw = pixelToWindowWidth(calcImpulsePixelWidth());

    bbox += Point(bbox.getXMin() - lw/2, bbox.getYMin());
    bbox += Point(bbox.getXMax() + lw/2, bbox.getYMin());
  }

  return bbox;
}

//---

QString
CQChartsXYPlot::
posStr(const Point &w) const
{
  if (isSplitGroups()) {
    int ng = numVisibleGroups();

    auto groupInd = std::min(std::max(int(w.x), 0), ng - 1);

    const auto &range = getGroupRange(groupInd);

    double x = CMathUtil::map(w.x, groupInd, groupInd + 1, range.xmin(), range.xmax());
    double y = CMathUtil::map(w.y, 0.0, 1.0, range_.ymin(), range_.ymax());

    return CQChartsPlot::posStr(Point(x, y));
  }

  return CQChartsPlot::posStr(w);
}

//---

bool
CQChartsXYPlot::
addMenuItems(QMenu *menu)
{
  int ns = numSets();

  menu->addSeparator();

  //---

  if (ns > 1)
    addMenuCheckedAction(menu, "Bivariate", isBivariateLines(), SLOT(setBivariateLinesSlot(bool)));

  //---

  addMenuCheckedAction(menu, "Points", isPoints(), SLOT(setPointsSlot(bool)));
  addMenuCheckedAction(menu, "Lines" , isLines(), SLOT(setLinesSlot(bool)));

  if (labelColumn().isValid())
    addMenuCheckedAction(menu, "Labels", isPointLabels(), SLOT(setPointLabels(bool)));

  menu->addSeparator();

  //---

  addMenuCheckedAction(menu, "Stacked"   , isStacked        (), SLOT(setStacked(bool)));
  addMenuCheckedAction(menu, "Cumulative", isCumulative     (), SLOT(setCumulative(bool)));
  addMenuCheckedAction(menu, "Impulse"   , isImpulseVisible (), SLOT(setImpulseVisibleSlot(bool)));
  addMenuCheckedAction(menu, "Fill Under", isFillUnderFilled(), SLOT(setFillUnderFilledSlot(bool)));

  //---

  auto *overlaysMenu = new QMenu("Overlays", menu);

  (void) addMenuCheckedAction(overlaysMenu, "Best Fit"   ,
                              isBestFit      (), SLOT(setBestFit      (bool)));
  (void) addMenuCheckedAction(overlaysMenu, "Hull"       ,
                              isHull         (), SLOT(setHull         (bool)));
  (void) addMenuCheckedAction(overlaysMenu, "Stats Lines",
                              isStatsLines   (), SLOT(setStatsLines   (bool)));
  (void) addMenuCheckedAction(overlaysMenu, "Moving Average",
                              isMovingAverage(), SLOT(setMovingAverage(bool)));

  menu->addMenu(overlaysMenu);

  //---

  auto *xMenu = new QMenu("X Axis Annotation", menu);
  auto *yMenu = new QMenu("Y Axis Annotation", menu);

  (void) addMenuCheckedAction(xMenu, "Rug", isXRug(), SLOT(setXRug(bool)));
  (void) addMenuCheckedAction(yMenu, "Rug", isYRug(), SLOT(setYRug(bool)));

  menu->addMenu(xMenu);
  menu->addMenu(yMenu);

  //---

  if (canDrawColorMapKey())
    addColorMapKeyItems(menu);

  return true;
}

//---

CQChartsGeom::BBox
CQChartsXYPlot::
axesFitBBox() const
{
  if (! isSplitGroups()) {
    return CQChartsPlot::axesFitBBox();
  }
  else {
    BBox bbox;

    for (auto *xaxis : xaxes_)
      bbox += xaxis->fitBBox();

    if (! isSplitSharedY()) {
      for (auto *yaxis : yaxes_)
        bbox += yaxis->fitBBox();
    }
    else {
      if (isYAxisVisible())
        bbox += yAxis()->fitBBox();
    }

    return bbox;
  }
}

//---

bool
CQChartsXYPlot::
hasFgAxes() const
{
  return true;
}

void
CQChartsXYPlot::
drawFgAxes(PaintDevice *device) const
{
  if (! isSplitGroups()) {
    CQChartsPlot::drawFgAxes(device);
  }
  else {
    //auto *th = const_cast<CQChartsXYPlot *>(this);

    auto *masterXAxis = xAxis();

    for (auto *xaxis : xaxes_) {
      xaxis->setAxesLineData         (masterXAxis->axesLineData());
      xaxis->setAxesTickLabelTextData(masterXAxis->axesTickLabelTextData());
      xaxis->setAxesLabelTextData    (masterXAxis->axesLabelTextData());
      xaxis->setAxesMajorGridLineData(masterXAxis->axesMajorGridLineData());
      xaxis->setAxesMinorGridLineData(masterXAxis->axesMinorGridLineData());
      xaxis->setAxesGridFillData     (masterXAxis->axesGridFillData());

      xaxis->draw(this, device);
    }

    if (! isSplitSharedY()) {
      auto *masterYAxis = yAxis();

      int iy = 0;

      auto sm = splitMargin();

      for (auto *yaxis : yaxes_) {
        auto y = (iy == 0 ? iy : iy + sm);

        yaxis->setPosition(CQChartsOptReal(y));

        yaxis->setAxesLineData         (masterYAxis->axesLineData());
        yaxis->setAxesTickLabelTextData(masterYAxis->axesTickLabelTextData());
        yaxis->setAxesLabelTextData    (masterYAxis->axesLabelTextData());
        yaxis->setAxesMajorGridLineData(masterYAxis->axesMajorGridLineData());
        yaxis->setAxesMinorGridLineData(masterYAxis->axesMinorGridLineData());
        yaxis->setAxesGridFillData     (masterYAxis->axesGridFillData());

        yaxis->draw(this, device);

        ++iy;
      }
    }
    else
      drawFgYAxis(device);
  }
}

//------

void
CQChartsXYPlot::
drawBackgroundRect(PaintDevice *device, const DrawRegion &drawRegion, const BBox &rect,
                   const BrushData &brushData, const PenData &penData, const Sides &sides) const
{
  if (! isSplitGroups() || drawRegion != DrawRegion::DATA)
    return CQChartsPlot::drawBackgroundRect(device, drawRegion, rect, brushData, penData, sides);

  //---

  int ng = numVisibleGroups();

//auto xmin = rect.getXMin();
  auto ymin = rect.getYMin();
//auto xmax = rect.getXMax();
  auto ymax = rect.getYMax();

  for (int i = 0; i < ng; ++i) {
    //auto pg = groupRange_.find(i);
    //assert(pg != groupRange_.end());

    //const auto &range = (*pg).second;

    auto sm = splitMargin();

    auto xmin1 = (i == 0      ? i     : i + sm      );
    auto xmax1 = (i == ng - 1 ? i + 1 : i + 1.0 - sm);

    auto rect1 = BBox(xmin1, ymin, xmax1, ymax);

    CQChartsPlot::drawBackgroundRect(device, drawRegion, rect1, brushData, penData, sides);
  }
}

//------

CQChartsGeom::BBox
CQChartsXYPlot::
calcExtraFitBBox() const
{
  CQPerfTrace trace("CQChartsXYPlot::calcExtraFitBBox");

  BBox bbox;

  if (isXRug() || isYRug()) {
    double dx = 0.0, dy = 0.0;

    auto addBBoxX = [&](const BBox &bbox1) {
      if (bbox1.isSet()) {
        dy += bbox1.getHeight();

        bbox += bbox1;
      }
    };

    auto addBBoxY = [&](const BBox &bbox1) {
      if (bbox1.isSet()) {
        dx += bbox1.getWidth();

        bbox += bbox1;
      }
    };

    // x/y rug axis
    if (isXRug()) addBBoxX(xRug_->calcBBox());
    if (isYRug()) addBBoxY(yRug_->calcBBox());
  }

  return bbox;
}

double
CQChartsXYPlot::
xAxisHeight(const CQChartsAxisSide::Type &side) const
{
  double h = CQChartsPlot::xAxisHeight(side);

  if (isXRug()) {
    auto addHeight = [&](const BBox &bbox) {
      if (bbox.isSet())
        h += bbox.getHeight();
    };

    //---

    // x rug axis
    if (isXRug() && xRug_->side().type() == side)
      addHeight(xRug_->calcBBox());
  }

  return h;
}

double
CQChartsXYPlot::
yAxisWidth(const CQChartsAxisSide::Type &side) const
{
  double w = CQChartsPlot::yAxisWidth(side);

  if (isYRug()) {
    auto addWidth = [&](const BBox &bbox) {
      if (bbox.isSet())
        w += bbox.getWidth();
    };

    //---

    // y rug axis
    if (isYRug() && yRug_->side().type() == side)
      addWidth(yRug_->calcBBox());
  }

  return w;
}

//------

void
CQChartsXYPlot::
preDrawObjs(PaintDevice *) const
{
  if (isAdjustText())
    placer_->clear();
}

void
CQChartsXYPlot::
postDrawObjs(PaintDevice *device) const
{
  if (isAdjustText()) {
    auto rect = this->calcDataRect();

    placer_->place(rect);

    placer_->draw(device);
  }
}

void
CQChartsXYPlot::
addDrawText(PaintDevice *device, const QString &str, const Point &point,
            const TextOptions &textOptions, const Point &targetPoint,
            bool centered) const
{
  placer_->addDrawText(device, str, point, textOptions, targetPoint, /*margin*/0, centered);
}

//------

bool
CQChartsXYPlot::
hasBackground() const
{
  if (isXRug()) return true;
  if (isYRug()) return true;

  return false;
}

void
CQChartsXYPlot::
execDrawBackground(PaintDevice *device) const
{
  CQChartsPlot::execDrawBackground(device);

  //---

  // draw axis annotatons in inside->outside order
  xAxisSideHeight_[CQChartsAxisSide::Type::BOTTOM_LEFT] =
    xAxisSideDelta(CQChartsAxisSide::Type::BOTTOM_LEFT);
  xAxisSideHeight_[CQChartsAxisSide::Type::TOP_RIGHT] =
    xAxisSideDelta(CQChartsAxisSide::Type::TOP_RIGHT);

  yAxisSideWidth_[CQChartsAxisSide::Type::BOTTOM_LEFT] =
    yAxisSideDelta(CQChartsAxisSide::Type::BOTTOM_LEFT);
  yAxisSideWidth_[CQChartsAxisSide::Type::TOP_RIGHT] =
    yAxisSideDelta(CQChartsAxisSide::Type::TOP_RIGHT);

  //---

  if (isXRug()) drawXRug(device);
  if (isYRug()) drawYRug(device);
}

//---

void
CQChartsXYPlot::
drawXAxisAt(PaintDevice *device, CQChartsPlot *plot, double pos) const
{
  auto addHeight = [&](const BBox &bbox) {
    if (! bbox.isSet()) return;

    if (xAxis()->side().type() == CQChartsAxisSide::Type::TOP_RIGHT)
      pos += bbox.getHeight();
    else
      pos -= bbox.getHeight();
  };

  if (isXRug() && xRug_->side().type() == xAxis()->side().type())
    addHeight(xRug_->calcBBox());

  CQChartsPlot::drawXAxisAt(device, plot, pos);
}

void
CQChartsXYPlot::
drawYAxisAt(PaintDevice *device, CQChartsPlot *plot, double pos) const
{
  auto addWidth = [&](const BBox &bbox) {
    if (! bbox.isSet()) return;

    if (yAxis()->side().type() == CQChartsAxisSide::Type::TOP_RIGHT)
      pos += bbox.getWidth();
    else
      pos -= bbox.getWidth();
  };

  if (isYRug() && yRug_->side().type() == yAxis()->side().type())
    addWidth(yRug_->calcBBox());

  CQChartsPlot::drawYAxisAt(device, plot, pos);
}

//------

void
CQChartsXYPlot::
drawArrow(PaintDevice *device, const Point &p1, const Point &p2) const
{
  CQChartsWidgetUtil::AutoDisconnect arrowDisconnect(
    arrowObj_.get(), SIGNAL(dataChanged()), const_cast<CQChartsXYPlot *>(this), SLOT(updateSlot()));

  arrowObj_->setFrom(p1);
  arrowObj_->setTo  (p2);

  arrowObj_->draw(device);
}

//------

void
CQChartsXYPlot::
drawXRug(PaintDevice *device) const
{
  double delta = xAxisSideHeight_[xRug_->side().type()];

  drawXYRug(device, xRug_, delta);

  xAxisSideHeight_[xRug_->side().type()] += xRug_->calcBBox().getHeight();
}

void
CQChartsXYPlot::
drawYRug(PaintDevice *device) const
{
  double delta = yAxisSideWidth_[yRug_->side().type()];

  drawXYRug(device, yRug_, delta);

  yAxisSideWidth_[yRug_->side().type()] += yRug_->calcBBox().getWidth();
}

void
CQChartsXYPlot::
drawXYRug(PaintDevice *device, const RugP &rug, double delta) const
{
  rug->clearPoints();

  for (const auto &plotObj : plotObjects()) {
    if (isInterrupt())
      return;

    //---

    auto *pointObj = dynamic_cast<CQChartsXYPointObj *>(plotObj);

    if (pointObj) {
      PenBrush penBrush;

      pointObj->calcPenBrush(penBrush, /*updateState*/false);

      if (rug->direction() == Qt::Horizontal)
        rug->addPoint(CQChartsAxisRug::RugPoint(pointObj->point().x, penBrush.pen.color()));
      else
        rug->addPoint(CQChartsAxisRug::RugPoint(pointObj->point().y, penBrush.pen.color()));

      continue;
    }

    auto *biLineObj = dynamic_cast<CQChartsXYBiLineObj *>(plotObj);

    if (biLineObj) {
      PenBrush penBrush;

      biLineObj->calcPointPenBrush(penBrush, /*updateState*/false);

      if (rug->direction() == Qt::Horizontal)
        rug->addPoint(CQChartsAxisRug::RugPoint(biLineObj->x(), penBrush.pen.color()));
      else {
        rug->addPoint(CQChartsAxisRug::RugPoint(biLineObj->y1(), penBrush.pen.color()));
        rug->addPoint(CQChartsAxisRug::RugPoint(biLineObj->y2(), penBrush.pen.color()));
      }

      continue;
    }
  }

  rug->draw(device, delta);
}

//---

void
CQChartsXYPlot::
drawDataLabel(PaintDevice *device, const BBox &bbox, const QString &str,
              const PenBrush &penBrush, const Font &font) const
{
  //auto *th = const_cast<CQChartsXYPlot *>(this);

  if (isAdjustText())
    dataLabel_->setTextPlacer(placer_);

  dataLabel()->draw(device, bbox, str,
                    static_cast<CQChartsDataLabel::Position>(dataLabelPosition()),
                    penBrush, font);

  if (isAdjustText())
    dataLabel_->setTextPlacer(nullptr);
}

//---

void
CQChartsXYPlot::
write(std::ostream &os, const QString &plotVarName, const QString &modelVarName,
      const QString &viewVarName) const
{
  CQChartsPointPlot::write(os, plotVarName, modelVarName, viewVarName);

  arrowObj_->write(os, plotVarName);
}

//---

CQChartsPlotCustomControls *
CQChartsXYPlot::
createCustomControls()
{
  auto *controls = new CQChartsXYPlotCustomControls(charts());

  controls->init();

  controls->setPlot(this);

  controls->updateWidgets();

  return controls;
}

//------

void
CQChartsXYInvalidator::
invalidate(bool reload)
{
  CQChartsInvalidator::invalidate(reload);

  auto *plot = dynamic_cast<CQChartsXYPlot *>(obj_);
  assert(plot);

  emit plot->customDataChanged();
}

//------

CQChartsXYBiLineObj::
CQChartsXYBiLineObj(const Plot *plot, int groupInd, const BBox &rect,
                    double x, double y1, double y2, const QModelIndex &ind,
                    const ColorInd &is, const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, is, ig, iv), plot_(plot),
 groupInd_(groupInd), x_(x), y1_(y1), y2_(y2)
{
  setModelInd(ind);
}

QString
CQChartsXYBiLineObj::
calcId() const
{
  auto ind1 = plot_->unnormalizeIndex(modelInd());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3").arg(typeName()).arg(is_.i).arg(iv_.i);
}

QString
CQChartsXYBiLineObj::
calcTipId() const
{
  const auto &range = plot()->getGroupRange(groupInd_);

  double x  = plot()->unmapGroupX(range, groupInd_, this->x());
  double y1 = plot()->unmapGroupY(range, this->y1());
  double y2 = plot()->unmapGroupY(range, this->y2());

  auto name  = plot()->valueName(-1, -1, modelInd().row());
  auto xstr  = plot()->xStr(x);
  auto y1str = plot()->yStr(y1);
  auto y2str = plot()->yStr(y2);

  CQChartsTableTip tableTip;

  if (name.length())
    tableTip.addTableRow("Name", name);

  QString xname;

  (void) plot()->xColumnName(xname, "X", /*tip*/true);

  tableTip.addTableRow(xname, xstr );
  tableTip.addTableRow("Y1" , y1str);
  tableTip.addTableRow("Y2" , y2str);

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

bool
CQChartsXYBiLineObj::
isVisible() const
{
  if (! plot()->isLines() && ! plot()->isPoints())
    return false;

  return CQChartsPlotObj::isVisible();
}

bool
CQChartsXYBiLineObj::
inside(const Point &p) const
{
  if (! isVisible())
    return false;

  auto p1 = plot()->windowToPixel(Point(x(), y1()));
  auto p2 = plot()->windowToPixel(Point(x(), y2()));

  double sx, sy;

  plot()->pixelSymbolSize(plot()->symbolSize(), sx, sy);

  BBox pbbox(p1.x - sx, p1.y - sy, p2.x + sx, p2.y + sy);

  auto pp = plot()->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsXYBiLineObj::
getObjSelectIndices(Indices &inds) const
{
  if (! isVisible())
    return;

  addColumnSelectIndex(inds, CQChartsColumn(modelInd().column()));
}

//---

void
CQChartsXYBiLineObj::
draw(PaintDevice *device) const
{
  auto p1 = Point(x(), y1());
  auto p2 = Point(x(), y2());

  if (plot()->isLines())
    drawLines(device, p1, p2);

  if (plot()->isPoints())
    drawPoints(device, p1, p2);
}

void
CQChartsXYBiLineObj::
drawLines(PaintDevice *device, const Point &p1, const Point &p2) const
{
  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  plot()->setBivariateLineDataPen(penBrush.pen, is_);

  if (updateState)
    plot()->updateObjPenBrushState(this, penBrush);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //--

  // draw line
  device->drawLine(p1, p2);
}

void
CQChartsXYBiLineObj::
drawPoints(PaintDevice *device, const Point &p1, const Point &p2) const
{
  // get symbol and size
  auto symbol = plot()->symbol();

  double sx, sy;

  plot()->pixelSymbolSize(plot()->symbolSize(), sx, sy);

  //---

  // calc pen and brush
  PenBrush penBrush;

  calcPointPenBrush(penBrush, /*updateState*/false);

  //---

  // draw symbols
  auto ss = CQChartsLength::pixel(CMathUtil::avg(sx, sy));

  if (symbol.isValid()) {
    // already scaled
    CQChartsDrawUtil::drawSymbol(device, penBrush, symbol, p1, ss, /*scale*/false);
    CQChartsDrawUtil::drawSymbol(device, penBrush, symbol, p2, ss, /*scale*/false);
  }
}

void
CQChartsXYBiLineObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  calcPointPenBrush(penBrush, updateState);
}

void
CQChartsXYBiLineObj::
calcPointPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto ic = (is_.n <= 1 ? ig_.n <= 1 ? iv_ : ig_ : iv_);

  plot_->setSymbolPenBrush(penBrush, ic);

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush, CQChartsPlot::DrawType::SYMBOL);
}

//------

CQChartsXYImpulseLineObj::
CQChartsXYImpulseLineObj(const Plot *plot, int groupInd, const BBox &rect,
                         double x, double y1, double y2, const QModelIndex &ind,
                         const ColorInd &is, const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, is, ig, iv), plot_(plot),
 groupInd_(groupInd), x_(x), y1_(y1), y2_(y2)
{
  setModelInd(ind);
}

QString
CQChartsXYImpulseLineObj::
calcId() const
{
  auto ind1 = plot()->unnormalizeIndex(modelInd());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3").arg(typeName()).arg(is_.i).arg(iv_.i);
}

QString
CQChartsXYImpulseLineObj::
calcTipId() const
{
  const auto &range = plot()->getGroupRange(groupInd_);

  double x  = plot()->unmapGroupX(range, groupInd_, this->x());
  double y1 = plot()->unmapGroupY(range, this->y1());
  double y2 = plot()->unmapGroupY(range, this->y2());

  auto name  = plot()->valueName(is_.i, is_.n, modelInd().row());
  auto xstr  = plot()->xStr(x);
  auto y1str = plot()->yStr(y1);
  auto y2str = plot()->yStr(y2);

  CQChartsTableTip tableTip;

  //---

  // add group column (TODO: check group column)
  if (ig_.n > 1) {
    auto groupColumn = plot_->groupIndColumn();

    if (! tableTip.hasColumn(groupColumn)) {
      auto groupName = plot()->groupIndName(groupInd_);

      tableTip.addTableRow("Group", groupName);

      tableTip.addColumn(groupColumn);
    }
  }

  //---

  if (name.length())
    tableTip.addTableRow("Name", name);

  QString xname;

  (void) plot()->xColumnName(xname, "X", /*tip*/true);

  tableTip.addTableRow(xname, xstr );
  tableTip.addTableRow("Y1" , y1str);
  tableTip.addTableRow("Y2" , y2str);

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

bool
CQChartsXYImpulseLineObj::
isVisible() const
{
  return CQChartsPlotObj::isVisible();
}

bool
CQChartsXYImpulseLineObj::
inside(const Point &p) const
{
  if (! isVisible())
    return false;

  auto p1 = plot()->windowToPixel(Point(x(), y1()));
  auto p2 = plot()->windowToPixel(Point(x(), y2()));

  double b = 2;

  double lw = plot()->calcImpulsePixelWidth(b);

  BBox pbbox(p1.x - lw/2, p1.y - b, p2.x + lw/2, p2.y + b);

  auto pp = plot()->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsXYImpulseLineObj::
getObjSelectIndices(Indices &inds) const
{
  if (! isVisible())
    return;

  addColumnSelectIndex(inds, CQChartsColumn(modelInd().column()));
}

//---

void
CQChartsXYImpulseLineObj::
draw(PaintDevice *device) const
{
  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw impulse
  double lw = plot()->calcImpulsePixelWidth(0);

  bool isThinLine = (lw <= 1.0);

  auto p1 = Point(x(), y1());
  auto p2 = Point(x(), y2());

  if (plot()->isImpulseLines() || isThinLine) {
    if (isThinLine) {
      device->drawLine(p1, p2);
    }
    else {
      auto pp1 = plot()->windowToPixel(p1);
      auto pp2 = plot()->windowToPixel(p2);

      BBox pbbox(pp1.x - lw/2.0, pp1.y, pp1.x + lw/2.0, pp2.y);

      CQChartsDrawUtil::drawRoundedRect(device, plot_->pixelToWindow(pbbox));
    }
  }
  else {
    auto pp1 = plot()->windowToPixel(p1);
    auto pp2 = plot()->windowToPixel(p2);

    BBox pbbox(pp1.x - lw/2.0, pp1.y, pp1.x + lw/2.0, pp2.y);

    CQChartsDrawUtil::drawRoundedRect(device, plot_->pixelToWindow(pbbox));
  }
}

void
CQChartsXYImpulseLineObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto ic = (is_.n <= 1 ? ig_.n <= 1 ? iv_ : ig_ : iv_);

  double lw = plot()->calcImpulsePixelWidth(0);

  bool isThinLine (lw <= 1.0);

  if (plot()->isImpulseLines() || isThinLine) {
    if (isThinLine) {
      plot()->setImpulsePen(penBrush.pen, ic);
    }
    else {
      auto strokeColor = plot()->interpImpulseStrokeColor(ic);

      plot()->setPenBrush(penBrush, PenData(false), plot()->impulseBrushData(strokeColor));
    }
  }
  else {
    auto bc = plot_->interpImpulseStrokeColor(ic);
    auto fc = plot_->interpImpulseFillColor  (ic);

    plot()->setPenBrush(penBrush, plot_->impulsePenData(bc), plot_->impulseBrushData(fc));
  }

  if (updateState)
    plot()->updateObjPenBrushState(this, penBrush);
}

//------

CQChartsXYPointObj::
CQChartsXYPointObj(const Plot *plot, int groupInd, const BBox &rect, const Point &pos,
                   const QModelIndex &ind, const ColorInd &is, const ColorInd &ig,
                   const ColorInd &iv) :
 CQChartsPlotPointObj(const_cast<Plot *>(plot), rect, pos, is, ig, iv), plot_(plot),
 groupInd_(groupInd)
{
  if (ind.isValid())
    setModelInd(ind);
}

//---

void
CQChartsXYPointObj::
setSelected(bool b)
{
  CQChartsPlotObj::setSelected(b);

  if (plot()->isPointLineSelect() && lineObj())
    const_cast<PolylineObj *>(lineObj())->setSelected(b);
}

//---

CQChartsSymbol
CQChartsXYPointObj::
calcSymbol() const
{
  CQChartsSymbol symbol;

  if (extraData(/*create*/false))
    symbol = this->symbol();

  if (! symbol.isValid())
    symbol = plot_->symbol();

  return symbol;
}

CQChartsLength
CQChartsXYPointObj::
calcSymbolSize() const
{
  Length symbolSize;

  if (extraData(/*create*/false))
    symbolSize = this->symbolSize();

  if (! symbolSize.isValid())
    symbolSize = plot()->symbolSize();

  return symbolSize;
}

CQChartsLength
CQChartsXYPointObj::
calcFontSize() const
{
  Length fontSize;

  if (extraData(/*create*/false))
    fontSize = this->fontSize();

  if (! fontSize.isValid())
    fontSize = plot()->dataLabelFontSize();

  return fontSize;
}

CQChartsColor
CQChartsXYPointObj::
calcColor() const
{
  Color color;

  if (extraData(/*create*/false))
    color = this->color();

  return color;
}

CQChartsImage
CQChartsXYPointObj::
calcImage() const
{
  CQChartsImage image;

  if (extraData(/*create*/false))
    image = this->image();

  return image;
}

bool
CQChartsXYPointObj::
isVector() const
{
  return (extraData(/*create*/false) ? bool(extraData()->vector) : false);
}

CQChartsGeom::Point
CQChartsXYPointObj::
vector() const
{
  return (extraData(/*create*/false) ? *extraData()->vector : Point());
}

//---

CQChartsXYPointObj::ExtraData *
CQChartsXYPointObj::
extraData()
{
  if (! edata_)
    edata_ = std::make_unique<ExtraData>();

  return edata_.get();
}

const CQChartsXYPointObj::ExtraData *
CQChartsXYPointObj::
extraData(bool create) const
{
  if (! edata_) {
    if (! create)
      return nullptr;

    const_cast<CQChartsXYPointObj *>(this)->edata_ = std::make_unique<ExtraData>();
  }

  return edata_.get();
}

//---

bool
CQChartsXYPointObj::
isVisible() const
{
  if (! plot()->isPoints())
    return false;

  return CQChartsPlotObj::isVisible();
}

//---

QString
CQChartsXYPointObj::
calcId() const
{
  auto ind1 = plot()->unnormalizeIndex(modelInd());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3:%4").arg(typeName()).arg(is_.i).arg(ig_.i).arg(iv_.i);
}

QString
CQChartsXYPointObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  plot()->addTipHeader(tableTip, modelInd());

  plot()->addNoTipColumns(tableTip);

  //---

  // add label (name column) as header
  auto labelStr    = (labelObj() ? labelObj()->label() : QString());
  auto labelColumn = (labelObj() ? labelObj()->labelColumn() : Column());

  if (labelStr.length() && ! tableTip.hasColumn(labelColumn)) {
    auto name = plot()->columnHeaderName(labelColumn, /*tip*/true);

    tableTip.addTableRow(name, labelStr);

    tableTip.addColumn(labelColumn);
  }

  //---

  // add id column
  if (! tableTip.hasColumn(plot()->idColumn())) {
    auto ind1 = plot()->unnormalizeIndex(modelInd());

    QString idStr;

    if (calcColumnId(ind1, idStr)) {
      tableTip.addTableRow(plot()->idHeaderName(), idStr);

      tableTip.addColumn(plot()->idColumn());
    }
  }

  //---

  // add group column (TODO: check group column)
  if (ig_.n > 1) {
    auto groupColumn = plot_->groupIndColumn();

    if (! tableTip.hasColumn(groupColumn)) {
      auto groupName = plot()->groupIndName(groupInd_);

      tableTip.addTableRow("Group", groupName);

      tableTip.addColumn(groupColumn);
    }
  }

  //---

#if 0
  // add name column (TODO: needed or combine with header)
  if (! labelStr.length() && ! tableTip.hasColumn(plot_->nameColumn())) {
    auto name = plot()->columnHeaderName(plot_->nameColumn(), /*tip*/true);

    auto value = plot()->valueName(is_.i, is_.n, modelInd().row());

    if (value.length())
      tableTip.addTableRow(name, value);

    tableTip.addColumn(plot_->nameColumn());
  }
#endif

  //---

  // add x, y columns
  const auto &range = plot()->getGroupRange(groupInd_);

  if (! tableTip.hasColumn(plot()->xColumn())) {
    double x = plot()->unmapGroupX(range, groupInd_, point().x);

    QString xstr;

    if (plot()->calcMapXColumn()) {
      auto *columnDetails = plot()->columnDetails(plot()->xColumn());

      xstr = (columnDetails ? columnDetails->uniqueValue(int(x)).toString() : plot()->xStr(x));
    }
    else
      xstr = plot()->xStr(x);

    QString xname;

    (void) plot()->xColumnName(xname, "X", /*tip*/true);

    tableTip.addTableRow(xname, xstr);

    tableTip.addColumn(plot()->xColumn());
  }

  if (! tableTip.hasColumn(plot()->yColumns().getColumn(0))) {
    double y = plot()->unmapGroupY(range, point().y);

    auto ystr = plot()->yStr(y);

    QString yname;

    (void) plot()->yColumnName(yname, "Y", /*tip*/true);

    tableTip.addTableRow(yname, ystr);

    tableTip.addColumn(plot()->yColumns().getColumn(0));
  }

  //---

  auto addColumnRowValue = [&](const CQChartsColumn &column) {
    plot_->addTipColumn(tableTip, column, modelInd());
  };

  //---

  // add symbol type, symbol size and font size columns
  addColumnRowValue(plot_->symbolTypeColumn());
  addColumnRowValue(plot_->symbolSizeColumn());
  addColumnRowValue(plot_->fontSizeColumn  ());

  //---

  // add color column
  addColumnRowValue(plot_->colorColumn());

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

void
CQChartsXYPointObj::
getObjSelectIndices(Indices &inds) const
{
  if (! isVisible())
    return;

  addColumnSelectIndex(inds, plot()->xColumn());
  addColumnSelectIndex(inds, plot()->yColumns().getColumn(is_.i));

  addColumnSelectIndex(inds, plot_->symbolTypeColumn());
  addColumnSelectIndex(inds, plot_->symbolSizeColumn());
  addColumnSelectIndex(inds, plot_->fontSizeColumn  ());
  addColumnSelectIndex(inds, plot_->colorColumn     ());
}

//---

void
CQChartsXYPointObj::
draw(PaintDevice *device) const
{
  bool isVector = this->isVector();

  if (! isVisible() && ! isVector)
    return;

  //---

  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  if (isVisible()) {
    device->setColorNames();

    // override symbol for custom symbol
    auto symbol = this->calcSymbol();

    double sx, sy;

    calcSymbolPixelSize(sx, sy, /*square*/true, /*enforceMinSize*/false);

    //---

    // draw symbol or image
    auto image = this->calcImage();

    if (! image.isValid()) {
      if (symbol.isValid()) {
        if (plot_->isSplitGroups()) {
          CQChartsDrawUtil::setPenBrush(device, penBrush);

          CQChartsDrawUtil::drawSymbol(device, symbol, rect(), /*scale*/false);
        }
        else
          plot()->drawSymbol(device, point(), symbol, sx, sy, penBrush, /*scaled*/false);
      }
    }
    else {
      double aspect = (1.0*image.width())/image.height();

      if (aspect > 1.0) {
        sy = sx;
        sx = sy*aspect;
      }
      else {
        sy = sx*(1.0/aspect);
      }

      auto ps = plot()->windowToPixel(point());

      BBox ibbox(ps.x - sx, ps.y - sy, ps.x + 2*sx, ps.y + 2*sy);

      device->drawImageInRect(plot()->pixelToWindow(ibbox), image);
    }

    device->resetColorNames();
  }

  //---

  // draw optional vector
  // TODO: custom color for this sets what ?
  if (isVector) {
    auto p1 = point();
    auto p2 = p1 + this->vector();

    plot()->drawArrow(device, p1, p2);
  }
}

void
CQChartsXYPointObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  ColorInd ic;

  if (plot_->colorType() == CQChartsPlot::ColorType::AUTO) {
    // default for xy is set or group color (not value color !!)
    if      (is_.n > 1)
      ic = is_;
    else if (ig_.n > 1)
      ic = ig_;
  }
  else
    ic = calcColorInd();

  //---

  // calc pen and brush
  plot()->setSymbolPenBrush(penBrush, ic);

  // override symbol fill color for custom color
  auto color = this->calcColor();

  if (color.isValid()) {
    auto c = plot()->interpColor(color, ic);

    penBrush.brush.setColor(CQChartsDrawUtil::setColorAlpha(c, plot_->symbolFillAlpha()));
  }

  if (updateState)
    plot()->updateObjPenBrushState(this, penBrush, drawType());
}

//------

CQChartsXYLabelObj::
CQChartsXYLabelObj(const Plot *plot, int groupInd, const BBox &rect,
                   double x, double y, const QString &label, const QModelIndex &ind,
                   const ColorInd &is, const ColorInd &iv) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, is, ColorInd(), iv), plot_(plot),
 groupInd_(groupInd), pos_(x, y), label_(label)
{
  setModelInd(ind);
}

CQChartsXYLabelObj::
~CQChartsXYLabelObj()
{
}

QString
CQChartsXYLabelObj::
calcId() const
{
  auto ind1 = plot()->unnormalizeIndex(modelInd());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3").arg(typeName()).arg(is_.i).arg(iv_.i);
}

QString
CQChartsXYLabelObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow(plot_->columnHeaderName(plot_->labelColumn()), label_);

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

bool
CQChartsXYLabelObj::
isVisible() const
{
  if (! plot()->isPointLabels())
    return false;

  return CQChartsPlotObj::isVisible();
}

bool
CQChartsXYLabelObj::
inside(const Point &p) const
{
  if (! isVisible())
    return false;

  auto ppos = plot()->windowToPixel(Point(pos_));

  // TODO: better text bounding box
  double sx = 16;
  double sy = 16;

  BBox pbbox(ppos.x - sx, ppos.y - sy, ppos.x + sx, ppos.y + sy);

  auto pp = plot()->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsXYLabelObj::
getObjSelectIndices(Indices &inds) const
{
  if (! isVisible())
    return;

  addColumnSelectIndex(inds, plot()->xColumn());
  addColumnSelectIndex(inds, plot()->yColumns().getColumn(is_.i));
}

//---

void
CQChartsXYLabelObj::
draw(PaintDevice *device) const
{
  // text font color
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  // get font size
  auto fontSize = pointObj()->calcFontSize();

  //---

  // get font
  auto font  = plot_->dataLabelFont();
  auto font1 = font;

  if (fontSize.isValid()) {
    double fontPixelSize = plot_->lengthPixelHeight(fontSize);

    // scale to font size
    fontPixelSize = plot_->limitFontSize(fontPixelSize);

    font1.setPointSizeF(fontPixelSize);
  }

  //---

  // draw text
  auto ps = plot()->windowToPixel(pos_);

  // TODO: better symbol bounding box
  double sx = 16.0, sy = 16.0;

  BBox ptbbox(ps.x - sx, ps.y - sy, ps.x + sx, ps.y + sy);

  plot_->drawDataLabel(device, plot_->pixelToWindow(ptbbox), label_, penBrush, font1);
}

void
CQChartsXYLabelObj::
calcPenBrush(PenBrush &penBrush, bool /*updateState*/) const
{
  auto *dataLabel = plot_->dataLabel();

  auto tc = dataLabel->interpTextColor(ColorInd());

  plot()->setPenBrush(penBrush, PenData(true, tc, dataLabel->textAlpha()), BrushData(false));
}

//------

CQChartsXYPolylineObj::
CQChartsXYPolylineObj(const Plot *plot, int groupInd, const BBox &rect,
                      const Polygon &poly, const QString &name, const ColorInd &is,
                      const ColorInd &ig) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, is, ig, ColorInd()), plot_(plot),
 groupInd_(groupInd), poly_(poly), name_(name)
{
  setDetailHint(DetailHint::MAJOR);
}

CQChartsXYPolylineObj::
~CQChartsXYPolylineObj()
{
}

QString
CQChartsXYPolylineObj::
calcId() const
{
  return QString("%1:%2:%3").arg(typeName()).arg(ig_.i).arg(is_.i);
}

QString
CQChartsXYPolylineObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Name", name());

  //---

  //plot()->addTipColumns(tableTip, ind);

  //---

  return tableTip.str();
}

#if 0
bool
CQChartsXYPolylineObj::
isVisible() const
{
  if (! plot()->isLines())
    return false;

  return CQChartsPlotObj::isVisible();
}
#endif

bool
CQChartsXYPolylineObj::
inside(const Point &p) const
{
  if (! plot()->isLines())
    return false;

  if (! plot()->isLinesSelectable())
    return false;

  auto pp = plot()->windowToPixel(p);

  for (int i = 1; i < poly_.size(); ++i) {
    auto p1 = poly_.point(i - 1);
    auto p2 = poly_.point(i    );

    auto pl1 = plot()->windowToPixel(p1);
    auto pl2 = plot()->windowToPixel(p2);

    double d;

    if (CQChartsUtil::PointLineDistance(pp, pl1, pl2, &d) && d < 1)
      return true;
  }

  return false;
}

#if 0
bool
CQChartsXYPolylineObj::
rectIntersect(const BBox &r, bool inside) const
{
  return CQChartsPlotObj::rectIntersect(r, inside);
}
#endif

bool
CQChartsXYPolylineObj::
isSelectable() const
{
  return plot()->isLinesSelectable();
}

bool
CQChartsXYPolylineObj::
interpY(double x, std::vector<double> &yvals) const
{
  if (! plot()->isLines())
    return false;

  if (plot()->isRoundedLines()) {
    initSmooth();

    double y = smooth_->interp(x);

    yvals.push_back(y);
  }
  else {
    for (int i = 1; i < poly_.size(); ++i) {
      double x1 = poly_.point(i - 1).x;
      double y1 = poly_.point(i - 1).y;
      double x2 = poly_.point(i    ).x;
      double y2 = poly_.point(i    ).y;

      if (x >= x1 && x <= x2) {
        double y = (y2 - y1)*(x - x1)/(x2 - x1) + y1;

        yvals.push_back(y);
      }
    }
  }

  return ! yvals.empty();
}

#if 0
bool
CQChartsXYPolylineObj::
isOutlier(double y) const
{
  const_cast<CQChartsXYPolylineObj *>(this)->initStats();

  return statData_.isOutlier(y);
}
#endif

void
CQChartsXYPolylineObj::
getObjSelectIndices(Indices &) const
{
  if (! plot()->isLines())
    return;

  // all objects part of line (don't support select)
}

void
CQChartsXYPolylineObj::
initSmooth() const
{
  // init smooth if needed
  if (! smooth_) {
    auto *th = const_cast<CQChartsXYPolylineObj *>(this);

    th->smooth_ = std::make_unique<CQChartsSmooth>(poly_, /*sorted*/false);
  }
}

void
CQChartsXYPolylineObj::
resetBestFit()
{
  bestFit_.resetFitted();
}

void
CQChartsXYPolylineObj::
initBestFit()
{
  if (! bestFit_.isFitted()) {
    if (! plot()->isBestFitOutliers()) {
      initStats();

      //---

      Polygon poly;

      for (const auto &p : poly_.qpoly()) {
        if (! statData_.isOutlier(p.y()))
          poly.addPoint(p);
      }

      //---

      bestFit_.calc(poly, plot()->bestFitOrder());
    }
    else
      bestFit_.calc(poly_, plot()->bestFitOrder());
  }
}

void
CQChartsXYPolylineObj::
initStats()
{
  if (! statData_.set) {
    std::vector<double> y;

    for (int i = 0; i < poly_.size(); ++i)
      y.push_back(poly_.point(i).y);

    std::sort(y.begin(), y.end());

    statData_.calcStatValues(y);
  }
}

//---

void
CQChartsXYPolylineObj::
draw(PaintDevice *device) const
{
  if (plot()->isHull())
    drawHull(device);

  //---

  // draw lines
  if (plot()->isLines())
    drawLines(device);

  //---

  if (plot()->isBestFit())
    drawBestFit(device);

  //---

  if (plot()->isStatsLines())
    drawStatsLines(device);

  //---

  if (plot()->isMovingAverage())
    drawMovingAverage(device);

  //---

  if (plot()->isLineLabel())
    drawLineLabel(device);
}

//---

void
CQChartsXYPolylineObj::
drawHull(PaintDevice *device) const
{
  auto *th = const_cast<CQChartsXYPolylineObj *>(this);

  if (! th->hull_)
    th->hull_ = std::make_unique<Hull>();

  auto *hull = th->hull_.get();

  //---

  hull->clear();

  for (int i = 0; i < poly_.size(); ++i)
    hull->addPoint(poly_.point(i));

  //---

  // calc pen/brush
  auto ic = (ig_.n > 1 ? ig_ : is_);

  PenBrush penBrush;

  plot()->setHullPenBrush(penBrush, ic);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  hull->draw(device);
}

//---

void
CQChartsXYPolylineObj::
drawLines(PaintDevice *device) const
{
  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  if (plot()->isRoundedLines()) {
    initSmooth();

    // draw path
    auto path = smooth_->createPath(/*closed*/false);

    device->setColorNames();

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->drawPath(path);

    device->resetColorNames();
  }
  else {
    // draw path
    device->setColorNames();

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    int np = poly_.size();

    for (int i = 1; i < np; ++i)
      device->drawLine(poly_.point(i - 1), poly_.point(i));

    device->resetColorNames();
  }
}

//---

void
CQChartsXYPolylineObj::
drawBestFit(PaintDevice *device) const
{
  const_cast<CQChartsXYPolylineObj *>(this)->initBestFit();

  //---

  PenBrush penBrush;

  bool updateState = device->isInteractive();

  auto ic = (ig_.n > 1 ? ig_ : is_);

  plot_->setBestFitPenBrush(penBrush, ic);

  if (updateState)
    plot_->updateObjPenBrushState(this, ic, penBrush, CQChartsPlot::DrawType::LINE);

  plot()->drawBestFit(device, &bestFit_, penBrush);
}

//---

void
CQChartsXYPolylineObj::
drawStatsLines(PaintDevice *device) const
{
  const_cast<CQChartsXYPolylineObj *>(this)->initStats();

  //---

  bool updateState = device->isInteractive();

  // calc pen and brush
  auto ic = (ig_.n > 1 ? ig_ : is_);

  PenBrush penBrush;

  plot()->setStatsPenBrush(penBrush, ic);

  if (updateState)
    plot()->updateObjPenBrushState(this, ic, penBrush, drawType());

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw lines
  auto dataXMin = plot()->drawRangeXMin(groupInd_, /*adjust*/true);
  auto dataXMax = plot()->drawRangeXMax(groupInd_, /*adjust*/true);

  auto drawStatLine = [&](double y) {
    Point p1(dataXMin, y);
    Point p2(dataXMax, y);

    device->drawLine(p1, p2);
  };

  auto fillRect = [&](const BBox &bbox, double f=1.0) {
    double alpha = CQChartsDrawUtil::brushAlpha(penBrush.brush);
    CQChartsDrawUtil::setBrushAlpha(penBrush.brush, alpha*f);
    device->fillRect(bbox);
    CQChartsDrawUtil::setBrushAlpha(penBrush.brush, alpha);
  };

  //--

  fillRect(BBox(dataXMin, statData_.loutlier   , dataXMax, statData_.uoutlier   ), 0.3);
  fillRect(BBox(dataXMin, statData_.lowerMedian, dataXMax, statData_.upperMedian), 1.0);

  //--

  drawStatLine(statData_.loutlier   );
  drawStatLine(statData_.lowerMedian);
  drawStatLine(statData_.median     );
  drawStatLine(statData_.upperMedian);
  drawStatLine(statData_.uoutlier   );
}

//---

void
CQChartsXYPolylineObj::
drawMovingAverage(PaintDevice *device) const
{
  int na = plot_->numMovingAverage();
  if (na < 2) return;

  int np = poly_.size();

  Polygon poly;

  for (int i = na; i < np; ++i) {
    double xsum = 0.0, ysum = 0.0;

    for (int j = 0; j < na; ++j) {
      auto p = poly_.point(i - na + j);

      xsum += p.x;
      ysum += p.y;
    }

    auto xa = xsum/na;
    auto ya = ysum/na;

    poly.addPoint(Point(xa, ya));
  }

  //---

  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  auto ic = (ig_.n > 1 ? ig_ : is_);

  plot()->setMovingAverageLineDataPen(penBrush.pen, ic);

  if (updateState)
    plot()->updateObjPenBrushState(this, penBrush);

  //---

  // draw lines
  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawPolyline(poly);
}

//---

void
CQChartsXYPolylineObj::
drawLineLabel(PaintDevice *device) const
{
  int np = poly_.size();
  if (np < 1) return;

  //---

  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  auto ds = plot_->pixelToWindowSize(Size(4, 4));

  auto p = poly_.point(np - 1) + Point(ds.width(), ds.height());

  CQChartsTextOptions textOptions;

  textOptions.align = Qt::AlignLeft;

  if (plot_->isAdjustText())
    plot_->addDrawText(device, name(), p, textOptions, p, /*centered*/false);
  else
    CQChartsDrawUtil::drawTextAtPoint(device, p, name(), textOptions, /*centered*/false);
}

//---

void
CQChartsXYPolylineObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  auto ic = (ig_.n > 1 ? ig_ : is_);

  plot()->setLineDataPen(penBrush.pen, ic);

  if (updateState)
    plot()->updateObjPenBrushState(this, ic, penBrush, drawType());
}

CQChartsGeom::BBox
CQChartsXYPolylineObj::
fitBBox() const
{
  if (! plot()->isLineLabel())
    return BBox();

  int np = poly_.size();
  if (np < 1) return BBox();

  //---

  auto p = plot_->windowToPixel(poly_.point(np - 1)) + Point(4, 4);

  auto font = plot_->view()->plotFont(plot_, plot_->font());

  CQChartsTextOptions options;

  auto psize = CQChartsDrawUtil::calcTextSize(name(), font, options);

  auto pbbox = BBox(p.x                , p.y - psize.height()/2.0,
                    p.x + psize.width(), p.y + psize.height()/2.0);

  return plot_->pixelToWindow(pbbox);
}

//------

CQChartsXYPolygonObj::
CQChartsXYPolygonObj(const Plot *plot, int groupInd, const BBox &rect,
                     const Polygon &poly, const QString &name, const ColorInd &is,
                     const ColorInd &ig, bool under) :
 CQChartsPlotObj(const_cast<Plot *>(plot), rect, is, ig, ColorInd()), plot_(plot),
 groupInd_(groupInd), poly_(poly), name_(name), under_(under)
{
  setDetailHint(DetailHint::MAJOR);
}

CQChartsXYPolygonObj::
~CQChartsXYPolygonObj()
{
}

QString
CQChartsXYPolygonObj::
calcId() const
{
  return QString("%1:%2:%3").arg(typeName()).arg(ig_.i).arg(is_.i);
}

QString
CQChartsXYPolygonObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  tableTip.addTableRow("Name", name());
  tableTip.addTableRow("Area", poly_.area());

  //---

  //plot()->addTipColumns(tableTip, ind);

  //---

  return tableTip.str();
}

bool
CQChartsXYPolygonObj::
isVisible() const
{
  if (! plot()->isFillUnderFilled())
    return false;

  return CQChartsPlotObj::isVisible();
}

bool
CQChartsXYPolygonObj::
inside(const Point &p) const
{
  if (! isVisible())
    return false;

  if (! plot()->isFillUnderSelectable())
    return false;

  return poly_.containsPoint(p, Qt::OddEvenFill);
}

#if 0
bool
CQChartsXYPolygonObj::
rectIntersect(const BBox &r, bool inside) const
{
  return CQChartsPlotObj::rectIntersect(r, inside);
}
#endif

bool
CQChartsXYPolygonObj::
isSelectable() const
{
  return plot()->isFillUnderSelectable();
}

void
CQChartsXYPolygonObj::
getObjSelectIndices(Indices &) const
{
  if (! isVisible())
    return;

  // all objects part of polygon (don't support select)
}

void
CQChartsXYPolygonObj::
initSmooth() const
{
  // init smooth if needed
  // (not first point and last point are the extra points to make the square protrusion
  if (! smooth_) {
    auto *th = const_cast<CQChartsXYPolygonObj *>(this);

    th->smooth_ = std::make_unique<CQChartsSmooth>(poly_, /*sorted*/false);

    th->smooth_->setUnder(under_);
  }
}

//---

void
CQChartsXYPolygonObj::
draw(PaintDevice *device) const
{
  // calc pen and brush
  PenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  // create polygon path (rounded or normal)
  if (plot()->isRoundedLines()) {
    initSmooth();

    // draw polygon
    auto path = smooth_->createPath(/*closed*/true);

    device->setColorNames();

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->drawPath(path);

    device->resetColorNames();
  }
  else {
    // draw polygon
    device->setColorNames();

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->drawPolygon(poly_);

    device->resetColorNames();
  }
}

void
CQChartsXYPolygonObj::
calcPenBrush(PenBrush &penBrush, bool updateState) const
{
  // calc pen and brush
  auto ic = (ig_.n > 1 ? ig_ : is_);

  auto fillColor = plot()->interpFillUnderFillColor(ic);

  plot()->setPenBrush(penBrush,
    PenData  (false),
    BrushData(true, fillColor, plot()->fillUnderFillAlpha(), plot()->fillUnderFillPattern()));

  if (updateState)
    plot()->updateObjPenBrushState(this, penBrush);
}

//------

CQChartsXYColorKeyItem::
CQChartsXYColorKeyItem(Plot *plot, const ColorInd &is, const ColorInd &ig) :
 CQChartsColorBoxKeyItem(plot, is, ig, ColorInd()), plot_(plot)
{
  setClickable(true);
}

void
CQChartsXYColorKeyItem::
doSelect(SelMod selMod)
{
  auto *obj = plotObj();
  if (! obj) return;

  CQChartsPlot::PlotObjs objs;

  if      (selMod == SelMod::REPLACE) {
    objs.push_back(obj);
  }
  else if (selMod == SelMod::ADD) {
    for (int ig = 0; ig < ig_.n; ++ig) {
      auto *obj1 = plot()->getGroupObj(ig);
      if (! obj1) continue;

      if (obj1->isSelected() || obj == obj1)
        objs.push_back(obj1);
    }
  }
  else if (selMod == SelMod::REMOVE) {
    for (int ig = 0; ig < ig_.n; ++ig) {
      auto *obj1 = plot()->getGroupObj(ig);
      if (! obj1) continue;

      if (obj1->isSelected() && obj != obj1)
        objs.push_back(obj1);
    }
  }
  else if (selMod == SelMod::TOGGLE) {
    bool selected = obj->isSelected();

    for (int ig = 0; ig < ig_.n; ++ig) {
      auto *obj1 = plot()->getGroupObj(ig);
      if (! obj1) continue;

      if (obj == obj1) {
        if (! selected)
          objs.push_back(obj1);
      }
      else {
        if (obj1->isSelected())
          objs.push_back(obj1);
      }
    }
  }

  //---

  CQChartsPlot::PlotObjs objs1;

  for (auto *obj1 : objs) {
    auto *polyObj = dynamic_cast<CQChartsXYPolylineObj *>(obj1);

    if (polyObj) {
      for (auto *obj2 : polyObj->pointObjs())
        objs1.push_back(obj2);
    }

    objs1.push_back(obj1);
  }

  //---

  plot()->selectObjs(objs1, /*export*/true);

  key_->redraw(/*wait*/ true);
}

void
CQChartsXYColorKeyItem::
draw(PaintDevice *device, const BBox &rect) const
{
  if (plot()->isKeyLine())
    drawLine(device, rect);
  else
    CQChartsColorBoxKeyItem::draw(device, rect);
}

void
CQChartsXYColorKeyItem::
drawLine(PaintDevice *device, const BBox &rect) const
{
  bool updateState = device->isInteractive();

  device->save();

  auto *keyPlot = qobject_cast<CQChartsPlot *>(key_->plot());

  //auto *xyKeyPlot = qobject_cast<CQChartsXYPlot *>(keyPlot);
  //if (! xyKeyPlot) xyKeyPlot = plot();

  auto prect = keyPlot->windowToPixel(rect);

  bool swapped;
  auto pbbox1 = prect.adjusted(2, 2, -2, -2, swapped);
  if (swapped) return;

  device->setClipRect(keyPlot->pixelToWindow(pbbox1), Qt::IntersectClip);

  QColor hideBg;
  Alpha  hideAlpha;

  if (plot()->isSetHidden(ic_.i)) {
    hideBg    = key_->interpBgColor();
    hideAlpha = key_->hiddenAlpha();
  }

  if (plot()->isFillUnderFilled()) {
    PenBrush fillPenBrush;

    auto fillColor = plot()->interpFillUnderFillColor(ic_);

    plot()->setPenBrush(fillPenBrush,
      PenData  (false),
      BrushData(true, fillColor, plot()->fillUnderFillAlpha(), plot()->fillUnderFillPattern()));

    double x1 = prect.getXMin() + 4;
    double x2 = prect.getXMax() - 4;
    double y1 = prect.getYMin() + 4;
    double y2 = prect.getYMax() - 4;

    if (isInside())
      fillPenBrush.brush.setColor(plot()->insideColor(fillPenBrush.brush.color()));

    CQChartsDrawUtil::setPenBrush(device, fillPenBrush);

    BBox pbbox1(x1, y1, x2, y2);

    device->fillRect(pbbox1);
  }

  if (plot()->isLines() || plot()->isBestFit() || plot()->calcImpulseVisible()) {
    double x1 = prect.getXMin() + 4;
    double x2 = prect.getXMax() - 4;
    double y  = prect.getYMid();

    PenBrush linePenBrush;

    if      (plot()->isLines()) {
      plot()->setLineDataPen(linePenBrush.pen, ic_);

      if (plot()->isSetHidden(ic_.i))
        linePenBrush.pen.setColor(
          CQChartsUtil::blendColors(linePenBrush.pen.color(), hideBg, hideAlpha));
    }
    else if (plot()->isBestFit()) {
      auto fitColor = plot()->interpBestFitStrokeColor(ic_);

      if (plot()->isSetHidden(ic_.i))
        fitColor = CQChartsUtil::blendColors(fitColor, hideBg, hideAlpha);

      plot()->setPenBrush(linePenBrush,
        PenData  (true, fitColor, plot()->bestFitStrokeAlpha(),
                  plot()->bestFitStrokeWidth(), plot()->bestFitStrokeDash()),
        BrushData(false));
    }
    else if (plot()->calcImpulseVisible()) {
      plot()->setImpulsePen(linePenBrush.pen, ic_);

      if (plot()->isSetHidden(ic_.i))
        linePenBrush.pen.setColor(
          CQChartsUtil::blendColors(linePenBrush.pen.color(), hideBg, hideAlpha));
    }

    linePenBrush.brush = QBrush(Qt::NoBrush);

    auto *obj = plotObj();

    if (obj) {
      if (updateState)
        plot()->updateObjPenBrushState(obj, ig_, linePenBrush, CQChartsPlot::DrawType::LINE);
    }

    if (isInside())
      linePenBrush.pen = plot()->insideColor(linePenBrush.pen.color());

    device->setPen(linePenBrush.pen);

    device->drawLine(plot()->pixelToWindow(Point(x1, y)), plot()->pixelToWindow(Point(x2, y)));
  }

  if (plot()->isPoints()) {
    double dx = keyPlot->pixelToWindowWidth(4);

    double x1 = rect.getXMin() + dx;
    double x2 = rect.getXMax() - dx;
    double y  = rect.getYMid();

    auto p1 = keyPlot->windowToPixel(Point(x1, y));
    auto p2 = keyPlot->windowToPixel(Point(x2, y));

    //---

    auto pointStrokeColor = plot()->interpSymbolStrokeColor(ic_);
    auto pointFillColor   = plot()->interpSymbolFillColor  (ic_);

    if (plot()->isSetHidden(ic_.i)) {
      pointStrokeColor = CQChartsUtil::blendColors(pointStrokeColor, hideBg, hideAlpha);
      pointFillColor   = CQChartsUtil::blendColors(pointFillColor  , hideBg, hideAlpha);
    }

    //---

    PenBrush penBrush;

    plot()->setPenBrush(penBrush,
      plot_->symbolPenData  (pointStrokeColor),
      plot_->symbolBrushData(pointFillColor));

    auto *obj = plotObj();

    if (obj) {
      if (updateState)
        plot()->updateObjPenBrushState(obj, ig_, penBrush, CQChartsPlot::DrawType::SYMBOL);
    }

    //---

    auto symbol = plot()->symbol();

    if (symbol.isValid()) {
      auto symbolSize = plot()->symbolSize();

      Point ps(CMathUtil::avg(p1.x, p2.x), CMathUtil::avg(p1.y, p2.y));

      auto ps1 = plot()->pixelToWindow(ps);

      CQChartsDrawUtil::drawSymbol(device, penBrush, symbol, ps1, symbolSize);
    }
  }

  device->restore();
}

QBrush
CQChartsXYColorKeyItem::
fillBrush() const
{
  PenBrush penBrush;

  QColor c;
  Alpha  alpha;
  auto   pattern = CQChartsFillPattern::makeSolid();

  if      (plot()->canBivariateLines()) {
    c = plot()->interpBivariateLinesColor(is_);

    alpha = plot()->bivariateLinesAlpha();
  }
  else if (plot()->isOverlay()) {
    if (plot()->prevPlot() || plot()->nextPlot()) {
      c = plot()->interpLinesColor(is_);

      alpha = plot()->linesAlpha();
    }
    else
      c = CQChartsColorBoxKeyItem::fillBrush().color();
  }
  else
    c = CQChartsColorBoxKeyItem::fillBrush().color();

  adjustFillColor(c);

  plot()->setBrush(penBrush, BrushData(true, c, alpha, pattern));

  return penBrush.brush;
}

bool
CQChartsXYColorKeyItem::
calcHidden() const
{
  bool hidden = false;

  if      (is_.n > 1 && plot_->isSetHidden(is_.i))
    hidden = true;
  else if (ig_.n > 1 && plot_->isSetHidden(ig_.i))
    hidden = true;

  return hidden;
}

CQChartsPlotObj *
CQChartsXYColorKeyItem::
plotObj() const
{
  if (ig_.n <= 1)
    return nullptr;

  auto *obj = plot()->getGroupObj(ig_.i);

  return obj;
}

//------

CQChartsXYTextKeyItem::
CQChartsXYTextKeyItem(Plot *plot, const QString &text, const ColorInd &is, const ColorInd &ig) :
 CQChartsTextKeyItem(plot, text, is.n > 1 ? is : ig), plot_(plot)
{
}

QColor
CQChartsXYTextKeyItem::
interpTextColor(const ColorInd &ind) const
{
  auto c = CQChartsTextKeyItem::interpTextColor(ind);

  //adjustFillColor(c);

  return c;
}

bool
CQChartsXYTextKeyItem::
calcHidden() const
{
  return plot()->isSetHidden(ic_.i);
}

//------

CQChartsXYPlotCustomControls::
CQChartsXYPlotCustomControls(CQCharts *charts) :
 CQChartsPointPlotCustomControls(charts, "xy")
{
}

void
CQChartsXYPlotCustomControls::
init()
{
  addWidgets();

  addLayoutStretch();

  connectSlots(true);
}

void
CQChartsXYPlotCustomControls::
addWidgets()
{
  addColumnWidgets();

  addGroupColumnWidgets();

  addOptionsWidgets();

  addKeyList();
}

void
CQChartsXYPlotCustomControls::
addColumnWidgets()
{
  // columns group
  auto columnsFrame = createGroupFrame("Columns", "columnsFrame");

  addNamedColumnWidgets(QStringList() << "x" << "y" << "label", columnsFrame);
}

void
CQChartsXYPlotCustomControls::
addOptionsWidgets()
{
  // options group
  optionsFrame_ = createGroupFrame("Options", "optionsFrame", "groupBox", /*stretch*/false);

  //---

  pointsCheck_ = createBoolEdit("points", /*choice*/false);
  linesCheck_  = createBoolEdit("lines" , /*choice*/false);

  addFrameColWidget(optionsFrame_, pointsCheck_);
  addFrameColWidget(optionsFrame_, linesCheck_ );
  addFrameSpacer   (optionsFrame_);

  fillUnderCheck_ = createBoolEdit("fillUnder", /*choice*/false);
  stackedCheck_   = createBoolEdit("stacked"  , /*choice*/false);
  impulseCheck_   = createBoolEdit("impulse"  , /*choice*/false);

  addFrameColWidget(optionsFrame_, fillUnderCheck_);
  addFrameColWidget(optionsFrame_, stackedCheck_  );
  addFrameColWidget(optionsFrame_, impulseCheck_  );
  addFrameSpacer   (optionsFrame_);

  bestFitCheck_       = createBoolEdit("bestFit"      , /*choice*/false);
  hullCheck_          = createBoolEdit("convexHull"   , /*choice*/false);
  statsCheck_         = createBoolEdit("statsLines"   , /*choice*/false);
  movingAverageCheck_ = createBoolEdit("movingAverage", /*choice*/false);

  addFrameColWidget(optionsFrame_, bestFitCheck_      );
  addFrameColWidget(optionsFrame_, hullCheck_         );
  addFrameColWidget(optionsFrame_, statsCheck_        );
  addFrameColWidget(optionsFrame_, movingAverageCheck_);
  addFrameSpacer   (optionsFrame_);
}

void
CQChartsXYPlotCustomControls::
connectSlots(bool b)
{
  CQChartsWidgetUtil::optConnectDisconnect(b,
    pointsCheck_, SIGNAL(stateChanged(int)), this, SLOT(pointsSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    linesCheck_ , SIGNAL(stateChanged(int)), this, SLOT(linesSlot(int)));

  CQChartsWidgetUtil::optConnectDisconnect(b,
    fillUnderCheck_, SIGNAL(stateChanged(int)), this, SLOT(fillUnderSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    stackedCheck_  , SIGNAL(stateChanged(int)), this, SLOT(stackedSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    impulseCheck_  , SIGNAL(stateChanged(int)), this, SLOT(impulseSlot(int)));

  CQChartsWidgetUtil::optConnectDisconnect(b,
    bestFitCheck_      , SIGNAL(stateChanged(int)), this, SLOT(bestFitSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    hullCheck_         , SIGNAL(stateChanged(int)), this, SLOT(convexHullSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    statsCheck_        , SIGNAL(stateChanged(int)), this, SLOT(statsLinesSlot(int)));
  CQChartsWidgetUtil::optConnectDisconnect(b,
    movingAverageCheck_, SIGNAL(stateChanged(int)), this, SLOT(movingAverageSlot(int)));

  CQChartsPointPlotCustomControls::connectSlots(b);
}

void
CQChartsXYPlotCustomControls::
setPlot(CQChartsPlot *plot)
{
  CQChartsWidgetUtil::optDisconnect(plot_, SIGNAL(customDataChanged()),
                                    this, SLOT(updateWidgets()));

  plot_ = dynamic_cast<CQChartsXYPlot *>(plot);

  CQChartsPointPlotCustomControls::setPlot(plot);

  CQChartsWidgetUtil::optConnect(plot_, SIGNAL(customDataChanged()),
                                 this, SLOT(updateWidgets()));
}

void
CQChartsXYPlotCustomControls::
updateWidgets()
{
  connectSlots(false);

  //---

  if (pointsCheck_) pointsCheck_->setChecked(plot_->isPoints());
  if (linesCheck_ ) linesCheck_ ->setChecked(plot_->isLines());

  if (fillUnderCheck_) fillUnderCheck_->setChecked(plot_->isFillUnderFilled());
  if (stackedCheck_  ) stackedCheck_  ->setChecked(plot_->isStacked());
  if (impulseCheck_  ) impulseCheck_  ->setChecked(plot_->isImpulseVisible());

  if (bestFitCheck_      ) bestFitCheck_      ->setChecked(plot_->isBestFit());
  if (hullCheck_         ) hullCheck_         ->setChecked(plot_->isHull());
  if (statsCheck_        ) statsCheck_        ->setChecked(plot_->isStatsLines());
  if (movingAverageCheck_) movingAverageCheck_->setChecked(plot_->isMovingAverage());

  if (stackedCheck_) stackedCheck_->setEnabled(plot_->yColumns().count() > 1);

  //---

  CQChartsPointPlotCustomControls::updateWidgets();

  //---

  connectSlots(true);
}

void
CQChartsXYPlotCustomControls::
pointsSlot(int state)
{
  plot_->setPoints(state);
}

void
CQChartsXYPlotCustomControls::
linesSlot(int state)
{
  plot_->setLines(state);
}

void
CQChartsXYPlotCustomControls::
fillUnderSlot(int state)
{
  plot_->setFillUnderFilled(state);
}

void
CQChartsXYPlotCustomControls::
stackedSlot(int state)
{
  plot_->setStacked(state);
}

void
CQChartsXYPlotCustomControls::
impulseSlot(int state)
{
  plot_->setImpulseVisible(state);
}

void
CQChartsXYPlotCustomControls::
bestFitSlot(int state)
{
  plot_->setBestFit(state);
}

void
CQChartsXYPlotCustomControls::
convexHullSlot(int state)
{
  plot_->setHull(state);
}

void
CQChartsXYPlotCustomControls::
statsLinesSlot(int state)
{
  plot_->setStatsLines(state);
}

void
CQChartsXYPlotCustomControls::
movingAverageSlot(int state)
{
  plot_->setMovingAverage(state);
}
