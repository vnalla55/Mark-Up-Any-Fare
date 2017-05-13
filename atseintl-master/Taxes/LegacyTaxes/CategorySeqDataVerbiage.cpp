// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Taxes/LegacyTaxes/CategorySeqDataVerbiage.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDisplayFormatter.h"

#include <iterator>
#include <sstream>
#include <boost/date_time/posix_time/posix_time.hpp>
//#include <algorithm>

using namespace tse;

// ----------------------------------------------------------------------------
// <PRE>
//
// @function CategorySeqDataVerb::CategorySeqDataVerb
//
// Description:  Constructor
//
// </PRE>
// ----------------------------------------------------------------------------

CategorySeqDataVerb::CategorySeqDataVerb() : _subCat1(EMPTY_STRING())
{
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function CategorySeqDataVerb::~CategorySeqDataVerb
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------

CategorySeqDataVerb::~CategorySeqDataVerb() {}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function CategorySeqDataVerb::build
//
// Description:  destructor
//
// </PRE>
// ----------------------------------------------------------------------------
void
CategorySeqDataVerb::build(TaxTrx& trx, TaxDisplayItem& taxDisplayItem)
{

  TaxCodeReg* tcreg = taxDisplayItem.taxCodeReg();

  if (!tcreg->effDate().isValid() || !tcreg->firstTvlDate().isValid())
  {
    return;
  }

  // CURRENTLY EFFECTIVE FOR SALES AND TRAVEL
  if ((tcreg->effDate() <= trx.getRequest()->ticketingDT()) &&
      (tcreg->firstTvlDate() <= trx.getRequest()->ticketingDT()))
  {
    expiryDataInfo(trx, tcreg);
    return;
  }

  // CURRENTLY EFFECTIVE FOR SALES AND NOT TRAVEL
  if ((tcreg->effDate() <= trx.getRequest()->ticketingDT()) &&
      (tcreg->firstTvlDate() >= trx.getRequest()->ticketingDT()))
  {
    _subCat1 = "EFFECTIVE FOR TRAVEL ON OR AFTER ";
    _subCat1 += tcreg->firstTvlDate().dateToString(tse::DDMMMYY, "");
  }
  else // NOT EFFICTIVE FOR SALES AND TRAVEL
  {

    if (tcreg->effDate() == tcreg->firstTvlDate())
    {
      _subCat1 = "EFFECTIVE FOR SALE AND TRAVEL ON OR AFTER ";
      _subCat1 += tcreg->effDate().dateToString(tse::DDMMMYY, "");
    }
    else
    {
      _subCat1 = "EFFECTIVE FOR SALE ON OR AFTER ";
      _subCat1 += tcreg->effDate().dateToString(tse::DDMMMYY, "");

      _subCat1 += " AND TRAVEL ON OR AFTER ";
      _subCat1 += tcreg->firstTvlDate().dateToString(tse::DDMMMYY, "");
    }
  }

  if (!_subCat1.empty())
  {
    _subCat1 += "\n";
  }

  expiryDataInfo(trx, tcreg);
  return;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function CategorySeqDataVerb:expiryDataInfo
//
// Description:  Expiry sell, travel and tax data info
//
// </PRE>
// ----------------------------------------------------------------------------
void
CategorySeqDataVerb::expiryDataInfo(TaxTrx& trx, TaxCodeReg* tcreg)
{

  if (!tcreg->discDate().isValid() && !tcreg->lastTvlDate().isValid())
  {
    return;
  }

  if (tcreg->lastTvlDate() == tcreg->discDate())
  {
    _subCat1 = "DISCONTINUED FOR SALE AND LAST TRAVEL ON ";
    _subCat1 += tcreg->discDate().dateToString(tse::DDMMMYY, "");
  }
  else
  {
    if (tcreg->discDate().isValid())
    {
      _subCat1 += "DISCONTINUED FOR SALE ON ";
      _subCat1 += tcreg->discDate().dateToString(tse::DDMMMYY, "");

      if (tcreg->lastTvlDate().isValid())
      {
        _subCat1 += "AND ";
      }
    }

    if (tcreg->lastTvlDate().isValid())
    {
      _subCat1 += "LAST TRAVEL DATE BEFORE ";
      _subCat1 += tcreg->lastTvlDate().dateToString(tse::DDMMMYY, "");
    }
  }

  if (tcreg->expireDate().isValid())
  {
    // Tom Holbrook request: if lastTvlDate - expiredate <= 1 day, display only last travel date
    if (tcreg->lastTvlDate().isValid() && tcreg->lastTvlDate() > tcreg->expireDate() &&
        DateTime::diffTime(tcreg->lastTvlDate(), tcreg->expireDate()) > SECONDS_PER_DAY)
    {
      _subCat1 += " - TAX VALUE EXPIRES ON ";
      _subCat1 += tcreg->expireDate().dateToString(tse::DDMMMYY, "");
    }
  }

  if (!_subCat1.empty())
  {
    _subCat1 += "\n";
  }

  return;
}
