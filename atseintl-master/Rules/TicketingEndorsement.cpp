#include "Rules/TicketingEndorsement.h"

#include "Common/FareCalcUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AirSeg.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/Customer.h"
#include "DBAccess/FareCalcConfig.h"
#include "DBAccess/TicketEndorsementsInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag858Collector.h"
#include "Util/BranchPrediction.h"

#include <algorithm>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>
#include <boost/iterator/filter_iterator.hpp>
#include <boost/tokenizer.hpp>

namespace tse
{
FALLBACK_DECL(endorsementExpansion)
//ToDo: while removing fallback remove
//EndorseCutterLimited
//trimEndorsementMsg
//maxEndorsementMsgLen
//MAX_TKT_ENDORSEMENT_LINE_SIZE
//MAX_TKT_ENDORSEMENT_LINE_SIZE_FOR_ABACUS
FALLBACK_DECL(fallbackProcessEmptyTktEndorsements)
// TODO: when removing this fallback, remove
// - class LowerPriorityNumAndCarrierOld
// - class LowerPnrSegmentOrder
FALLBACK_DECL(fallbackEndorsementsRefactoring)

const Indicator TicketingEndorsement::TKTLOCIND_FOP_1;
const Indicator TicketingEndorsement::TKTLOCIND_2;
const Indicator TicketingEndorsement::TKTLOCIND_FOP_3;
const Indicator TicketingEndorsement::TKTLOCIND_4;
const Indicator TicketingEndorsement::TKTLOCIND_FOP_5;
const Indicator TicketingEndorsement::TKTLOCIND_6;

TicketEndorseItem::TicketEndorseItem(const TicketEndorsementsInfo& tei, const PaxTypeFare& fare)
  : priorityCode(tei.priorityCode()),
    carrier(fare.carrier()),
    endorsementTxt(tei.endorsementTxt()),
    tktLocInd(tei.tktLocInd()),
    paxTypeFare(&fare),
    itemNo(tei.itemNo())
{
}

// ### remove together with fallbackEndorsementsRefactoring ########################################
void
TicketEndorseLine::addSegmentOrder(const std::vector<TravelSeg*>& travelSegs)
{
  for (const TravelSeg* ts: travelSegs)
    segmentOrders.insert(ts->pnrSegment());
}
//##################################################################################################

void
EndorseCutter::operator()(const std::vector<TicketEndorseItem>& items,
                          TicketEndorseLine* target) const
{
  if (UNLIKELY(items.empty()))
    return;

  target->carrier = items.front().carrier;

  target->priorityCode = std::min_element(items.begin(), items.end(),
    [](const TicketEndorseItem& item1, const TicketEndorseItem& item2)
    { return item1.priorityCode < item2.priorityCode; } )->priorityCode;

  // In case you're wondering. The original code did not process Endos items if the first one was
  // blank. Yeah, go figure!
  if (UNLIKELY(items.front().endorsementTxt.empty()))
    return;

  for (const TicketEndorseItem& item : items)
  {
    if (UNLIKELY(item.endorsementTxt.empty()))
      continue;

    if (!target->endorseMessage.empty())
    {
      const char ind = target->endorseMessage.back();
      if (ind != '/')
      {
        if (ind != '+')
          target->endorseMessage += '/';
        else
          target->endorseMessage.resize(target->endorseMessage.size() - 1);
      }
    }
    target->endorseMessage += item.endorsementTxt;
  }
}

// ### remove together with fallbackEndorsementsRefactoring ########################################
void
EndorseCutter::operator()(EndorseItemConstItr, EndorseItemConstItr, TicketEndorseLine*) const
{
  /* DUMMY */
}

void
EndorseCutterUnlimited::operator()(EndorseItemConstItr begin, EndorseItemConstItr end,
                                   TicketEndorseLine* target) const
{
  TSE_ASSERT(target);

  if (end == begin)
    return;

  uint32_t minPrio = begin->priorityCode;
  target->endorseMessage = begin->endorsementTxt;
  // Insane but consistent with old solution
  target->carrier = begin->carrier;
  if (begin->paxTypeFare)
    target->addSegmentOrder(begin->paxTypeFare->fareMarket()->travelSeg());

  EndorseItemConstItr endoItem = begin + 1;
  for (; endoItem != end; ++endoItem)
  {
    if (endoItem->priorityCode < minPrio)
      minPrio = endoItem->priorityCode;

    TicketingEndorsement::glueEndorsementMessageTktEnd(target->endorseMessage,
                                                       endoItem->endorsementTxt);
  }
  target->priorityCode = minPrio;
}

void
EndorseCutterLimited::operator()(EndorseItemConstItr begin, EndorseItemConstItr end,
                                 TicketEndorseLine* target) const
{
  TSE_ASSERT(target);

  if (UNLIKELY(end == begin))
    return;

  uint32_t minPrio = begin->priorityCode;
  target->endorseMessage = begin->endorsementTxt;
  // Insane but consistent with old solution
  target->carrier = begin->carrier;
  if (LIKELY(begin->paxTypeFare))
    target->addSegmentOrder(begin->paxTypeFare->fareMarket()->travelSeg());

  EndorseItemConstItr endoItem = begin + 1;
  for (; endoItem != end; ++endoItem)
  {
    size_t oldSize = target->endorseMessage.length();
    const char oldLastChar = *(target->endorseMessage.rbegin());
    TicketingEndorsement::glueEndorsementMessageTktEnd(target->endorseMessage,
                                                       endoItem->endorsementTxt);

    if (target->endorseMessage.length() > _maxLen)
    {
      target->endorseMessage.resize(_maxLen);
      char currentLastChar = *(target->endorseMessage.rbegin());

      const bool needToProcessPlus = (_maxLen - oldSize == 0 && oldLastChar == '+');
      const bool needToProcessNotSlash = (_maxLen - oldSize > 1 && currentLastChar != '/');

      if (endoItem->priorityCode < minPrio && (needToProcessNotSlash || needToProcessPlus))
        minPrio = endoItem->priorityCode;

      break;
    }

    if (endoItem->priorityCode < minPrio)
      minPrio = endoItem->priorityCode;
  }

  target->priorityCode = minPrio;
}
//##################################################################################################

Record3ReturnTypes
TicketingEndorsement::process(PricingTrx& trx, FareUsage& fareUsage, const PaxTypeFare& fare)
{
  if (UNLIKELY(!_teInfo))
    return FAIL;

  //----------------------------------------------------------------------
  // Check the Unavailable Data Tag
  //----------------------------------------------------------------------
  if (UNLIKELY(_teInfo->unavailTag() == 'X')) // Incomplete data?
    return FAIL; // Yes, fail this fare

  if (_teInfo->unavailTag() == 'Y') // Text data only?
    return SKIP; // Yes, skip this category

  //-----------------------------------------------------------------------------------
  // Do not process endorsement for Reissued ticket only at the original ticket process
  // Abacus user needs to process the value '3' - FOP box for Reissued ticket only
  //-----------------------------------------------------------------------------------
  if ((_teInfo->tktLocInd() == TKTLOCIND_4) ||
      ((_teInfo->tktLocInd() == TKTLOCIND_FOP_3) &&
       (!trx.getRequest()->ticketingAgent()->abacusUser())))
  {
    if (!trx.getRequest()->ticketingAgent()->infiniUser())
      return PASS;
  }
  insertIfOrReplaceIf(fareUsage.tktEndorsement(), fare);

  return PASS;
}

bool
TicketingEndorsement::lowerPriorityNumTktEnd(const TicketEndorseItem& item1,
                                             const TicketEndorseItem& item2)
{
  return item1.priorityCode < item2.priorityCode;
}

std::vector<TicketEndorseItem>::const_iterator
TicketingEndorsement::eraseNotWantedMsgsTktEnd(std::vector<TicketEndorseItem>& endorsements,
                                               bool isDirectFareFiling)
{
  std::vector<TicketEndorseItem>::iterator newEnd = remove_if(
      endorsements.begin(),
      endorsements.end(),
      boost::bind(&TicketEndorseItem::tktLocInd, _1) == TicketingEndorsement::TKTLOCIND_FOP_1 ||
          boost::bind(&TicketEndorseItem::tktLocInd, _1) == TicketingEndorsement::TKTLOCIND_FOP_3 ||
          boost::bind(&TicketEndorseItem::tktLocInd, _1) == TicketingEndorsement::TKTLOCIND_FOP_5);
  if (!isDirectFareFiling)
    stable_sort(endorsements.begin(), newEnd, lowerPriorityNumTktEnd);

  return newEnd;
}

void
TicketingEndorsement::eraseNotWantedMsgsTktEnd(std::vector<TicketEndorseItem>& endorsements,
                                               std::vector<TicketEndorseItem>& endoOut,
                                               bool isDirectFareFiling)
{
  for (TicketEndorseItem& item : endorsements)
  {
    if (shouldBeRemoved(item))
      continue;

    endoOut.push_back(item);
  }
  if (!isDirectFareFiling)
    std::stable_sort(endoOut.begin(), endoOut.end(), lowerPriorityNumTktEnd);

  return;
}

// ### remove together with fallbackEndorsementsRefactoring ########################################
void
TicketingEndorsement::glueEndorsementMessageTktEnd(std::string& target, const std::string& source)
{
  if (UNLIKELY(target.size() == 0))
    return;

  if (target[target.size() - 1] != '/')
  {
    if (LIKELY(target[target.size() - 1] != '+'))
    {
      target += "/";
    }
    else
    {
      target.erase(target.size() - 1, 1);
    }
  }
  target += source;
}
//##################################################################################################

void
TicketingEndorsement::insertIfOrReplaceIf(std::vector<TicketEndorseItem>& endorsements,
                                          const PaxTypeFare& fare) const
{
  std::vector<TicketEndorseItem>::iterator i = endorsements.begin();
  for (; i != endorsements.end(); ++i)
  {
    if (_teInfo->endorsementTxt() == i->endorsementTxt)
    {
      if (LIKELY((_teInfo->tktLocInd() == i->tktLocInd) ||
          (_teInfo->tktLocInd() % 2 > 0 && i->tktLocInd % 2 > 0) ||
          (_teInfo->tktLocInd() % 2 == 0 && i->tktLocInd % 2 == 0)))
      {
        return;
      }
    }
  }

  endorsements.push_back(TicketEndorseItem(*_teInfo, fare));
}

// ### remove together with fallbackEndorsementsRefactoring ########################################
std::string
TicketingEndorsement::trimEndorsementMsg(const PricingTrx& trx, const std::string& endorsementMsg)
{
  if (!fallback::endorsementExpansion(&trx))
    return endorsementMsg;

  return (trx.getRequest()->ticketingAgent()->abacusUser() ||
          trx.getRequest()->ticketingAgent()->infiniUser())
             ? endorsementMsg.substr(0, MAX_TKT_ENDORSEMENT_LINE_SIZE_FOR_ABACUS)
             : endorsementMsg.substr(0, MAX_TKT_ENDORSEMENT_LINE_SIZE);
}

uint16_t
TicketingEndorsement::maxEndorsementMsgLen(const PricingTrx& trx)
{
  return (trx.getRequest()->ticketingAgent()->abacusUser() ||
          trx.getRequest()->ticketingAgent()->infiniUser())
             ? MAX_TKT_ENDORSEMENT_LINE_SIZE_FOR_ABACUS
             : MAX_TKT_ENDORSEMENT_LINE_SIZE;
}
//##################################################################################################

bool
TicketingEndorsement::isEndorseUser(const PricingTrx& trx) // need to add condition for hosted
                                                           // carrier
{
  if (trx.getRequest() && trx.getRequest()->ticketingAgent())
  {
    if ((trx.getRequest()->ticketingAgent()->agentTJR() &&
         (trx.getRequest()->ticketingAgent()->agentTJR()->crsCarrier() == SABRE_MULTIHOST_ID ||
          trx.getRequest()->ticketingAgent()->agentTJR()->hostName() == SABRE_USER)) ||
        ((trx.getRequest()->ticketingAgent()->agentTJR() == nullptr) && trx.billing() &&
         (!trx.billing()->partitionID().empty()) && (trx.billing()->aaaCity().size() < 4)))
    {
      return true; // Sabre and hosted carrier
    }
    else if (trx.getRequest()->ticketingAgent()->abacusUser())
    {
      return true; // Abacus
    }
    else if (trx.getRequest()->ticketingAgent()->infiniUser())
    {
      return true; // Infini
    }
    else if (trx.getRequest()->ticketingAgent()->axessUser())
    {
      return true; // Axess
    }
  }
  return false;
}

TicketEndorseLine*
TicketingEndorsement::sortAndGlue(const PricingTrx& trx,
                                  const Itin& itin,
                                  FareUsage& fareUsage,
                                  const EndorseCutter& endoCat,
                                  Diag858Collector* dc)
{
  typedef std::vector<TicketEndorseItem> EndoItems;
  TicketEndorseLine* endoLine = nullptr;

  if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
  {
    if (fareUsage.tktEndorsement().size() == 0)
      return nullptr;

    endoLine = trx.dataHandle().create<TicketEndorseLine>();
  }
  else
  {
    endoLine = trx.dataHandle().create<TicketEndorseLine>();

    if (fareUsage.tktEndorsement().size() == 0)
      return endoLine;
  }

  EndoItems& tktEndorsement = fareUsage.tktEndorsement();

  bool dropEndorsementHierarchy = false;

  const bool isDirectFareFiling =
      trx.dataHandle().getVendorType(fareUsage.paxTypeFare()->vendor()) == RuleConst::SMF_VENDOR;

  dropEndorsementHierarchy = isDirectFareFiling && isEndorseUser(trx);

  if (trx.getRequest() && trx.getRequest()->ticketingAgent())
  {
    if (TrxUtil::isAbacusEndorsementHierarchyAtpcoFaresActive(trx) &&
        trx.getRequest()->ticketingAgent()->abacusUser())
      dropEndorsementHierarchy = true;

    if (TrxUtil::isInfiniEndorsementHierarchyAtpcoFaresActive(trx) &&
        trx.getRequest()->ticketingAgent()->infiniUser())
      dropEndorsementHierarchy = true;
  }


  EndoItems endoToDisplay;
  eraseNotWantedMsgsTktEnd(tktEndorsement, endoToDisplay, dropEndorsementHierarchy);

  size_t validMsgSize = endoToDisplay.size();

  if (UNLIKELY(dc))
  {
    EndoItems endosForDiag = tktEndorsement;

    if (!dropEndorsementHierarchy)
      std::stable_sort(endosForDiag.begin(), endosForDiag.end(), lowerPriorityNumTktEnd);

    dc->printEndorsement(fareUsage.paxTypeFare(), endosForDiag, validMsgSize);
  }

  if (!validMsgSize)
  {
    if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
    {
      return nullptr;
    }
    else
    {
      return endoLine;
    }
  }

  if (!fallback::fallbackEndorsementsRefactoring(&trx))
  {
    endoCat(endoToDisplay, endoLine);
    addSegmentOrders(endoLine, *endoToDisplay.begin(), itin);
  }
  else
  {
    endoCat(endoToDisplay.begin(), endoToDisplay.end(), endoLine);
  }

  if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
  {
    if (endoLine->endorseMessage.empty())
      return nullptr;
  }

  return endoLine;
}

namespace
{

// ### remove together with fallbackEndorsementsRefactoring ########################################
class LowerPriorityNumAndCarrierOld
{
public:
  LowerPriorityNumAndCarrierOld(const CarrierCode& carrier) : _carrier(carrier) {}

