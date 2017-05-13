//----------------------------------------------------------------------------
//  Copyright Sabre 2011
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
#include "FareCalc/NvbNvaOrchestrator.h"

#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/CombinabilityRuleItemInfo.h"
#include "DBAccess/EndOnEnd.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "DBAccess/NvbNvaInfo.h"
#include "DBAccess/NvbNvaSeg.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag861Collector.h"
#include "FareCalc/CalcTotals.h"
#include "Rules/RuleUtil.h"

#include <boost/bind.hpp>

namespace tse
{
using namespace std;
const char NvbNvaOrchestrator::EOE_IND_RESTRICTIONS = 'R';

NvbNvaOrchestrator::NvbNvaOrchestrator(PricingTrx& trx,
                                       const FarePath& farePath,
                                       CalcTotals& calcTotals)
  : _trx(trx),
    _farePath(farePath),
    _calcTotals(calcTotals),
    _diag(*static_cast<Diag861Collector*>(DCFactory::instance()->create(trx)))
{
  if (trx.diagnostic().diagnosticType() == Diagnostic861)
  {
    _diag.enable(Diagnostic861);
    _diag.trx() = &trx;
  }
}

NvbNvaOrchestrator::~NvbNvaOrchestrator()
{
  if (_diag.isActive())
  {
    _diag.flushMsg();
  }
}

void
NvbNvaOrchestrator::process()
{
  processNVANVBDate(_trx, _farePath, _calcTotals);
  processNvbNvaTable();
  if (TrxUtil::isNetRemitEnabled(_trx))
  {
    processNvbNvaSuppression();
  }
  _diag.clearLogIfDataEmpty();
}

void
NvbNvaOrchestrator::processNvbNvaTable()
{
  _diag.printNvbNvaTableHeader();

  vector<FareUsage*> fus;
  buildVirtualPu(fus);

  if (!fus.empty())
  {
    processPu(fus, false);
  }
  else
  {
    for (PricingUnit* pu : _farePath.pricingUnit())
      processPu(pu->fareUsage(), true);
  }

  _diag.printNvbNvaTableFooter();
}

void
NvbNvaOrchestrator::buildVirtualPu(std::vector<FareUsage*>& fus) const
{
  bool foundEndOnEnd = false;
  bool foundAllSmf = false;

  for (PricingUnit* pu : _farePath.pricingUnit())
  {
    if (pu->fareUsage().end() != find_if(pu->fareUsage().begin(),
                                         pu->fareUsage().end(),
                                         boost::bind(&NvbNvaOrchestrator::eoeRequired, *this, _1)))
    {
      foundEndOnEnd = true;
    }

    if (areAllFaresSmf(pu->fareUsage()))
    {
      foundAllSmf = true;
    }
  }

  if (foundEndOnEnd && foundAllSmf)
  {
    _diag << "EOE REQUIRED - SPECIAL PU PROCESSING ACTIVATED" << std::endl;

    for (PricingUnit* pu : _farePath.pricingUnit())
    {
      _diag.logPricingUnit(pu->fareUsage(), _farePath.itin(), _calcTotals);

      if (areAllFaresSmf(pu->fareUsage()))
      {
        for (FareUsage* fu : pu->fareUsage())
          fus.push_back(fu);
      }
      else if (std::any_of(pu->fareUsage().cbegin(),
                           pu->fareUsage().cend(),
                           [](const FareUsage* fu)
                           { return fu->endOnEndRequired(); }))
      {
        for (FareUsage* fu : pu->fareUsage())
          fus.push_back(fu);

        _diag << "PRICING UNIT CONTAINS FARE WITH EOE REQUIRED" << std::endl;
      }
      else
      {
        _diag << "PRICING UNIT NOT PROCESSED" << std::endl;
      }
    }
    _diag << "PROCESS FARES " << std::endl;
  }
}

void
NvbNvaOrchestrator::processPu(const vector<FareUsage*>& fus, bool isPuReal)
{
  if (LIKELY(isPuReal))
  {
    _diag.logPricingUnit(fus, _farePath.itin(), _calcTotals);
    _diag << "PROCESS FARES " << std::endl;
    if (!(areAllFaresSmf(fus) && areAnyFaresWithoutPenalties(fus)))
    {
      _diag << "PRICING UNIT NOT PROCESSED" << std::endl;
      return;
    }
  }
  Indicator nvb = NVB_EMPTY;
  Indicator nva = NVA_EMPTY;

  for (FareUsage* fu : fus)
  {
    _diag << *fu;
    if (fu->changePenaltyApply())
      _diag << " CATEGORY 16 FEE" << std::endl;

    const std::vector<NvbNvaInfo*>* nvbNvaInfos =
        &(_trx.dataHandle().getNvbNvaInfo(fu->paxTypeFare()->vendor(),
                                          fu->paxTypeFare()->carrier(),
                                          fu->paxTypeFare()->fareTariff(),
                                          fu->paxTypeFare()->ruleNumber()));

    if (nvbNvaInfos->empty())
    {
      nvbNvaInfos = &(_trx.dataHandle().getNvbNvaInfo(fu->paxTypeFare()->vendor(),
                                                      fu->paxTypeFare()->carrier(),
                                                      fu->paxTypeFare()->fareTariff(),
                                                      ANY_RULE));
    }
    if (nvbNvaInfos->empty())
    {
      if (fu->endOnEndRequired())
      {
        if (!fu->changePenaltyApply())
          _diag << " EXTENSION PU PER EOE REQUIRED SEE PROCESS" << std::endl;
      }
      else
      {
        _diag << " NOT APPLICABLE - NO VALID NVBNVA RECORD FOUND" << std::endl;
      }
    }
    else
    {
      for (NvbNvaInfo* nvbNvaInfo : *nvbNvaInfos)
        findNvbNva(*nvbNvaInfo, nvb, nva, fu->paxTypeFare()->fareClass().c_str());
    }
  }

  processNvb(fus, nvb);
  processNva(fus, nva);
}

bool
NvbNvaOrchestrator::areAllFaresSmf(const vector<FareUsage*>& fus) const
{
  for (FareUsage* fu : fus)
    if (_trx.dataHandle().getVendorType(fu->paxTypeFare()->vendor()) != RuleConst::SMF_VENDOR)
      return false;

  return true;
}

bool
NvbNvaOrchestrator::areAnyFaresWithoutPenalties(const vector<FareUsage*>& fus) const
{
  for (FareUsage* fu : fus)
    if (!fu->changePenaltyApply())
      return true;

  return false;
}

void
NvbNvaOrchestrator::findNvbNva(const NvbNvaInfo& nvbNvaInfo,
                               Indicator& nvb,
                               Indicator& nva,
                               const FareBasisCode& fareBasis) const
{
  _diag << nvbNvaInfo;
  for (NvbNvaSeg* seg : nvbNvaInfo.segs())
  {
    if (RuleUtil::matchFareClass(seg->fareBasis().c_str(), fareBasis.c_str()))
    {
      _diag.logNvbNvaSeg(*seg);
      setWinningNvbNvaFlag(nvb, seg->nvb());
      setWinningNvbNvaFlag(nva, seg->nva());
      return;
    }
    else
      _diag.logNvbNvaSeg(*seg, false);
  }
  _diag << "VALID SEGMENT NOT FOUND" << std::endl << " " << std::endl;
}

void
NvbNvaOrchestrator::setWinningNvbNvaFlag(Indicator& nvbNva, Indicator alternateNvbNva) const
{
  // choose flag with higher priority               NVA are digits   NVB are letters
  //                               " !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ"
  const static char priorities[] = "10000000000000000234000000000000000000000350000400020000000";

  if (priorities[nvbNva - NVB_EMPTY] < priorities[alternateNvbNva - NVB_EMPTY])
    nvbNva = alternateNvbNva;
}

void
NvbNvaOrchestrator::processNvb(const vector<FareUsage*>& fus, const Indicator& nvb)
{
  _diag.logNvb(nvb);
  switch (nvb)
  {
  case NVB_1ST_SECTOR:
    applyNvb1stSector(fus);
    break;
  case NVB_1ST_INTL_SECTOR:
    applyNvb1stIntlSector(fus);
    break;
  case NVB_ENTIRE_OUTBOUND:
    applyNvbEntireOutbound(fus);
    break;
  case NVB_ENTIRE_JOURNEY:
    applyNvbEntireJourney(fus);
    break;
  case NVB_EMPTY:
  default:
    break;
  }
}

void
NvbNvaOrchestrator::applyNvb1stSector(const vector<FareUsage*>& fus)
{
  FareUsage& fu = *fus.front();
  if (!fu.changePenaltyApply())
  {
    const TravelSeg* firstSeg = fu.travelSeg().front();
    _calcTotals.tvlSegNVB[segmentId(firstSeg)] = firstSeg->departureDT();
  }
}

void
NvbNvaOrchestrator::applyNvb1stIntlSector(const vector<FareUsage*>& fus)
{
  const DateTime& firstObSectorDate = fus.front()->travelSeg().front()->departureDT();

  for (FareUsage* fu : fus)
  {
    for (TravelSeg* travelSeg : fu->travelSeg())
    {
      if (LocUtil::isInternational(*travelSeg->origin(), *travelSeg->destination()))
      {
        if (!fu->changePenaltyApply())
          _calcTotals.tvlSegNVB[segmentId(travelSeg)] = firstObSectorDate;

        return;
      }
    }
  }
}

void
NvbNvaOrchestrator::applyNvbEntireOutbound(const vector<FareUsage*>& fus)
{
  const DateTime& firstObSectorDate = fus.front()->travelSeg().front()->departureDT();

  for (FareUsage* fu : fus)
  {
    if (fu->isOutbound() && !fu->changePenaltyApply())
    {
      for (TravelSeg* travelSeg : fu->travelSeg())
        _calcTotals.tvlSegNVB[segmentId(travelSeg)] = firstObSectorDate;
    }
  }
}

void
NvbNvaOrchestrator::applyNvbEntireJourney(const vector<FareUsage*>& fus)
{
  const DateTime& firstObSectorDate = fus.front()->travelSeg().front()->departureDT();

  for (FareUsage* fu : fus)
  {
    if (!fu->changePenaltyApply())
    {
      for (TravelSeg* travelSeg : fu->travelSeg())
        _calcTotals.tvlSegNVB[segmentId(travelSeg)] = firstObSectorDate;
    }
  }
}

void
NvbNvaOrchestrator::processNva(const vector<FareUsage*>& fus, const Indicator& nva)
{
  _diag.logNva(nva);
  switch (nva)
  {
  case NVA_1ST_SECTOR_EARLIEST:
    applyNva1stSectorEarliest(fus);
    break;
  case NVA_1ST_INTL_SECTOR_EARLIEST:
    applyNva1stIntlSectorEarliest(fus);
    break;
  case NVA_ENTIRE_OUTBOUND_EARLIEST:
    applyNvaEntireOutboundEarliest(fus);
    break;
  case NVA_EMPTY:
  default:
    break;
  }
}

// most restrictive date means earliest date
const DateTime&
NvbNvaOrchestrator::getMostRestrictiveNvaDate(const vector<FareUsage*>& fus) const
{
  static const DateTime maxDate(9999, 1, 1);
  const DateTime* earliestDT = &maxDate;

  for (FareUsage* fu : fus)
  {
    if (!fu->changePenaltyApply())
    {
      for (TravelSeg* travelSeg : fu->travelSeg())
      {
        const DateTime& dt = _calcTotals.tvlSegNVA[segmentId(travelSeg)];
        if (dt < *earliestDT)
          earliestDT = &dt;
      }
    }
  }
  return *earliestDT;
}

void
NvbNvaOrchestrator::applyNva1stSectorEarliest(const vector<FareUsage*>& fus)
{
  const FareUsage& fu = *fus.front();
  if (!fu.changePenaltyApply())
  {
    const TravelSeg* firstSeg = fu.travelSeg().front();
    _calcTotals.tvlSegNVA[segmentId(firstSeg)] = getMostRestrictiveNvaDate(fus);
  }
}

void
NvbNvaOrchestrator::applyNva1stIntlSectorEarliest(const vector<FareUsage*>& fus)
{
  const DateTime& mostRestrictiveNvaDate = getMostRestrictiveNvaDate(fus);

  for (FareUsage* fu : fus)
  {
    for (TravelSeg* travelSeg : fu->travelSeg())
    {
      if (LocUtil::isInternational(*travelSeg->origin(), *travelSeg->destination()))
      {
        if (!fu->changePenaltyApply())
        {
          _calcTotals.tvlSegNVA[segmentId(travelSeg)] = mostRestrictiveNvaDate;
        }
        return;
      }
    }
  }
}

void
NvbNvaOrchestrator::applyNvaEntireOutboundEarliest(const vector<FareUsage*>& fus)
{
  const DateTime& mostRestrictiveNvaDate = getMostRestrictiveNvaDate(fus);

  for (FareUsage* fu : fus)
  {
    if (fu->isOutbound() && !fu->changePenaltyApply())
    {
      for (TravelSeg* travelSeg : fu->travelSeg())
        _calcTotals.tvlSegNVA[segmentId(travelSeg)] = mostRestrictiveNvaDate;
    }
  }
}

void
NvbNvaOrchestrator::processNvbNvaSuppression()
{
  _diag.printSuppressionHeader();
  for (PricingUnit* pu : _farePath.pricingUnit())
  {
    for (FareUsage* fu : pu->fareUsage())
    {
      for (FareUsage::TktNetRemitPscResult nrResult : fu->netRemitPscResults())
      {
        if (nrResult._tfdpscSeqNumber && nrResult._tfdpscSeqNumber->suppressNvbNva() != ' ')
        {
          _diag << (*nrResult._tfdpscSeqNumber);
          suppressNvbNva(segmentId(nrResult._startTravelSeg), segmentId(nrResult._endTravelSeg));
        }
      }
    }
  }
  _diag.printSuppressionFooter();
}

void
NvbNvaOrchestrator::suppressNvbNva(int16_t startSegmentOrder, int16_t endSegmentOrder)
{
  for (int16_t idx = startSegmentOrder; idx <= endSegmentOrder; idx++)
  {
    _diag.logTravelSeg(idx, _farePath);
    _calcTotals.tvlSegNVA[idx] = _calcTotals.tvlSegNVB[idx] = DateTime::openDate();
  }
}

bool
NvbNvaOrchestrator::eoeRequired(const FareUsage* fu)
{
  std::vector<const EndOnEnd*> endOnEnds;
  getEndOnEnds(*fu, endOnEnds);

  for (const EndOnEnd* eoe : endOnEnds)
  {
    if (eoe->eoeNormalInd() == EOE_IND_RESTRICTIONS ||
        eoe->eoespecialInd() == EOE_IND_RESTRICTIONS || eoe->intlInd() == EOE_IND_RESTRICTIONS ||
        eoe->domInd() == EOE_IND_RESTRICTIONS || eoe->uscatransborderInd() == EOE_IND_RESTRICTIONS)
    {
      return true;
    }
  }
  return false;
}

void
NvbNvaOrchestrator::getEndOnEnds(const FareUsage& fu, std::vector<const EndOnEnd*>& endOnEnds)
{
  if (!fu.endOnEndRequired())
    return;

  for (const CombinabilityRuleItemInfo* cat10Seg : fu.eoeRules())
  {
    if (LIKELY(cat10Seg && cat10Seg->itemNo() != 0 && cat10Seg->textonlyInd() != 'Y'))
    {
      const EndOnEnd* pEndOnEnd =
          _trx.dataHandle().getEndOnEnd(fu.paxTypeFare()->vendor(), cat10Seg->itemNo());
      if (pEndOnEnd)
        endOnEnds.push_back(pEndOnEnd);
    }
  }
}

void
NvbNvaOrchestrator::processNVANVBDate(PricingTrx& trx,
                                      const FarePath& farePath,
                                      CalcTotals& calcTotals)
{
  const Itin* itin = farePath.itin();
  bool isWQTrx = (nullptr != dynamic_cast<NoPNRPricingTrx*>(&trx));

  if (!itin)
    return;

  // get all FareUsages, apply the most restriction rules from their
  // required End-On-End combination fares
  std::vector<FareUsage*> allFUVec;
  std::map<const FareUsage*, const PricingUnit*>::const_iterator fuPUMapI =
      calcTotals.pricingUnits.begin();
  std::map<const FareUsage*, const PricingUnit*>::const_iterator fuPUMapIEnd =
      calcTotals.pricingUnits.end();
  for (; fuPUMapI != fuPUMapIEnd; fuPUMapI++)
    allFUVec.push_back(const_cast<FareUsage*>(fuPUMapI->first));

  applyNVANVBEndOnEndRestr(trx, *itin, allFUVec);

  // Process each PricingUnit in the farePath
  std::vector<PricingUnit*>::const_iterator puI = farePath.pricingUnit().begin();
  const std::vector<PricingUnit*>::const_iterator puIEnd = farePath.pricingUnit().end();

  for (; puI != puIEnd; puI++)
  {
    PricingUnit& pu = **puI;

    processNVANVBDate(trx, pu, *itin);
  }

  puI = farePath.pricingUnit().begin();

  for (; puI != puIEnd; puI++)
  {
    PricingUnit& pu = **puI;

    SegnoDTMapI tvlNVBIter = pu.tvlSegNVB().begin();
    SegnoDTMapI tvlNVBIterEnd = pu.tvlSegNVB().end();
    for (; tvlNVBIter != tvlNVBIterEnd; tvlNVBIter++)
    {
      int16_t tvlSegNo = tvlNVBIter->first;
      if (LIKELY(tvlSegNo > 0))
        calcTotals.tvlSegNVB[tvlSegNo] = tvlNVBIter->second;
    }
    SegnoDTMapI tvlNVAIter = pu.tvlSegNVA().begin();
    SegnoDTMapI tvlNVAIterEnd = pu.tvlSegNVA().end();
    for (; tvlNVAIter != tvlNVAIterEnd; tvlNVAIter++)
    {
      int16_t tvlSegNo = tvlNVAIter->first;
      if (LIKELY(tvlSegNo > 0))
        calcTotals.tvlSegNVA[tvlSegNo] = tvlNVAIter->second;
    }
  }

  // If the date of itin first segment departure is not open date, apply
  // one year after the departure to anything that does not have NVA
  // restriction
  // lint -e{578}

  const TravelSeg& firstTvlSeg = *itin->travelSeg().front();
  if (firstTvlSeg.isOpenWithoutDate() && !isWQTrx)
  {
    return;
  }

  DateTime maxNVADT = firstTvlSeg.departureDT().addYears(1);

  std::map<int16_t, DateTime>::iterator tvlNVAIter = calcTotals.tvlSegNVA.begin();
  std::map<int16_t, DateTime>::iterator tvlNVAEndIter = calcTotals.tvlSegNVA.end();

  for (; tvlNVAIter != tvlNVAEndIter; tvlNVAIter++)
  {
    DateTime& nvaDate = tvlNVAIter->second;
    nvaDate.setWithEarlier(maxNVADT);

    // make sure NVA is not before segment actual departure date
    const DateTime& tvlSegTVLDT = (itin->travelSeg()[tvlNVAIter->first - 1])->departureDT();
    if (LIKELY(!tvlSegTVLDT.isOpenDate() || isWQTrx))
    {
      nvaDate.setWithLater(tvlSegTVLDT);
    }
  }
}

void
NvbNvaOrchestrator::processNVANVBDate(PricingTrx& trx, PricingUnit& pu, const Itin& itin)
{
  // initialize nva/nvb
  // if a PU all segments are open, do not display NVA/NVA on any its segment
  bool isWQTrx = (nullptr != dynamic_cast<NoPNRPricingTrx*>(&trx));
  bool isAllOpenPU = isAllSegmentsOpen(pu.travelSeg());

  std::vector<TravelSeg*>::const_iterator tvlSegI = pu.travelSeg().begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEndI = pu.travelSeg().end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    int16_t segmentOrder = itin.segmentOrder(*tvlSegI);

    pu.tvlSegNVA()[segmentOrder] = DateTime::openDate();
    pu.tvlSegNVB()[segmentOrder] = DateTime::openDate();
    if (UNLIKELY(isWQTrx))
      pu.tvlSegNVABDone()[segmentOrder] = false;
    else
      pu.tvlSegNVABDone()[segmentOrder] = isAllOpenPU;
  }

