#include <CQChartsCmdBase.h>
#include <CQChartsInput.h>
#include <CQBaseModel.h>
#include <CQPerfMonitor.h>
#include <CQWidgetFactory.h>
#include <CQUtil.h>
#include <CQTclUtil.h>

#include <QApplication>

namespace {

void
errorMsg(const QString &msg)
{
  std::cerr << msg.toStdString() << "\n";
}

}

class CQChartsTclCmd {
 public:
  using Vars = std::vector<QVariant>;

 public:
  CQChartsTclCmd(CQChartsCmdBase *cmdBase, const QString &name) :
   cmdBase_(cmdBase), name_(name) {
    cmdId_ = cmdBase_->qtcl()->createObjCommand(name_,
               (CQTcl::ObjCmdProc) &CQChartsTclCmd::commandProc, (CQTcl::ObjCmdData) this);
  }

  static int commandProc(ClientData clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv) {
    auto *command = (CQChartsTclCmd *) clientData;

    Vars vars;

    for (int i = 1; i < objc; ++i) {
      auto *obj = const_cast<Tcl_Obj *>(objv[i]);

      vars.push_back(command->cmdBase_->qtcl()->variantFromObj(obj));
    }

    if (! command->cmdBase_->processCmd(command->name_, vars))
      return TCL_ERROR;

    return TCL_OK;
  }

 private:
  CQChartsCmdBase *cmdBase_  { nullptr };
  QString          name_;
  Tcl_Command      cmdId_ { nullptr };
};

//----

CQChartsCmdBase::
CQChartsCmdBase()
{
  qtcl_ = new CQTcl();
}

CQChartsCmdBase::
~CQChartsCmdBase()
{
  delete qtcl_;
}

void
CQChartsCmdBase::
addCommands()
{
  static bool cmdsAdded;

  if (! cmdsAdded) {
    addCommand("help", new CQChartsBaseHelpCmd(this));

    // qt generic
    addCommand("qt_create_widget", new CQChartsBaseCreateWidgetCmd(this));

    addCommand("qt_get_property", new CQChartsBaseGetPropertyCmd(this));
    addCommand("qt_set_property", new CQChartsBaseSetPropertyCmd(this));
    addCommand("qt_has_property", new CQChartsBaseHasPropertyCmd(this));

    addCommand("qt_sync", new CQChartsBaseQtSyncCmd(this));

    addCommand("perf", new CQChartsBasePerfCmd(this));

    qtcl()->createAlias("echo", "puts");

    addCommand("assert", new CQChartsBaseAssertCmd(this));

    addCommand("sh", new CQChartsBaseShellCmd(this));

    //---

    cmdsAdded = true;
  }
}

void
CQChartsCmdBase::
addCommand(const QString &name, CQChartsCmdProc *proc)
{
  proc->setName(name);

  auto tclCmd = new CQChartsTclCmd(this, name);

  proc->setTclCmd(tclCmd);

  commandNames_.push_back(name);

  commandProcs_[name] = proc;
}

bool
CQChartsCmdBase::
processCmd(const QString &cmd, const Vars &vars)
{
  auto p = commandProcs_.find(cmd);

  if (p != commandProcs_.end()) {
    auto *proc = (*p).second;

    CQChartsCmdArgs argv(cmd, vars);

    return proc->exec(argv);
  }

  //---

  if (cmd == "exit") { exit(0); }

  //---

  return false;
}

//------

bool
CQChartsCmdBase::
qtCreateWidgetCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::qtCreateWidgetCmd");

  argv.addCmdArg("-parent", CQChartsCmdArg::Type::String, "parent name");
  argv.addCmdArg("-type"  , CQChartsCmdArg::Type::String, "widget type");
  argv.addCmdArg("-name"  , CQChartsCmdArg::Type::String, "widget name");

  if (! argv.parse())
    return false;

  QString typeName = argv.getParseStr("type");

  if (typeName == "?") {
    auto names = CQWidgetFactoryMgrInst->widgetFactoryNames();

    return setCmdRc(names);
  }

  QWidget *parentWidget = nullptr;

  if (argv.hasParseArg("parent")) {
    QString parentName = argv.getParseStr("parent");

    auto *parent = CQUtil::nameToObject(parentName);

    parentWidget = qobject_cast<QWidget *>(parent);

    if (! parentWidget) {
      errorMsg(QString("No parent '%1'").arg(parentName));
      return false;
    }
  }

  QString name     = argv.getParseStr("name");

  if (! CQWidgetFactoryMgrInst->isWidgetFactory(typeName)) {
    errorMsg(QString("Invalid type '%1'").arg(typeName));
    return false;
  }

  QWidget *w = CQWidgetFactoryMgrInst->createWidget(typeName, parentWidget);

  if (! w) {
    errorMsg(QString("Failed to create '%1'").arg(typeName));
    return false;
  }

  if (name != "")
    w->setObjectName(name);

  setCmdRc(CQUtil::fullName(w));

  return true;
}

