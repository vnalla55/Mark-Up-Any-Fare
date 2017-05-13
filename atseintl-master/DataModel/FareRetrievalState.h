//-------------------------------------------------------------------
//
//  File:        FareRetrievalState.h
//  Created:     August 11, 2009
//
//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
//

#pragma once

#include "DataModel/FareMarket.h"

namespace tse
{

class FareRetrievalState : protected SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>
{
public:
  using SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>::isSet;
  using SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>::set;
  using SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>::isNull;
  using SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>::setNull;
  using SmallBitSet<uint8_t, FareMarket::FareRetrievalFlags>::isAllSet;

  void markHistoricalFare(bool setBits = true) { set(FareMarket::RetrievHistorical, setBits); }

  void markTvlCommenceFare(bool setBits = true) { set(FareMarket::RetrievTvlCommence, setBits); }

  void markKeepFare(bool setBits = true) { set(FareMarket::RetrievKeep, setBits); }

  void markCurrentFare(bool setBits = true) { set(FareMarket::RetrievCurrent, setBits); }

  void markLastReissueFare(bool setBits = true) { set(FareMarket::RetrievLastReissue, setBits); }

  void markAllFare(bool setBits = true)
  {
    set((FareMarket::FareRetrievalFlags)(FareMarket::RetrievHistorical | FareMarket::RetrievKeep |
                                         FareMarket::RetrievTvlCommence | FareMarket::RetrievCurrent
                                         //| FareMarket::RetrievLastReissue
                                         ),
        setBits);
  }

  bool needHistoricalFare() const { return isSet(FareMarket::RetrievHistorical); }

  bool needKeepFare() const { return isSet(FareMarket::RetrievKeep); }

  bool needTvlCommenceFare() const { return isSet(FareMarket::RetrievTvlCommence); }

  bool needCurrentFare() const { return isSet(FareMarket::RetrievCurrent); }

  bool needLastReissueFare() const { return isSet(FareMarket::RetrievLastReissue); }

  bool needAllFare() const
  {
    return isAllSet(
        (FareMarket::FareRetrievalFlags)(FareMarket::RetrievHistorical | FareMarket::RetrievKeep |
                                         FareMarket::RetrievTvlCommence | FareMarket::RetrievCurrent
                                         // | FareMarket::RetrievLastReissue
                                         ));
  }

  bool needRetrieveFare() const { return !isNull(); }
};
}