  if (isAllOpenPU && !isWQTrx)
    return;

  std::vector<FareUsage*>::const_iterator fareUsageI = pu.fareUsage().begin();
  std::vector<FareUsage*>::const_iterator fareUsageEndI = pu.fareUsage().end();

  bool isThereAnyNoPenalty = false;
  bool isThereAnyNotConfirmed = false;
  for (; fareUsageI != fareUsageEndI; fareUsageI++)
  {
    // If there is penalty
    if ((*fareUsageI)->changePenaltyApply())
      nvabApplyFUNoChange(trx, pu, isThereAnyNotConfirmed, itin, **fareUsageI);
    else
      isThereAnyNoPenalty = true;
  }
  if (!isThereAnyNoPenalty && !isThereAnyNotConfirmed && !isWQTrx)
    return;
  // For those no penalty or not confirmed (open), we would check Cat6/Cat7

  // Cat6, Cat7, Cat14, Combined, get the most restricted
  fareUsageI = pu.fareUsage().begin();
  for (; fareUsageI != fareUsageEndI; fareUsageI++)
    processNVANVBDate(pu, **fareUsageI, isWQTrx);

  applyMostRestrictedNVAToSegsInFront(pu);
}

void
NvbNvaOrchestrator::processNVANVBDate(PricingUnit& pu, FareUsage& fareUsage, bool isWQTrx)
{
  // Cat6
  int16_t tvlSegNo = fareUsage.startNVBTravelSeg();

  SegnoDTMapI tvlNVBIter = (tvlSegNo == 0) ? pu.tvlSegNVB().begin() : pu.tvlSegNVB().find(tvlSegNo);
  SegnoDTMapI tvlNVBEndIter = pu.tvlSegNVB().end();

  for (; tvlNVBIter != tvlNVBEndIter; tvlNVBIter++)
  {
    if (pu.tvlSegNVABDone()[tvlNVBIter->first] == true || (isWQTrx && tvlNVBIter->first == 1))
      continue;

    DateTime& mostRestrictedNVBDate = tvlNVBIter->second;
    if (!fareUsage.minStayDate().isOpenDate())
      mostRestrictedNVBDate.setWithLater(fareUsage.minStayDate());
  }

  // Cat7
  SegnoDTMapI tvlNVAIter = pu.tvlSegNVA().begin();
  SegnoDTMapI tvlNVAEndIter = pu.tvlSegNVA().end();

  for (; tvlNVAIter != tvlNVAEndIter; tvlNVAIter++)
  {
    if (pu.tvlSegNVABDone()[tvlNVAIter->first] == true)
      continue;

    DateTime& mostRestrictedNVADate = tvlNVAIter->second;
    if (!fareUsage.maxStayDate().isOpenDate())
      mostRestrictedNVADate.setWithEarlier(fareUsage.maxStayDate());
  }

  // Cat14 NVA
  std::map<uint16_t, const DateTime*>* nvaData = fareUsage.getNVAData();
  if (nvaData)
    applyNVAData(*nvaData, pu);

  nvaData = fareUsage.paxTypeFare()->getNVAData();
  if (nvaData)
    applyNVAData(*nvaData, pu);
}

