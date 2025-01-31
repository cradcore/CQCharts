#include <CQChartsCmdLine.h>
#include <CQCharts.h>
#include <CQChartsWidgetUtil.h>
#include <CQChartsVariant.h>

#include <CQCommand.h>
#include <CQTclUtil.h>
#include <CTclParse.h>
#include <CQStrUtil.h>
#include <COSExec.h>
#include <COSProcess.h>

#include <QVBoxLayout>

#include <iostream>

CQChartsCmdLineDlgMgr::
CQChartsCmdLineDlgMgr()
{
}

void
CQChartsCmdLineDlgMgr::
showDialog(CQCharts *charts)
{
  if (dlg_ && dlg_->charts() != charts) {
    delete dlg_;

    dlg_ = nullptr;
  }

  if (! dlg_)
    dlg_ = new CQChartsCmdLine(charts);

  dlg_->show();

  dlg_->raise();
}

//---

class CQChartsCmdWidget : public CQCommand::CommandWidget {
 public:
  CQChartsCmdWidget(CQChartsCmdLine *cmdLine);

  bool complete(const QString &text, int pos,
                QString &newText, CompleteMode completeMode) const override {
    bool interactive = (completeMode == CompleteMode::Interactive);

    return cmdLine_->complete(const_cast<CQChartsCmdWidget *>(this), text,
                              pos, newText, interactive);
  }

 private:
  CQChartsCmdLine *cmdLine_ { nullptr };
};

//---

class CQChartsCmdScrollArea : public CQCommand::ScrollArea {
 public:
  CQChartsCmdScrollArea(CQChartsCmdLine *cmdLine) :
   cmdLine_(cmdLine) {
  }

  CQCommand::CommandWidget *createCommandWidget() const override {
    return new CQChartsCmdWidget(cmdLine_);
  }

 private:
  CQChartsCmdLine *cmdLine_ { nullptr };
};

//---

CQChartsCmdWidget::
CQChartsCmdWidget(CQChartsCmdLine *cmdLine) :
 CQCommand::CommandWidget(nullptr), cmdLine_(cmdLine)
{
}

//---

CQChartsCmdLine::
CQChartsCmdLine(CQCharts *charts, QWidget *parent) :
 QFrame(parent), charts_(charts)
{
  setWindowTitle("Charts Command Line");

  setObjectName("cmdLine");

  //---

  auto *layout = new QVBoxLayout(this);

  command_ = new CQChartsCmdScrollArea(this);

  command_->getCommand()->setMinLines(10);
  command_->getCommand()->setPrompt("> ");

  connect(command_, SIGNAL(executeCommand(const QString &)),
          this, SLOT(executeCommand(const QString &)));

  layout->addWidget(command_);

  //---

  qtcl_ = charts_->cmdTcl();
}

CQChartsCmdLine::
~CQChartsCmdLine()
{
}

