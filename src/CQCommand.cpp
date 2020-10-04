#include <CQCommand.h>
#include <CQUtil.h>

#include <QApplication>
#include <QPainter>
#include <QFontMetrics>
#include <QScrollBar>
#include <QContextMenuEvent>
#include <QMenu>
#include <QKeyEvent>
#include <QClipboard>
#include <QEventLoop>

#include <iostream>

namespace CQCommand {

ScrollArea::
ScrollArea(QWidget *parent) :
 QScrollArea(parent)
{
  setObjectName("scrolledCommand");

  setVerticalScrollBarPolicy  (Qt::ScrollBarAlwaysOn);
  setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
}

void
ScrollArea::
init()
{
  command_ = createCommandWidget();

  setWidget(command_);

  connect(command_, SIGNAL(executeCommand(const QString &)),
          this, SIGNAL(executeCommand(const QString &)));

  connect(command_, SIGNAL(scrollEnd()), this, SLOT(updateScroll()));

  setFocusProxy(command_);

  initialized_ = true;
}

CommandWidget *
ScrollArea::
createCommandWidget() const
{
  return new CommandWidget(const_cast<ScrollArea *>(this));
}

CommandWidget *
ScrollArea::
getCommand() const
{
  if (! initialized_)
    const_cast<ScrollArea *>(this)->init();

  return command_;
}

void
ScrollArea::
resizeEvent(QResizeEvent *)
{
  //---

  int w = width();
  int h = height();

  int sh = horizontalScrollBar()->height();
  int sw = verticalScrollBar  ()->width ();

  getCommand()->updateSize(w - sw, h - sh);

  updateScroll();
}

void
ScrollArea::
outputText(const QString &str)
{
  getCommand()->outputText(str);
}

void
ScrollArea::
updateScroll()
{
  ensureVisible(0, getCommand()->height() - 1);
}

//------------

CommandWidget::
CommandWidget(QWidget *parent) :
 QFrame(parent)
{
  setObjectName("command");

  //---

  prompt_ = "> ";

  parentWidth_  = 0;
  parentHeight_ = 0;

  setFocusPolicy(Qt::StrongFocus);

  updateSize(0, 0);

  //---

  auto fixedFont = CQUtil::getMonospaceFont();

  setFont(fixedFont);

  //---

  setContextMenuPolicy(Qt::DefaultContextMenu);
}

CommandWidget::
~CommandWidget()
{
}

QSize
CommandWidget::
sizeHint() const
{
  return QSize(std::max(width(), 100), std::max(height(), 100));
}

void
CommandWidget::
updateSize(int w, int h)
{
  if (w > 0 && h > 0) {
    parentWidth_  = w;
    parentHeight_ = h;
  }

  int numLines = std::max(int(lines_.size()) + 1, minLines());

  QFontMetrics fm(font());

  charHeight_ = fm.height();

  int w1 = std::max(parentWidth_, 100);
  int h1 = std::max(parentHeight_, (numLines + 1)*charHeight_);

  resize(w1, h1);

  setMinimumSize(w1, h1);
  setMaximumSize(w1, h1);
}

void
CommandWidget::
paintEvent(QPaintEvent *)
{
  QPainter painter(this);

  painter.fillRect(rect(), bgColor_);

  //---

//int w = width();
  int h = height();

  QFontMetrics fm(font());

  charWidth_   = fm.width("X");
  charHeight_  = fm.height();
  charAscent_  = fm.ascent();
  charDescent_ = fm.descent();

  //---

  int indLen = 1;
  int indVal = lastInd_;

  while (indVal > 9) {
    indVal /= 10;

    ++indLen;
  }

  indMargin_ = charWidth_*indLen;

  //--

  int x = indMargin_;
  int y = h - 1; // bottom

  //---

  // draw prompt
  promptY_     = y;
  promptWidth_ = prompt().length()*charWidth_;

  drawPrompt(&painter, nullptr, y);

  x += promptWidth_;

  //---

  // get entry text before/after cursor
  const auto &str = entry_.getText();
  int         pos = entry_.getPos();

  auto lhs = str.mid(0, pos);
  auto rhs = str.mid(pos);

  //---

  // draw entry text before cursor
  painter.setPen(commandColor_);

  drawText(&painter, x, y, lhs);

  x += lhs.length()*charWidth_;

  int cx = x;

  drawText(&painter, x, y, rhs);

  x += rhs.length()*charWidth_;

  //---

  // draw cursor
  drawCursor(&painter, cx, y, (rhs.length() ? rhs[0] : QChar()));

  //---

  // draw lines (bottom to top)
  y -= charHeight_;

  int numLines = lines_.size();

  for (int i = numLines - 1; i >= 0; --i) {
    const auto &line = lines_[i];

    drawLine(&painter, line, y);

    y -= charHeight_;
  }

  //---

  int lineNum1 = pressLineNum_;
  int charNum1 = pressCharNum_;
  int lineNum2 = moveLineNum_;
  int charNum2 = moveCharNum_;

  if (lineNum1 > lineNum2 || (lineNum1 == lineNum2 && charNum1 > charNum2)) {
    std::swap(lineNum1, lineNum2);
    std::swap(charNum1, charNum2);
  }

  if (lineNum1 != lineNum2 || charNum1 != charNum2)
    drawSelectedChars(&painter, lineNum1, charNum1, lineNum2, charNum2);
}

void
CommandWidget::
drawCursor(QPainter *painter, int x, int y, const QChar &c)
{
  QRect r(x, y - charHeight_, charWidth_, charHeight_);

  if (hasFocus()) {
    painter->fillRect(r, cursorColor_);

    if (! c.isNull()) {
      painter->setPen(bgColor_);

      drawText(painter, x, y, QString(c));
    }
  }
  else {
    painter->setPen(cursorColor_);

    painter->drawRect(r);
  }
}

void
CommandWidget::
drawLine(QPainter *painter, Line *line, int y)
{
  line->setY(y);

  if      (line->type() == LineType::OUTPUT)
    line->setX(0);
  else if (line->type() == LineType::COMMAND)
    line->setX(indMargin_ + promptWidth_);
  else
    line->setX(0);

  if (y + charHeight_ < 0)
    return;

  if (line->type() == LineType::COMMAND)
    drawPrompt(painter, line, y);

  if      (line->type() == LineType::OUTPUT)
    painter->setPen(outputColor_);
  else if (line->type() == LineType::COMMAND)
    painter->setPen(commandColor_);
  else
    painter->setPen(Qt::red);

  int x = line->x();

  const auto &parts = line->parts();

  if (parts.empty()) {
    const auto &text = line->text();

    drawText(painter, x, y, text);
  }
  else {
    for (const auto &part : parts) {
      if (part.color.isValid())
        painter->setPen(part.color);
      else
        painter->setPen(outputColor_);

      const auto &text = part.text;

      drawText(painter, x, y, text);
    }
  }
}

void
CommandWidget::
drawPrompt(QPainter *painter, Line *line, int y)
{
  int ind = (line ? line->ind() : lastInd_ + 1);

  painter->setPen(indColor_);

  int x = 0;

  drawText(painter, x, y, QString("%1").arg(ind));

  x += indMargin_;

  painter->setPen(promptColor_);

  drawText(painter, x, y, prompt());
}

void
CommandWidget::
drawSelectedChars(QPainter *painter, int lineNum1, int charNum1, int lineNum2, int charNum2)
{
  int numLines = lines_.size();

  painter->setPen(bgColor_);

  for (int i = lineNum1; i <= lineNum2; ++i) {
    if (i < 0 || i >= numLines) continue;

    const auto &line = lines_[i];

    int ty = line->y();
    int tx = line->x();

    const auto &text = line->text();

    int len = text.size();

    for (int j = 0; j < len; ++j) {
      if      (i == lineNum1 && i == lineNum2) {
        if (j < charNum1 || j > charNum2)
          continue;
      }
      else if (i == lineNum1) {
        if (j < charNum1)
          continue;
      }
      else if (i == lineNum2) {
        if (j > charNum2)
          continue;
      }

      int tx1 = tx + j*charWidth_;

      painter->fillRect(QRect(tx1, ty - charHeight_, charWidth_, charHeight_), outputColor_);

      drawText(painter, tx1, ty, text.mid(j, 1));
    }
  }
}

void
CommandWidget::
drawText(QPainter *painter, int x, int y, const QString &text)
{
  for (int i = 0; i < text.length(); ++i) {
    painter->drawText(x, y - charDescent_, text[i]);

    x += charWidth_;
  }
}

void
CommandWidget::
mousePressEvent(QMouseEvent *e)
{
  if      (e->button() == Qt::LeftButton) {
    pixelToText(e->pos(), pressLineNum_, pressCharNum_);

    moveLineNum_ = pressLineNum_;
    moveCharNum_ = pressCharNum_;
    pressed_     = true;
  }
  else if (e->button() == Qt::MiddleButton) {
    paste();

    update();
  }
}

void
CommandWidget::
mouseMoveEvent(QMouseEvent *e)
{
  if (pressed_) {
    pixelToText(e->pos(), moveLineNum_, moveCharNum_);

    update();
  }
}

void
CommandWidget::
mouseReleaseEvent(QMouseEvent *e)
{
  if (pressed_)
    pixelToText(e->pos(), moveLineNum_, moveCharNum_);

  pressed_ = false;
}

void
CommandWidget::
pixelToText(const QPoint &p, int &lineNum, int &charNum)
{
  int numLines = lines_.size();

  lineNum = -1;
  charNum = -1;

  for (int i = 0; i < numLines; ++i) {
    const auto &line = lines_[i];

    int y2 = line->y(); // bottom
    int y1 = y2 - charHeight_ + 1;

    if (p.y() >= y1 && p.y() <= y2) {
      lineNum = i;
      charNum = (p.x() - line->x())/charWidth_;
      break;
    }
  }
}

bool
CommandWidget::
event(QEvent *e)
{
  if (e->type() == QEvent::KeyPress) {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);

    if (ke->key() == Qt::Key_Tab) {
      CompleteMode completeMode = CompleteMode::Longest;

      if (ke->modifiers() & Qt::ControlModifier)
        completeMode = CompleteMode::Interactive;

      QString text;

      if (complete(entry_.getText(), entry_.getPos(), text, completeMode)) {
        entry_.setText(text);

        entry_.cursorEnd();

        update();
      }

      return true;
    }
  }

