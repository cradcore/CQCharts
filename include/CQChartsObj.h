#ifndef CQChartsObj_H
#define CQChartsObj_H

#include <CQChartsGeom.h>
#include <CQChartsUtil.h>
#include <QObject>
#include <future>
#include <boost/optional.hpp>

class CQCharts;
class CQChartsPaintDevice;

/*!
 * \brief Charts object base class
 * \ingroup Charts
 *
 * An object has the following properties:
 *  . unique id
 *  . bounding box (rect)
 *  . tooltip
 *  . is selected
 *  . is mouse inside
 *  . can be selected
 *  . can be edited
 *
 * Id and Tip Id are cached (calculated by calcId and calcTipId virtuals)
 */
class CQChartsObj : public QObject {
  Q_OBJECT

  Q_PROPERTY(QString            id         READ id           WRITE setId        )
  Q_PROPERTY(CQChartsGeom::BBox rect       READ rect         WRITE setRect      )
  Q_PROPERTY(QString            tipId      READ tipId        WRITE setTipId     )
  Q_PROPERTY(bool               visible    READ isVisible    WRITE setVisible   )
  Q_PROPERTY(bool               selected   READ isSelected   WRITE setSelected  )
  Q_PROPERTY(bool               inside     READ isInside     WRITE setInside    )
  Q_PROPERTY(bool               selectable READ isSelectable WRITE setSelectable)
  Q_PROPERTY(bool               editable   READ isEditable   WRITE setEditable  )
  Q_PROPERTY(int                priority   READ priority     WRITE setPriority  )

 public:
  using BBox  = CQChartsGeom::BBox;
  using Size  = CQChartsGeom::Size;
  using Point = CQChartsGeom::Point;

 public:
  CQChartsObj(CQCharts *charts, const BBox &rect=BBox());

  //---

  //! get charts
  CQCharts *charts() const { return charts_; }

  //---

  // unique id of object

  //! has unique id
  bool hasId() const { return !!id_; }

  //! get/set unique id
  const QString &id() const;
  void setId(const QString &s);

  //! calculate unique id of object (on demand) : must be non-empty
  virtual QString calcId() const { return ""; }

  //---

  //! get set rect
  const BBox &rect() const { return rect_; }
  virtual void setRect(const BBox &r) { rect_ = r; dataInvalidate(); }

  //---

  //! intersect shape with line (used for arrow annotation connection point)
  virtual bool intersectShape(const Point & /*p1*/, const Point & /*p2*/,
                              Point & /*pi*/) const { return false; }

  //---

  // tip id for object (string to display in tooltip)

  //! has tp id
  bool hasTipId() const { return !!tipId_; }

  //! get/set tip id
  const QString &tipId() const;
  void setTipId(const QString &s);

  //! reset tip id (force recalc(
  void resetTipId() { tipId_ = OptString(); dataInvalidate(); }

  //! calculate tip id (on demand)
  virtual QString calcTipId() const { return calcId(); }

  //---

  //! get/set visible
  virtual bool isVisible() const { return visible_; }
  virtual void setVisible(bool b) { visible_ = b; if (notificationsEnabled_) dataInvalidate(); }

  //! set/get selected
  virtual bool isSelected() const { return selected_; }
  virtual void setSelected(bool b) { selected_ = b; if (notificationsEnabled_) dataInvalidate(); }

  //! set/get inside
  virtual bool isInside() const { return inside_; }
  virtual void setInside(bool b) { inside_ = b; if (notificationsEnabled_) dataInvalidate(); }

  //---

  //! get/set is selectable
  virtual bool isSelectable() const { return selectable_; }
  virtual void setSelectable(bool b) { selectable_ = b; }

  //! get/set is editable
  virtual bool isEditable() const { return editable_; }
  virtual void setEditable(bool b) { editable_ = b; }

  //---

  //! get/set priority
  int priority() const { return priority_; }
  void setPriority(int i) { priority_ = i; }

  //---

  //! is point inside object
  virtual bool contains(const Point &p) const = 0;

  //---

  bool isNotificationsEnabled() const { return notificationsEnabled_; }
  void setNotificationsEnabled(bool b) { notificationsEnabled_ = b; }

  //---

  //! handle data change
  //! TODO: signal ?
  virtual void dataInvalidate() { }

 signals:
  //! emitted when object id changed
  void idChanged();

 protected:
  using OptString = boost::optional<QString>;

  CQCharts*          charts_               { nullptr }; //!< charts
  OptString          id_;                               //!< id
  BBox               rect_;                             //!< bbox
  OptString          tipId_;                            //!< tip id
  bool               visible_              { true };    //!< is visible
  bool               selected_             { false };   //!< is selected
  bool               inside_               { false };   //!< is mouse inside
  bool               editable_             { false };   //!< is editable
  bool               selectable_           { true };    //!< is selectable
  bool               notificationsEnabled_ { true };    //!< is selectable
  int                priority_             { 0 };       //!< priority
  mutable std::mutex mutex_;                            //!< mutex for calc id/tip
};

//------

class CQChartsEditHandles;

// Select Interface
class CQChartsSelectableIFace {
 public:
  using SelMod = CQChartsSelMod;
  using Point  = CQChartsGeom::Point;

  struct SelData {
    SelData() { }

    explicit SelData(SelMod selMod) :
     selMod(selMod) {
    }

    SelMod selMod { CQChartsSelMod::REPLACE };
    bool   select { true };
  };

 public:
  CQChartsSelectableIFace() { }

  virtual ~CQChartsSelectableIFace() { }

  //! handle select press, move, release
  virtual bool selectPress  (const Point &, SelData &) { return false; }
  virtual bool selectMove   (const Point &) { return false; }
  virtual bool selectRelease(const Point &) { return false; }

  //! handle select double click
  virtual bool selectDoubleClick(const Point &, SelMod) { return false; }
};

// Edit Interface
class CQChartsEditableIFace {
 public:
  using EditHandles = CQChartsEditHandles;
  using ResizeSide  = CQChartsResizeSide;
  using PaintDevice = CQChartsPaintDevice;
  using BBox        = CQChartsGeom::BBox;
  using Point       = CQChartsGeom::Point;

 public:
  CQChartsEditableIFace() { }

  virtual ~CQChartsEditableIFace() { }

  virtual EditHandles *editHandles() const = 0;

  // handle edit press, move, motion, release
  virtual bool editPress  (const Point &) { return false; }
  virtual bool editMove   (const Point &) { return false; }
  virtual bool editMotion (const Point &) { return false; } // return true if inside
  virtual bool editRelease(const Point &) { return true; }

  //! handle edit move by (move)
  virtual void editMoveBy(const Point &) { }

  //! set new bounding box (resize)
  virtual void setEditBBox(const BBox &, const ResizeSide &) { }

  //! can edit resize
  virtual bool isEditResize() const { return false; }

  //! draw
  virtual void drawEditHandles(PaintDevice *device) const = 0;
};

#endif