bool
CQChartsCmdLine::
complete(CQChartsCmdWidget *widget, const QString &text, int pos,
         QString &newText, bool interactive) const
{
  int len = text.length();

  if (pos >= len)
    pos = len - 1;

  //---

  CTclParse parse;

  CTclParse::Tokens tokens;

  parse.parseLine(text.toStdString(), tokens);

  auto *token = parse.getTokenForPos(tokens, pos);

  if (! token)
    token = parse.getTokenForPos(tokens, pos - 1);

  //---

  auto lhs = text.mid(0, token ? token->pos() : pos + 1);
  auto str = (token ? token->str() : "");
  auto rhs = text.mid(token ? token->endPos() + 1 : pos + 1);

  //---

  std::string  command;
  int          commandPos { -1 };
  std::string  option;
//int          optionPos { -1 };
  OptionValues optionValues;

  // complete command
  if      (token && token->type() == CTclToken::Type::COMMAND) {
    command    = str;
    commandPos = token->pos();

    return completeCommand(widget, lhs, QString::fromStdString(command), rhs, interactive, newText);
  }
  // complete option
  else if (str[0] == '-') {
    option    = str.substr(1);
//  optionPos = token->pos();

    // get previous command token for command name
    for (int pos1 = pos - 1; pos1 >= 0; --pos1) {
      auto *token1 = parse.getTokenForPos(tokens, pos1);
      if (! token1) continue;

      const auto &str = token1->str();
      if (str.empty()) continue;

      if (token1->type() == CTclToken::Type::COMMAND) {
        command    = str;
        commandPos = token1->pos();
        break;
      }
    }

    if (command == "")
      return false;

    //---

    return completeOption(widget, QString::fromStdString(command),
                          lhs, QString::fromStdString(option), rhs, interactive, newText);
  }
  else {
    // get previous tokens for option name and command name
    for (int pos1 = pos - 1; pos1 >= 0; --pos1) {
      auto *token1 = parse.getTokenForPos(tokens, pos1);
      if (! token1) continue;

      const auto &str = token1->str();
      if (str.empty()) continue;

      if      (token1->type() == CTclToken::Type::COMMAND) {
        command    = str;
        commandPos = token1->pos();
        break;
      }
      else if (str[0] == '-') {
        if (option.empty()) {
          option    = str.substr(1);
//        optionPos = token1->pos();
        }

        if (pos1 > token1->pos())
          pos1 = token1->pos(); // move to start
      }
    }

    if (command == "" || option == "")
      return false;

    // get option values to next command
    std::string lastOption;

    for (int pos1 = commandPos + int(command.length()); pos1 < text.length(); ++pos1) {
      auto *token1 = parse.getTokenForPos(tokens, pos1);
      if (! token1) continue;

      const auto &str = token1->str();
      if (str.empty()) continue;

      if      (token1->type() == CTclToken::Type::COMMAND) {
        break;
      }
      else if (str[0] == '-') {
        lastOption = str.substr(1);

        optionValues[lastOption] = "";

        pos1 = token1->pos() + int(token1->str().length()); // move to end
      }
      else {
        if (lastOption != "")
          optionValues[lastOption] = str;

        pos1 = token1->pos() + int(token1->str().length()); // move to end
      }
    }

    //---

    return completeArg(widget, QString::fromStdString(command), QString::fromStdString(option),
                       optionValues, lhs, "", rhs, interactive, newText);
  }
}

bool
CQChartsCmdLine::
completeCommand(CQChartsCmdWidget *widget, const QString &lhs, const QString &command,
                const QString &rhs, bool interactive, QString &newText) const
{
  const auto &cmds = qtcl_->commandNames();

  auto matchCmds = CQStrUtil::matchStrs(command, cmds);

  QString matchStr;
  bool    exact = false;

  if      (matchCmds.size() == 1) {
    matchStr = matchCmds[0];
    exact    = true;
  }
  else if (matchCmds.size() > 1) {
    if (interactive) {
      matchStr = widget->showCompletionChooser(matchCmds);

      if (matchStr != "")
        exact = true;
    }
  }
  else {
    matchStr = CQStrUtil::longestMatch(matchCmds, exact);
  }

  //---

  newText = lhs;

  if (matchStr != "")
    newText += matchStr;

  if (exact)
    newText += " ";

  newText += rhs;

  return (matchStr.length() > command.length());
}

