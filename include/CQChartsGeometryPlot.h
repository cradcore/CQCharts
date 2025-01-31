#ifndef CQChartsGeometryPlot_H
#define CQChartsGeometryPlot_H

#include <CQChartsPlot.h>
#include <CQChartsPlotType.h>
#include <CQChartsPlotObj.h>
#include <CQChartsPath.h>
#include <CQChartsStyle.h>

class CQChartsDataLabel;

//---

//! geometry shape data
struct CQChartsGeometryShape {
  enum class Type {
    NONE,
    POLYGON_LIST,
    POLYGON,
    RECT,
    PATH
  };

  using Polygons = CQChartsGeom::Polygons;
  using Polygon  = CQChartsGeom::Polygon;
  using BBox     = CQChartsGeom::BBox;

  CQChartsGeometryShape() = default;

  CQChartsGeometryShape(const QString &str);

  Type         type { Type::NONE };
  Polygons     polygonList;
  Polygon      polygon;
  BBox         rect;
  CQChartsPath path;
};

/*!
 * \brief Geometry plot type
 * \ingroup Charts
 */
class CQChartsGeometryPlotType : public CQChartsPlotType {
 public:
  using ColumnType = CQBaseModelType;

 public:
  CQChartsGeometryPlotType();

  QString name() const override { return "geometry"; }
  QString desc() const override { return "Geometry"; }

  void addParameters() override;

  bool canProbe() const override { return true; }

  QString description() const override;

  //---

  bool isColumnForParameter(ColumnDetails *columnDetails, Parameter *parameter) const override;

  void analyzeModel(ModelData *modelData, AnalyzeModelData &analyzeModelData) override;

  //---

  Plot *create(View *view, const ModelP &model) const override;
};

//---

class CQChartsGeometryPlot;

/*!
 * \brief Geometry Plot object
 * \ingroup Charts
 */
class CQChartsGeometryObj : public CQChartsPlotObj {
  Q_OBJECT

  Q_PROPERTY(double        value READ value WRITE setValue)
  Q_PROPERTY(QString       name  READ name  WRITE setName )
//Q_PROPERTY(CQChartsColor color READ color WRITE setColor)
  Q_PROPERTY(CQChartsStyle style READ style WRITE setStyle)

 public:
  using GeometryPlot = CQChartsGeometryPlot;
  using Style        = CQChartsStyle;
  using Column       = CQChartsColumn;
  using Color        = CQChartsColor;
  using OptReal      = CQChartsOptReal;

 public:
  CQChartsGeometryObj(const GeometryPlot *plot, const BBox &rect, const Polygons &polygons,
                      const QModelIndex &ind, const ColorInd &iv);

  QString typeName() const override { return "geom"; }

  QString calcId() const override;

  QString calcTipId() const override;

  bool isPolygon() const override { return true; }
  Polygon polygon() const override { return polygons_[0]; }

  //---

  const QString &name() const { return name_; }
  void setName(const QString &s) { name_ = s; }

  //const Color &color() const { return color_; }
  //void setColor(const Color &c) { color_ = c; }

  const Style &style() const { return style_; }
  void setStyle(const Style &s) { style_ = s; }

  bool hasValue() const { return value_.isSet(); }
  double value() const { return value_.realOr(0.0); }
  void setValue(double r) { value_ = OptReal(r); }

  bool inside(const Point &p) const override;

  void getObjSelectIndices(Indices &inds) const override;

  //---

  void draw(PaintDevice *device) const override;

  void drawFg(PaintDevice *device) const override;

  //---

  void calcPenBrush(PenBrush &penBrush, bool updateState) const override;

 private:
  const GeometryPlot* plot_     { nullptr }; //!< parent plot
  Polygons            polygons_;             //!< geometry polygons
  QString             name_;                 //!< geometry name
//Color               color_;                //!< optional color
  Style               style_;                //!< optional style
  OptReal             value_;                //!< geometry value
};

//---

/*!
 * \brief Geometry Plot
 * \ingroup Charts
 */
