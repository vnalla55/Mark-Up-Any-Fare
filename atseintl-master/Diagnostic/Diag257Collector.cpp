//----------------------------------------------------------------------------
//  Copyright Sabre 2005
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag257Collector.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DiagRequest.h"
#include "AddonConstruction/FareDup.h"
#include "DBAccess/AddonFareInfo.h"

namespace tse
{
namespace
{
const std::string
NOT_APPLICABLE_FOR_TRAVEL_DATE_MSG("FARE REMOVED: NOT APPLICABLE FOR TRAVEL DATE: ");
const std::string
NOT_APPLICABLE_FOR_TICKETING_DATE_MSG("              AND TICKETING DATE: ");

const std::string
SECOND_FARE_REMOVED_MSG("SECOND FARE REMOVED");

const std::string
FARE_CLASS_PRIORITY_MSG("FARE CLASS PRIORITY : ");

const std::string
CONSTR_FARE_DUP_TITLE_MSG("DUPLICATES ELIMINATION");
const std::string
CXR_PREF_SINGLE_OVER_DBL_MSG("CARRIER PREFERS SINGLE OVER DOUBLE");

const std::string
CXR_PREFRS_SINGLE_OVER_DBL_MSG("CARRIER PREFERS SINGLE OVER DBL");
const std::string
LOWER_FARE_AMOUNT_MSG("DUPLICATES, FIRST FARE AMT LESS THAN SECOND");
const std::string
LESS_OR_EQUAL_FARE_AMOUNT_MSG("DUPLICATES, FIRST FARE AMT LESS OR EQUAL TO SECOND");
const std::string
HIGHER_FC_PRIORITY_MSG("DUPLICATES, FIRST FARE HAS HIGHER FARE CLASS PRIORITY");

const std::string
FARE_VENDORS_NE_MSG("VENDORS NOT EQUAL");
const std::string
FARE_CARRIERS_NE_MSG("CARRIERS NOT EQUAL");
const std::string
FARE_MARKETS_NE_MSG("MARKETS NOT EQUAL");
const std::string
FARE_TARIFF_NE_MSG("FARE TARIFFS NOT EQUAL");
const std::string
GLOBAL_DIR_NE_MSG("GLOBAL DIRECTION NOT EQUAL");
const std::string
FARE_CLASS_NE_MSG("FARE CLASS NOT EQUAL");
const std::string
CURRENCIES_NE_MSG("CURRENCIES NOT EQUAL");
const std::string
RULE_NUMBER_NE_MSG("RULE NUMBER NOT EQUAL");
const std::string
OWRT_NE_MSG("OWRT NOT EQUAL");
const std::string
FOOTNOTES_NE_MSG("FOOT NOTES NOT EQUAL");
const std::string
ROUTINGS_NE_MSG("ROUTINGS NOT EQUAL");
const std::string
DIRECTIONALITY_NE_MSG("DIRECTIONALITY NOT EQUAL");
const std::string
KEEP_BOTH_FARES_MSG("BOTH FARES WILL BE KEPT");
}

void
Diag257Collector::writeDupsRemovalHeader()
{
  if (!_active)
    return;

  writeCommonHeader(_cJob->vendorCode());

  (*this) << " \n" << CONSTR_FARE_DUP_TITLE_MSG << ": \n" << CXR_PREF_SINGLE_OVER_DBL_MSG << ": "
          << (_cJob->singleOverDouble() ? "YES" : "NO") << "\n";

  writeFiresHeader();
}

void
Diag257Collector::writeDupsRemovalFooter()
{
  if (_active)
    writeFiresFooter();
}

void
Diag257Collector::writeNotEffective(const ConstructedFareInfo& cfi)
{
  if (!_active || !_diagRequest->isValidForDiag(cfi))
    return;

  writeConstructedFare(cfi);

  (*this) << NOT_APPLICABLE_FOR_TRAVEL_DATE_MSG;
  formatDateTime(_cJob->travelDate());
  (*this) << '\n';

  if (_cJob->isHistorical())
  {
    (*this) << NOT_APPLICABLE_FOR_TICKETING_DATE_MSG;
    formatDateTime(_cJob->ticketingDate());
    (*this) << '\n';
  }

  (*this) << " \n";
}

void
Diag257Collector::writeDupDetail(const ConstructedFareInfo& cfi1,
                                 const ConstructedFareInfo& cfi2,
                                 const DupRemoveReason reason)
{
  if (!_active || !_diagRequest->isValidForDiag(cfi1))
    return;

  writeConstructedFare(cfi1);
  if (reason == DRR_FARE_CLASS_PRIORITY)
    (*this) << FARE_CLASS_PRIORITY_MSG << cfi1.atpFareClassPriority() << "\n";

  writeConstructedFare(cfi2);
  if (reason == DRR_FARE_CLASS_PRIORITY)
    (*this) << FARE_CLASS_PRIORITY_MSG << cfi2.atpFareClassPriority() << "\n";

  switch (reason)
  {
  case DRR_SINGLE_OVER_DBL:
    (*this) << CXR_PREFRS_SINGLE_OVER_DBL_MSG;
    break;

  case DRR_FARE_CLASS_PRIORITY:
    (*this) << HIGHER_FC_PRIORITY_MSG;
    break;

  case DRR_LESS_FARE_AMOUNT:
    (*this) << LOWER_FARE_AMOUNT_MSG;
    break;

  case DRR_LESS_OR_EQ_FARE_AMOUNT:
    (*this) << LESS_OR_EQUAL_FARE_AMOUNT_MSG;
    break;
  }

  (*this) << "\n" << SECOND_FARE_REMOVED_MSG << "\n \n";
}

}
