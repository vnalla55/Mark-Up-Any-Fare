//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Pricing/MergedFareMarket.h"

#include "DataModel/PaxTypeFare.h"

#include <limits>

namespace tse
{

bool
MergedFareMarket::cxrFareTypeExists(const PaxType* paxType,
                                    const FareType& fareType,
                                    const CxrFareTypeBitSet& cxrFareTypeBitSet,
                                    const CarrierCode & valCxr) const
{
  const std::map<const PaxType*, std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> > >::const_iterator it =
                                                                    _allCxrFareTypes.find(paxType);
  if (LIKELY(it != _allCxrFareTypes.end()))
  {
    const std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> >& fareTypeMap =
        it->second;
    std::map<FareType, std::map<CarrierCode, CxrFareTypeBitSet> >::const_iterator ftIt =
        fareTypeMap.find(fareType);

    if (ftIt != fareTypeMap.end())
    {
      bool cxrFareExists = false;
      auto jt = ftIt->second.find(valCxr);
      if (jt != ftIt->second.end())
      {
        const CxrFareTypeBitSet& bitSet = jt->second;
        if ((bitSet.value() & cxrFareTypeBitSet.value()) == cxrFareTypeBitSet.value())
        {
          cxrFareExists = true;
        }
      }
      return cxrFareExists;
    }

  }
  return false;
}


bool
MergedFareMarket::cxrFareTypeExists_old(const PaxType* paxType,
                                        const FareType& fareType,
                                        const CxrFareTypeBitSet& cxrFareTypeBitSet) const
{
  const std::map<const PaxType*, std::map<FareType, CxrFareTypeBitSet> >::const_iterator it =
      _allCxrFareTypes_old.find(paxType);
  if (it != _allCxrFareTypes_old.end())
  {
    const std::map<FareType, CxrFareTypeBitSet>& fareTypeMap = it->second;
    const std::map<FareType, CxrFareTypeBitSet>::const_iterator ftIt = fareTypeMap.find(fareType);
    if (ftIt != fareTypeMap.end())
    {
      const CxrFareTypeBitSet& bitSet = ftIt->second;

      return ((bitSet.value() & cxrFareTypeBitSet.value()) ==
              cxrFareTypeBitSet.value()); // for multiple status
    }
  }
  return false;
}

void
MergedFareMarket::findFirstFare(const std::vector<PaxType*>& paxTypes)
{
  for (const PaxType* paxType : paxTypes)
  {
    MoneyAmount min = std::numeric_limits<MoneyAmount>::infinity();
    PaxTypeFare* minFare = nullptr;

    for (const FareMarket* fm : _mergedFareMarket)
    {
      const PaxTypeBucket& cortege = *fm->paxTypeCortege(paxType);
      const std::vector<PaxTypeFare*>& ptfs = cortege.paxTypeFare();
      if (ptfs.empty())
        continue;

      PaxTypeFare* const fare = ptfs.front();
      const MoneyAmount amount = fare->nucAmountWithEstimatedTaxes(cortege);

      if (LIKELY(amount < min))
      {
        min = amount;
        minFare = fare;
      }
    }

    _paxFirstFare[paxType] = { minFare, min };
  }
}

}