  return QFrame::event(e);
}

void
CommandWidget::
keyPressEvent(QKeyEvent *event)
{
  auto key = event->key();
  auto mod = event->modifiers();

  if (key == Qt::Key_Return || key == Qt::Key_Enter) {
    auto str = entry_.getText();

    outputTypeText(str + "\n", LineType::COMMAND, ++lastInd_);

    entry_.clear();

    emit executeCommand(str);

    emit scrollEnd();

    commands_.push_back(str);

    commandNum_ = -1;
  }
  else if (key == Qt::Key_Left) {
    entry_.cursorLeft();
  }
  else if (key == Qt::Key_Right) {
    entry_.cursorRight();
  }
  else if (key == Qt::Key_Up) {
    QString command;

    int pos = commands_.size() + commandNum_;

    if (pos >= 0 && pos < commands_.size()) {
      command = commands_[pos];

      --commandNum_;
    }

    entry_.setText(command);

    entry_.cursorEnd();
  }
  else if (key == Qt::Key_Down) {
    ++commandNum_;

    QString command;

    int pos = commands_.size() + commandNum_;

    if (pos >= 0 && pos < commands_.size())
      command = commands_[pos];
    else
      commandNum_ = -1;

    entry_.setText(command);

    entry_.cursorEnd();
  }
  else if (key == Qt::Key_Backspace) {
    entry_.backSpace();
  }
  else if (key == Qt::Key_Delete) {
    entry_.deleteChar();
  }
  else if (key == Qt::Key_Insert) {
    paste();
  }
  else if (key == Qt::Key_Tab) {
    // handled by event
  }
  else if (key == Qt::Key_Home) {
    // TODO
  }
  else if (key == Qt::Key_Escape) {
    // TODO
  }
  else if ((key >= Qt::Key_A && key <= Qt::Key_Z) && (mod & Qt::ControlModifier)) {
    if      (key == Qt::Key_A) // beginning
      entry_.cursorStart();
    else if (key == Qt::Key_E) // end
      entry_.cursorEnd();
//  else if (key == Qt::Key_U) // delete all before
//    entry_.clearBefore();
    else if (key == Qt::Key_H)
      entry_.backSpace();
//  else if (key == Qt::Key_W) // delete word before
//    entry_.clearWordBefore();
//  else if (key == Qt::Key_K) // delete all after
//    entry_.clearAfter();
//  else if (key == Qt::Key_T) // swap chars before
//    entry_.swapBefore();
    else if (key == Qt::Key_C)
      copy(selectedText());
    else if (key == Qt::Key_V)
      paste();
  }
  else {
    entry_.insert(event->text());
  }

  update();
}

