#include <CQChartsSymbol.h>
#include <CQPropertyView.h>

CQUTIL_DEF_META_TYPE(CQChartsSymbol, toString, fromString)

int CQChartsSymbol::metaTypeId;

//---

QString
CQChartsSymbol::
typeToName(Type type)
{
  switch (type) {
    case Type::DOT:       return "dot";
    case Type::CROSS:     return "cross";
    case Type::PLUS:      return "plus";
    case Type::Y:         return "y";
    case Type::Z:         return "z";
    case Type::TRIANGLE:  return "triangle";
    case Type::ITRIANGLE: return "itriangle";
    case Type::BOX:       return "box";
    case Type::DIAMOND:   return "diamond";
    case Type::STAR5:     return "star5";
    case Type::STAR6:     return "star6";
    case Type::CIRCLE:    return "circle";
    case Type::PENTAGON:  return "pentagon";
    case Type::IPENTAGON: return "ipentagon";
    case Type::HLINE:     return "hline";
    case Type::VLINE:     return "vline";
    case Type::PAW:       return "paw";
    case Type::HASH:      return "hash";
    case Type::CHAR:      return "char";
    default:              return "none";
  }
}

CQChartsSymbol::Type
CQChartsSymbol::
nameToType(const QString &str)
{
  auto lstr = str.toLower();

  if (lstr == "dot"      ) return Type::DOT;
  if (lstr == "cross"    ) return Type::CROSS;
  if (lstr == "plus"     ) return Type::PLUS;
  if (lstr == "y"        ) return Type::Y;
  if (lstr == "z"        ) return Type::Z;
  if (lstr == "triangle" ) return Type::TRIANGLE;
  if (lstr == "itriangle") return Type::ITRIANGLE;
  if (lstr == "square"   ) return Type::BOX;
  if (lstr == "box"      ) return Type::BOX;
  if (lstr == "diamond"  ) return Type::DIAMOND;
  if (lstr == "star"     ) return Type::STAR5;
  if (lstr == "star5"    ) return Type::STAR5;
  if (lstr == "star6"    ) return Type::STAR6;
  if (lstr == "circle"   ) return Type::CIRCLE;
  if (lstr == "pentagon" ) return Type::PENTAGON;
  if (lstr == "ipentagon") return Type::IPENTAGON;
  if (lstr == "hline"    ) return Type::HLINE;
  if (lstr == "vline"    ) return Type::VLINE;
  if (lstr == "paw"      ) return Type::PAW;
  if (lstr == "hash"     ) return Type::HASH;
  if (lstr == "char"     ) return Type::CHAR;

  return Type::NONE;
}

QStringList
CQChartsSymbol::
typeNames()
{
  static QStringList typeNames = QStringList() <<
    "dot"      << "cross"    << "plus"     << "y"        << "z"     <<
    "triangle" << "itriangle"<< "box"      << "diamond"  << "star5" <<
    "star6"    << "circle"   << "pentagon" << "ipentagon"<< "hline" <<
    "vline"    << "paw"      << "hash";

  return typeNames;
}

//---

void
CQChartsSymbol::
registerMetaType()
{
  metaTypeId = CQUTIL_REGISTER_META(CQChartsSymbol);

  CQPropertyViewMgrInst->setUserName("CQChartsSymbol", "symbol");
}

CQChartsSymbol::
CQChartsSymbol(const QString &s)
{
  type_ = nameToType(s);
}

QString
CQChartsSymbol::
toString() const
{
  if (type_ == Type::CHAR)
    return QString("char:%1").arg(charData_.c);
  else
    return typeToName(type_);
}

bool
CQChartsSymbol::
fromString(const QString &s)
{
  if (s.left(5) == "char:") {
    type_       = Type::CHAR;
    charData_.c = s.mid(5);
  }
  else
    type_ = nameToType(s);

  return (type_ != Type::NONE);
}
