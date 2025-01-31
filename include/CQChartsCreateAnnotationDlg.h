#ifndef CQChartsCreateAnnotationDlg_H
#define CQChartsCreateAnnotationDlg_H

#include <CQChartsUnits.h>
#include <CQChartsTypes.h>
#include <QDialog>

class CQChartsView;
class CQChartsPlot;

class CQChartsPositionEdit;
class CQChartsRectEdit;
class CQChartsLengthEdit;
class CQChartsMarginEdit;
class CQChartsSidesEdit;
class CQChartsPolygonEdit;
class CQChartsSymbolDataEdit;
class CQChartsTextDataEdit;
class CQChartsFillDataEdit;
class CQChartsStrokeDataEdit;
class CQChartsArrowDataEdit;
class CQChartsImageEdit;
class CQChartsColorLineEdit;
class CQChartsAngleEdit;
class CQChartsLineEdit;

class CQRealSpin;
class CQCheckBox;

class QStackedWidget;
class QFrame;
class QComboBox;
class QRadioButton;
class QLabel;
class QBoxLayout;
class QHBoxLayout;
class QGridLayout;

/*!
 * \brief dialog to create a new annotation
 * \ingroup Charts
 */
class CQChartsCreateAnnotationDlg : public QDialog {
  Q_OBJECT

 public:
  using View  = CQChartsView;
  using Plot  = CQChartsPlot;
  using Units = CQChartsUnits::Type;

 public:
  CQChartsCreateAnnotationDlg(QWidget *parent, View *view);
  CQChartsCreateAnnotationDlg(QWidget *parent, Plot *plot);

  View *view() const { return view_; }
  Plot *plot() const { return plot_; }

 private:
  struct Widgets {
    QFrame*                 frame              { nullptr };
    CQChartsFillDataEdit*   backgroundDataEdit { nullptr };
    CQChartsStrokeDataEdit* strokeDataEdit     { nullptr };
    CQChartsSidesEdit*      borderSidesEdit    { nullptr };
  };

  // rectange, start, end, objRef, shapeType, numSides, angle, lineWidth, symbolType, symbolSize
  struct RectWidgets : public Widgets {
    CQChartsRectEdit*   rectEdit    { nullptr };
    CQChartsMarginEdit* marginEdit  { nullptr };
    CQChartsMarginEdit* paddingEdit { nullptr };
  };

  // center, xRadius, yRadius, objRef
  struct EllipseWidgets : public Widgets {
    QRadioButton*         centerRadio { nullptr };
    QRadioButton*         rectRadio   { nullptr };
    CQChartsPositionEdit* centerEdit  { nullptr };
    CQChartsLengthEdit*   rxEdit      { nullptr };
    CQChartsLengthEdit*   ryEdit      { nullptr };
    CQChartsRectEdit*     rectEdit    { nullptr };
    CQChartsLineEdit*     paddingEdit { nullptr };
  };

  // polygon, roundedLines
  struct PolygonWidgets : public Widgets {
    CQChartsPolygonEdit* pointsEdit { nullptr };
  };

  // polygon, roundedLines
  struct PolylineWidgets : public Widgets {
    CQChartsPolygonEdit* pointsEdit { nullptr };
  };

  // position, rectangle, objRef
  struct TextWidgets : public Widgets {
    QRadioButton*         positionRadio { nullptr };
    QRadioButton*         rectRadio     { nullptr };
    CQChartsPositionEdit* positionEdit  { nullptr };
    CQChartsRectEdit*     rectEdit      { nullptr };
    CQChartsLineEdit*     textEdit      { nullptr };
    CQChartsTextDataEdit* dataEdit      { nullptr };
  };

  // position, rectangle, objRef, image, disabledImage
  struct ImageWidgets : public Widgets {
    QRadioButton*         positionRadio     { nullptr };
    QRadioButton*         rectRadio         { nullptr };
    CQChartsPositionEdit* positionEdit      { nullptr };
    CQChartsRectEdit*     rectEdit          { nullptr };
    CQChartsImageEdit*    imageEdit         { nullptr };
    CQChartsImageEdit*    disabledImageEdit { nullptr };
  };

