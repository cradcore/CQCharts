#ifndef CQChartsImagePlot_H
#define CQChartsImagePlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsPlotObj.h>

//---

/*!
 * \brief Image plot type
 * \ingroup Charts
 */
class CQChartsImagePlotType : public CQChartsPlotType {
 public:
  CQChartsImagePlotType();

  QString name() const override { return "image"; }
  QString desc() const override { return "Image"; }

  // no dimension (uses whole model)
  Dimension dimension() const override { return Dimension::NONE; }

  void addParameters() override;

  bool hasAxes() const override { return false; }

  bool canProbe() const override { return true; }

  QString description() const override;

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsImagePlot;

/*!
 * \brief Image Plot cell object
 * \ingroup Charts
 */
class CQChartsImageObj : public CQChartsPlotObj {
  Q_OBJECT

 public:
  using Plot  = CQChartsImagePlot;
  using Image = CQChartsImage;

 public:
  CQChartsImageObj(const Plot *plot, const BBox &rect, int row, int col,
                   double value, const QModelIndex &ind, const ColorInd &iv);
  CQChartsImageObj(const Plot *plot, const BBox &rect, int row, int col,
                   const Image &image, const QModelIndex &ind);

  //---

  QString typeName() const override { return "image"; }

  QString calcId() const override;

  QString calcTipId() const override;

  //---

  double value() const { return value_; }

  //---

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  void calcPenBrush(CQChartsPenBrush &penBrush, bool updateState) const;

  //---

  void writeScriptData(ScriptPaintDevice *device) const override;

  //---

  double xColorValue(bool relative) const override;
  double yColorValue(bool relative) const override;

 private:
  const Plot*     plot_       { nullptr };               //!< parent plot
  int             row_        { -1 };                    //!< row
  int             col_        { -1 };                    //!< column
  double          value_      { 0.0 };                   //!< value
  Image           image_;                                //!< image
  CQBaseModelType columnType_ { CQBaseModelType::REAL }; //!< data type
};

//---

/*!
 * \brief Image Plot
 * \ingroup Charts
 */
class CQChartsImagePlot : public CQChartsPlot,
 public CQChartsObjCellShapeData    <CQChartsImagePlot>,
 public CQChartsObjCellLabelTextData<CQChartsImagePlot>,
 public CQChartsObjXLabelTextData   <CQChartsImagePlot>,
 public CQChartsObjYLabelTextData   <CQChartsImagePlot> {
  Q_OBJECT

  // value range
  Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)

  // cell
  Q_PROPERTY(CellStyle cellStyle READ cellStyle WRITE setCellStyle)

  Q_PROPERTY(bool cellLabels READ isCellLabels WRITE setCellLabels)

//Q_PROPERTY(bool scaleCellLabels READ isScaleCellLabels WRITE setScaleCellLabels)

  CQCHARTS_NAMED_SHAPE_DATA_PROPERTIES(Cell, cell)

  CQCHARTS_NAMED_TEXT_DATA_PROPERTIES(CellLabel, cellLabel)

  // x/y labels
  Q_PROPERTY(bool xLabels READ isXLabels WRITE setXLabels)

  CQCHARTS_NAMED_TEXT_DATA_PROPERTIES(XLabel, xLabel)

  Q_PROPERTY(bool yLabels READ isYLabels WRITE setYLabels)

  CQCHARTS_NAMED_TEXT_DATA_PROPERTIES(YLabel, yLabel)

  Q_ENUMS(CellStyle)

 public:
  enum class CellStyle {
    RECT,
    BALLOON
  };

  using Image    = CQChartsImage;
  using ImageObj = CQChartsImageObj;

 public:
  CQChartsImagePlot(View *view, const ModelP &model);
 ~CQChartsImagePlot();

  //---

  void init() override;
  void term() override;

  //---

  // dimension
  int numRows   () const { return nr_; }
  int numColumns() const { return nc_; }

  //---

  // value range
  double minValue() const { return minValue_; }
  void setMinValue(double r);

  double maxValue() const { return maxValue_; }
  void setMaxValue(double r);

  //---

  // cell labels
  bool isCellLabels     () const { return cellLabels_; }
//bool isScaleCellLabels() const { return scaleCellLabels_; }

  // x labels
  bool isXLabels() const { return xLabels_; }

  // y labels
  bool isYLabels() const { return yLabels_; }

  //---

  // cell style
  const CellStyle &cellStyle() const { return cellStyle_; }

  bool isRectStyle   () const { return cellStyle_ == CellStyle::RECT; }
  bool isBalloonStyle() const { return cellStyle_ == CellStyle::BALLOON; }

  // balloon min/max size
  double minBalloonSize() const { return minBalloonSize_; }
  void setMinBalloonSize(double r) { minBalloonSize_ = r; }

  double maxBalloonSize() const { return maxBalloonSize_; }
  void setMaxBalloonSize(double r) { maxBalloonSize_ = r; }

  //---

  void addProperties() override;

  Range calcRange() const override;

  bool createObjs(PlotObjs &objs) const override;

  //---

  bool probe(ProbeData &probeData) const override;

  //---

  bool addMenuItems(QMenu *menu) override;

  //---

  bool hasForeground() const override;

  void execDrawForeground(PaintDevice *device) const override;

  //---

  BBox calcAnnotationBBox() const override;

 public slots:
  void setRectStyle   (bool b);
  void setBalloonStyle(bool b);

  void setCellStyle(const CellStyle &style);

  void setCellLabels(bool b);

  void setXLabels(bool b);
  void setYLabels(bool b);

//void setScaleCellLabels(bool b);

 protected:
  void addImageObj(int row, int col, double x, double y, double dx, double dy,
                   double value, const QModelIndex &ind, PlotObjs &objs) const;
  void addImageObj(int row, int col, double x, double y, double dx, double dy,
                   const Image &image, const QModelIndex &ind, PlotObjs &objs) const;

  void drawXLabels(PaintDevice *device) const;
  void drawYLabels(PaintDevice *device) const;

  virtual ImageObj *createImageObj(const BBox &rect, int row, int col, double value,
                                   const QModelIndex &ind, const ColorInd &iv) const;
  virtual ImageObj *createImageObj(const BBox &rect, int row, int col,
                                   const Image &image, const QModelIndex &ind) const;

 private:
  CellStyle cellStyle_       { CellStyle::RECT }; //!< cell style
  bool      cellLabels_      { false };           //!< cell labels
//bool      scaleCellLabels_ { false };           //!< scale cell labels
  bool      xLabels_         { false };           //!< x labels
  bool      yLabels_         { false };           //!< y labels
  int       nc_              { 0 };               //!< number of grid columns
  int       nr_              { 0 };               //!< number of grid rows
  double    minValue_        { 0.0 };             //!< min value
  double    maxValue_        { 0.0 };             //!< max value
  double    minBalloonSize_  { 0.1 };             //!< min balloon size (cell fraction)
  double    maxBalloonSize_  { 1.0 };             //!< max balloon size (cell fraction)
};

#endif
