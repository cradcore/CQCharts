#include <CQChartsScatterPlot3D.h>
#include <CQChartsView.h>
#include <CQChartsKey.h>
#include <CQChartsTitle.h>
#include <CQChartsValueSet.h>
#include <CQChartsModelDetails.h>
#include <CQChartsModelData.h>
#include <CQChartsColumnType.h>
#include <CQChartsDataLabel.h>
#include <CQChartsVariant.h>
#include <CQChartsTip.h>
#include <CQChartsHtml.h>
#include <CQChartsDrawUtil.h>
#include <CQCharts.h>
#include <CQChartsCamera.h>
#include <CQChartsSymbolBuffer.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQColorsPalette.h>
#include <CQPerfMonitor.h>

#include <CQGLControl.h>
#include <GL/glut.h>

#include <QMenu>
#include <QGLWidget>

CQChartsScatterPlot3DType::
CQChartsScatterPlot3DType()
{
}

void
CQChartsScatterPlot3DType::
addParameters()
{
  startParameterGroup("Scatter");

  // columns
  addColumnParameter("x", "X", "xColumn").
    setRequired().setNumericColumn().setPropPath("columns.x").setTip("X Value Column");
  addColumnParameter("y", "Y", "yColumn").
    setRequired().setNumericColumn().setPropPath("columns.y").setTip("Y Value Column");
  addColumnParameter("z", "Z", "zColumn").
    setRequired().setNumericColumn().setPropPath("columns.z").setTip("Z Value Column");

  addColumnParameter("name", "Name", "nameColumn").
    setStringColumn().setPropPath("columns.name").setTip("Optional Name Column");
  addColumnParameter("label", "Label", "labelColumn").
    setStringColumn().setPropPath("columns.label").setTip("Custom Label");

  addColumnParameter("symbolType", "Symbol Type", "symbolTypeColumn").
    setPropPath("columns.symbolType").setTip("Custom Symbol Type").setMapped().
    setMapMinMax(CQChartsSymbolType::minFillValue(), CQChartsSymbolType::maxFillValue());

  addColumnParameter("symbolSize", "Symbol Size", "symbolSizeColumn").
    setPropPath("columns.symbolSize").setTip("Custom Symbol Size").setMapped().
    setMapMinMax(CQChartsSymbolSize::minValue(), CQChartsSymbolSize::maxValue());

  addColumnParameter("fontSize", "Font Size", "fontSizeColumn").
    setPropPath("columns.fontSize").setTip("Custom Font Size for Label").setMapped().
    setMapMinMax(CQChartsFontSize::minValue(), CQChartsFontSize::maxValue());

  //--

  // options
  addBoolParameter("pointLabels", "Point Labels", "pointLabels").
    setTip("Show Label at Point").setPropPath("labels.visible");

  endParameterGroup();

  //---

  CQChartsPlot3DType::addParameters();
}

QString
CQChartsScatterPlot3DType::
description() const
{
  auto B   = [](const QString &str) { return CQChartsHtml::Str::bold(str); };
  auto IMG = [](const QString &src) { return CQChartsHtml::Str::img(src); };

  return CQChartsHtml().
   h2("Scatter Plot").
    h3("Summary").
     p("Draws scatter plot of x, y, z points with support for grouping and customization of "
       "point symbol type, symbol size and symbol color.").
     p("The points can have individual labels in which case the label font size can "
       "also be customized.").
    h3("Grouping").
     p("The points can be grouped by specifying a " + B("Name") + " column, all values "
       "with the same name are placed in that group and will be default colored by the "
       "group index.").
    h3("Columns").
     p("The points are specified by the " + B("X") + ", " + B("Y") + " and " + B("Z") +
       " columns.").
     p("An optional " + B("SymbolType") + " column can be specified to supply the type of the "
       "symbol drawn at the point. An optional " + B("SymbolSize") + " column can be specified "
       "to supply the size of the symbol drawn at the point. An optional " + B("Color") + " "
       "column can be specified to supply the fill color of the symbol drawn at the point.").
     p("An optional point label can be specified using the " + B("Label") + " column or the " +
       B("Name") + " column. The font size of the label can be specified using the " +
       B("FontSize") + " column.").
    h3("Limitations").
     p("None.").
    h3("Example").
     p(IMG("images/scatterplot3d.png"));
}

CQChartsPlot *
CQChartsScatterPlot3DType::
create(View *view, const ModelP &model) const
{
  return new CQChartsScatterPlot3D(view, model);
}

//---

CQChartsScatterPlot3D::
CQChartsScatterPlot3D(View *view, const ModelP &model) :
 CQChartsPlot3D(view, view->charts()->plotType("scatter3d"), model),
 CQChartsObjPointData<CQChartsScatterPlot3D>(this)
{
}

CQChartsScatterPlot3D::
~CQChartsScatterPlot3D()
{
  term();
}

//---

void
CQChartsScatterPlot3D::
init()
{
  CQChartsPlot3D::init();

  //---

  NoUpdate noUpdate(this);

  //---

  // create a data label (shared state for all data labels)
  dataLabel_ = new CQChartsDataLabel(this);

  dataLabel_->setSendSignal(true);

  connect(dataLabel_, SIGNAL(dataChanged()), this, SLOT(dataLabelChanged()));

  connect(this, SIGNAL(colorDetailsChanged()), this, SLOT(colorChanged()));

  //---

  setSymbol(Symbol::circle());
  setSymbolStroked(true);
  setSymbolFilled (true);
  setSymbolFillColor(Color::makePalette());

  //---

  addKey();

  addTitle();

  //---

  addColorMapKey();
}

void
CQChartsScatterPlot3D::
term()
{
  delete dataLabel_;
}

//------

