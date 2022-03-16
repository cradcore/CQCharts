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
#include <CQChartsDelaunay.h>
#include <CQChartsContour.h>
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
#include <CQChartsModelData.h>
#include <CQChartsModelUtil.h>
#include <CQChartsWordCloud.h>
#include <CQChartsTreeMapPlace.h>
#include <CQChartsGraphMgr.h>

#include <CQPropertyViewModel.h>
#include <CQPropertyViewItem.h>
#include <CQModelVisitor.h>
#include <CQUtil.h>
#include <CQWinWidget.h>
#include <CMathRound.h>

const QStringList &
CQChartsAnnotation::
typeNames()
{
  static auto names = QStringList() <<
    "arc" << "arrow" << "axis" << "button" << "ellipse" << "image" << "key" << "path" <<
    "pie_slice" << "point" << "point_set" << "polygon" << "polyline" << "rectangle" <<
    "text" << "value_set" << "widget" << "group";

  return names;
}

const QStringList &
CQChartsAnnotation::
typeDescs()
{
  static auto names = QStringList() <<
    "Arc" << "Arrow" << "Axis" << "Button" << "Ellipse" << "Image" << "Key" << "Path" <<
    "Pie Slice" << "Point" << "Point Set" << "Polygon" << "Polyline" << "Rectangle" <<
    "Text" << "Value Set" << "Widget" << "Group";

  return names;
}

CQChartsAnnotation::Type
CQChartsAnnotation::
stringToType(const QString &str)
{
  if      (str == "group"          ) return Type::GROUP;
  else if (str == "rect"           ) return Type::RECT;
  else if (str == "ellipse"        ) return Type::ELLIPSE;
  else if (str == "polygon"        ) return Type::POLYGON;
  else if (str == "polylin"        ) return Type::POLYLINE;
  else if (str == "text"           ) return Type::TEXT;
  else if (str == "image"          ) return Type::IMAGE;
  else if (str == "path"           ) return Type::PATH;
  else if (str == "arrow"          ) return Type::ARROW;
  else if (str == "arc"            ) return Type::ARC;
  else if (str == "arc_connector"  ) return Type::ARC_CONNECTOR;
  else if (str == "point"          ) return Type::POINT;
  else if (str == "pie_slice"      ) return Type::PIE_SLICE;
  else if (str == "axis"           ) return Type::AXIS;
  else if (str == "key"            ) return Type::KEY;
  else if (str == "point3d_set"    ) return Type::POINT3D_SET;
  else if (str == "point_set"      ) return Type::POINT_SET;
  else if (str == "value_set"      ) return Type::VALUE_SET;
  else if (str == "button"         ) return Type::BUTTON;
  else if (str == "widget"         ) return Type::WIDGET;
  else if (str == "symbol_map_key" ) return Type::SYMBOL_MAP_KEY;

  return Type::NONE;
}

//---

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
    if (nv.first == "id")
      continue;

    QString str;

    if (! CQChartsVariant::toString(nv.second, str))
      str.clear();

    os << "set_charts_property -annotation $" << varName.toStdString();

    os << " -name " << nv.first.toStdString() << " -value {" << str.toStdString() << "}\n";
  }
}

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

void
CQChartsAnnotation::
setDefaultPalette(const PaletteName &name)
{
  CQChartsUtil::testAndSet(defaultPalette_, name, [&]() { invalidate(); } );
}

//---

void
CQChartsAnnotation::
setValue(const OptReal &r)
{
  value_ = r;
}

//---

void
CQChartsAnnotation::
setModelIndex(const ModelIndex &ind)
{
  CQChartsUtil::testAndSet(modelIndex_, ind, [&]() {
    modelIndex_.setPlot(plot()); emitDataChanged();
  } );
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
  auto path1 = path + "/" + propertyId();

  model->setObjectRoot(path1, this);

  addProp(model, path1, "id" , "id" , "Annotation id");
  addProp(model, path1, "tip", "tip", "Annotation tip");

  //---

  // state
  auto statePath = path1 + "/state";

  addProp(model, statePath, "visible"   , "visible"   , "Is visible");
  addProp(model, statePath, "enabled"   , "enabled"   , "Is enabled"   , true);
  addProp(model, statePath, "checkable" , "checkable" , "Is checkable" , true);
  addProp(model, statePath, "checked"   , "checked"   , "Is checked"   , true);
  addProp(model, statePath, "selected"  , "selected"  , "Is selected"  , true);
  addProp(model, statePath, "selectable", "selectable", "Is selectable");
  addProp(model, statePath, "editable"  , "editable"  , "Is editable"  , true);
  addProp(model, statePath, "fitted"    , "fitted"    , "Is fitted"    , true);

  //---

  if (hasMargin())
    addProp(model, path1, "margin", "", "Outer margin", true);

  if (hasPadding())
    addProp(model, path1, "padding", "", "Inner padding", true);

  //---

  auto layerPath = path1 + "/layer";

  addProp(model, layerPath, "drawLayer", "drawLayer", "Draw layer", true);

  //---

  auto coloringPath = path1 + "/coloring";

  addProp(model, coloringPath, "defaultPalette"  , "defaultPalette"  ,
          "Default palette");
  addProp(model, coloringPath, "disabledLighter" , "disabledLighter" ,
          "Ligher when disabled" , true);
  addProp(model, coloringPath, "uncheckedLighter", "uncheckedLighter",
          "Ligher when unchecked", true);

  //---

  addProp(model, path1, "value" , "", "Associated value");

  // TODO: modelIndex
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
  if (isSolid)
    addStyleProp(model, path, "stroked", "visible", "Stroke visible");

  addStyleProp(model, path, "strokeColor", "color", "Stroke color");
  addStyleProp(model, path, "strokeAlpha", "alpha", "Stroke alpha");
  addStyleProp(model, path, "strokeWidth", "width", "Stroke width");
  addStyleProp(model, path, "strokeDash" , "dash" , "Stroke dash" );
  addStyleProp(model, path, "strokeCap"  , "cap"  , "Stroke cap"  );
  addStyleProp(model, path, "strokeJoin" , "join" , "Stroke join" );

  if (isSolid) {
    addStyleProp(model, path, "cornerSize" , "cornerSize", "Box corner size"  );
    addStyleProp(model, path, "borderSides", "sides"     , "Box visible sides");
  }
}

void
CQChartsAnnotation::
addTextProperties(PropertyModel *model, const QString &path, uint types)
{
  if (types & uint(TextOptions::ValueType::DATA))
    addStyleProp(model, path, "textData", "style", "Text style", true);

  addStyleProp(model, path, "textColor", "color", "Text color");
  addStyleProp(model, path, "textAlpha", "alpha", "Text alpha");
  addStyleProp(model, path, "textFont" , "font" , "Text font");

  if (types & uint(TextOptions::ValueType::ALIGN))
    addStyleProp(model, path, "textAlign", "align", "Text align");

  if (types & uint(TextOptions::ValueType::ANGLE))
    addStyleProp(model, path, "textAngle", "angle", "Text angle");

  if (types & uint(TextOptions::ValueType::CONTRAST))
    addStyleProp(model, path, "textContrast", "contrast", "Text has contrast");

  if (types & uint(TextOptions::ValueType::FORMATTED))
    addStyleProp(model, path, "textFormatted", "formatted", "Text formatted to fit in box");

  if (types & uint(TextOptions::ValueType::SCALED))
    addStyleProp(model, path, "textScaled", "scaled", "Text scaled to fit box");

  if (types & uint(TextOptions::ValueType::HTML))
    addStyleProp(model, path, "textHtml", "html", "Text is HTML");

  if (types & uint(TextOptions::ValueType::CLIP_LENGTH))
    addStyleProp(model, path, "textClipLength", "clipLength", "Text clip length");

  if (types & uint(TextOptions::ValueType::CLIP_ELIDE))
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
  auto c1 = c;

  if (defaultPalette().isValid())
    c1 = charts()->adjustDefaultPalette(c, defaultPalette_.name());

  if      (plot())
    return plot()->interpColor(c1, ind);
  else if (view())
    return view()->interpColor(c1, ind);
  else
    return charts()->interpColor(c1, ind);
}

CQChartsPaletteName
CQChartsAnnotation::
calcPalette() const
{
  if (defaultPalette().isValid())
    return defaultPalette();

  if (plot() && plot()->defaultPalette().isValid())
    return plot()->defaultPalette();

  return CQChartsPaletteName();
}

QString
CQChartsAnnotation::
calcPaletteName() const
{
  if (defaultPalette().isValid())
    return defaultPalette().name();

  if (plot() && plot()->defaultPalette().isValid())
    return plot()->defaultPalette().name();

  return "";
}

//---

bool
CQChartsAnnotation::
selectPress(const Point &, SelData &)
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

  if (dragSide == CQChartsResizeSide::NONE)
    return false;

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

CQChartsResizeHandle *
CQChartsAnnotation::
createExtraHandle() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  return (handles->view() ? new CQChartsResizeHandle(handles->view(), CQChartsResizeSide::EXTRA) :
                            new CQChartsResizeHandle(handles->plot(), CQChartsResizeSide::EXTRA));
}

//---

void
CQChartsAnnotation::
moveTo(const Point &p)
{
  auto bbox = calcBBox();

  auto c = (bbox.isValid() ? bbox.getCenter() : Point(0, 0));

  double dx = p.x - c.x;
  double dy = p.y - c.y;

  if (bbox.isValid())
    bbox.moveBy(Point(dx, dy));
  else
    bbox = BBox(p.x - 0.5, p.y - 0.5, p.x + 0.5, p.y + 0.5);

  setEditBBox(bbox, CQChartsResizeSide::NONE);
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

  if (plot() && plot()->showBoxes())
    plot()->drawWindowColorBox(device, annotationBBox());

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
  for (auto *annotation : annotations_)
    annotation->setGroup(nullptr);
}

void
CQChartsAnnotationGroup::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

CQChartsRect
CQChartsAnnotationGroup::
rectangle() const
{
  return Rect(annotationBBox(), parentUnits());
}

void
CQChartsAnnotationGroup::
setRectangle(const Rect &rect)
{
  assert(rect.isValid());

  if (plot())
    setAnnotationBBox(plot()->rectToPlot(rect));
  else
    setAnnotationBBox(view()->rectToView(rect));
}

//---

void
CQChartsAnnotationGroup::
setShapeType(const ShapeType &s)
{
  CQChartsUtil::testAndSet(shapeType_, s, [&]() { invalidate(); } );
}

