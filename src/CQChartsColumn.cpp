#include <CQChartsColumn.h>
#include <CQChartsUtil.h>
#include <CQPropertyView.h>
#include <CQTclUtil.h>
#include <CQStrParse.h>

CQUTIL_DEF_META_TYPE(CQChartsColumn, toString, fromString)

int CQChartsColumn::metaTypeId;

void
CQChartsColumn::
registerMetaType()
{
  metaTypeId = CQUTIL_REGISTER_META(CQChartsColumn);

  CQPropertyViewMgrInst->setUserName("CQChartsColumn", "column");
}

CQChartsColumn::
CQChartsColumn(int column, int role) :
 type_(Type::DATA), column_(column), role_(role)
{
  if (column_ < 0)
    type_ = Type::NONE;
}

CQChartsColumn::
CQChartsColumn(const QString &s)
{
  if (! setValue(s))
    type_ = Type::NONE;
}

CQChartsColumn::
CQChartsColumn(Type type, int column, const QString &s, int role) :
 type_(type), column_(column), role_(role)
{
  if (type_ == Type::EXPR || type_ == Type::DATA_INDEX) {
    int len = s.length();

    expr_ = new char [len + 1];

    memcpy(expr_, s.toLatin1().constData(), len + 1);
  }
}

CQChartsColumn::
CQChartsColumn(const CQChartsColumn &rhs) :
 type_(rhs.type_), column_(rhs.column_), role_(rhs.role_)
{
  if (rhs.hasExpr() || rhs.hasIndex()) {
    int len = strlen(rhs.expr_);

    expr_ = new char [len + 1];

    memcpy(expr_, rhs.expr_, len + 1);
  }

  if (rhs.name_) {
    int len = strlen(rhs.name_);

    name_ = new char [len + 1];

    memcpy(name_, rhs.name_, len + 1);
  }
}

CQChartsColumn::
~CQChartsColumn()
{
  delete [] expr_;
  delete [] name_;
}

CQChartsColumn &
CQChartsColumn::
operator=(const CQChartsColumn &rhs)
{
  delete [] expr_;
  delete [] name_;

  type_   = rhs.type_;
  column_ = rhs.column_;
  role_   = rhs.role_;
  expr_   = nullptr;
  name_   = nullptr;

  if (rhs.hasExpr() || rhs.hasIndex()) {
    int len = strlen(rhs.expr_);

    expr_ = new char [len + 1];

    memcpy(expr_, rhs.expr_, len + 1);
  }

  if (rhs.name_) {
    int len = strlen(rhs.name_);

    name_ = new char [len + 1];

    memcpy(name_, rhs.name_, len + 1);
  }

  return *this;
}

void
CQChartsColumn::
setName(const QString &name)
{
  delete [] name_;

  int len = name.length();

  if (len) {
    name_ = new char [len + 1];

    memcpy(name_, name.toStdString().c_str(), len + 1);
  }
  else
    name_ = nullptr;
}

bool
CQChartsColumn::
setValue(const QString &str)
{
  Type    type;
  int     column;
  int     role;
  QString expr;
  QString name;

  if (! decodeString(str, type, column, role, expr, name))
    return false;

  delete [] expr_;
  delete [] name_;

  type_   = type;
  column_ = column;
  role_   = role;
  expr_   = nullptr;
  name_   = nullptr;

  if (type == Type::EXPR || type == Type::DATA_INDEX) {
    int len = expr.length();

    expr_ = new char [len + 1];

    memcpy(expr_, expr.toStdString().c_str(), len + 1);
  }

  if (name != "")
    setName(name);

  return true;
}

QString
CQChartsColumn::
toString() const
{
  if      (type_ == Type::DATA) {
    if (role_ >= 0)
      return QString("%1@%2").arg(column_).arg(role_);
    else
      return QString("%1").arg(column_);
  }
  else if (type_ == Type::EXPR)
    return QString("(%1)").arg(expr_);
  else if (type_ == Type::ROW) {
    if (role_ > 0)
      return "@ROW1";
    else
      return "@ROW";
  }
  else if (type_ == Type::VHEADER)
    return "@VH";
  else if (type_ == Type::GROUP)
    return "@GROUP";
  else if (type_ == Type::DATA_INDEX) {
    if (role_ >= 0)
      return QString("%1@%2[%3]").arg(column_).arg(role_).arg(expr_);
    else
      return QString("%1[%2]").arg(column_).arg(expr_);
  }

  return "";
}

bool
CQChartsColumn::
fromString(const QString &s)
{
  if (! setValue(s))
    type_ = Type::NONE;

  return (type_ != Type::NONE);
}

