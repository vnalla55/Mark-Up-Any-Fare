// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2010
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

#include "Taxes/LegacyTaxes/TaxSP8000.h"

#include "Common/Global.h"
#include "Common/Logger.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TaxTrx.h"
#include "DBAccess/TaxCodeReg.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"
#include "Taxes/LegacyTaxes/UtcUtility.h"

#include <boost/regex.hpp>

using namespace tse;
using namespace std;

log4cxx::LoggerPtr
TaxSP8000::_logger(log4cxx::Logger::getLogger("atseintl.Taxes.TaxSP8000"));

bool
TaxSP8000::validateLocRestrictions(PricingTrx& trx,
                                   TaxResponse& taxResponse,
                                   TaxCodeReg& taxCodeReg,
                                   uint16_t& startIndex,
                                   uint16_t& endIndex)

{
  TaxTrx* taxTrx = dynamic_cast<TaxTrx*>(&trx);

  if (!taxTrx)
    return false;

  Itin* itin = taxResponse.farePath()->itin();

  if (!itin)
    return false;

  if (itin->anciliaryServiceCode().empty())
  {
    return false;
  }
  else
  {
    bool locRestr =
        Tax::validateLocRestrictions(trx, taxResponse, taxCodeReg, startIndex, endIndex);

    std::string cnfVal = utc::getSpecConfigParameter(
        trx, taxCodeReg.specConfigName(), "ANCILLARYSERVICE", trx.getRequest()->ticketingDT());
    if (!cnfVal.empty())
    {
      boost::regex exp(cnfVal);
      bool match = boost::regex_match(itin->anciliaryServiceCode(), exp);
      std::string matchMode = utc::getSpecConfigParameter(
          trx, taxCodeReg.specConfigName(), "POSITIVEMATCH", trx.getRequest()->ticketingDT());
      if (matchMode == "TRUE")
      {
        return match && locRestr;
      }
      return !match && locRestr;
    }

    return locRestr;
  }
}