  // Logic taken from old code, don't try to understand, AFAIK nobody complains about endos order;)
  bool operator()(const TicketEndorseLine* item1, const TicketEndorseLine* item2)
  {
    TSE_ASSERT(item1 && item2);
    if (item1->priorityCode == item2->priorityCode)
    {
      const bool cxrsEqual = (item1->carrier == item2->carrier);
      const bool cxrMatchToItem =
          ((!_carrier.empty()) && (item1->carrier == _carrier || item2->carrier == _carrier));
      if (cxrsEqual)
        return false;

      if (cxrMatchToItem)
        return item1->carrier == _carrier;
      // What should we do if the two carriers are not the same but neither is _carrier???
    }

    return item1->priorityCode < item2->priorityCode;
  }

private:
  const CarrierCode& _carrier;
};

class LowerPnrSegmentOrder
{
public:
  bool operator()(const TicketEndorseLine* item1, const TicketEndorseLine* item2)
  {
    TSE_ASSERT(item1 && item2);
    if ((item1->segmentOrders.size() > 0) && (item2->segmentOrders.size() > 0))
      return *item1->segmentOrders.begin() < *item2->segmentOrders.begin();
    else
      return item1->segmentOrders.size() > 0;
  }
};
//##################################################################################################

class LowerPriorityNumAndCarrier
{
public:
  LowerPriorityNumAndCarrier(const CarrierCode& carrier) : _carrier(carrier) {}

