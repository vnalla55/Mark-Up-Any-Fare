//----------------------------------------------------------------------------
//  File:        FareValidatorESV.h
//  Created:     2008-04-16
//
//  Description: Class used to validate fares on specified fare market
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "DataModel/PricingUnit.h"
#include "DataModel/ShoppingTrx.h"
#include "Rules/RuleConst.h"
#include "Rules/RuleControllerWithChancelor.h"
#include "Server/TseServer.h"

namespace tse
{

class RuleControllers
{
public:
  RuleControllers(std::vector<uint16_t>& PVseq,
                  std::vector<uint16_t>& SCVseq,
                  std::vector<uint16_t>& FRVseq,
                  std::vector<uint16_t>& C4Qseq)
    : preValidationRC(PreValidation, PVseq),
      shoppingComponentValidationRC(ShoppingComponentValidation, SCVseq),
      shoppingComponentWithFlightsValidationRC(ShoppingComponentWithFlightsValidation, FRVseq),
      shoppingComponentValidateQualifiedCat4RC(ShoppingComponentValidateQualifiedCat4, C4Qseq)
  {
    RuleItem& ri = preValidationRC.categoryRuleItemSet().categoryRuleItem().ruleItem();
    ri.clearHandlers();
    ri.setHandler(RuleConst::SALE_RESTRICTIONS_RULE, &RuleItem::handleSalesSecurity);
    ri.setHandler(RuleConst::ELIGIBILITY_RULE, &RuleItem::handleEligibility);
  }

  RuleControllerWithChancelor<FareMarketRuleController> preValidationRC;
  RuleControllerWithChancelor<FareMarketRuleController> shoppingComponentValidationRC;
  RuleControllerWithChancelor<FareMarketRuleController> shoppingComponentWithFlightsValidationRC;
  RuleControllerWithChancelor<FareMarketRuleController> shoppingComponentValidateQualifiedCat4RC;
};

class FareValidatorESV
{

public:
  enum RuleCategory
  {
    CAT_1,
    CAT_2,
    CAT_2FR,
    CAT_3,
    CAT_4FR,
    CAT_4Q,
    CAT_5,
    CAT_6,
    CAT_7,
    CAT_8FR,
    CAT_9FR,
    CAT_11,
    CAT_14,
    CAT_14FR,
    CAT_15S,
    CAT_15
  };

  enum ValidatedFareType
  {
    VDF_TAG_1_3, // OW, OJ (tag 1 or 3)
    VDF_TAG_2, // OJ (tag 2)
    VDF_TAG_1_2, // RT, CT (tag 1 or 2)
    VDF_TAG_1_3_104, // OW, OJ (tag 1 or 3 with sc104 = false)
    VDF_TAG_2_104, // OJ (tag 2 with sc104 = false)
    VDF_TAG_1_2_102_104, // RT (tag 1 or 2 with sc102 = false or sc104 = false)
    VDF_TAG_1_2_103, // CT (tag 1 or 2 with sc103 = false)
    VDF_TAG_END
  };

  struct ValidatedFares
  {
    ValidatedFares() : paxTypeFare(VDF_TAG_END, nullptr) {}

    PaxTypeFare*& operator[](size_t tag) { return paxTypeFare[tag]; }

  private:
    std::vector<PaxTypeFare*> paxTypeFare;
  };

  struct ValidatedPTF
  {
    static ValidatedPTF* create(DataHandle& dh, const BookingCode& c, PaxTypeFare* f, bool isFlow)
    {
      ValidatedPTF* v = nullptr;
      dh.get(v);
      v->bkc = c;
      v->ptFare = f;
      v->flow = isFlow;
      return v;
    }

    ValidatedPTF() : ptFare(nullptr), flow(false) {}

    BookingCode bkc;
    PaxTypeFare* ptFare;
    bool flow;

    double queueRank() const { return ptFare->fareAmount(); }
  };

  typedef std::vector<ValidatedPTF*> ValidatedPaxTypeFares;

  struct LocalJourneyValidatedFares
  {
    static LocalJourneyValidatedFares* create(DataHandle& dh)
    {
      LocalJourneyValidatedFares* ljvf = nullptr;
      dh.get(ljvf);
      for (auto& elem : ljvf->paxTypeFares)
        dh.get(elem);
      return ljvf;
    }

    LocalJourneyValidatedFares() : paxTypeFares(VDF_TAG_END, nullptr) {}

    ValidatedPaxTypeFares*& operator[](size_t tag) { return paxTypeFares[tag]; }

  private:
    std::vector<ValidatedPaxTypeFares*> paxTypeFares;
  };

  struct VFHolder
  {
    VFHolder() : _ljvf(nullptr), _vf(nullptr) {}

