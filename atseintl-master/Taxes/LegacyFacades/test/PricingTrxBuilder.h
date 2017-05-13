// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2015
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
#pragma once

#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/Diversity.h"
#include "DataModel/Fare.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/Loc.h"

namespace tse
{

constexpr struct ItinLayoutA{} itinLayoutA; // a tag

class PricingTrxBuilder
{
public:
  PricingTrxBuilder();
  explicit PricingTrxBuilder(ItinLayoutA);

  PricingTrx& trx() { return _trx; }

  Itin i1, i2;
  FarePath f1_1, f1_2, f2_1;
  FarePath f1_1A, f1_1B, f1_1C;
  FarePath f1_2A, f1_2B;
  FarePath f2_1A, f2_1B;

private:
  class Impl;
  void fillIrrelevantData();
  PricingTrx _trx;

  tse::Loc agentLocation;
  tse::Billing billing;
  tse::PricingUnit pricingUnit;
  tse::FareUsage fareUsage;
  tse::FareInfo fareInfo, baseFareInfo;
  tse::Fare fare, baseFare;
  tse::Agent ticketingAgent;
  tse::PricingRequest pricingRequest;
  tse::PricingOptions pricingOptions;
  tse::AirSeg airSeg1, airSeg2;
  tse::Loc hidden1, hidden2;
  tse::Loc loc1, loc2, loc3, loc4;
  tse::PaxTypeFare paxTypeFare, basePaxTypeFare;
  tse::FareByRuleApp fareByRuleApp;
  tse::FBRPaxTypeFareRuleData ruleData;
  tse::FareByRuleItemInfo ruleInfo;
  tse::PaxTypeFare::PaxTypeFareAllRuleData allRuleData;
  tse::PaxType inputPaxType0;
  tse::PaxType outputPaxType;
  tse::PaxType fareOutputPaxType;

private: // val carrier
  tse::ValidatingCxrGSAData validatingCxrGSAData;
};

} // namespace tse

// TODO: do it the right way after I fixed the linking problems
#include "LegacyFacades/test/PricingTrxBuilder.cpp.h"