//---

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

  for (auto *annotation1 : annotations_) {
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
doLayout()
{
  assert(layoutType() != LayoutType::NONE);

  //---

  if      (layoutType() == LayoutType::HV)
    layoutHV();
  else if (layoutType() == LayoutType::CIRCLE)
    layoutCircle();
  else if (layoutType() == LayoutType::TEXT_OVERLAP)
    layoutTextOverlap();
  else if (layoutType() == LayoutType::TEXT_CLOUD)
    layoutTextCloud();
  else if (layoutType() == LayoutType::TREEMAP)
    layoutTreemap();
  else if (layoutType() == LayoutType::GRAPH)
    layoutGraph();

  if (plot())
    plot()->drawObjs();

  initBBox_ = true;
}

void
CQChartsAnnotationGroup::
layoutHV()
{
  // place child annotations in bbox
  double x    { 0.0 }, y    { 0.0 };
  double maxW { 0.0 }, maxH { 0.0 };
  double sumW { 0.0 }, sumH { 0.0 };

  int n = 0;

  for (auto *annotation : annotations_) {
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

  if (annotationBBox().isSet()) {
    x = annotationBBox().getXMin();
    y = annotationBBox().getYMax();
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

    for (auto *annotation : annotations_) {
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

    for (auto *annotation : annotations_) {
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
layoutCircle()
{
  class AnnotationNode : public CQChartsCircleNode {
   public:
    AnnotationNode(CQChartsAnnotation *annotation) :
     annotation_(annotation) {
    }

    CQChartsAnnotation *annotation() const { return annotation_; }

    double radius() const override {
      auto bbox = annotation_->calcBBox();

      return (bbox.isValid() ? bbox.getWidth()/2.0 : 1.0);
    }

   private:
    CQChartsAnnotation *annotation_ { nullptr };
  };

  //---

  // pack child annotations using circle pack
  using Pack = CQChartsCirclePack<AnnotationNode>;

  Pack pack;

  using Nodes = std::vector<AnnotationNode *>;

  Nodes nodes;

  for (auto *annotation : annotations_) {
    auto *annotationGroup = dynamic_cast<CQChartsAnnotationGroup *>(annotation);

    if (annotationGroup)
      annotationGroup->layoutCircle();

    //---

    auto *node = new AnnotationNode(annotation);

    nodes.push_back(node);

    pack.addNode(node);
  }

  //---

  // get bounding circle
  double xc { 0.0 }, yc { 0.0 }, r { 1.0 };

  pack.boundingCircle(xc, yc, r);

  //---

  // move annotation
  setDisableSignals(true);

  if (! annotationBBox().isValid()) {
    auto cbbox = childrenBBox();

    setAnnotationBBox(cbbox);
  }

  BBox bbox(xc - r, yc - r, xc + r, yc + r);

  moveTo(Point(xc, yc));

  setAnnotationBBox(BBox(xc - r, yc - r, xc + r, yc + r));

  setDisableSignals(false);

  //--

  for (auto *node : nodes) {
    auto *annotation = node->annotation();

    annotation->setDisableSignals(true);

    annotation->moveTo(Point(node->x(), node->y()));

    annotation->setDisableSignals(false);

    delete node;
  }
}

void
CQChartsAnnotationGroup::
layoutTextOverlap()
{
  if (! plot())
    return;

  const auto &rect = plot()->dataRect();
  if (! rect.isValid()) return;

  using TextPlacer     = CQChartsTextPlacer;
  using TextAnnotation = CQChartsTextAnnotation;

  TextPlacer placer;

  using AnnotationDrawTexts = std::map<TextAnnotation *, TextPlacer::DrawText *>;

  AnnotationDrawTexts annotationDrawTexts;

  for (auto *annotation : annotations_) {
    auto *textAnnotation = dynamic_cast<TextAnnotation *>(annotation);
    if (! textAnnotation) continue;

    CQChartsTextOptions textOptions;

    auto c = QColor(Qt::black);

    auto bbox = textAnnotation->annotationBBox();

    auto *drawText =
      new TextPlacer::DrawText(textAnnotation->textStr(), bbox.getCenter(), textOptions,
                               c, Alpha(), bbox.getCenter(), QFont());

    drawText->setBBox(bbox);

    placer.addDrawText(drawText);

    annotationDrawTexts[textAnnotation] = drawText;
  }

  placer.place(rect);

  for (const auto &p : annotationDrawTexts) {
    auto *textAnnotation = p.first;
    auto *drawText       = p.second;

    auto bbox = textAnnotation->calcBBox();

    auto c = (bbox.isValid() ? bbox.getCenter() : Point(0, 0));

    double dx = drawText->point.x - c.x;
    double dy = drawText->point.y - c.y;

    if (bbox.isValid())
      bbox.moveBy(Point(dx, dy));
    else
      bbox = BBox(drawText->point.x - 0.0, drawText->point.y - 0.5,
                  drawText->point.x + 0.5, drawText->point.y + 0.5);

    textAnnotation->setDisableSignals(true);

    textAnnotation->moveTo(bbox.getCenter());

    textAnnotation->setDisableSignals(false);
  }
}

void
CQChartsAnnotationGroup::
layoutTextCloud()
{
  using WordCloud      = CQChartsWordCloud;
  using TextAnnotation = CQChartsTextAnnotation;

  WordCloud wordCloud;

  using WordTextAnnotation = std::map<int, TextAnnotation *>;

  WordTextAnnotation wordTextAnnotation;

  for (auto *annotation : annotations_) {
    auto *textAnnotation = dynamic_cast<TextAnnotation *>(annotation);
    if (! textAnnotation) continue;

    auto value = std::max(textAnnotation->value().realOr(1.0), 1.0);

    int ind = wordCloud.addWord(textAnnotation->textStr(), int(value));

    wordTextAnnotation[ind] = textAnnotation;
  }

  wordCloud.setMinFontSize(windowToPixelWidth(0.04));
  wordCloud.setMaxFontSize(windowToPixelWidth(0.30));

  wordCloud.place(plot());

  for (const auto &wordData : wordCloud.wordDatas()) {
    BBox bbox(wordData->wordRect.xmin(), wordData->wordRect.ymin(),
              wordData->wordRect.xmax(), wordData->wordRect.ymax());

    auto *textAnnotation = wordTextAnnotation[wordData->ind];

    textAnnotation->setDisableSignals(true);

    textAnnotation->setTextAlign (Qt::AlignHCenter | Qt::AlignVCenter);
    textAnnotation->setTextHtml  (true); // TODO: html should not be needed !!
    textAnnotation->setTextScaled(true);

    //auto f = textAnnotation->textFont();
    //f.setPointSizeF(wordData->fontSize);
    //textAnnotation->setTextFont(f);

    textAnnotation->setRectangle(CQChartsRect::plot(bbox));

    textAnnotation->setEditBBox(bbox, CQChartsResizeSide::NONE);

    textAnnotation->setDisableSignals(false);
  }
}

void
CQChartsAnnotationGroup::
layoutTreemap()
{
  auto bbox = annotationBBox();
  if (! bbox.isValid()) return;

  CQChartsTreeMapPlace place(bbox);

  for (auto *annotation : annotations_) {
    auto value = std::max(annotation->value().realOr(1.0), 1.0);
    if (value <= 0.0) continue;

    place.addValue(value, annotation);
  }

  place.placeValues();

  place.processAreas([&](const BBox &bbox, const CQChartsTreeMapPlace::IArea &iarea) {
    auto *annotation = static_cast<CQChartsAnnotation *>(iarea.data);

    annotation->setDisableSignals(true);

    annotation->setEditBBox(bbox, CQChartsResizeSide::NONE);

    annotation->setDisableSignals(true);
  });
}

void
CQChartsAnnotationGroup::
layoutGraph()
{
  auto rect = annotationBBox();

  if (! rect.isValid()) {
    setAnnotationBBox(BBox(0, 0, 100, 100));

    rect = annotationBBox();
  }

  double w = rect.getWidth ();
//double h = rect.getHeight();

  CQChartsGraphMgr mgr(plot());

  // use plot units to avoid conversion so we use annotation bbox
  mgr.setNodeWidth  (Length::plot(w/ 20.0));
  mgr.setNodeMargin (Length::plot(w/100.0));
  mgr.setNodeSpacing(Length::plot(0.2    ));

//mgr.setNodeScaled    (true);
  mgr.setNodePerpScaled(true);

  auto *graph = mgr.createGraph("graph");

  for (auto *annotation : annotations_) {
    if (annotation->subType() != SubType::CONNECTOR)
      continue;

    auto *connector = dynamic_cast<CQChartsConnectorAnnotationBase *>(annotation);
    assert(connector);

    BBox         bbox;
    CQChartsObj *startObj = nullptr, *endObj = nullptr;

    if (! objectRect(connector->startObjRef(), startObj, bbox) ||
        ! objectRect(connector->endObjRef  (), endObj  , bbox))
      continue;

    auto *startAnnotation = dynamic_cast<CQChartsAnnotation *>(startObj);
    auto *endAnnotation   = dynamic_cast<CQChartsAnnotation *>(endObj  );
    if (! startAnnotation || ! endAnnotation) continue;

    auto *startNode = mgr.findNode(startAnnotation->pathId());
    auto *endNode   = mgr.findNode(endAnnotation  ->pathId());

    if (! startNode) {
      startNode = mgr.addNode(startAnnotation->pathId());

      graph->addNode(startNode);
    }

    if (! endNode) {
      endNode = mgr.addNode(endAnnotation->pathId());

      graph->addNode(endNode);
    }

    CQChartsOptReal value(connector->value());

    auto *edge = mgr.createEdge(value, startNode, endNode);

    startNode->addDestEdge(edge);
    endNode  ->addSrcEdge (edge);
  }

  graph->placeGraph(rect);

  for (const auto *node : graph->nodes()) {
    auto *annotation = (plot() ? plot()->getAnnotationByPathId(node->name()) : nullptr);
    if (! annotation) continue;

    annotation->setDisableSignals(true);

    annotation->setEditBBox(node->rect(), CQChartsResizeSide::NONE);

    annotation->setDisableSignals(false);
  }

  delete graph;
}

//---

void
CQChartsAnnotationGroup::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  //---

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "rectangle", "", "Rectangle");
  addProp(model, path1, "shapeType", "", "Shape type");

  //---

  auto layoutPath = path1 + "/layout";

  addProp(model, layoutPath, "layoutType"   , "type"   , "Layout type");
  addProp(model, layoutPath, "layoutOrient" , "orient" , "Layout orientation");
  addProp(model, layoutPath, "layoutAlign"  , "align"  , "Layout alignment");
  addProp(model, layoutPath, "layoutSpacing", "spacing", "Layout spacing");
  addProp(model, layoutPath, "layoutMargin" , "margin" , "Layout margin");

  //---

  addStrokeFillProperties(model, path1);
}

//---

bool
CQChartsAnnotationGroup::
inside(const Point &p) const
{
  for (auto *annotation : annotations_)
    if (annotation->inside(p))
      return true;

  return false;
}

//---

void
CQChartsAnnotationGroup::
setEditBBox(const BBox &bbox, const ResizeSide &dragSide)
{
  moveChildren(bbox);

  setAnnotationBBox(bbox);

  if (dragSide != CQChartsResizeSide::NONE && dragSide != CQChartsResizeSide::MOVE) {
    if (layoutType() != LayoutType::NONE)
      doLayout();
  }
}

void
CQChartsAnnotationGroup::
moveChildren(const BBox &bbox)
{
  if (! annotationBBox().isValid())
    return;

  double dx = bbox.getXMid() - annotationBBox().getXMid();
  double dy = bbox.getYMid() - annotationBBox().getYMid();
  if (CMathUtil::isZero(dx) && CMathUtil::isZero(dy)) return;

  moveChildren(dx, dy);
}

void
CQChartsAnnotationGroup::
moveChildren(double dx, double dy)
{
  for (auto *annotation : annotations_) {
    auto bbox1 = annotation->annotationBBox();
    if (! bbox1.isValid()) continue;

    bbox1.moveBy(Point(dx, dy));

    annotation->moveTo(bbox1.getCenter());
  }
}

void
CQChartsAnnotationGroup::
moveTo(const Point &p)
{
  auto bbox = calcBBox();

  auto c = (bbox.isValid() ? bbox.getCenter() : Point(0, 0));

  double dx = p.x - c.x;
  double dy = p.y - c.y;

  moveChildren(dx, dy);

  if (bbox.isValid()) {
    bbox.moveBy(Point(dx, dy));

    setAnnotationBBox(bbox);
  }
}

void
CQChartsAnnotationGroup::
flip(Qt::Orientation orient)
{
  if (layoutType() != LayoutType::NONE) {
    // TODO
  }
  else {
    bool flipX = (orient == Qt::Horizontal);
    bool flipY = (orient == Qt::Vertical  );

    auto center = annotationBBox().getCenter();

    for (auto *annotation : annotations_) {
      auto bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      auto center1 = bbox1.getCenter();

      Point p1 = Point((flipX ? 2*center.x - center1.x : center1.x),
                       (flipY ? 2*center.y - center1.y : center1.y));

      annotation->flip(orient);

      double dx = p1.x - bbox1.getXMid();
      double dy = p1.y - bbox1.getYMid();

      bbox1.moveBy(Point(dx, dy));

      annotation->moveTo(bbox1.getCenter());
    }
  }
}

//---

CQChartsGeom::BBox
CQChartsAnnotationGroup::
childrenBBox() const
{
  BBox bbox;

  for (auto *annotation : annotations_) {
    const auto &bbox1 = annotation->calcBBox();
    if (! bbox1.isSet()) continue;

    bbox += bbox1;
  }

  return bbox;
}

//---

void
CQChartsAnnotationGroup::
draw(PaintDevice *device)
{
  if (initBBox_) {
    auto bbox = childrenBBox();

    if (bbox.isValid()) {
      setAnnotationBBox(bbox);

      initBBox_ = false;
    }
  }

  //---

  if (needsLayout_) {
    if (layoutType() != LayoutType::NONE)
      doLayout();

    needsLayout_ = false;
  }

  //---

  drawInit(device);

  //---

  // draw box
  if (shapeType() != ShapeType::NONE) {
    // set pen and brush
    PenBrush penBrush;

    calcPenBrush(penBrush);

    updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

    //---

    auto rect = annotationBBox();

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    if      (shapeType() == ShapeType::BOX)
      device->drawRect(rect);
    else if (shapeType() == ShapeType::CIRCLE)
      device->drawEllipse(rect);
  }

  //---

  for (auto *annotation : annotations_)
    annotation->draw(device);

  //---

  if (layoutType() == LayoutType::NONE) {
    BBox bbox;

    for (auto *annotation : annotations_) {
      const auto &bbox1 = annotation->annotationBBox();
      if (! bbox1.isSet()) continue;

      bbox += bbox1;
    }

    setAnnotationBBox(bbox);
  }

  //---

  drawTerm(device);
}

void
CQChartsAnnotationGroup::
writeDetails(std::ostream &os, const QString &parentVarName, const QString &varName) const
{
  for (auto *annotation : annotations_)
    annotation->write(os, parentVarName, varName);
}

//------

CQChartsShapeAnnotationBase::
CQChartsShapeAnnotationBase(View *view, Type type) :
 CQChartsAnnotation(view, type)
{
}

CQChartsShapeAnnotationBase::
CQChartsShapeAnnotationBase(Plot *plot, Type type) :
 CQChartsAnnotation(plot, type)
{
}

CQChartsShapeAnnotationBase::
~CQChartsShapeAnnotationBase()
{
}

//---

void
CQChartsShapeAnnotationBase::
setAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(angle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotationBase::
setObjRef(const ObjRef &o)
{
  CQChartsUtil::testAndSet(objRef_, o, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotationBase::
setTextInd(const QString &ind)
{
  if (ind != textInd_) {
    auto *textAnnotation = (plot() ?
      dynamic_cast<CQChartsTextAnnotation *>(plot()->getAnnotationByPathId(textInd_)) : nullptr);

    if (textAnnotation)
      textAnnotation->setVisible(true);

    textInd_ = ind;

    textAnnotation = (plot() ?
      dynamic_cast<CQChartsTextAnnotation *>(plot()->getAnnotationByPathId(textInd_)) : nullptr);

    if (textAnnotation)
      textAnnotation->setVisible(false);

    emitDataChanged();
  }
}

void
CQChartsShapeAnnotationBase::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "angle"  , "", "Shape angle");
  addProp(model, path1, "objRef" , "", "Object reference");
  addProp(model, path1, "textInd", "", "Text annotation ind");

  CQChartsAnnotation::addProperties(model, path, desc);
}

void
CQChartsShapeAnnotationBase::
drawText(PaintDevice *device, const BBox &rect)
{
  if (textInd() == "")
    return;

  auto *textAnnotation = (plot() ?
    dynamic_cast<CQChartsTextAnnotation *>(plot()->getAnnotationByPathId(textInd())) : nullptr);

  if (! textAnnotation)
    return;

  // TODO: per annotation type rect (e.g. pie slice)
  textAnnotation->drawInRect(device, rect);
}

//------

CQChartsPolyShapeAnnotationBase::
CQChartsPolyShapeAnnotationBase(View *view, Type type, const Polygon &polygon) :
 CQChartsShapeAnnotationBase(view, type), polygon_(polygon)
{
}

CQChartsPolyShapeAnnotationBase::
CQChartsPolyShapeAnnotationBase(Plot *plot, Type type, const Polygon &polygon) :
 CQChartsShapeAnnotationBase(plot, type), polygon_(polygon)
{
}

CQChartsPolyShapeAnnotationBase::
~CQChartsPolyShapeAnnotationBase()
{
}

//--

void
CQChartsPolyShapeAnnotationBase::
setPolygon(const Polygon &polygon)
{
  CQChartsUtil::testAndSet(polygon_, polygon, [&]() { emitDataChanged(); } );
}

void
CQChartsPolyShapeAnnotationBase::
setSmoothed(bool b)
{
  CQChartsUtil::testAndSet(smoothed_, b, [&]() { invalidate(); } );
}

//--

void
CQChartsPolyShapeAnnotationBase::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  auto path1 = path + "/" + propertyId();

  // polygon added in CQChartsPolygonAnnotation and CQChartsPolylineAnnotation
  addProp(model, path1, "smoothed", "", "Smooth lines");

  CQChartsShapeAnnotationBase::addProperties(model, path, desc);
}

//--

void
CQChartsPolyShapeAnnotationBase::
initSmooth() const
{
  // init smooth if needed
  if (! smooth_) {
    auto *th = const_cast<CQChartsPolyShapeAnnotationBase *>(this);

    const auto &polygon = polygon_.polygon();

    th->smooth_ = std::make_unique<Smooth>(polygon, /*sorted*/false);
  }
}

//------

CQChartsRectangleAnnotation::
CQChartsRectangleAnnotation(View *view, const Rect &rectangle) :
 CQChartsShapeAnnotationBase(view, Type::RECT), rectangle_(rectangle)
{
  init();
}

CQChartsRectangleAnnotation::
CQChartsRectangleAnnotation(Plot *plot, const Rect &rectangle) :
 CQChartsShapeAnnotationBase(plot, Type::RECT), rectangle_(rectangle)
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

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

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

//---

void
CQChartsRectangleAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "rectangle", "", "Rectangle bounding box");
  addProp(model, path1, "start"    , "", "Rectangle bottom left", true);
  addProp(model, path1, "end"      , "", "Rectangle top right", true);

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

//---

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

  // draw rectangle
  auto rect = annotationBBox();

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  CQChartsDrawUtil::drawRoundedRect(device, rect, cornerSize(), borderSides(), angle());

  //---

  drawText(device, annotationBBox());

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

CQChartsShapeAnnotation::
CQChartsShapeAnnotation(View *view, const Rect &rectangle) :
 CQChartsShapeAnnotationBase(view, Type::RECT), rectangle_(rectangle)
{
  init();
}

CQChartsShapeAnnotation::
CQChartsShapeAnnotation(Plot *plot, const Rect &rectangle) :
 CQChartsShapeAnnotationBase(plot, Type::RECT), rectangle_(rectangle)
{
  init();
}

CQChartsShapeAnnotation::
~CQChartsShapeAnnotation()
{
}

void
CQChartsShapeAnnotation::
init()
{
  assert(rectangle_.isValid());

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setMargin(CQChartsMargin());

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

void
CQChartsShapeAnnotation::
setRectangle(const Rect &rectangle)
{
  assert(rectangle.isValid());

  rectangle_ = rectangle;

  emitDataChanged();
}

void
CQChartsShapeAnnotation::
setRectangle(const Position &start, const Position &end)
{
  auto rectangle = CQChartsViewPlotObj::makeRect(view(), plot(), start, end);

  setRectangle(rectangle);
}

CQChartsPosition
CQChartsShapeAnnotation::
start() const
{
  if (! rectangle().bbox().isValid())
    return Position();

  Point p(rectangle().bbox().getXMin(), rectangle().bbox().getYMin());

  return Position(p, rectangle().units());
}

void
CQChartsShapeAnnotation::
setStart(const Position &p)
{
  auto start = positionToParent(objRef(), p);
  auto end   = positionToParent(objRef(), this->end());

  rectangle_ = Rect(BBox(start, end), parentUnits());

  assert(rectangle_.isValid());

  emitDataChanged();
}

CQChartsPosition
CQChartsShapeAnnotation::
end() const
{
  if (! rectangle().bbox().isValid())
    return Position();

  Point p(rectangle().bbox().getXMax(), rectangle().bbox().getYMax());

  return Position(p, rectangle().units());
}

void
CQChartsShapeAnnotation::
setEnd(const Position &p)
{
  auto start = positionToParent(objRef(), this->start());
  auto end   = positionToParent(objRef(), p);

  rectangle_ = Rect(BBox(start, end), parentUnits());

  assert(rectangle_.isValid());

  emitDataChanged();
}

void
CQChartsShapeAnnotation::
setShapeType(const ShapeType &s)
{
  CQChartsUtil::testAndSet(shapeType_, s, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotation::
setNumSides(int n)
{
  CQChartsUtil::testAndSet(numSides_, n, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotation::
setLineWidth(const Length &l)
{
  CQChartsUtil::testAndSet(lineWidth_, l, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotation::
setSymbol(const Symbol &t)
{
  CQChartsUtil::testAndSet(symbol_, t, [&]() { emitDataChanged(); } );
}

void
CQChartsShapeAnnotation::
setSymbolSize(const Length &s)
{
  CQChartsUtil::testAndSet(symbolSize_, s, [&]() { emitDataChanged(); } );
}

//---

bool
CQChartsShapeAnnotation::
intersectShape(const Point &p1, const Point &p2, Point &pi) const
{
  auto rect = this->rect();

  if      (shapeType() == ShapeType::CIRCLE || shapeType() == ShapeType::DOUBLE_CIRCLE) {
    return CQChartsGeom::lineIntersectCircle(rect, p1, p2, pi);
  }
  else if (shapeType() == ShapeType::POLYGON) {
    // get polygon path
    QPainterPath path;

    CQChartsDrawUtil::polygonSidesPath(path, rect, numSides() > 2 ? numSides() : 4, angle());

    auto points = CQChartsPath::pathPoints(path);

    return CQChartsGeom::lineIntersectPolygon(points, p1, p2, pi);
  }

  return false;
}

//---

void
CQChartsShapeAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  //---

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "rectangle", "", "Rectangle bounding box");
  addProp(model, path1, "start"    , "", "Rectangle bottom left", true);
  addProp(model, path1, "end"      , "", "Rectangle top right", true);
  addProp(model, path1, "shapeType", "", "Node shape type");
  addProp(model, path1, "numSides" , "", "Number of Shape Sides");

  //---

  auto dotPath1 = path1 + "/dotLine";

  addProp(model, dotPath1, "lineWidth" , "", "Dot line width");
  addProp(model, dotPath1, "symbol"    , "", "Dot line symbol");
  addProp(model, dotPath1, "symbolSize", "", "Dot line symbol size");

  //---

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsShapeAnnotation::
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

//---

void
CQChartsShapeAnnotation::
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

  if      (shapeType() == ShapeType::TRIANGLE) {
    // rounded ?
    device->drawPolygonSides(rect, 3, angle());
  }
  else if (shapeType() == ShapeType::DIAMOND) {
    if (! angle().isZero())
      device->drawPolygonSides(rect, 4, angle());
    else
      device->drawDiamond(rect);
  }
  else if (shapeType() == ShapeType::BOX) {
    if (! angle().isZero())
      device->drawPolygonSides(rect, 4, angle() + Angle::degrees(45));
    else
      device->drawRect(rect);
  }
  else if (shapeType() == ShapeType::POLYGON) {
    // rounded ?
    device->drawPolygonSides(rect, numSides() > 2 ? numSides() : 4, angle());
  }
  else if (shapeType() == ShapeType::CIRCLE) {
    device->drawEllipse(rect, angle());
  }
  else if (shapeType() == ShapeType::DOUBLE_CIRCLE) {
    double dx = rect.getWidth ()/10.0;
    double dy = rect.getHeight()/10.0;

    auto rect1 = rect.expanded(dx, dy, -dx, -dy);

    device->drawEllipse(rect , angle());
    device->drawEllipse(rect1, angle());
  }
  else if (shapeType() == ShapeType::DOT_LINE) {
    //bool horizontal = CMathUtil::realEq(std::abs(angle().degrees()), 90.0);

    CQChartsDrawUtil::drawDotLine(device, penBrush, rect, lineWidth(), false,
                                  symbol(), symbolSize(), angle());
  }
  else {
    CQChartsDrawUtil::drawRoundedRect(device, rect, cornerSize(), borderSides(), angle());
  }

  //---

  drawText(device, annotationBBox());

  //---

  drawTerm(device);
}

void
CQChartsShapeAnnotation::
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
 CQChartsShapeAnnotationBase(view, Type::ELLIPSE), center_(center),
 xRadius_(xRadius), yRadius_(yRadius)
{
  init();
}

CQChartsEllipseAnnotation::
CQChartsEllipseAnnotation(Plot *plot, const Position &center, const Length &xRadius,
                          const Length &yRadius) :
 CQChartsShapeAnnotationBase(plot, Type::ELLIPSE), center_(center),
 xRadius_(xRadius), yRadius_(yRadius)
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

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

void
CQChartsEllipseAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "center" , "", "Ellipse center point");
  addProp(model, path1, "xRadius", "", "Ellipse x radius");
  addProp(model, path1, "yRadius", "", "Ellipse y radius");

  addStrokeFillProperties(model, path1);
}

//---

CQChartsGeom::BBox
CQChartsEllipseAnnotation::
calcBBox() const
{
  auto c = positionToParent(objRef(), center());

  double xr = lengthParentWidth (xRadius());
  double yr = lengthParentHeight(yRadius());

  return BBox(c.x - xr, c.y - yr, c.x + xr, c.y + yr);
}

//---

void
CQChartsEllipseAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  center_ = Position::plot(bbox.getCenter());

  double w = bbox.getWidth ();
  double h = bbox.getHeight();

  assert(w > 0.0 && h > 0.0);

  xRadius_ = Length(w/2, parentUnits());
  yRadius_ = Length(h/2, parentUnits());

  setAnnotationBBox(bbox);
}

//---

void
CQChartsEllipseAnnotation::
moveTo(const Point &p)
{
  center_ = Position::plot(p);

  setAnnotationBBox(calcBBox());
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

  path.addEllipse(annotationBBox().qrect());

  if (! angle().isZero())
    path = CQChartsDrawUtil::rotatePath(path, angle().degrees());

  //---

  // draw path
  device->fillPath  (path, penBrush.brush);
  device->strokePath(path, penBrush.pen  );

  //---

  drawText(device, annotationBBox());

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
 CQChartsPolyShapeAnnotationBase(view, Type::POLYGON, polygon)
{
  init();
}

CQChartsPolygonAnnotation::
CQChartsPolygonAnnotation(Plot *plot, const Polygon &polygon) :
 CQChartsPolyShapeAnnotationBase(plot, Type::POLYGON, polygon)
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

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

//---

void
CQChartsPolygonAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsPolyShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "polygon", "", "Polygon points");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPolygonAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  double dx = bbox.getXMin() - annotationBBox().getXMin();
  double dy = bbox.getYMin() - annotationBBox().getYMin();
  double sx = (annotationBBox().getWidth () > 0 ?
                 bbox.getWidth ()/annotationBBox().getWidth () : 1.0);
  double sy = (annotationBBox().getHeight() > 0 ?
                 bbox.getHeight()/annotationBBox().getHeight() : 1.0);

  double x1 = annotationBBox().getXMin();
  double y1 = annotationBBox().getYMin();

  auto poly = polygon_.polygon();

  for (int i = 0; i < poly.size(); ++i) {
    double x = sx*(poly.point(i).x - x1) + x1 + dx;
    double y = sy*(poly.point(i).y - y1) + y1 + dy;

    poly.setPoint(i, Point(x, y));
  }

  Polygon polygon(poly);

  if (polygon.isValid(/*closed*/true)) {
    polygon_ = polygon;

    setAnnotationBBox(bbox);
  }
}

void
CQChartsPolygonAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  long i = CQChartsVariant::toInt(data, ok);

  int np = apoly_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  //---

  auto p = apoly_.point(int(i));

  apoly_.setPoint(int(i), Point(p.x + dx, p.y + dy));

  //---

  if (! angle().isZero())
    polygon_ = apoly_.rotated(polygon_.getCenter(), -angle());
  else
    polygon_ = apoly_;
}

//---

bool
CQChartsPolygonAnnotation::
inside(const Point &p) const
{
  const auto &polygon = apoly_.polygon();

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

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  // create path
  QPainterPath path;

  if (isSmoothed()) {
    initSmooth();

    // draw path
    path = smooth_->createPath(/*closed*/true);
  }
  else {
    path = CQChartsDrawUtil::polygonToPath(polygon, /*closed*/true);
  }

  //---

  // TODO: rounded ?

  // calc rotated path
  auto apath = path;

  if (! angle().isZero())
    apath = CQChartsDrawUtil::rotatePath(path, angle().degrees());

  //---

  // draw filled path
  device->fillPath  (apath, penBrush.brush);
  device->strokePath(apath, penBrush.pen  );

  //---

  setAnnotationBBox(BBox(apath.boundingRect()));

  //---

  CQChartsPolygon apoly;

  if (! angle().isZero())
    apoly_ = polygon_.rotated(polygon_.getCenter(), angle());
  else
    apoly_ = polygon_;

  //---

  drawText(device, annotationBBox());

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

  int np = apoly_.numPoints();

  while (int(extraHandles.size()) > np)
    handles->removeExtraHandle();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = createExtraHandle();

    handles->addExtraHandle(extraHandle);
  }

  //---

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(CQChartsVariant::fromInt(i));

    auto p = apoly_.point(i);

    extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

    ++i;
  }

  return handles;
}

//---

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
 CQChartsPolyShapeAnnotationBase(view, Type::POLYLINE, polygon)
{
  init();
}

CQChartsPolylineAnnotation::
CQChartsPolylineAnnotation(Plot *plot, const Polygon &polygon) :
 CQChartsPolyShapeAnnotationBase(plot, Type::POLYLINE, polygon)
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

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);
  setFilled (false);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

