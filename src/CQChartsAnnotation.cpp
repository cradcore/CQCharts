#include <CQChartsAnnotation.h>
#include <CQChartsPlot.h>
#include <CQChartsView.h>
#include <CQChartsArrow.h>
#include <CQChartsEditHandles.h>
#include <CQChartsSmooth.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsVariant.h>
#include <CQChartsArcData.h>
#include <CQChartsDensity.h>
#include <CQChartsBivariateDensity.h>
#include <CQChartsFitData.h>
#include <CQChartsAxis.h>
#include <CQChartsKey.h>
#include <CQChartsMapKey.h>
#include <CQCharts.h>
#include <CQChartsViewPlotPaintDevice.h>
#include <CQChartsSVGPaintDevice.h>
#include <CQChartsPlotDrawUtil.h>
#include <CQChartsValueSet.h>
#include <CQChartsWidgetUtil.h>
#include <CQChartsResizeHandle.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQUtil.h>
#include <CQWinWidget.h>
#include <CMathRound.h>

#include <QStylePainter>

const QStringList &
CQChartsAnnotation::
typeNames()
{
  static QStringList names = QStringList() <<
    "rectangle" << "ellipse" << "polygon" << "polyline" << "text" << "image" << "path" <<
    "arrow" << "arc" << "point" << "pie_slice" << "axis" << "key" << "point_set" <<
    "value_set" << "button" << "widget" << "group";

  return names;
}

CQChartsAnnotation::
CQChartsAnnotation(View *view, Type type) :
 CQChartsTextBoxObj(view), type_(type)
{
  static int s_lastViewInd;

  init(s_lastViewInd);
}

CQChartsAnnotation::
CQChartsAnnotation(Plot *plot, Type type) :
 CQChartsTextBoxObj(plot), type_(type)
{
  static int s_lastPlotInd;

  init(s_lastPlotInd);
}

CQChartsAnnotation::
~CQChartsAnnotation()
{
}

void
CQChartsAnnotation::
init(int &lastInd)
{
  ind_ = ++lastInd;

  setEditable(true);
}

QString
CQChartsAnnotation::
calcId() const
{
  if (id_ && id_->length())
    return *id_;

  return QString("annotation:%1").arg(ind_);
}

QString
CQChartsAnnotation::
calcTipId() const
{
  if (tipId_ && tipId_->length())
    return *tipId_;

  return calcId();
}

QString
CQChartsAnnotation::
pathId() const
{
  auto id = propertyId();

  if      (plot())
    return plot()->pathId() + "|" + id;
  else if (view())
    return view()->id() + "|" + id;
  else
    return id;
}

//---

void
CQChartsAnnotation::
write(std::ostream &os, const QString &parentVarName, const QString &varName) const
{
  // -view/-plot -id -tip
  writeKeys(os, cmdName(), parentVarName, varName);

  writeDetails(os, parentVarName, varName);
}

void
CQChartsAnnotation::
writeKeys(std::ostream &os, const QString &cmd, const QString &parentVarName,
          const QString &varName) const
{
  auto parentName = [&]() {
    if (parentVarName == "") {
      if      (plot())
        return plot()->id();
      else if (view())
        return view()->id();
      else
        return QString("$parent");
    }
    else
      return QString("$") + parentVarName;
  };

  auto annotationName = [&]() {
    return (varName != "" ? varName : "plot");
  };

  os << "set " << annotationName().toStdString();
  os << " [" << cmd.toStdString();

  if      (plot())
    os << " -plot " << parentName().toStdString();
  else if (view())
    os << " -view " << parentName().toStdString();

  if (id() != "")
    os << " -id " << id().toStdString();

  if (tipId() != "")
    os << " -tip \"" << tipId().toStdString() << "\"";
}

void
CQChartsAnnotation::
writePoints(std::ostream &os, const Polygon &polygon) const
{
  if (polygon.size()) {
    os << " -points {";

    for (int i = 0; i < polygon.size(); ++i) {
      if (i > 0) os << " ";

      auto p1 = polygon.point(i);

      os << "{" << p1.x << " " << p1.y << "}";
    }

    os << "}";
  }
}

void
CQChartsAnnotation::
writeProperties(std::ostream &os, const QString &varName) const
{
  CQPropertyViewModel::NameValues nameValues;

  auto *propertyModel = this->propertyModel();

  if (propertyModel)
    propertyModel->getChangedNameValues(this, nameValues, /*tcl*/true);

  if (nameValues.empty())
    return;

  os << "\n";

  for (const auto &nv : nameValues) {
    QString str;

    if (! CQChartsVariant::toString(nv.second, str))
      str = "";

    os << "set_charts_property -annotation $" << varName.toStdString();

    os << " -name " << nv.first.toStdString() << " -value {" << str.toStdString() << "}\n";
  }
}

#if 0
void
CQChartsAnnotation::
writeFill()
{
  if (isFilled())
    os << " -filled 1";

  if (fillColor().isValid())
    os << " -fill_color {" << fillColor().toString().toStdString() << "}";

  if (fillAlpha() != Alpha())
    os << " -fill_alpha " << fillAlpha().value();
}

void
CQChartsAnnotation::
writeStroke()
{
  if (isStroked())
    os << " -stroked 1";

  if (strokeColor().isValid())
    os << " -stroke_color {" << strokeColor().toString().toStdString() << "}";

  if (strokeAlpha() != Alpha())
    os << " -stroke_alpha " << strokeAlpha().value();

  if (strokeWidth().isSet())
    os << " -stroke_width {" << strokeWidth().toString().toStdString() << "}";
}

#endif

//---

void
CQChartsAnnotation::
initRectangle()
{
}

//---

void
CQChartsAnnotation::
setEnabled(bool b)
{
  CQChartsUtil::testAndSet(enabled_, b, [&]() { invalidate(); } );
}

void
CQChartsAnnotation::
setCheckable(bool b)
{
  CQChartsUtil::testAndSet(checkable_, b, [&]() { invalidate(); } );
}

void
CQChartsAnnotation::
setChecked(bool b)
{
  CQChartsUtil::testAndSet(checked_, b, [&]() { invalidate(); } );
}

void
CQChartsAnnotation::
setDrawLayer(const DrawLayer &l)
{
  CQChartsUtil::testAndSet(drawLayer_, l, [&]() { invalidate(); } );
}

//---

void
CQChartsAnnotation::
setAnnotationBBox(const BBox &bbox)
{
  annotationBBox_ = bbox;
  rect_           = bbox;
}

//---

QString
CQChartsAnnotation::
propertyId() const
{
  return QString("%1%2").arg(propertyName()).arg(ind());
}

//---

void
CQChartsAnnotation::
invalidate()
{
  if      (plot()) {
    plot()->invalidateLayers ();
    plot()->invalidateOverlay();
  }
  else if (view()) {
    view()->invalidateObjects();
    view()->invalidateOverlay();

    view()->doUpdate();
  }
}

void
CQChartsAnnotation::
emitDataChanged()
{
  if (! isDisableSignals())
    emit dataChanged();
}

//---

void
CQChartsAnnotation::
getMarginValues(double &xlm, double &xrm, double &ytm, double &ybm) const
{
  xlm = lengthParentWidth (margin().left  ());
  xrm = lengthParentWidth (margin().right ());
  ytm = lengthParentHeight(margin().top   ());
  ybm = lengthParentHeight(margin().bottom());
}

void
CQChartsAnnotation::
getPaddingValues(double &xlp, double &xrp, double &ytp, double &ybp) const
{
  xlp = lengthParentWidth (padding().left  ());
  xrp = lengthParentWidth (padding().right ());
  ytp = lengthParentHeight(padding().top   ());
  ybp = lengthParentHeight(padding().bottom());
}

//---

void
CQChartsAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  model->setObjectRoot(path, this);

  addProp(model, path, "id"        , "id"        , "Annotation id");
  addProp(model, path, "visible"   , "visible"   , "Is visible");
  addProp(model, path, "enabled"   , "enabled"   , "Is enabled"   , true);
  addProp(model, path, "checkable" , "checkable" , "Is checkable" , true);
  addProp(model, path, "checked"   , "checked"   , "Is checked"   , true);
  addProp(model, path, "selected"  , "selected"  , "Is selected"  , true);
  addProp(model, path, "selectable", "selectable", "Is selectable");
  addProp(model, path, "editable"  , "editable"  , "Is editable"  , true);
  addProp(model, path, "fitted"    , "fitted"    , "Is fitted"    , true);
  addProp(model, path, "drawLayer" , "drawLayer" , "Draw layer"   , true);

  auto coloringPath = path + "/coloring";

  addProp(model, coloringPath, "disabledLighter" , "disabledLighter" ,
          "Ligher when disabled" , true);
  addProp(model, coloringPath, "uncheckedLighter", "uncheckedLighter",
          "Ligher when unchecked", true);
}

void
CQChartsAnnotation::
addStrokeFillProperties(PropertyModel *model, const QString &path, bool isSolid)
{
  addStrokeProperties(model, path + "/stroke", isSolid);
  addFillProperties  (model, path + "/fill"  );
}

void
CQChartsAnnotation::
addFillProperties(PropertyModel *model, const QString &path)
{
  addStyleProp(model, path, "filled"     , "visible", "Fill visible");
  addStyleProp(model, path, "fillColor"  , "color"  , "Fill color"  );
  addStyleProp(model, path, "fillAlpha"  , "alpha"  , "Fill alpha"  );
  addStyleProp(model, path, "fillPattern", "pattern", "Fill pattern");
}

void
CQChartsAnnotation::
addStrokeProperties(PropertyModel *model, const QString &path, bool isSolid)
{
  addStyleProp(model, path, "stroked"    , "visible", "Stroke visible");
  addStyleProp(model, path, "strokeColor", "color"  , "Stroke color"  );
  addStyleProp(model, path, "strokeAlpha", "alpha"  , "Stroke alpha"  );
  addStyleProp(model, path, "strokeWidth", "width"  , "Stroke width"  );
  addStyleProp(model, path, "strokeDash" , "dash"   , "Stroke dash"   );
  addStyleProp(model, path, "strokeCap"  , "cap"    , "Stroke cap"    );
  addStyleProp(model, path, "strokeJoin" , "join"   , "Stroke join"   );

  if (isSolid) {
    addStyleProp(model, path, "cornerSize" , "cornerSize", "Box corner size"  );
    addStyleProp(model, path, "borderSides", "sides"     , "Box visible sides");
  }
}

void
CQChartsAnnotation::
addTextProperties(PropertyModel *model, const QString &path, uint types)
{
  if (types & uint(CQChartsTextOptions::ValueType::DATA))
    addStyleProp(model, path, "textData", "style", "Text style", true);

  addStyleProp(model, path, "textColor", "color", "Text color");
  addStyleProp(model, path, "textAlpha", "alpha", "Text alpha");
  addStyleProp(model, path, "textFont" , "font" , "Text font");

  if (types & uint(CQChartsTextOptions::ValueType::ALIGN))
    addStyleProp(model, path, "textAlign", "align", "Text align");

  if (types & uint(CQChartsTextOptions::ValueType::ANGLE))
    addStyleProp(model, path, "textAngle", "angle", "Text angle");

  if (types & uint(CQChartsTextOptions::ValueType::CONTRAST))
    addStyleProp(model, path, "textContrast", "contrast", "Text has contrast");

  if (types & uint(CQChartsTextOptions::ValueType::FORMATTED))
    addStyleProp(model, path, "textFormatted", "formatted", "Text formatted to fit in box");

  if (types & uint(CQChartsTextOptions::ValueType::SCALED))
    addStyleProp(model, path, "textScaled", "scaled", "Text scaled to fit box");

  if (types & uint(CQChartsTextOptions::ValueType::HTML))
    addStyleProp(model, path, "textHtml", "html", "Text is HTML");

  if (types & uint(CQChartsTextOptions::ValueType::CLIP_LENGTH))
    addStyleProp(model, path, "textClipLength", "clipLength", "Text clip length");

  if (types & uint(CQChartsTextOptions::ValueType::CLIP_ELIDE))
    addStyleProp(model, path, "textClipElide" , "clipElide" , "Text clip elide");
}

CQPropertyViewItem *
CQChartsAnnotation::
addStyleProp(PropertyModel *model, const QString &path, const QString &name,
             const QString &alias, const QString &desc, bool hidden)
{
  auto *item = addProp(model, path, name, alias, desc, hidden);

  CQCharts::setItemIsStyle(item);

  return item;
}

CQPropertyViewItem *
CQChartsAnnotation::
addProp(PropertyModel *model, const QString &path, const QString &name,
        const QString &alias, const QString &desc, bool hidden)
{
  auto *item = model->addProperty(path, this, name, alias);

  item->setDesc(desc);

  if (hidden)
    CQCharts::setItemIsHidden(item);

  return item;
}

//---

bool
CQChartsAnnotation::
getProperty(const QString &name, QVariant &value) const
{
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return false;

  if (propertyModel->getProperty(this, name, value))
    return true;

  if (CQUtil::getTclProperty(this, name, value))
    return true;

  return false;
}

bool
CQChartsAnnotation::
getTclProperty(const QString &name, QVariant &value) const
{
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return false;

  if (propertyModel->getTclProperty(this, name, value))
    return true;

  if (CQUtil::getTclProperty(this, name, value))
    return true;

  return false;
}

bool
CQChartsAnnotation::
getPropertyDesc(const QString &name, QString &desc, bool hidden) const
{
  const auto *item = propertyItem(name, hidden);
  if (! item) return false;

  desc = item->desc();

  return true;
}

bool
CQChartsAnnotation::
getPropertyType(const QString &name, QString &type, bool hidden) const
{
  const auto *item = propertyItem(name, hidden);
  if (! item) return false;

  type = item->typeName();

  return true;
}

bool
CQChartsAnnotation::
getPropertyUserType(const QString &name, QString &type, bool hidden) const
{
  const auto *item = propertyItem(name, hidden);
  if (! item) return false;

  type = item->userTypeName();

  return true;
}

bool
CQChartsAnnotation::
getPropertyObject(const QString &name, QObject* &object, bool hidden) const
{
  object = nullptr;

  const auto *item = propertyItem(name, hidden);
  if (! item) return false;

  object = item->object();

  return true;
}

bool
CQChartsAnnotation::
getPropertyIsHidden(const QString &name, bool &is_hidden) const
{
  is_hidden = false;

  const auto *item = propertyItem(name, /*hidden*/true);
  if (! item) return false;

  is_hidden = CQCharts::getItemIsHidden(item);

  return true;
}

bool
CQChartsAnnotation::
getPropertyIsStyle(const QString &name, bool &is_style) const
{
  is_style = false;

  const auto *item = propertyItem(name, /*hidden*/true);
  if (! item) return false;

  is_style = CQCharts::getItemIsStyle(item);

  return true;
}

bool
CQChartsAnnotation::
setProperties(const QString &properties)
{
  bool rc = true;

  CQChartsNameValues nameValues(properties);

  for (const auto &nv : nameValues.nameValues()) {
    const auto &name  = nv.first;
    const auto &value = nv.second;

    if (! setProperty(name, value))
      rc = false;
  }

  return rc;
}

bool
CQChartsAnnotation::
setProperty(const QString &name, const QVariant &value)
{
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return false;

  if (propertyModel->setProperty(this, name, value))
    return true;

  if (CQUtil::setProperty(this, name, value))
    return true;

  return false;
}

void
CQChartsAnnotation::
getPropertyNames(QStringList &names, bool hidden) const
{
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return;

  propertyModel->objectNames(this, names, hidden);
}

CQPropertyViewModel *
CQChartsAnnotation::
propertyModel() const
{
  if      (plot())
    return plot()->propertyModel();
  else if (view())
    return view()->propertyModel();
  else
    return nullptr;
}

const CQPropertyViewItem *
CQChartsAnnotation::
propertyItem(const QString &name, bool hidden) const
{
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return nullptr;

  const auto *item = propertyModel->propertyItem(this, name, hidden);
  if (! item) return nullptr;

  return item;
}

//---

bool
CQChartsAnnotation::
contains(const Point &p) const
{
  if (! isVisible())
    return false;

  return inside(p);
}

bool
CQChartsAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