  // Logic taken from old code, don't try to understand, AFAIK nobody complains about endos order;)
  // ^ That's what she said!
  bool operator()(const TicketEndorseLine* item1, const TicketEndorseLine* item2)
  {
    if (item1->priorityCode != item2->priorityCode)
      return item1->priorityCode < item2->priorityCode;

    if (item1->carrier == item2->carrier)
      return false;

    if (!_carrier.empty() && (item1->carrier == _carrier || item2->carrier == _carrier))
      return item1->carrier == _carrier;
    // What should we do if the two carriers are not the same but neither is _carrier???

    return false;
  }

private:
  const CarrierCode& _carrier;
};

bool
lowerPriorityNumAndSegmentOrder(const TicketEndorseLine* item1, const TicketEndorseLine* item2)
{
  if (item1->priorityCode == item2->priorityCode)
  {
    if (!item1->segmentOrders.empty() && !item2->segmentOrders.empty())
      return *item1->segmentOrders.begin() < *item2->segmentOrders.begin();
    else
      return false;
  }
  return item1->priorityCode < item2->priorityCode;
}

bool
lowerPnrSegmentOrder(const TicketEndorseLine* item1, const TicketEndorseLine* item2)
{
  if (!item1->segmentOrders.empty() && !item2->segmentOrders.empty())
    return *item1->segmentOrders.begin() < *item2->segmentOrders.begin();
  else
    return false;
}

}

void
TicketingEndorsement::collectEndorsements(const PricingTrx& trx,
                                          const FarePath& farePath,
                                          std::vector<TicketEndorseLine*>& message,
                                          const EndorseCutter& endoCat)
{
  if (UNLIKELY(trx.diagnostic().diagnosticType() == Diagnostic858))
  {
    _factory = DCFactory::instance();
    _diagPtr = static_cast<Diag858Collector*>(_factory->create(const_cast<PricingTrx&>(trx)));
    _diagPtr->enable(Diagnostic858);
  }

  if (_diagPtr)
    _diagPtr->printHeader();

  for (const PricingUnit* prU : farePath.pricingUnit())
  {
    if (UNLIKELY(_diagPtr))
      _diagPtr->print(*prU);

    for (FareUsage* fu : prU->fareUsage())
    {
      TicketEndorseLine* line = sortAndGlue(trx, *farePath.itin(), *fu, endoCat, _diagPtr);

      if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
      {
        if (!line)
          continue;
      }
      message.push_back(line);
    }
  }

  if (UNLIKELY(_diagPtr))
    _diagPtr->printEndorsement(farePath);

  for (const TicketEndorseItem& tei : farePath.tktEndorsement())
  {
    TicketEndorseLine* newLine = &trx.dataHandle().safe_create<TicketEndorseLine>(tei);
    addSegmentOrders(newLine, tei, *farePath.itin());
    message.push_back(newLine);
  }
}

bool
TicketingEndorsement::checkTicketEndoLines(const std::vector<TicketEndorseLine*>& messages)
{
  for (const TicketEndorseLine* line : messages)
  {
    if (!line)
      return false;

    if (line->priorityCode == 0)
      continue;

    if (!line->segmentOrders.size())
      return false;
  }
  return true;
}

void
TicketingEndorsement::sortLinesByPnr(const PricingTrx& trx,
                                     std::vector<TicketEndorseLine*>& messages)
{
  if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
  {
    if (messages.empty())
      return;

    TSE_ASSERT(checkTicketEndoLines(messages));
  }

  if (!fallback::fallbackEndorsementsRefactoring(&trx))
  {
    removeTheSameTexts(messages);
  }
  else
  {
    removeTheSameTextsOld(messages);
  }

  if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
  {
    std::sort(messages.begin(), messages.end(), lowerPnrSegmentOrder);
  }
  else
  {
    std::sort(messages.begin(), messages.end(), LowerPnrSegmentOrder());
  }

  if (UNLIKELY(_diagPtr))
  {
    _diagPtr->printGluedMsgs(messages);
    _diagPtr->flushMsg();
  }
}

void
TicketingEndorsement::sortLinesByPrio(const PricingTrx& trx,
                                      const FarePath& farePath,
                                      std::vector<TicketEndorseLine*>& messages)
{
  if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
  {
    if (messages.empty())
      return;

    TSE_ASSERT(checkTicketEndoLines(messages));
  }

  CarrierCode matchCxr;
  // Set up the carrier to use for match. If validating carrier exists
  // use it. Otherwise use the originating carrier for the itinerary.
  if (trx.getRequest()->validatingCarrier().empty())
  {
    const AirSeg* seg = dynamic_cast<const AirSeg*>(*farePath.itin()->travelSeg().begin());
    if (LIKELY(seg))
      matchCxr = seg->carrier();
  }
  else
    matchCxr = trx.getRequest()->validatingCarrier();

  if (trx.getRequest() && trx.getRequest()->ticketingAgent() &&
      ((TrxUtil::isAbacusEndorsementHierarchyAtpcoFaresActive(trx) &&
        trx.getRequest()->ticketingAgent()->abacusUser()) ||
       (TrxUtil::isInfiniEndorsementHierarchyAtpcoFaresActive(trx) &&
        trx.getRequest()->ticketingAgent()->infiniUser())))
  {
    std::sort(messages.begin(), messages.end(), lowerPriorityNumAndSegmentOrder);
  }
  else
  {
    if (!fallback::fallbackProcessEmptyTktEndorsements(&trx))
    {
      LowerPriorityNumAndCarrier comparator(matchCxr);
      std::sort(messages.begin(), messages.end(), comparator);
    }
    else
    {
      LowerPriorityNumAndCarrierOld comparator(matchCxr);
      std::sort(messages.begin(), messages.end(), comparator);
    }
  }

  if (!fallback::fallbackEndorsementsRefactoring(&trx))
  {
    removeTheSameTexts(messages);
  }
  else
  {
    removeTheSameTextsOld(messages);
  }

  if (UNLIKELY(_diagPtr))
  {
    _diagPtr->printGluedMsgs(messages, matchCxr);
    _diagPtr->flushMsg();
  }
}

// ### remove together with fallbackEndorsementsRefactoring ########################################
void
TicketingEndorsement::removeTheSameTextsOld(std::vector<TicketEndorseLine*>& messages) const
{
  std::vector<TicketEndorseLine*> output;

  for (TicketEndorseLine* line : messages)
  {
    if (line->endorseMessage.empty())
      continue;

    // Behavior below is taken from old code. I don't understand it, I don't even try to
    // understand it;)
    TicketEndoLines::iterator oldLine = find_if(output.begin(), output.end(), LineComparator(line));
    if (oldLine == output.end())
      output.push_back(line);
    else
    {
      (**oldLine).carrier = line->carrier;
      (**oldLine).priorityCode = line->priorityCode;
      (**oldLine).segmentOrders.insert(line->segmentOrders.begin(), line->segmentOrders.end());
    }
  }

  messages.swap(output);
}
//##################################################################################################

void
TicketingEndorsement::removeTheSameTexts(std::vector<TicketEndorseLine*>& messages) const
{
  std::vector<TicketEndorseLine*> output;

  for (TicketEndorseLine* line : messages)
  {
    if (line->endorseMessage.empty())
      continue;

    std::vector<TicketEndorseLine*>::iterator oldLine = find_if(output.begin(), output.end(),
      [line](TicketEndorseLine* item) { return item->endorseMessage == line->endorseMessage; });

    if (oldLine == output.end())
    {
      output.push_back(line);
    }
    else
    {
      // this overrides carrier and priority code - do we really want to do that?!
//      (*oldLine)->carrier = line->carrier;
//      (*oldLine)->priorityCode = line->priorityCode;
      (*oldLine)->segmentOrders.insert(line->segmentOrders.begin(), line->segmentOrders.end());
    }
  }

  messages.swap(output);
}

void
TicketingEndorsement::addSegmentOrders(TicketEndorseLine* tel,
                                       const TicketEndorseItem& tei,
                                       const Itin& itin)
{
  if (!tei.paxTypeFare)
    return;

  for (const TravelSeg* ts : tei.paxTypeFare->fareMarket()->travelSeg())
  {
    tel->segmentOrders.insert(itin.segmentOrder(ts));
  }
}

}
