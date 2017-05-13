// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
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
#include "DataModel/RequestResponse/InputPointOfSale.h"
#include "DomainDataObjects/PointOfSale.h"
#include "Factories/PointOfSaleFactory.h"

namespace tax
{

PointOfSale
PointOfSaleFactory::createFromInput(const InputPointOfSale& inputPointOfSale)
{
  PointOfSale result;
  result.loc() = inputPointOfSale.loc();
  result.agentPcc() = inputPointOfSale.agentPcc();
  result.vendorCrsCode() = inputPointOfSale.vendorCrsCode();
  result.carrierCode() = inputPointOfSale.carrierCode();
  result.agentDuty() = inputPointOfSale.agentDuty();
  result.agentFunction() = inputPointOfSale.agentFunction();
  result.agentCity() = inputPointOfSale.agentCity();
  result.iataNumber() = inputPointOfSale.iataNumber();
  result.ersp() = inputPointOfSale.ersp();
  result.agentAirlineDept() = inputPointOfSale.agentAirlineDept();
  result.agentOfficeDesignator() = inputPointOfSale.agentOfficeDesignator();

  return result;
}

} // namespace tax