//---

void
CQChartsPolylineAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsPolyShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "polygon"   , "", "Polyline points");
  addProp(model, path1, "pointsType", "", "Polyline points type");

  addStrokeProperties(model, path1 + "/stroke", /*isSolid*/false);

  addStyleProp(model, path1 + "/stroke", "fillPattern", "pattern", "Stroke pattern");
}

//---

void
CQChartsPolylineAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  double dx = bbox.getXMin() - annotationBBox().getXMin();
  double dy = bbox.getYMin() - annotationBBox().getYMin();
  double sx = (annotationBBox().getWidth () > 0 ?
                 bbox.getWidth ()/annotationBBox().getWidth () : 1.0);
  double sy = (annotationBBox().getHeight() > 0 ?
                 bbox.getHeight()/annotationBBox().getHeight() : 1.0);

  double x1 = annotationBBox().getXMin();
  double y1 = annotationBBox().getYMin();

  auto poly = polygon_.polygon();

  for (int i = 0; i < poly.size(); ++i) {
    double x = sx*(poly.point(i).x - x1) + x1 + dx;
    double y = sy*(poly.point(i).y - y1) + y1 + dy;

    poly.setPoint(i, Point(x, y));
  }

  Polygon polygon(poly);

  if (polygon.isValid(/*closed*/false)) {
    polygon_ = polygon;

    setAnnotationBBox(bbox);
  }
}

void
CQChartsPolylineAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  long i = CQChartsVariant::toInt(data, ok);

  int np = apoly_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  //---

  auto p = apoly_.point(int(i));

  apoly_.setPoint(int(i), Point(p.x + dx, p.y + dy));

  //---

  if (! angle().isZero())
    polygon_ = apoly_.rotated(polygon_.getCenter(), -angle());
  else
    polygon_ = apoly_;
}

//---

