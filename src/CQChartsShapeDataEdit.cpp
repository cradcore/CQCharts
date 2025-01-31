#include <CQChartsShapeDataEdit.h>
#include <CQChartsStrokeDataEdit.h>
#include <CQChartsFillDataEdit.h>
#include <CQChartsView.h>
#include <CQChartsPlot.h>
#include <CQChartsDrawUtil.h>
#include <CQChartsPixelPaintDevice.h>
#include <CQCharts.h>
#include <CQChartsWidgetUtil.h>

#include <CQPropertyView.h>
#include <CQWidgetMenu.h>
#include <CQUtil.h>

#include <QLabel>
#include <QVBoxLayout>

CQChartsShapeDataLineEdit::
CQChartsShapeDataLineEdit(QWidget *parent) :
 CQChartsLineEditBase(parent)
{
  setObjectName("shapeDataLineEdit");

  setToolTip("Shape Data");

  //---

  menuEdit_ = dataEdit_ = new CQChartsShapeDataEdit;

  menu_->setWidget(dataEdit_);

  //---

  connectSlots(true);

  shapeDataToWidgets();
}

const CQChartsShapeData &
CQChartsShapeDataLineEdit::
shapeData() const
{
  return dataEdit_->data();
}

void
CQChartsShapeDataLineEdit::
setShapeData(const CQChartsShapeData &shapeData)
{
  updateShapeData(shapeData, /*updateText*/ true);
}

void
CQChartsShapeDataLineEdit::
updateShapeData(const CQChartsShapeData &shapeData, bool updateText)
{
  connectSlots(false);

  dataEdit_->setData(shapeData);

  connectSlots(true);

  if (updateText)
    shapeDataToWidgets();

  emit shapeDataChanged();
}

void
CQChartsShapeDataLineEdit::
textChanged()
{
  CQChartsShapeData shapeData(edit_->text());

  if (! shapeData.isValid())
    return;

  updateShapeData(shapeData, /*updateText*/ false);
}

void
CQChartsShapeDataLineEdit::
shapeDataToWidgets()
{
  connectSlots(false);

  if (shapeData().isValid())
    edit_->setText(shapeData().toString());
  else
    edit_->setText("");

  auto tip = QString("%1 (%2)").arg(shapeData().toString());

  edit_->setToolTip(tip);

  connectSlots(true);
}

void
CQChartsShapeDataLineEdit::
menuEditChanged()
{
  shapeDataToWidgets();

  emit shapeDataChanged();
}

void
CQChartsShapeDataLineEdit::
connectSlots(bool b)
{
  connectBaseSlots(b);

  CQChartsWidgetUtil::connectDisconnect(b,
    dataEdit_, SIGNAL(shapeDataChanged()), this, SLOT(menuEditChanged()));
}

void
CQChartsShapeDataLineEdit::
drawPreview(QPainter *painter, const QRect &rect)
{
  CQChartsShapeDataEditPreview::draw(painter, shapeData(), rect, plot(), view());
}

//------

CQPropertyViewEditorFactory *
CQChartsShapeDataPropertyViewType::
getEditor() const
{
  return new CQChartsShapeDataPropertyViewEditor;
}

void
CQChartsShapeDataPropertyViewType::
drawPreview(QPainter *painter, const QRect &rect, const QVariant &value,
            CQChartsPlot *plot, CQChartsView *view)
{
  auto data = CQChartsShapeData::fromVariant(value);

  CQChartsShapeDataEditPreview::draw(painter, data, rect, plot, view);
}

QString
CQChartsShapeDataPropertyViewType::
tip(const QVariant &value) const
{
  auto str = CQChartsShapeData::fromVariant(value).toString();

  return str;
}

//------

CQChartsLineEditBase *
CQChartsShapeDataPropertyViewEditor::
createPropertyEdit(QWidget *parent)
{
  return new CQChartsShapeDataLineEdit(parent);
}

void
CQChartsShapeDataPropertyViewEditor::
connect(QWidget *w, QObject *obj, const char *method)
{
  auto *edit = qobject_cast<CQChartsShapeDataLineEdit *>(w);
  assert(edit);

  QObject::connect(edit, SIGNAL(shapeDataChanged()), obj, method);
}

QVariant
CQChartsShapeDataPropertyViewEditor::
getValue(QWidget *w)
{
  auto *edit = qobject_cast<CQChartsShapeDataLineEdit *>(w);
  assert(edit);

  return CQChartsShapeData::toVariant(edit->shapeData());
}

void
CQChartsShapeDataPropertyViewEditor::
setValue(QWidget *w, const QVariant &var)
{
  auto *edit = qobject_cast<CQChartsShapeDataLineEdit *>(w);
  assert(edit);

  auto data = CQChartsShapeData::fromVariant(var);

  edit->setShapeData(data);
}

//------

