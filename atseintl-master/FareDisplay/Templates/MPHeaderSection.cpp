#include "FareDisplay/Templates/MPHeaderSection.h"

#include "Common/DateTime.h"
#include "DataModel/FareDisplayTrx.h"

namespace tse
{
void
MPHeaderSection::buildDisplay()
{
  _trx.response() << _trx.boardMultiCity() << _trx.offMultiCity() << "-";
  if (!_trx.preferredCarriers().empty())
    _trx.response() << *_trx.preferredCarriers().begin();
  else
    _trx.response() << "  ";
  _trx.response() << std::setw(8) << std::right << formatDate(_trx.travelDate()) << "\n";
}

std::string
MPHeaderSection::formatDate(const DateTime& date)
{
  return date.dateToString(DDMMMYY, "");
}
} // tse namespace