bool
CQChartsPolylineAnnotation::
inside(const Point &p) const
{
  const auto &polygon = apoly_.polygon();

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

  // set pen
  PenBrush penBrush;

  auto strokeColor = interpStrokeColor(ColorInd());

  bool isFilled = (fillPattern().isValid() && ! fillPattern().isSolid());

  if (isFilled) {
    setPenBrush(penBrush,
      PenData  (true, strokeColor, strokeAlpha(), strokeWidth(), strokeDash(),
                strokeCap(), strokeJoin()),
      BrushData(true, strokeColor, strokeAlpha(), fillPattern()));
  }
  else {
    setPen(penBrush,
      PenData(true, strokeColor, strokeAlpha(), strokeWidth(),
              strokeDash(), strokeCap(), strokeJoin()));
  }

  if (isFilled)
    updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);
  else
    updatePenBrushState(penBrush, CQChartsObjDrawType::LINE);

  //---

  // create path
  QPainterPath path;

  if      (pointsType() == PointsType::CONTIGUOUS) {
    if (isSmoothed()) {
      initSmooth();

      // smooth path
      path = smooth_->createPath(/*closed*/false);
    }
    else {
      path = CQChartsDrawUtil::polygonToPath(polygon, /*closed*/false);
    }
  }
  else if (pointsType() == PointsType::LINE_PAIRS) {
    int nl = polygon.size()/2;

    for (int i = 0; i < nl; ++i) {
      path.moveTo(polygon.point(2*i + 0).qpoint());
      path.lineTo(polygon.point(2*i + 1).qpoint());
    }
  }

  //---

  // calc rotated path
  auto apath = path;

  if (! angle().isZero())
    apath = CQChartsDrawUtil::rotatePath(path, angle().degrees());

  //---

  if (isFilled) {
    // fill stroked path (for pattern)
    QPainterPathStroker stroker;

    stroker.setCapStyle   (penBrush.pen.capStyle());
    stroker.setDashOffset (penBrush.pen.dashOffset());
    stroker.setDashPattern(penBrush.pen.dashPattern());
    stroker.setJoinStyle  (penBrush.pen.joinStyle());
    stroker.setMiterLimit (penBrush.pen.miterLimit());
    stroker.setWidth      (device->lengthWindowWidth(strokeWidth()));

    device->fillPath(stroker.createStroke(apath), penBrush.brush);
  }
  else {
    // draw path
    device->strokePath(apath, penBrush.pen);
  }

  //---

  setAnnotationBBox(BBox(apath.boundingRect()));

  //---

  CQChartsPolygon apoly;

  if (! angle().isZero())
    apoly_ = polygon_.rotated(polygon_.getCenter(), angle());
  else
    apoly_ = polygon_;

  //---

  drawText(device, annotationBBox());

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

  int np = apoly_.numPoints();

  while (int(extraHandles.size()) > np)
    handles->removeExtraHandle();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = createExtraHandle();

    handles->addExtraHandle(extraHandle);
  }

  //---

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(CQChartsVariant::fromInt(i));

    auto p = apoly_.point(i);

    extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

    ++i;
  }

  return handles;
}

//---

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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setTextStr  (textStr);
  setTextColor(Color::makeInterfaceValue(1.0));

  setStroked(false);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

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
  if (! p.isSet())
    return;

  assert(p.position().isValid());

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
  if (r.isSet()) {
    assert(r.rect().isValid());

    position_  = OptPosition();
    rectangle_ = r;
  }
  else {
    if (! position_.isSet()) {
      if (rectangle_.isSet())
        position_  = OptPosition(rectangleValue().center());
      else
        position_  = OptPosition(Position::pixel(Point(0, 0)));
    }

    rectangle_ = OptRect();
  }

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

//---

void
CQChartsTextAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position" , "", "Text origin");
  addProp(model, path1, "rectangle", "", "Text bounding box");
  addProp(model, path1, "objRef"   , "", "Object reference");

  //---

  auto textPath = path1 + "/text";

  addProp(model, textPath, "textStr", "string", "Text string");

  addTextProperties(model, textPath, TextOptions::ValueType::ALL);

  //---

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

    rectangle_ = OptRect(rect);
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
  if (! isVisible())
    return;

  // recalculate position to bbox on draw as can change depending on pixel mapping
  if (! rectangle().isSet())
    positionToBBox();

  auto rect = annotationBBox();

  if (! rect.isValid())
    return;

  //---

  drawInit(device);

  drawInRect(device, rect);

  drawTerm(device);
}

void
CQChartsTextAnnotation::
drawInRect(PaintDevice *device, const BBox &rect)
{
  // set rect pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  if (isEnabled())
    updatePenBrushState(penBrush, CQChartsObjDrawType::TEXT);

  //---

  // draw box
  CQChartsDrawUtil::drawRoundedRect(device, penBrush, rect, cornerSize(), borderSides());

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
  auto textOptions = this->textOptions();

  textOptions.clipped = false;

  adjustTextOptions(textOptions);

  //---

  // set font
  setPainterFont(device, textFont());

  //---

  // set box
//auto pbbox = windowToPixel(rect);

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double tx =          rect.getXMin  () +       xlm + xlp;
  double ty =          rect.getYMin  () +       ybm + ybp;
  double tw = std::max(rect.getWidth () - xlm - xrm - xlp - xrp, 0.0);
  double th = std::max(rect.getHeight() - ybm - ytm - ybp - ytp, 0.0);

  BBox tbbox(tx, ty, tx + tw, ty + th);

  //---

  // draw text
  if (tbbox.isValid()) {
    device->setRenderHints(QPainter::Antialiasing);

    // TODO: multiple strings
    CQChartsDrawUtil::drawTextInBox(device, tbbox, textStr(), textOptions);
  }
}

void
CQChartsTextAnnotation::
initRectangle()
{
  // convert position to rectangle if needed
  if (! rectangle().isSet()) {
    positionToBBox();

    //---

    auto rect = Rect(annotationBBox(), parentUnits());

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
 CQChartsShapeAnnotationBase(view, Type::IMAGE), image_(image)
{
  setPosition(position);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(Plot *plot, const Position &position, const Image &image) :
 CQChartsShapeAnnotationBase(plot, Type::IMAGE), image_(image)
{
  setPosition(position);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(View *view, const Rect &rect, const Image &image) :
 CQChartsShapeAnnotationBase(view, Type::IMAGE), image_(image)
{
  setRectangle(rect);

  init();
}

CQChartsImageAnnotation::
CQChartsImageAnnotation(Plot *plot, const Rect &rect, const Image &image) :
 CQChartsShapeAnnotationBase(plot, Type::IMAGE), image_(image)
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  disabledImage_ = Image();

  image_        .setId(objectName());
  disabledImage_.setId(objectName() + "_dis");

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

//---

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
  if (! p.isSet())
    return;

  assert(p.position().isValid());

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
  if (r.isSet()) {
    assert(r.rect().isValid());

    position_  = OptPosition();
    rectangle_ = r;
  }
  else {
    if (! position_.isSet()) {
      if (rectangle_.isSet())
        position_  = OptPosition(rectangleValue().center());
      else
        position_  = OptPosition(Position::pixel(Point(0, 0)));
    }

    rectangle_ = OptRect();
  }

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

//---

void
CQChartsImageAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position"     , "position"     , "Image origin");
  addProp(model, path1, "rectangle"    , "rectangle"    , "Image bounding box");
  addProp(model, path1, "image"        , "image"        , "Image name");
  addProp(model, path1, "disabledImage", "disabledImage", "Disabled image name");

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

    rectangle_ = OptRect(rect);
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
  return CQChartsShapeAnnotationBase::inside(p);
}

void
CQChartsImageAnnotation::
draw(PaintDevice *device)
{
  image_.resolve(charts());

  //---

  // recalculate position to bbox on draw as can change depending on pixel mapping
  if (! rectangle().isSet())
    positionToBBox();

  auto rect = annotationBBox();

  if (! rect.isValid())
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
  CQChartsDrawUtil::drawRoundedRect(device, penBrush, rect, cornerSize(), borderSides());

  //---

  // get inner padding
  double xlp, xrp, ytp, ybp;

  getPaddingValues(xlp, xrp, ytp, ybp);

  // get outer margin
  double xlm, xrm, ytm, ybm;

  getMarginValues(xlm, xrm, ytm, ybm);

  double tx =          rect.getXMin  () +       xlm + xlp;
  double ty =          rect.getYMin  () +       ybm + ybp;
  double tw = std::max(rect.getWidth () - xlm - xrm - xlp - xrp, 0.0);
  double th = std::max(rect.getHeight() - ybm - ytm - ybp - ytp, 0.0);

  BBox tbbox(tx, ty, tx + tw, ty + th);

  //---

  // draw image
  if (! isEnabled()) {
    updateDisabledImage(DisabledImageType::DISABLED);

    device->drawImageInRect(tbbox, disabledImage_, /*stretch*/true, angle());
  }
  else {
    if (isCheckable() && ! isChecked()) {
      updateDisabledImage(DisabledImageType::UNCHECKED);

      device->drawImageInRect(tbbox, disabledImage_, /*stretch*/true, angle());
    }
    else {
      device->drawImageInRect(tbbox, image_, /*stretch*/true, angle());
    }
  }

  //---

  drawText(device, annotationBBox());

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

    auto rect = annotationBBox();

    auto prect = windowToPixel(rect);

    const auto &image = image_.sizedImage(int(prect.getWidth()), int(prect.getHeight()));

    disabledImage_     = Image(CQChartsUtil::disabledImage(image, bg, f));
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

    auto rect = Rect(annotationBBox(), parentUnits());

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
    os << " -image {" << image_.filename().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPathAnnotation::
CQChartsPathAnnotation(View *view, const Path &path) :
 CQChartsShapeAnnotationBase(view, Type::PATH), path_(path)
{
  init();
}

CQChartsPathAnnotation::
CQChartsPathAnnotation(Plot *plot, const Path &path) :
 CQChartsShapeAnnotationBase(plot, Type::PATH), path_(path)
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);
  setFilled (true);

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

//---

void
CQChartsPathAnnotation::
setPath(const Path &path)
{
  path_ = path;

  emitDataChanged();
}

//---

void
CQChartsPathAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "path", "path", "Path name");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPathAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &dragSide)
{
  if (dragSide == CQChartsResizeSide::MOVE) {
    // get position
    double dx = bbox.getXMid() - annotationBBox().getXMid();
    double dy = bbox.getYMid() - annotationBBox().getYMid();

    path_.move(dx, dy);
  }
  else {
    auto c1 = annotationBBox().getCenter();
    auto c2 = bbox.getCenter();

    auto w1 = annotationBBox().getWidth();
    auto w2 = bbox.getWidth();

    auto h1 = annotationBBox().getHeight();
    auto h2 = bbox.getHeight();

    double dx = c2.x - c1.x;
    double dy = c2.y - c1.y;

    double sx = (w1 > 0 ? w2/w1 : 1.0);
    double sy = (h1 > 0 ? h2/h1 : 1.0);

    path_.moveScale(dx, dy, sx, sy);
  }

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
  long i = CQChartsVariant::toInt(data, ok);

  int np = path_.numPoints();

  if (! ok || i < 0 || i >= np)
    return;

  auto p = path_.pointAt(int(i));

  path_.setPointAt(int(i), Point(p.x + dx, p.y + dy));
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

  auto path = path_.path();

  if (! angle().isZero())
    path = CQChartsDrawUtil::rotatePath(path, angle().degrees());

  device->drawPath(path);

  //---

  drawText(device, annotationBBox());

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

  while (int(extraHandles.size()) > np)
    handles->removeExtraHandle();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = createExtraHandle();

    handles->addExtraHandle(extraHandle);
  }

  //---

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  int i = 0;

  for (auto &extraHandle : handles->extraHandles()) {
    extraHandle->setData(CQChartsVariant::fromInt(i));

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

CQChartsConnectorAnnotationBase::
CQChartsConnectorAnnotationBase(View *view, Type type) :
 CQChartsAnnotation(view, type)
{
}

CQChartsConnectorAnnotationBase::
CQChartsConnectorAnnotationBase(Plot *plot, Type type) :
 CQChartsAnnotation(plot, type)
{
}

CQChartsConnectorAnnotationBase::
~CQChartsConnectorAnnotationBase()
{
}

//------

CQChartsArrowAnnotation::
CQChartsArrowAnnotation(View *view, const Position &start, const Position &end) :
 CQChartsConnectorAnnotationBase(view, Type::ARROW), start_(start), end_(end)
{
  init();
}

CQChartsArrowAnnotation::
CQChartsArrowAnnotation(Plot *plot, const Position &start, const Position &end) :
 CQChartsConnectorAnnotationBase(plot, Type::ARROW), start_(start), end_(end)
{
  init();
}

CQChartsArrowAnnotation::
~CQChartsArrowAnnotation()
{
}

//---

void
CQChartsArrowAnnotation::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  if (plot())
    arrow_ = std::make_unique<CQChartsArrow>(plot());
  else
    arrow_ = std::make_unique<CQChartsArrow>(view());

  connect(arrow(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  connect(editHandles(), SIGNAL(extraHandleMoved(const QVariant &, double, double)),
          this, SLOT(moveExtraHandle(const QVariant &, double, double)));
}

//---

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

//---

void
CQChartsArrowAnnotation::
setPath(const Path &path)
{
  path_ = path;

  emitDataChanged();
}

//---

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

//---

void
CQChartsArrowAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
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

  CQChartsAnnotation::addProperties(model, path, desc);

  //---

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "start"      , "", "Arrow start point");
  addProp(model, path1, "startObjRef", "", "Arrow start object reference");
  addProp(model, path1, "end"        , "", "Arrow end point");
  addProp(model, path1, "endObjRef"  , "", "Arrow end object reference");

  addProp(model, path1, "path", "path", "Path name");

  //---

  auto linePath = path1 + "/line";

  addArrowStyleProp(linePath, "lineWidth", "width", "Arrow connecting line width");

  addArrowStyleProp(linePath, "rectilinear", "rectilinear", "Rectilinear line");

  //---

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

  //---

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

  start_ = Position::plot(start);
  end_   = Position::plot(end  );

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
  if (path_.isValid()) {
    return drawPath_.contains(p.qpoint());
  }
  else {
    auto p1 = windowToPixel(p);

    return arrow()->contains(p1);
  }
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
  if (path_.isValid()) {
    CQChartsArrowData arrowData;
    QPainterPath      arrowPath;

    arrowData.setFHeadType(static_cast<CQChartsArrowData::HeadType>(arrow()->frontType()));
    arrowData.setTHeadType(static_cast<CQChartsArrowData::HeadType>(arrow()->tailType ()));

    auto lw = device->pixelToWindowWidth(4);

    if (arrow()->lineWidth().isSet() && arrow()->lineWidth().value() > 0)
      lw = device->lengthWindowWidth(arrow()->lineWidth());

    double frontLen = device->pixelToWindowWidth(4);
    double tailLen  = frontLen;

    if (arrow()->frontLength().isSet())
      frontLen = device->lengthWindowWidth(arrow()->frontLength());
    if (arrow()->tailLength().isSet())
      tailLen  = device->lengthWindowWidth(arrow()->tailLength());

    CQChartsArrow::pathAddArrows(path_.path(), arrowData, lw,
                                 (lw > 0.0 ? frontLen/lw : 1.0),
                                 (lw > 0.0 ? tailLen /lw : 1.0), arrowPath);

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    device->drawPath(arrowPath);

    drawPath_ = arrowPath;
  }
  else {
    CQChartsWidgetUtil::AutoDisconnect
      autoDisconnect(arrow(), SIGNAL(dataChanged()), this, SIGNAL(dataChanged()));

    arrow()->setFrom(start);
    arrow()->setTo  (end  );

    arrow()->draw(device, penBrush);
  }

  //---

  drawTerm(device);
}

//--

CQChartsEditHandles *
CQChartsArrowAnnotation::
editHandles() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  const auto &extraHandles = handles->extraHandles();

  int np = 2;

  if (path_.isValid())
    np = path_.numPoints();

  while (int(extraHandles.size()) > np)
    handles->removeExtraHandle();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = createExtraHandle();

    handles->addExtraHandle(extraHandle);
  }

  //---

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  if (path_.isValid()) {
    int i = 0;

    for (auto &extraHandle : handles->extraHandles()) {
      extraHandle->setData(CQChartsVariant::fromInt(i));

      auto p = path_.pointAt(i);

      extraHandle->setBBox(BBox(p.x - pw, p.y - ph, p.x + pw, p.y + ph));

      ++i;
    }
  }
  else {
    auto *extraHandle1 = handles->extraHandles()[0];
    auto *extraHandle2 = handles->extraHandles()[1];

    auto start = positionToParent(startObjRef(), this->start());
    auto end   = positionToParent(endObjRef  (), this->end  ());

    extraHandle1->setBBox(BBox(start.x - pw, start.y - ph, start.x + pw, start.y + ph));
    extraHandle2->setBBox(BBox(end  .x - pw, end  .y - ph, end  .x + pw, end  .y + ph));
  }

  return handles;
}

