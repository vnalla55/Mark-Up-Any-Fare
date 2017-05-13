//----------------------------------------------------------------------------
//  File:        Diag688Collector.C
//  Authors:     Grzegorz Cholewiak
//  Created:     Apr 13 2007
//
//  Description: Diagnostic 688 formatter
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2007
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag688Collector.h"

#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ExcItin.h"
#include "DataModel/ProcessTagInfo.h"
#include "DataModel/ProcessTagPermutation.h"
#include "DataModel/RefundPermutation.h"
#include "DataModel/RefundProcessInfo.h"
#include "DBAccess/VoluntaryRefundsInfo.h"
#include "RexPricing/TagWarEngine.h"

#include <boost/bind.hpp>
#include <boost/functional/value_factory.hpp>

#include <algorithm>
#include <set>

namespace tse
{
Diag688Collector&
Diag688Collector::operator<<(const ProcessTagInfo& pti)
{
  DiagCollector& dc(*this);
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << std::setw(7) << pti.fareMarket()->boardMultiCity() + " " + pti.fareMarket()->offMultiCity()
     << " ";
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(6) << pti.record3()->itemNo() << " ";

  if (pti.reissueSequence()->orig())
  {
    dc << std::setw(6) << pti.reissueSequence()->itemNo() << " " << std::setw(6)
       << pti.reissueSequence()->seqNo() << " " << std::setw(3)
       << pti.reissueSequence()->orig()->processingInd() << " " << std::setw(4)
       << pti.reissueSequence()->orig()->stopInd() << " " << std::setw(9)
       << pti.reissueSequence()->orig()->expndKeep() << "\n";
  }
  else
    dc << "NO T988\n";

  if (_withOverridenData && pti.isOverriden())
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(7) << "OVERIDN"
       << " ";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc << std::setw(6) << pti.record3()->overriding()->itemNo() << " " << std::setw(6)
       << pti.reissueSequence()->overriding()->itemNo() << " " << std::setw(6)
       << pti.reissueSequence()->overriding()->seqNo() << " " << std::setw(3)
       << pti.reissueSequence()->overriding()->processingInd() << " " << std::setw(4)
       << pti.reissueSequence()->overriding()->stopInd() << " " << std::setw(9)
       << pti.reissueSequence()->overriding()->expndKeep() << "\n";
  }

  return *this;
}

Diag688Collector&
Diag688Collector::operator<<(
    std::pair<CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType*, bool> permInfo)
{
  if (permInfo.first && isVisible())
  {
    _withOverridenData = permInfo.second;
    *this << *permInfo.first;
  }
  return *this;
}

Diag688Collector&
Diag688Collector::operator<<(
    CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType& permSelector)
{
  DiagCollector& dc(*this);

  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << "\nPERMUTATION " << _permNumber;
  if ((static_cast<PricingTrx*>(trx()))->getTrxType() == PricingTrx::MIP_TRX &&
      (static_cast<PricingTrx*>(trx()))->excTrxType() == PricingTrx::AR_EXC_TRX)
    dc << " FOR NEW ITIN " << (static_cast<BaseExchangeTrx*>(trx()))->itinIndex();
  dc << std::endl << std::setw(7) << "FM"
     << " ";
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(6) << "REC3"
     << " " << std::setw(6) << "ITEM"
     << " " << std::setw(6) << "SEQ"
     << " "
     << "TAG STOP EXPNDKEEP\n";

  CartesianProduct<std::vector<ProcessTagInfo*> >::ProductType::iterator b = permSelector.begin(),
                                                                         i(b),
                                                                         e = permSelector.end();
  for (; i != e; i++)
    *this << **i;

  return *this;
}

void
Diag688Collector::printKeepFareMapping()
{
  RexPricingTrx& trx = static_cast<RexPricingTrx&>(*_trx);

  DiagCollector::operator<<("\n");

  SortKeepMap skm;
  std::copy(trx.newItinKeepFares().begin(),
            trx.newItinKeepFares().end(),
            std::inserter(skm, skm.begin()));

  printKeepFareMap(skm, "KEEP");

  SortKeepMap skme;
  std::transform(trx.expndKeepMap().begin(),
                 trx.expndKeepMap().end(),
                 std::inserter(skme, skme.begin()),
                 boost::bind(boost::value_factory<std::pair<const PaxTypeFare*, const FareMarket*> >(),
                             boost::bind(&RexPricingTrx::ExpndKeepMap::value_type::second, _1),
                             boost::bind(&RexPricingTrx::ExpndKeepMap::value_type::first, _1)));

  printKeepFareMap(skme, "EXPN");
}

void
Diag688Collector::printKeepFareMap(const SortKeepMap& map, const std::string& title)
{
  for (const Diag688Collector::SortKeepMap::value_type& pair : map)
  {
    DiagCollector::operator<<("\n" + title + " FARE MAP - EXC: ");
    DiagCollector::operator<<(*pair.first->fareMarket());
    DiagCollector::operator<<(" NEW: ");
    DiagCollector::operator<<(*pair.second);
  }
}