class CQChartsGeometryPlot : public CQChartsPlot,
 public CQChartsObjShapeData<CQChartsGeometryPlot> {
  Q_OBJECT

  // columns
  Q_PROPERTY(CQChartsColumn nameColumn     READ nameColumn     WRITE setNameColumn    )
  Q_PROPERTY(CQChartsColumn geometryColumn READ geometryColumn WRITE setGeometryColumn)
  Q_PROPERTY(CQChartsColumn valueColumn    READ valueColumn    WRITE setValueColumn   )
  Q_PROPERTY(CQChartsColumn styleColumn    READ styleColumn    WRITE setStyleColumn   )

  // color
  Q_PROPERTY(bool colorByValue READ isColorByValue WRITE setColorByValue)

  // value range
  Q_PROPERTY(double minValue READ minValue WRITE setMinValue)
  Q_PROPERTY(double maxValue READ maxValue WRITE setMaxValue)

  // cell
  Q_PROPERTY(ValueStyle valueStyle READ valueStyle WRITE setValueStyle)

  // selectable
  Q_PROPERTY(bool geometrySelectable READ isGeometrySelectable WRITE setGeometrySelectable)

  // shape
  CQCHARTS_SHAPE_DATA_PROPERTIES

  Q_ENUMS(ValueStyle)

 public:
  enum class ValueStyle {
    NONE,   // ???
    COLOR,  // color by value
    BALLOON // draw balloon using value
  };

  using OptReal = CQChartsOptReal;
  using Style   = CQChartsStyle;
  using Color   = CQChartsColor;

  //! geometry data
  struct Geometry {
    QString     name;     //!< name
    Polygons    polygons; //!< polygon list
    OptReal     value;    //!< value
//  Color       color;    //!< custom color
    Style       style;    //!< custom style
    BBox        bbox;     //!< bounding box
    QModelIndex ind;      //!< associated model index
  };

  using DataLabel   = CQChartsDataLabel;
  using GeometryObj = CQChartsGeometryObj;
  using ColorInd    = CQChartsUtil::ColorInd;

 public:
  CQChartsGeometryPlot(View *view, const ModelP &model);

  virtual ~CQChartsGeometryPlot();

  //---

  void init() override;
  void term() override;

  //---

  const Column &nameColumn() const { return nameColumn_; }
  void setNameColumn(const Column &c);

  const Column &geometryColumn() const { return geometryColumn_; }
  void setGeometryColumn(const Column &c);

  const Column &valueColumn() const { return valueColumn_; }
  void setValueColumn(const Column &c);

  const Column &styleColumn() const { return styleColumn_; }
  void setStyleColumn(const Column &c);

  //---

  Column getNamedColumn(const QString &name) const override;
  void setNamedColumn(const QString &name, const Column &c) override;

  //---

  bool isColorByValue() const { return valueStyle_ == ValueStyle::COLOR; }
  void setColorByValue(bool b);

  //---

  // value range
  double minValue() const;
  void setMinValue(double r);

  double maxValue() const;
  void setMaxValue(double r);

  //---

  // value style
  const ValueStyle &valueStyle() const { return valueStyle_; }

  //---

  bool isGeometrySelectable() const { return geometrySelectable_; }
  void setGeometrySelectable(bool b) { geometrySelectable_ = b; }

  //---

  // balloon min/max size
  double minBalloonSize() const { return minBalloonSize_; }
  void setMinBalloonSize(double r) { minBalloonSize_ = r; }

  double maxBalloonSize() const { return maxBalloonSize_; }
  void setMaxBalloonSize(double r) { maxBalloonSize_ = r; }

  //---

  const DataLabel *dataLabel() const { return dataLabel_; }
  DataLabel *dataLabel() { return dataLabel_; }

  //---

  void addProperties() override;

  void getPropertyNames(QStringList &names, bool hidden) const override;

  //---

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

  void write(std::ostream &os, const QString &plotVarName, const QString &modelVarName,
             const QString &viewVarName) const override;

  //---

  virtual GeometryObj *createGeometryObj(const BBox &rect, const Polygons &polygons,
                                         const QModelIndex &ind, const ColorInd &iv) const;

 public slots:
  void setValueStyle(const ValueStyle &style);

 private:
  void addRow(const QAbstractItemModel *model, const ModelVisitor::VisitData &data,
              Range &dataRange) const;

  bool decodeGeometry(const QString &geomStr, Polygons &polygons) const;

 protected:
  CQChartsPlotCustomControls *createCustomControls() override;

 private:
  using Geometries = std::vector<Geometry>;

  // columns
  Column     nameColumn_;                              //!< name column
  Column     geometryColumn_;                          //!< geometry column
  Column     valueColumn_;                             //!< value column
  Column     styleColumn_;                             //!< style column
  ColumnType geometryColumnType_ { ColumnType::NONE }; //!< geometry column type
  ColumnType colorColumnType_    { ColumnType::NONE }; //!< color column type
  ColumnType alphaColumnType_    { ColumnType::NONE }; //!< alpha column type
  ColumnType styleColumnType_    { ColumnType::NONE }; //!< style column type

  // labels
  DataLabel* dataLabel_ { nullptr }; //!< data label style

  // value
  OptReal    minValue_;                         //!< user min value
  OptReal    maxValue_;                         //!< user max value
  RMinMax    valueRange_;                       //!< data value range
  ValueStyle valueStyle_ { ValueStyle::COLOR }; //!< value style

  // selectable
  bool geometrySelectable_ { true }; //!< is geometry object selectable

  // balloon
  double minBalloonSize_ { 0.01 }; //!< min balloon size (fraction of height)
  double maxBalloonSize_ { 0.25 }; //!< max balloon size (fraction of height)

  // shapes
  Geometries geometries_; //!< geometry shapes
};

//---

#include <CQChartsPlotCustomControls.h>

/*!
 * \brief Geometry Plot plot custom controls
 * \ingroup Charts
 */
class CQChartsGeometryPlotCustomControls : public CQChartsPlotCustomControls {
  Q_OBJECT

 public:
  CQChartsGeometryPlotCustomControls(CQCharts *charts);

  void init() override;

  void setPlot(CQChartsPlot *plot) override;

 public slots:
  void updateWidgets() override;

 protected:
  void addWidgets() override;

  void addColumnWidgets() override;

  void connectSlots(bool b) override;

  //---

  CQChartsColor getColorValue() override;
  void setColorValue(const CQChartsColor &c) override;

 protected:
  CQChartsGeometryPlot* plot_ { nullptr };
};

#endif