void
CQChartsArrowAnnotation::
moveExtraHandle(const QVariant &data, double dx, double dy)
{
  bool ok;
  long i = CQChartsVariant::toInt(data, ok);

  if (path_.isValid()) {
    int np = path_.numPoints();

    if (! ok || i < 0 || i >= np)
      return;

    auto p = path_.pointAt(int(i));

    path_.setPointAt(int(i), Point(p.x + dx, p.y + dy));
  }
  else {
    if (i == 0) {
      auto start = positionToParent(startObjRef(), this->start());

      start_ = Position(Point(start.x + dx, start.y + dy), start_.units());
    }
    else {
      auto end = positionToParent(endObjRef(), this->end());

      end_ = Position(Point(end.x + dx, end.y + dy), end_.units());
    }
  }
}

//--

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

  if (CQChartsArrowData::dataToName(static_cast<CQChartsArrowData::HeadType>(arrow()->frontType()),
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

  if (CQChartsArrowData::dataToName(static_cast<CQChartsArrowData::HeadType>(arrow()->tailType()),
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
      str.clear();

    os << "set_charts_property -annotation $" << varName.toStdString();

    os << " -name " << nv.first.toStdString() << " -value {" << str.toStdString() << "}\n";
  }
}

//------

CQChartsArcAnnotation::
CQChartsArcAnnotation(View *view, const Position &start, const Position &end) :
 CQChartsConnectorAnnotationBase(view, Type::ARC), start_(start), end_(end)
{
  init();
}

CQChartsArcAnnotation::
CQChartsArcAnnotation(Plot *plot, const Position &start, const Position &end) :
 CQChartsConnectorAnnotationBase(plot, Type::ARC), start_(start), end_(end)
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);
}

//---

void
CQChartsArcAnnotation::
setSolid(bool b)
{
  CQChartsUtil::testAndSet(isSolid_, b, [&]() { invalidate(); } );
}

void
CQChartsArcAnnotation::
setEdgeType(const EdgeType &type)
{
  CQChartsUtil::testAndSet(edgeType_, type, [&]() { invalidate(); } );
}

//---

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
setArrowSize(double s)
{
  CQChartsUtil::testAndSet(arrowSize_, s, [&]() { invalidate(); } );
}

//---

void
CQChartsArcAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "start"      , "", "Arc start point");
  addProp(model, path1, "startObjRef", "", "Arc start object reference");
  addProp(model, path1, "end"        , "", "Arc end point");
  addProp(model, path1, "endObjRef"  , "", "Arc end object reference");

  addProp(model, path1, "isSolid"  , "", "Edge is solid");
  addProp(model, path1, "edgeType" , "", "Edge type (arc, rectilinear or line)");
  addProp(model, path1, "frontType", "", "Arc front arrow type");
  addProp(model, path1, "tailType" , "", "Arc tail arrow type");
  addProp(model, path1, "lineWidth", "", "Edge line width");
  addProp(model, path1, "arrowSize", "", "Arrow size factor");

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
  double dx1 = bbox.getXMin() - bbox_.getXMin();
  double dy1 = bbox.getYMin() - bbox_.getYMin();
  double dx2 = bbox.getXMax() - bbox_.getXMax();
  double dy2 = bbox.getYMax() - bbox_.getYMax();

  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  if (start.x > end.x)
    std::swap(dx1, dx2);

  if (start.y > end.y)
    std::swap(dy1, dy2);

  start += Point(dx1, dy1);
  end   += Point(dx2, dy2);

  start_ = Position(start, start_.units());
  end_   = Position(end  , end_  .units());

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

  //---

  if (! isSolid()) {
    CQChartsArrowData arrowData;

    setArrowData(arrowData);

    bool hasArrows = (arrowData.calcIsFHead() || arrowData.calcIsTHead());

    if (! hasArrows)
      device->setBrush(Qt::NoBrush);
  }

  //---

  device->drawPath(path);

  //---

  drawTerm(device);

  setAnnotationBBox(BBox(path.boundingRect()));
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

  bool useBoxes = (startObj || endObj);

  //---

  CQChartsArrowData arrowData;

  setArrowData(arrowData);

  bool hasArrows = (arrowData.calcIsFHead() || arrowData.calcIsTHead());

  //---

  Angle angle = (useBoxes ? Angle::pointAngle(ibbox.getCenter(), obbox.getCenter()) :
                            Angle::pointAngle(start, end));

  auto drawEdgeType = static_cast<CQChartsDrawUtil::EdgeType>(edgeType());

  auto arrowSize = std::max(this->arrowSize(), 1.0);

  QPainterPath lpath;

  if (! isSelf) {
    if (isSolid()) {
      if (hasArrows) {
        // create line path
        if (useBoxes)
          CQChartsDrawUtil::curvePath(lpath, ibbox, obbox, drawEdgeType, angle);
        else
          CQChartsDrawUtil::curvePath(lpath, start, end, drawEdgeType, angle, angle);

        // add arrows with line width
        CQChartsArrow::pathAddArrows(lpath, arrowData, lw, arrowSize, arrowSize, path);
      }
      else {
        // create line width line (no arrows)
        if (useBoxes)
          CQChartsDrawUtil::edgePath(path, ibbox, obbox, drawEdgeType, angle);
        else
          CQChartsDrawUtil::edgePath(path, start, end, lw, drawEdgeType, angle, angle);
      }
    }
    else {
      // create line path
      if (useBoxes)
        CQChartsDrawUtil::curvePath(lpath, ibbox, obbox, drawEdgeType, angle);
      else
        CQChartsDrawUtil::curvePath(lpath, start, end, drawEdgeType, angle, angle);

      // add arrows (no line width)
      if (hasArrows)
        CQChartsArrow::pathAddArrows(lpath, arrowData, -lw, arrowSize, arrowSize, path);
      else
        path = lpath;
    }
  }
  else {
    if (isSolid()) {
      if (hasArrows) {
        // create line path
        CQChartsDrawUtil::selfCurvePath(lpath, ibbox, drawEdgeType);

        // add arrows with line width
        CQChartsArrow::pathAddArrows(lpath, arrowData, -lw, arrowSize, arrowSize, path);
      }
      else
        CQChartsDrawUtil::selfEdgePath(path, ibbox, lw, drawEdgeType);
    }
    else {
      CQChartsDrawUtil::selfCurvePath(lpath, ibbox, drawEdgeType);

      if (hasArrows)
        CQChartsArrow::pathAddArrows(lpath, arrowData, -lw, arrowSize, arrowSize, path);
      else
        path = lpath;
    }
  }
}

void
CQChartsArcAnnotation::
setArrowData(CQChartsArrowData &arrowData) const
{
  arrowData.setFHeadType(static_cast<CQChartsArrowData::HeadType>(frontType()));
  arrowData.setTHeadType(static_cast<CQChartsArrowData::HeadType>(tailType ()));

  double lw = lengthParentHeight(lineWidth());

  arrowData.setLength(Length(lw, parentUnits()));
}

//--

CQChartsEditHandles *
CQChartsArcAnnotation::
editHandles() const
{
  auto *handles = CQChartsViewPlotObj::editHandles();

  const auto &extraHandles = handles->extraHandles();

  int np = 2;

  while (int(extraHandles.size()) > np)
    handles->removeExtraHandle();

  while (int(extraHandles.size()) < np) {
    auto *extraHandle = createExtraHandle();

    handles->addExtraHandle(extraHandle);
  }

  //---

  double pw = pixelToWindowWidth (4);
  double ph = pixelToWindowHeight(4);

  auto *extraHandle1 = handles->extraHandles()[0];
  auto *extraHandle2 = handles->extraHandles()[1];

  auto start = positionToParent(startObjRef(), this->start());
  auto end   = positionToParent(endObjRef  (), this->end  ());

  extraHandle1->setBBox(BBox(start.x - pw, start.y - ph, start.x + pw, start.y + ph));
  extraHandle2->setBBox(BBox(end  .x - pw, end  .y - ph, end  .x + pw, end  .y + ph));

  return handles;
}

//--

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

CQChartsArcConnectorAnnotation::
CQChartsArcConnectorAnnotation(Plot *plot, const Position &center, const Length &radius,
                               const Angle &srcStartAngle, const Angle &srcSpanAngle,
                               const Angle &destStartAngle, const Angle &destSpanAngle,
                               bool self) :
 CQChartsConnectorAnnotationBase(plot, Type::ARC_CONNECTOR), center_(center), radius_(radius),
 srcStartAngle_(srcStartAngle), srcSpanAngle_(srcSpanAngle),
 destStartAngle_(destStartAngle), destSpanAngle_(destSpanAngle), self_(self)
{
  init();
}

CQChartsArcConnectorAnnotation::
~CQChartsArcConnectorAnnotation()
{
}

void
CQChartsArcConnectorAnnotation::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);
  setFilled (true);
}

//---

