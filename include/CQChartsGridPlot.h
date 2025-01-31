#ifndef CQChartsGridPlot_H
#define CQChartsGridPlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsPlotObj.h>
#include <CQChartsData.h>
#include <CQChartsValueSet.h>

//---

/*!
 * \brief Grid plot type
 * \ingroup Charts
 */
class CQChartsGridPlotType : public CQChartsPlotType {
 public:
  CQChartsGridPlotType();

  QString name() const override { return "grid"; }
  QString desc() const override { return "Grid"; }

  void addParameters() override;

  bool hasObjs() const override { return true; }

  bool hasAxes() const override { return false; }

  bool canEqualScale() const override { return true; }

  bool allowXLog() const override { return false; }
  bool allowYLog() const override { return false; }

  QString description() const override;

  bool canProbe() const override { return false; }

  //---

  void analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData) override;

  //---

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsGridPlot;
class CQChartsGrid;
class CQChartsGridCellObj;

//---

/*!
 * \brief Grid Plot
 * \ingroup Charts
 */
class CQChartsGridPlot : public CQChartsPlot,
 public CQChartsObjShapeData<CQChartsGridPlot>,
 public CQChartsObjTextData <CQChartsGridPlot> {
  Q_OBJECT

  // columns
  Q_PROPERTY(CQChartsColumn  nameColumn   READ nameColumn   WRITE setNameColumn  )
  Q_PROPERTY(CQChartsColumn  labelColumn  READ labelColumn  WRITE setLabelColumn )
  Q_PROPERTY(CQChartsColumn  rowColumn    READ rowColumn    WRITE setRowColumn   )
  Q_PROPERTY(CQChartsColumn  columnColumn READ columnColumn WRITE setColumnColumn)
  Q_PROPERTY(CQChartsColumns valueColumns READ valueColumns WRITE setValueColumns)

  // options
  Q_PROPERTY(DrawType            drawType    READ drawType    WRITE setDrawType   )
  Q_PROPERTY(double              cellMargin  READ cellMargin  WRITE setCellMargin )
  Q_PROPERTY(CQChartsPaletteName cellPalette READ cellPalette WRITE setCellPalette)
  Q_PROPERTY(bool                showValue   READ isShowValue WRITE setShowValue  )

  // shape
  CQCHARTS_SHAPE_DATA_PROPERTIES

  // text
  CQCHARTS_TEXT_DATA_PROPERTIES

  Q_ENUMS(DrawType)

 public:
  enum class DrawType {
    NONE,
    PIE,
    TREEMAP
  };

  using PaletteName = CQChartsPaletteName;
  using RValues     = CQChartsRValues;
  using Color       = CQChartsColor;
  using Alpha       = CQChartsAlpha;

 public:
  CQChartsGridPlot(View *view, const ModelP &model);

  virtual ~CQChartsGridPlot();

  //---

  void init() override;
  void term() override;

  //---

  // columns
  const Column &nameColumn() const { return nameColumn_; }
  void setNameColumn(const Column &c);

  const Column &labelColumn() const { return labelColumn_; }
  void setLabelColumn(const Column &c);

  const Column &rowColumn() const { return rowColumn_; }
  void setRowColumn(const Column &c);

  const Column &columnColumn() const { return columnColumn_; }
  void setColumnColumn(const Column &c);

  const Columns &valueColumns() const { return valueColumns_; }
  void setValueColumns(const Columns &c);

  //---

  Column getNamedColumn(const QString &name) const override;
  void setNamedColumn(const QString &name, const Column &c) override;

  Columns getNamedColumns(const QString &name) const override;
  void setNamedColumns(const QString &name, const Columns &c) override;

  //---

  const DrawType &drawType() const { return drawType_; }
  void setDrawType(const DrawType &t);

  double cellMargin() const { return cellMargin_; }
  void setCellMargin(double r);

  const PaletteName &cellPalette() const { return cellPalette_; }
  void setCellPalette(const PaletteName &n);

  bool isShowValue() const { return showValue_; }
  void setShowValue(bool b);

  //---

  double minValue() const { return minValue_; }
  double maxValue() const { return maxValue_; }

  //---

  void addProperties() override;

  //---

  Range calcRange() const override;

  bool createObjs(PlotObjs &objs) const override;

  //---

  bool addMenuItems(QMenu *menu) override;

  //---

 protected:
  using CellObj = CQChartsGridCellObj;

  virtual CellObj *createCellObj(const BBox &bbox, const QString &name, const QString &label,
                                 int row, int column, const RValues &values) const;

 protected:
  CQChartsPlotCustomControls *createCustomControls() override;

 private:
  // columns
  Column  nameColumn_;   //!< name column
  Column  labelColumn_;  //!< label column
  Column  rowColumn_;    //!< row column
  Column  columnColumn_; //!< column column
  Columns valueColumns_; //!< value columns

  // options
  DrawType    drawType_    { DrawType::PIE }; //!< draw type
  double      cellMargin_  { 0.1 };           //!< cell margin
  PaletteName cellPalette_ { "moreland" };    //!< cell object palette
  bool        showValue_   { true };          //!< show value in cell

  // working data
  double minValue_ { 0.0 }; //!< min overall value
  double maxValue_ { 0.0 }; //!< max overall value
};

//---

/*!
 * \brief Grid Cell object
 * \ingroup Charts
 */
class CQChartsGridCellObj : public CQChartsPlotObj {
  Q_OBJECT

 public:
  using Plot    = CQChartsGridPlot;
  using RValues = CQChartsRValues;
  using Angle   = CQChartsAngle;

 public:
  CQChartsGridCellObj(const Plot *plot, const BBox &bbox, const QString &name,
                      const QString &label, int row, int column, const RValues &values);

  QString typeName() const override { return "cell"; }

  QString calcId() const override;

  QString calcTipId() const override;

  void draw(PaintDevice *device) const override;

  void calcPenBrush(PenBrush &penBrush, bool updateState) const override;

  void getObjSelectIndices(Indices &inds) const override;

 private:
  const Plot* plot_   { nullptr }; //!< parent plot
  QString     name_;               //!< cell name
  QString     label_;              //!< optional label name
  int         row_    { 0 };       //!< cell row
  int         column_ { 0 };       //!< cell column
  RValues     values_;             //!< values
};

//---

#include <CQChartsPlotCustomControls.h>

/*!
 * \brief Grid Plot plot custom controls
 * \ingroup Charts
 */
class CQChartsGridPlotCustomControls : public CQChartsPlotCustomControls {
  Q_OBJECT

 public:
  CQChartsGridPlotCustomControls(CQCharts *charts);

  void init() override;

  void setPlot(CQChartsPlot *plot) override;

 public slots:
  void updateWidgets() override;

 protected:
  void addWidgets() override;

  void addColumnWidgets() override;

  void addOptionsWidgets() override;

  void connectSlots(bool b) override;

 protected slots:
  void drawTypeSlot();

 protected:
  CQChartsGridPlot* plot_ { nullptr };

  FrameData optionsFrame_;

  CQChartsEnumParameterEdit* drawTypeCombo_ { nullptr };
};

#endif
