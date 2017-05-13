//-------------------------------------------------------------------
//
//  File: RoutingSequenceGenerator.cpp
//  Author:Abu
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "FareDisplay/RoutingSequenceGenerator.h"

#include "Common/Logger.h"
#include "Common/RoutingUtil.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayResponse.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/PaxTypeFare.h"
#include "Routing/RoutingConsts.h"

#include <algorithm>
#include <map>
#include <set>
#include <utility>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.FareDisplay.RoutingSequenceGenerator");

RoutingSequenceGenerator::~RoutingSequenceGenerator() {}

RoutingSequenceGenerator::RoutingSequenceGenerator() {}

bool
RoutingSequenceGenerator::generateRoutingSequence(std::list<PaxTypeFare*>& fares)
{
  generate(fares, _generatedSequence, _processedGlobals);
  return true;
}

struct GetUniqueGlobals : public std::unary_function<PaxTypeFare, GlobalDirection>

{
  GlobalDirection operator()(const PaxTypeFare* p) const { return p->globalDirection(); }

  // lint -e{1509}
};

void
RoutingSequenceGenerator::generate(std::list<PaxTypeFare*>& fares,
                                   std::map<RoutingSequence, std::string>& generatedSequence,
                                   std::map<GlobalDirection, uint16_t>& processedGlobals)
{
  GlobalDirection currentGlobal(GlobalDirection::ZZ);
  uint16_t counter = 0;

  std::map<RoutingSequence, std::string>::iterator r(generatedSequence.end()),
      end(generatedSequence.end());
  std::list<PaxTypeFare*>::iterator i(fares.begin());
  for (; i != fares.end(); ++i)
  {
    const PaxTypeFare* fare(*i);
    if (fare == nullptr)
      continue;

    FareDisplayInfo* info = const_cast<FareDisplayInfo*>(fare->fareDisplayInfo());
    if (info == nullptr)
      continue;

    // user routing vendor that was saved during FBR processing
    VendorCode vendor = info->routingVendor().empty() ? fare->vendor() : info->routingVendor();
    RoutingNumber origAddonRouting, destAddonRouting;

    // TODO maybe other places where this test is better than isContructed())
    if (fare->hasConstructedRouting())
    {
      origAddonRouting = fare->origAddonRouting();
      destAddonRouting = fare->destAddonRouting();
    }

    RoutingNumber routingNum = fare->routingNumber();
    RoutingUtil::getVendor(*i, info->fareDisplayTrx().dataHandle(), vendor);
    if (routingNum == CAT25_EMPTY_ROUTING)
    {
      routingNum = fare->baseFare()->routingNumber();
      vendor = fare->baseFare()->vendor();
    }

    RoutingSequence seq(fare->globalDirection(),
                        routingNum,
                        origAddonRouting,
                        destAddonRouting,
                        fare->carrier(),
                        vendor);

    r = generatedSequence.find(seq);

    if (r != end)
    {

      info->routingSequence() = r->second;
    }
    else
    {
      std::map<GlobalDirection, uint16_t>::iterator j(processedGlobals.end());
      currentGlobal = fare->globalDirection();
      if ((j = processedGlobals.find(currentGlobal)) == processedGlobals.end())
      {
        counter = 1;
        processedGlobals.insert(std::make_pair(currentGlobal, counter));
      }
      else
      {
        ++(j->second);
        counter = j->second;
      }

      info->routingSequence() = getRTGSeq(currentGlobal, counter);
      generatedSequence.insert(
          std::map<RoutingSequence, std::string>::value_type(seq, info->routingSequence()));
    }
  }
}

// -------------------------------------------------------
// TODO: We need to remove this method during cleanup of old templates
// -------------------------------------------------------
void
RoutingSequenceGenerator::generate(std::vector<AddonFareInfo*>& addOnFareInfoList,
                                   std::map<const AddonFareInfo*, std::string>& addOnRoutingSeq,
                                   const RecordScope& crossRefType) const
{
  return;
}

// -----------------------------------------
// New generate method for FDAddOnFareInfo
// -----------------------------------------
void
RoutingSequenceGenerator::generate(std::vector<FDAddOnFareInfo*>& addOnFareInfoList,
                                   const RecordScope& crossRefType) const
{
  std::map<RoutingSequence, std::string> generatedSequence;
  std::map<RoutingSequence, std::string>::iterator r(generatedSequence.end()),
      end(generatedSequence.end());

  GlobalDirection currentGlobal(GlobalDirection::ZZ);
  GlobalDirection lastGlobal(GlobalDirection::ZZ);
  uint16_t counter = 0;

  std::vector<FDAddOnFareInfo*>::iterator i = addOnFareInfoList.begin();

  for (; i != addOnFareInfoList.end(); ++i)
  {
    FDAddOnFareInfo* fare(*i);
    if (fare == nullptr)
      continue;
    // -------------------------------------
    // If global direction is empty display routing
    // -------------------------------------
    if (fare->globalDir() == GlobalDirection::ZZ)
    {
      fare->addOnRoutingSeq() = fare->routing();
      continue;
    }

    // ------------------------------------------------------
    // if Domestic && routing != 0000 ( mileage ) use routing
    // making it at par legacy
    // ------------------------------------------------------
    if (crossRefType == DOMESTIC && fare->routing() != MILEAGE_ROUTING)
    {
      fare->addOnRoutingSeq() = fare->routing();
      continue;
    }

    // ------------------------------------------------
    // Generate routing or use existing routing seq no
    // ------------------------------------------------
    RoutingSequence seq(fare->globalDir(),
                        fare->routing(),
                        EMPTY_STRING(),
                        EMPTY_STRING(),
                        fare->carrier(),
                        fare->vendor());
    r = generatedSequence.find(seq);

    if (r != end)
    {
      fare->addOnRoutingSeq() = r->second;
      continue;
    }
    currentGlobal = fare->globalDir();
    if (currentGlobal != lastGlobal)
    {
      lastGlobal = currentGlobal;
      counter = 1;
    }
    else
    {
      counter++;
    }
    fare->addOnRoutingSeq() = getRTGSeq(currentGlobal, counter);
    generatedSequence.insert(
        std::map<RoutingSequence, std::string>::value_type(seq, fare->addOnRoutingSeq()));
  }
  return;
}

void
RoutingSequenceGenerator::getRoutingOrder(std::string& order, uint16_t counter) const
{
  std::ostringstream sequence;
  if (counter < TWO_DIGIT)
  {
    sequence << FILLER_ZERO << counter;
  }
  else
  {
    sequence << counter;
  }

  order = sequence.str();
}

std::string
RoutingSequenceGenerator::getRTGSeq(const GlobalDirection& currentGlobal, uint16_t counter) const
{
  std::string order;
  std::string gd;
  getRoutingOrder(order, counter);
  globalDirectionToStr(gd, currentGlobal);
  return (gd + order);
}
}