void
CQChartsArcConnectorAnnotation::
setCenter(const Position &c)
{
  CQChartsUtil::testAndSet(center_, c, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setRadius(const Length &r)
{
  CQChartsUtil::testAndSet(radius_, r, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setSrcStartAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(srcStartAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setSrcSpanAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(srcSpanAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setDestStartAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(destStartAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setDestSpanAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(destSpanAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsArcConnectorAnnotation::
setSelf(bool b)
{
  CQChartsUtil::testAndSet(self_, b, [&]() { emitDataChanged(); } );
}

//---

void
CQChartsArcConnectorAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "center"        , "", "Arc connector center");
  addProp(model, path1, "radius"        , "", "Arc connector radius");
  addProp(model, path1, "srcStartAngle" , "", "Arc connector source start angle");
  addProp(model, path1, "srcSpanAngle"  , "", "Arc connector source span angle");
  addProp(model, path1, "destStartAngle", "", "Arc connector destination start angle");
  addProp(model, path1, "destSpanAngle" , "", "Arc connector destination span angle");
  addProp(model, path1, "self"          , "", "Arc connector is self");

  //---

  addStrokeFillProperties(model, path1);
}

void
CQChartsArcConnectorAnnotation::
getPropertyNames(QStringList &names, bool hidden) const
{
  CQChartsAnnotation::getPropertyNames(names, hidden);
}

//---

void
CQChartsArcConnectorAnnotation::
setEditBBox(const BBox &, const ResizeSide &)
{
}

//---

bool
CQChartsArcConnectorAnnotation::
inside(const Point &p) const
{
  QPainterPath path;

  calcPath(path);

  return path.contains(p.qpoint());
}

void
CQChartsArcConnectorAnnotation::
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

  device->drawPath(path);

  //---

  drawTerm(device);
}

void
CQChartsArcConnectorAnnotation::
calcPath(QPainterPath &path) const
{
  auto center = positionToParent(this->center());

  double xr = lengthParentWidth (this->radius());
  double yr = lengthParentHeight(this->radius());

  BBox ibbox(center.x - xr, center.y - yr, center.x + xr, center.y + yr);

  CQChartsDrawUtil::arcsConnectorPath(path, ibbox, srcStartAngle(), srcSpanAngle(),
                                      destStartAngle(), destSpanAngle(), isSelf());
}

void
CQChartsArcConnectorAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  writeProperties(os, varName);
}

//------

CQChartsPointAnnotation::
CQChartsPointAnnotation(View *view, const Position &position, const Symbol &symbol) :
 CQChartsAnnotation(view, Type::POINT),
 CQChartsObjPointData<CQChartsPointAnnotation>(this),
 position_(position)
{
  init(symbol);
}

CQChartsPointAnnotation::
CQChartsPointAnnotation(Plot *plot, const Position &position, const Symbol &symbol) :
 CQChartsAnnotation(plot, Type::POINT),
 CQChartsObjPointData<CQChartsPointAnnotation>(this),
 position_(position)
{
  init(symbol);
}

CQChartsPointAnnotation::
~CQChartsPointAnnotation()
{
}

//---

void
CQChartsPointAnnotation::
setPosition(const Position &p)
{
  CQChartsUtil::testAndSet(position_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsPointAnnotation::
setShapeType(const ShapeType &t)
{
  CQChartsUtil::testAndSet(shapeType_, t, [&]() { emitDataChanged(); } );
}

void
CQChartsPointAnnotation::
setObjRef(const ObjRef &o)
{
  CQChartsUtil::testAndSet(objRef_, o, [&]() { emitDataChanged(); } );
}

//---

void
CQChartsPointAnnotation::
init(const Symbol &symbol)
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setSymbol(symbol);
}

//---

void
CQChartsPointAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position" , "", "Point position");
  addProp(model, path1, "shapeType", "", "Shape type");
  addProp(model, path1, "objRef"   , "", "Object reference");

  //---

  auto symbolPath = path1 + "/symbol";

  addProp(model, symbolPath, "symbol"    , "symbol", "Point symbol");
  addProp(model, symbolPath, "symbolSize", "size"  , "Point symbol size");

  //---

  auto fillPath = path1 + "/fill";

//addStyleProp(model, fillPath, "symbolFilled"     , "visible", "Point symbol fill visible");
  addStyleProp(model, fillPath, "symbolFillColor"  , "color"  , "Point symbol fill color");
  addStyleProp(model, fillPath, "symbolFillAlpha"  , "alpha"  , "Point symbol fill alpha");
  addStyleProp(model, fillPath, "symbolFillPattern", "pattern", "Point symbol fill pattern");

  //---

  auto strokePath = path1 + "/stroke";

//addStyleProp(model, strokePath, "symbolStroked"    , "visible", "Point symbol stroke visible");
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

  double dx = bbox.getXMin() - annotationBBox().getXMin();
  double dy = bbox.getYMin() - annotationBBox().getYMin();

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

  bool isSolid = symbolData.symbol().isFilled();

  if (isSolid) {
    setPenBrush(penBrush,
      PenData  (true, lineColor, strokeData.alpha(), strokeData.width(), strokeData.dash()),
      BrushData(true, fillColor, fillData.alpha(), fillData.pattern()));
  }
  else {
    setPenBrush(penBrush,
      PenData  (true , fillColor, fillData.alpha(), strokeData.width(), strokeData.dash()),
      BrushData(false, fillColor, fillData.alpha(), fillData.pattern()));
  }

  updatePenBrushState(penBrush,
    (isSolid ? CQChartsObjDrawType::SYMBOL : CQChartsObjDrawType::LINE));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  //---

  // draw symbol
  auto symbol = symbolData.symbol();

  if (! symbol.isValid())
    symbol = Symbol::circle();

  auto rect = annotationBBox();

  Point ps(rect.getXMid(), rect.getYMid());

  if      (shapeType() == ShapeType::CORNER_HANDLE) {
    QPainterPath path;

    CQChartsDrawUtil::cornerHandlePath(device, path, ps);

    device->drawPath(path);
  }
  else if (shapeType() == ShapeType::RESIZE_HANDLE) {
    QPainterPath path;

    CQChartsDrawUtil::resizeHandlePath(device, path, ps);

    device->drawPath(path);
  }
  else if (shapeType() == ShapeType::EXTRA_HANDLE) {
    QPainterPath path;

    CQChartsDrawUtil::extraHandlePath (device, path, ps);

    device->drawPath(path);
  }
  else {
    CQChartsDrawUtil::drawSymbol(device, symbol, ps, symbolData.size(), /*scale*/true);
  }

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

  if (symbol().isValid())
    os << " -symbol {" << symbol().toString().toStdString() << "}";

  if (symbolData.size().isSet())
    os << " -size {" << symbolData.size().toString().toStdString() << "}";

  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsPieSliceAnnotation::
CQChartsPieSliceAnnotation(View *view, const Position &position, const Length &innerRadius,
                           const Length &outerRadius, const Angle &startAngle,
                           const Angle &spanAngle) :
 CQChartsShapeAnnotationBase(view, Type::PIE_SLICE), position_(position),
 innerRadius_(innerRadius), outerRadius_(outerRadius),
 startAngle_(startAngle), spanAngle_(spanAngle)
{
  init();
}

CQChartsPieSliceAnnotation::
CQChartsPieSliceAnnotation(Plot *plot, const Position &position, const Length &innerRadius,
                           const Length &outerRadius, const Angle &startAngle,
                           const Angle &spanAngle) :
 CQChartsShapeAnnotationBase(plot, Type::PIE_SLICE), position_(position),
 innerRadius_(innerRadius), outerRadius_(outerRadius),
 startAngle_(startAngle), spanAngle_(spanAngle)
{
  init();
}

CQChartsPieSliceAnnotation::
~CQChartsPieSliceAnnotation()
{
}

//---

void
CQChartsPieSliceAnnotation::
setPosition(const Position &p)
{
  CQChartsUtil::testAndSet(position_, p, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
setInnerRadius(const Length &r)
{
  CQChartsUtil::testAndSet(innerRadius_, r, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
setOuterRadius(const Length &r)
{
  CQChartsUtil::testAndSet(outerRadius_, r, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
setStartAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(startAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
setSpanAngle(const Angle &a)
{
  CQChartsUtil::testAndSet(spanAngle_, a, [&]() { emitDataChanged(); } );
}

void
CQChartsPieSliceAnnotation::
setArcType(const ArcType &t)
{
  CQChartsUtil::testAndSet(arcType_, t, [&]() { emitDataChanged(); } );
}

//---

void
CQChartsPieSliceAnnotation::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);
}

//---

void
CQChartsPieSliceAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position"   , "", "Pie slice position");
  addProp(model, path1, "innerRadius", "", "Pie slice inner radius");
  addProp(model, path1, "outerRadius", "", "Pie slice outer radius");
  addProp(model, path1, "startAngle" , "", "Pie slice start angle");
  addProp(model, path1, "spanAngle"  , "", "Pie slice span angle");
  addProp(model, path1, "arcType"    , "", "Pie slice arc type");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPieSliceAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  auto p = positionToParent(objRef(), position());

  auto rect = annotationBBox();

  double dx = bbox.getXMin() - rect.getXMin();
  double dy = bbox.getYMin() - rect.getYMin();

  p += Point(dx, dy);

  position_ = Position::plot(p);

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

  double ri = CMathUtil::avg(xri, yri);
  double ro = CMathUtil::avg(xro, yro);

  QPainterPath path;

  if      (arcType() == ArcType::SLICE) { // works for zero innter radius ?
    auto a2 = startAngle() + spanAngle();

    CQChartsDrawUtil::pieSlicePath(path, c, ri, ro, startAngle(), a2, false, false);

    if (CQChartsAngle::isCircle(startAngle(), a2))
      CQChartsDrawUtil::drawPieSlice(device, c, ri, ro, startAngle(), a2, false, false);
    else
      device->drawPath(path);
  }
  else if (arcType() == ArcType::SEGMENT) {
    BBox ibbox(c.x - ri, c.y - ri, c.x + ri, c.x + ri);
    BBox obbox(c.x - ro, c.y - ro, c.x + ro, c.x + ro);

    CQChartsDrawUtil::arcSegmentPath(path, ibbox, obbox, startAngle(), spanAngle());

    device->drawPath(path);
  }
  else if (arcType() == ArcType::ARC) {
    BBox bbox(c.x - ro, c.y - ro, c.x + ro, c.x + ro);

    CQChartsDrawUtil::arcPath(path, bbox, startAngle(), spanAngle());

    device->drawPath(path);
  }

  //---

  drawText(device, BBox(path.boundingRect()));

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
 CQChartsAnnotation(plot, Type::AXIS), direction_(direction), start_(start), end_(end)
{
  init();

  axis_ = std::make_unique<Axis>(plot_, direction_, start_, end_);

  axis_->setAnnotation     (true);
  axis_->setAllowHtmlLabels(true);

  connectAxis(true);
}

CQChartsAxisAnnotation::
~CQChartsAxisAnnotation()
{
}

void
CQChartsAxisAnnotation::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (false);
}

//---

void
CQChartsAxisAnnotation::
connectAxis(bool b)
{
  CQChartsWidgetUtil::connectDisconnect(b,
    axis_.get(), SIGNAL(ticksChanged()), this, SLOT(emitDataChanged()));
  CQChartsWidgetUtil::connectDisconnect(b,
    axis_.get(), SIGNAL(appearanceChanged()), this, SLOT(invalidateSlot()));
}

//---

void
CQChartsAxisAnnotation::
setDirection(Qt::Orientation &d)
{
  direction_ = d;

  axis_->setDirection(direction_);
}

void
CQChartsAxisAnnotation::
setPosition(double r)
{
  position_ = r;

  axis_->setPosition(CQChartsOptReal(position_));
}

void
CQChartsAxisAnnotation::
setStart(double r)
{
  start_ = r;

  axis_->setStart(start_);
}

void
CQChartsAxisAnnotation::
setEnd(double r)
{
  end_ = r;

  axis_->setEnd(end_);
}

//---

void
CQChartsAxisAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addStrokeProperties(model, path1 + "/stroke", /*isSolid*/false);

  axis_->addProperties(model, path1, CQChartsAxis::PropertyType::ANNOTATION);

  addProp(model, path1, "direction", "", "Axis direction");
  addProp(model, path1, "start"    , "", "Axis start");
  addProp(model, path1, "end"      , "", "Axis end");
  addProp(model, path1, "position" , "", "Axis position");
  addProp(model, path1, "objRef"   , "", "Object reference");
}

//---

void
CQChartsAxisAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  double dx1 = bbox.getXMin() - bbox_.getXMin();
  double dy1 = bbox.getYMin() - bbox_.getYMin();
  double dx2 = bbox.getXMax() - bbox_.getXMax();
  double dy2 = bbox.getYMax() - bbox_.getYMax();

  if (direction_ == Qt::Horizontal) {
    start_    += dx1;
    end_      += dx2;
    position_ += dy1;
  }
  else {
    start_    += dy1;
    end_      += dy2;
    position_ += dx1;
  }

  updateAxis();

  setAnnotationBBox(bbox);
}

//---

bool
CQChartsAxisAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsAxisAnnotation::
draw(PaintDevice *device)
{
  if (! isVisible())
    return;

  if (objRef_.isValid())
    updateAxis();

  //---

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
updateAxis()
{
  Point p1, p2;

  if (direction_ == Qt::Horizontal) {
    p1 = Point(start_, position_);
    p2 = Point(end_  , position_);
  }
  else {
    p1 = Point(position_, start_);
    p2 = Point(position_, end_  );
  }

  auto ll = CQChartsObjRef(objRef().name(), CQChartsObjRef::Location::LL);
  auto ur = CQChartsObjRef(objRef().name(), CQChartsObjRef::Location::UR);

  auto start = positionToParent(ll, Position(p1, parentUnits()));
  auto end   = positionToParent(ur, Position(p2, parentUnits()));

  connectAxis(false);

  axis_->setUpdatesEnabled(false);

  if (direction_ == Qt::Horizontal) {
    axis_->setStart   (start.x);
    axis_->setEnd     (end  .x);
    axis_->setPosition(OptReal(start.y));
  }
  else {
    axis_->setStart   (start.y);
    axis_->setEnd     (end  .y);
    axis_->setPosition(OptReal(start.x));
  }

  axis_->updateCalc();

  axis_->setUpdatesEnabled(true);

  connectAxis(true);
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
   CQChartsColumnKey(plot, column), annotation_(annotation) {
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);
}

CQChartsColumn
CQChartsKeyAnnotation::
column() const
{
  auto columnKey = dynamic_cast<CQChartsKeyAnnotationColumnKey *>(key_);

  return (columnKey ? columnKey->column() : Column());
}

void
CQChartsKeyAnnotation::
setColumn(const Column &c)
{
  auto columnKey = dynamic_cast<CQChartsKeyAnnotationColumnKey *>(key_);

  if (columnKey)
    columnKey->setColumn(c);
}

//--

void
CQChartsKeyAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  //---

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "objRef", "", "Object reference");

  addStrokeProperties(model, path1 + "/stroke");

  //---

  auto keyPath = path1 + "/key";

  key_->addProperties(model, keyPath, "");
}

void
CQChartsKeyAnnotation::
updateLocationSlot()
{
  auto *plotKey = qobject_cast<CQChartsPlotKey *>(key_);

  if (plotKey)
    plotKey->updatePlotLocation();
}

//---

bool
CQChartsKeyAnnotation::
editMove(const Point &p)
{
  auto *plotKey = qobject_cast<CQChartsPlotKey *>(key_);

  if (plotKey) {
    const auto &dragPos  = editHandles()->dragPos();
    const auto &dragSide = editHandles()->dragSide();

    if (dragSide == CQChartsResizeSide::NONE)
      return false;

    double dx = p.x - dragPos.x;
    double dy = p.y - dragPos.y;

    editHandles()->updateBBox(dx, dy);

    if (dragSide == ResizeSide::MOVE)
      plotKey->editDragMove(Point(dx, dy));

    setEditBBox(editHandles()->bbox(), dragSide);

    editHandles()->setDragPos(p);

    invalidate();

    return true;
  }
  else
    return CQChartsAnnotation::editMove(p);
}

//---

bool
CQChartsKeyAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

bool
CQChartsKeyAnnotation::
selectPress(const Point &w, SelData &selData)
{
  if (key_->selectPress(w, selData)) {
    emit pressed(QString("%1:%2").arg(id()).arg(key_->id()));
    return true;
  }

  auto *plotKey = dynamic_cast<CQChartsPlotKey *>(key_);

  if (plotKey->contains(w)) {
    auto *item = plotKey->getItemAt(w);

    if (item) {
      bool handled = item->selectPress(w, selData);

      if (handled) {
        selData.select = false;
        emit pressed(QString("%1:%2:%3").arg(id()).arg(key_->id()).arg(item->id()));
        return true;
      }
    }
  }

  return CQChartsAnnotation::selectPress(w, selData);
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
CQChartsPointSetAnnotation(View *view, const Rect &rectangle, const Points &points) :
 CQChartsShapeAnnotationBase(view, Type::POINT_SET),
 CQChartsObjPointData<CQChartsPointSetAnnotation>(this),
 rectangle_(rectangle), values_(points)
{
  init();
}

CQChartsPointSetAnnotation::
CQChartsPointSetAnnotation(Plot *plot, const Rect &rectangle, const Points &points) :
 CQChartsShapeAnnotationBase(plot, Type::POINT_SET),
 CQChartsObjPointData<CQChartsPointSetAnnotation>(this),
 rectangle_(rectangle), values_(points)
{
  init();
}

CQChartsPointSetAnnotation::
~CQChartsPointSetAnnotation()
{
}

//---

void
CQChartsPointSetAnnotation::
init()
{
  assert(rectangle_.isValid());

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);

  updateValues();

  //---

  setAnnotationBBox(rectangle_.bbox());
}

//---

void
CQChartsPointSetAnnotation::
setRectangle(const Rect &rectangle)
{
  CQChartsUtil::testAndSet(rectangle_, rectangle, [&]() {
    setAnnotationBBox(rectangle_.bbox()); emitDataChanged();
  } );
}

void
CQChartsPointSetAnnotation::
setValues(const Points &values)
{
  values_  = values;
  xColumn_ = CQChartsModelColumn();
  yColumn_ = CQChartsModelColumn();

  updateValues();

  emitDataChanged();
}

void
CQChartsPointSetAnnotation::
setXColumn(const ModelColumn &c)
{
  CQChartsUtil::testAndSet(xColumn_, c, [&]() {
    xColumn_ = c; xColumn_.setCharts(charts()); updateValues(); emitDataChanged();
  } );
}

void
CQChartsPointSetAnnotation::
setYColumn(const ModelColumn &c)
{
  CQChartsUtil::testAndSet(yColumn_, c, [&]() {
    yColumn_ = c; yColumn_.setCharts(charts()); updateValues(); emitDataChanged();
  } );
}

//---

void
CQChartsPointSetAnnotation::
updateValues()
{
  calcValues();

  for (auto &p : values_.points()) {
    auto p1 = positionToParent(objRef(), p);

    xrange_.add(p1.x);
    yrange_.add(p1.y);
  }

  hullDirty_ = true;
  gridDirty_ = true;
}

void
CQChartsPointSetAnnotation::
calcValues()
{
  if (! xColumn_.isValid() || ! yColumn_.isValid())
    return;

  class ModelVisitor : public CQModelVisitor {
   public:
    ModelVisitor(const CQChartsPointSetAnnotation *annotation, const CQChartsColumn &xColumn,
                 const CQChartsColumn &yColumn) :
     annotation_(annotation), xColumn_(xColumn), yColumn_(yColumn) {
      charts_ = annotation_->charts();
    }

    State visit(const QAbstractItemModel *model, const VisitData &data) override {
      bool ok1, ok2;

      auto x = CQChartsModelUtil::modelReal(charts_, model, data.row,
                                            xColumn_, data.parent, ok1);
      auto y = CQChartsModelUtil::modelReal(charts_, model, data.row,
                                            yColumn_, data.parent, ok2);
      if (! ok1 || ! ok2) return State::SKIP;

      points_.addPoint(CQChartsPosition::plot(Point(x, y)));

      return State::OK;
    }

    const CQChartsPoints &points() const { return points_; }

   private:
    const CQChartsPointSetAnnotation *annotation_ { nullptr };
    CQChartsColumn                    xColumn_;
    CQChartsColumn                    yColumn_;
    CQCharts*                         charts_     { nullptr };
    CQChartsPoints                    points_;
  };

  if (xColumn_.modelInd() == yColumn_.modelInd()) {
    ModelVisitor visitor(this, xColumn_.column(), yColumn_.column());

    auto *modelData = charts()->getModelDataByInd(xColumn_.modelInd());

    CQModelVisit::exec(modelData->model().data(), visitor);

    values_ = visitor.points();
  }
}

//---

void
CQChartsPointSetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "rectangle", "", "Bounding rectangle");
  addProp(model, path1, "values"   , "", "Values");
  addProp(model, path1, "xColumn"  , "", "X Column");
  addProp(model, path1, "yColumn"  , "", "Y Column");
  addProp(model, path1, "drawType" , "", "Draw type");

  //---

  addStrokeFillProperties(model, path1);

  //---

  auto symbolPath = path1 + "/symbol";

  addProp(model, symbolPath, "symbol"    , "symbol", "Point symbol");
  addProp(model, symbolPath, "symbolSize", "size"  , "Point symbol size");
}

//---

void
CQChartsPointSetAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  rectangle_ = Rect(bbox, parentUnits());

  setAnnotationBBox(rectangle_.bbox());

  hullDirty_     = true;
  gridDirty_     = true;
  delaunayDirty_ = true;
}

//---

bool
CQChartsPointSetAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsPointSetAnnotation::
draw(PaintDevice *device)
{
  assert(plot());

  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::SYMBOL);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if      (drawType() == DrawType::SYMBOLS)
    drawSymbols(device);
  else if (drawType() == DrawType::HULL)
    drawHull(device);
  else if (drawType() == DrawType::DENSITY)
    drawDensity(device);
  else if (drawType() == DrawType::BEST_FIT)
    drawBestFit(device);
  else if (drawType() == DrawType::GRID)
    drawGrid(device);
  else if (drawType() == DrawType::DELAUNAY)
    drawDelaunay(device);

  //---

  drawTerm(device);
}

// draw point symbols (scatter/xy)
void
CQChartsPointSetAnnotation::
drawSymbols(PaintDevice *device)
{
  auto bbox = rectangle_.bbox();

  const auto &symbolData = this->symbolData();

  auto symbol = symbolData.symbol();

  if (! symbol.isValid())
    symbol = CQChartsSymbol::circle();

  for (const auto &p : values_.points()) {
    auto p1 = positionToParent(objRef(), p);

    auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
    auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

    CQChartsDrawUtil::drawSymbol(device, symbol, Point(x1, y1), symbolData.size(), /*scale*/true);
  }
}

void
CQChartsPointSetAnnotation::
drawHull(PaintDevice *device)
{
  if (! hull_)
    hull_ = std::make_unique<Hull>();

  if (hullDirty_) {
    auto bbox = rectangle_.bbox();

    hull_->clear();

    for (auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
      auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

      hull_->addPoint(Point(x1, y1));
    }

    hullDirty_ = false;
  }

  hull_->draw(device);
}

void
CQChartsPointSetAnnotation::
drawDensity(PaintDevice *device)
{
  if (! plot_) return;

  auto bbox = rectangle_.bbox();

  CQChartsBivariateDensity density;

  CQChartsBivariateDensity::Data data;

  data.gridSize = 16;
  data.delta    = 0.0;

  for (const auto &p : values_.points()) {
    auto p1 = positionToParent(objRef(), p);

    auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
    auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

    data.values.emplace_back(x1, y1);
  }

  CQChartsGeom::RMinMax xrange;
  CQChartsGeom::RMinMax yrange;

  xrange.add(bbox.getXMin()); xrange.add(bbox.getXMax());
  yrange.add(bbox.getYMin()); yrange.add(bbox.getYMax());

  data.xrange = xrange;
  data.yrange = yrange;

  density.calc(plot_, data);

  density.draw(plot_, device);
}

void
CQChartsPointSetAnnotation::
drawBestFit(PaintDevice *device)
{
  auto bbox = rectangle_.bbox();

  CQChartsFitData fitData;

  Polygon poly;

  for (const auto &p : values_.points()) {
    auto p1 = positionToParent(objRef(), p);

    auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
    auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

    poly.addPoint(Point(x1, y1));
  }

  fitData.calc(poly, 3);

  auto p1 = device->windowToPixel(Point(bbox.getXMin(), bbox.getYMin()));
  auto p2 = device->windowToPixel(Point(bbox.getXMax(), bbox.getYMax()));

  double dx = (p2.x - p1.x)/100.0;

  Polygon fitPoly;

  for (int i = 0; i <= 100; ++i) {
    double px = p1.x + i*dx;

    auto p = device->pixelToWindow(Point(px, 0.0));

    double y = fitData.interp(p.x);

    fitPoly.addPoint(Point(p.x, y));
  }

  auto path = CQChartsDrawUtil::polygonToPath(fitPoly, /*closed*/false);

  device->strokePath(path, device->pen());
}

void
CQChartsPointSetAnnotation::
drawGrid(PaintDevice *device)
{
  auto bbox = rectangle_.bbox();

  if (! gridCell_)
    gridCell_ = std::make_unique<GridCell>();

  if (gridDirty_) {
    gridCell_->setNX(40);
    gridCell_->setNY(40);

    gridCell_->setXInterval(xrange_.min(), xrange_.max());
    gridCell_->setYInterval(yrange_.min(), yrange_.max());

    gridCell_->resetPoints();

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
      auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

      gridCell_->addPoint(Point(x1, y1));
    }

    gridDirty_ = true;
  }

  int maxN = gridCell_->maxN();

  //---

  auto strokeColor = interpStrokeColor(ColorInd());

  PenData penData1(isStroked(), strokeColor, strokeAlpha(), strokeWidth(),
                   strokeDash(), strokeCap(), strokeJoin());

  //---

  for (const auto &px : gridCell_->xyPoints()) {
    int         ix      = px.first;
    const auto &yPoints = px.second;

    double xmin, xmax;

    gridCell_->xIValues(ix, xmin, xmax);

    for (const auto &py : yPoints) {
      int         iy     = py.first;
      const auto &points = py.second;

      auto n = points.size();
      if (n <= 0) continue;

      double ymin, ymax;

      gridCell_->yIValues(iy, ymin, ymax);

      //---

      BBox bbox1(xmin, ymin, xmax, ymax);

      ColorInd colorInd(CMathUtil::map(double(n), 1.0, 1.0*maxN, 0.0, 1.0));

      auto bgColor = view()->interpColor(Color::makePalette(), colorInd);

      // set pen and brush
      PenBrush penBrush1;

      setPenBrush(penBrush1, penData1, BrushData(true, bgColor));

      CQChartsDrawUtil::setPenBrush(device, penBrush1);

      // draw rect
      device->drawRect(bbox1);
    }
  }
}

void
CQChartsPointSetAnnotation::
drawDelaunay(PaintDevice *device)
{
  if (! delaunay_) {
    auto *th = const_cast<CQChartsPointSetAnnotation *>(this);

    th->delaunay_ = std::make_unique<CQChartsDelaunay>();
  }

  if (delaunayDirty_) {
    auto bbox = rectangle_.bbox();

    delaunay_->clear();

    for (const auto &p : values_.points()) {
      auto p1 = positionToParent(objRef(), p);

      auto x1 = CMathUtil::map(p1.x, xrange_.min(), xrange_.max(), bbox.getXMin(), bbox.getXMax());
      auto y1 = CMathUtil::map(p1.y, yrange_.min(), yrange_.max(), bbox.getYMin(), bbox.getYMax());

      delaunay_->addVertex(x1, y1);
    }

    delaunay_->calc();

   delaunayDirty_ = false;
  }

  // draw delaunay triangles
  for (auto pf = delaunay_->facesBegin(); pf != delaunay_->facesEnd(); ++pf) {
    const auto *f = *pf;

    if (! f->isLower()) continue;

    auto *v1 = f->vertex(0);
    auto *v2 = f->vertex(1);
    auto *v3 = f->vertex(2);

    Point p1(v1->x(), v1->y());
    Point p2(v2->x(), v2->y());
    Point p3(v3->x(), v3->y());

    QPainterPath path;

    CQChartsDrawUtil::trianglePath(path, p1, p2, p3);

    device->strokePath(path, device->pen());
  }
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

CQChartsPoint3DSetAnnotation::
CQChartsPoint3DSetAnnotation(View *view, const Points &points) :
 CQChartsShapeAnnotationBase(view, Type::POINT_SET), points_(points)
{
  init();
}

CQChartsPoint3DSetAnnotation::
CQChartsPoint3DSetAnnotation(Plot *plot, const Points &points) :
 CQChartsShapeAnnotationBase(plot, Type::POINT3D_SET), points_(points)
{
  init();
}

CQChartsPoint3DSetAnnotation::
~CQChartsPoint3DSetAnnotation()
{
}

void
CQChartsPoint3DSetAnnotation::
init()
{
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);

  updateValues();
}

//---

void
CQChartsPoint3DSetAnnotation::
updateValues()
{
  for (auto &p : points_) {
    xvals_.addValue(p.x);
    yvals_.addValue(p.y);
    zvals_.addValue(p.z);
  }
}

//---

void
CQChartsPoint3DSetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "drawType", "", "Draw type");

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsPoint3DSetAnnotation::
setEditBBox(const BBox &, const ResizeSide &)
{
#if 0
  // TODO: move all points ?
#endif
}