void
CQChartsScatterPlot3D::
setNameColumn(const Column &c)
{
  CQChartsUtil::testAndSet(nameColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

void
CQChartsScatterPlot3D::
setLabelColumn(const Column &c)
{
  CQChartsUtil::testAndSet(labelColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

//---

void
CQChartsScatterPlot3D::
setXColumn(const Column &c)
{
  CQChartsUtil::testAndSet(xColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

void
CQChartsScatterPlot3D::
setYColumn(const Column &c)
{
  CQChartsUtil::testAndSet(yColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

void
CQChartsScatterPlot3D::
setZColumn(const Column &c)
{
  CQChartsUtil::testAndSet(zColumn_, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

//---

const CQChartsColumn &
CQChartsScatterPlot3D::
symbolTypeColumn() const
{
  return symbolTypeData_.column;
}

void
CQChartsScatterPlot3D::
setSymbolTypeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(symbolTypeData_.column, c, [&]() {
    updateObjs(); emit customDataChanged();
  } );
}

const CQChartsColumn &
CQChartsScatterPlot3D::
symbolSizeColumn() const
{
  return symbolSizeData_.column;
}

void
CQChartsScatterPlot3D::
setSymbolSizeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(symbolSizeData_.column, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

const CQChartsColumn &
CQChartsScatterPlot3D::
fontSizeColumn() const
{
  return fontSizeData_.column;
}

void
CQChartsScatterPlot3D::
setFontSizeColumn(const Column &c)
{
  CQChartsUtil::testAndSet(fontSizeData_.column, c, [&]() {
    updateRangeAndObjs(); emit customDataChanged();
  } );
}

//---

CQChartsColumn
CQChartsScatterPlot3D::
getNamedColumn(const QString &name) const
{
  Column c;
  if      (name == "x"         ) c = this->xColumn();
  else if (name == "y"         ) c = this->yColumn();
  else if (name == "z"         ) c = this->zColumn();
  else if (name == "name"      ) c = this->nameColumn();
  else if (name == "label"     ) c = this->labelColumn();
  else if (name == "color"     ) c = this->colorColumn();
  else if (name == "symbolType") c = this->symbolTypeColumn();
  else if (name == "symbolSize") c = this->symbolSizeColumn();
  else if (name == "fontSize"  ) c = this->fontSizeColumn();
  else                           c = CQChartsPlot::getNamedColumn(name);

  return c;
}

void
CQChartsScatterPlot3D::
setNamedColumn(const QString &name, const Column &c)
{
  if      (name == "x"         ) this->setXColumn(c);
  else if (name == "y"         ) this->setYColumn(c);
  else if (name == "z"         ) this->setZColumn(c);
  else if (name == "name"      ) this->setNameColumn(c);
  else if (name == "label"     ) this->setLabelColumn(c);
  else if (name == "color"     ) this->setColorColumn(c);
  else if (name == "symbolType") this->setSymbolTypeColumn(c);
  else if (name == "symbolSize") this->setSymbolSizeColumn(c);
  else if (name == "fontSize"  ) this->setFontSizeColumn(c);
  else                           CQChartsPlot::setNamedColumn(name, c);
}

//---

void
CQChartsScatterPlot3D::
setDrawSymbols(bool b)
{
  CQChartsUtil::testAndSet(drawSymbols_, b, [&]() { drawObjs(); } );
}

void
CQChartsScatterPlot3D::
setDrawLines(bool b)
{
  CQChartsUtil::testAndSet(drawLines_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsScatterPlot3D::
setDrawBars(bool b)
{
  CQChartsUtil::testAndSet(drawBars_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsScatterPlot3D::
setFillUnder(bool b)
{
  CQChartsUtil::testAndSet(fillUnder_, b, [&]() { updateRangeAndObjs(); } );
}

void
CQChartsScatterPlot3D::
setBarSize(double s)
{
  CQChartsUtil::testAndSet(barSize_, s, [&]() { updateRangeAndObjs(); } );
}

//---

void
CQChartsScatterPlot3D::
dataLabelChanged()
{
  // TODO: not enough info to optimize behavior so reload all objects
  updateRangeAndObjs();
}

void
CQChartsScatterPlot3D::
colorChanged()
{
  if (view()->is3D())
    updateRangeAndObjs();
}

//---

void
CQChartsScatterPlot3D::
addProperties()
{
  addBaseProperties();

  // columns
  addProp("columns", "xColumn", "x", "X column");
  addProp("columns", "yColumn", "y", "Y column");
  addProp("columns", "zColumn", "z", "Z column");

  addProp("columns", "nameColumn" , "name" , "Name column");
  addProp("columns", "labelColumn", "label", "Label column");

  addProp("columns", "symbolTypeColumn", "symbolType", "Symbol type column");
  addProp("columns", "symbolSizeColumn", "symbolSize", "Symbol size column");
  addProp("columns", "fontSizeColumn"  , "fontSize"  , "Font size column");

  //---

  // options
  addProp("options", "drawSymbols", "drawSymbols", "Draw symbols at points");
  addProp("options", "drawLines"  , "drawLines"  , "Draw lines between points");
  addProp("options", "drawBars"   , "drawBars"   , "Draw bars between points");
  addProp("options", "fillUnder"  , "fillUnder"  , "Fill under lines between points");

  addProp("options", "barSize", "barSize", "Bar size factor");

  //---

  // symbol
  addSymbolProperties("symbol", "", "");

  // data labels
  dataLabel()->addBasicProperties("labels", "Labels");
  dataLabel()->addTextProperties ("labels", "Labels");

  //---

  CQChartsPlot3D::addCameraProperties();

  CQChartsPlot3D::addProperties();

  //---

  // color map
  addColorMapProperties();

  // color map key
  addColorMapKeyProperties();
}

//---

CQChartsGeom::Range
CQChartsScatterPlot3D::
calcRange() const
{
  CQPerfTrace trace("CQChartsScatterPlot3D::calcRange");

  NoUpdate noUpdate(this);

  auto *th = const_cast<CQChartsScatterPlot3D *>(this);

  th->clearErrors();

  //---

  // check columns
  bool columnsValid = true;

  if (! checkColumn(xColumn(), "X", th->xColumnType_, /*required*/true))
    columnsValid = false;
  if (! checkColumn(yColumn(), "Y", th->yColumnType_, /*required*/true))
    columnsValid = false;
  if (! checkColumn(zColumn(), "Z", th->zColumnType_, /*required*/true))
    columnsValid = false;

  if (! checkColumn(nameColumn (), "Name" )) columnsValid = false;
  if (! checkColumn(labelColumn(), "Label")) columnsValid = false;

  if (! columnsValid)
    return Range(-1.0, -1.0, 1.0, 1.0);

  //---

  initGroupData(Columns(), Column());

  //---

  // calc data range (x, y, z values)
  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsScatterPlot3D *plot) :
     plot_(plot) {
      hasGroups_ = (plot_->numGroups() > 1);
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      if (plot_->isInterrupt())
        return State::TERMINATE;

      //---

      // init group
      ModelIndex xModelInd(plot_, data.row, plot_->xColumn(), data.parent);

      int groupInd = plot_->rowGroupInd(xModelInd);

      bool hidden = (hasGroups_ && plot_->isSetHidden(groupInd));
      if (hidden) return State::OK;

      //---

      bool isCell = (plot_->xColumn().isCell() ||
                     plot_->yColumn().isCell() ||
                     plot_->zColumn().isCell());

      if (isCell) {
        for (int col = 0; col < numCols(); ++col) {
          visitCell(col, data);
        }

        return State::OK;
      }
      else {
        return visitCell(-1, data);
      }
    }

    State visitCell(int col, const VisitData &data) {
      ModelIndex xModelInd(plot_, data.row, plot_->xColumn(), data.parent);
      ModelIndex yModelInd(plot_, data.row, plot_->yColumn(), data.parent);
      ModelIndex zModelInd(plot_, data.row, plot_->zColumn(), data.parent);

      xModelInd.setCellCol(col);
      yModelInd.setCellCol(col);
      zModelInd.setCellCol(col);

      //---

      double x   { 0.0  }, y   { 0.0  }, z   { 0.0  };
      bool   okx { true }, oky { true }, okz { true };

      //---

      auto modelValue = [&](const ColumnType &columnType, const ModelIndex &modelInd,
                            double &value, bool &ok, bool isLog, int &numUnique) {
        if      (columnType == ColumnType::REAL || columnType == ColumnType::INTEGER) {
          double defVal = plot_->getRowBadValue(data.row);

          ok = plot_->modelMappedReal(modelInd, value, isLog, defVal);
        }
        else if (columnType == ColumnType::TIME) {
          value = plot_->modelReal(modelInd, ok);
        }
        else if (modelInd.column().isRow() ||
                 modelInd.column().isColumn() ||
                 modelInd.column().isCell())
          value = plot_->modelReal(modelInd, ok);
        else {
          value = uniqueId(modelInd); ++numUnique;
        }
      };

      modelValue(plot_->xColumnType(), xModelInd, x, okx, plot_->isLogX(), uniqueX_);
      modelValue(plot_->yColumnType(), yModelInd, y, oky, plot_->isLogY(), uniqueY_);
      modelValue(plot_->zColumnType(), zModelInd, z, okz, /*log*/false   , uniqueZ_);

      //---

      if (plot_->isSkipBad() && (! okx || ! oky || ! okz))
        return State::SKIP;

      if (CMathUtil::isNaN(x) || CMathUtil::isNaN(y) || CMathUtil::isNaN(z))
        return State::SKIP;

      range3D_.updateRange(x, y, z);

      return State::OK;
    }

    int uniqueId(const ModelIndex &columnInd) {
      bool ok;

      auto var = plot_->modelValue(columnInd, ok);
      if (! var.isValid()) return -1;

      auto *columnDetails = this->columnDetails(columnInd.column());

      return (columnDetails ? columnDetails->uniqueId(var) : -1);
    }

    CQChartsModelColumnDetails *columnDetails(const Column &column) {
      if (! details_) {
        auto *modelData = plot_->getModelData();

        details_ = (modelData ? modelData->details() : nullptr);
      }

      return (details_ ? details_->columnDetails(column) : nullptr);
    }

    const Range3D &range3D() const { return range3D_; }

    bool isUniqueX() const { return uniqueX_ == numRows(); }
    bool isUniqueY() const { return uniqueY_ == numRows(); }
    bool isUniqueZ() const { return uniqueZ_ == numRows(); }

   private:
    const CQChartsScatterPlot3D* plot_      { nullptr };
    int                          hasGroups_ { false };
    Range3D                      range3D_;
    CQChartsModelDetails*        details_   { nullptr };
    int                          uniqueX_   { 0 };
    int                          uniqueY_   { 0 };
    int                          uniqueZ_   { 0 };
  };

  RowVisitor visitor(this);

  visitModel(visitor);

  auto dataRange3D = visitor.range3D();

  if (xmin().isSet()) dataRange3D.setXMin(xmin().real());
  if (xmax().isSet()) dataRange3D.setXMax(xmax().real());
  if (ymin().isSet()) dataRange3D.setYMin(ymin().real());
  if (ymax().isSet()) dataRange3D.setYMax(ymax().real());
  if (zmin().isSet()) dataRange3D.setZMin(zmin().real());
  if (zmax().isSet()) dataRange3D.setZMax(zmax().real());

  bool uniqueX = visitor.isUniqueX();
  bool uniqueY = visitor.isUniqueY();
  bool uniqueZ = visitor.isUniqueZ();

  if (isInterrupt())
    return Range(-1, -1, 1, 1);

  //---

  if (dataRange3D.isSet()) {
    if (uniqueX || uniqueY || uniqueZ) {
      if (uniqueX) {
        dataRange3D.updateRange(dataRange3D.xmin() - 0.5, dataRange3D.ymin(), dataRange3D.zmin());
        dataRange3D.updateRange(dataRange3D.xmax() + 0.5, dataRange3D.ymax(), dataRange3D.zmax());
      }

      if (uniqueY) {
        dataRange3D.updateRange(dataRange3D.xmin(), dataRange3D.ymin() - 0.5, dataRange3D.zmin());
        dataRange3D.updateRange(dataRange3D.xmax(), dataRange3D.ymax() + 0.5, dataRange3D.zmax());
      }

      if (uniqueZ) {
        dataRange3D.updateRange(dataRange3D.xmin(), dataRange3D.ymin(), dataRange3D.zmin() - 0.5);
        dataRange3D.updateRange(dataRange3D.xmax(), dataRange3D.ymax(), dataRange3D.zmin() + 0.5);
      }
    }
  }

  //---

  th->range3D_ = dataRange3D;

  th->camera_->init();

  th->groupNameValues_.clear();

  return Range(-1, -1, 1, 1);
}

void
CQChartsScatterPlot3D::
postUpdateRange()
{
  CQChartsPlot3D::postUpdateRange();

  groupObj_.clear();

  addAxis(xColumn(), yColumn(), zColumn());

  initGroups();

  addPointObjects();

  addObjs();
}

//------

bool
CQChartsScatterPlot3D::
createObjs(PlotObjs &) const
{
  return true;
}

void
CQChartsScatterPlot3D::
updateColumnNames()
{
  // set column header names
  CQChartsPlot::updateColumnNames();

  QString xname, yname;

  columnNames_[xColumn()] = xname;
  columnNames_[yColumn()] = yname;

  setColumnHeaderName(xColumn    (), "X"    );
  setColumnHeaderName(yColumn    (), "Y"    );
  setColumnHeaderName(zColumn    (), "Z"    );
  setColumnHeaderName(nameColumn (), "Name" );
  setColumnHeaderName(labelColumn(), "Label");
}

void
CQChartsScatterPlot3D::
initGroups()
{
  auto *th = const_cast<CQChartsScatterPlot3D *>(this);

  // init name values
  if (groupNameValues_.empty())
    addNameValues();

  th->groupPoints_.clear();

  //---

  th->updateColumnNames();
}

void
CQChartsScatterPlot3D::
addPointObjects() const
{
  auto *th = const_cast<CQChartsScatterPlot3D *>(this);

  //---

  auto *columnTypeMgr = charts()->columnTypeMgr();

  columnTypeMgr->startCache(model().data());

  //---

  int hasGroups = (numGroups() > 1);

  int ig = 0;
  int ng = int(groupNameValues_.size());

  if (! hasGroups) {
    if (ng <= 1 && parentPlot()) {
      ig = parentPlot()->childPlotIndex(this);
      ng = parentPlot()->numChildPlots();
    }
  }

  for (const auto &groupNameValue : groupNameValues_) {
    if (isInterrupt())
      break;

    int         groupInd   = groupNameValue.first;
    const auto &nameValues = groupNameValue.second;

    //---

    bool hidden = (hasGroups && isSetHidden(groupInd));

    if (hidden) { ++ig; continue; }

    //---

    // get group points
    auto pg = th->groupPoints_.find(groupInd);

    if (pg == th->groupPoints_.end())
      pg = th->groupPoints_.insert(pg, GroupPoints::value_type(groupInd, Points()));

    auto &points = const_cast<Points &>((*pg).second);

    //---

    int is = 0;
    int ns = int(nameValues.size());

    for (const auto &nameValue : nameValues) {
      if (isInterrupt())
        break;

      bool hidden = (! hasGroups && isSetHidden(is));

      if (hidden) { ++is; continue; }

      //---

    //const auto &name   = nameValue.first;
      const auto &values = nameValue.second.values;

      int nv = int(values.size());

      for (int iv = 0; iv < nv; ++iv) {
        if (isInterrupt())
          break;

        //---

        // get point position
        const auto &valuePoint = values[size_t(iv)];

        const auto &p = valuePoint.p;

        //---

        // get symbol size (needed for bounding box)
        Length symbolSize;

        double sx, sy;

        plotSymbolSize(symbolSize.isValid() ? symbolSize : this->symbolSize(), sx, sy);

        //---

        // create point object
        ColorInd is1(is, ns);
        ColorInd ig1(ig, ng);
        ColorInd iv1(iv, nv);

        BBox bbox(p.x - sx, p.y - sy, p.x + sx, p.y + sy);

        auto *pointObj = createPointObj(groupInd, bbox, p, is1, ig1, iv1);

        if (valuePoint.ind.isValid())
          pointObj->setModelInd(valuePoint.ind);

        if (symbolSize.isValid()) {
          pointObj->setSymbolSize(symbolSize);
//        pointObj->setSymbolDir (symbolSizeDir);
        }

      //objs.push_back(pointObj);

        points.push_back(p);

        //---

        // get point
        th->addPointObj(p, pointObj);

        //---

        // set optional symbol
        Symbol symbol;

        if (symbolTypeColumn().isValid()) {
          if (! columnSymbolType(valuePoint.row, valuePoint.ind.parent(), symbolTypeData_, symbol))
            symbol = Symbol();
        }

        if (symbol.isValid())
          pointObj->setSymbol(symbol);

        //---

        // set optional font size
        Length          fontSize;
        Qt::Orientation fontSizeDir { Qt::Horizontal };

        if (fontSizeColumn().isValid()) {
          if (! columnFontSize(valuePoint.row, valuePoint.ind.parent(), fontSizeData_,
                               fontSize, fontSizeDir))
            fontSize = Length();
        }

        if (fontSize.isValid()) {
          pointObj->setFontSize(fontSize);
//        pointObj->setLabelDir(fontSizeDir);
        }

        if (fontSize.isValid())
          pointObj->setFontSize(fontSize);

        //---

        // set optional symbol fill color
        Color symbolColor;

        if (colorColumn().isValid()) {
          if (! colorColumnColor(valuePoint.row, valuePoint.ind.parent(), symbolColor))
            symbolColor = Color();
        }

        if (symbolColor.isValid())
          pointObj->setColor(symbolColor);

        //---

        // set optional point label
        QString pointName;

        if (labelColumn().isValid() || nameColumn().isValid()) {
          bool ok;

          if (labelColumn().isValid()) {
            ModelIndex labelInd(th, valuePoint.row, labelColumn(), valuePoint.ind.parent());

            pointName = modelString(labelInd, ok);
          }
          else {
            ModelIndex nameInd(th, valuePoint.row, nameColumn(), valuePoint.ind.parent());

            pointName = modelString(nameInd, ok);
          }

          if (! ok)
            pointName.clear();
        }

        if (pointName.length())
          pointObj->setName(pointName);

        //---

        if (dataLabel()->isVisible()) {
          CQChartsPenBrush penBrush;

          auto tc = dataLabel()->interpTextColor(ColorInd());

          setPenBrush(penBrush,
            PenData(true, tc, dataLabel()->textAlpha()), BrushData(false));

          Length          fontSize;
          Qt::Orientation fontSizeDir;

          if (fontSizeColumn().isValid()) {
            if (! columnFontSize(valuePoint.row, valuePoint.ind.parent(), fontSizeData_,
                                 fontSize, fontSizeDir))
              fontSize = Length();
          }

          auto font = this->font();

          if (! font.isValid()) {
            font = dataLabel()->textFont();

            if (fontSize.isValid()) {
              double fontPixelSize = lengthPixelHeight(fontSize);

              // scale to font size
              fontPixelSize = limitFontSize(fontPixelSize);

              font.setPointSizeF(fontPixelSize);
            }
          }

          auto textOptions = dataLabel()->textOptions();

          textOptions.align = Qt::AlignHCenter | Qt::AlignBottom;

          auto *textObj = createTextObj(p, p, pointName);

          textObj->setPenBrush   (penBrush);
          textObj->setFont       (font);
          textObj->setTextOptions(textOptions);

          th->addPointObj(p, textObj);
        }
      }

      ++is;
    }

    ++ig;
  }

  //---

  columnTypeMgr->endCache(model().data());
}

void
CQChartsScatterPlot3D::
addNameValues() const
{
  CQPerfTrace trace("CQChartsScatterPlot3D::addNameValues");

  class RowVisitor : public ModelVisitor {
   public:
    RowVisitor(const CQChartsScatterPlot3D *plot) :
     plot_(plot) {
    }

    State visit(const QAbstractItemModel *, const VisitData &data) override {
      bool isCell = (plot_->xColumn().isCell() ||
                     plot_->yColumn().isCell() ||
                     plot_->zColumn().isCell());

      if (isCell) {
        for (int col = 0; col < numCols(); ++col) {
          (void) visitCell(col, data);
        }

        return State::OK;
      }
      else {
        return visitCell(-1, data);
      }
    }

    State visitCell(int col, const VisitData &data) {
      ModelIndex xModelInd(plot_, data.row, plot_->xColumn(), data.parent);
      ModelIndex yModelInd(plot_, data.row, plot_->yColumn(), data.parent);
      ModelIndex zModelInd(plot_, data.row, plot_->zColumn(), data.parent);

      xModelInd.setCellCol(col);
      yModelInd.setCellCol(col);
      zModelInd.setCellCol(col);

      //---

      // get group
      int groupInd = plot_->rowGroupInd(xModelInd);

      //---

      // get x, y, z value
      auto xInd  = plot_->modelIndex(xModelInd);
      auto xInd1 = plot_->normalizeIndex(xInd);

      double x   { 0.0  }, y   { 0.0  }, z   { 0.0  };
      bool   okx { true }, oky { true }, okz { true };

      //---

      auto modelValue = [&](const ColumnType &columnType, const ModelIndex &modelInd,
                            double &value, bool &ok, bool isLog) {
        if      (columnType == ColumnType::REAL || columnType == ColumnType::INTEGER) {
          double defVal = plot_->getRowBadValue(data.row);

          ok = plot_->modelMappedReal(modelInd, value, isLog, defVal);
        }
        else if (columnType == ColumnType::TIME) {
          value = plot_->modelReal(modelInd, ok);
        }
        else if (modelInd.column().isRow() ||
                 modelInd.column().isColumn() ||
                 modelInd.column().isCell())
          value = plot_->modelReal(modelInd, ok);
        else {
          value = uniqueId(modelInd);
        }
      };

      modelValue(plot_->xColumnType(), xModelInd, x, okx, plot_->isLogX());
      modelValue(plot_->yColumnType(), yModelInd, y, oky, plot_->isLogY());
      modelValue(plot_->zColumnType(), zModelInd, z, okz, /*log*/false);

      //---

      if (plot_->isSkipBad() && (! okx || ! oky || ! okz))
        return State::SKIP;

      if (CMathUtil::isNaN(x) || CMathUtil::isNaN(y))
        return State::SKIP;

      //---

      // get optional grouping name (name column, title)
      QString name;

      if (plot_->nameColumn().isValid()) {
        ModelIndex nameColumnInd(plot_, data.row, plot_->nameColumn(), data.parent);

        bool ok;

        name = plot_->modelString(nameColumnInd, ok);
      }

      if (! name.length() && plot_->title())
        name = plot_->title()->textStr();

      //---

      // get symbol type, size, font size and color
      Color color;

      // get color label (needed if not string ?)
      if (plot_->colorColumn().isValid()) {
        (void) plot_->colorColumnColor(data.row, data.parent, color);
      }

      //---

      auto *plot = const_cast<CQChartsScatterPlot3D *>(plot_);

      Point3D p(x, y, z);

      plot->addNameValue(groupInd, name, p, data.row, xInd1, color);

      return State::OK;
    }

    int uniqueId(const ModelIndex &columnInd) {
      bool ok;

      auto var = plot_->modelValue(columnInd, ok);
      if (! var.isValid()) return -1;

      auto *columnDetails = this->columnDetails(columnInd.column());

      return (columnDetails ? columnDetails->uniqueId(var) : -1);
    }

    CQChartsModelColumnDetails *columnDetails(const Column &column) {
      if (! details_) {
        auto *modelData = plot_->getModelData();

        details_ = (modelData ? modelData->details() : nullptr);
      }

      return (details_ ? details_->columnDetails(column) : nullptr);
    }

   private:
    const CQChartsScatterPlot3D* plot_    { nullptr };
    CQChartsModelDetails*        details_ { nullptr };
  };

  RowVisitor visitor(this);

  visitModel(visitor);
}

void
CQChartsScatterPlot3D::
addNameValue(int groupInd, const QString &name, const Point3D &p, int row,
             const QModelIndex &xind, const Color &color)
{
  auto &valuesData = groupNameValues_[groupInd][name];

  valuesData.xrange.add(p.x);
  valuesData.yrange.add(p.y);
  valuesData.zrange.add(p.z);

  valuesData.values.emplace_back(p, row, xind, color);
}

void
CQChartsScatterPlot3D::
addKeyItems(PlotKey *key)
{
  if (isOverlay() && ! isFirstPlot())
    return;

  addPointKeyItems(key);

  key->plot()->updateKeyPosition(/*force*/true);
}

void
CQChartsScatterPlot3D::
addPointKeyItems(CQChartsPlotKey *key)
{
  auto addKeyItem = [&](int ind, const QString &name, int i, int n) {
    ColorInd ic(i, n);

    auto *colorItem = new CQChartsScatter3DColorKeyItem(this, ind , ic);
    auto *textItem  = new CQChartsTextKeyItem          (this, name, ic);

    auto *groupItem = new CQChartsGroupKeyItem(this);

    groupItem->addRowItems(colorItem, textItem);

    //key->addItem(colorItem, i, 0);
    //key->addItem(textItem , i, 1);

    key->addItem(groupItem, i, 0);

    return colorItem;
  };

  int ng = int(groupNameValues_.size());

  // multiple group - key item per group
  if      (ng > 1) {
    int ig = 0;

    for (const auto &groupNameValue : groupNameValues_) {
      int  groupInd  = groupNameValue.first;
      auto groupName = groupIndName(groupInd);

      auto *colorItem = addKeyItem(groupInd, groupName, ig, ng);

      //--

      if (colorColumn().isValid() && colorColumn().isGroup()) {
        if (isColorMapped()) {
          double r = CMathUtil::map(groupInd, colorMapDataMin(), colorMapDataMax(),
                                    colorMapMin(), colorMapMax());

          auto color = Color::makePaletteValue(r);

          if (color.isValid())
            colorItem->setColor(color);
        }
      }

      //--

      ++ig;
    }
  }
  // single group - key item per value set
  else if (ng > 0) {
    const auto &nameValues = (*groupNameValues_.begin()).second;

    int ns = int(nameValues.size());

    if (ns > 1) {
      int is = 0;

      for (const auto &nameValue : nameValues) {
        const auto &name = nameValue.first;

        auto *colorItem = addKeyItem(-1, name, is, ns);

        //--

        if (colorColumn().isValid()) {
          const auto &values = nameValue.second.values;

          int nv = int(values.size());

          if (nv > 0) {
            const auto &valuePoint = values[0];

            Color color;

            if (colorColumnColor(valuePoint.row, valuePoint.ind.parent(), color))
              colorItem->setColor(color);
          }
        }

        //--

        ++is;
      }
    }
    else {
      if (parentPlot() && ! nameValues.empty()) {
        const auto &name = nameValues.begin()->first;

        int ig = parentPlot()->childPlotIndex(this);
        int ng = parentPlot()->numChildPlots();

        (void) addKeyItem(-1, name, ig, ng);
      }
    }
  }
}

//---

bool
CQChartsScatterPlot3D::
probe(ProbeData &probeData) const
{
  CQChartsPlotObj *obj;

  if (! objNearestPoint(probeData.p, obj))
    return false;

  auto c = obj->rect().getCenter();

  probeData.p    = c;
  probeData.both = true;

  probeData.xvals.emplace_back(c.x, "", "");
  probeData.yvals.emplace_back(c.y, "", "");

  return true;
}

//---

bool
CQChartsScatterPlot3D::
addMenuItems(QMenu *)
{
  return true;
}

//------

bool
CQChartsScatterPlot3D::
hasBackground() const
{
  return true;
}

void
CQChartsScatterPlot3D::
execDrawBackground(PaintDevice *device) const
{
  CQChartsPlot::execDrawBackground(device);
}

bool
CQChartsScatterPlot3D::
hasForeground() const
{
  if (! isLayerActive(CQChartsLayer::Type::FOREGROUND))
    return false;

  return true;
}

void
CQChartsScatterPlot3D::
execDrawForeground(PaintDevice *) const
{
}

void
CQChartsScatterPlot3D::
preDrawObjs(PaintDevice *) const
{
}

void
CQChartsScatterPlot3D::
addObjs() const
{
  auto *th = const_cast<CQChartsScatterPlot3D *>(this);

  int ng = int(groupPoints_.size());

  if (isDrawBars()) {
    for (auto &gp : groupPoints_) {
      for (auto &p : gp.second) {
        ColorInd ig(gp.first, ng);

        th->addBarPolygons(p, ig);
      }
    }
  }

  if (isFillUnder()) {
    for (auto &gp : groupPoints_) {
      if (gp.second.empty())
        continue;

      auto &groupData = th->getGroupData(gp.first);

      if (! groupData.polygon) {
        auto p = gp.second[0];

        //---

        groupData.polygon = createPolygonObj();

        groupData.polygon->setIg(ColorInd(gp.first, ng));

        th->addPointObj(p, groupData.polygon);
      }

      //---

      using Points = std::set<Point3D>;

      Points points;

      for (auto &p : gp.second)
        points.insert(p);

      //---

      bool first = true;

      Point3D p2;

      for (const auto &p : points) {
        p2 = p;

        if (first) {
          Point3D p1(p2.x, p2.y, range3D_.zmin());

          groupData.polygon->addPoint(p1);

          first = false;
        }

        groupData.polygon->addPoint(p2);
      }

      if (! first) {
        Point3D p1(p2.x, p2.y, range3D_.zmin());

        groupData.polygon->addPoint(p1);
      }
    }
  }

  //---

  if (isDrawLines()) {
    for (auto &gp : groupPoints_) {
      if (gp.second.empty())
        continue;

      auto &groupData = th->getGroupData(gp.first);

      if (! groupData.polyline) {
        auto p = gp.second[0];

        //---

        groupData.polyline = createPolylineObj();

        groupData.polyline->setIg(ColorInd(gp.first, ng));

        th->addPointObj(p, groupData.polyline);
      }

      //---

      using Points = std::set<Point3D>;

      Points points;

      for (auto &p : gp.second)
        points.insert(p);

      //---

      for (auto &p : points)
        groupData.polyline->addPoint(p);
    }
  }
}

//---

void
CQChartsScatterPlot3D::
addBarPolygons(const Point3D &p, const ColorInd &ig)
{
  const auto &range3D = this->range3D();

  double dt = this->barSize();

  double dx = dt*range3D.xsize();
  double dy = dt*range3D.ysize();
//double dz = dt*range3D.zsize();

  Point3D p1(p.x - dx, p.y - dy, p.z);
  Point3D p2(p.x + dx, p.y - dy, p.z);
  Point3D p3(p.x + dx, p.y + dy, p.z);
  Point3D p4(p.x - dx, p.y + dy, p.z);

  Point3D p5(p.x - dx, p.y - dy, range3D.zmin());
  Point3D p6(p.x + dx, p.y - dy, range3D.zmin());
  Point3D p7(p.x + dx, p.y + dy, range3D.zmin());
  Point3D p8(p.x - dx, p.y + dy, range3D.zmin());

  auto createPoly = [&](const Point3D &p1, const Point3D &p2, const Point3D &p3, const Point3D &p4,
                        const Point3D &n) {
    Polygon3D poly;

    poly.addPoint(p1);
    poly.addPoint(p2);
    poly.addPoint(p3);
    poly.addPoint(p4);

    auto *polyObj = createPolygonObj(poly);

    polyObj->setIg(ig);
    polyObj->setNormal(n);

    Point3D pm((p1 + p2 + p3 + p4)/4.0);

    Point3D pm1(pm.x, pm.y, range3D.zmin());

    addPointObj(pm1, polyObj);
  };

  createPoly(p1, p2, p6, p5, Point3D( 0, -1,  0));
  createPoly(p2, p3, p7, p6, Point3D( 1,  0,  0));
  createPoly(p3, p4, p8, p7, Point3D( 0,  1,  0));
  createPoly(p4, p1, p5, p8, Point3D(-1,  0,  0));
  createPoly(p1, p2, p3, p4, Point3D( 0,  0,  1));
  createPoly(p5, p6, p7, p8, Point3D( 0,  0, -1));
}

//---

void
CQChartsScatterPlot3D::
postDrawObjs(PaintDevice *device) const
{
  drawPointObjs(device);
}

//---

void
CQChartsScatterPlot3D::
init3D()
{
  init3D_ = false;
}

void
CQChartsScatterPlot3D::
draw3D()
{
  auto *glWidget = qobject_cast<QGLWidget *>(view()->glWidget());

  //auto *glControl = view()->glControl();

  if (! init3D_) {
    values3D_ = ValuesData();

    class RowVisitor : public ModelVisitor {
     public:
      RowVisitor(const CQChartsScatterPlot3D *plot, ValuesData *values) :
       plot_(plot), values_(values) {
      }

      State visit(const QAbstractItemModel *, const VisitData &data) override {
        ModelIndex xModelInd(plot_, data.row, plot_->xColumn(), data.parent);
        ModelIndex yModelInd(plot_, data.row, plot_->yColumn(), data.parent);
        ModelIndex zModelInd(plot_, data.row, plot_->zColumn(), data.parent);

        //---

        double x   { 0.0  }, y   { 0.0  }, z   { 0.0  };
        bool   okx { true }, oky { true }, okz { true };

        //---

        auto modelValue = [&](const ModelIndex &modelInd, double &value, bool &ok, bool isLog) {
          double defVal = plot_->getRowBadValue(data.row);

          ok = plot_->modelMappedReal(modelInd, value, isLog, defVal);
        };

        modelValue(xModelInd, x, okx, plot_->isLogX());
        modelValue(yModelInd, y, oky, plot_->isLogY());
        modelValue(zModelInd, z, okz, /*log*/false   );

        //---

        if (plot_->isSkipBad() && (! okx || ! oky || ! okz))
          return State::SKIP;

        if (CMathUtil::isNaN(x) || CMathUtil::isNaN(y) || CMathUtil::isNaN(z))
          return State::SKIP;

        Point3D p(x, y, z);

        values_->xrange.add(p.x);
        values_->yrange.add(p.y);
        values_->zrange.add(p.z);

        ValueData v(p);

        if (plot_->colorColumn().isValid()) {
          Color color;

          if (plot_->colorColumnColor(data.row, data.parent, color))
            v.color = color;
        }

        values_->values.push_back(v);

        return State::OK;
      }

     private:
      const CQChartsScatterPlot3D* plot_    { nullptr };
      ValuesData*                  values_;
    };

    RowVisitor visitor(this, &values3D_);

    visitModel(visitor);

    if (values3D_.xrange.isSet() && values3D_.yrange.isSet() && values3D_.zrange.isSet())
      range3D_ = Range3D(values3D_.xrange.min(), values3D_.yrange.min(), values3D_.zrange.min(),
                         values3D_.xrange.max(), values3D_.yrange.max(), values3D_.zrange.max());
    else
      range3D_ = Range3D(-1, -1, -1, 1, 1, 1);

    camera_->init();

    setWindowRange(BBox(-1, -1, 1, 1), BBox(-1, -1, 1, 1));

    init3D_ = true;
  }

  //---

  drawAxes3D();

  //---

  double sx, sy, psx, psy;

  plotSymbolSize (symbolSize(), sx, sy);
  pixelSymbolSize(symbolSize(), psx, psy);

  double ss  = std::min(sx , sy );
  double pss = std::min(psx, psy);

  //---

  CQChartsPenBrush penBrush;

  setSymbolPenBrush(penBrush, ColorInd());

  //---

  int textureId = -1;

  if (symbol().isValid()) {
    auto image = CQChartsSymbolBufferInst->getImage(symbol(), pss, penBrush.pen, penBrush.brush);

    glEnable(GL_TEXTURE_2D);

    textureId = int(glWidget->bindTexture(image));

    glBindTexture(GL_TEXTURE_2D, GLuint(textureId));

  //glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
  //glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_DECAL);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  //glEnable(GL_POINT_SPRITE);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
  }

  glPointSize(GLfloat(pss));

  glColor4f(1.0, 1.0, 1.0, 1.0);

  if (! symbol().isValid())
    glBegin(GL_POINTS);

  //---

  // get the current modelview matrix
  float modelview[16];
  glGetFloatv(GL_MODELVIEW_MATRIX, modelview);

  CVector3D right(modelview[0], modelview[4], modelview[8]);
  CVector3D up(modelview[1], modelview[5], modelview[9]);

  auto billboardTransform = [&](double x, double y, double z, double dx, double dy,
                                double &x1, double &y1, double &z1) {
    x1 = x + right[0]*dx*ss + up[0]*dy*ss;
    y1 = y + right[1]*dx*ss + up[1]*dy*ss;
    z1 = z + right[2]*dx*ss + up[2]*dy*ss;
  };

  //---

  for (const auto &v : values3D_.values) {
    if (v.color.isValid()) {
      auto c = interpColor(v.color, ColorInd());

      glColor4f(GLfloat(c.redF()), GLfloat(c.greenF()), GLfloat(c.blueF()), 1.0f);
    }

    auto x = CMathUtil::map(v.p.x, range3D_.xmin(), range3D_.xmax(), -1, 1);
    auto y = CMathUtil::map(v.p.y, range3D_.ymin(), range3D_.ymax(), -1, 1);
    auto z = CMathUtil::map(v.p.z, range3D_.zmin(), range3D_.zmax(), -1, 1);

    if (symbol().isValid()) {
      glBegin(GL_QUADS);

      double tx1 = 0.0, tx2 = 1.0;
      double ty1 = 0.0, ty2 = 1.0;

      double x1, y1, z1;
      double x2, y2, z2;
      double x3, y3, z3;
      double x4, y4, z4;

      billboardTransform(x, y, z, -1, -1, x1, y1, z1);
      billboardTransform(x, y, z,  1, -1, x2, y2, z2);
      billboardTransform(x, y, z,  1,  1, x3, y3, z3);
      billboardTransform(x, y, z, -1,  1, x4, y4, z4);

      glTexCoord2d(tx1, ty1); glVertex3d(x1, y1, z1);
      glTexCoord2d(tx1, ty2); glVertex3d(x2, y2, z2);
      glTexCoord2d(tx2, ty2); glVertex3d(x3, y3, z3);
      glTexCoord2d(tx2, ty1); glVertex3d(x4, y4, z4);

      glEnd();
    }
    else
      glVertex3d(x, y, z);
  }

  if (! symbol().isValid())
    glEnd();

  if (symbol().isValid()) {
    glWidget->deleteTexture(GLuint(textureId));

  //glDisable(GL_POINT_SPRITE);

    glDisable(GL_TEXTURE_2D);
    glDisable(GL_BLEND);
  }
}

void
CQChartsScatterPlot3D::
drawAxes3D()
{
  auto *glWidget = qobject_cast<QGLWidget *>(view()->glWidget());

#if 0
  //double r = bbox_.getRadius();

  double sphere_radius = /*0.7*/0.035;
  double cone_base     = /*0.6*/0.03;
  double cone_height   = /*4.0*/0.20;

  /* Name-stack manipulation for the purpose of selection hit
     processing when mouse button is pressed. Names are ignored
     in normal OpenGL rendering mode. */

  glPushMatrix();
    /* No name for grey sphere */

    glColor3f(0.3, 0.3, 0.3);
    glutSolidSphere(sphere_radius, 20, 20);

    glPushMatrix();
    glPushName(1);        /* Red cone is 1 */
      glColor3f(1, 0, 0);
      glRotatef(90, 0, 1, 0);
      glutSolidCone(cone_base, cone_height, 20, 20);
    glPopName();
    glPopMatrix();

    glPushMatrix ();
    glPushName(2);        /* Green cone is 2 */
      glColor3f(0, 1, 0);
      glRotatef(-90, 1, 0, 0);
      glutSolidCone(cone_base, cone_height, 20, 20);
    glPopName();
    glPopMatrix();

    glPushName(3);
      glColor3f(0, 0, 1); /* Blue cone is 3 */
      glutSolidCone(cone_base, cone_height, 20, 20);
    glPopName();

  glPopMatrix();
#endif

  //---

  auto drawLine = [&](double x1, double y1, double z1, double x2, double y2, double z2) {
    glBegin(GL_LINE_STRIP);
    glVertex3d(x1, y1, z1);
    glVertex3d(x2, y2, z2);
    glEnd();
  };

  auto drawPlane = [&](double x1, double y1, double z1, double x2, double y2, double z2,
                       double x3, double y3, double z3, double x4, double y4, double z4) {
    glBegin(GL_QUADS);
    glVertex3d(x1, y1, z1);
    glVertex3d(x2, y2, z2);
    glVertex3d(x3, y3, z3);
    glVertex3d(x4, y4, z4);
    glEnd();
  };

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE);

  glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

  drawLine(-1.0, -1.0, -1.0,  1.0, -1.0, -1.0);
  drawLine(-1.0, -1.0, -1.0, -1.0,  1.0, -1.0);
  drawLine(-1.0, -1.0, -1.0, -1.0, -1.0,  1.0);

  //---

  CInterval xinterval, yinterval, zinterval;

  xinterval.setStart(range3D_.xmin()); xinterval.setEnd(range3D_.xmax()); xinterval.setNumMajor(10);
  yinterval.setStart(range3D_.ymin()); yinterval.setEnd(range3D_.ymax()); yinterval.setNumMajor(10);
  zinterval.setStart(range3D_.zmin()); zinterval.setEnd(range3D_.zmax()); zinterval.setNumMajor(10);

  glColor4f(0.5f, 0.5f, 0.5f, 0.5f);

  drawPlane(-1, -1, -1,  1, -1, -1,  1,  1, -1, -1,  1, -1);
  drawPlane(-1, -1, -1, -1,  1, -1, -1,  1,  1, -1, -1,  1);
  drawPlane(-1, -1, -1,  1, -1, -1,  1, -1,  1, -1, -1,  1);

  //---

  for (double i = 0; i <= xinterval.calcNumMajor(); ++i) {
    auto x = xinterval.calcStart() + i*xinterval.calcIncrement();
    if (x < range3D_.xmin() || x > range3D_.xmax()) continue;

    double x1 = CMathUtil::map(x, range3D_.xmin(), range3D_.xmax(), -1.0, 1.0);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glWidget->renderText(x1, -1.1, -1.1, QString::number(x));

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    drawLine(x1, -1, -1, x1,  1, -1);
    drawLine(x1, -1, -1, x1, -1,  1);
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glWidget->renderText(0, -1.2, -1.2, "X");

  for (double i = 0; i <= yinterval.calcNumMajor(); ++i) {
    auto y = yinterval.calcStart() + i*yinterval.calcIncrement();
    if (y < range3D_.ymin() || y > range3D_.ymax()) continue;

    double y1 = CMathUtil::map(y, range3D_.ymin(), range3D_.ymax(), -1.0, 1.0);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glWidget->renderText(-1.1, y1, -1.1, QString::number(y));

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    drawLine(-1, y1, -1, -1, y1,  1);
    drawLine(-1, y1, -1,  1, y1, -1);
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glWidget->renderText(-1.2, 0, -1.2, "Y");

  for (double i = 0; i <= zinterval.calcNumMajor(); ++i) {
    auto z = zinterval.calcStart() + i*zinterval.calcIncrement();
    if (z < range3D_.zmin() || z > range3D_.zmax()) continue;

    double z1 = CMathUtil::map(z, range3D_.zmin(), range3D_.zmax(), -1.0, 1.0);

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    glWidget->renderText(-1.1, -1.1, z1, QString::number(z));

    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);

    drawLine(-1, -1, z1,  1, -1, z1);
    drawLine(-1, -1, z1, -1,  1, z1);
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  glWidget->renderText(-1.2, -1.2, 0, "Z");

  //---

  glDisable(GL_BLEND);
}

//---

CQChartsScatterPlot3D::GroupData &
CQChartsScatterPlot3D::
getGroupData(int groupId)
{
  auto pg = groupObj_.find(groupId);

  if (pg == groupObj_.end())
    pg = groupObj_.insert(pg, GroupObj::value_type(groupId, GroupData()));

  return (*pg).second;
}

//---

CQChartsScatterPoint3DObj *
CQChartsScatterPlot3D::
createPointObj(int groupInd, const BBox &rect, const Point3D &pos,
               const ColorInd &is, const ColorInd &ig, const ColorInd &iv) const
{
  return new CQChartsScatterPoint3DObj(this, groupInd, rect, pos, is, ig, iv);
}

//---

CQChartsPlotCustomControls *
CQChartsScatterPlot3D::
createCustomControls()
{
  auto *controls = new CQChartsScatterPlot3DCustomControls(charts());

  controls->init();

  controls->setPlot(this);

  controls->updateWidgets();

  return controls;
}

//------

CQChartsScatterPoint3DObj::
CQChartsScatterPoint3DObj(const CQChartsScatterPlot3D *plot, int groupInd,
                          const BBox &rect, const Point3D &pos,
                          const ColorInd &is, const ColorInd &ig, const ColorInd &iv) :
 CQChartsPlot3DObj(plot), groupInd_(groupInd), pos_(pos)
{
  setRect(rect);

  setIs(is);
  setIg(ig);
  setIv(iv);

  setDetailHint(DetailHint::MAJOR);
}

const CQChartsScatterPlot3D *
CQChartsScatterPoint3DObj::
scatterPlot() const
{
  return dynamic_cast<const CQChartsScatterPlot3D *>(plot3D());
}

//---

CQChartsSymbol
CQChartsScatterPoint3DObj::
symbol() const
{
  auto symbol = extraData().symbol;

  if (! symbol.isValid())
    symbol = scatterPlot()->symbol();

  return symbol;
}

CQChartsLength
CQChartsScatterPoint3DObj::
symbolSize() const
{
  auto symbolSize = extraData().symbolSize;

  if (! symbolSize.isValid())
    symbolSize = scatterPlot()->symbolSize();

  return symbolSize;
}

CQChartsLength
CQChartsScatterPoint3DObj::
fontSize() const
{
  auto fontSize = extraData().fontSize;

  return fontSize;
}

CQChartsColor
CQChartsScatterPoint3DObj::
color() const
{
  auto color = extraData().color;

  return color;
}

//---

QString
CQChartsScatterPoint3DObj::
calcId() const
{
  auto ind1 = plot_->unnormalizeIndex(modelInd());

  QString idStr;

  if (calcColumnId(ind1, idStr))
    return idStr;

  return QString("%1:%2:%3:%4").arg(typeName()).arg(is_.i).arg(ig_.i).arg(iv_.i);
}

QString
CQChartsScatterPoint3DObj::
calcTipId() const
{
  CQChartsTableTip tableTip;

  // add name (label or name column) as header
  if (name_.length())
    tableTip.addBoldLine(name_);

  //---

  // TODO: id column

  //---

  // add group column
  if (ig_.n > 1) {
    auto groupName = scatterPlot()->groupIndName(groupInd_);

    tableTip.addTableRow("Group", groupName);
  }

  //---

  // add x, y columns
  auto xstr = plot()->xStr(pos_.x);
  auto ystr = plot()->yStr(pos_.y);
  auto zstr = plot()->yStr(pos_.z);

  tableTip.addTableRow(scatterPlot()->xHeaderName(), xstr);
  tableTip.addTableRow(scatterPlot()->yHeaderName(), ystr);
  tableTip.addTableRow(scatterPlot()->zHeaderName(), zstr);

  //---

  // get values for name (grouped id identical names)
  CQChartsScatterPlot3D::ValueData valuePoint;

  auto pg = scatterPlot()->groupNameValues().find(groupInd_);
  assert(pg != scatterPlot()->groupNameValues().end());

  auto p = (*pg).second.find(name_);

  if (p != (*pg).second.end()) {
    const auto &values = (*p).second.values;

    valuePoint = values[size_t(iv_.i)];
  }

  //---

  auto addColumnRowValue = [&](const CQChartsColumn &column) {
    if (! column.isValid()) return;

    ModelIndex columnInd(plot_, modelInd().row(), column, modelInd().parent());

    bool ok;

    auto str = plot_->modelString(columnInd, ok);
    if (! ok) return;

    tableTip.addTableRow(plot_->columnHeaderName(column), str);
  };

  //---

  // add color column
  if (valuePoint.color.isValid())
    tableTip.addTableRow(plot_->colorHeaderName(), valuePoint.color.colorStr());
  else
    addColumnRowValue(plot_->colorColumn());

  //---

  plot()->addTipColumns(tableTip, modelInd());

  //---

  return tableTip.str();
}

//---

bool
CQChartsScatterPoint3DObj::
inside(const Point &p) const
{
  double sx, sy;

  plot_->pixelSymbolSize(this->symbolSize(), sx, sy);

  auto *camera = plot3D()->camera();

  auto pt = camera->transform(point());

  auto p1 = plot_->windowToPixel(Point(pt.x, pt.y));

  BBox pbbox(p1.x - sx, p1.y - sy, p1.x + sx, p1.y + sy);

  auto pp = plot_->windowToPixel(p);

  return pbbox.inside(pp);
}

void
CQChartsScatterPoint3DObj::
getObjSelectIndices(Indices &inds) const
{
  addColumnSelectIndex(inds, scatterPlot()->xColumn());
  addColumnSelectIndex(inds, scatterPlot()->yColumn());
  addColumnSelectIndex(inds, scatterPlot()->zColumn());

  addColumnSelectIndex(inds, plot_->colorColumn());
}

//---

void
CQChartsScatterPoint3DObj::
calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const
{
  auto ic = calcColorInd();

  scatterPlot()->setSymbolPenBrush(penBrush, ic);

  // override symbol fill color for custom color
  auto color = this->color();

  if (color.isValid()) {
    auto c = plot_->interpColor(color, ic);

    penBrush.brush.setColor(CQChartsDrawUtil::setColorAlpha(c, scatterPlot()->symbolFillAlpha()));
  }

  if (updateState)
    plot_->updateObjPenBrushState(this, penBrush, CQChartsPlot::DrawType::SYMBOL);
}

void
CQChartsScatterPoint3DObj::
postDraw(PaintDevice *device)
{
  if (! scatterPlot()->isDrawSymbols())
    return;

  // calc pen and brush
  CQChartsPenBrush penBrush;

  bool updateState = device->isInteractive();

  calcPenBrush(penBrush, updateState);

  //---

  device->setColorNames();

  //---

  // get symbol type and size
  auto symbol     = this->symbol();
  auto symbolSize = this->symbolSize();

  //---

  auto *camera = plot3D()->camera();

  auto pt = camera->transform(point());

  auto pt2 = pt.point2D();

  if (symbol.isValid())
    CQChartsDrawUtil::drawSymbol(device, penBrush, symbol, pt2, symbolSize, /*scale*/false);

  //---

  double sx = plot_->lengthPlotWidth (symbolSize);
  double sy = plot_->lengthPlotHeight(symbolSize);

  setDrawBBox(BBox(pt2.x - sx, pt2.y - sy, pt2.x + sx, pt2.y + sy));
}

//------

CQChartsScatter3DColorKeyItem::
CQChartsScatter3DColorKeyItem(CQChartsScatterPlot3D *plot, int groupInd, const ColorInd &ic) :
 CQChartsColorBoxKeyItem(plot, ColorInd(), ColorInd(), ic), groupInd_(groupInd)
{
}

bool
CQChartsScatter3DColorKeyItem::
selectPress(const Point &, SelData &selData)
{
  auto *plot = qobject_cast<CQChartsScatterPlot3D *>(plot_);

  int ih = hideIndex();

  if (selData.selMod == SelMod::ADD) {
    for (int i = 0; i < ic_.n; ++i) {
      plot_->CQChartsPlot::setSetHidden(i, i != ih);
    }
  }
  else {
    plot->setSetHidden(ih, ! plot->isSetHidden(ih));
  }

  return true;
}

QBrush
CQChartsScatter3DColorKeyItem::
fillBrush() const
{
  auto *plot = qobject_cast<CQChartsScatterPlot3D *>(plot_);

  QColor c;

  if (color_.isValid())
    c = plot_->interpColor(color_, ColorInd());
  else {
    c = plot->interpSymbolFillColor(ic_);

    //c = CQChartsColorBoxKeyItem::fillBrush().color();
  }

  CQChartsDrawUtil::setColorAlpha(c, plot->symbolFillAlpha());

  adjustFillColor(c);

  return c;
}

bool
CQChartsScatter3DColorKeyItem::
calcHidden() const
{
  auto *plot = qobject_cast<CQChartsScatterPlot3D *>(plot_);

  int ih = hideIndex();

  return plot->isSetHidden(ih);
}

int
CQChartsScatter3DColorKeyItem::
hideIndex() const
{
  return (groupInd_ >= 0 ? groupInd_ : ic_.i);
}

//------

CQChartsScatterPlot3DCustomControls::
CQChartsScatterPlot3DCustomControls(CQCharts *charts) :
 CQChartsPlotCustomControls(charts, "scatter3d")
{
}

void
CQChartsScatterPlot3DCustomControls::
init()
{
  addWidgets();

  addLayoutStretch();

  connectSlots(true);
}

void
CQChartsScatterPlot3DCustomControls::
addWidgets()
{
  // columns group
  auto columnsFrame = createGroupFrame("Columns", "columnsFrame");

  //---

  addNamedColumnWidgets(QStringList() <<
    "x" << "y" << "z" << "name" << "label" << "color" <<
    "symbolType" << "symbolSize" << "fontSize", columnsFrame);

  //---

  addKeyList();
}

void
CQChartsScatterPlot3DCustomControls::
connectSlots(bool b)
{
  CQChartsPlotCustomControls::connectSlots(b);
}

void
CQChartsScatterPlot3DCustomControls::
setPlot(CQChartsPlot *plot)
{
  if (plot_)
    disconnect(plot_, SIGNAL(customDataChanged()), this, SLOT(updateWidgets()));

  plot_ = dynamic_cast<CQChartsScatterPlot3D *>(plot);

  CQChartsPlotCustomControls::setPlot(plot);

  if (plot_)
    connect(plot_, SIGNAL(customDataChanged()), this, SLOT(updateWidgets()));
}

void
CQChartsScatterPlot3DCustomControls::
updateWidgets()
{
  connectSlots(false);

  //---

  CQChartsPlotCustomControls::updateWidgets();

  //---

  connectSlots(true);
}
