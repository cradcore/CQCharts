#include <CQChartsSymbolSet.h>

CQChartsSymbolSetMgr::
CQChartsSymbolSetMgr(CQCharts *charts) :
 charts_(charts)
{
}

void
CQChartsSymbolSetMgr::
addSymbolSet(CQChartsSymbolSet *symbolSet)
{
  auto p = namedSymbolSets_.find(symbolSet->name());
  assert(p == namedSymbolSets_.end());

  symbolSets_     .push_back(symbolSet);
  namedSymbolSets_.insert(p, NamedSymbolSets::value_type(symbolSet->name(), symbolSet));
}

int
CQChartsSymbolSetMgr::
numSymbolSets() const
{
  return symbolSets_.size();
}

bool
CQChartsSymbolSetMgr::
hasSymbolSet(const QString &name) const
{
  return symbolSet(name);
}

CQChartsSymbolSet *
CQChartsSymbolSetMgr::
symbolSet(const QString &name) const
{
  auto p = namedSymbolSets_.find(name);
  if (p == namedSymbolSets_.end()) return nullptr;

  return (*p).second;
}

CQChartsSymbolSet *
CQChartsSymbolSetMgr::
symbolSet(int i) const
{
  assert(i >= 0 && i < numSymbolSets());

  return symbolSets_[i];
}

QStringList
CQChartsSymbolSetMgr::
symbolSetNames() const
{
  QStringList names;

  for (const auto &symbolSet : symbolSets_)
    names.push_back(symbolSet->name());

  return names;
}

//---

CQChartsSymbolSet::
CQChartsSymbolSet(const QString &name) :
 name_(name)
{
}

void
CQChartsSymbolSet::
addSymbol(const CQChartsSymbol &symbol, bool filled)
{
  assert(symbol.isValid());

  symbols_.push_back(SymbolData(symbol, filled));
}

int
CQChartsSymbolSet::
numSymbols() const
{
  return symbols_.size();
}

const CQChartsSymbolSet::SymbolData &
CQChartsSymbolSet::
symbolData(int i) const
{
  assert(i >= 0 && i < numSymbols());

  return symbols_[i];
}

CQChartsSymbolSet::SymbolData
CQChartsSymbolSet::
interpI(int i) const
{
  int i1;

  if (i < 0) {
    int i2 = -i - 1;
    int i3 = (i2 % numSymbols());

    i1 = numSymbols() - i3 - 1;
  }
  else {
    i1 = (i % numSymbols());
  }

  return symbolData(i1);
}

CQChartsSymbol
CQChartsSymbolSet::
symbol(int i) const
{
  if (i < 0 || i >= numSymbols())
    return CQChartsSymbol();

  return symbols_[i].symbol;
}

bool
CQChartsSymbolSet::
isFilled(int i) const
{
  if (i < 0 || i >= numSymbols())
    return false;

  return symbols_[i].filled;
}

QStringList
CQChartsSymbolSet::
symbolNames() const
{
  QStringList names;

  for (const auto &symbolData : symbols_)
    names.push_back(symbolData.symbol.toString());

  return names;
}

bool
CQChartsSymbolSet::
moveUp(int i)
{
  if (i <= 0 || i >= numSymbols())
    return false;

  std::swap(symbols_[i - 1], symbols_[i]);

  return true;
}

bool
CQChartsSymbolSet::
moveDown(int i)
{
  if (i < 0 || i >= numSymbols() - 1)
    return false;

  std::swap(symbols_[i], symbols_[i + 1]);

  return true;
}