CQChartsShapeDataEdit::
CQChartsShapeDataEdit(QWidget *parent, bool tabbed) :
 CQChartsEditBase(parent), tabbed_(tabbed)
{
  setObjectName("shapeDataEdit");

  setToolTip("Shape Data");

  //---

  auto *layout = CQUtil::makeLayout<QGridLayout>(this, 0, 2);

  int row = 0;

  //---

  if (tabbed_) {
    auto *tab = CQUtil::makeWidget<QTabWidget>("tab");

    layout->addWidget(tab, row, 0, 1, 2); ++row;

    //----

    // fill frame
    auto *fillFrame       = CQUtil::makeWidget<QFrame>("fillFrame");
    auto *fillFrameLayout = CQUtil::makeLayout<QVBoxLayout>(fillFrame, 0, 2);

    tab->addTab(fillFrame, "Fill");

    //--

    // fill
    fillEdit_ = new CQChartsFillDataEdit;

    fillEdit_->setPreview(false);

    fillFrameLayout->addWidget(fillEdit_);

    //----

    // stroke frame
    auto *strokeFrame       = CQUtil::makeWidget<QFrame>("strokeFrame");
    auto *strokeFrameLayout = CQUtil::makeLayout<QVBoxLayout>(strokeFrame, 0, 2);

    tab->addTab(strokeFrame, "Stroke");

    //--

    // stroke
    strokeEdit_ = new CQChartsStrokeDataEdit;

    strokeEdit_->setPreview(false);

    strokeFrameLayout->addWidget(strokeEdit_);
  }
  else {
    // fill
    fillEdit_ = new CQChartsFillDataEdit;

    fillEdit_->setTitle("Fill");
    fillEdit_->setPreview(false);

    layout->addWidget(fillEdit_, row, 0, 1, 2); ++row;

    //--

    // stroke
    strokeEdit_ = new CQChartsStrokeDataEdit;

    strokeEdit_->setTitle("Stroke");
    strokeEdit_->setPreview(false);

    layout->addWidget(strokeEdit_, row, 0, 1, 2); ++row;
  }

  //---

  // preview
  preview_ = new CQChartsShapeDataEditPreview(this);

  layout->addWidget(preview_, row, 1);

  //---

  layout->setRowStretch(row, 1);

  //---

  connectSlots(true);

  dataToWidgets();
}

void
CQChartsShapeDataEdit::
setData(const CQChartsShapeData &d)
{
  data_ = d;

  dataToWidgets();
}

void
CQChartsShapeDataEdit::
setPlot(CQChartsPlot *plot)
{
  CQChartsEditBase::setPlot(plot);

  fillEdit_  ->setPlot(plot);
  strokeEdit_->setPlot(plot);
}

void
CQChartsShapeDataEdit::
setView(CQChartsView *view)
{
  CQChartsEditBase::setView(view);

  fillEdit_  ->setView(view);
  strokeEdit_->setView(view);
}

void
CQChartsShapeDataEdit::
setTitle(const QString &)
{
  if (! tabbed_) {
    fillEdit_  ->setTitle("Fill");
    strokeEdit_->setTitle("Stroke");
  }
}

void
CQChartsShapeDataEdit::
setPreview(bool b)
{
  fillEdit_  ->setPreview(b);
  strokeEdit_->setPreview(b);

  preview_->setVisible(b);
}

void
CQChartsShapeDataEdit::
connectSlots(bool b)
{
  auto connectDisconnect = [&](bool b, QWidget *w, const char *from, const char *to) {
    CQChartsWidgetUtil::connectDisconnect(b, w, from, this, to);
  };

  connectDisconnect(b, fillEdit_, SIGNAL(fillDataChanged()), SLOT(widgetsToData()));
  connectDisconnect(b, strokeEdit_, SIGNAL(strokeDataChanged()), SLOT(widgetsToData()));
}

void
CQChartsShapeDataEdit::
dataToWidgets()
{
  connectSlots(false);

  fillEdit_  ->setData(data_.fill());
  strokeEdit_->setData(data_.stroke());

  preview_->update();

  connectSlots(true);

}

void
CQChartsShapeDataEdit::
widgetsToData()
{
  data_.setFill  (fillEdit_  ->data());
  data_.setStroke(strokeEdit_->data());

  preview_->update();

  emit shapeDataChanged();
}

//------

CQChartsShapeDataEditPreview::
CQChartsShapeDataEditPreview(CQChartsShapeDataEdit *edit) :
 CQChartsEditPreview(edit), edit_(edit)
{
  setToolTip("Shape Preview");
}

void
CQChartsShapeDataEditPreview::
draw(QPainter *painter)
{
  const auto &data = edit_->data();

  draw(painter, data, rect(), edit_->plot(), edit_->view());
}

void
CQChartsShapeDataEditPreview::
draw(QPainter *painter, const CQChartsShapeData &data, const QRect &rect,
     CQChartsPlot *plot, CQChartsView *view)
{
  // set pen and brush
  auto pc = interpColor(plot, view, data.stroke().color());
  auto fc = interpColor(plot, view, data.fill  ().color());

  double width = data.stroke().width().value();

  width = (plot ? plot->limitLineWidth(width) : view->limitLineWidth(width));

  QPen pen;

  CQChartsUtil::setPen(pen, data.stroke().isVisible(), pc, data.stroke().alpha(),
                       width, data.stroke().dash());

  QBrush brush;

  CQChartsBrushData brushData;

  brushData.setVisible(data.fill().isVisible());
  brushData.setColor  (fc);
  brushData.setAlpha  (data.fill().alpha());
  brushData.setPattern(data.fill().pattern());

  CQChartsDrawUtil::setBrush(brush, brushData);

  painter->setPen  (pen);
  painter->setBrush(brush);

  //---

  // draw shape
  CQChartsPixelPaintDevice device(painter);

  CQChartsDrawUtil::drawRoundedRect(&device, CQChartsGeom::BBox(rect), data.stroke().cornerSize());
}