void
NvbNvaOrchestrator::applyNVANVBEndOnEndRestr(PricingTrx& trx,
                                             const Itin& itin,
                                             std::vector<FareUsage*>& allFUVec)
{
  std::vector<FareUsage*>::iterator fuIter = allFUVec.begin();
  const std::vector<FareUsage*>::iterator fuIterEnd = allFUVec.end();
  for (; fuIter != fuIterEnd; fuIter++)
  {
    if ((*fuIter)->changePenaltyApply())
    {
      // already most restricted
      continue;
    }

    if ((*fuIter)->paxTypeFare()->tcrTariffCat() == RuleConst::PRIVATE_TARIFF)
      continue;

    std::vector<const FareUsage*> eoeFUVec;

    if (getEndOnEndRequiredFareUsages(trx.dataHandle(), fuIter, allFUVec, eoeFUVec))
    {
      std::vector<const FareUsage*>::const_iterator eoeFUIter = eoeFUVec.begin();
      std::vector<const FareUsage*>::const_iterator eoeFUIterEnd = eoeFUVec.end();
      for (; eoeFUIter != eoeFUIterEnd; eoeFUIter++)
      {
        // Apply the combined fares restriction dates, use the most
        // restricted one
        const FareUsage& fuCombined = **eoeFUIter;
        if (fuCombined.changePenaltyApply())
        {
          // no change allowed, apply same restriction
          (*fuIter)->changePenaltyApply() = true;
          break; // already most restricted
        }

        // Only apply MinStay to inbound fares;
        // make sure my fare's departure is after the MinStay,
        // Otherwise since we did not fail it, we can not display
        if ((*fuIter)->isInbound() && !fuCombined.minStayDate().isOpenDate() &&
            ((*fuIter)->travelSeg().front()->departureDT().date() >=
             fuCombined.minStayDate().date()))
        {
          (*fuIter)->minStayDate().setWithLater(fuCombined.minStayDate());
          int16_t myTvlSegOrder = itin.segmentOrder((*fuIter)->travelSeg().front());

          if ((*fuIter)->startNVBTravelSeg() == 0 || (*fuIter)->startNVBTravelSeg() > myTvlSegOrder)
          {
            (*fuIter)->startNVBTravelSeg() = myTvlSegOrder;
          }
        }
        // make sure my fare's departure is before the MaxStay,
        // Otherwise since we did not fail it, we can not display
        if ((*fuIter)->isInbound() && !fuCombined.maxStayDate().isOpenDate() &&
            ((*fuIter)->travelSeg().front()->departureDT().date() <=
             fuCombined.maxStayDate().date()))
        {
          (*fuIter)->maxStayDate().setWithEarlier(fuCombined.maxStayDate());
        }
      }
    }
  }
}

