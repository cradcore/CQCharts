#include <CQChartsRegionMgr.h>
#include <CQChartsView.h>
#include <CQChartsWidgetUtil.h>
#include <CQUtil.h>

#include <QToolButton>

#include <svg/region_light_svg.h>
#include <svg/region_dark_svg.h>

CQChartsRegionMgr::
CQChartsRegionMgr(CQChartsView *view) :
 view_(view)
{
}

CQChartsRegionMgr::
~CQChartsRegionMgr()
{
}

void
CQChartsRegionMgr::
addButton(CQChartsRegionButton *button)
{
  buttonSet_.insert(button);

  if (button)
    connect(button, SIGNAL(clicked(bool)), this, SLOT(regionSlot(bool)));
}

void
CQChartsRegionMgr::
removeButton(CQChartsRegionButton *button)
{
  if (button)
    disconnect(button, SIGNAL(clicked(bool)), this, SLOT(regionSlot(bool)));

  buttonSet_.erase(button);
}

void
CQChartsRegionMgr::
regionSlot(bool b)
{
  if (! view_) return;

  auto *button = qobject_cast<CQChartsRegionButton *>(sender());
  assert(button);

  if (b) {
    view_->setMode(CQChartsView::Mode::REGION);

    if (button->mode() == CQChartsRegionButton::Mode::RECT)
      view_->setRegionMode(CQChartsView::RegionMode::RECT);
    else
      view_->setRegionMode(CQChartsView::RegionMode::POINT);
  }
  else
    view_->setMode(CQChartsView::Mode::SELECT);

  for (auto &button1 : buttonSet_) {
    if (button1 != button) {
      button1->setChecked(false);

      disconnect(view_, SIGNAL(regionRectRelease(const CQChartsGeom::BBox &)),
                 button1, SLOT(rectRegionSetSlot(const CQChartsGeom::BBox &)));
      disconnect(view_, SIGNAL(regionPointRelease(const CQChartsGeom::Point &)),
                 button1, SLOT(pointRegionSetSlot(const CQChartsGeom::Point &)));
    }
  }

  button->setChecked(b);

  CQChartsWidgetUtil::connectDisconnect(b,
    view_, SIGNAL(regionRectRelease(const CQChartsGeom::BBox &)),
    button, SLOT(rectRegionSetSlot(const CQChartsGeom::BBox &)));
  CQChartsWidgetUtil::connectDisconnect(b,
    view_, SIGNAL(regionPointRelease(const CQChartsGeom::Point &)),
    button, SLOT(pointRegionSetSlot(const CQChartsGeom::Point &)));
}

//---

CQChartsRegionButton::
CQChartsRegionButton(QWidget *parent) :
 QToolButton(parent)
{
  setObjectName("region");

  setCheckable(true);
  setIcon(CQPixmapCacheInst->getIcon("REGION_LIGHT", "REGION_DARK"));
}

CQChartsRegionButton::
~CQChartsRegionButton()
{
  if (view_)
    view_->regionMgr()->removeButton(this);
}

void
CQChartsRegionButton::
setView(CQChartsView *view)
{
  if (view_)
    view_->regionMgr()->removeButton(this);

  view_ = view;

  if (view_)
    view_->regionMgr()->addButton(this);
}

void
CQChartsRegionButton::
rectRegionSetSlot(const CQChartsGeom::BBox &bbox)
{
  setRect(bbox);

  emit rectRegionSet(bbox);
}

void
CQChartsRegionButton::
pointRegionSetSlot(const CQChartsGeom::Point &p)
{
  setPoint(p);

  emit pointRegionSet(p);
}
