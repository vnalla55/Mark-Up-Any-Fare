// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         28-09-2011
//! \file         DiagSoloPQCollector.cpp
//! \brief
//!
//!  Copyright (C) Sabre 2011
//!
//!          The copyright to the computer program(s) herein
//!          is the property of Sabre.
//!          The program(s) may be used and/or copied only with
//!          the written permission of Sabre or in accordance
//!          with the terms and conditions stipulated in the
//!          agreement/contract under which the program(s)
//!          have been supplied.
//
// -------------------------------------------------------------------
#include "Pricing/Shopping/PQ/DiagSoloPQCollector.h"

#include "Common/Assert.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag942Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

namespace tse
{
namespace
{
ConfigurableValue<std::string>
excludedSolutionPatterns("SHOPPING_DIVERSITY", "EXCLUDED_SOLUTION_PATTERNS");
}
namespace shpq
{
namespace
{
const char* const DASH_LINE = "---------------------------------------------------\n";
const char* const STAR_LINE = "***************************************************\n";

const char* const LEGEND =
    ""
    "   PUSH/POP       operation: add/get item to/from the PQ\n"
    "   SP/CR/CRC/FPF  item type: SolPattern/CnxnRoute/CnxnRouteCarrier/FarePathFactory\n"
    "   SPxx           solution pattern number\n"
    "   [xxxx]         unique item identifier\n"
    "   xxxxx.xx       item score, i.e. lower bound amount in NUC\n"
    "   item-info      item specific information described below\n"
    "   (from xxxxxx)  base item for the current item\n"
    "\n"
    "SP/CR/CRC Fields:\n"
    "leg-specific information separated with :, each leg info consists of:\n"
    "   basic-leg-info [additional-leg-info]\n"
    " where\n"
    "   basic-leg-info       short information about this leg\n"
    "   additional-leg-info  first element in to-be-expanded list\n"
    "   _                    marks leg for same-level expansion\n"
    "   *                    marks that leg can no longer be expanded\n"
    "                        since the expansion reached the last element\n"
    "\n"
    "FPF Field:\n"
    "   (noFP)               fare path no available\n"
    "   UNKNW                lower bound score no available\n"
    "   [FPF delta: xxx]     xxx is score of next top PQ item\n"
    "   FP: CTY1-CXR1-FBC1-CTY2 : CTY2-CXR2-FBC2-CTY3 - fare path info\n"
    "       where FP: is a fixed string to mark start of fare path,\n"
    "             CTYx - city, CXRx - carrier, FBCx - fare basis code,\n"
    "             and : is used to separate pricing units\n";

const char*
puTypeStr(const PU& pu)
{
  switch (pu.puType())
  {
  case PricingUnit::Type::OPENJAW:
    return "PU TYPE: 101/OJ";
  case PricingUnit::Type::ROUNDTRIP:
    return "PU TYPE: 102/RT";
  case PricingUnit::Type::CIRCLETRIP:
    return "PU TYPE: 103/CT";
  case PricingUnit::Type::ONEWAY:
    return "PU TYPE: 104/OW";
  default:
    return "PU TYPE: UNKNOWN";
  }
}

const char*
puSubTypeStr(const PU& pu)
{
  switch (pu.puSubType())
  {
  case PricingUnit::DEST_OPENJAW:
    return "(TOJ)";
  case PricingUnit::ORIG_OPENJAW:
    return "(OOJ)";
  case PricingUnit::DOUBLE_OPENJAW:
    return "(DOW)";
  default:
    return "";
  }
}

char
getDirection(Directionality dir)
{
  switch (dir)
  {
  case FROM:
    return 'O';
  case TO:
    return 'I';
  default:
    return ' ';
  }
}
}

DiagSoloPQCollector::DiagSoloPQCollector(ShoppingTrx& trx, DiagCollector* diag942)
  : _dc(nullptr), _dc942(diag942), _filter(), _idMap(), _expansionsCount(0)
{
  if (Diagnostic929 == trx.diagnostic().diagnosticType())
  {
    _dc = DCFactory::instance()->create(trx);
    _dc->activate();

    _filter.initialize(_dc->rootDiag());

    printInitialHeader();
    printLegend();
    printExcludedSolutionPatterns();
  }
}

DiagSoloPQCollector::~DiagSoloPQCollector()
{
  if (_dc)
  {
    _dc->flushMsg();
    _dc = nullptr;
  }
}

void
DiagSoloPQCollector::printInitialHeader()
{
  TSE_ASSERT(_dc);
  *_dc << STAR_LINE << " DIAG 929: SOL PQ EXPANSION\n" << STAR_LINE;
}

void
DiagSoloPQCollector::printLegend()
{
  TSE_ASSERT(_dc);
  *_dc << LEGEND << DASH_LINE << "\n";
}

void
DiagSoloPQCollector::printExcludedSolutionPatterns()
{
  if (_dc)
  {
    const std::string& excludedSP = excludedSolutionPatterns.getValue();
    if (!excludedSP.empty())
    {
      *_dc << "Excluded solution patterns: " << excludedSP << "\n\n";
    }
  }
}

DiagSoloPQCollector::PtrToId::PtrToId() : map_(), lastIdx_(1u) // start from 1
{
}

size_t
DiagSoloPQCollector::PtrToId::getId(uint32_t itemId)
{
  std::pair<std::map<uint32_t, size_t>::iterator, bool> insertStatus =
      map_.insert(std::make_pair(itemId, lastIdx_));
  if (insertStatus.second)
    ++lastIdx_;
  return insertStatus.first->second;
}

namespace
{
struct ItemPrinter
{
public:
  ItemPrinter(const SoloPQItem::StrVerbosityLevel verbosity,
              const SoloPQItem& item,
              const size_t& itemId)
    : _item(item), _itemId(itemId), _verbosity(verbosity)
  {
  }

