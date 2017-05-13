#include "FareDisplay/Templates/MPSection.h"

#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareDisplaySurcharge.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "FareDisplay/MPData.h"

namespace tse
{

using std::ostringstream;
using std::string;
using std::setw;
using std::right;
using std::fixed;
using std::showpoint;
using std::setprecision;

namespace
{
Logger
logger("atseintl.FareDisplay.Templates.MPSection");

const uint16_t SurchargeFactor = 5;
const uint16_t ColumnLengths[MPSection::SurchargeRanges] = { 7, 10, 10, 10, 10, 10 };

template <typename Value>
Value
calculateSurcharge(uint16_t surchargeNo, Value value)
{
  return static_cast<Value>(((static_cast<float>(surchargeNo * SurchargeFactor)) / 100) * value);
}

template <typename Value, typename Vector>
void
populateSurcharges(Value value, Vector& vector)
{
  for (uint16_t i = 0; i < vector.size(); ++i)
  {
    vector[i] = value + calculateSurcharge(i, value);
  }
}
}

// const uint16_t MPSection::SurchargeRanges = 6;

void
MPSection::buildDisplay()
{
  doBuildDisplay();
}

void
MPSection::doBuildDisplay()
{
  if (&_trx != nullptr && _trx.mpData() != nullptr)
  {
    MPMsByGlobal mpmsByGlobal;
    prepareMPMs(mpmsByGlobal);
    Amounts amounts(SurchargeRanges);
    prepareAmounts(amounts);
    displayHeader();
    displayColumnHeaders();
    displaySpace();
    displayData(mpmsByGlobal, amounts);
  }
  else
  {
    LOG4CXX_ERROR(logger, "MPSection not initialized or MPData is null");
  }
}

void
MPSection::prepareMPMs(MPMsByGlobal& mpmsByGlobal)
{
  const GlobalMPMMap& gdMPM(_trx.mpData()->getMPMs());
  if (!gdMPM.empty())
  {
    mpmsByGlobal.clear();
    GlobalMPMMapConstIter i(gdMPM.begin()), e(gdMPM.end());
    for (; i != e; ++i)
    {
      MPMs mpms(SurchargeRanges);
      populateSurcharges(i->second, mpms);
      mpmsByGlobal.insert(MPMsByGlobal::value_type(i->first, mpms));
    }
  }
  else
  {
    LOG4CXX_ERROR(logger, "MPData is empty");
  }
}

void
MPSection::prepareAmounts(Amounts& amounts)
{
  MoneyAmount truncatedValue = _trx.mpData()->getAmount();
  if (_trx.mpData()->getCurrency() == "NUC")
    CurrencyUtil::truncateNUCAmount(truncatedValue);
  populateSurcharges(truncatedValue, amounts);
}

void
MPSection::displayHeader()
{
  displayLine(" \n TICKETED POINT SURCHARGES APPLY.\n");
}

void
MPSection::displayColumnHeaders()
{
  ostringstream o;
  o << setw(6) << "GI";
  for (uint16_t i = 0; i < SurchargeRanges; ++i)
  {
    ostringstream os;
    if (i > 0)
      os << i* SurchargeFactor;
    os << "M";
    displayColumn(o, i, os.str());
  }
  o << "\n";
  displayLine(o.str());
}

void
MPSection::displaySpace()
{
  displayLine(" \n");
}

void
MPSection::displayData(const MPMsByGlobal& mpmsByGlobal, const Amounts& amounts)
{
  for (const auto& mpmsByGlobalItem : mpmsByGlobal)
  {
    string gd;
    globalDirectionToStr(gd, mpmsByGlobalItem.first);
    displayLine("MPM " + gd);
    displayMPMs(mpmsByGlobalItem.second);
    displayLine(_trx.mpData()->getCurrency());
    displayAmounts(amounts);
  }
}

void
MPSection::displayMPMs(const MPMs& mpms)
{
  ostringstream o;
  for (uint16_t i = 0; i < mpms.size(); ++i)
  {
    ostringstream os;
    os << mpms[i];
    displayColumn(o, i, os.str());
  }
  o << "\n";
  displayLine(o.str());
}

void
MPSection::displayAmounts(const Amounts& amounts)
{
  CurrencyCode ccode = _trx.itin().front()->calculationCurrency();
  Money money(ccode);
  CurrencyConverter curConverter;
  ostringstream o;

  for (uint16_t i = 0; i < amounts.size(); ++i)
  {
    ostringstream os;
    os.setf(std::ios::fixed);
    os.precision(money.noDec(_trx.ticketingDate()));

    MoneyAmount displayAmount = amounts[i];

    // For NUC need to truncate amount, not round.
    if (money.isNuc())
    {
      money.value() = amounts[i];
      RoundingFactor roundingUnit = 0.01;

      curConverter.roundNone(money, roundingUnit);
      displayAmount = money.value();
    }
    else
      FareDisplaySurcharge::roundAmount(&_trx, displayAmount);
    os << displayAmount;
    if (i > 0)
      displayColumn(o, i, os.str());
    else
      displayColumn(o, 1, os.str());
  }
  o << "\n";
  displayLine(o.str());
}

void
MPSection::displayLine(const string& s)
{
  _trx.response() << s;
}

void
MPSection::displayColumn(ostringstream& os, uint16_t colNo, const string& str)
{
  if (colNo < SurchargeRanges)
    os << setw(ColumnLengths[colNo]);
  os << right << str;
}
} // tse