bool
CQChartsAnnotation::
intersects(const BBox &r, bool inside) const
{
  if (inside)
    return r.inside(annotationBBox());
  else
    return r.overlaps(annotationBBox());
}

//---

QColor
CQChartsAnnotation::
interpColor(const CQChartsColor &c, const ColorInd &ind) const
{
  if      (plot())
    return plot()->interpColor(c, ind);
  else if (view())
    return view()->interpColor(c, ind);
  else
    return charts()->interpColor(c, ind);
}

//---

bool
CQChartsAnnotation::
selectPress(const Point &, SelMod)
{
  if (! isEnabled())
    return false;

  if (isCheckable())
    setChecked(! isChecked());

  emit pressed(id());

  return id().length();
}

//---

bool
CQChartsAnnotation::
editPress(const Point &p)
{
  if (! isEnabled())
    return false;

  editHandles()->setDragPos(p);

  return true;
}

bool
CQChartsAnnotation::
editMove(const Point &p)
{
  const auto &dragPos  = editHandles()->dragPos();
  const auto &dragSide = editHandles()->dragSide();

  double dx = p.x - dragPos.x;
  double dy = p.y - dragPos.y;

  editHandles()->updateBBox(dx, dy);

  if (dragSide != ResizeSide::MOVE)
    initRectangle();

  setEditBBox(editHandles()->bbox(), dragSide);

  editHandles()->setDragPos(p);

  invalidate();

  return true;
}

bool
CQChartsAnnotation::
editMotion(const Point &p)
{
  return editHandles()->selectInside(p);
}

void
CQChartsAnnotation::
editMoveBy(const Point &f)
{
  editHandles()->setDragSide(ResizeSide::MOVE);

  editHandles()->updateBBox(f.x, f.y);

  setEditBBox(editHandles()->bbox(), ResizeSide::MOVE);

  invalidate();
}

//---

void
CQChartsAnnotation::
calcPenBrush(PenBrush &penBrush)
{
  // set pen and brush
  auto bgColor     = interpFillColor  (ColorInd());
  auto strokeColor = interpStrokeColor(ColorInd());

  if (isEnabled()) {
    if (isCheckable() && ! isChecked()) {
      double f = uncheckedLighter();

      bgColor     = CQChartsUtil::blendColors(backgroundColor(), bgColor    , f);
      strokeColor = CQChartsUtil::blendColors(backgroundColor(), strokeColor, f);
    }
  }
  else {
    double f = disabledLighter();

    bgColor     = CQChartsUtil::blendColors(backgroundColor(), bgColor    , f);
    strokeColor = CQChartsUtil::blendColors(backgroundColor(), strokeColor, f);
  }

  setPenBrush(penBrush,
    PenData  (isStroked(), strokeColor, strokeAlpha(), strokeWidth(), strokeDash(),
              strokeCap(), strokeJoin()),
    BrushData(isFilled (), bgColor, fillAlpha(), fillPattern()));
}

void
CQChartsAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);
  drawTerm(device);
}

void
CQChartsAnnotation::
drawInit(PaintDevice *device)
{
  device->save();

  if (device->type() == PaintDevice::Type::SVG) {
    auto *painter = dynamic_cast<CQChartsSVGPaintDevice *>(device);

    CQChartsSVGPaintDevice::GroupData groupData;

    groupData.onclick   = true;
    groupData.clickProc = "annotationClick";

    painter->startGroup(id(), groupData);
  }

  if (plot())
    plot()->setClipRect(device);

  bbox_ = rect();
}

void
CQChartsAnnotation::
drawTerm(PaintDevice *device)
{
  if (device->type() == PaintDevice::Type::SVG) {
    auto *painter = dynamic_cast<CQChartsSVGPaintDevice *>(device);

    painter->endGroup();
  }

  device->restore();
}

//------

CQChartsAnnotationGroup::
CQChartsAnnotationGroup(View *view) :
 CQChartsAnnotation(view, Type::GROUP)
{
  init();
}

CQChartsAnnotationGroup::
CQChartsAnnotationGroup(Plot *plot) :
 CQChartsAnnotation(plot, Type::GROUP)
{
  init();
}

CQChartsAnnotationGroup::
~CQChartsAnnotationGroup()
{
  for (auto &annotation : annotations_)
    annotation->setGroup(nullptr);
}

void
CQChartsAnnotationGroup::
init()
{
  setObjectName(QString("group.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

void
CQChartsAnnotationGroup::
addAnnotation(CQChartsAnnotation *annotation)
{
  if (annotation->group())
    const_cast<CQChartsAnnotationGroup *>(annotation->group())->removeAnnotation(annotation);

  annotation->setGroup(this);

  annotations_.push_back(annotation);

  needsLayout_ = true;
}

void
CQChartsAnnotationGroup::
removeAnnotation(CQChartsAnnotation *annotation)
{
  assert(annotation->group() == this);

  Annotations annotations;

  for (auto &annotation1 : annotations_) {
    if (annotation != annotation1)
      annotations.push_back(annotation1);
  }

  std::swap(annotations, annotations_);

  annotation->setGroup(nullptr);

  needsLayout_ = true;
}

//---

void
CQChartsAnnotationGroup::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "layout"       , "", "Layout children");
  addProp(model, path1, "layoutOrient" , "", "Layout orientation");
  addProp(model, path1, "layoutAlign"  , "", "Layout alignment");
  addProp(model, path1, "layoutSpacing", "", "Layout spacing");
  addProp(model, path1, "layoutMargin" , "", "Layout margin");
}

//---

bool
CQChartsAnnotationGroup::
inside(const Point &p) const
{
  for (auto &annotation : annotations_)
    if (annotation->inside(p))
      return true;

  return false;
}

//---

void
CQChartsAnnotationGroup::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  if (isLayout()) {
    setAnnotationBBox(bbox);

    layout();
  }
  else {
    double dx = bbox.getXMid() - annotationBBox_.getXMid();
    double dy = bbox.getYMid() - annotationBBox_.getYMid();

    for (auto &annotation : annotations_) {
      auto bbox1 = annotation->annotationBBox();

      bbox1.moveBy(Point(dx, dy));

      annotation->setEditBBox(bbox1, ResizeSide::MOVE);
    }
  }
}

void
CQChartsAnnotationGroup::
flip(Qt::Orientation orient)
{
  if (isLayout()) {
    // TODO
  }
  else {
    bool flipX = (orient == Qt::Horizontal);
    bool flipY = (orient == Qt::Vertical  );

    auto center = annotationBBox_.getCenter();

    for (auto &annotation : annotations_) {
      auto bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      auto center1 = bbox1.getCenter();

      Point p1 = Point((flipX ? 2*center.x - center1.x : center1.x),
                       (flipY ? 2*center.y - center1.y : center1.y));

      annotation->flip(orient);

      double dx = p1.x - bbox1.getXMid();
      double dy = p1.y - bbox1.getYMid();

      bbox1.moveBy(Point(dx, dy));

      annotation->setEditBBox(bbox1, ResizeSide::MOVE);
    }
  }
}

//---

void
CQChartsAnnotationGroup::
draw(PaintDevice *device)
{
  if (needsLayout_) {
    if (isLayout())
      layout();

    needsLayout_ = false;
  }

  //---

  for (auto &annotation : annotations_)
    annotation->draw(device);

  //---

  if (! isLayout()) {
    BBox bbox;

    for (auto &annotation : annotations_) {
      const auto &bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      bbox += bbox1;
    }

    setAnnotationBBox(bbox);
  }
}

void
CQChartsAnnotationGroup::
layout()
{
  assert(isLayout());

  //---

  // place child annotations in bbox
  double x    { 0.0 }, y    { 0.0 };
  double maxW { 0.0 }, maxH { 0.0 };
  double sumW { 0.0 }, sumH { 0.0 };

  int n = 0;

  for (auto &annotation : annotations_) {
    const auto &bbox1 = annotation->annotationBBox();
    if (! bbox1.isSet()) continue;

    if (n == 0) {
      x = bbox1.getXMin();
      y = bbox1.getYMax();
    }

    double w1 = bbox1.getWidth ();
    double h1 = bbox1.getHeight();

    maxW = std::max(maxW, w1);
    maxH = std::max(maxH, h1);

    sumW += w1;
    sumH += h1;

    ++n;
  }

  if (annotationBBox_.isSet()) {
    x = annotationBBox_.getXMin();
    y = annotationBBox_.getYMax();
  }

  // left to right
  if (layoutOrient() == Qt::Horizontal) {
    double spacing = pixelToWindowWidth(layoutSpacing());
    double margin  = pixelToWindowWidth(layoutMargin());

    //---

    double aw = sumW + 2*margin + (n - 1)*spacing;
    double ah = maxH + 2*margin;

    y -= ah;

    setAnnotationBBox(BBox(x, y, x + aw, y + ah));

    x += margin;

    for (auto &annotation : annotations_) {
      const auto &bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      double w1 = bbox1.getWidth ();
      double h1 = bbox1.getHeight();

      double y1 = y;

      if      (layoutAlign() & Qt::AlignBottom)
        y1 = y;
      else if (layoutAlign() & Qt::AlignVCenter)
        y1 = y + (ah - h1)/2;
      else if (layoutAlign() & Qt::AlignTop)
        y1 = y + ah - h1;

      annotation->setDisableSignals(true);

      annotation->setEditBBox(BBox(x, y1, x + w1, y1 + h1), CQChartsResizeSide::NONE);

      annotation->setDisableSignals(false);

      x += w1 + spacing;
    }
  }
  // top to bottom
  else {
    double spacing = pixelToWindowHeight(layoutSpacing());
    double margin  = pixelToWindowHeight(layoutMargin());

    //---

    double aw = maxW + 2*margin;
    double ah = sumH + 2*margin + (n - 1)*spacing;

    setAnnotationBBox(BBox(x, y - ah, x + aw, y));

    y -= margin;

    for (auto &annotation : annotations_) {
      const auto &bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      double w1 = bbox1.getWidth ();
      double h1 = bbox1.getHeight();

      double x1 = x;

      if      (layoutAlign() & Qt::AlignLeft)
        x1 = x;
      else if (layoutAlign() & Qt::AlignHCenter)
        x1 = x + (aw - w1)/2;
      else if (layoutAlign() & Qt::AlignRight)
        x1 = x + aw - w1;

      annotation->setDisableSignals(true);

      annotation->setEditBBox(BBox(x1, y - h1, x1 + w1, y), CQChartsResizeSide::NONE);

      annotation->setDisableSignals(false);

      y -= h1 + spacing;
    }
  }
}

void
CQChartsAnnotationGroup::
writeDetails(std::ostream &os, const QString &parentVarName, const QString &varName) const
{
  for (auto &annotation : annotations_)
    annotation->write(os, parentVarName, varName);
}

//------

CQChartsPolyShapeAnnotation::
CQChartsPolyShapeAnnotation(View *view, Type type, const Polygon &polygon) :
 CQChartsAnnotation(view, type), polygon_(polygon)
{
}

CQChartsPolyShapeAnnotation::
CQChartsPolyShapeAnnotation(Plot *plot, Type type, const Polygon &polygon) :
 CQChartsAnnotation(plot, type), polygon_(polygon)
{
}

void
CQChartsPolyShapeAnnotation::
setPolygon(const Polygon &polygon)
{
  CQChartsUtil::testAndSet(polygon_, polygon, [&]() { emitDataChanged(); } );
}

//------

CQChartsRectangleAnnotation::
CQChartsRectangleAnnotation(View *view, const Rect &rectangle) :
 CQChartsAnnotation(view, Type::RECT), rectangle_(rectangle)
{
  init();
}

CQChartsRectangleAnnotation::
CQChartsRectangleAnnotation(Plot *plot, const Rect &rectangle) :
 CQChartsAnnotation(plot, Type::RECT), rectangle_(rectangle)
{
  init();
}

CQChartsRectangleAnnotation::
~CQChartsRectangleAnnotation()
{
}

void
CQChartsRectangleAnnotation::
init()
{
  assert(rectangle_.isValid());

  setObjectName(QString("rectangle.%1").arg(ind()));

  setMargin(CQChartsMargin());

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

void
CQChartsRectangleAnnotation::
setRectangle(const Rect &rectangle)
{
  assert(rectangle.isValid());

  rectangle_ = rectangle;

  emitDataChanged();
}

void
CQChartsRectangleAnnotation::
setRectangle(const Position &start, const Position &end)
{
  auto rectangle = CQChartsViewPlotObj::makeRect(view(), plot(), start, end);

  setRectangle(rectangle);
}

CQChartsPosition
CQChartsRectangleAnnotation::
start() const
{
  if (! rectangle().bbox().isValid())
    return Position();

  Point p(rectangle().bbox().getXMin(), rectangle().bbox().getYMin());

  return Position(p, rectangle().units());
}

void
CQChartsRectangleAnnotation::
setStart(const Position &p)
{
  auto start = positionToParent(objRef(), p);
  auto end   = positionToParent(objRef(), this->end());

  rectangle_ = Rect(BBox(start, end), parentUnits());

  assert(rectangle_.isValid());

  emitDataChanged();
}

CQChartsPosition
CQChartsRectangleAnnotation::
end() const
{
  if (! rectangle().bbox().isValid())
    return Position();

  Point p(rectangle().bbox().getXMax(), rectangle().bbox().getYMax());

  return Position(p, rectangle().units());
}

void
CQChartsRectangleAnnotation::
setEnd(const Position &p)
{
  auto start = positionToParent(objRef(), this->start());
  auto end   = positionToParent(objRef(), p);

  rectangle_ = Rect(BBox(start, end), parentUnits());

  assert(rectangle_.isValid());

  emitDataChanged();
}

void
CQChartsRectangleAnnotation::
setShapeType(const ShapeType &s)
{
  CQChartsUtil::testAndSet(shapeType_, s, [&]() { emitDataChanged(); } );
}

void
CQChartsRectangleAnnotation::
setNumSides(int n)
{
  CQChartsUtil::testAndSet(numSides_, n, [&]() { emitDataChanged(); } );
}

void
CQChartsRectangleAnnotation::
setAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(angle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsRectangleAnnotation::
setLineWidth(const Length &l)
{
  CQChartsUtil::testAndSet(lineWidth_, l, [&]() { emitDataChanged(); } );
}

void
CQChartsRectangleAnnotation::
setSymbolType(const Symbol &t)
{
  CQChartsUtil::testAndSet(symbolType_, t, [&]() { emitDataChanged(); } );
}

void
CQChartsRectangleAnnotation::
setSymbolSize(const Length &s)
{
  CQChartsUtil::testAndSet(symbolSize_, s, [&]() { emitDataChanged(); } );
}

//---

bool
CQChartsRectangleAnnotation::
intersectShape(const Point &p1, const Point &p2, Point &pi) const
{
  auto rect = this->rect();

  if (shapeType() == ShapeType::CIRCLE || shapeType() == ShapeType::DOUBLE_CIRCLE) {
    auto a = pointAngle(p1, p2);

    double c = std::cos(a);
    double s = std::sin(a);

    pi = Point(rect.getXMid() + rect.getWidth ()*c/2.0,
               rect.getYMid() + rect.getHeight()*s/2.0);

    return true;
  }
  else if (shapeType() == ShapeType::POLYGON) {
    // get polygon path
    QPainterPath path;

    CQChartsDrawUtil::polygonSidesPath(path, rect, numSides() > 2 ? numSides() : 4, angle());

    auto points = CQChartsPath::pathPoints(path);

    //---

    // find closest point
    int n = points.size();

    int    ind = -1;
    double d   = 0.0;

    for (int i = 0; i < n; ++i) {
      double d1 = CQChartsUtil::PointPointDistance(p2, points[i]);

      if (ind < 0 || d1 < d) {
        ind = i;
        d   = d1;
      }
    }

    pi = points[ind];

    return true;
  }

  return false;
}

//---

void
CQChartsRectangleAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "rectangle" , "", "Rectangle bounding box");
  addProp(model, path1, "start"     , "", "Rectangle bottom left", true);
  addProp(model, path1, "end"       , "", "Rectangle top right", true);
  addProp(model, path1, "margin"    , "", "Rectangle inner margin", true);
  addProp(model, path1, "objRef"    , "", "Object reference");
  addProp(model, path1, "shapeType" , "", "Node shape type");
  addProp(model, path1, "numSides"  , "", "Number of Shape Sides");
  addProp(model, path1, "angle"     , "", "Shape angle");
  addProp(model, path1, "lineWidth" , "", "Dot line width");
  addProp(model, path1, "symbolType", "", "Dot line symbol type");
  addProp(model, path1, "symbolSize", "", "Dot line symbol size");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsRectangleAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto start = positionToParent(objRef(), this->start());
  auto end   = positionToParent(objRef(), this->end  ());

  double x1 = bbox.getXMin(), y1 = bbox.getYMin();
  double x2 = bbox.getXMax(), y2 = bbox.getYMax();

  // external margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  x1 -= xlm; y1 -= ybm;
  x2 += xrm; y2 += ytm;

  start = Point(std::min(x1, x2), std::min(y1, y2));
  end   = Point(std::max(x1, x2), std::max(y1, y2));

  rectangle_ = Rect(BBox(start, end), parentUnits());

  assert(rectangle_.isValid());

  setAnnotationBBox(bbox);
}