  friend DiagCollector& operator<<(DiagCollector& dc, const ItemPrinter& prn)
  {
    dc << "[" << std::setw(5) << std::setfill('0') << prn._itemId << "] ";
    dc << prn._item.str(prn._verbosity);
    return dc;
  }

private:
  const SoloPQItem& _item;
  const size_t& _itemId;
  const SoloPQItem::StrVerbosityLevel _verbosity;
};
}

void
DiagSoloPQCollector::onPQEnqueueImpl(const SoloPQItemPtr& item,
                                     const SoloPQItem* const expandedFrom)
{
  const size_t itemId(getItemId(item));
  const size_t parentItemId(expandedFrom ? getItemId(expandedFrom) : 0);

  _filter.addIdRelationShip(itemId, parentItemId);

  if (_filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "PUSH: " << ItemPrinter(_filter.getVerbosityLevel(), *item, itemId);
  if (expandedFrom)
  {
    *_dc << " (from " << ItemPrinter(SoloPQItem::SVL_HEADINGONLY, *expandedFrom, parentItemId)
         << ")";
  }
  *_dc << "\n";
}

void
DiagSoloPQCollector::onPQDequeueImpl(const SoloPQItemPtr& item)
{
  ++_expansionsCount;
  const size_t itemId(getItemId(item));

  if (_filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "\nPOP:  " << ItemPrinter(_filter.getVerbosityLevel(), *item, itemId) << "\n";
}

void
DiagSoloPQCollector::onPQPeekImpl(const SoloPQItemPtr& item)
{
  const size_t itemId(getItemId(item));

  if (_filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "PEEK: " << ItemPrinter(_filter.getVerbosityLevel(), *item, itemId) << "\n";
}

void
DiagSoloPQCollector::onSkipLocalPattern(const SoloPQItemPtr& item)
{
  if (!_dc)
    return;

  const size_t itemId(getItemId(item));
  if (_filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "\nSKIP SOL FM: " << ItemPrinter(_filter.getVerbosityLevel(), *item, itemId) << "\n";
}

void
DiagSoloPQCollector::onHurryOutCondition()
{
  if (_dc)
    *_dc << "HURRY OUT CONDITION MET\n";
  else if (_dc942)
    dynamic_cast<Diag942Collector&>(*_dc942).printPQIsEmpty("HURRY OUT CONDITION MET");
}

void
DiagSoloPQCollector::onSOLHurryOutCondition()
{
  if (_dc)
    *_dc << "SOL HURRY OUT CONDITION MET\n";
}

void
DiagSoloPQCollector::onOBSchedulesCondition()
{
  if (_dc)
    *_dc << "OUTBOUND SCHEDULES CONDITION MET\n";
}

void
DiagSoloPQCollector::onPQEmpty()
{
  if (_dc)
    *_dc << "PQ EMPTY\n";
  else if (_dc942)
    dynamic_cast<Diag942Collector&>(*_dc942).printPQIsEmpty("PQ EMPTY");
}

void
DiagSoloPQCollector::onNotUsedFPCondtion(size_t notUsedFPs)
{
  if (_dc)
    *_dc << "EXCEEDED THE MAXIMUM NUMBER OF UNUSED FAREPATHS: " << notUsedFPs << "\n";
}

void
DiagSoloPQCollector::onFailedFPCondition(size_t failedFPs)
{
  if (_dc)
    *_dc << "EXCEEDED THE MAXIMUM NUMBER OF FAILED FAREPATHS: " << failedFPs << "\n";
}

void
DiagSoloPQCollector::displayPUPathImpl(const SoloPUPathCollector::PUStruct& puStruct,
                                       const SoloPQItem* const parentItem)
{
  if (parentItem && _filter.isFMPFilteredOut(parentItem, getItemId(parentItem)))
    return;

  *_dc << std::setfill(' ');
  printFMPath(puStruct._fmPath, puStruct._itin);
  printPUPath(*(parentItem->getSolPattern()), puStruct._puPath, puStruct._itin);
}

void
DiagSoloPQCollector::displayValidationMsgImpl(const std::string& msg,
                                              const SoloPQItem* const parentItem)
{
  if (_filter.isItemFilteredOut(parentItem, getItemId(parentItem)))
    return;

  *_dc << msg << "\n";
}

void
DiagSoloPQCollector::printFMPath(const FareMarketPath* const fmPath, const Itin* const itin)
{
  *_dc << "******* ITINERARY ********\n";
  if (itin)
  {
    for (TravelSeg* tvlS : itin->travelSeg())
    {
      std::string depDate(tvlS->departureDT().dateToString(DDMMM, ""));

      ArunkSeg* arunkS = dynamic_cast<ArunkSeg*>(tvlS);
      if (arunkS)
      {
        depDate = " ";
      }

      AirSeg* airS = dynamic_cast<AirSeg*>(tvlS);
      CarrierCode carrier = (airS) ? airS->carrier() : " ";
      *_dc << std::setw(4) << carrier << std::setw(6) << depDate << std::setw(4)
           << tvlS->boardMultiCity() << std::setw(4) << tvlS->offMultiCity() << std::setw(2)
           << tvlS->getBookingCode() << std::endl;
    }
  }
  else
    *_dc << "No Itinerary\n";

  *_dc << "****************** FARE MARKET PATH ******************\n";
  if (fmPath)
  {
    for (MergedFareMarket* fm : fmPath->fareMarketPath())
    {
      if (itin)
      {
        *_dc << itin->segmentOrder(fm->travelSeg().front()) << "--"
             << itin->segmentOrder(fm->travelSeg().back()) << ":";
      }

      *_dc << fm->boardMultiCity() << " " << fm->offMultiCity() << " ";
    }
    *_dc << '\n';
  }
  else
    *_dc << "No valid fare market path\n";
}

void
DiagSoloPQCollector::printPUPath(const SolutionPattern& solutionPattern,
                                 const PUPath* const puPath,
                                 const Itin* const itin)
{
  *_dc << "********************** PU PATH ***********************\n";
  *_dc << solutionPattern.getSPIdStr() << ": " << solutionPattern.getPUPathStr() << std::endl;
  if (puPath)
  {
    for (const PU* const pu : puPath->puPath())
    {
      *_dc << puTypeStr(*pu) << " " << puSubTypeStr(*pu) << std::endl;

      std::vector<MergedFareMarket*>::const_iterator fmIter = pu->fareMarket().begin(),
                                                     fmEndIter = pu->fareMarket().end();
      std::vector<Directionality>::const_iterator dirIt = pu->fareDirectionality().begin(),
                                                  dirEndIt = pu->fareDirectionality().end();

      for (; fmIter != fmEndIter && dirIt != dirEndIt; ++fmIter, ++dirIt)
      {
        const MergedFareMarket* const fm = *fmIter;
        *_dc << " " << fm->boardMultiCity() << " " << fm->offMultiCity() << " : ";

        if (itin)
        {
          for (const TravelSeg* const tvlSeg : fm->travelSeg())
            *_dc << itin->segmentOrder(tvlSeg) << " ";
        }
        *_dc << " DIR: " << getDirection(*dirIt) << std::endl;
      }
    }
  }
  else
    *_dc << "No valid PU Path\n";
}

void
DiagSoloPQCollector::informFarePathInvalidImpl(const SoloPQItem* const item, const char* msg)
{
  const size_t itemId(item ? getItemId(item) : 0);
  if (!item || _filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "Fare path failed for: "

       // SVL_HEAINGONLY as anything other probably doesn't make any sense.
       // Originaly displayed fare path has already been overriden
       // by FarePathFactory::getNextFPPQItem() called by
       // FarePathFactoryPQItem::expand()
       << ItemPrinter(SoloPQItem::SVL_HEADINGONLY, *item, itemId) << (msg ? " " : "")
       << (msg ? msg : "") << ".\n";
}

void
DiagSoloPQCollector::displayNoOfExpansionsImpl(const size_t notYetExpandedPqSize)
{
  *_dc << "\n*** PQ processing stopped after " << _expansionsCount << " expansions.";
  if (notYetExpandedPqSize)
    *_dc << " " << notYetExpandedPqSize << " items still in PQ.";
  *_dc << " ***\n";
}

void
DiagSoloPQCollector::onCrcToFpfExpandFail(const SoloPQItemPtr& item, const char* msg)
{
  if (!_dc)
    return;
  const size_t itemId(item ? getItemId(item) : 0);
  if (!item || _filter.isItemFilteredOut(item, itemId))
    return;
  *_dc << "FPF was not created: " << msg << "\n";
}

void
DiagSoloPQCollector::onSkipCRExpansion(const SoloPQItemPtr& item,
                                       const SoloPQItem* const expandedFrom)
{
  if (!_dc)
    return;
  const size_t itemId(getItemId(item));
  const size_t parentItemId(expandedFrom ? getItemId(expandedFrom) : 0);

  _filter.addIdRelationShip(itemId, parentItemId);

  if (_filter.isItemFilteredOut(item, itemId))
    return;

  *_dc << "TAG-1/2/3 SKIP CR ITEM: " << ItemPrinter(_filter.getVerbosityLevel(), *item, itemId);
  if (expandedFrom)
    *_dc << " (from " << ItemPrinter(SoloPQItem::SVL_HEADINGONLY, *expandedFrom, parentItemId)
         << ")";
  *_dc << "\n";
}
} /* namespace shpq */
} /* namespace tse */