void
CommandWidget::
contextMenuEvent(QContextMenuEvent *e)
{
  auto *menu = new QMenu;

  //---

  auto *copyAction  = menu->addAction("&Copy\tCtrl+C");
  auto *pasteAction = menu->addAction("&Paste\tCtrl+V");

  connect(copyAction, SIGNAL(triggered()), this, SLOT(copySlot()));
  connect(pasteAction, SIGNAL(triggered()), this, SLOT(pasteSlot()));

  //---

  (void) menu->exec(e->globalPos());

  delete menu;
}

QString
CommandWidget::
selectedText() const
{
  int lineNum1 = pressLineNum_;
  int charNum1 = pressCharNum_;
  int lineNum2 = moveLineNum_;
  int charNum2 = moveCharNum_;

  if (lineNum1 > lineNum2 || (lineNum1 == lineNum2 && charNum1 > charNum2)) {
    std::swap(lineNum1, lineNum2);
    std::swap(charNum1, charNum2);
  }

  if (lineNum1 == lineNum2 && charNum1 == charNum2)
    return "";

  int numLines = lines_.size();

  QString str;

  for (int i = lineNum1; i <= lineNum2; ++i) {
    if (i < 0 || i >= numLines) continue;

    const auto &line = lines_[i];

    const auto &text = line->text();

    if (str.length() > 0)
      str += "\n";

    int len = text.size();

    for (int j = 0; j < len; ++j) {
      if      (i == lineNum1 && i == lineNum2) {
        if (j < charNum1 || j > charNum2)
          continue;
      }
      else if (i == lineNum1) {
        if (j < charNum1)
          continue;
      }
      else if (i == lineNum2) {
        if (j > charNum2)
          continue;
      }

      str += text.mid(j, 1);
    }
  }

  return str;
}