  // objRef, path
  struct PathWidgets : public Widgets {
    CQChartsLineEdit* pathEdit { nullptr };
  };

  // start, startObjRef, end, endObjRef
  struct ArrowWidgets : public Widgets {
    CQChartsPositionEdit*  startEdit       { nullptr };
    CQChartsPositionEdit*  endEdit         { nullptr };
    CQChartsArrowDataEdit* dataEdit        { nullptr };
    CQChartsLengthEdit*    strokeWidthEdit { nullptr };
    CQChartsColorLineEdit* strokeColorEdit { nullptr };
    CQCheckBox*            filledCheck     { nullptr };
    CQChartsColorLineEdit* fillColorEdit   { nullptr };
  };

  // start, startObjRef, end, endObjRef, isLine, rectilinear, frontType, tailType, lineWidth
  struct ArcWidgets : public Widgets {
  };

  // position, objRef, point data
  struct PointWidgets : public Widgets {
    CQChartsPositionEdit*   positionEdit { nullptr };
    CQChartsSymbolDataEdit* dataEdit     { nullptr };
  };

  // position, objRef, innerRadius, outerRadius, startAngle, spanAngle
  struct PieSliceWidgets : public Widgets {
    CQChartsPositionEdit* centerEdit      { nullptr };
    CQChartsLengthEdit*   innerRadiusEdit { nullptr };
    CQChartsLengthEdit*   outerRadiusEdit { nullptr };
    CQChartsAngleEdit*    startAngleEdit  { nullptr };
    CQChartsAngleEdit*    spanAngleEdit   { nullptr };
  };

  // direction, position, start, end, objRef
  struct AxisWidgets : public Widgets {
    QComboBox*  orientationEdit { nullptr };
    CQRealSpin* positionEdit    { nullptr };
    CQRealSpin* startEdit       { nullptr };
    CQRealSpin* endEdit         { nullptr };
  };

  // objRef
  struct KeyWidgets : public Widgets {
  };

  // objRef, values, drawType
  struct PointSetWidgets : public Widgets {
  };

  // rectangle, objRef, values
  struct ValueSetWidgets : public Widgets {
  };

  // position, objRef
  struct ButtonWidgets : public Widgets {
    CQChartsPositionEdit* positionEdit { nullptr };
    CQChartsLineEdit*     textEdit     { nullptr };
  };

  // position, rectangle, objRef, widget, align, sizePolicy, interactive
  struct WidgetWidgets : public Widgets {
  };

  // position
  struct SymbolMapKeyWidgets : public Widgets {
  };

 private:
  struct GridData {
    QGridLayout *layout;
    int          row { 0 };
  };

 private:
  QStringList typeNames() const;

  void initWidgets();

  QFrame *createNamedFrame(const QString &typeName);

  QFrame *createRectFrame    ();
  QFrame *createEllipseFrame ();
  QFrame *createPolygonFrame ();
  QFrame *createPolyLineFrame();
  QFrame *createTextFrame    ();
  QFrame *createImageFrame   ();
  QFrame *createPathFrame    ();
  QFrame *createArrowFrame   ();
  QFrame *createArcFrame     ();
  QFrame *createPointFrame   ();
  QFrame *createPieSliceFrame();
  QFrame *createAxisFrame    ();
  QFrame *createKeyFrame     ();
  QFrame *createPointSetFrame();
  QFrame *createValueSetFrame();
  QFrame *createButtonFrame  ();

  void addFillWidgets  (Widgets &widgets, QBoxLayout *playout);
  void addStrokeWidgets(Widgets &widgets, QBoxLayout *playout, bool corner);

  void addSidesWidget(Widgets &widgets, QBoxLayout *playout);

  QHBoxLayout *addLabelWidget(QBoxLayout *playout, const QString &label, QWidget *widget);

  //---

