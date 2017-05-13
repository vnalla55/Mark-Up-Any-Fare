//-------------------------------------------------------------------
//
//  File:     OrigDestSection.cpp
//  Author:   Mike Carroll
//  Date:     July 26, 2005
//
//  Copyright Sabre 2005
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/Templates/OrigDestSection.h"

#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/TseUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/Itin.h"
#include "FareDisplay/Templates/Section.h"
#include "Fares/FDFareCurrencySelection.h"

namespace tse
{
static Logger
logger("atseintl.FareDisplay.Templates.OrigDestSection");

void
OrigDestSection::buildDisplay()
{
  std::ostringstream* oss = &_trx.response();

  // Board and off point
  *oss << _trx.boardMultiCity() << "-" << _trx.offMultiCity();

  // Carrier
  oss->setf(std::ios::right, std::ios::adjustfield);
  *oss << std::setw(7) << std::setfill(' ') << " ";

  if (_trx.isShopperRequest() || _trx.multipleCarriersEntered())
  {
    *oss << "SHOP";
    *oss << std::setw(9) << std::setfill(' ') << " ";
  }
  else
  {
    *oss << "CXR-" << *_trx.preferredCarriers().begin();
    *oss << std::setw(7) << std::setfill(' ') << " ";
  }

  // Travel date(s)
  if (_trx.getRequest()->dateRangeLower().isValid() &&
      _trx.getRequest()->dateRangeUpper().isValid())
  {
    *oss << WEEKDAYS_UPPER_CASE[_trx.getRequest()->dateRangeLower().dayOfWeek()] << " "
         << _trx.getRequest()->dateRangeLower().dateToString(DDMMMYY, '\0') << " * "
         << WEEKDAYS_UPPER_CASE[_trx.getRequest()->dateRangeUpper().dayOfWeek()] << " "
         << _trx.getRequest()->dateRangeUpper().dateToString(DDMMMYY, '\0');
    *oss << std::setw(7) << std::setfill(' ') << " ";
  }
  else
  {
    *oss << WEEKDAYS_UPPER_CASE[_trx.getRequest()->requestedDepartureDT().dayOfWeek()] << " "
         << _trx.getRequest()->requestedDepartureDT().dateToString(DDMMMYY, '\0');
    *oss << std::setw(21) << std::setfill(' ') << " ";
  }

  // As Addon Fare doesn't pass through FVO itin::calculationCurrency() doesn't hold
  // the appropriate displaycurrency
  if (_trx.getRequest()->inclusionCode() == FD_ADDON)
  {
    CurrencyCode displayCurrency;
    FDFareCurrencySelection::getDisplayCurrency(_trx, displayCurrency);
    *oss << displayCurrency;
  }
  else
  {
    const Itin* itin = _trx.itin().front();
    *oss << itin->calculationCurrency();
  }

  *oss << std::endl;

  // Return date
  const DateTime travelDate = TseUtil::getTravelDate(_trx.travelSeg());
  if (_trx.getRequest()->returnDate().isValid() && travelDate.isValid())
  {
    *oss << "TRIP DURATION ";
    const TimeDuration diff = _trx.getRequest()->returnDate() - travelDate;
    const int32_t numDays = diff.hours() / HOURS_PER_DAY;
    if (numDays == 1)
    {
      oss->setf(std::ios::right, std::ios::adjustfield);
      *oss << std::setfill(' ') << std::setw(3) << numDays;
      *oss << " DAY ";
    }
    else if (numDays > 1)
    {
      oss->setf(std::ios::right, std::ios::adjustfield);
      *oss << std::setfill(' ') << std::setw(3) << numDays;
      *oss << " DAYS";
    }
    else
    {
      oss->setf(std::ios::right, std::ios::adjustfield);
      *oss << std::setfill(' ') << std::setw(3) << 0;
      *oss << " DAYS";
    }

    spaces(5);

    *oss << WEEKDAYS_UPPER_CASE[travelDate.dayOfWeek()] << " "
         << travelDate.dateToString(DDMMMYY, '\0') << " R "
         << WEEKDAYS_UPPER_CASE[_trx.getRequest()->returnDate().dayOfWeek()] << " "
         << _trx.getRequest()->returnDate().dateToString(DDMMMYY, '\0') << std::endl;
  }

  LOG4CXX_DEBUG(logger, "Display: " << _trx.response().str());
}
} // tse namespace