void
CommandWidget::
copySlot()
{
  copy(selectedText());
}

void
CommandWidget::
pasteSlot()
{
  paste();

  update();
}

void
CommandWidget::
copy(const QString &text) const
{
  auto *clipboard = QApplication::clipboard();

  clipboard->setText(text, QClipboard::Selection);
}

void
CommandWidget::
paste()
{
  auto *clipboard = QApplication::clipboard();

  auto text = clipboard->text(QClipboard::Selection);

  entry_.insert(text);
}

void
CommandWidget::
outputText(const QString &str)
{
  outputTypeText(str, CQCommand::LineType::OUTPUT, -1);
}

void
CommandWidget::
outputTypeText(const QString &str, const LineType &type, int ind)
{
  static QString buffer;

  buffer += str;

  auto pos = buffer.indexOf('\n');

  while (pos != -1) {
    auto lhs = buffer.mid(0, pos);

    auto *line = new Line(lhs, type, ind);

    if (type == CQCommand::LineType::OUTPUT)
      line->initParts();

    lines_.push_back(line);

    buffer = buffer.mid(pos + 1);

    pos = buffer.indexOf('\n');
  }

  updateSize(0, 0);
}

QString
CommandWidget::
showCompletionChooser(const QStringList &strs, bool modal)
{
  if (! completionList_)
    completionList_ = new CompletionList(this);

  completionList_->move(0, height() - charHeight_);

  int margin = 4;
  int border = 10;

  int w = 0;

  for (const auto &str : strs)
    w = std::max(w, str.length()*charWidth_);

  if (w + 2*border >= width() - 2*margin)
    w = width() - 2*margin - 2*border;

  int h = strs.size()*charHeight_;

  if (h + 2*border >= height() - charHeight_ - 2*margin)
    h = height() - charHeight_ - 2*margin - 2*border;

  completionList_->move(margin, height() - h - 2*border - charHeight_ - margin);
  completionList_->resize(w + 2*border, h + 2*border);

  completionList_->clear();

  for (const auto &str : strs)
    completionList_->addItem(str);

  completionList_->setCurrentItem(completionList_->item(0));

  completionList_->show();

  completionList_->setFocus();

  completionItem_ = "";

  if (modal) {
    connect(completionList_, SIGNAL(itemSelected(const QString &)),
            this, SLOT(completionSelectedSlot(const QString &)));
    connect(completionList_, SIGNAL(itemCancelled()), this, SLOT(completionCancelledSlot()));

    assert(! eventLoop_);

    eventLoop_ = new QEventLoop;

    (void) eventLoop_->exec();

    disconnect(completionList_, SIGNAL(itemSelected(const QString &)),
               this, SLOT(completionSelectedSlot(const QString &)));
    disconnect(completionList_, SIGNAL(itemCancelled()), this, SLOT(completionCancelledSlot()));

    delete eventLoop_;

    eventLoop_ = nullptr;
  }

  return completionItem_;
}