void
CQChartsRectangleAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // calc box
  auto start = positionToParent(objRef(), this->start());
  auto end   = positionToParent(objRef(), this->end  ());

  // external margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double x1 = std::min(start.x, end.x);
  double y1 = std::min(start.y, end.y);
  double x2 = std::max(start.x, end.x);
  double y2 = std::max(start.y, end.y);

  double x = x1 + xlm; // left
  double y = y1 + ybm; // bottom
  double w = (x2 - x1) - xlm - xrm;
  double h = (y2 - y1) - ytm - ybm;

  setAnnotationBBox(BBox(x, y, x + w, y + h));

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // draw box
  auto rect = annotationBBox();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if      (shapeType() == ShapeType::TRIANGLE)
    device->drawPolygonSides(rect, 3, angle());
  else if (shapeType() == ShapeType::DIAMOND)
    device->drawDiamond(rect);
  else if (shapeType() == ShapeType::BOX)
    device->drawRect(rect);
  else if (shapeType() == ShapeType::POLYGON)
    device->drawPolygonSides(rect, numSides() > 2 ? numSides() : 4, angle());
  else if (shapeType() == ShapeType::CIRCLE)
    device->drawEllipse(rect);
  else if (shapeType() == ShapeType::DOUBLE_CIRCLE) {
    double dx = rect.getWidth ()/10.0;
    double dy = rect.getHeight()/10.0;

    auto rect1 = rect.expanded(dx, dy, -dx, -dy);

    device->drawEllipse(rect );
    device->drawEllipse(rect1);
  }
  else if (shapeType() == ShapeType::DOT_LINE) {
    bool horizontal = CMathUtil::realEq(std::abs(angle().degrees()), 90.0);

    CQChartsDrawUtil::drawDotLine(device, penBrush, rect, lineWidth(), horizontal,
                                  symbolType(), symbolSize());
  }
  else {
    CQChartsDrawUtil::drawRoundedRect(device, rect, cornerSize(), borderSides());
  }

  //---

  drawTerm(device);
}

void
CQChartsRectangleAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
#if 0
  if (start().isSet())
    os << " -start {" << start().toString().toStdString() << "}";

  if (end().isSet())
    os << " -end {" << end().toString().toStdString() << "}";
#endif

  if (rectangle().isSet())
    os << " -rectangle {" << rectangle().toString().toStdString() << "}";

#if 0
  if (margin().isValid())
    os << " -margin " << margin().toString();
#endif

#if 0
  if (cornerSize().isSet())
    os << " -corner_size " << cornerSize();

  if (! borderSides().isAll())
    os << " -border_sides " << borderSides().toString().toStdString();
#endif

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsEllipseAnnotation::
CQChartsEllipseAnnotation(View *view, const Position &center, const Length &xRadius,
                          const Length &yRadius) :
 CQChartsAnnotation(view, Type::ELLIPSE), center_(center), xRadius_(xRadius), yRadius_(yRadius)
{
  init();
}

CQChartsEllipseAnnotation::
CQChartsEllipseAnnotation(Plot *plot, const Position &center, const Length &xRadius,
                          const Length &yRadius) :
 CQChartsAnnotation(plot, Type::ELLIPSE), center_(center), xRadius_(xRadius), yRadius_(yRadius)
{
  init();
}

CQChartsEllipseAnnotation::
~CQChartsEllipseAnnotation()
{
}

void
CQChartsEllipseAnnotation::
setCenter(const Position &c)
{
  CQChartsUtil::testAndSet(center_, c, [&]() { emitDataChanged(); } );
}

void
CQChartsEllipseAnnotation::
init()
{
  assert(xRadius_.value() > 0.0 && yRadius_.value() > 0.0);

  setObjectName(QString("ellipse.%1").arg(ind()));

  setStroked(true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

void
CQChartsEllipseAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "center" , "", "Ellipse center point");
  addProp(model, path1, "xRadius", "", "Ellipse x radius");
  addProp(model, path1, "yRadius", "", "Ellipse y radius");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsEllipseAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  center_ = Position(bbox.getCenter(), Position::Units::PLOT);

  double w = bbox.getWidth ();
  double h = bbox.getHeight();

  assert(w > 0.0 && h > 0.0);

  xRadius_ = Length(w/2, parentUnits());
  yRadius_ = Length(h/2, parentUnits());

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsEllipseAnnotation::
inside(const Point &p) const
{
  auto c = positionToParent(objRef(), center());

  double dx = p.getX() - c.x;
  double dy = p.getY() - c.y;

  double xr = lengthParentWidth (xRadius());
  double yr = lengthParentHeight(yRadius());

  double xr2 = xr*xr;
  double yr2 = yr*yr;

  return (((dx*dx)/xr2 + (dy*dy)/yr2) < 1);
}

void
CQChartsEllipseAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  auto c = positionToParent(objRef(), center());

  double xr = lengthParentWidth (xRadius());
  double yr = lengthParentHeight(yRadius());

  double x1 = c.x - xr;
  double y1 = c.y - yr;
  double x2 = c.x + xr;
  double y2 = c.y + yr;

  setAnnotationBBox(BBox(x1, y1, x2, y2));

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // create path
  QPainterPath path;

  path.addEllipse(rect_.qrect());

  //---

  // draw path
  device->fillPath  (path, penBrush.brush);
  device->strokePath(path, penBrush.pen  );

  //---

  drawTerm(device);
}

void
CQChartsEllipseAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (center().isSet())
    os << " -center {" << center().toString().toStdString() << "}";

  if (xRadius().isSet())
    os << " -rx {" << xRadius().toString().toStdString() << "}";

  if (yRadius().isSet())
    os << " -ry {" << yRadius().toString().toStdString() << "}";

#if 0
  if (cornerSize().isSet())
    os << " -corner_size " << cornerSize();

  if (! borderSides().isAll())
    os << " -border_sides " << borderSides().toString().toStdString();
#endif

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPolygonAnnotation::
CQChartsPolygonAnnotation(View *view, const Polygon &polygon) :
 CQChartsPolyShapeAnnotation(view, Type::POLYGON, polygon)
{
  init();
}

CQChartsPolygonAnnotation::
CQChartsPolygonAnnotation(Plot *plot, const Polygon &polygon) :
 CQChartsPolyShapeAnnotation(plot, Type::POLYGON, polygon)
{
  init();
}

CQChartsPolygonAnnotation::
~CQChartsPolygonAnnotation()
{
}

void
CQChartsPolygonAnnotation::
init()
{
  assert(polygon_.isValid(/*closed*/true));

  setObjectName(QString("polygon.%1").arg(ind()));

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

void
CQChartsPolygonAnnotation::
setRoundedLines(bool b)
{
  CQChartsUtil::testAndSet(roundedLines_, b, [&]() { invalidate(); } );
}

void
CQChartsPolygonAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "polygon"     , ""       , "Polygon points");
  addProp(model, path1, "roundedLines", "rounded", "Smooth lines");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPolygonAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  double dx = bbox.getXMin() - rect_.getXMin();
  double dy = bbox.getYMin() - rect_.getYMin();
  double sx = (rect_.getWidth () > 0 ? bbox.getWidth ()/rect_.getWidth () : 1.0);
  double sy = (rect_.getHeight() > 0 ? bbox.getHeight()/rect_.getHeight() : 1.0);

  double x1 = rect_.getXMin();
  double y1 = rect_.getYMin();

  auto poly = polygon_.polygon();

  for (int i = 0; i < poly.size(); ++i) {
    double x = sx*(poly.point(i).x - x1) + x1 + dx;
    double y = sy*(poly.point(i).y - y1) + y1 + dy;

    poly.setPoint(i, Point(x, y));
  }

  Polygon polygon(poly);

  if (polygon.isValid(/*closed*/true)) {
    polygon_ = polygon;
    rect_    = bbox;
  }
}

void
CQChartsPolygonAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  int i = data.toInt(&ok);

  int np = polygon_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  auto p = polygon_.point(i);

  polygon_.setPoint(i, Point(p.x + dx, p.y + dy));
}

//---

bool
CQChartsPolygonAnnotation::
inside(const Point &p) const
{
  const auto &polygon = polygon_.polygon();

  return (polygon.containsPoint(p, Qt::OddEvenFill));
}

//---

void
CQChartsPolygonAnnotation::
draw(PaintDevice *device)
{
  const auto &polygon = polygon_.polygon();
  if (! polygon.size()) return;

  //---

  drawInit(device);

  //---

  // calc bbox
  double x1 = polygon.point(0).x;
  double y1 = polygon.point(0).y;
  double x2 = x1;
  double y2 = y1;

  for (int i = 1; i < polygon.size(); ++i) {
    x1 = std::min(x1, polygon.point(i).x);
    y1 = std::min(y1, polygon.point(i).y);
    x2 = std::max(x2, polygon.point(i).x);
    y2 = std::max(y2, polygon.point(i).y);
  }

  setAnnotationBBox(BBox(x1, y1, x2, y2));

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // create path
  QPainterPath path;

  if (isRoundedLines()) {
    initSmooth();

    // draw path
    path = smooth_->createPath(/*closed*/true);
  }
  else {
    path = CQChartsDrawUtil::polygonToPath(polygon, /*closed*/true);
  }

  //---

  // draw filled path
  device->fillPath  (path, penBrush.brush);
  device->strokePath(path, penBrush.pen  );

  //---

  drawTerm(device);
}

//--

CQChartsEditHandles *
CQChartsPolygonAnnotation::
editHandles() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  const auto &extraHandles = handles->extraHandles();

  int np = polygon_.numPoints();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = (handles->view() ?
      new CQChartsResizeHandle(handles->view(), CQChartsResizeSide::EXTRA) :
      new CQChartsResizeHandle(handles->plot(), CQChartsResizeSide::EXTRA));

    handles->addExtraHandle(extraHandle);
  }

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(QVariant(i));

    auto p = polygon_.point(i);

    extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

    ++i;
  }

  return handles;
}

//--

void
CQChartsPolygonAnnotation::
initSmooth() const
{
  // init smooth if needed
  if (! smooth_) {
    auto *th = const_cast<CQChartsPolygonAnnotation *>(this);

    const auto &polygon = polygon_.polygon();

    th->smooth_ = std::make_unique<Smooth>(polygon, /*sorted*/false);
  }
}

void
CQChartsPolygonAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  writePoints(os, polygon_.polygon());

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPolylineAnnotation::
CQChartsPolylineAnnotation(View *view, const Polygon &polygon) :
 CQChartsPolyShapeAnnotation(view, Type::POLYLINE, polygon)
{
  init();
}

CQChartsPolylineAnnotation::
CQChartsPolylineAnnotation(Plot *plot, const Polygon &polygon) :
 CQChartsPolyShapeAnnotation(plot, Type::POLYLINE, polygon)
{
  init();
}

CQChartsPolylineAnnotation::
~CQChartsPolylineAnnotation()
{
}

void
CQChartsPolylineAnnotation::
init()
{
  assert(polygon_.isValid(/*closed*/false));

  setObjectName(QString("polyline.%1").arg(ind()));

  setStroked(true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

void
CQChartsPolylineAnnotation::
setRoundedLines(bool b)
{
  CQChartsUtil::testAndSet(roundedLines_, b, [&]() { invalidate(); } );
}

void
CQChartsPolylineAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "polygon"     , ""       , "Polyline points");
  addProp(model, path1, "roundedLines", "rounded", "Smooth lines");

  addStrokeProperties(model, path1 + "/stroke", /*isSolid*/false);
}

//---

void
CQChartsPolylineAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  double dx = bbox.getXMin() - rect_.getXMin();
  double dy = bbox.getYMin() - rect_.getYMin();
  double sx = (rect_.getWidth () > 0 ? bbox.getWidth ()/rect_.getWidth () : 1.0);
  double sy = (rect_.getHeight() > 0 ? bbox.getHeight()/rect_.getHeight() : 1.0);

  double x1 = rect_.getXMin();
  double y1 = rect_.getYMin();

  auto poly = polygon_.polygon();

  for (int i = 0; i < poly.size(); ++i) {
    double x = sx*(poly.point(i).x - x1) + x1 + dx;
    double y = sy*(poly.point(i).y - y1) + y1 + dy;

    poly.setPoint(i, Point(x, y));
  }

  Polygon polygon(poly);

  if (polygon.isValid(/*closed*/false)) {
    polygon_ = polygon;
    rect_    = bbox;
  }
}

void
CQChartsPolylineAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  int i = data.toInt(&ok);

  int np = polygon_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  auto p = polygon_.point(i);

  polygon_.setPoint(i, Point(p.x + dx, p.y + dy));
}

//---

bool
CQChartsPolylineAnnotation::
inside(const Point &p) const
{
  const auto &polygon = polygon_.polygon();

  if (! polygon.size())
    return false;

  auto pp = windowToPixel(p);

  auto pl1 = windowToPixel(polygon.point(0));

  for (int i = 1; i < polygon.size(); ++i) {
    auto pl2 = windowToPixel(polygon.point(i));

    double d;

    if (CQChartsUtil::PointLineDistance(pp, pl1, pl2, &d) && d < 3)
      return true;

    pl1 = pl2;
  }

  return false;
}

//---

void
CQChartsPolylineAnnotation::
draw(PaintDevice *device)
{
  const auto &polygon = polygon_.polygon();
  if (! polygon.size()) return;

  //---

  drawInit(device);

  //---

  // calc bbox
  double x1 = polygon.point(0).x;
  double y1 = polygon.point(0).y;
  double x2 = x1;
  double y2 = y1;

  for (int i = 1; i < polygon.size(); ++i) {
    x1 = std::min(x1, polygon.point(i).x);
    y1 = std::min(y1, polygon.point(i).y);
    x2 = std::max(x2, polygon.point(i).x);
    y2 = std::max(y2, polygon.point(i).y);
  }

  setAnnotationBBox(BBox(x1, y1, x2, y2));

  //---

  // set pen
  PenBrush penBrush;

  auto strokeColor = interpStrokeColor(ColorInd());

  setPen(penBrush,
    PenData(true, strokeColor, strokeAlpha(), strokeWidth(), strokeDash(),
            strokeCap(), strokeJoin()));

  updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  //---

  // create path
  QPainterPath path;

  if (isRoundedLines()) {
    initSmooth();

    // draw path
    path = smooth_->createPath(/*closed*/false);
  }
  else {
    path = CQChartsDrawUtil::polygonToPath(polygon, /*closed*/false);
  }

  //---

  // draw path
  device->strokePath(path, penBrush.pen);

  //---

  drawTerm(device);
}

//--

CQChartsEditHandles *
CQChartsPolylineAnnotation::
editHandles() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  const auto &extraHandles = handles->extraHandles();

  int np = polygon_.numPoints();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = (handles->view() ?
      new CQChartsResizeHandle(handles->view(), CQChartsResizeSide::EXTRA) :
      new CQChartsResizeHandle(handles->plot(), CQChartsResizeSide::EXTRA));

    handles->addExtraHandle(extraHandle);
  }

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(QVariant(i));

    auto p = polygon_.point(i);

    extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

    ++i;
  }

  return handles;
}

//--