bool
NvbNvaOrchestrator::isAllSegmentsOpen(const std::vector<TravelSeg*>& tvlSegs)
{
  std::vector<TravelSeg*>::const_iterator tvlSegI = tvlSegs.begin();
  const std::vector<TravelSeg*>::const_iterator tvlSegEndI = tvlSegs.end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
    if (LIKELY(!(*tvlSegI)->isOpenWithoutDate()))
      return false;

  return true;
}

void
NvbNvaOrchestrator::nvabApplyFUNoChange(
    PricingTrx& trx, PricingUnit& pu, bool& isThereAnyNotConfirmed, const Itin& itin, FareUsage& fu)
{
  NoPNRPricingTrx* noPNRTrx = dynamic_cast<NoPNRPricingTrx*>(&trx);

  std::vector<TravelSeg*>::const_iterator tvlSegI = fu.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlSegEndI = fu.travelSeg().end();

  for (; tvlSegI != tvlSegEndI; tvlSegI++)
  {
    const TravelSeg& tvlSeg = **tvlSegI;
    if ((tvlSeg.resStatus() == CONFIRM_RES_STATUS) || (tvlSeg.resStatus() == NOSEAT_RES_STATUS) ||
        noPNRTrx)
    {
      int16_t segmentOrder = itin.segmentOrder(*tvlSegI);

      if (UNLIKELY(noPNRTrx && segmentOrder != 1 && itin.dateType() == Itin::NoDate))
        continue;

      DateTime departureDT = tvlSeg.departureDT();

      if (UNLIKELY(noPNRTrx))
        noPNRTrx->updateOpenDateIfNeccesary(&tvlSeg, departureDT);

      pu.tvlSegNVA()[segmentOrder] = departureDT;
      pu.tvlSegNVB()[segmentOrder] = departureDT;
      pu.tvlSegNVABDone()[segmentOrder] = true;
    }
    else
    {
      isThereAnyNotConfirmed = true;
    }
  }
}

