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
#include "LegacyFacades/test/PricingTrxBuilder.h"

#define INLINE inline

namespace tse
{

class PricingTrxBuilder::Impl
{
  PricingTrxBuilder& _this;

public:
  Impl(PricingTrxBuilder& this_obj) : _this(this_obj) {}

  tse::Itin& fill(tse::Itin& itin)
  {
    _this.airSeg1.origin() = &_this.loc1;
    _this.airSeg1.destination() = &_this.loc2;
    _this.airSeg2.origin() = &_this.loc3;
    _this.airSeg2.destination() = &_this.loc4;
    itin.travelSeg() = {&_this.airSeg1, &_this.airSeg2};
    itin.validatingCarrier() = "LA";
    itin.validatingCxrGsaData() = &_this.validatingCxrGSAData;

    SpValidatingCxrGSADataMap* spGsaDataMap =
      _this._trx.dataHandle().create<SpValidatingCxrGSADataMap>();
    spGsaDataMap->insert(std::pair<SettlementPlanType,
          const ValidatingCxrGSAData*>("ARC", &_this.validatingCxrGSAData));
    itin.spValidatingCxrGsaDataMap() = spGsaDataMap;

    return itin;
  }

  tse::FarePath& fill(tse::FarePath& farePath)
  {
    farePath.setTotalNUCAmount(100);
    farePath.baseFareCurrency() = "USD";
    farePath.calculationCurrency() = "USD";
    farePath.pricingUnit().push_back(&_this.pricingUnit);
    farePath.paxType() = &_this.outputPaxType;
    return farePath;
  }

  void tie(tse::Itin& itin, tse::FarePath& farePath)
  {
    fill(itin);
    fill(farePath);

    itin.farePath().push_back(&farePath);
    farePath.itin() = &itin;
  }

  void addGsaAlternative(tse::FarePath& farePath, const char (&cc)[3], tse::FarePath& altPath)
  {
    farePath.validatingCarriers().push_back(cc);
    fill(altPath);
    altPath.itin() = farePath.itin();
    altPath.validatingCarriers().push_back(cc);
    farePath.gsaClonedFarePaths().push_back(&altPath);
  }

