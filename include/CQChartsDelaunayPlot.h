#ifndef CQChartsDelaunayPlot_H
#define CQChartsDelaunayPlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsPlotObj.h>
#include <CQChartsData.h>

//---

/*!
 * \brief Delaunay plot type
 * \ingroup Charts
 */
class CQChartsDelaunayPlotType : public CQChartsPlotType {
 public:
  CQChartsDelaunayPlotType();

  QString name() const override { return "delaunay"; }
  QString desc() const override { return "Delaunay"; }

  Category category() const override { return Category::TWO_D; }

  void addParameters() override;

  bool canProbe() const override { return false; }

  bool canEqualScale() const override { return true; }

  QString description() const override;

  //---

  void analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData) override;

  //---

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsDelaunayPlot;
class CQChartsDelaunay;

//---

/*!
 * \brief Delaunay Plot Point object
 * \ingroup Charts
 */
class CQChartsDelaunayPointObj : public CQChartsPlotPointObj {
  Q_OBJECT

 public:
  using Plot   = CQChartsDelaunayPlot;
  using Length = CQChartsLength;
  using Symbol = CQChartsSymbol;

 public:
  CQChartsDelaunayPointObj(const Plot *plot, const BBox &rect, const Point &p, double value,
                           const QModelIndex &ind, const ColorInd &iv);

  //---

  const Plot *plot() const { return plot_; }

  double value() const { return value_; }

  //---

  QString calcId   () const override;
  QString calcTipId() const override;

  //---

  bool isVisible() const override;

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  void calcPenBrush(PenBrush &penBrush, bool updateState) const override;

  Length calcSymbolSize() const override;

 private:
  const Plot* plot_  { nullptr }; //!< plot
  double      value_ { 0.0 };     //!< value
};

//---

CQCHARTS_NAMED_LINE_DATA (Delaunay, delaunay)
CQCHARTS_NAMED_LINE_DATA (Voronoi, voronoi)
CQCHARTS_NAMED_POINT_DATA(Voronoi, voronoi)
CQCHARTS_NAMED_SHAPE_DATA(Voronoi, voronoi)

/*!
 * \brief Delaunay Plot
 * \ingroup Charts
 */