void
NvbNvaOrchestrator::applyMostRestrictedNVAToSegsInFront(PricingUnit& pu)
{
  // All NVA of segment in front should apply the restriction of segment
  // after. For example, segment2 should not be NVA 01/11/2007 if segment3
  // NVA 03/11/2006. Exception, segment of no change penalty should remain
  // the same.
  VecMap<int16_t, DateTime>::reverse_iterator tvlNVARIterBack = pu.tvlSegNVA().rbegin();
  const VecMap<int16_t, DateTime>::reverse_iterator tvlNVARIterEnd = pu.tvlSegNVA().rend();

  for (; tvlNVARIterBack != tvlNVARIterEnd; tvlNVARIterBack++)
  {
    if (pu.tvlSegNVABDone()[tvlNVARIterBack->first])
      continue;

    const DateTime& nvaDateBack = tvlNVARIterBack->second;

    if (nvaDateBack.isOpenDate())
      continue;

    VecMap<int16_t, DateTime>::reverse_iterator tvlNVARIterFront = tvlNVARIterBack;
    tvlNVARIterFront++;
    for (; tvlNVARIterFront != tvlNVARIterEnd; tvlNVARIterFront++)
    {
      if (pu.tvlSegNVABDone()[tvlNVARIterFront->first])
        continue;

      DateTime& nvaDateFront = tvlNVARIterFront->second;

      if (!nvaDateFront.isOpenDate() && (nvaDateFront <= nvaDateBack))
        break;

      nvaDateFront = nvaDateBack;
    }
  }
}