void
CQChartsPolylineAnnotation::
initSmooth() const
{
  // init smooth if needed
  if (! smooth_) {
    auto *th = const_cast<CQChartsPolylineAnnotation *>(this);

    const auto &polygon = polygon_.polygon();

    th->smooth_ = std::make_unique<Smooth>(polygon, /*sorted*/false);
  }
}

//--

void
CQChartsPolylineAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  writePoints(os, polygon_.polygon());

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsTextAnnotation::
CQChartsTextAnnotation(View *view, const Position &position, const QString &textStr) :
 CQChartsAnnotation(view, Type::TEXT)
{
  setPosition(position);

  init(textStr);
}

CQChartsTextAnnotation::
CQChartsTextAnnotation(Plot *plot, const Position &position, const QString &textStr) :
 CQChartsAnnotation(plot, Type::TEXT)
{
  setPosition(position);

  init(textStr);
}

CQChartsTextAnnotation::
CQChartsTextAnnotation(View *view, const Rect &rect, const QString &textStr) :
 CQChartsAnnotation(view, Type::TEXT)
{
  setRectangle(rect);

  init(textStr);
}

CQChartsTextAnnotation::
CQChartsTextAnnotation(Plot *plot, const Rect &rect, const QString &textStr) :
 CQChartsAnnotation(plot, Type::TEXT)
{
  setRectangle(rect);

  init(textStr);
}

CQChartsTextAnnotation::
~CQChartsTextAnnotation()
{
}

void
CQChartsTextAnnotation::
init(const QString &textStr)
{
  setObjectName(QString("text.%1").arg(ind()));

  setTextStr  (textStr);
  setTextColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 1.0));

  setStroked(false);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

CQChartsPosition
CQChartsTextAnnotation::
positionValue() const
{
  return position_.positionOr(Position());
}

void
CQChartsTextAnnotation::
setPosition(const Position &p)
{
  setPosition(OptPosition(p));
}

void
CQChartsTextAnnotation::
setPosition(const OptPosition &p)
{
  rectangle_ = OptRect();
  position_  = p;

  positionToBBox();

  emitDataChanged();
}

CQChartsRect
CQChartsTextAnnotation::
rectangleValue() const
{
  return rectangle().rectOr(Rect());
}

void
CQChartsTextAnnotation::
setRectangle(const Rect &r)
{
  setRectangle(OptRect(r));
}

void
CQChartsTextAnnotation::
setRectangle(const OptRect &r)
{
  if (r.isSet())
    assert(r.rect().isValid());

  position_  = OptPosition();
  rectangle_ = r;

  rectToBBox();

  emitDataChanged();
}

void
CQChartsTextAnnotation::
rectToBBox()
{
  if (rectangle().isSet()) {
    auto rect = this->rectangleValue();

    if (plot())
      setAnnotationBBox(plot()->rectToPlot(rect));
    else
      setAnnotationBBox(view()->rectToView(rect));
  }
  else
    setAnnotationBBox(BBox());
}

void
CQChartsTextAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position" , "", "Text origin");
  addProp(model, path1, "rectangle", "", "Text bounding box");
  addProp(model, path1, "objRef"   , "", "Object reference");

  auto textPath = path1 + "/text";

  addProp(model, textPath, "textStr", "string", "Text string");

  addTextProperties(model, textPath, CQChartsTextOptions::ValueType::ALL);

  addProp(model, path1, "padding", "", "Text rectangle inner padding");
  addProp(model, path1, "margin" , "", "Text rectangle outer margin");

  addStrokeFillProperties(model, path1);
}

void
CQChartsTextAnnotation::
calcTextSize(Size &psize, Size &wsize) const
{
  // get font
  auto font = calcFont(textFont());

  // get text size (pixel)
  CQChartsTextOptions textOptions;

  textOptions.html = isTextHtml();

  psize = CQChartsDrawUtil::calcTextSize(textStr(), font, textOptions);

  // convert to window size
  if      (plot()) wsize = plot()->pixelToWindowSize(psize);
  else if (view()) wsize = view()->pixelToWindowSize(psize);
  else             wsize = psize;
}

void
CQChartsTextAnnotation::
positionToLL(double w, double h, double &x, double &y) const
{
  auto p = positionToParent(objRef(), positionValue());

  x = 0.0;
  y = 0.0;

  if      (textAlign() & Qt::AlignLeft)
    x = p.x;
  else if (textAlign() & Qt::AlignRight)
    x = p.x - w;
  else
    x = p.x - w/2;

  if      (textAlign() & Qt::AlignTop)
    y = p.y;
  else if (textAlign() & Qt::AlignBottom)
    y = p.y - h;
  else
    y = p.y - h/2;
}

//---

void
CQChartsTextAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  if (rectangle().isSet()) {
    auto rect = Rect(bbox, parentUnits());

    setRectangle(rect);
  }
  else {
    // get inner padding
    double xlp, xrp, ytp, ybp;

    getPaddingValues(xlp, xrp, ytp, ybp);

    // get outer margin
    double xlm, xrm, ytm, ybm;

    getMarginValues(xlm, xrm, ytm, ybm);

    //---

    // get position from align
    double x1, y1;

    if      (textAlign() & Qt::AlignLeft)
      x1 = bbox.getXMin() + xlp + xlm;
    else if (textAlign() & Qt::AlignRight)
      x1 = bbox.getXMax() - xrp - xrm;
    else
      x1 = bbox.getXMid();

    if      (textAlign() & Qt::AlignBottom)
      y1 = bbox.getYMin() + ybp + ybm;
    else if (textAlign() & Qt::AlignTop)
      y1 = bbox.getYMax() - ytp - ytm;
    else
      y1 = bbox.getYMid();

    Point ll(x1, y1);

    //double x2 = x1 + bbox.getWidth () - xlp - xrp - xlm - xrm;
    //double y2 = y1 + bbox.getHeight() - ybp - ytp - ybm - xtm;

    //Point ur(x2, y2);

    //---

    auto position = Position(ll, parentUnits());

    setPosition(position);
  }

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsTextAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsTextAnnotation::
draw(PaintDevice *device)
{
  // recalculate position to bbox on draw as can change depending on pixel mapping
  if (! rectangle().isSet())
    positionToBBox();

  if (! rect_.isValid())
    return;

  //---

  drawInit(device);

  //---

  // set rect pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  if (isEnabled())
    updatePenBrushState(penBrush, CQChartsObjDrawType::TEXT);

  //---

  // draw box
  CQChartsDrawUtil::drawRoundedRect(device, penBrush, rect_, cornerSize(), borderSides());

  //---

  // set text pen and brush
  auto c = interpColor(textColor(), ColorInd());

  if (isEnabled()) {
    if (isCheckable() && ! isChecked()) {
      double f = uncheckedLighter();

      c = CQChartsUtil::blendColors(backgroundColor(), c, f);
    }
  }
  else {
    double f = disabledLighter();

    c = CQChartsUtil::blendColors(backgroundColor(), c, f);
  }

  setPenBrush(penBrush, PenData(true, c, textAlpha()), BrushData(false));

  if (isEnabled())
    updatePenBrushState(penBrush, CQChartsObjDrawType::TEXT);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // set text options
  CQChartsTextOptions textOptions;

  textOptions.angle         = textAngle();
  textOptions.align         = textAlign();
  textOptions.contrast      = isTextContrast();
  textOptions.contrastAlpha = textContrastAlpha();
  textOptions.formatted     = isTextFormatted();
  textOptions.scaled        = isTextScaled();
  textOptions.html          = isTextHtml();
  textOptions.clipLength    = lengthPixelWidth(textClipLength());
  textOptions.clipElide     = textClipElide();
  textOptions.clipped       = false;

  adjustTextOptions(textOptions);

  //---

  // set font
  setPainterFont(device, textFont());

  //---

  // set box
//auto pbbox = windowToPixel(rect_);

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double tx =          rect_.getXMin  () +       xlm + xlp;
  double ty =          rect_.getYMin  () +       ybm + ybp;
  double tw = std::max(rect_.getWidth () - xlm - xrm - xlp - xrp, 0.0);
  double th = std::max(rect_.getHeight() - ybm - ytm - ybp - ytp, 0.0);

  BBox tbbox(tx, ty, tx + tw, ty + th);

  //---

  // draw text
  if (tbbox.isValid()) {
    device->setRenderHints(QPainter::Antialiasing);

    CQChartsDrawUtil::drawTextInBox(device, tbbox, textStr(), textOptions);
  }

  //---

  drawTerm(device);
}

void
CQChartsTextAnnotation::
initRectangle()
{
  // convert position to rectangle if needed
  if (! rectangle().isSet()) {
    positionToBBox();

    //---

    auto rect = Rect(rect_, parentUnits());

    setRectangle(rect);
  }
}

void
CQChartsTextAnnotation::
positionToBBox()
{
  assert(! rectangle().isSet());

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  Size psize, wsize;

  calcTextSize(psize, wsize);

  double x, y;

  positionToLL(wsize.width(), wsize.height(), x, y);

  Point ll(x                 - xlp - xlm, y                  - ybp - ybm);
  Point ur(x + wsize.width() + xrp + xrm, y + wsize.height() + ytp + ytm);

  setAnnotationBBox(BBox(ll, ur));
}

void
CQChartsTextAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (rectangle().isSet()) {
    if (rectangleValue().isSet())
      os << " -rectangle {" << rectangleValue().toString().toStdString() << "}";
  }
  else {
    if (positionValue().isSet())
      os << " -position {" << positionValue().toString().toStdString() << "}";
  }

  if (textStr().length())
    os << " -text {" << textStr().toStdString() << "}";

  if (textFont() != view()->font())
    os << " -font {" << textFont().toString().toStdString() << "}";

  if (textColor().isValid())
    os << " -color {" << textColor().toString().toStdString() << "}";

  if (textAlpha() != Alpha())
    os << " -alpha " << textAlpha().value();

  if (textAngle() != Angle())
    os << " -angle " << textAngle().value();

  if (isTextContrast())
    os << " -contrast 1";

  if (textAlign() != (Qt::AlignLeft | Qt::AlignVCenter))
    os << " -align {" << CQUtil::alignToString(textAlign()).toStdString() << "}";

  if (isTextHtml())
    os << " -html";

#if 0
  if (cornerSize().isSet())
    os << " -corner_size " << cornerSize();

  if (! borderSides().isAll())
    os << " -border_sides " << borderSides().toString().toStdString();
#endif

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsImageAnnotation::
CQChartsImageAnnotation(View *view, const Position &position, const Image &image) :
 CQChartsAnnotation(view, Type::IMAGE), image_(image)
{
  setPosition(position);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(Plot *plot, const Position &position, const Image &image) :
 CQChartsAnnotation(plot, Type::IMAGE), image_(image)
{
  setPosition(position);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(View *view, const Rect &rect, const Image &image) :
 CQChartsAnnotation(view, Type::IMAGE), image_(image)
{
  setRectangle(rect);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(Plot *plot, const Rect &rect, const Image &image) :
 CQChartsAnnotation(plot, Type::IMAGE), image_(image)
{
  setRectangle(rect);

  init();
}

CQChartsImageAnnotation::
~CQChartsImageAnnotation()
{
}

void
CQChartsImageAnnotation::
init()
{
  setObjectName(QString("image.%1").arg(ind()));

  disabledImage_ = Image();

  image_        .setId(objectName());
  disabledImage_.setId(objectName() + "_dis");

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

CQChartsPosition
CQChartsImageAnnotation::
positionValue() const
{
  return position_.positionOr(Position());
}

void
CQChartsImageAnnotation::
setPosition(const Position &p)
{
  setPosition(OptPosition(p));
}

void
CQChartsImageAnnotation::
setPosition(const OptPosition &p)
{
  rectangle_ = OptRect();
  position_  = p;

  positionToBBox();

  emitDataChanged();
}

CQChartsRect
CQChartsImageAnnotation::
rectangleValue() const
{
  return rectangle().rectOr(Rect());
}

void
CQChartsImageAnnotation::
setRectangle(const Rect &r)
{
  setRectangle(OptRect(r));
}

void
CQChartsImageAnnotation::
setRectangle(const OptRect &r)
{
  if (r.isSet())
    assert(r.rect().isValid());

  position_  = OptPosition();
  rectangle_ = r;

  rectToBBox();

  if (disabledImageType_ != DisabledImageType::FIXED)
    disabledImageType_ = DisabledImageType::NONE; // invalidate disabled image

  emitDataChanged();
}

void
CQChartsImageAnnotation::
rectToBBox()
{
  if (rectangle().isSet()) {
    auto rect = this->rectangleValue();

    if (plot())
      setAnnotationBBox(plot()->rectToPlot(rect));
    else
      setAnnotationBBox(view()->rectToView(rect));
  }
  else
    setAnnotationBBox(BBox());
}

void
CQChartsImageAnnotation::
setImage(const Image &image)
{
  image_ = image;

  if (disabledImageType_ != DisabledImageType::FIXED)
    disabledImageType_ = DisabledImageType::NONE; // invalidate disabled image

  emitDataChanged();
}

void
CQChartsImageAnnotation::
setDisabledImage(const Image &image)
{
  disabledImage_ = image;

  if (disabledImage_.isValid())
    disabledImageType_ = DisabledImageType::FIXED;
  else
    disabledImageType_ = DisabledImageType::NONE;

  emitDataChanged();
}

void
CQChartsImageAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position"     , "position"     , "Image origin");
  addProp(model, path1, "rectangle"    , "rectangle"    , "Image bounding box");
  addProp(model, path1, "image"        , "image"        , "Image name");
  addProp(model, path1, "disabledImage", "disabledImage", "Disabled image name");

  addProp(model, path1, "padding", "", "Image rectangle inner padding");
  addProp(model, path1, "margin" , "", "Image rectangle outer margin");

  addStrokeFillProperties(model, path1);
}

void
CQChartsImageAnnotation::
calcImageSize(Size &psize, Size &wsize) const
{
  // convert to window size
  psize = Size(image_.size());

  if      (plot()) wsize = plot()->pixelToWindowSize(psize);
  else if (view()) wsize = view()->pixelToWindowSize(psize);
  else             wsize = psize;
}

void
CQChartsImageAnnotation::
positionToLL(double w, double h, double &x, double &y) const
{
  auto p = positionToParent(objRef(), positionValue());

  x = p.x - w/2;
  y = p.y - h/2;
}

//---

void
CQChartsImageAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  if (rectangle().isSet()) {
    auto rect = Rect(bbox, parentUnits());

    setRectangle(rect);
  }
  else {
    // get position
    double x1 = bbox.getXMid();
    double y1 = bbox.getYMid();

    Point ll(x1, y1);

    //---

    auto position = Position(ll, parentUnits());

    setPosition(position);
  }

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsImageAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsImageAnnotation::
draw(PaintDevice *device)
{
  // recalculate position to bbox on draw as can change depending on pixel mapping
  if (! rectangle().isSet())
    positionToBBox();

  if (! rect_.isValid())
    return;

  //---

  drawInit(device);

  //---

  // set rect pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::IMAGE);

  //---

  // draw box
  CQChartsDrawUtil::drawRoundedRect(device, penBrush, rect_, cornerSize(), borderSides());

  //---

  // set box
//auto pbbox = windowToPixel(rect_);

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double tx =          rect_.getXMin  () +       xlm + xlp;
  double ty =          rect_.getYMin  () +       ybm + ybp;
  double tw = std::max(rect_.getWidth () - xlm - xrm - xlp - xrp, 0.0);
  double th = std::max(rect_.getHeight() - ybm - ytm - ybp - ytp, 0.0);

  BBox tbbox(tx, ty, tx + tw, ty + th);

  //---

  // draw image
  if (! isEnabled()) {
    updateDisabledImage(DisabledImageType::DISABLED);

    device->drawImageInRect(tbbox, disabledImage_);
  }
  else {
    if (isCheckable() && ! isChecked()) {
      updateDisabledImage(DisabledImageType::UNCHECKED);

      device->drawImageInRect(tbbox, disabledImage_);
    }
    else {
      device->drawImageInRect(tbbox, image_);
    }
  }

  //---

  drawTerm(device);
}

void
CQChartsImageAnnotation::
updateDisabledImage(const DisabledImageType &type)
{
  // fixed
  if (disabledImageType_ == DisabledImageType::FIXED)
    return;

  //---

  // auto calc
  if (! disabledImage_.isValid() || disabledImageType_ != type) {
    double f = 1.0;

    if (type == DisabledImageType::UNCHECKED)
      f = uncheckedLighter();
    else
      f = disabledLighter();

    auto bg = backgroundColor();

    auto prect = windowToPixel(rect_);

    const auto &image = image_.sizedImage(prect.getWidth(), prect.getHeight());

    int iw = image.width ();
    int ih = image.height();

    auto disabledImage = CQChartsUtil::initImage(QSize(iw, ih));

    for (int y = 0; y < ih; ++y) {
      for (int x = 0; x < iw; ++x) {
        auto rgba = image.pixel(x, y);

        int r = qRed  (rgba);
        int g = qGreen(rgba);
        int b = qBlue (rgba);
        int a = qAlpha(rgba);

        QColor c(r, g, b, a);

        auto c1 = CQChartsUtil::blendColors(bg, c, f);

      //c1.setAlpha(a);

        disabledImage.setPixel(x, y, c1.rgba());
      }
    }

    disabledImage_     = Image(disabledImage);
    disabledImageType_ = type;
  }
}

void
CQChartsImageAnnotation::
initRectangle()
{
  // convert position to rectangle if needed
  if (! rectangle().isSet()) {
    positionToBBox();

    //---

    auto rect = Rect(rect_, parentUnits());

    setRectangle(rect);
  }
}

void
CQChartsImageAnnotation::
positionToBBox()
{
  assert(! rectangle().isSet());

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  Size psize, wsize;

  calcImageSize(psize, wsize);

  double x, y;

  positionToLL(wsize.width(), wsize.height(), x, y);

  Point ll(x                 - xlp - xlm, y                  - ybp - ybm);
  Point ur(x + wsize.width() + xrp + xrm, y + wsize.height() + ytp + ytm);

  setAnnotationBBox(BBox(ll, ur));
}

void
CQChartsImageAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (rectangle().isSet()) {
    if (rectangleValue().isSet())
      os << " -rectangle {" << rectangleValue().toString().toStdString() << "}";
  }
  else {
    if (positionValue().isSet())
      os << " -position {" << positionValue().toString().toStdString() << "}";
  }

  if (image_.isValid())
    os << " -image {" << image_.fileName().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPathAnnotation::
CQChartsPathAnnotation(View *view, const Path &path) :
 CQChartsAnnotation(view, Type::PATH), path_(path)
{
  init();
}

CQChartsPathAnnotation::
CQChartsPathAnnotation(Plot *plot, const Path &path) :
 CQChartsAnnotation(plot, Type::PATH), path_(path)
{
  init();
}

CQChartsPathAnnotation::
~CQChartsPathAnnotation()
{
}

void
CQChartsPathAnnotation::
init()
{
  setObjectName(QString("path.%1").arg(ind()));

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

void
CQChartsPathAnnotation::
setPath(const Path &path)
{
  path_ = path;

  emitDataChanged();
}

void
CQChartsPathAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "path", "path", "Path name");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPathAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  // get position
  double dx = bbox.getXMid() - annotationBBox_.getXMid();
  double dy = bbox.getYMid() - annotationBBox_.getYMid();

  path_.move(dx, dy);

  setAnnotationBBox(bbox);
}

void
CQChartsPathAnnotation::
flip(Qt::Orientation orient)
{
  if (orient == Qt::Horizontal)
    path_.flip(/*x*/true, /*y*/false);
  else
    path_.flip(/*x*/false, /*y*/true);

  emitDataChanged();
}

void
CQChartsPathAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  int i = data.toInt(&ok);

  int np = path_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  auto p = path_.pointAt(i);

  path_.setPointAt(i, Point(p.x + dx, p.y + dy));
}

//---

bool
CQChartsPathAnnotation::
inside(const Point &p) const
{
  return path_.path().contains(p.qpoint());
}

void
CQChartsPathAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // set rect pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // draw path
  CQChartsDrawUtil::setPenBrush(device, penBrush);

  device->drawPath(path_.path());

  //---

  drawTerm(device);

  //---

  auto bbox = BBox(path_.path().boundingRect());

  setAnnotationBBox(bbox);
}

