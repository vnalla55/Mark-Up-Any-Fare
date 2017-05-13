//----------------------------------------------------------------------------
//  Copyright Sabre 2010
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
#pragma once

#include "DataModel/FareUsage.h"
#include "Rules/NetRemitFareSelection.h"

#include <vector>

namespace tse
{
class AirSeg;
class PaxTypeFare;
class NegFareRestExt;
class NegFareRestExtSeq;
class RepricingTrx;

class NetRemitPscFareSelection : public NetRemitFareSelection
{
  friend class NetRemitFareSelection;
  friend class NetRemitPscFareSelectionTest;

private:
  NetRemitPscFareSelection(PricingTrx& trx,
                           const FarePath& fPath,
                           PricingUnit& pu,
                           FareUsage& fu,
                           const NegFareRest& negFareRest,
                           const NegFareRestExt& negFareRestExt,
                           const std::vector<NegFareRestExtSeq*>& negFareRestExtSeqs);

  void process();

  void displayTicketedFareData() override {}
  void displayNetRemitPscResults();
  const CarrierCode& getCarrierCode(const CarrierCode& carrier, const AirSeg& seg) const;
  virtual const PaxTypeFare* selectTktPblFare(const FareMarket& fareMarket);
  virtual const FareMarket* getPblFareMarket(const Loc* origLoc,
                                             const Loc* destLoc,
                                             const CarrierCode& cxr,
                                             const DateTime& deptDT,
                                             const DateTime& arrDT);
  void createTravelSeg(const TravelSeg* startTvlSeg, const TravelSeg* endTvlSeg);
  const PaxTypeFare*
  selectTktPblFare(const TravelSeg* tvlSeg, const FareUsage::TktNetRemitPscResult& pcsRes);
  void selectTktPblFare();
  bool checkAndBetw(std::vector<PaxTypeFare*>& selFares, bool isAbacusEnabled) override;
  bool isMatchedFare(const PaxTypeFare& paxTypeFare) const override;
  bool isFareFamilySelection() override;
  RepricingTrx* runRepriceTrx(std::vector<TravelSeg*>& tvlSegs, const CarrierCode& cxr) override;
  const FareMarket* getFareMarket(RepricingTrx* rpTrx,
                                  std::vector<TravelSeg*>& tvlSegs,
                                  const CarrierCode& cxr) override;
  const Loc* getOrig(const Loc* origLoc, const Loc* destLoc) const override;
  const Loc* getDest(const Loc* origLoc, const Loc* destLoc) const override;
  void clearResultFare();

  // Data
private:
  const NegFareRestExt& _negFareRestExt;
  const std::vector<NegFareRestExtSeq*>& _negFareRestExtSeqs;
  const NegFareRestExtSeq* _negFareRestExtSeq = nullptr;
  GlobalDirection _overrideGlobalDir = GlobalDirection::NO_DIR;
  std::vector<TravelSeg*> _tvlSegs;
};
}