//---

bool
CQChartsPoint3DSetAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsPoint3DSetAnnotation::
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

    auto symbol = symbolData.symbol();

    if (! symbol.isValid())
      symbol = CQChartsSymbol::circle();

    for (const auto &p : points()) {
      auto p1 = CQChartsGeom::Point(p.x, p.y);

      CQChartsDrawUtil::drawSymbol(device, symbol, Point(p1), symbolData.size(), /*scale*/true);
    }
  }
  else if (drawType() == DrawType::CONTOUR) {
    if (! contour_)
      initContour();

    contour_->drawContour(device);
  }

  //---

  drawTerm(device);
}

void
CQChartsPoint3DSetAnnotation::
initContour() const
{
  uint nx = uint(xvals_.numUnique());
  uint ny = uint(yvals_.numUnique());
  uint nz = nx*ny;

  std::vector<double> x, y, z;

  x.resize(nx);
  y.resize(ny);
  z.resize(nz);

  for (uint ix = 0; ix < nx; ++ix)
    x[ix] = xvals_.ivalue(int(ix));

  for (uint iy = 0; iy < ny; ++iy)
    y[iy] = yvals_.ivalue(int(iy));

  for (uint iz = 0; iz < nz; ++iz)
    CMathUtil::isNaN(z[iz]);

  for (const auto &p : points_) {
    uint ix = uint(xvals_.id(p.x));
    uint iy = uint(yvals_.id(p.y));

    z[iy*nx + ix] = p.z;
  }

  auto *th = const_cast<CQChartsPoint3DSetAnnotation *>(this);

  if (! contour_) {
    th->contour_ = std::make_unique<CQChartsContour>();

    th->contour_->setPlot(plot());
  }

  th->contour_->setData(&x[0], &y[0], &z[0], int(nx), int(ny));
}

void
CQChartsPoint3DSetAnnotation::
writeDetails(std::ostream &os, const QString &, const QString &varName) const
{
  os << "]\n";

  //---

  writeProperties(os, varName);
}

//------

CQChartsValueSetAnnotation::
CQChartsValueSetAnnotation(View *view, const Rect &rectangle, const Reals &reals) :
 CQChartsShapeAnnotationBase(view, Type::VALUE_SET), rectangle_(rectangle), reals_(reals)
{
  init();
}

CQChartsValueSetAnnotation::
CQChartsValueSetAnnotation(Plot *plot, const Rect &rectangle, const Reals &reals) :
 CQChartsShapeAnnotationBase(plot, Type::VALUE_SET), rectangle_(rectangle), reals_(reals)
{
  init();
}

CQChartsValueSetAnnotation::
~CQChartsValueSetAnnotation()
{
}

//---

void
CQChartsValueSetAnnotation::
init()
{
  assert(rectangle_.isValid());

  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  setStroked(true);
  setFilled (true);

  updateValues();

  //---

  setAnnotationBBox(rectangle_.bbox());
}

//---

void
CQChartsValueSetAnnotation::
setRectangle(const Rect &rectangle)
{
  CQChartsUtil::testAndSet(rectangle_, rectangle, [&]() {
    setAnnotationBBox(rectangle_.bbox()); emitDataChanged();
  } );
}

void
CQChartsValueSetAnnotation::
setValues(const Reals &values)
{
  reals_       = values;
  modelColumn_ = CQChartsModelColumn();

  updateValues();

  emitDataChanged();
}

void
CQChartsValueSetAnnotation::
setModelColumn(const ModelColumn &c)
{
  CQChartsUtil::testAndSet(modelColumn_, c, [&]() {
    modelColumn_.setCharts(charts()); updateValues(); emitDataChanged();
  } );
}

//---

void
CQChartsValueSetAnnotation::
updateValues()
{
  if (! density_) {
    density_ = std::make_unique<CQChartsDensity>();

    density_->setDrawType(CQChartsDensity::DrawType::WHISKER);

    connect(density_.get(), SIGNAL(dataChanged()), this, SLOT(invalidateSlot()));
  }

  calcReals();

  density_->setXVals(reals_.reals());
}