//--

CQChartsEditHandles *
CQChartsPathAnnotation::
editHandles() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  const auto &extraHandles = handles->extraHandles();

  int np = path_.numPoints();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = (handles->view() ?
      new CQChartsResizeHandle(handles->view(), CQChartsResizeSide::EXTRA) :
      new CQChartsResizeHandle(handles->plot(), CQChartsResizeSide::EXTRA));

    handles->addExtraHandle(extraHandle);
  }

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(QVariant(i));

    auto p = path_.pointAt(i);

    extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

    ++i;
  }

  return handles;
}

//--

void
CQChartsPathAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (path_.isValid())
    os << " -path {" << path_.toString().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsArrowAnnotation::
CQChartsArrowAnnotation(View *view, const Position &start, const Position &end) :
 CQChartsAnnotation(view, Type::ARROW), start_(start), end_(end)
{
  init();
}

CQChartsArrowAnnotation::
CQChartsArrowAnnotation(Plot *plot, const Position &start, const Position &end) :
 CQChartsAnnotation(plot, Type::ARROW), start_(start), end_(end)
{
  init();
}

CQChartsArrowAnnotation::
~CQChartsArrowAnnotation()
{
}

void
CQChartsArrowAnnotation::
setStart(const Position &p)
{
  CQChartsUtil::testAndSet(start_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsArrowAnnotation::
setEnd(const Position &p)
{
  CQChartsUtil::testAndSet(end_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsArrowAnnotation::
init()
{
  setObjectName(QString("arrow.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  if (plot())
    arrow_ = std::make_unique<CQChartsArrow>(plot());
  else
    arrow_ = std::make_unique<CQChartsArrow>(view());

  connect(arrow(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));
}

const CQChartsArrowData &
CQChartsArrowAnnotation::
arrowData() const
{
  return arrow()->data();
}

void
CQChartsArrowAnnotation::
setArrowData(const CQChartsArrowData &data)
{
  arrow()->setData(data);

  emitDataChanged();
}

void
CQChartsArrowAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto addArrowProp = [&](const QString &path, const QString &name, const QString &alias,
                          const QString &desc, bool hidden=false) {
    auto *item = model->addProperty(path, arrow(), name, alias);
    item->setDesc(desc);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  auto addArrowStyleProp = [&](const QString &path, const QString &name, const QString &alias,
                               const QString &desc, bool hidden=false) {
    auto *item = addArrowProp(path, name, alias, desc, hidden);
    CQCharts::setItemIsStyle(item);
    return item;
  };

  //---

  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "start"      , "", "Arrow start point");
  addProp(model, path1, "startObjRef", "", "Arrow start object reference");
  addProp(model, path1, "end"        , "", "Arrow end point");
  addProp(model, path1, "endObjRef"  , "", "Arrow end object reference");

  auto linePath = path1 + "/line";

  addArrowStyleProp(linePath, "lineWidth", "width", "Arrow connecting line width");

  addArrowStyleProp(linePath, "rectilinear", "rectilinear", "Rectilinear line");

  auto frontHeadPath = path1 + "/frontHead";

  addArrowProp     (frontHeadPath, "frontVisible"  , "visible",
                    "Arrow front head visible", /*hidden*/true);
  addArrowProp     (frontHeadPath, "frontType"     , "type",
                    "Arrow front head type");
  addArrowStyleProp(frontHeadPath, "frontLength"   , "length",
                    "Arrow front head length");
  addArrowStyleProp(frontHeadPath, "frontAngle"    , "angle",
                    "Arrow front head angle");
  addArrowStyleProp(frontHeadPath, "frontBackAngle", "backAngle",
                    "Arrow front head back angle", /*hidden*/true);
  addArrowStyleProp(frontHeadPath, "frontLineEnds" , "line",
                    "Arrow front head is drawn using lines", /*hidden*/true);

  auto tailHeadPath = path1 + "/tailHead";

  addArrowProp     (tailHeadPath, "tailVisible"  , "visible",
                    "Arrow tail head visible", /*hidden*/true);
  addArrowProp     (tailHeadPath, "tailType"    , "type",
                    "Arrow tail head type");
  addArrowStyleProp(tailHeadPath, "tailLength"   , "length",
                    "Arrow tail head length");
  addArrowStyleProp(tailHeadPath, "tailAngle"    , "angle",
                    "Arrow tail head angle");
  addArrowStyleProp(tailHeadPath, "tailBackAngle", "backAngle",
                    "Arrow tail head back angle", /*hidden*/true);
  addArrowStyleProp(tailHeadPath, "tailLineEnds" , "line",
                    "Arrow tail head is drawn using lines", /*hidden*/true);

  //---

#if 0
  addStrokeFillProperties(model, path1);
#else
  auto fillPath = path1 + "/fill";

  addArrowStyleProp(fillPath, "filled"   , "visible", "Arrow fill visible");
  addArrowStyleProp(fillPath, "fillColor", "color"  , "Arrow fill color");
  addArrowStyleProp(fillPath, "fillAlpha", "alpha"  , "Arrow fill alpha");

  auto strokePath = path1 + "/stroke";

  addArrowStyleProp(strokePath, "stroked"    , "visible", "Arrow stroke visible");
  addArrowStyleProp(strokePath, "strokeColor", "color"  , "Arrow stroke color");
  addArrowStyleProp(strokePath, "strokeAlpha", "alpha"  , "Arrow stroke alpha");
  addArrowStyleProp(strokePath, "strokeWidth", "width"  , "Arrow stroke width");
#endif

#if DEBUG_LABELS
  addArrowProp(path1, "debugLabels", "debugLabels", "Show debug labels", /*hidden*/true);
#endif
}

void
CQChartsArrowAnnotation::
getPropertyNames(QStringList &names, bool hidden) const
{
  CQChartsAnnotation::getPropertyNames(names, hidden);

  names << "line.width";

  names << "frontHead.type" << "frontHead.length" << "frontHead.angle";
  names << "tailHead.type"  << "tailHead.length"  << "tailHead.angle";

  names << "fill.visible" << "fill.color" << "fill.alpha";

  names << "stroke.visible" << "stroke.color" << "stroke.alpha" << "stroke.width";

#if 0
  // can't use objectNames as root is wrong
  auto *propertyModel = this->propertyModel();
  if (! propertyModel) return;

  propertyModel->objectNames(arrow(), names);
#endif
}

//---

void
CQChartsArrowAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  double x1 = bbox.getXMin(), y1 = bbox.getYMin();
  double x2 = bbox.getXMax(), y2 = bbox.getYMax();

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  x1 += xlp + xlm; y1 += ybp + ybm;
  x2 -= xrp + xrm; y2 -= ytp + ytm;

  if (start.x > end.x) std::swap(x1, x2);
  if (start.y > end.y) std::swap(y1, y2);

  start.setX(x1); end.setX(x2);
  start.setY(y1); end.setY(y2);

  start_ = Position(start, Position::Units::PLOT);
  end_   = Position(end  , Position::Units::PLOT);

  setAnnotationBBox(bbox);
}

void
CQChartsArrowAnnotation::
flip(Qt::Orientation orient)
{
  if (start().units() != end().units())
    return;

  auto x1 = start().p().x;
  auto y1 = start().p().y;
  auto x2 = end  ().p().x;
  auto y2 = end  ().p().y;

  if (orient == Qt::Horizontal)
    std::swap(x1, x2);
  else
    std::swap(y1, y2);

  start_ = Position(Point(x1, y1), start().units());
  end_   = Position(Point(x2, y2), start().units());

  emitDataChanged();
}

//---

bool
CQChartsArrowAnnotation::
inside(const Point &p) const
{
  auto p1 = windowToPixel(p);

  return arrow()->contains(p1);
}

void
CQChartsArrowAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // calc box
  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  if (startObjRef().location() == ObjRef::Location::INTERSECT ||
      endObjRef  ().location() == ObjRef::Location::INTERSECT) {
    if (startObjRef().location() == ObjRef::Location::INTERSECT)
      start = intersectObjRef(startObjRef(), start, end);

    if (endObjRef().location() == ObjRef::Location::INTERSECT)
      end = intersectObjRef(endObjRef(), end, start);
  }

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double x1 = std::min(start.x, end.x);
  double y1 = std::min(start.y, end.y);
  double x2 = std::max(start.x, end.x);
  double y2 = std::max(start.y, end.y);

  double x = x1 - xlp - xlm; // left
  double y = y1 - ybp - ybm; // bottom
  double w = (x2 - x1) + xlp + xrp + xlm + xrm;
  double h = (y2 - y1) + ybp + ytp + ybm + ytm;

  setAnnotationBBox(BBox(x, y, x + w, y + h));

  //---

  // set pen and brush
  PenBrush penBrush;

  auto bgColor     = arrow()->interpFillColor  (ColorInd());
  auto strokeColor = arrow()->interpStrokeColor(ColorInd());

  if (isCheckable() && ! isChecked()) {
    double f = uncheckedLighter();

    bgColor     = CQChartsUtil::blendColors(backgroundColor(), bgColor    , f);
    strokeColor = CQChartsUtil::blendColors(backgroundColor(), strokeColor, f);
  }

  setPenBrush(penBrush,
    PenData  (arrow()->isStroked(), strokeColor, arrow()->strokeAlpha()),
    BrushData(arrow()->isFilled (), bgColor, arrow()->fillAlpha(), arrow()->fillPattern()));

  if (arrow()->isSolid())
    updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);
  else
    updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  //---

  // draw arrow
  {
  CQChartsWidgetUtil::AutoDisconnect
    autoDisconnect(arrow(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

  arrow()->setFrom(start);
  arrow()->setTo  (end  );

  arrow()->draw(device, penBrush);
  }

  //---

  drawTerm(device);
}

void
CQChartsArrowAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  // start/end points
  if (start().isSet())
    os << " -start {" << start().toString().toStdString() << "}";

  if (end().isSet())
    os << " -end {" << end().toString().toStdString() << "}";

  // line width
  if (arrow()->lineWidth().isSet() && arrow()->lineWidth().value() > 0)
    os << " -line_width {" << arrow()->lineWidth().toString().toStdString() << "}";

  // front head data
  QString fName;
  bool    fcustom = false;

  if (CQChartsArrowData::dataToName((CQChartsArrowData::HeadType) arrow()->frontType(),
                                    arrow()->isFrontLineEnds(), arrow()->isFrontVisible(),
                                    arrow()->frontAngle(), arrow()->frontBackAngle(), fName)) {
    os << " -fhead " << fName.toStdString();
  }
  else {
    fcustom = true;

    os << " -fhead 1";
  }

  // tail head data
  QString tName;
  bool    tcustom = false;

  if (CQChartsArrowData::dataToName((CQChartsArrowData::HeadType) arrow()->tailType(),
                                    arrow()->isTailLineEnds(), arrow()->isTailVisible(),
                                    arrow()->tailAngle(), arrow()->tailBackAngle(), tName)) {
    os << " -thead " << tName.toStdString();
  }
  else {
    tcustom = true;

    os << " -thead 1";
  }

  // add angles if custom
  if      (fcustom && tcustom) {
    if      (! arrow()->frontAngle().isZero() && ! arrow()->tailAngle().isZero())
      os << " -angle {" << arrow()->frontAngle().value() << " " <<
                           arrow()->tailAngle ().value() << "}";
    else if (! arrow()->frontAngle().isZero())
      os << " -angle {" << arrow()->frontAngle().value() << " 0.0 }";
    else if (! arrow()->tailAngle().isZero())
      os << " -angle {0.0 " << arrow()->tailAngle().value() << "}";

    if      (arrow()->frontBackAngle().value() >= 0.0 &&
             arrow()->tailBackAngle ().value() >= 0.0) // delta angle
      os << " -angle {" << arrow()->frontBackAngle().value() << " " <<
                           arrow()->tailBackAngle ().value() << "}";
    else if (arrow()->frontBackAngle().value() >= 0.0) // delta angle
      os << " -angle {" << arrow()->frontBackAngle().value() << " -1}";
    else if (arrow()->tailBackAngle ().value() >= 0.0) // delta angle
      os << " -angle {-1 " << arrow()->tailBackAngle().value() << "}";
  }
  else if (fcustom) {
    if (! arrow()->frontAngle().isZero())
      os << " -angle {" << arrow()->frontAngle().value() << " 0.0 }";

    if (arrow()->frontBackAngle().value() >= 0.0) // delta angle
      os << " -angle {" << arrow()->frontBackAngle().value() << " -1}";
  }
  else if (tcustom) {
    if (! arrow()->tailAngle().isZero())
      os << " -angle {0.0 " << arrow()->tailAngle().value() << "}";

    if (arrow()->tailBackAngle().value() >= 0.0) // delta angle
      os << " -angle {-1 " << arrow()->tailBackAngle().value() << "}";
  }

  if      (arrow()->frontLength().isSet() && arrow()->tailLength().isSet())
    os << " -length {" << arrow()->frontLength().toString().toStdString() <<
                   " " << arrow()->tailLength ().toString().toStdString() << "}";
  else if (arrow()->frontLength().isSet())
    os << " -length {" << arrow()->frontLength().toString().toStdString() << " -1}";
  else if (arrow()->tailLength().isSet())
    os << " -length {-1 " << arrow()->tailLength ().toString().toStdString() << "}";

#if 0
  if (arrow()->isFilled())
    os << " -filled 1";

  if (arrow()->fillColor().isValid())
    os << " -fill_color {" << arrow()->fillColor().toString().toStdString() << "}";

  if (arrow()->isStroked())
    os << " -stroked 1";

  if (arrow()->strokeColor().isValid())
    os << " -stroke_color {" << arrow()->strokeColor().toString().toStdString() << "}";

  if (arrow()->strokeWidth().isValid())
    os << " -stroke_width {" << arrow()->strokeWidth().toString().toStdString() << "}";
#endif

  os << "]\n";

  //---

  writeProperties(os, varName);

  //---

  // write arrow properties
  auto *propertyModel = this->propertyModel();

  PropertyModel::NameValues nameValues;

  if (propertyModel)
    propertyModel->getChangedNameValues(this, arrow(), nameValues, /*tcl*/true);

  if (nameValues.empty())
    return;

  os << "\n";

  for (const auto &nv : nameValues) {
    QString str;

    if (! CQChartsVariant::toString(nv.second, str))
      str = "";

    os << "set_charts_property -annotation $" << varName.toStdString();

    os << " -name " << nv.first.toStdString() << " -value {" << str.toStdString() << "}\n";
  }
}

//------

CQChartsArcAnnotation::
CQChartsArcAnnotation(View *view, const Position &start, const Position &end) :
 CQChartsAnnotation(view, Type::ARC), start_(start), end_(end)
{
  init();
}

CQChartsArcAnnotation::
CQChartsArcAnnotation(Plot *plot, const Position &start, const Position &end) :
 CQChartsAnnotation(plot, Type::ARC), start_(start), end_(end)
{
  init();
}

CQChartsArcAnnotation::
~CQChartsArcAnnotation()
{
}

void
CQChartsArcAnnotation::
setStart(const Position &p)
{
  CQChartsUtil::testAndSet(start_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsArcAnnotation::
setEnd(const Position &p)
{
  CQChartsUtil::testAndSet(end_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsArcAnnotation::
init()
{
  setObjectName(QString("arc.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);
}

//---

void
CQChartsArcAnnotation::
setLine(bool b)
{
  CQChartsUtil::testAndSet(isLine_, b, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
setRectilinear(bool b)
{
  CQChartsUtil::testAndSet(rectilinear_, b, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
setFrontType(const HeadType &type)
{
  CQChartsUtil::testAndSet(frontType_, type, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
setTailType(const HeadType &type)
{
  CQChartsUtil::testAndSet(tailType_, type, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
setLineWidth(const Length &l)
{
  CQChartsUtil::testAndSet(lineWidth_, l, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "start"      , "", "Arc start point");
  addProp(model, path1, "startObjRef", "", "Arc start object reference");
  addProp(model, path1, "end"        , "", "Arc end point");
  addProp(model, path1, "endObjRef"  , "", "Arc end object reference");
  addProp(model, path1, "isLine"     , "", "Arc is line");
  addProp(model, path1, "rectilinear", "", "Arc is rectilinear");
  addProp(model, path1, "frontType"  , "", "Arc front type");
  addProp(model, path1, "tailType"   , "", "Arc tail type");
  addProp(model, path1, "lineWidth"  , "", "Arc line width");

  //---

  addStrokeFillProperties(model, path1);
}

void
CQChartsArcAnnotation::
getPropertyNames(QStringList &names, bool hidden) const
{
  CQChartsAnnotation::getPropertyNames(names, hidden);
}

//---

void
CQChartsArcAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  double x1 = bbox.getXMin(), y1 = bbox.getYMin();
  double x2 = bbox.getXMax(), y2 = bbox.getYMax();

  double dx1 = x1 - start.x;
  double dy1 = y1 - start.y;
  double dx2 = x2 - end  .x;
  double dy2 = y2 - end  .y;

  auto spos = this->start().p();
  auto epos = this->end  ().p();

  spos += Point(dx1, dy1);
  epos += Point(dx2, dy2);

  start_ = Position(spos, start_.units());
  end_   = Position(epos, end_  .units());

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsArcAnnotation::
inside(const Point &p) const
{
  QPainterPath path;

  calcPath(path);

  // TODO: handle line
  return path.contains(p.qpoint());
}

void
CQChartsArcAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // draw path
  QPainterPath path;

  calcPath(path);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if (isLine())
    device->setBrush(Qt::NoBrush);

  device->drawPath(path);

  //---

  drawTerm(device);
}

void
CQChartsArcAnnotation::
calcPath(QPainterPath &path) const
{
  bool isSelf = (startObjRef().isValid() && startObjRef() == endObjRef());

  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  double lw  = lengthParentHeight(lineWidth());
  double lw2 = lw/2.0;

  BBox         ibbox, obbox;
  CQChartsObj *startObj = nullptr, *endObj = nullptr;

  if (! startObjRef().isValid() || ! objectRect(startObjRef(), startObj, ibbox))
    ibbox = BBox(Point(start.x - lw2, start.y - lw2), Point(start.x + lw2, start.y + lw2));
  if (! endObjRef  ().isValid() || ! objectRect(endObjRef  (), endObj  , obbox))
    obbox = BBox(Point(end  .x - lw2, end  .y - lw2), Point(end  .x + lw2, end  .y + lw2));

  CQChartsArrowData arrowData;

  arrowData.setFHeadType((CQChartsArrowData::HeadType) frontType());
  arrowData.setTHeadType((CQChartsArrowData::HeadType) tailType ());

  arrowData.setLength(Length(lw, parentUnits()));

  QPainterPath lpath;

  if (! arrowData.isFHead() && ! arrowData.isTHead() && ! isRectilinear()) {
    if (! isSelf) {
      if (startObj || endObj)
        CQChartsDrawUtil::edgePath(path, ibbox, obbox, isLine());
      else {
        CQChartsDrawUtil::curvePath(lpath, start, end, isLine());

        CQChartsArrow::pathAddArrows(lpath, arrowData, lw, 2.0, path);
      }
    }
    else {
      CQChartsDrawUtil::selfEdgePath(path, ibbox, lw);
    }
  }
  else {
    if (! isSelf) {
      if (startObj || endObj)
        CQChartsDrawUtil::curvePath(lpath, ibbox, obbox, isRectilinear());
      else
        CQChartsDrawUtil::curvePath(lpath, start, end, isRectilinear());
    }
    else {
      CQChartsDrawUtil::selfCurvePath(lpath, ibbox, isRectilinear());
    }

    CQChartsArrow::pathAddArrows(lpath, arrowData, lw, 2.0, path);
  }
}

void
CQChartsArcAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  // start/end rect
  if (start().isSet())
    os << " -start {" << start().toString().toStdString() << "}";

  if (end().isSet())
    os << " -end {" << end().toString().toStdString() << "}";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPointAnnotation::
CQChartsPointAnnotation(View *view, const Position &position, const Symbol &type) :
 CQChartsAnnotation(view, Type::POINT), CQChartsObjPointData<CQChartsPointAnnotation>(this),
 position_(position), type_(type)
{
  init();
}

CQChartsPointAnnotation::
CQChartsPointAnnotation(Plot *plot, const Position &position, const Symbol &type) :
 CQChartsAnnotation(plot, Type::POINT), CQChartsObjPointData<CQChartsPointAnnotation>(this),
 position_(position), type_(type)
{
  init();
}

CQChartsPointAnnotation::
~CQChartsPointAnnotation()
{
}

void
CQChartsPointAnnotation::
setPosition(const Position &p)
{
  CQChartsUtil::testAndSet(position_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsPointAnnotation::
init()
{
  setObjectName(QString("point.%1").arg(ind()));

  setSymbolType(type_);
}

void
CQChartsPointAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position", "", "Point position");
  addProp(model, path1, "objRef"  , "", "Object reference");

  auto symbolPath = path1 + "/symbol";

  addProp(model, symbolPath, "symbolType", "type", "Point symbol type");
  addProp(model, symbolPath, "symbolSize", "size", "Point symbol size");

  auto fillPath = path1 + "/fill";

  addStyleProp(model, fillPath, "symbolFilled"     , "visible", "Point symbol fill visible");
  addStyleProp(model, fillPath, "symbolFillColor"  , "color"  , "Point symbol fill color");
  addStyleProp(model, fillPath, "symbolFillAlpha"  , "alpha"  , "Point symbol fill alpha");
  addStyleProp(model, fillPath, "symbolFillPattern", "pattern", "Point symbol fill pattern");

  auto strokePath = path1 + "/stroke";

  addStyleProp(model, strokePath, "symbolStroked"    , "visible", "Point symbol stroke visible");
  addStyleProp(model, strokePath, "symbolStrokeColor", "color"  , "Point symbol stroke color");
  addStyleProp(model, strokePath, "symbolStrokeAlpha", "alpha"  , "Point symbol stroke alpha");
  addStyleProp(model, strokePath, "symbolStrokeWidth", "width"  , "Point symbol stroke width");
  addStyleProp(model, strokePath, "symbolStrokeDash" , "dash"   , "Point symbol stroke dash");
}

//---

void
CQChartsPointAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto p = positionToParent(objRef(), position());

  double dx = bbox.getXMin() - rect_.getXMin();
  double dy = bbox.getYMin() - rect_.getYMin();

  p += Point(dx, dy);

  auto position1 = Position(p, parentUnits());

  position_ = positionFromParent(objRef(), position1);

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsPointAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsPointAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  const auto &symbolData = this->symbolData();

  auto p = positionToParent(objRef(), position());

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double sw = lengthParentWidth (symbolData.size());
  double sh = lengthParentHeight(symbolData.size());

  double w = sw + xlp + xrp + xlm + xrm;
  double h = sh + ybp + ytp + ybm + ytm;
  double x = p.x - w/2.0; // left
  double y = p.y - h/2.0; // bottom

  setAnnotationBBox(BBox(x, y, x + w, y + h));

  //---

  const auto &strokeData = symbolData.stroke();
  const auto &fillData   = symbolData.fill();

  // set pen and brush
  PenBrush penBrush;

  auto lineColor = interpColor(strokeData.color(), ColorInd());
  auto fillColor = interpColor(fillData  .color(), ColorInd());

  if (isCheckable() && ! isChecked()) {
    double f = uncheckedLighter();

    lineColor = CQChartsUtil::blendColors(backgroundColor(), lineColor, f);
    fillColor = CQChartsUtil::blendColors(backgroundColor(), fillColor, f);
  }

  setPenBrush(penBrush,
    PenData  (strokeData.isVisible(), lineColor, strokeData.alpha(),
              strokeData.width(), strokeData.dash()),
    BrushData(fillData  .isVisible(), fillColor, fillData.alpha(), fillData.pattern()));

  bool isSolid = (fillData.isVisible() && symbolData.type().type() != CQChartsSymbol::Type::DOT);

  if (isSolid)
    updatePenBrushState(penBrush, CQChartsObjDrawType::SYMBOL);
  else
    updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw symbol
  Point ps(rect_.getXMid(), rect_.getYMid());

  CQChartsDrawUtil::drawSymbol(device, symbolData.type(), ps, symbolData.size());

  //---

  drawTerm(device);
}

void
CQChartsPointAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  const auto &symbolData = this->symbolData();

  if (position().isSet())
    os << " -position {" << position().toString().toStdString() << "}";

  if (symbolType().type() != Symbol::Type::NONE)
    os << " -type {" << symbolType().toString().toStdString() << "}";

  if (symbolData.size().isSet())
    os << " -size {" << symbolData.size().toString().toStdString() << "}";

#if 0
  writeFill  ();
  writeStroke();
#endif

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPieSliceAnnotation::
CQChartsPieSliceAnnotation(View *view, const Position &position, const Length &innerRadius,
                           const Length &outerRadius, const Angle &startAngle,
                           const Angle &spanAngle) :
 CQChartsAnnotation(view, Type::PIE_SLICE), position_(position),
 innerRadius_(innerRadius), outerRadius_(outerRadius),
 startAngle_(startAngle), spanAngle_(spanAngle)
{
  init();
}

CQChartsPieSliceAnnotation::
CQChartsPieSliceAnnotation(Plot *plot, const Position &position, const Length &innerRadius,
                           const Length &outerRadius, const Angle &startAngle,
                           const Angle &spanAngle) :
 CQChartsAnnotation(plot, Type::PIE_SLICE), position_(position),
 innerRadius_(innerRadius), outerRadius_(outerRadius),
 startAngle_(startAngle), spanAngle_(spanAngle)
{
  init();
}

CQChartsPieSliceAnnotation::
~CQChartsPieSliceAnnotation()
{
}

void
CQChartsPieSliceAnnotation::
setPosition(const Position &p)
{
  CQChartsUtil::testAndSet(position_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
init()
{
  setObjectName(QString("pie_slice.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);
}

void
CQChartsPieSliceAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position"   , "", "Pie slice position");
  addProp(model, path1, "innerRadius", "", "Pie slice inner radius");
  addProp(model, path1, "outerRadius", "", "Pie slice outer radius");
  addProp(model, path1, "startAngle" , "", "Pie slice start angle");
  addProp(model, path1, "spanAngle"  , "", "Pie slice span angle");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPieSliceAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto p = positionToParent(objRef(), position());

  double dx = bbox.getXMin() - rect_.getXMin();
  double dy = bbox.getYMin() - rect_.getYMin();

  p += Point(dx, dy);

  position_ = Position(p, Position::Units::PLOT);

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsPieSliceAnnotation::
inside(const Point &p) const
{
  CQChartsArcData arcData;

  auto c = positionToParent(objRef(), position());

  arcData.setCenter(Point(c));

  arcData.setInnerRadius(lengthParentWidth(innerRadius()));
  arcData.setOuterRadius(lengthParentWidth(outerRadius()));

  arcData.setAngle1(startAngle());
  arcData.setAngle2(startAngle() + spanAngle());

  return arcData.inside(p);
}

void
CQChartsPieSliceAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // calc box
  auto p = positionToParent(objRef(), position());

  double xri = lengthParentWidth (innerRadius());
  double yri = lengthParentHeight(innerRadius());

  double xro = lengthParentWidth (outerRadius());
  double yro = lengthParentHeight(outerRadius());

  Point c(p);

  setAnnotationBBox(BBox(c.x - xro, c.y - yro, c.x + xro, c.y + yro));

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // draw pie slice
  CQChartsDrawUtil::setPenBrush(device, penBrush);

  CQChartsDrawUtil::drawPieSlice(device, c, CMathUtil::avg(xri, yri), CMathUtil::avg(xro, yro),
                                 startAngle(), startAngle() + spanAngle(), false, false);

  //---

  drawTerm(device);
}

void
CQChartsPieSliceAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (position().isSet())
    os << " -position {" << position().toString().toStdString() << "}";

  if (innerRadius().isValid())
    os << " -inner_radius {" << innerRadius().toString().toStdString() << "}";

  if (outerRadius().isValid())
    os << " -outer_radius {" << outerRadius().toString().toStdString() << "}";

  os << " -start_angle " << startAngle().value();

  os << " -span_angle " << spanAngle().value();

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsAxisAnnotation::
CQChartsAxisAnnotation(Plot *plot, Qt::Orientation direction, double start, double end) :
 CQChartsAnnotation(plot, Type::AXIS)
{
  init();

  axis_ = std::make_unique<Axis>(plot, direction, start, end);

  axis_->setAnnotation     (true);
  axis_->setAllowHtmlLabels(true);

  connect(axis_.get(), SIGNAL(ticksChanged()), this, SLOT(emitDataChanged()));
  connect(axis_.get(), SIGNAL(appearanceChanged()), this, SLOT(invalidateSlot()));
}

CQChartsAxisAnnotation::
~CQChartsAxisAnnotation()
{
}

void
CQChartsAxisAnnotation::
init()
{
  setObjectName(QString("axis.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (false);
}

Qt::Orientation
CQChartsAxisAnnotation::
direction()
{
  return axis_->direction();
}

void
CQChartsAxisAnnotation::
setDirection(Qt::Orientation &d)
{
  axis_->setDirection(d);
}

double
CQChartsAxisAnnotation::
start() const
{
  return axis_->start();
}

void
CQChartsAxisAnnotation::
setStart(double r)
{
  axis_->setStart(r);
}

double
CQChartsAxisAnnotation::
end() const
{
  return axis_->end();
}

void
CQChartsAxisAnnotation::
setEnd(double r)
{
  axis_->setEnd(r);
}

double
CQChartsAxisAnnotation::
position() const
{
  return axis_->position().realOr(0.0);
}

void
CQChartsAxisAnnotation::
setPosition(double r)
{
  axis_->setPosition(CQChartsOptReal(r));
}

void
CQChartsAxisAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addStrokeProperties(model, path1 + "/stroke", /*isSolid*/false);

  axis_->addProperties(model, path1, CQChartsAxis::PropertyType::ANNOTATION);

  addProp(model, path1, "direction", "", "Axis direction");
  addProp(model, path1, "start"    , "", "Axis start");
  addProp(model, path1, "end"      , "", "Axis end");
  addProp(model, path1, "position" , "", "Axis position");
}

//---

void
CQChartsAxisAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  setAnnotationBBox(bbox);
}

//---

bool
CQChartsAxisAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

void
CQChartsAxisAnnotation::
draw(PaintDevice *device)
{
  if (! isVisible())
    return;

  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  bool usePen     = true;
  bool forceColor = false;

  if ((plot()->drawLayerType() == CQChartsLayer::Type::MOUSE_OVER && isInside  ()) ||
      (plot()->drawLayerType() == CQChartsLayer::Type::SELECTION  && isSelected()))
    forceColor = true;

  axis_->draw(plot(), device, usePen, forceColor);

  //---

  drawTerm(device);

  //--

  setAnnotationBBox(axis_->bbox());
}

void
CQChartsAxisAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

class CQChartsKeyAnnotationPlotKey : public CQChartsPlotKey {
 public:
  CQChartsKeyAnnotationPlotKey(Plot *plot, CQChartsKeyAnnotation *annotation) :
   CQChartsPlotKey(plot), annotation_(annotation) {
  }

  void redraw(bool) override {
    annotation_->invalidate();
  }

 private:
  CQChartsKeyAnnotation *annotation_ { nullptr };
};

class CQChartsKeyAnnotationColumnKey : public CQChartsColumnKey {
 public:
  using KeyAnnotation = CQChartsKeyAnnotation;
  using Column        = CQChartsColumn;

 public:
  CQChartsKeyAnnotationColumnKey(Plot *plot, KeyAnnotation *annotation, const Column &column) :
   CQChartsColumnKey(plot), annotation_(annotation) {
    setColumn(column);
  }

  void redraw(bool) override {
    annotation_->invalidate();
  }

 private:
  KeyAnnotation *annotation_ { nullptr };
};

CQChartsKeyAnnotation::
CQChartsKeyAnnotation(View *view) :
 CQChartsAnnotation(view, Type::KEY)
{
  init();

  key_ = new CQChartsViewKey(view);
}

CQChartsKeyAnnotation::
CQChartsKeyAnnotation(Plot *plot, const Column &column) :
 CQChartsAnnotation(plot, Type::KEY)
{
  init();

  if (column.isValid())
    key_ = new CQChartsKeyAnnotationColumnKey(plot, this, column);
  else
    key_ = new CQChartsKeyAnnotationPlotKey(plot, this);

  connect(plot_, SIGNAL(rangeChanged()), this, SLOT(updateLocationSlot()));
}

CQChartsKeyAnnotation::
~CQChartsKeyAnnotation()
{
  delete key_;
}

void
CQChartsKeyAnnotation::
init()
{
  setObjectName(QString("key.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

void
CQChartsKeyAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addStrokeProperties(model, path1 + "/stroke");

  auto keyPath = path1 + "/key";

  key_->addProperties(model, keyPath, "");
}

void
CQChartsKeyAnnotation::
updateLocationSlot()
{
  if (plot_) {
    auto *plotKey = qobject_cast<CQChartsPlotKey *>(key_);

    if (plotKey)
      plotKey->updatePlotLocation();
  }
}

//---

void
CQChartsKeyAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  setAnnotationBBox(bbox);
}

//---

bool
CQChartsKeyAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

bool
CQChartsKeyAnnotation::
selectPress(const Point &w, SelMod selMod)
{
  if (key_->selectPress(w, selMod)) {
    emit pressed(QString("%1:%2").arg(id()).arg(key_->id()));
    return true;
  }

  auto *plotKey = dynamic_cast<CQChartsPlotKey *>(key_);

  if (plotKey->contains(w)) {
    auto *item = plotKey->getItemAt(w);

    if (item) {
      bool handled = item->selectPress(w, selMod);

      if (handled) {
        emit pressed(QString("%1:%2:%3").arg(id()).arg(key_->id()).arg(item->id()));
        return true;
      }
    }
  }

  return CQChartsAnnotation::selectPress(w, selMod);
}

void
CQChartsKeyAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  key_->draw(device);

  //---

  drawTerm(device);

  //---

  setAnnotationBBox(key_->bbox());
}

void
CQChartsKeyAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPointSetAnnotation::
CQChartsPointSetAnnotation(View *view, const Points &values) :
 CQChartsAnnotation(view, Type::POINT_SET), values_(values)
{
  init();
}

CQChartsPointSetAnnotation::
CQChartsPointSetAnnotation(Plot *plot, const Points &values) :
 CQChartsAnnotation(plot, Type::POINT_SET), values_(values)
{
  init();
}

CQChartsPointSetAnnotation::
~CQChartsPointSetAnnotation()
{
}

void
CQChartsPointSetAnnotation::
init()
{
  setObjectName(QString("pointset.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);

  updateValues();
}

void
CQChartsPointSetAnnotation::
updateValues()
{
  hull_.clear();

  for (auto &p : values_.points()) {
    auto p1 = positionToParent(objRef(), p);

    hull_.addPoint(p1);

    xrange_.add(p1.x);
    yrange_.add(p1.y);

    gridCell_.addPoint(Point(p1));
  }
}

void
CQChartsPointSetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "values"  , "", "Values");
  addProp(model, path1, "drawType", "", "Draw type");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPointSetAnnotation::
setEditBBox(const BBox &, const ResizeSide &)
{
#if 0
  auto hbbox = hull_.bbox();

  auto ll = positionToParent(objRef(), hbbox.getUL());

  double dx = hbbox.getXMin() - rect_.getXMin();
  double dy = hbbox.getYMin() - rect_.getYMin();

  ll += Point(dx, dy);

  rect_ = BBox(ll.x, ll.y, bbox.getWidth(), bbox.getHeight());

  // TODO: move all points ?
#endif
}

//---

bool
CQChartsPointSetAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

void
CQChartsPointSetAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::SYMBOL);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if      (drawType() == DrawType::SYMBOLS) {
    CQChartsSymbolData symbolData;

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      CQChartsDrawUtil::drawSymbol(device, symbolData.type(), Point(p1), symbolData.size());
    }
  }
  else if (drawType() == DrawType::HULL) {
    hull_.draw(device);
  }
  else if (drawType() == DrawType::DENSITY) {
    if (! plot_) return;

    CQChartsBivariateDensity density;

    CQChartsBivariateDensity::Data data;

    data.gridSize = 16;
    data.delta    = 0.0;

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      data.values.push_back(p1);
    }

    data.xrange = xrange_;
    data.yrange = yrange_;

    density.calc(plot_, data);

    density.draw(plot_, device);
  }
  else if (drawType() == DrawType::BEST_FIT) {
    CQChartsFitData fitData;

    Polygon poly;

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      poly.addPoint(p1);
    }

    fitData.calc(poly, 3);

    auto p1 = device->windowToPixel(Point(xrange_.min(), yrange_.min()));
    auto p2 = device->windowToPixel(Point(xrange_.max(), yrange_.max()));

    double dx = (p2.x - p1.x)/100.0;

    Polygon fitPoly;

    for (int i = 0; i <= 100; ++i) {
      double px = p1.x + i*dx;

      auto p = device->pixelToWindow(Point(px, 0.0));

      double y = fitData.interp(p.x);

      fitPoly.addPoint(Point(p.x, y));
    }

    auto path = CQChartsDrawUtil::polygonToPath(fitPoly, /*closed*/false);

    device->strokePath(path, penBrush.pen);
  }
  else if (drawType() == DrawType::GRID) {
    gridCell_.setNX(40);
    gridCell_.setNY(40);

    gridCell_.setXInterval(xrange_.min(), xrange_.max());
    gridCell_.setYInterval(yrange_.min(), yrange_.max());

    gridCell_.resetPoints();

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      gridCell_.addPoint(Point(p1));
    }

    int maxN = gridCell_.maxN();

    //---

    auto strokeColor = interpStrokeColor(ColorInd());

    PenData penData1(isStroked(), strokeColor, strokeAlpha(), strokeWidth(),
                     strokeDash(), strokeCap(), strokeJoin());

    //---

    for (const auto &px : gridCell_.xyPoints()) {
      int         ix      = px.first;
      const auto &yPoints = px.second;

      double xmin, xmax;

      gridCell_.xIValues(ix, xmin, xmax);

      for (const auto &py : yPoints) {
        int         iy     = py.first;
        const auto &points = py.second;

        int n = points.size();
        if (n <= 0) continue;

        double ymin, ymax;

        gridCell_.yIValues(iy, ymin, ymax);

        //---

        BBox bbox(xmin, ymin, xmax, ymax);

        ColorInd colorInd(CMathUtil::map(double(n), 1.0, 1.0*maxN, 0.0, 1.0));

        auto bgColor = view()->interpColor(Color(Color::Type::PALETTE), colorInd);

        // set pen and brush
        PenBrush penBrush1;

        setPenBrush(penBrush1, penData1, BrushData(true, bgColor));

        CQChartsDrawUtil::setPenBrush(device, penBrush1);

        // draw rect
        device->drawRect(bbox);
      }
    }
  }

  //---

  drawTerm(device);
}

void
CQChartsPointSetAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << " -values {" << values().toString().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsValueSetAnnotation::
CQChartsValueSetAnnotation(View *view, const Rect &rectangle, const Reals &values) :
 CQChartsAnnotation(view, Type::VALUE_SET), rectangle_(rectangle), values_(values)
{
  init();
}

CQChartsValueSetAnnotation::
CQChartsValueSetAnnotation(Plot *plot, const Rect &rectangle, const Reals &values) :
 CQChartsAnnotation(plot, Type::VALUE_SET), rectangle_(rectangle), values_(values)
{
  init();
}

CQChartsValueSetAnnotation::
~CQChartsValueSetAnnotation()
{
}

void
CQChartsValueSetAnnotation::
setRectangle(const Rect &rectangle)
{
  CQChartsUtil::testAndSet(rectangle_, rectangle, [&]() { emitDataChanged(); } );
}

void
CQChartsValueSetAnnotation::
init()
{
  assert(rectangle_.isValid());

  setObjectName(QString("valueset.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);

  updateValues();
}

void
CQChartsValueSetAnnotation::
updateValues()
{
  density_ = std::make_unique<CQChartsDensity>();

  density_->setDrawType(CQChartsDensity::DrawType::WHISKER);

  density_->setXVals(values_.reals());

  connect(density_.get(), SIGNAL(dataChanged()), this, SLOT(invalidateSlot()));
}

void
CQChartsValueSetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto addDensityProp = [&](const QString &path, const QString &name, const QString &alias,
                            const QString &desc) {
    return &(model->addProperty(path, density_.get(), name, alias)->setDesc(desc));
  };

  //---

  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "rectangle", "", "Rectangle");
  addProp(model, path1, "values"   , "", "Values");
  addProp(model, path1, "drawType" , "", "Draw Type");

  auto densityPath = path1 + "/density";

  addDensityProp(densityPath, "numSamples"     , "", "Number of samples");
  addDensityProp(densityPath, "smoothParameter", "", "Smooth parameter");
  addDensityProp(densityPath, "drawType"       , "", "Density draw types");
  addDensityProp(densityPath, "orientation"    , "", "Density orientation");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsValueSetAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto bbox1 = rectangle_.bbox();

  auto ll = positionToParent(objRef(), Position(bbox1.getMin(), Position::Units::PLOT));

  double dx = bbox.getXMin() - rect_.getXMin();
  double dy = bbox.getYMin() - rect_.getYMin();

  ll += Point(dx, dy);

  bbox1 = BBox(ll.x, ll.y, ll.x + bbox1.getWidth(), ll.y + bbox1.getHeight());

  if (bbox1.isSet())
    rectangle_.setValue(rectangle_.units(), bbox1);
}

//---

bool
CQChartsValueSetAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

void
CQChartsValueSetAnnotation::
draw(PaintDevice *device)
{
  assert(plot());

  drawInit(device);

  //---

  auto bbox = rectangle_.bbox();

  setAnnotationBBox(density_->bbox(bbox));

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::SYMBOL);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  // draw pie chart
  if      (drawType() == DrawType::PIE) {
    CQChartsRValues values;

    for (const auto &r : values_.reals())
      values.addValue(r);

    CQChartsPlotDrawUtil::drawPie(const_cast<CQChartsPlot *>(plot()), device,
                                  values, bbox, plot_->defaultPalette());
  }
  else if (drawType() == DrawType::TREEMAP) {
    CQChartsRValues values;

    for (const auto &r : values_.reals())
      values.addValue(r);

    CQChartsPlotDrawUtil::drawTreeMap(const_cast<CQChartsPlot *>(plot()), device,
                                      values, bbox, plot_->defaultPalette());
  }
  // draw density
  else if (drawType() == DrawType::DENSITY) {
    density_->draw(plot(), device, bbox);
  }

  //---

  drawTerm(device);
}

void
CQChartsValueSetAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (rectangle().isSet())
    os << " -rectangle {" << rectangle().toString().toStdString() << "}";

  os << " -values {" << values().toString().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsButtonAnnotation::
CQChartsButtonAnnotation(View *view, const Position &position, const QString &textStr) :
 CQChartsAnnotation(view, Type::BUTTON)
{
  setPosition(position);

  init(textStr);
}

CQChartsButtonAnnotation::
CQChartsButtonAnnotation(Plot *plot, const Position &position, const QString &textStr) :
 CQChartsAnnotation(plot, Type::BUTTON)
{
  setPosition(position);

  init(textStr);
}

CQChartsButtonAnnotation::
~CQChartsButtonAnnotation()
{
}

void
CQChartsButtonAnnotation::
init(const QString &textStr)
{
  setObjectName(QString("button.%1").arg(ind()));

  setTextStr  (textStr);
  setTextColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 1.0));
}

void
CQChartsButtonAnnotation::
setPosition(const Position &p)
{
  position_ = p;

  emitDataChanged();
}

void
CQChartsButtonAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position", "position", "Text origin");

  auto textPath = path1 + "/text";

  addProp(model, textPath, "textStr", "string", "Text string");

  addTextProperties(model, textPath, CQChartsTextOptions::ValueType::NONE);

  addStrokeFillProperties(model, path1);
}

bool
CQChartsButtonAnnotation::
selectPress(const Point &, SelMod)
{
  pressed_ = true;

  invalidate();

  return true;
}

bool
CQChartsButtonAnnotation::
selectMove(const Point &)
{
  return true;
}

bool
CQChartsButtonAnnotation::
selectRelease(const Point &)
{
  pressed_ = false;

  invalidate();

  return true;
}

//---

bool
CQChartsButtonAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsButtonAnnotation::
draw(PaintDevice *device)
{
  if (device->type() == PaintDevice::Type::SVG)
    return;

  //---

  drawInit(device);

  //---

  prect_ = calcPixelRect();
  rect_  = pixelToWindow(BBox(prect_));

  //---

  if (device->isInteractive()) {
    auto *painter = dynamic_cast<CQChartsViewPlotPaintDevice *>(device);

    auto img = CQChartsUtil::initImage(QSize(prect_.width(), prect_.height()));

    QStylePainter spainter(&img, view());

    QStyleOptionButton opt;

    opt.rect = QRect(0, 0, prect_.width(), prect_.height());
    opt.text = textStr();

    if (pressed_)
      opt.state |= QStyle::State_Sunken;
    else
      opt.state |= QStyle::State_Raised;

    opt.state |= QStyle::State_Active;

    if (isEnabled())
      opt.state |= QStyle::State_Enabled;

    if (isCheckable()) {
      if (isChecked())
        opt.state |= QStyle::State_On;
      else
        opt.state |= QStyle::State_Off;
    }

    opt.palette = view()->palette();

    spainter.setFont(calcFont(textFont()));

    auto bg = opt.palette.color(QPalette::Button);
    auto fg = opt.palette.color(QPalette::ButtonText);

    auto c = fg;

    if      (! isEnabled())
      c = CQChartsUtil::blendColors(bg, fg, 0.6);
    else if (isInside())
      c = Qt::blue;

    opt.palette.setColor(QPalette::ButtonText, c);

    spainter.drawControl(QStyle::CE_PushButton, opt);

    auto p = prect_.topLeft();

    painter->painter()->drawImage(p.x(), p.y(), img);
  }

  //---

  drawTerm(device);
}

void
CQChartsButtonAnnotation::
writeHtml(HtmlPaintDevice *device)
{
  prect_ = calcPixelRect();
  rect_  = pixelToWindow(BBox(prect_));

  device->createButton(rect_, textStr(), id(), "annotationClick");
}

QRect
CQChartsButtonAnnotation::
calcPixelRect() const
{
  auto font = calcFont(textFont());

  QFontMetrics fm(font);

  int w = fm.width(textStr()) + 4;
  int h = fm.height() + 4;

  auto p = positionToPixel(objRef(), position());

  return QRect(int(p.x), int(p.y), w, h);
}

void
CQChartsButtonAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << " -position {" << position().toString().toStdString() << "}";

  if (textStr().length())
    os << " -text {" << textStr().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsWidgetAnnotation::
CQChartsWidgetAnnotation(View *view, const Position &position, const Widget &widget) :
 CQChartsAnnotation(view, Type::WIDGET), widget_(widget)
{
  setPosition(position);

  init();
}

CQChartsWidgetAnnotation::
CQChartsWidgetAnnotation(Plot *plot, const Position &position, const Widget &widget) :
 CQChartsAnnotation(plot, Type::WIDGET), widget_(widget)
{
  setPosition(position);

  init();
}

CQChartsWidgetAnnotation::
CQChartsWidgetAnnotation(View *view, const Rect &rect, const Widget &widget) :
 CQChartsAnnotation(view, Type::WIDGET), widget_(widget)
{
  setRectangle(rect);

  init();
}

CQChartsWidgetAnnotation::
CQChartsWidgetAnnotation(Plot *plot, const Rect &rect, const Widget &widget) :
 CQChartsAnnotation(plot, Type::WIDGET), widget_(widget)
{
  setRectangle(rect);

  init();
}

CQChartsWidgetAnnotation::
~CQChartsWidgetAnnotation()
{
  delete winWidget_;
}

void
CQChartsWidgetAnnotation::
init()
{
  setObjectName(QString("widget.%1").arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  widget_.setParent(view());

  widget_.initSize();
}

CQChartsPosition
CQChartsWidgetAnnotation::
positionValue() const
{
  return position_.positionOr(Position());
}

void
CQChartsWidgetAnnotation::
setPosition(const Position &p)
{
  setPosition(OptPosition(p));
}

void
CQChartsWidgetAnnotation::
setPosition(const OptPosition &p)
{
  rectangle_ = OptRect();
  position_  = p;

  positionToBBox();

  if (winWidget_) {
    disconnect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));

    auto p = positionToPixel(objRef(), positionValue());

    winWidget_->setPos(p.x, p.y);

    connect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));
  }

  emitDataChanged();
}

CQChartsRect
CQChartsWidgetAnnotation::
rectangleValue() const
{
  return rectangle().rectOr(Rect());
}

void
CQChartsWidgetAnnotation::
setRectangle(const Rect &r)
{
  setRectangle(OptRect(r));
}

void
CQChartsWidgetAnnotation::
setRectangle(const OptRect &r)
{
  if (r.isSet())
    assert(r.rect().isValid());

  position_  = OptPosition();
  rectangle_ = r;

  rectToBBox();

  if (winWidget_) {
    disconnect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));

    BBox bbox;

    if      (plot())
      bbox = plot()->rectToPixel(rectangle_.rect());
    else if (view())
      bbox = view()->rectToPixel(rectangle_.rect());

    winWidget_->setPos (bbox.getXMin(), bbox.getYMin());
    winWidget_->setSize(bbox.getWidth(), bbox.getHeight());

    connect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));
  }

  emitDataChanged();
}

void
CQChartsWidgetAnnotation::
updateWinGeometry()
{
  if (! winWidget_)
    return;

  Point pos;
  Size  size;

  int x = winWidget_->getXPos();
  int y = winWidget_->getYPos();

  if      (plot()) {
    pos  = plot()->pixelToWindow(Point(x, y));
    size = plot()->pixelToWindowSize(Size(winWidget_->getWidth(), winWidget_->getHeight()));
 }
  else if (view()) {
    pos  = view()->pixelToWindow(Point(x, y));
    size = view()->pixelToWindowSize(Size(winWidget_->getWidth(), winWidget_->getHeight()));
  }

  position_  = OptPosition();
  rectangle_ = OptRect(Rect(BBox(pos, size)));

  rectToBBox();

  emitDataChanged();
}

void
CQChartsWidgetAnnotation::
rectToBBox()
{
  if (rectangle().isSet()) {
    auto rect = this->rectangleValue();

    if (plot())
      setAnnotationBBox(plot()->rectToPlot(rect));
    else
      setAnnotationBBox(view()->rectToView(rect));
  }
  else
    setAnnotationBBox(BBox());
}

void
CQChartsWidgetAnnotation::
setWidget(const Widget &widget)
{
  widget_ = widget;

  emitDataChanged();
}

void
CQChartsWidgetAnnotation::
setAlign(const Qt::Alignment &a)
{
  align_ = a;

  emitDataChanged();
}

void
CQChartsWidgetAnnotation::
setSizePolicy(const QSizePolicy &p)
{
  sizePolicy_ = p;

  emitDataChanged();
}

void
CQChartsWidgetAnnotation::
setInteractive(bool b)
{
  if (b != interactive_) {
    interactive_ = b;

    if (interactive_) {
      if (! winWidget_) {
        winWidget_ = new CQWinWidget;

        winWidget_->setObjectName("window");

        connect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));
      }

      winWidget_->setChild(widget_.widget());

      winWidget_->setParent(view());
      winWidget_->setVisible(true);
      winWidget_->setClosable(false);
    }
    else {
      widget_.setParent(view());

      delete winWidget_;

      winWidget_ = nullptr;
    }
  }
}

void
CQChartsWidgetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addProp(model, path1, "position"   , "position"   , "Widget position");
  addProp(model, path1, "rectangle"  , "rectangle"  , "Widget bounding box");
  addProp(model, path1, "widget"     , "widget"     , "Widget name");
  addProp(model, path1, "align"      , "align"      , "Widget position alignment");
  addProp(model, path1, "sizePolicy" , "sizePolicy" , "Widget size policy");
  addProp(model, path1, "interactive", "interactive", "Widget is interactive");
}