  CQCheckBox*            addBoolEdit(GridData &gridData, const QString &name,
                                     const QString &label, const QString &tip) const;
  CQChartsLineEdit*      addLineEdit(GridData &gridData, const QString &name,
                                     const QString &label, const QString &tip) const;
  CQRealSpin*            addRealEdit(GridData &gridData, const QString &name, const QString &label,
                                     double r, const QString &tip) const;
  QComboBox*             addComboEdit(GridData &gridData, const QString &name, const QString &label,
                                      const QStringList &items, const QString &tip) const;
  CQChartsPositionEdit*  addPositionEdit(GridData &gridData, const QString &name,
                                         const QString &label, double x, double y,
                                         const QString &tip) const;
  CQChartsLengthEdit*    addLengthEdit(GridData &gridData, const QString &name,
                                       const QString &label, double l, const QString &tip) const;
  CQChartsRectEdit*      addRectEdit(GridData &gridData, const QString &name,
                                     const QString &label, const QString &tip) const;
  CQChartsPolygonEdit*   addPolygonEdit(GridData &gridData, const QString &name,
                                        const QString &label, const QString &tip) const;
  CQChartsMarginEdit*    addMarginEdit(GridData &gridData, const QString &name,
                                       const QString &label, const QString &tip) const;
  CQChartsImageEdit*     addImageEdit(GridData &gridData, const QString &name,
                                      const QString &label, const QString &tip) const;
  CQChartsColorLineEdit* addColorEdit(GridData &gridData, const QString &name,
                                      const QString &label, const QString &tip) const;
  CQChartsAngleEdit*     addAngleEdit(GridData &gridData, const QString &name,
                                      const QString &label, double a, const QString &tip) const;

  GridData createGrid(QBoxLayout *layout) const;

  //---

  bool createRectangleAnnotation();
  bool createEllipseAnnotation  ();
  bool createPolygonAnnotation  ();
  bool createPolylineAnnotation ();
  bool createTextAnnotation     ();
  bool createImageAnnotation    ();
  bool createPathAnnotation     ();
  bool createArrowAnnotation    ();
  bool createArcAnnotation      ();
  bool createPointAnnotation    ();
  bool createPieSliceAnnotation ();
  bool createAxisAnnotation     ();
  bool createKeyAnnotation      ();
  bool createPointSetAnnotation ();
  bool createValueSetAnnotation ();
  bool createButtonAnnotation   ();

  //---

  bool setErrorMsg(const QString &msg);
  void clearErrorMsg();

 private slots:
  void typeSlot(int ind);

  void ellipseCenterSlot(bool);
  void textPositionSlot (bool);
  void imagePositionSlot(bool);

  void okSlot();
  bool applySlot();
  void cancelSlot();

 private:
  View*             view_             { nullptr }; //!< associated view
  Plot*             plot_             { nullptr }; //!< associated plot
  QComboBox*        typeCombo_        { nullptr }; //!< type combo
  CQChartsLineEdit* idEdit_           { nullptr }; //!< id edit
  CQChartsLineEdit* tipEdit_          { nullptr }; //!< tip edit
  QStackedWidget*   typeStack_        { nullptr }; //!< type stacked widget
  RectWidgets       rectWidgets_;                  //!< rect edit widgets
  EllipseWidgets    ellipseWidgets_;               //!< ellipse edit widgets
  PolygonWidgets    polygonWidgets_;               //!< polygon edit widgets
  PolylineWidgets   polylineWidgets_;              //!< polyline edit widgets
  TextWidgets       textWidgets_;                  //!< text edit widgets
  ImageWidgets      imageWidgets_;                 //!< image edit widgets
  PathWidgets       pathWidgets_;                  //!< path edit widgets
  ArrowWidgets      arrowWidgets_;                 //!< arrow edit widgets
  ArcWidgets        arcWidgets_;                   //!< arc edit widgets
  PointWidgets      pointWidgets_;                 //!< point edit widgets
  PieSliceWidgets   pieSliceWidgets_;              //!< pie slice widgets
  AxisWidgets       axisWidgets_;                  //!< axis widgets
  KeyWidgets        keyWidgets_;                   //!< key widgets
  PointSetWidgets   pointSetWidgets_;              //!< point set widgets
  ValueSetWidgets   valueSetWidgets_;              //!< value set widgets
  ButtonWidgets     buttonWidgets_;                //!< button widgets
  QLabel*           msgLabel_         { nullptr }; //!< error message label
};

#endif