void
NvbNvaOrchestrator::applyNVAData(std::map<uint16_t, const DateTime*>& nvaData, PricingUnit& pu)
{
  for (auto& tnvSegNva : nvaData)
  {
    SegnoDTMapI tvlNVAIter = pu.tvlSegNVA().find(tnvSegNva.first);

    if (tvlNVAIter != pu.tvlSegNVA().end())
    {
      DateTime& mostRestrictedNVADate = tvlNVAIter->second;
      if (mostRestrictedNVADate.isOpenDate())
      {
        mostRestrictedNVADate = *tnvSegNva.second;
      }
      else
      {
        mostRestrictedNVADate.setWithEarlier(*tnvSegNva.second);
      }
    }
  }
}

bool
NvbNvaOrchestrator::getEndOnEndRequiredFareUsages(
    DataHandle& dataHandle,
    const std::vector<FareUsage*>::iterator& curFUIter,
    const std::vector<FareUsage*>& allFUVec,
    std::vector<const FareUsage*>& eoeFUVec)
{
  if (!(*curFUIter)->endOnEndRequired())
    return false;

  FareUsage& curFareUsage = **curFUIter;

  for (const CombinabilityRuleItemInfo* cat10Seg: curFareUsage.eoeRules())
  {
    if (!cat10Seg || cat10Seg->itemNo() == 0 || cat10Seg->textonlyInd() == 'Y')
      continue;

    const EndOnEnd* pEndOnEnd =
        dataHandle.getEndOnEnd(curFareUsage.paxTypeFare()->vendor(), cat10Seg->itemNo());

    if (!pEndOnEnd)
      continue;

    EOEAllSegmentIndicator allSegmentIndicator = Adjacent;

    if (cat10Seg->eoeallsegInd() == 'A')
    {
      allSegmentIndicator = AllSegment;
    }
    else if (cat10Seg->eoeallsegInd() == 'C')
    {
      allSegmentIndicator = CommonPoint;
    }

    getEndOnEndCombinationsFU(*pEndOnEnd, allSegmentIndicator, curFUIter, allFUVec, eoeFUVec);
  } // for eoeRuleIter

  return !eoeFUVec.empty();
}