void
CQChartsWidgetAnnotation::
calcWidgetSize(Size &psize, Size &wsize) const
{
  // convert to window size
  psize = Size(widget_.size());

  if      (plot()) wsize = plot()->pixelToWindowSize(psize);
  else if (view()) wsize = view()->pixelToWindowSize(psize);
  else             wsize = psize;
}

void
CQChartsWidgetAnnotation::
positionToTopLeft(double w, double h, double &x, double &y) const
{
  auto p = positionToParent(objRef(), positionValue());

  if      (align() & Qt::AlignLeft)
    x = p.x;
  else if (align() & Qt::AlignHCenter)
    x = p.x - w/2;
  else
    x = p.x - w;

  if      (align() & Qt::AlignBottom)
    y = p.y - h;
  else if (align() & Qt::AlignVCenter)
    y = p.y - h/2;
  else
    y = p.y;
}

//---

void
CQChartsWidgetAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  if (rectangle().isSet()) {
    auto rect = Rect(bbox, parentUnits());

    setRectangle(rect);
  }
  else {
    // get position
    double x1 = bbox.getXMid();
    double y1 = bbox.getYMid();

    Point ll(x1, y1);

    //---

    auto position = Position(ll, parentUnits());

    setPosition(position);
  }

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsWidgetAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsWidgetAnnotation::
draw(PaintDevice *)
{
  if (! winWidget_)
    widget_.setVisible(isVisible());
  else
    winWidget_->setVisible(isVisible());

  //---

  if (! winWidget_) {
    // recalculate position to bbox on draw as can change depending on pixel mapping
    if (! rectangle().isSet())
      positionToBBox();

    if (! rect_.isValid())
      return;

    //---

    //drawInit(device);

    // set box
    //auto pbbox = windowToPixel(rect_);

    // get inner padding
    double xlp, xrp, ytp, ybp;

    getPaddingValues(xlp, xrp, ytp, ybp);

    // get outer margin
    double xlm, xrm, ytm, ybm;

    getMarginValues(xlm, xrm, ytm, ybm);

    double tx =          rect_.getXMin  () +       xlm + xlp;
    double ty =          rect_.getYMin  () +       ybm + ybp;
    double tw = std::max(rect_.getWidth () - xlm - xrm - xlp - xrp, 0.0);
    double th = std::max(rect_.getHeight() - ybm - ytm - ybp - ytp, 0.0);

    BBox tbbox(tx, ty, tx + tw, ty + th);

    //---

    auto ptbbox = windowToPixel(tbbox);

    widget_.resize(CMathRound::RoundNearest(ptbbox.getWidth()),
                   CMathRound::RoundNearest(ptbbox.getHeight()));

    widget_.move(int(ptbbox.getXMin()), int(ptbbox.getYMax()));

    //---

    //drawTerm(device);
  }
}

