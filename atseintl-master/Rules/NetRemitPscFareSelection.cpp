//----------------------------------------------------------------------------
//  Copyright Sabre 2010
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
#include "Rules/NetRemitPscFareSelection.h"

#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/RepricingTrx.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareRestExt.h"
#include "DBAccess/NegFareRestExtSeq.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

namespace tse
{
NetRemitPscFareSelection::NetRemitPscFareSelection(
    PricingTrx& trx,
    const FarePath& fPath,
    PricingUnit& pu,
    FareUsage& fu,
    const NegFareRest& negFareRest,
    const NegFareRestExt& negFareRestExt,
    const std::vector<NegFareRestExtSeq*>& negFareRestExtSeqs)
  : NetRemitFareSelection(trx, fPath, pu, fu, negFareRest),
    _negFareRestExt(negFareRestExt),
    _negFareRestExtSeqs(negFareRestExtSeqs)
{
}

void
NetRemitPscFareSelection::process()
{
  printHeader();

  displayNetRemitPscResults();
  selectTktPblFare();

  if (UNLIKELY(_diag))
    _diag->flushMsg();
}

void
NetRemitPscFareSelection::displayNetRemitPscResults()
{
  if (LIKELY(!_diag || !_diag->isActive()))
    return;

  DiagCollector& dc = *_diag;

  dc.setf(std::ios::left, std::ios::adjustfield);

  std::vector<const NegFareRestExtSeq*> negFareRestExtSeq;

  FareUsage::TktNetRemitPscResultVec::const_iterator pscResI = _fu.netRemitPscResults().begin();
  for (; pscResI != _fu.netRemitPscResults().end(); ++pscResI)
    negFareRestExtSeq.push_back(pscResI->_tfdpscSeqNumber);

  dc.displayTFDPSC(dc, negFareRestExtSeq, true);
}

const CarrierCode&
NetRemitPscFareSelection::getCarrierCode(const CarrierCode& carrier, const AirSeg& seg) const
{
  if (carrier.empty())
    return _fu.paxTypeFare()->carrier();
  else if (carrier == ANY_CARRIER)
    return seg.carrier();
  return carrier;
}

const PaxTypeFare*
NetRemitPscFareSelection::selectTktPblFare(const FareMarket& fareMarket)
{
  return NetRemitFareSelection::selectTktPblFare(fareMarket);
}

const FareMarket*
NetRemitPscFareSelection::getPblFareMarket(const Loc* origLoc,
                                           const Loc* destLoc,
                                           const CarrierCode& cxr,
                                           const DateTime& deptDT,
                                           const DateTime& arrDT)
{
  GlobalDirectionFinderV2Adapter::getGlobalDirection(
      &_trx, _trx.travelDate(), _tvlSegs, _overrideGlobalDir);

  if (_overrideGlobalDir == GlobalDirection::XX)
    return nullptr;

  return NetRemitFareSelection::getPblFareMarket(origLoc, destLoc, cxr, deptDT, arrDT);
}

void
NetRemitPscFareSelection::createTravelSeg(const TravelSeg* startTvlSeg, const TravelSeg* endTvlSeg)
{
  const std::vector<TravelSeg*>& fuTvlSegs = _fu.paxTypeFare()->fareMarket()->travelSeg();

  std::vector<TravelSeg*>::const_iterator tvlSegI =
      std::find(fuTvlSegs.begin(), fuTvlSegs.end(), startTvlSeg);
  std::vector<TravelSeg*>::const_iterator tvlSegIE =
      std::find(fuTvlSegs.begin(), fuTvlSegs.end(), endTvlSeg);
  if (tvlSegI == fuTvlSegs.end() || tvlSegIE == fuTvlSegs.end() || tvlSegIE < tvlSegI)
    return;
  ++tvlSegIE;
  _tvlSegs.assign(tvlSegI, tvlSegIE);
}

const PaxTypeFare*
NetRemitPscFareSelection::selectTktPblFare(const TravelSeg* tvlSeg,
                                           const FareUsage::TktNetRemitPscResult& pcsRes)
{
  const AirSeg* seg = static_cast<const AirSeg*>(tvlSeg);
  const CarrierCode& cxr = getCarrierCode(pcsRes._tfdpscSeqNumber->carrier(), *seg);

  createTravelSeg(tvlSeg, pcsRes._endTravelSeg);

  const FareMarket* fareMarket = getPblFareMarket(seg->origin(),
                                                  pcsRes._endTravelSeg->destination(),
                                                  cxr,
                                                  seg->departureDT(),
                                                  pcsRes._endTravelSeg->arrivalDT());
  if (!fareMarket)
    return nullptr;
  return selectTktPblFare(*fareMarket);
}

void
NetRemitPscFareSelection::selectTktPblFare()
{
  FareUsage::TktNetRemitPscResultVec::iterator pscResI = _fu.netRemitPscResults().begin();
  for (; pscResI != _fu.netRemitPscResults().end(); ++pscResI)
  {
    _negFareRestExtSeq = pscResI->_tfdpscSeqNumber;
    const PaxTypeFare* paxTypeFare = selectTktPblFare(pscResI->_startTravelSeg, *pscResI);
    if (!paxTypeFare)
    {
      clearResultFare();
      return;
    }
    pscResI->_resultFare = paxTypeFare;
  }
}

bool
NetRemitPscFareSelection::checkAndBetw(std::vector<PaxTypeFare*>& selFares, bool isAbacusEnabled)
{
  return true;
}

bool
NetRemitPscFareSelection::isMatchedFare(const PaxTypeFare& paxTypeFare) const
{
  if (!_negFareRestExtSeq->publishedFareBasis().empty() &&
      !RuleUtil::matchFareClass(_negFareRestExtSeq->publishedFareBasis().c_str(),
                                paxTypeFare.fareClass().c_str()))
    return false;

  if (paxTypeFare.directionality() == BOTH ||
      (_fu.isInbound() && paxTypeFare.directionality() == TO) ||
      (_fu.isOutbound() && paxTypeFare.directionality() == FROM))
    return true;
  return false;
}

bool
NetRemitPscFareSelection::isFareFamilySelection()
{
  return NetRemitFareSelection::isFareFamilySelection(_negFareRestExtSeq->publishedFareBasis());
}

RepricingTrx*
NetRemitPscFareSelection::runRepriceTrx(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr)
{
  return TrxUtil::reprice(_trx,
                          tvlSegs,
                          FMDirection::UNKNOWN,
                          false,
                          &cxr,
                          &_overrideGlobalDir,
                          ((_fp.paxType()->paxType() == JCB) ? ADULT : ""),
                          false,
                          TrxUtil::cat35LtypeEnabled(_trx));
}

const FareMarket*
NetRemitPscFareSelection::getFareMarket(RepricingTrx* rpTrx,
                                        std::vector<TravelSeg*>& tvlSegs,
                                        const CarrierCode& cxr)
{
  const FareMarket* fareMarket = NetRemitFareSelection::getFareMarket(rpTrx, tvlSegs, cxr);
  if (!fareMarket)
    return nullptr;
  const_cast<FareMarket*>(fareMarket)->travelSeg() = _tvlSegs;
  return fareMarket;
}

const Loc*
NetRemitPscFareSelection::getOrig(const Loc* origLoc, const Loc* destLoc) const
{
  return origLoc;
}

const Loc*
NetRemitPscFareSelection::getDest(const Loc* origLoc, const Loc* destLoc) const
{
  return destLoc;
}

void
NetRemitPscFareSelection::clearResultFare()
{
  FareUsage::TktNetRemitPscResultVec::iterator pscResI = _fu.netRemitPscResults().begin();
  for (; pscResI != _fu.netRemitPscResults().end(); ++pscResI)
  {
    pscResI->_resultFare = nullptr;
  }
}
}