int
CQChartsColumn::
cmp(const CQChartsColumn &lhs, const CQChartsColumn &rhs)
{
  if (lhs.type_ < rhs.type_) return -1;
  if (lhs.type_ > rhs.type_) return  1;

  if (lhs.column_ < rhs.column_) return -1;
  if (lhs.column_ > rhs.column_) return  1;

  if (lhs.role_ < rhs.role_) return -1;
  if (lhs.role_ > rhs.role_) return  1;

  if (lhs.expr_ != rhs.expr_) {
    if (! lhs.expr_) return -1;
    if (! rhs.expr_) return  1;

    return strcmp(lhs.expr_, rhs.expr_);
  }

  return 0;
}

void
CQChartsColumn::
print(std::ostream &os) const
{
  os << toString().toStdString();
}

bool
CQChartsColumn::
stringToColumns(const QString &str, Columns &columns)
{
  QStringList strs;

  if (! CQTcl::splitList(str, strs))
    return false;

  bool rc = true;

  for (int i = 0; i < strs.length(); ++i) {
    CQChartsColumn c(strs[i]);

    if (! c.isValid())
      rc = false;

    columns.push_back(c);
  }

  return rc;
}

QString
CQChartsColumn::
columnsToString(const Columns &columns)
{
  QStringList strs;

  for (const auto &c : columns) {
    strs += c.toString();
  }

  return CQTcl::mergeList(strs);
}

bool
CQChartsColumn::
decodeString(const QString &str, Type &type, int &column, int &role, QString &expr, QString &name)
{
  type   = Type::NONE;
  column = -1;
  role   = -1;
  expr   = "";
  name   = "";

  CQStrParse parse(str);

  parse.skipSpace();

  if (parse.eof())
    return false;

  //---

  // get optional name
  if (parse.isChar('#')) {
    parse.skipChar();

    int pos = parse.getPos();

    while (! parse.eof()) {
      if      (parse.isChar('\\')) {
        parse.skipChar();

        if (! parse.eof())
          parse.skipChar();
      }
      else if (parse.isSpace())
        break;
      else
        parse.skipChar();
    }

    name = parse.getAt(pos);

    parse.skipSpace();
  }

  //---

  // expression
  if (parse.isChar('(')) {
    parse.skipChar();

    int nb = 1;

    int pos = parse.getPos();

    while (! parse.eof()) {
      if      (parse.isChar('('))
        ++nb;
      else if (parse.isChar(')')) {
        --nb;

        if (nb == 0)
          break;
      }

      parse.skipChar();
    }

    if (parse.isChar(')'))
      parse.skipChar();

    QString str = parse.getAt(pos);

    expr = str.simplified();
    type = Type::EXPR;

    //parse.skipSpace();

    //if (! parse.eof())
    //  return false;

    return true;
  }

  //--

  if (parse.isChar('@')) {
    // row or row offset
    if (parse.isWord("@R") || parse.isWord("@ROW")) {
      type = Type::ROW; return true;
    }

    if (parse.isWord("@R1") || parse.isWord("@ROW1")) {
      role = 1; type = Type::ROW; return true;
    }

    //---

    // vertical header
    if (parse.isWord("@VH") || parse.isWord("@VHEADER")) {
      type = Type::VHEADER; return true;
    }

    // group
    if (parse.isWord("@GROUP")) {
      type = Type::GROUP; return true;
    }
  }

  //---

  // integer column number
  long column1 { 0 };

  if (parse.readInteger(&column1)) {
    if (column1 < 0)
      return false;
  }

  //---

  // optional role (directly after)
  long role1 = -1;

  if (parse.isChar('@')) {
    parse.skipChar();

    if (! parse.readInteger(&role1))
      return false;

    if (role1 < 0)
      return false;
  }

  //---

  // optional index string (directly after)
  QString indexStr;

  if (parse.isChar('[')) {
    parse.skipChar();

    int pos = parse.getPos();

    while (! parse.eof()) {
      if (parse.isChar(']'))
        break;

      parse.skipChar();
    }

    if (parse.isChar(']'))
      parse.skipChar();

    indexStr = parse.getAt(pos);
  }

  //---

  parse.skipSpace();

  if (! parse.eof())
    return false;

  //---

  if (indexStr.length()) {
    type = Type::DATA_INDEX;
    expr = indexStr.simplified();
  }
  else
    type = Type::DATA;

  //---

  column = int(column1);
  role   = int(role1);

  return true;
}

//---

CQUTIL_DEF_META_TYPE(CQChartsColumns, toString, fromString)

int CQChartsColumns::metaTypeId;

void
CQChartsColumns::
registerMetaType()
{
  metaTypeId = CQUTIL_REGISTER_META(CQChartsColumns);

  CQPropertyViewMgrInst->setUserName("CQChartsColumns", "column_list");
}