void
CQChartsWidgetAnnotation::
initRectangle()
{
  // convert position to rectangle if needed
  if (! rectangle().isSet()) {
    positionToBBox();

    //---

    auto rect = Rect(rect_, parentUnits());

    setRectangle(rect);
  }
}

void
CQChartsWidgetAnnotation::
positionToBBox()
{
  assert(! rectangle().isSet());

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  Size psize, wsize;

  calcWidgetSize(psize, wsize);

  double w = wsize.width ();
  double h = wsize.height();

  double x, y;

  positionToTopLeft(w, h, x, y);

  double vr = view()->viewportRange();

  if (sizePolicy().horizontalPolicy() == QSizePolicy::Expanding) {
    if (plot())
      w = plot()->viewToWindowWidth(vr);
    else
      w = vr;

    w -= xlp + xlm + xrp + xrm;
  }

  if (sizePolicy().verticalPolicy() == QSizePolicy::Expanding) {
    if (plot())
      h = plot()->viewToWindowHeight(vr);
    else
      h = vr;

    h -= xlp + xlm + xrp + xrm;
  }

  Point ll(x     - xlp - xlm, y     - ybp - ybm);
  Point ur(x + w + xrp + xrm, y + h + ytp + ytm);

  setAnnotationBBox(BBox(ll, ur));
}

