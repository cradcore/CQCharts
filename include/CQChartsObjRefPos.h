#ifndef CQChartsObjRefPos_H
#define CQChartsObjRefPos_H

#include <CQChartsObjRef.h>
#include <CQChartsPosition.h>
#include <CQUtilMeta.h>

/*!
 * \brief ObjRef Position class
 * ingroup Charts
 */
class CQChartsObjRefPos :
  public CQChartsEqBase<CQChartsObjRefPos>,
  public CQChartsToStringBase<CQChartsObjRefPos> {
 public:
  static void registerMetaType();

  static int metaTypeId;

  //---

  CQUTIL_DEF_META_CONVERSIONS(CQChartsObjRefPos, metaTypeId)

 public:
  using ObjRef   = CQChartsObjRef;
  using Position = CQChartsPosition;

 public:
  CQChartsObjRefPos() = default;

  CQChartsObjRefPos(const ObjRef &objRef, const Position &position) :
   objRef_(objRef), position_(position) {
  }

  //---

  const ObjRef &objRef() const { return objRef_; }
  void setObjRef(const ObjRef &objRef) { objRef_ = objRef; }

  const Position &position() const { return position_; }
  void setPosition(const Position &p) { position_ = p; }

  //---

  QString toString() const;
  bool fromString(const QString &s);

  //---

  friend bool operator==(const CQChartsObjRefPos &lhs, const CQChartsObjRefPos &rhs) {
    if (lhs.objRef_   != rhs.objRef_  ) return false;
    if (lhs.position_ != rhs.position_) return false;
    return true;
  }

  //---

 private:
  ObjRef   objRef_;
  Position position_;
};

//---

CQUTIL_DCL_META_TYPE(CQChartsObjRefPos)

#endif
