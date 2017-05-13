//-------------------------------------------------------------------
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "DataModel/RexExchangeTrx.h"

#include "DataModel/ExcItin.h"
#include "DataModel/FarePath.h"

namespace tse
{

bool
RexExchangeTrx::repriceWithSameFareDate()
{
  return (RexBaseTrx::PRICE_NEWITIN_PHASE == _trxPhase &&
          !getMultiItinData().repriceWithDiffDates());
}

bool
RexExchangeTrx::needRetrieveKeepFareAnyItin() const
{
  for (MultiItinData* multiItinData : _multiItinData)
    if (multiItinData->fareRetrievalFlags().isSet(FareMarket::RetrievKeep))
      return true;

  return false;
}

void
RexExchangeTrx::ititializeMultiItinData()
{
  _multiItinData.reserve(_itin.size());

  for(uint32_t i = 0; i < _itin.size(); ++i)
  {
    MultiItinData* multiItinData;
    _dataHandle.get(multiItinData);
    multiItinData->departureDateValidator().assign(*this);
    _multiItinData.push_back(multiItinData);
  }
}
} // tse namespace