void
CQChartsWidgetAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  if (rectangle().isSet()) {
    if (rectangleValue().isSet())
      os << " -rectangle {" << rectangleValue().toString().toStdString() << "}";
  }
  else {
    if (positionValue().isSet())
      os << " -position {" << positionValue().toString().toStdString() << "}";
  }

  if (widget_.isValid())
    os << " -widget {" << widget_.path().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsSymbolSizeMapKeyAnnotation::
CQChartsSymbolSizeMapKeyAnnotation(Plot *plot) :
 CQChartsAnnotation(plot, Type::SYMBOL_MAP_KEY)
{
  init();

  key_ = new CQChartsSymbolSizeMapKey(plot);

  connect(key_, SIGNAL(dataChanged()), this, SLOT(invalidateSlot()));
}

CQChartsSymbolSizeMapKeyAnnotation::
~CQChartsSymbolSizeMapKeyAnnotation()
{
  delete key_;
}

void
CQChartsSymbolSizeMapKeyAnnotation::
init()
{
  setObjectName(QString("symbolMapKey.%1").arg(ind()));

  setStroked(true);
  setFilled (true);

  setTextColor(CQChartsColor(CQChartsColor::Type::INTERFACE_VALUE, 1.0));
}

void
CQChartsSymbolSizeMapKeyAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &/*desc*/)
{
  auto path1 = path + "/" + propertyId();

  CQChartsAnnotation::addProperties(model, path1);

  addStrokeFillProperties(model, path1, /*isSolid*/false);

  auto textPath = path1 + "/text";

  addTextProperties(model, textPath, CQChartsTextOptions::ValueType::NONE);

  //---

  auto keyPath = path1 + "/key";

  auto addProp = [&](const QString &name, const QString &desc, bool hidden=false) {
    auto *item = model->addProperty(keyPath, this, name);
    item->setDesc(desc);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  //---

  key_->addProperties(model, keyPath);

  //---

  addProp("position", "Position");
}

void
CQChartsSymbolSizeMapKeyAnnotation::
updateLocationSlot()
{
}

//---

void
CQChartsSymbolSizeMapKeyAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  position_ = Position(bbox.getCenter(), Position::Units::PLOT);
}

//---

bool
CQChartsSymbolSizeMapKeyAnnotation::
inside(const Point &p) const
{
  return annotationBBox().inside(p);
}

void
CQChartsSymbolSizeMapKeyAnnotation::
draw(PaintDevice *device)
{
  drawInit(device);

  //---

  // set position
  disconnect(key_, SIGNAL(dataChanged()), this, SLOT(invalidateSlot()));

  if (position_.isSet())
    key_->setPosition(position_);
  else
    key_->setPosition(Position());

  connect(key_, SIGNAL(dataChanged()), this, SLOT(invalidateSlot()));

  //---

  key_->initDraw();

  //---

  // set pen and brush for circles
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  // draw circles
  key_->drawCircles(device, /*usePenBrush*/true);

  //---

  // set text options for text
  CQChartsTextOptions textOptions;

  textOptions.angle         = textAngle();
  textOptions.align         = Qt::AlignLeft;
  textOptions.contrast      = isTextContrast();
  textOptions.contrastAlpha = textContrastAlpha();
  textOptions.formatted     = isTextFormatted();
  textOptions.scaled        = isTextScaled();
  textOptions.html          = isTextHtml();
  textOptions.clipLength    = lengthPixelWidth(textClipLength());
  textOptions.clipElide     = textClipElide();
  textOptions.clipped       = false;

  adjustTextOptions(textOptions);

  //---

  // set text pen and font
  auto c = interpColor(textColor(), ColorInd());

  setPenBrush(penBrush, PenData(true, c, textAlpha()), BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  setPainterFont(device, textFont());

  // draw text
  key_->drawText(device, textOptions, /*usePenBrush*/true);

  //---

  // draw border
  auto bc = interpColor(strokeColor(), ColorInd());

  setPenBrush(penBrush,
    PenData(true, bc, strokeAlpha(), strokeWidth(), strokeDash(),
            strokeCap(), strokeJoin()),
    BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  key_->drawBorder(device, /*usePenBrush*/true);

  //---

  drawTerm(device);

  //---

  setAnnotationBBox(key_->bbox());
}

void
CQChartsSymbolSizeMapKeyAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << "]\n";

  //---

  writeProperties(os, varName);
}