class CQChartsDelaunayPlot : public CQChartsPlot,
 public CQChartsObjDelaunayLineData<CQChartsDelaunayPlot>,
 public CQChartsObjVoronoiLineData <CQChartsDelaunayPlot>,
 public CQChartsObjVoronoiPointData<CQChartsDelaunayPlot>,
 public CQChartsObjVoronoiShapeData<CQChartsDelaunayPlot>,
 public CQChartsObjPointData       <CQChartsDelaunayPlot> {
  Q_OBJECT

  // columns
  Q_PROPERTY(CQChartsColumn xColumn     READ xColumn     WRITE setXColumn    )
  Q_PROPERTY(CQChartsColumn yColumn     READ yColumn     WRITE setYColumn    )
  Q_PROPERTY(CQChartsColumn nameColumn  READ nameColumn  WRITE setNameColumn )
  Q_PROPERTY(CQChartsColumn valueColumn READ valueColumn WRITE setValueColumn)

  // delaunay, voronoi
  Q_PROPERTY(bool delaunay       READ isDelaunay       WRITE setDelaunay      )
  Q_PROPERTY(bool voronoi        READ isVoronoi        WRITE setVoronoi       )
  Q_PROPERTY(bool voronoiCircles READ isVoronoiCircles WRITE setVoronoiCircles)
  Q_PROPERTY(bool voronoiPolygon READ isVoronoiPolygon WRITE setVoronoiPolygon)

  // delaunay lines
  CQCHARTS_NAMED_LINE_DATA_PROPERTIES (Delaunay, delaunay)

  // voronoi lines, points, shapes
  CQCHARTS_NAMED_LINE_DATA_PROPERTIES (Voronoi, voronoi)
  CQCHARTS_NAMED_POINT_DATA_PROPERTIES(Voronoi, voronoi)
  CQCHARTS_NAMED_SHAPE_DATA_PROPERTIES(Voronoi, voronoi)

  // points (display, symbol)
  CQCHARTS_POINT_DATA_PROPERTIES

 public:
  using Color     = CQChartsColor;
  using PenBrush  = CQChartsPenBrush;
  using PenData   = CQChartsPenData;
  using BrushData = CQChartsBrushData;
  using Symbol    = CQChartsSymbol;
  using ColorInd  = CQChartsUtil::ColorInd;

 public:
  CQChartsDelaunayPlot(View *view, const ModelP &model);
 ~CQChartsDelaunayPlot();

  //---

  void init() override;
  void term() override;

  //---

  // columns
  const Column &xColumn() const { return xColumn_; }
  void setXColumn(const Column &c);

  const Column &yColumn() const { return yColumn_; }
  void setYColumn(const Column &c);

  const Column &nameColumn() const { return nameColumn_; }
  void setNameColumn(const Column &c);

  const Column &valueColumn() const { return valueColumn_; }
  void setValueColumn(const Column &c);

  //---

  Column getNamedColumn(const QString &name) const override;
  void setNamedColumn(const QString &name, const Column &c) override;

  //---

  // voronoi
  bool isDelaunay() const { return delaunay_; }

  bool isVoronoi() const { return voronoi_; }

  bool isVoronoiCircles() const { return voronoiCircles_; }
  void setVoronoiCircles(bool b);

  bool isVoronoiPolygon() const { return voronoiPolygon_; }
  void setVoronoiPolygon(bool b);

  //---

  const QString &yname() const { return yname_; }

  //---

  void addProperties() override;

  Range calcRange() const override;

  void postCalcRange() override;

  bool createObjs(PlotObjs &objs) const override;

  void addPointObj(const Point &p, double value, const QModelIndex &xind,
                   int r, int nr, PlotObjs &objs) const;

  //---

  bool addMenuItems(QMenu *menu) override;

  //---

  bool hasBackground() const override;
  bool hasForeground() const override;

  void execDrawBackground(PaintDevice *device) const override;
  void execDrawForeground(PaintDevice *device) const override;

 public slots:
  void setDelaunay(bool b);
  void setVoronoi(bool b);

 protected:
  void drawDelaunay(PaintDevice *device) const;
  void drawVoronoi (PaintDevice *device) const;

  //---

  using PointObj = CQChartsDelaunayPointObj;

  virtual PointObj *createPointObj(const BBox &rect, const Point &p, double value,
                                   const QModelIndex &ind, const ColorInd &iv) const;

 protected:
  CQChartsPlotCustomControls *createCustomControls() override;

 private:
  using Delaunay = CQChartsDelaunay;

  Column    xColumn_;                    //!< x column
  Column    yColumn_;                    //!< y column
  Column    nameColumn_;                 //!< name column
  Column    valueColumn_;                //!< value column
  bool      delaunay_       { false };   //!< is delaunay
  bool      voronoi_        { true };    //!< is voronoi
  bool      voronoiCircles_ { false };   //!< voronoi circle
  bool      voronoiPolygon_ { false };   //!< voronoi polygon
  RMinMax   valueRange_;                 //!< value range
  Delaunay* delaunayData_   { nullptr }; //!< delaunay data
  QString   yname_;                      //!< y name
};

//---

#include <CQChartsPlotCustomControls.h>

/*!
 * \brief Delaunay Plot plot custom controls
 * \ingroup Charts
 */
class CQChartsDelaunayPlotCustomControls : public CQChartsPlotCustomControls {
  Q_OBJECT

 public:
  CQChartsDelaunayPlotCustomControls(CQCharts *charts);

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
  void delaunaySlot();
  void voronoiSlot();

 protected:
  CQChartsDelaunayPlot* plot_ { nullptr };

  FrameData optionsFrame_;

  CQChartsBoolParameterEdit* delaunayCheck_ { nullptr };
  CQChartsBoolParameterEdit* voronoiCheck_  { nullptr };
};

#endif