// Printout of War of Tags
Diag688Collector& Diag688Collector::operator << (const ProcessTagPermutation* p)
{
  if (!p || !isVisible())
    return *this;

  const ProcessTagPermutation& perm = *p;

  RexPricingTrx& rexTrx = static_cast<RexPricingTrx&>(*trx());
  bool travelCommenced = !rexTrx.exchangeItin().front()->travelSeg().front()->unflown();
  DiagCollector& dc(*this);

  std::vector<ProcessTagInfo*>::const_iterator i = perm.processTags().begin();
  std::vector<ProcessTagInfo*>::const_iterator ie = perm.processTags().end();

  std::set<int> permItems;
  for (; i != ie; i++)
    if ((**i).reissueSequence()->orig())
      permItems.insert((*i)->processTag());

  std::set<int>::const_iterator bI;

  dc << " \nWAR OF TAGS RESULTS\n";

  // Printout header
  dc.setf(std::ios::right, std::ios::adjustfield);
  dc << std::setw(7) << "STATUS"
     << " ";
  for (bI = permItems.begin(); bI != permItems.end(); bI++)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << *bI << " ";
  }
  dc << "RESULT" << std::endl;
  //

  // Printout the rest
  std::set<FCChangeStatus>::const_iterator fcsI;
  for (fcsI = _fcStatuses.begin(); fcsI != _fcStatuses.end(); fcsI++)
  {
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(7) << " " + GetFCChangeStatusForTagWar(*fcsI) << " ";

    dc.setf(std::ios::right, std::ios::adjustfield);
    for (bI = permItems.begin(); bI != permItems.end(); bI++)
    {
      FareApplication fa = TagWarEngine::getMatrixValue(*bI, *fcsI, travelCommenced);
      dc << GetFareApplicationForTagWar(fa) << " ";
    }

    dc << std::setw(6) << GetFareApplicationForTagWar(perm.fareTypeSelection(*fcsI)) << std::endl;
  }
  if (perm.electronicTicket() == ProcessTagPermutation::ELECTRONIC_TICKET_MIXED)
    dc << "ELECTRONIC TICKET FAILED" << std::endl;
  //

  dc << std::endl;

  return *this;
}

std::string
Diag688Collector::GetFareApplicationForTagWar(FareApplication fa)
{

  switch (fa)
  {
  case TRAVEL_COMMENCEMENT:
    return "T";
  case HISTORICAL:
    return "H";
  case KEEP:
    return "K";
  case CURRENT:
    return "C";
  case CANCEL:
    return "X";
  default:
    return "-";
  }
}

std::string
Diag688Collector::GetFCChangeStatusForTagWar(FCChangeStatus cs)
{
  switch (cs)
  {
  case UU:
    return "UU";
  case UN:
    return "UN";
  case FL:
    return "FL";
  case UC:
    return "UC";
  default:
    return "-";
  }
}

Diag688Collector & Diag688Collector::operator<< (const std::string &msg)
{
  if (isVisible())
    ((std::ostringstream&)*this) << msg;
  return *this;
}

void
Diag688Collector::printPermStatus()
{
  if (isVisible())
  {
    DiagCollector& dc(*this);
    dc << " \nPERMUTATION " << (_permStatus ? "PASSED\n" : "FAILED\n");
    printLine();
  }
}

void
Diag688Collector::printSummary(RexPricingTrx& trx, int generatedPermCount)
{
  DiagCollector& dc(*this);
  dc << " \n";
  printLine();

  std::pair<size_t, size_t> permCount = trx.permutationCount();

  if (permCount.second)
    dc << permCount.second << " VALID TAG 1 STOP Y PERMUTATIONS\n";

  dc << permCount.first << " VALID TAG PERMUTATIONS\n" << generatedPermCount
     << " GENERATED TAG PERMUTATIONS\n";

  if (!trx.newItinROEConversionDate().isEmptyDate())
  {
    if (trx.curNewItin()->exchangeReissue() == EXCHANGE)
      dc << "EXCHANGE\n";
    else
      dc << "REISSUE\n";
    dc << "ROE RETRIEVAL DATE : " << trx.newItinROEConversionDate().toIsoExtendedString() << "\n";
  }
  printLine();
}

void
Diag688Collector::printRefundProcessInfo(const RefundProcessInfo& info)
{
  DiagCollector& dc(*this);
  dc.setf(std::ios::left);
  dc << std::setw(4) << info.fareMarket().boardMultiCity() << std::setw(4)
     << info.fareMarket().offMultiCity() << std::setw(7) << info.record3().itemNo()
     << info.record3().repriceInd() << '\n';
}

void
Diag688Collector::printRefundPermutation(const RefundPermutation& perm)
{
  DiagCollector& dc(*this);
  dc.setf(std::ios::left);
  dc << "PERMUTATION " << perm.number() << '\n';
  dc << std::setw(8) << "FM" << std::setw(7) << "REC3"
     << "RI\n";
  for (const auto elem : perm.processInfos())
    printRefundProcessInfo(*elem);
  dc << "     RE-PRICE: " << perm.repriceIndicator() << '\n';
}

void
Diag688Collector::printRefundPermutations(const std::vector<RefundPermutation*>& perm)
{
  DiagCollector& dc(*this);
  dc.setf(std::ios::left);
  dc << "CATEGORY 33 VOL REFUND - DIAG 688\n";
  printLine();
  dc << perm.size() << " VALID PERMUTATIONS\n";
  printLine();
  for (const auto elem : perm)
  {
    printRefundPermutation(*elem);
    printLine();
  }
}
}