//------

bool
CQChartsCmdBase::
qtGetPropertyCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::qtGetPropertyCmd");

  argv.addCmdArg("-object"  , CQChartsCmdArg::Type::String, "object name");
  argv.addCmdArg("-property", CQChartsCmdArg::Type::String, "property name");

  if (! argv.parse())
    return false;

  QString objectName = argv.getParseStr("object");
  QString propName   = argv.getParseStr("property");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj) {
    errorMsg(QString("No object '%1'").arg(objectName));
    return false;
  }

  QVariant v;

  if (! CQUtil::getProperty(obj, propName, v)) {
    errorMsg(QString("Failed to get property '%1' for '%2'").arg(propName).arg(objectName));
    return false;
  }

  setCmdRc(v);

  return true;
}

//------

bool
CQChartsCmdBase::
qtSetPropertyCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::qtSetPropertyCmd");

  argv.addCmdArg("-object"  , CQChartsCmdArg::Type::String, "object name");
  argv.addCmdArg("-property", CQChartsCmdArg::Type::String, "property name");
  argv.addCmdArg("-value"   , CQChartsCmdArg::Type::String, "property value");

  if (! argv.parse())
    return false;

  QString objectName = argv.getParseStr("object");
  QString propName   = argv.getParseStr("property");
  QString value      = argv.getParseStr("value");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj) {
    errorMsg(QString("No object '%1'").arg(objectName));
    return false;
  }

  if (! CQUtil::setProperty(obj, propName, value)) {
    errorMsg(QString("Failed to set property '%1' for '%2'").arg(propName).arg(objectName));
    return false;
  }

  return true;
}

//------

bool
CQChartsCmdBase::
qtHasPropertyCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::qtHasPropertyCmd");

  argv.addCmdArg("-object"  , CQChartsCmdArg::Type::String , "object name");
  argv.addCmdArg("-property", CQChartsCmdArg::Type::String , "property name");
  argv.addCmdArg("-writable", CQChartsCmdArg::Type::Boolean, "property is writable");

  if (! argv.parse())
    return false;

  QString objectName = argv.getParseStr("object");
  QString propName   = argv.getParseStr("property");

  auto *obj = CQUtil::nameToObject(objectName);

  if (! obj) {
    errorMsg(QString("No object '%1'").arg(objectName));
    return false;
  }

  bool b;

  if (argv.hasParseArg("writable"))
    b = CQUtil::hasWritableProperty(obj, propName);
  else
    b = CQUtil::hasProperty(obj, propName);

  setCmdRc(b);

  return true;
}

//------

bool
CQChartsCmdBase::
qtSyncCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::qtSyncCmd");

  argv.addCmdArg("-n", CQChartsCmdArg::Type::Integer, "loop count");

  if (! argv.parse())
    return false;

  int n = 1;

  if (argv.hasParseArg("n"))
    n = argv.getParseInt("n");

  for (int i = 0; i < n; ++i) {
    qApp->flush();

    qApp->processEvents();
  }

  return true;
}

//------

bool
CQChartsCmdBase::
perfCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::perfCmd");

  argv.addCmdArg("-start_recording", CQChartsCmdArg::Type::Boolean, "start recording");
  argv.addCmdArg("-end_recording"  , CQChartsCmdArg::Type::Boolean, "end recording"  );
  argv.addCmdArg("-tracing"        , CQChartsCmdArg::Type::SBool  , "enable tracing" );
  argv.addCmdArg("-debug"          , CQChartsCmdArg::Type::SBool  , "enable debug"   );

  if (! argv.parse())
    return false;

  //---

  if (argv.hasParseArg("start_recording"))
    CQPerfMonitorInst->startRecording();

  if (argv.hasParseArg("end_recording"))
    CQPerfMonitorInst->stopRecording();

  if (argv.hasParseArg("tracing"))
    CQPerfMonitorInst->setEnabled(argv.getParseBool("tracing"));

  if (argv.hasParseArg("debug"))
    CQPerfMonitorInst->setDebug(argv.getParseBool("debug"));

  return true;
}