void
CQChartsValueSetAnnotation::
calcReals()
{
  if (! modelColumn_.isValid())
    return;

  class ModelVisitor : public CQModelVisitor {
   public:
    ModelVisitor(const CQChartsValueSetAnnotation *annotation, const CQChartsColumn &column) :
     annotation_(annotation), column_(column) {
      charts_ = annotation_->charts();
    }

    State visit(const QAbstractItemModel *model, const VisitData &data) override {
      bool ok;

      auto value = CQChartsModelUtil::modelReal(charts_, model, data.row,
                                                column_, data.parent, ok);

      if (ok)
        reals_.addReal(value);

      return State::OK;
    }

    const CQChartsReals &reals() const { return reals_; }

   private:
    const CQChartsValueSetAnnotation *annotation_ { nullptr };
    CQChartsColumn                    column_;
    CQCharts*                         charts_     { nullptr };
    CQChartsReals                     reals_;
  };

  ModelVisitor visitor(this, modelColumn_.column());

  auto *modelData = charts()->getModelDataByInd(modelColumn_.modelInd());
  if (! modelData) return;

  CQModelVisit::exec(modelData->model().data(), visitor);

  reals_ = visitor.reals();
}

//---

void
CQChartsValueSetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsShapeAnnotationBase::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "rectangle"  , "", "Rectangle");
  addProp(model, path1, "values"     , "", "Values");
  addProp(model, path1, "modelColumn", "", "Model Column");
  addProp(model, path1, "drawType"   , "", "Draw Type");

  //---

  auto densityPath = path1 + "/density";

  auto addDensityProp = [&](const QString &name, const QString &alias, const QString &desc) {
    auto *item = model->addProperty(densityPath, density_.get(), name, alias);
    item->setDesc(desc);
    return item;
  };

  addDensityProp("numSamples"     , "", "Number of samples");
  addDensityProp("smoothParameter", "", "Smooth parameter");
  addDensityProp("drawType"       , "", "Density draw types");
  addDensityProp("orientation"    , "", "Density orientation");

  //---

  addStrokeFillProperties(model, path1);
}

//---

void
CQChartsValueSetAnnotation::
setEditBBox(const BBox &bbox, const ResizeSide &)
{
  rectangle_ = Rect(bbox, parentUnits());

  setAnnotationBBox(rectangle_.bbox());
}

//---

bool
CQChartsValueSetAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
}

void
CQChartsValueSetAnnotation::
draw(PaintDevice *device)
{
  assert(plot());

  drawInit(device);

  //---

  // set pen and brush
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::SYMBOL);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  if      (drawType() == DrawType::BARCHART)
    drawBarChart(device);
  else if (drawType() == DrawType::DENSITY) // box
    drawBox(device);
  else if (drawType() == DrawType::BUBBLE)
    drawBubble(device);
  else if (drawType() == DrawType::PIE)
    drawPie(device, penBrush.pen);
  else if (drawType() == DrawType::RADAR)
    drawRadar(device);
  else if (drawType() == DrawType::TREEMAP)
    drawTreeMap(device, penBrush.pen);

  //---

  drawTerm(device);
}

// draw bar chart
void
CQChartsValueSetAnnotation::
drawBarChart(PaintDevice *device)
{
  // draw bar for each value
//double ymin = density_->xmin();
  double ymax = density_->xmax();

  int nx = reals_.numReals();

  auto bbox = rectangle_.bbox();

  double width = bbox.getWidth();

  double dx = (nx > 0 ? width/nx : 0.0);

  double x = bbox.getXMin();
  double y = bbox.getYMin();

  auto color = Color::makePalette();

  color.setPaletteName(calcPaletteName());

  auto strokeColor = interpStrokeColor(ColorInd());

  PenData penData(isStroked(), strokeColor, strokeAlpha(), strokeWidth(),
                  strokeDash(), strokeCap(), strokeJoin());

  for (int ix = 0; ix < nx; ++ix) {
    PenBrush penBrush;

    plot_->setPenBrush(penBrush,
      penData, BrushData(true, plot_->interpColor(color, ColorInd(ix, nx))));

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    //---

    double y1 = CMathUtil::map(reals_.real(ix), 0.0, ymax, bbox.getYMin(), bbox.getYMax());

    BBox bbox1(x, y, x + dx, y1);

    device->drawRect(bbox1);

    x += dx;
  }
}

// draw box plot (density)
void
CQChartsValueSetAnnotation::
drawBox(PaintDevice *device)
{
  //setAnnotationBBox(density_->bbox(bbox));

  auto bbox = rectangle_.bbox();

  density_->draw(plot(), device, bbox, /*scaled*/true);
}

// draw bubbles
void
CQChartsValueSetAnnotation::
drawBubble(PaintDevice *device)
{
  if (! circlePack_) {
    circlePack_ = std::make_unique<CirclePack>();

    for (const auto &r : reals_.reals()) {
      auto *node = new CQChartsCircleNode(r, 0.0, 0.0);

      circlePack_->addNode(node);
    }
  }

  auto bbox = rectangle_.bbox();

  double xmin, ymin, xmax, ymax;

  circlePack_->boundingBox(xmin, ymin, xmax, ymax);

  double xs = (xmax > xmin ? bbox.getWidth ()/(xmax - xmin) : 1.0);
  double ys = (ymax > ymin ? bbox.getHeight()/(ymax - ymin) : 1.0);

  double xc1 = bbox.getXMid();
  double yc1 = bbox.getYMid();

  double xc2 = CMathUtil::avg(xmin, xmax);
  double yc2 = CMathUtil::avg(ymin, ymax);

  int ix = 0;
  int nx = reals_.numReals();

  auto color = Color::makePalette();

  color.setPaletteName(calcPaletteName());

  auto strokeColor = interpStrokeColor(ColorInd());

  PenData penData(isStroked(), strokeColor, strokeAlpha(), strokeWidth(),
                  strokeDash(), strokeCap(), strokeJoin());

  for (const auto &node : circlePack_->nodes()) {
    PenBrush penBrush;

    plot_->setPenBrush(penBrush,
      penData, BrushData(true, plot_->interpColor(color, ColorInd(ix, nx))));

    CQChartsDrawUtil::setPenBrush(device, penBrush);

    //---

    double x  = xs*(node->x() - xc2) + xc1;
    double y  = ys*(node->y() - yc2) + yc1;
    double xr = xs*node->radius();
    double yr = ys*node->radius();

    BBox nbbox(x - xr, y - yr, x + xr, y + yr);

    device->drawEllipse(nbbox);

    ++ix;
  }
}

// draw pie chart
void
CQChartsValueSetAnnotation::
drawPie(PaintDevice *device, const QPen &pen)
{
  CQChartsRValues values;

  for (const auto &r : reals_.reals())
    values.addValue(r);

  auto bbox = rectangle_.bbox();

  CQChartsPlotDrawUtil::drawPie(const_cast<CQChartsPlot *>(plot()), device,
                                values, bbox, calcPalette(), pen);
}

// draw radar
void
CQChartsValueSetAnnotation::
drawRadar(PaintDevice *)
{
  // TODO: multiple sets of values. each values normalized to max
}

// draw tree map
void
CQChartsValueSetAnnotation::
drawTreeMap(PaintDevice *device, const QPen &pen)
{
  CQChartsRValues values;

  for (const auto &r : reals_.reals())
    values.addValue(r);

  auto bbox = rectangle_.bbox();

  CQChartsPlotDrawUtil::drawTreeMap(const_cast<CQChartsPlot *>(plot()), device,
                                    values, bbox, calcPalette(), pen);
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setTextStr  (textStr);
  setTextColor(Color::makeInterfaceValue(1.0));
}

//---

void
CQChartsButtonAnnotation::
setPosition(const Position &p)
{
  position_ = p;

  emitDataChanged();
}

//---

void
CQChartsButtonAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position", "", "Button origin");
  addProp(model, path1, "objRef"  , "", "Object reference");

  //---

  auto textPath = path1 + "/text";

  addProp(model, textPath, "textStr", "string", "Text string");

  addTextProperties(model, textPath, TextOptions::ValueType::NONE);

  //---

  addStrokeFillProperties(model, path1);
}

bool
CQChartsButtonAnnotation::
selectPress(const Point &, SelData &)
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

  setAnnotationBBox(pixelToWindow(BBox(prect_)));

  //---

  if (device->isInteractive()) {
    // TODO : support text color, fill color, ... ?
    auto font = calcFont(textFont());

    device->setFont(font);

    CQChartsDrawUtil::ButtonData buttonData;

    buttonData.pressed   = pressed_;
    buttonData.enabled   = isEnabled();
    buttonData.checkable = isCheckable();
    buttonData.checked   = isChecked();
    buttonData.inside    = isInside();

    CQChartsDrawUtil::drawPushButton(device, BBox(prect_), textStr(), buttonData);
  }

  //---

  drawTerm(device);
}

void
CQChartsButtonAnnotation::
writeHtml(HtmlPaintDevice *device)
{
  prect_ = calcPixelRect();

  setAnnotationBBox(pixelToWindow(BBox(prect_)));

  device->createButton(annotationBBox(), textStr(), id(), "annotationClick");
}

QRect
CQChartsButtonAnnotation::
calcPixelRect() const
{
  auto font = calcFont(textFont());

  QFontMetrics fm(font);

  int w = fm.horizontalAdvance(textStr()) + 4;
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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  editHandles()->setMode(EditHandles::Mode::RESIZE);

  if (view()->is3D())
    widget_.setParent(view()->glWidget());
  else
    widget_.setParent(view());

  widget_.initSize();
}

//---

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
  if (! p.isSet())
    return;

  assert(p.position().isValid());

  rectangle_ = OptRect();
  position_  = p;

  positionToBBox();

  if (winWidget_) {
    disconnect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));

    auto p = positionToPixel(objRef(), positionValue());

    winWidget_->setPos(int(p.x), int(p.y));

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
  if (r.isSet()) {
    assert(r.rect().isValid());

    position_  = OptPosition();
    rectangle_ = r;
  }
  else {
    if (! position_.isSet()) {
      if (rectangle_.isSet())
        position_  = OptPosition(rectangleValue().center());
      else
        position_  = OptPosition(Position::pixel(Point(0, 0)));
    }

    rectangle_ = OptRect();
  }

  rectToBBox();

  if (winWidget_) {
    disconnect(winWidget_, SIGNAL(geometryChanged()), this, SLOT(updateWinGeometry()));

    BBox bbox;

    if      (plot())
      bbox = plot()->rectToPixel(rectangle_.rect());
    else if (view())
      bbox = view()->rectToPixel(rectangle_.rect());

    winWidget_->setPos (int(bbox.getXMin()), int(bbox.getYMin()));
    winWidget_->setSize(int(bbox.getWidth()), int(bbox.getHeight()));

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

      if (view()->is3D())
        winWidget_->setParent(view()->glWidget());
      else
        winWidget_->setParent(view());

      winWidget_->setVisible(true);
      winWidget_->setClosable(false);
    }
    else {
      if (view()->is3D())
        widget_.setParent(view()->glWidget());
      else
        widget_.setParent(view());

      delete winWidget_;

      winWidget_ = nullptr;
    }
  }
}

//---

void
CQChartsWidgetAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  auto path1 = path + "/" + propertyId();

  addProp(model, path1, "position"   , "", "Widget position");
  addProp(model, path1, "rectangle"  , "", "Widget bounding box");
  addProp(model, path1, "objRef"     , "", "Object reference");
  addProp(model, path1, "widget"     , "", "Widget name");
  addProp(model, path1, "align"      , "", "Widget position alignment");
  addProp(model, path1, "sizePolicy" , "", "Widget size policy");
  addProp(model, path1, "interactive", "", "Widget is interactive");
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

    rectangle_ = OptRect(rect);
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

    auto rect = annotationBBox();

    if (! rect.isValid())
      return;

    //---

    //drawInit(device);

    // get inner padding
    double xlp, xrp, ytp, ybp;

    getPaddingValues(xlp, xrp, ytp, ybp);

    // get outer margin
    double xlm, xrm, ytm, ybm;

    getMarginValues(xlm, xrm, ytm, ybm);

    double tx =          rect.getXMin  () +       xlm + xlp;
    double ty =          rect.getYMin  () +       ybm + ybp;
    double tw = std::max(rect.getWidth () - xlm - xrm - xlp - xrp, 0.0);
    double th = std::max(rect.getHeight() - ybm - ytm - ybp - ytp, 0.0);

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

    auto rect = Rect(annotationBBox(), parentUnits());

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
  setObjectName(QString("%1.%2").arg(typeName()).arg(ind()));

  setStroked(true);
  setFilled (true);

  setTextColor(Color::makeInterfaceValue(1.0));
}

//---

void
CQChartsSymbolSizeMapKeyAnnotation::
addProperties(PropertyModel *model, const QString &path, const QString &desc)
{
  CQChartsAnnotation::addProperties(model, path, desc);

  //---

  auto path1 = path + "/" + propertyId();

  addStrokeFillProperties(model, path1, /*isSolid*/false);

  //---

  auto textPath = path1 + "/text";

  addTextProperties(model, textPath, TextOptions::ValueType::NONE);

  //---

  auto keyPath = path1 + "/key";

  auto addProp = [&](const QString &name, const QString &desc, bool hidden=false) {
    auto *item = model->addProperty(keyPath, this, name);
    item->setDesc(desc);
    if (hidden) CQCharts::setItemIsHidden(item);
    return item;
  };

  addProp("position", "Position");

  //---

  key_->addProperties(model, keyPath);
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
  position_ = Position::plot(bbox.getCenter());
}

//---

bool
CQChartsSymbolSizeMapKeyAnnotation::
inside(const Point &p) const
{
  return CQChartsAnnotation::inside(p);
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

  key_->initDraw(device);

  //---

  // set pen and brush for circles
  PenBrush penBrush;

  calcPenBrush(penBrush);

  updatePenBrushState(penBrush, CQChartsObjDrawType::BOX);

  //---

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  // draw circles
  key_->drawCircles(device, CQChartsMapKey::DrawType::NONE, /*usePenBrush*/true);

  //---

  // set text options for text
  auto textOptions = this->textOptions();

  textOptions.align   = Qt::AlignLeft;
  textOptions.clipped = false;

  adjustTextOptions(textOptions);

  //---

  // set text pen and font
  auto c = interpColor(textColor(), ColorInd());

  setPenBrush(penBrush, PenData(true, c, textAlpha()), BrushData(false));

  CQChartsDrawUtil::setPenBrush(device, penBrush);

  setPainterFont(device, textFont());

  // draw text
  key_->drawContiguousText(device, textOptions, /*usePenBrush*/true);

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
