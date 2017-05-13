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
#include "DataModel/RequestResponse/InputYqYr.h"
#include "DomainDataObjects/YqYr.h"
#include "Factories/YqYrFactory.h"

namespace tax
{

YqYr
YqYrFactory::createFromInput(const InputYqYr& inputYqYr)
{
  YqYr result;
  result.seqNo() = inputYqYr._seqNo;
  result.amount() = inputYqYr._amount;
  result.originalAmount() = inputYqYr._originalAmount;
  result.originalCurrency() = inputYqYr._originalCurrency;
  result.carrierCode() = inputYqYr._carrierCode;
  result.code() = inputYqYr._code;
  result.type() = inputYqYr._type;
  result.taxIncluded() = inputYqYr._taxIncluded;
  return result;
}

} // namespace tax
