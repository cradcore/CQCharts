#ifndef CQChartsDensity_H
#define CQChartsDensity_H

#include <CQChartsGeom.h>
#include <CQChartsData.h>
#include <CQStatData.h>

#include <QObject>
#include <vector>

class CQChartsPlot;
class CQChartsPaintDevice;

//! whisker options
struct CQChartsWhiskerOpts {
  bool violin  { false };
  bool fitTail { false };
};

/*!
 * \brief Density plot data
 * \ingroup Charts
 */
class CQChartsDensity : public QObject {
  Q_OBJECT

  Q_PROPERTY(int             numSamples      READ numSamples      WRITE setNumSamples     )
  Q_PROPERTY(double          smoothParameter READ smoothParameter WRITE setSmoothParameter)
  Q_PROPERTY(DrawType        drawType        READ drawType        WRITE setDrawType       )
  Q_PROPERTY(Qt::Orientation orientation     READ orientation     WRITE setOrientation    )

  Q_ENUMS(DrawType)

 public:
  enum DrawType {
    WHISKER,
    WHISKER_BAR,
    DISTRIBUTION,
    CROSS_BAR,
    POINT_RANGE,
    ERROR_BAR,
    SCATTER,
    VIOLIN
  };

  using Plot        = CQChartsPlot;
  using PaintDevice = CQChartsPaintDevice;
  using WhiskerOpts = CQChartsWhiskerOpts;
  using Length      = CQChartsLength;
  using SymbolData  = CQChartsSymbolData;
  using XVals       = std::vector<double>;
  using Point       = CQChartsGeom::Point;
  using Points      = std::vector<Point>;
  using BBox        = CQChartsGeom::BBox;
  using Polygon     = CQChartsGeom::Polygon;

  struct DrawData {
    DrawData() { }

    bool scaled { true };
  };

 public:
  CQChartsDensity();

  //! get/set draw type
  const DrawType &drawType() const { return drawType_; }
  void setDrawType(const DrawType &t);

  //---

  //! get/set orientation
  const Qt::Orientation &orientation() const { return orientation_; }
  void setOrientation(const Qt::Orientation &o);

  bool isHorizontal() const { return orientation() == Qt::Horizontal; }
  bool isVertical  () const { return orientation() == Qt::Vertical  ; }

  //---

  //! get/set x values
  const XVals &xvals() const { return xvals_; }
  void setXVals(const XVals &xvals);

  const Points &opoints() const { return opoints_; }

  //! get/set num samples
  int numSamples() const { return numSamples_; }
  void setNumSamples(int i);

  //! get/set smooth parameter
  double smoothParameter() const { return smoothParameter_; }
  void setSmoothParameter(double r);

  //---

  double xmin() const { constCalc(); return xmin_; }
  double xmax() const { constCalc(); return xmax_; }

  double ymin() const { constCalc(); return ymin_; }
  double ymax() const { constCalc(); return ymax_; }

  double xmin1() const { constCalc(); return xmin1_; }
  double xmax1() const { constCalc(); return xmax1_; }

  double ymin1() const { constCalc(); return ymin1_; }
  double ymax1() const { constCalc(); return ymax1_; }

  double area() const { constCalc(); return area_; }

  //---

  double eval(double x) const;

  double yval(double x) const;

  //---

  void draw(const Plot *plot, PaintDevice *device, const BBox &rect,
            const DrawData &drawData=DrawData());

  BBox bbox(const BBox &rect) const;

  //---

  void drawWhisker(PaintDevice *device, const BBox &rect,
                   const Qt::Orientation &orientation, bool scaled) const;

  void drawWhiskerBar(PaintDevice *device, const BBox &rect,
                      const Qt::Orientation &orientation, bool scaled) const;

  void drawDistribution(const Plot *plot, PaintDevice *device, const BBox &rect,
                        const Qt::Orientation &orientation,
                        const WhiskerOpts &opts=WhiskerOpts()) const;

  void calcDistributionPoly(Polygon &ppoly, const Plot *plot, const BBox &rect,
                            const Qt::Orientation &orientation,
                            const WhiskerOpts &opts=WhiskerOpts()) const;

  void drawBuckets(PaintDevice *device, const BBox &rect,
                   const Qt::Orientation &orientation) const;

  //---

  static void drawCrossBar(PaintDevice *device, const BBox &rect, double mean,
                           const Qt::Orientation &orientation, const Length &cornerSize);

  static void drawPointRange(PaintDevice *device, const BBox &rect, double mean,
                             const Qt::Orientation &orientation, const SymbolData &symbol);

  static void drawErrorBar(PaintDevice *device, const BBox &rect, double mean,
                           const Qt::Orientation &orientation, const SymbolData &symbol);

  static void drawLineRange(PaintDevice *device, const BBox &rect,
                            const Qt::Orientation &orientation);

  //---

  static void drawScatter(PaintDevice *device, const BBox &rect, int n,
                          long seed, const Qt::Orientation &orientation);

 private:
  void invalidate();

  void constCalc() const;
  void calc();

  void constInit() const;
  void init();

 signals:
  void dataChanged();

 public:
  DrawType        drawType_        { DrawType::WHISKER }; //!< draw type
  Qt::Orientation orientation_     { Qt::Horizontal };    //!< orientation
  XVals           xvals_;                                 //!< x values
  int             nx_              { 0 };                 //!< number of x values
  Points          opoints_;                               //!< distribution points
  double          smoothParameter_ { -1.0 };              //!< smooth parameter
  int             numSamples_      { 100 };               //!< number of samples
  bool            initialized_     { false };             //!< is initialized
  bool            calced_          { false };             //!< is calculated

  long seed_ { -1 };

  // init data
  XVals      sxvals_;                   //!< sorted x values
  CQStatData statData_;                 //!< state data
  double     xmin_             { 0.0 }; //!< xmin
  double     xmax_             { 0.0 }; //!< xmax
  double     ymin_             { 0.0 }; //!< distribution ymin
  double     ymax_             { 0.0 }; //!< distribution ymax
  double     avg_              { 0.0 }; //!< x average
  double     sigma_            { 0.0 }; //!< x mean square root
  double     defaultBandwidth_ { 0.0 }; //!< bandwidth of point distribution

  // calc data
  double xmin1_ { 0.0 }; //!< adjusted xmin
  double xmax1_ { 0.0 }; //!< adjusted xmax
  double ymin1_ { 0.0 }; //!< adjusted ymin
  double ymax1_ { 0.0 }; //!< adjusted ymax
  double area_  { 1.0 }; //!< distrib polygon area
};

#endif
