//----------------------------------------------------------------------------
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/TsePrimitiveTypes.h"
#include "Service/Service.h"

#include <vector>

namespace tse
{
class FareMarket;
class FareSelectorService;
class FareSelector;
class FareDisplayTrx;
class TseServer;
class FarePath;
class PaxTypeFare;
class DataHandle;
class ConfigMan;
class StructuredRuleTrx;
class PaxTypeBucket;
class PaxType;
class FareCompInfo;

class FareSelectorService final : public Service
{
  friend class FareSelectorServiceTest;

public:
  FareSelectorService(const std::string& sname, TseServer& tseServer);

  bool initialize(int argc = 0, char* argv[] = nullptr) override;

  bool process(FareDisplayTrx& trx) override;
  bool process(StructuredRuleTrx& trx) override;
  FareSelector* getFareSelector(FareDisplayTrx& trx, DataHandle& dh);
  uint32_t getActiveThreads() override;

private:
  void filterBucketSFR(PaxTypeBucket* bucket,
                       const std::string& fareBasisCode,
                       const MoneyAmount fareAmount,
                       StructuredRuleTrx& trx);

  void processForSinglePassenger(StructuredRuleTrx& trx);
  void processForMultiPassenger(StructuredRuleTrx& trx);
  bool processOWRTForFT(FareDisplayTrx& trx, FarePath& farePath, PaxTypeFare& paxTypeFare);
  void filterAllPassengerBuckets(StructuredRuleTrx& trx);
  void accumulateFaresFromBuckets(StructuredRuleTrx& trx);
  void selectCorrectPaxTypeFaresInBuckets(FareMarket* fm, PricingTrx& trx);

  ConfigMan& _config;
};
}
