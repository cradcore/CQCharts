#include <CQChartsEval.h>
#include <CMathUtil.h>
#include <QColor>

CQChartsEval *
CQChartsEval::
instance()
{
  static CQChartsEval *inst;

  if (! inst)
    inst = new CQChartsEval;

  return inst;
}

CQChartsEval::
CQChartsEval()
{
  qtcl_ = new CQTcl;

  addFunc("color", (CQTcl::ObjCmdProc) &CQChartsEval::colorCmd);
}

CQChartsEval::
~CQChartsEval()
{
  delete qtcl_;
}

bool
CQChartsEval::
evalExpr(int row, const QString &exprStr, QVariant &var)
{
  qtcl()->createVar("x", row);

  return qtcl()->evalExpr(exprStr, var);
}

void
CQChartsEval::
addFunc(const QString &name, CQTcl::ObjCmdProc proc)
{
  qtcl_->createExprCommand(name, proc, (CQTcl::ObjCmdData) this);
}

int
CQChartsEval::
colorCmd(ClientData clientData, Tcl_Interp *, int objc, const Tcl_Obj **objv)
{
  auto clamp = [](double x) { return CMathUtil::clamp(x, 0.0, 255.0); };

  CQChartsEval *eval = static_cast<CQChartsEval *>(clientData);

  std::vector<QVariant> vars;

  for (int i = 1; i < objc; ++i) {
    const Tcl_Obj *obj = objv[i];

    vars.push_back(eval->qtcl()->variantFromObj(obj));
  }

  if      (vars.size() == 1) {
    QColor c(vars[0].toString());

    eval->qtcl()->setResult(c);
  }
  else if (vars.size() == 3) {
    bool ok;

    double r = vars[0].toDouble(&ok);
    double g = vars[1].toDouble(&ok);
    double b = vars[2].toDouble(&ok);

    QColor c;

    c.setRgb(clamp(r), clamp(g), clamp(b));

    eval->qtcl()->setResult(c);
  }

  return TCL_OK;
}