bool
NvbNvaOrchestrator::getEndOnEndCombinationsFU(const EndOnEnd& eoeRule,
                                              EOEAllSegmentIndicator& allSegmentIndicator,
                                              const std::vector<FareUsage*>::iterator& curFUIter,
                                              const std::vector<FareUsage*>& allFUVec,
                                              std::vector<const FareUsage*>& eoeFUVec)
{
  if (eoeRule.unavailTag() == 'Y' /* Combinations::TEXT_DATA_ONLY */)
    return false;

  FareUsage& curFareUsage = **curFUIter;

  std::vector<FareUsage*>::const_iterator fuIter = allFUVec.begin();
  const std::vector<FareUsage*>::const_iterator fuIterEnd = allFUVec.end();

  for (; fuIter != fuIterEnd; fuIter++)
  {
    if (curFUIter == fuIter)
      continue;

    const FareUsage* targetFU = *fuIter;

    bool fareMatch = false;

    if (allSegmentIndicator == AllSegment)
    {
      fareMatch = true;
    }
    else if (allSegmentIndicator == Adjacent)
    {
      if ((fuIter + 1) == curFUIter)
      {
        fareMatch = true;
      }
      else if (fuIter == (curFUIter + 1))
      {
        fareMatch = true;
        fuIter = fuIterEnd - 1; // no more FU would match
      }
    }
    else if (allSegmentIndicator == CommonPoint)
    {
      const LocCode& cOrig = curFareUsage.paxTypeFare()->fareMarket()->boardMultiCity();
      const LocCode& cDest = curFareUsage.paxTypeFare()->fareMarket()->offMultiCity();
      const LocCode& tOrig = (*fuIter)->paxTypeFare()->fareMarket()->boardMultiCity();
      const LocCode& tDest = (*fuIter)->paxTypeFare()->fareMarket()->offMultiCity();

      if ((cOrig == tOrig) || (cOrig == tDest) || (cDest == tOrig) || (cDest == tDest))
        fareMatch = true;
    }

    if (!fareMatch)
      continue;

    fareMatch = false;

    if ((eoeRule.eoeNormalInd() == EOE_IND_RESTRICTIONS && targetFU->isPaxTypeFareNormal()) ||
        (eoeRule.eoespecialInd() == EOE_IND_RESTRICTIONS && targetFU->paxTypeFare()->isSpecial()) ||
        (eoeRule.uscatransborderInd() == EOE_IND_RESTRICTIONS &&
         targetFU->paxTypeFare()->isTransborder()) ||
        (eoeRule.intlInd() == EOE_IND_RESTRICTIONS && targetFU->paxTypeFare()->isInternational()))
    {
      fareMatch = true;
    }
    else if (UNLIKELY(eoeRule.domInd() == EOE_IND_RESTRICTIONS))
    {
      const FareMarket& targetFM = *targetFU->paxTypeFare()->fareMarket();
      const NationCode& targetNation = targetFM.origin()->nation();
      if (targetNation == targetFM.destination()->nation())
      {
        const FareMarket& curFM = *curFareUsage.paxTypeFare()->fareMarket();
        if ((curFM.origin()->nation() == targetNation) ||
            (curFM.destination()->nation() == targetNation))
          fareMatch = true;
      }
    }

    if (fareMatch && eoeRule.sameCarrierInd() == 'X' /* Combinations::RESTRICTION_APPLIES*/)
    {
      if (curFareUsage.paxTypeFare()->carrier() != targetFU->paxTypeFare()->carrier())
        fareMatch = false;
    }

    // TODO, other check needed to find out what we really End-On-End
    // with
    if (fareMatch)
      eoeFUVec.push_back(targetFU);
  }

  return true;
}

int16_t
NvbNvaOrchestrator::segmentId(const TravelSeg* seg) const
{
  return _farePath.itin()->segmentOrder(seg);
}

} // namespace tse