void
CommandWidget::
completionSelectedSlot(const QString &str)
{
  completionItem_ = str;

  if (eventLoop_)
    eventLoop_->exit();
}

void
CommandWidget::
completionCancelledSlot()
{
  completionItem_ = "";

  if (eventLoop_)
    eventLoop_->exit();
}

//------

CommandWidget::Line::
Line(const QString &text, LineType type, int ind) :
 text_(text), type_(type), ind_(ind)
{
}

void
CommandWidget::Line::
setText(const QString &s)
{
  text_ = s;

  parts_.clear();
}

void
CommandWidget::Line::
setType(const LineType &t)
{
  type_ = t;
}

void
CommandWidget::Line::
initParts()
{
  auto getEscapeColor = [](int n) {
    if      (n ==  0) return QColor();
    else if (n == 31) return QColor(Qt::red);
    else if (n == 32) return QColor(Qt::green);
    else if (n == 33) return QColor(Qt::yellow);
    else if (n == 34) return QColor(Qt::blue);
    else if (n == 35) return QColor(Qt::magenta);
    else if (n == 36) return QColor(Qt::cyan);
    else if (n == 37) return QColor(Qt::white);
    else              return QColor();
  };

  parts_.clear();

  int len = text_.length();

  Part part;

  for (int i = 0; i < len; ++i) {
    char c = text_[i].toLatin1();

    if (c == '') {
      if (part.text.length()) {
        parts_.push_back(part);

        part = Part();
      }

      c = text_[++i].toLatin1();

      if (i < len && c == '[') {
        c = text_[++i].toLatin1();

        int n = 0;

        while (i < len && isdigit(c)) {
          n = n*10 + (c - '0');

          c = text_[++i].toLatin1();
        }

        if (i < len && c == 'm') {
          part.color = getEscapeColor(n);
        }
      }
    }
    else {
      part.text += text_[i];
    }
  }

  if (part.text.length())
    parts_.push_back(part);
}

//------

CompletionList::
CompletionList(CommandWidget *w) :
 QListWidget(w), w_(w)
{
  setFocusPolicy(Qt::StrongFocus);
}

bool
CompletionList::
event(QEvent *e)
{
  if (e->type() == QEvent::KeyPress) {
    QKeyEvent *ke = static_cast<QKeyEvent *>(e);

    if      (ke->key() == Qt::Key_Escape) {
      emit itemCancelled();

      hide();

      return true;
    }
    else if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter) {
      if (currentItem())
        emit itemSelected(currentItem()->text());

      hide();

      return true;
    }
    else if (ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down) {
      QListWidget::event(e);

      return true;
    }
  }

  return QListWidget::event(e);
}

}