  void addGsaAlternative(tse::FarePath& farePath, const char (&cc)[3])
  {
    farePath.validatingCarriers().push_back(cc);
  }
};

INLINE
PricingTrxBuilder::PricingTrxBuilder()
{
  fillIrrelevantData();
  Impl impl{*this};

  impl.tie(i1, f1_1);
  impl.tie(i1, f1_2);
  impl.tie(i2, f2_1);

  _trx.itin().push_back(&i1);
  _trx.itin().push_back(&i2);
}

INLINE
PricingTrxBuilder::PricingTrxBuilder(ItinLayoutA)
{
  fillIrrelevantData();
  Impl impl{*this};

  impl.tie(i1, f1_1);
  impl.tie(i2, f2_1);

  impl.addGsaAlternative(f1_1, "1A", f1_1A);
  impl.addGsaAlternative(f1_1, "1B");
  impl.addGsaAlternative(f2_1, "2A");
  impl.addGsaAlternative(f2_1, "2B");

  _trx.itin() = {&i1, &i2};
}

INLINE
void PricingTrxBuilder::fillIrrelevantData()
{
  billing.userPseudoCityCode() = "KRK";
  _trx.billing() = &billing;

  pricingRequest.ticketPointOverride() = "KTW";
  _trx.setRequest(&pricingRequest);
  ticketingAgent.currencyCodeAgent() = "USD";
  _trx.getRequest()->ticketingAgent() = &ticketingAgent;

  pricingOptions.currencyOverride() = "PLN";
  _trx.setOptions(&pricingOptions);

  _trx.ticketingDate() = tse::DateTime(2014, 9, 19, 14, 16, 0);

  agentLocation.loc() = std::string("DFW");
  agentLocation.nation() = std::string("US");
  _trx.getRequest()->ticketingAgent()->agentLocation() = &agentLocation;
  _trx.getRequest()->ticketingAgent()->tvlAgencyPCC() = "A123";
  _trx.getRequest()->ticketingAgent()->agentCity() = "DFW";
  _trx.getRequest()->ticketingAgent()->vendorCrsCode() = "X";
  _trx.getRequest()->ticketingAgent()->cxrCode() = "XX";

  inputPaxType0.age() = 20;
  _trx.paxType().push_back(&inputPaxType0);

  _trx.atpcoTaxesActivationStatus().setTaxOnItinYqYrTaxOnTax(true);
  _trx.setValidatingCxrGsaApplicable(true);

  // the following is what farePath or itin depend on
  validatingCxrGSAData.validatingCarriersData()["1A"];
  validatingCxrGSAData.validatingCarriersData()["1B"];
  validatingCxrGSAData.validatingCarriersData()["2A"];
  validatingCxrGSAData.validatingCarriersData()["2B"];


  pricingUnit.fareUsage().push_back(&fareUsage);
  fareUsage.paxTypeFare() = &paxTypeFare;
  fareUsage.travelSeg().push_back(&airSeg1);
  fareUsage.travelSeg().push_back(&airSeg2);
  ruleData.fbrApp() = &fareByRuleApp;
  ruleData.ruleItemInfo() = &ruleInfo;
  ruleData.baseFare() = &basePaxTypeFare;
  allRuleData.fareRuleData = &ruleData;
  paxTypeFare.setFare(&fare);
  paxTypeFare.segmentStatus().resize(2);
  paxTypeFare.actualPaxType() = &fareOutputPaxType;
  fare.setFareInfo(&fareInfo);
  basePaxTypeFare.setFare(&baseFare);
  baseFare.setFareInfo(&baseFareInfo);

  airSeg1.equipmentType() = "DH4";
  airSeg1.setOperatingCarrierCode("LA");
  airSeg1.setMarketingCarrierCode("LX");
  airSeg1.marketingFlightNumber() = 1111;
  airSeg1.departureDT() = tse::DateTime(2014, 9, 20, 14, 30, 0);
  airSeg1.arrivalDT() = tse::DateTime(2014, 9, 20, 15, 45, 0);
  airSeg2.equipmentType() = "767";
  airSeg2.setOperatingCarrierCode("LA");
  airSeg2.setMarketingCarrierCode("LX");
  airSeg2.marketingFlightNumber() = 2222;
  airSeg2.departureDT() = tse::DateTime(2014, 9, 21, 8, 30, 0);
  airSeg2.arrivalDT() = tse::DateTime(2014, 9, 22, 6, 00, 0);
  airSeg2.hiddenStops().push_back(&hidden1);
  airSeg2.hiddenStops().push_back(&hidden2);
  loc1.loc() = "KRK";
  loc1.city() = "KRK";
  loc1.nation() = "PL";
  loc2.loc() = "WAW";
  loc2.city() = "WAW";
  loc2.nation() = "PL";
  loc3.loc() = "WAW";
  loc3.city() = "WAW";
  loc3.nation() = "PL";
  loc4.loc() = "NRT";
  loc4.city() = "TYO";
  loc4.nation() = "JP";
  hidden1.loc() = "AAA";
  hidden1.city() = "AAA";
  hidden1.nation() = "RU";
  hidden2.loc() = "BBB";
  hidden2.city() = "BBB";
  hidden2.nation() = "CN";

  fareByRuleApp.accountCode() = "DEF";
  paxTypeFare.status().set(tse::PaxTypeFare::PTF_FareByRule);
  (*paxTypeFare.paxTypeFareRuleDataMap())[25] = &allRuleData;
  fare.nucFareAmount() = 100;
  fareInfo.owrt() = 'O';
  fareInfo.directionality() = tse::BOTH;
  fareInfo.market1() = "KRK";
  fareInfo.market2() = "TYO";

  baseFare.nucFareAmount() = 120;
  baseFareInfo.owrt() = 'O';
  baseFareInfo.directionality() = tse::BOTH;
  baseFareInfo.market1() = "KRK";
  baseFareInfo.market2() = "TYO";

  // Passengers init
  outputPaxType.age() = 20;
  outputPaxType.paxType() = "ADT";
  fareOutputPaxType.paxType() = "ADT";
}

} // namespace tse
