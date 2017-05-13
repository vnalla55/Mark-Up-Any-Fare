// vim:ts=2:sts=2:sw=2:cin:et
// -------------------------------------------------------------------
//
//! \author       Robert Luberda
//! \date         28-09-2011
//! \file         DiagSoloPQCollector.h
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

#pragma once

#include "Pricing/Shopping/PQ/DiagSoloPQFilter.h"
#include "Pricing/Shopping/PQ/SoloPQItem.h"
#include "Pricing/Shopping/PQ/SoloPUPathCollector.h"

namespace tse
{
class FareMarketPath;
class DiagCollector;
class Itin;
class PUPath;
class ShoppingTrx;
}

namespace tse
{
namespace shpq
{

class DiagSoloPQCollector
{
public:
  explicit DiagSoloPQCollector(ShoppingTrx& trx, DiagCollector* diag942);
  ~DiagSoloPQCollector();

  void onPQEnqueue(const SoloPQItemPtr& item, const SoloPQItem* const expandedFrom)
  {
    if (LIKELY(!_dc))
      return;
    onPQEnqueueImpl(item, expandedFrom);
  }

  void onPQDequeue(const SoloPQItemPtr& item)
  {
    if (LIKELY(!_dc))
      return;
    onPQDequeueImpl(item);
  }

  void onPQPeek(const SoloPQItemPtr& item)
  {
    if (!_dc)
      return;
    onPQPeekImpl(item);
  }

  void onSkipLocalPattern(const SoloPQItemPtr& item);
  void onSOLHurryOutCondition();
  void onHurryOutCondition();
  void onOBSchedulesCondition();
  void onPQEmpty();
  void onNotUsedFPCondtion(size_t notUsedFPs);
  void onFailedFPCondition(size_t failedFPs);

  void onCrcToFpfExpandFail(const SoloPQItemPtr& item, const char* msg);

  void
  displayPUPath(const SoloPUPathCollector::PUStruct& puStruct, const SoloPQItem* const parentItem)
  {
    if (LIKELY(!_dc))
      return;
    displayPUPathImpl(puStruct, parentItem);
  }

  void displayValidationMsg(const std::string& msg, const SoloPQItem* const parentItem)
  {
    if (!_dc)
      return;
    displayValidationMsgImpl(msg, parentItem);
  }

  void informFarePathInvalid(const SoloPQItem* const item, const char* msg = nullptr)
  {
    if (LIKELY(!_dc))
      return;
    informFarePathInvalidImpl(item, msg);
  }

  void displayNoOfExpansions(const size_t notYetExpandedPqSize)
  {
    if (!_dc)
      return;
    displayNoOfExpansionsImpl(notYetExpandedPqSize);
  }

  size_t getNoOfExpansions()
  {
    return _filter.getNoOfExpansions();
  } // TODO: Temporary code, remove it
  void onSkipCRExpansion(const SoloPQItemPtr& item, const SoloPQItem* const expandedFrom);

private:
  struct PtrToId
  {
  public:
    PtrToId();
    size_t getId(uint32_t itemId);

  private:
    std::map<uint32_t, size_t> map_;
    size_t lastIdx_;
  };

  DiagCollector* _dc;
  DiagCollector* _dc942;
  DiagSoloPQFilter _filter;
  PtrToId _idMap;
  size_t _expansionsCount; // estimated, assumption: PQDEqueue is called for expansion

  void onPQEnqueueImpl(const SoloPQItemPtr& item, const SoloPQItem* const expandedFrom);
  void onPQDequeueImpl(const SoloPQItemPtr& item);
  void onPQPeekImpl(const SoloPQItemPtr& item);

  size_t getItemId(const SoloPQItemPtr& item) { return _idMap.getId(item->getId()); }
  size_t getItemId(const SoloPQItem* const item) { return _idMap.getId(item->getId()); }

  void displayPUPathImpl(const SoloPUPathCollector::PUStruct&, const SoloPQItem* const);
  void displayValidationMsgImpl(const std::string&, const SoloPQItem* const);
  void printFMPath(const FareMarketPath* const fareMarketPath, const Itin* const itin);
  void printPUPath(const SolutionPattern&, const PUPath* const, const Itin* const);

  void informFarePathInvalidImpl(const SoloPQItem* const item, const char* msg = nullptr);

  void displayNoOfExpansionsImpl(const size_t notYetExpandedPqSize);

  void printInitialHeader();
  void printLegend();
  void printExcludedSolutionPatterns();
};

} /* namespace shpq */
} /* namespace tse */