bool
CQChartsCmdLine::
completeOption(CQChartsCmdWidget *widget, const QString &command, const QString &lhs,
               const QString &option, const QString &rhs, bool interactive, QString &newText) const
{
  // get all options for interactive complete
  QString matchStr;
  bool    exact = false;

  if (interactive) {
    auto cmd = QString("complete -command {%1} -option {*} -all").arg(command);

    QVariant res;

    (void) qtcl_->eval(cmd, res);

    auto strs = resultToStrings(res);

    auto matchStrs = CQStrUtil::matchStrs(option, strs);

    if      (matchStrs.size() == 1) {
      matchStr = matchStrs[0];
      exact    = true;
    }
    else if (matchStrs.size() > 1) {
      matchStr = widget->showCompletionChooser(matchStrs);

      if (matchStr != "") {
        matchStr = "-" + matchStr + " ";
        exact    = true;
      }
    }
  }

  //---

  if (matchStr == "") {
    // use complete command to complete command option
    auto cmd = QString("complete -command {%1} -option {%2} -exact_space").
                       arg(command).arg(option);

    QVariant res;

    (void) qtcl_->eval(cmd, res);

    matchStr = res.toString();
  }

  newText = lhs;

  if (matchStr != "")
    newText += matchStr;

  if (exact)
    newText += " ";

  newText += rhs;

  return (matchStr.length() > option.length());
}

bool
CQChartsCmdLine::
completeArg(CQChartsCmdWidget *widget, const QString &command, const QString &option,
            const OptionValues &optionValues, const QString &lhs, const QString &arg,
            const QString &rhs, bool interactive, QString &newText) const
{
  std::string nameValues;

  for (const auto &nv : optionValues) {
    if (! nameValues.empty())
      nameValues += " ";

    nameValues += "{{" + nv.first + "} {" + nv.second + "}}";
  }

  nameValues = "{" + nameValues + "}";

  //---

  // get all option values for interactive complete
  QString matchStr;

  if (interactive) {
    auto cmd =
      QString("complete -command {%1} -option {%2} -value {*} -name_values %3 -all").
              arg(command).arg(option).arg(QString::fromStdString(nameValues));

    QVariant res;

    (void) qtcl_->eval(cmd, res);

    auto strs = resultToStrings(res);

    auto matchStrs = CQStrUtil::matchStrs(arg, strs);

    if (matchStrs.size() > 1) {
      matchStr = widget->showCompletionChooser(matchStrs);

      if (matchStr != "")
        matchStr = matchStr + " ";
    }
  }

  //---

  if (matchStr == "") {
    // use complete command to complete command option value
    QVariant res;

    auto cmd =
      QString("complete -command {%1} -option {%2} -value {%3} -name_values %4 -exact_space").
              arg(command).arg(option).arg(arg).arg(QString::fromStdString(nameValues));

    qtcl_->eval(cmd, res);

    matchStr = res.toString();
  }

  newText = lhs + matchStr + rhs;

  return (matchStr.length() > arg.length());
}

QStringList
CQChartsCmdLine::
resultToStrings(const QVariant &var) const
{
  QStringList strs;

  if (CQChartsVariant::isList(var)) {
    auto vars = var.toList();

    for (int i = 0; i < vars.length(); ++i) {
      auto str = vars[i].toString();

      strs.push_back(str);
    }
  }
  else {
    if (! CQTcl::splitList(var.toString(), strs))
      strs << var.toString();
  }

  return strs;
}

//---

void
CQChartsCmdLine::
executeCommand(const QString &line)
{
  auto line1 = line.trimmed();

  if (! line1.length())
    return;

  if (line1[0] == '!') {
    line1 = line.mid(1);

    std::string str;

    COSProcess::executeCommand(line1.toStdString(), str);

    command_->outputText(QString::fromStdString(str));

    return;
  }

  COSExec::grabOutput();

  bool log = true;

  int rc = qtcl_->eval(line, /*showError*/true, /*showResult*/log);

  std::cout << std::flush;

  if (rc != TCL_OK)
    std::cerr << "Invalid line: '" << line.toStdString() + "'\n";

  std::string str;

  COSExec::readGrabbedOutput(str);

  COSExec::ungrabOutput();

  command_->outputText(QString::fromStdString(str));
}

QSize
CQChartsCmdLine::
sizeHint() const
{
  QFontMetrics fm(font());

  return QSize(fm.horizontalAdvance("X")*80, fm.height()*25);
}