    VFHolder(LocalJourneyValidatedFares* ljvf) : _ljvf(ljvf), _vf(nullptr) {}

    VFHolder(ValidatedFares* vf) : _ljvf(nullptr), _vf(vf) {}

    LocalJourneyValidatedFares* _ljvf;
    ValidatedFares* _vf;
  };

  FareValidatorESV(ShoppingTrx& shoppingTrx,
                   TseServer& server,
                   ShoppingTrx::Leg& leg,
                   Itin* journeyItin,
                   Itin* itin,
                   ItinIndex::ItinCellInfo& itinCellInfo,
                   uint32_t flightId,
                   FareMarket* fareMarket,
                   uint32_t fareMarketsCount,
                   PaxType* paxType,
                   RuleControllers& ruleControllersESV,
                   std::vector<uint32_t>& rule_validation_order)
    : _trx(&shoppingTrx),
      _tseServer(&server),
      _leg(&leg),
      _journeyItin(journeyItin),
      _itin(itin),
      _itinCellInfo(&itinCellInfo),
      _flightId(flightId),
      _fareMarket(fareMarket),
      _fareMarketsCount(fareMarketsCount),
      _paxType(paxType),
      _ruleControllersESV(&ruleControllersESV),
      _rule_validation_order(&rule_validation_order) {}

  void findFirstValidFares(ValidatedFares& validatedFares);
  void findFirstValidFares(LocalJourneyValidatedFares& validatedFares);

protected:
  typedef std::set<std::pair<BookingCode, ValidatedFareType> > ValidatedTypes;

  bool doRulesValidation(PaxTypeFare* paxTypeFare);

  bool outputResultsWithBFCat10Validation(PaxTypeFare* paxTypeFare, ValidatedFares& validatedFares);

  bool outputResultsWithBFCat10Validation(PaxTypeFare* paxTypeFare,
                                          const BookingCode& bookingCode,
                                          ValidatedTypes& validatedTypes,
                                          LocalJourneyValidatedFares& validatedFares,
                                          bool isFlow);

  void getFaresForPaxType(std::vector<PaxTypeFare*>& paxTypeFareVecRet);

  const std::vector<PaxTypeFare*>& getFaresForPaxType() const;

  bool validateBookingCode(PaxTypeFare* paxTypeFare,
                           std::vector<ClassOfServiceList>& availabilityList,
                           BookingCode& validatingBookingCode);

  bool checkAvailability(PaxTypeFare::SegmentStatusVec& segmentStatusVec,
                         const std::vector<TravelSeg*>& travelSegVector,
                         std::vector<ClassOfServiceList>& availabilityList,
                         std::vector<BookingCode>* bookingCodesPtr,
                         BookingCode& validatingBookingCode);

  void getFlowAvailability(std::vector<ClassOfServiceList>& availabilityList);

  void getLocalAvailability(std::vector<ClassOfServiceList>& availabilityList);

  void getCustomAvailability(std::vector<ClassOfServiceList>& availabilityList);

  bool skipFareIfCat10NotValid(PaxTypeFare* paxTypeFare, ValidatedFares& validatedFares);

  bool validateCat1(PaxTypeFare* paxTypeFare);
  bool validateCat15S(PaxTypeFare* paxTypeFare);

  bool validateCat2(PaxTypeFare* paxTypeFare);
  bool validateCat3(PaxTypeFare* paxTypeFare);
  bool validateCat5(PaxTypeFare* paxTypeFare);
  bool validateCat6(PaxTypeFare* paxTypeFare);
  bool validateCat7(PaxTypeFare* paxTypeFare);
  bool validateCat11(PaxTypeFare* paxTypeFare);
  bool validateCat14(PaxTypeFare* paxTypeFare);
  bool validateCat15(PaxTypeFare* paxTypeFare);

  bool validateCat2FR(PaxTypeFare* paxTypeFare);
  bool validateCat4FR(PaxTypeFare* paxTypeFare);
  bool validateCat8FR(PaxTypeFare* paxTypeFare);
  bool validateCat9FR(PaxTypeFare* paxTypeFare);
  bool validateCat14FR(PaxTypeFare* paxTypeFare);

  bool validateCat4Q(PaxTypeFare* paxTypeFare);

  ShoppingTrx* _trx;
  TseServer* _tseServer;
  ShoppingTrx::Leg* _leg;
  Itin* _journeyItin;
  Itin* _itin;
  ItinIndex::ItinCellInfo* _itinCellInfo;
  uint32_t _flightId;
  FareMarket* _fareMarket;
  uint32_t _fareMarketsCount;
  PaxType* _paxType;
  RuleControllers* _ruleControllersESV;
  std::vector<uint32_t>* _rule_validation_order;
};

} // End namespace tse

