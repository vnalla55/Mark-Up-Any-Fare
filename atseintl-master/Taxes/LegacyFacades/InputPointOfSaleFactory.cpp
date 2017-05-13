// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
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

#include "Common/TrxUtil.h"
#include "DBAccess/Loc.h"
#include "DataModel/Agent.h"
#include "DataModel/PricingTrx.h"
#include "Taxes/AtpcoTaxes/DataModel/RequestResponse/InputPointOfSale.h"
#include "Taxes/LegacyFacades/InputPointOfSaleFactory.h"
#include "Taxes/LegacyFacades/ConvertCode.h"

namespace tax
{

std::unique_ptr<InputPointOfSale>
InputPointOfSaleFactory::createInputPointOfSale(tse::PricingTrx& trx, const tse::Agent& agent,
                                                type::Index myId)
{
  std::unique_ptr<InputPointOfSale> result(new InputPointOfSale);

  const tse::LocCode& saleLoc = tse::TrxUtil::saleLoc(trx)->loc();
  result->agentCity() = tse::toTaxCityCode(saleLoc);
  result->loc() = tse::toTaxAirportCode(saleLoc);
  result->carrierCode() = tse::toTaxCarrierCode(trx.billing()->partitionID());
  result->agentPcc() = tse::toTaxPseudoCityCode(agent.tvlAgencyPCC());
  if (result->agentPcc().empty())
  {
    result->agentPcc() = tse::toTaxPseudoCityCode(saleLoc);
  }
  result->vendorCrsCode() = agent.cxrCode();
  result->agentAirlineDept() = agent.airlineDept();
  result->agentOfficeDesignator() = agent.officeDesignator();
  result->id() = myId;

  return result;
}

}