//------

bool
CQChartsCmdBase::
assertCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::assertCmd");

  if (! argv.parse())
    return false;

  //---

  const auto &pargs = argv.getParseArgs();

  QString expr = (! pargs.empty() ? pargs[0].toString() : "");

  QVariant res;

  bool rc = qtcl()->evalExpr(expr, res, /*showError*/true);
  if (! rc) return false;

  bool b = res.toBool();

  return b;
}

//------

bool
CQChartsCmdBase::
shellCmd(CQChartsCmdArgs &argv)
{
  CQPerfTrace trace("CQChartsCmdBase::shellCmd");

  if (! argv.parse())
    return false;

  //---

  const auto &pargs = argv.getParseArgs();

  QString cmd = (! pargs.empty() ? pargs[0].toString() : "");

  //---

  if (cmd == "") {
    errorMsg("No command");
    return false;
  }

  int rc = system(cmd.toLatin1().constData());

  setCmdRc(rc);

  return true;
}

//------

bool
CQChartsCmdBase::
helpCmd(CQChartsCmdArgs &argv)
{
  if (! argv.parse())
    return false;

  //---

  const auto &pargs = argv.getParseArgs();

  QString pattern = (! pargs.empty() ? pargs[0].toString() : "");

  //---

  if (pattern.length()) {
    QRegExp re(pattern, Qt::CaseSensitive, QRegExp::Wildcard);

    for (auto &p : commandProcs_) {
      if (re.exactMatch(p.first))
        std::cout << p.first.toStdString() << "\n";
    }
  }
  else {
    for (auto &p : commandProcs_)
      std::cout << p.first.toStdString() << "\n";
  }

  return true;
}

//------

QStringList
CQChartsCmdBase::
stringToCmdData(const QString &str) const
{
  return CQChartsInput::stringToLines(str,
   [&](QString &line, bool &join) {
    return isCompleteLine(line, join);
   }
  );
}

//------

#if 0
bool
CQChartsCmdBase::
valueToStrs(const QString &str, QStringList &strs) const
{
  if (str.length() >= 2 && str[0] == '{' && str[str.length() - 1] == '}')
    return qtcl()->splitList(str.mid(1, str.length() - 2), strs);
  else
    return qtcl()->splitList(str, strs);
}
#endif

//------

bool
CQChartsCmdBase::
setCmdRc(int rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(double rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(const QString &rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(const std::string &rc)
{
  qtcl()->setResult(QString(rc.c_str()));

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(const QVariant &rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(const QStringList &rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdRc(const QVariantList &rc)
{
  qtcl()->setResult(rc);

  return true;
}

bool
CQChartsCmdBase::
setCmdError(const QString &msg)
{
  errorMsg(msg);

  setCmdRc(QString());

  return false;
}

//------

QStringList
CQChartsCmdBase::
stringToCmds(const QString &str) const
{
  return CQChartsInput::stringToLines(str,
   [&](QString &line, bool &join) {
    return isCompleteLine(line, join);
   }
  );
}

//------

bool
CQChartsCmdBase::
isCompleteLine(QString &str, bool &join)
{
  join = false;

  if (! str.length())
    return true;

  if (str[str.size() - 1] == '\\') {
    str = str.mid(0, str.length() - 1);
    join = true;
    return false;
  }

  //---

  CQStrParse line(str);

  line.skipSpace();

  while (! line.eof()) {
    if      (line.isChar('{')) {
      if (! line.skipBracedString())
        return false;
    }
    else if (line.isChar('\"') || line.isChar('\'')) {
      if (! line.skipString())
        return false;
    }
    else {
      line.skipNonSpace();
    }

    line.skipSpace();
  }

  return true;
}

void
CQChartsCmdBase::
parseLine(const QString &line, bool log)
{
  int rc = qtcl()->eval(line, /*showError*/true, /*showResult*/log);

  if (rc != TCL_OK)
    errorMsg("Invalid line: '" + line + "'");
}
