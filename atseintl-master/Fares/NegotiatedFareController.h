//-------------------------------------------------------------------
//
//  File:        NegotiatedFareController.h
//  Created:     Aug 2, 2004
//  Authors:     Nakamon Thamsiriboon/Doug Boeving
//
//  Description: Negotiated fare factory
//
//  Updates:
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/CurrencyConversionFacade.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/NegPaxTypeFareData.h"
#include "DataModel/NegPaxTypeFareDataComparator.h"
#include "DataModel/NegPTFBucket.h"
#include "DataModel/NegPTFBucketContainer.h"
#include "Fares/FareController.h"
#include "Fares/FareRetailerRuleValidator.h"

#include <memory>

namespace tse
{
class CurrencyConversionFacade;
class CurrencyConversionCache;
class PricingTrx;
class FareMarket;
class PaxTypeFare;
class GeneralFareRuleInfo;
class CategoryRuleItemInfo;
class NegFareRest;
class NegFareSecurity;
class NegFareCalc;
class NegPaxTypeFareRuleData;
class Diag335Collector;
class NegFareRestExt;
class FareProperties;
class PrintOption;

class NegotiatedFareController : public FareController
{
  friend class NegotiatedFareControllerTest;

public:
  NegotiatedFareController(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  bool process();

  void writeDiagnostics() const;

  static const Indicator BLANK;
  static const Indicator LOC1_TO_LOC2;
  static const Indicator LOC2_TO_LOC1;
  static const Indicator DECLINED;
  static const Indicator PENDING;
  static const MoneyAmount INVALID_AMT;

protected:
  NegotiatedFareController(const NegotiatedFareController& rhs);
  NegotiatedFareController& operator=(const NegotiatedFareController& rhs);

  bool getFareRetailerRuleLookupInfo();
  bool checkFare(PaxTypeFare& ptFare) const;
  bool checkFareJalAxess(PaxTypeFare& ptFare) const;
  bool checkFareCat25WithCat35(const PaxTypeFare& ptFare) const;
  bool checkFareNeg(PaxTypeFare& ptFare) const;
  bool checkFareRexTrx(const PaxTypeFare& ptFare) const;
  static bool checkFareTariffCarrier(const PaxTypeFare& ptFare);
  static bool isSellingOrNet(const Indicator fcaDisplayCatType);
  void invalidateBaseFare(PaxTypeFare& ptFare) const;

  void processFare(PaxTypeFare& ptFare);
  void processCurrentRule(PaxTypeFare& ptFare, bool isLocationSwapped);
  bool checkNotNetFareForAxess(const PaxTypeFare& ptFare);
  bool checkGeneralFareRuleInfo(PaxTypeFare& paxTypeFare, GeneralFareRuleInfoVec& gfrInfoVec);
  void selectNegFares();
  void updateFareMarket();

  virtual void processRule(PaxTypeFare& ptFare, bool isLocationSwapped);
  void processRulePerCxr(PaxTypeFare& ptFare, bool isLocationSwapped);

  bool processCategoryRuleItemInfo(const PaxTypeFare& ptFare, bool isLocationSwapped) const;

  bool processNegFareRest(PaxTypeFare& ptFare);

  bool checkDirectionalityAndPaxType(const PaxTypeCode& paxType) const;

  bool getValidCalc(PaxTypeFare& ptFare,
                    NegPaxTypeFareRuleData* ruleData,
                    MoneyAmount& amt,
                    MoneyAmount& amtNuc);

  bool get979(PaxTypeFare& ptFare, bool isUpd, NegFareCalc& calcObj, bool isCat25Responsive=false);

  bool getOneCalc(PaxTypeFare& ptFare,
                  bool isUpd,
                  long secondarySellerId,
                  NegFareCalc& calcObj,
                  NegPaxTypeFareRuleData* ruleData,
                  Money& base,
                  MoneyAmount& amt,
                  MoneyAmount& amtNuc,
                  NegFareSecurityInfo* secRec);

  bool processMarkupForUpdate(
      PaxTypeFare& ptFare, bool isUpd, long secondarySellerId, NegFareCalc& calcObj, bool isTvlPcc);

  bool isFareRetailerRuleForUpdate(const FareRetailerRuleInfo* frri);

  bool matchMarkupRec(const MarkupControl& markupRec,
                      Indicator markupType,
                      bool isTvlPcc,
                      long secondarySellerId);

  static bool checkMarkupControl(const MarkupControl& markupRec, bool ticketOrExchange);
  bool matchedCorpIDInPricing(const PaxTypeFare& ptFare) const;

  bool keepMinAmt(MoneyAmount& saveAmt,
                  MoneyAmount newAmt,
                  MoneyAmount& saveAmtNuc,
                  MoneyAmount newAmtNuc,
                  NegPaxTypeFareRuleData* ruleData,
                  NegFareCalc& calcObj,
                  const PaxTypeFare& ptFare,
                  Indicator ticketingInd,
                  const NegFareRest& negFareRest,
                  Indicator fareDisplayCat35,
                  bool isFareRetailer = false);

  void keepFareRetailerMinAmtInfo(NegPaxTypeFareRuleData* ruleData,
                                  const PseudoCityCode& pcc,
                                  const uint64_t& ruleId,
                                  const uint64_t& seqNo,
                                  const FareRetailerCode& rc,
                                  bool isCat25Responsive=false);

  bool shallKeepMinAmt(const MoneyAmount& saveAmt,
                       const MoneyAmount& newAmt,
                       const NegPaxTypeFareRuleData& ruleData,
                       const PaxTypeFare& ptFare,
                       const NegFareRest& negFareRest);

  virtual void
  resetMinAmt(MoneyAmount& saveAmt, MoneyAmount& saveAmtNuc, NegPaxTypeFareRuleData*& ruleData);

  bool isSoftPass(NegPaxTypeFareRuleData* ruleData);

  virtual Record3ReturnTypes validate(PaxTypeFare& ptFare);
  bool validatePaxType(const PaxTypeFare& ptFare, const PaxTypeCode& paxTypeCode) const;
  bool validateDateOverrideTable(const PaxTypeFare& ptFare, uint32_t overrideDateTblItemNo) const;

  bool oldValidateCat35TktData(const PaxTypeFare& ptFare) const;
  bool validateCat35TktData(const PaxTypeFare& ptFare) const;
  bool oldValidateCat35TktDataL(const Indicator& fareType,
                                const Indicator& methodType,
                                bool anyTicketedFareData) const;
  bool validateCat35TktDataL(const Indicator& fareType,
                             const Indicator& methodType,
                             bool validCommisions) const;
  bool validateCat35TktDataT(const Indicator& fareType,
                             const Indicator& methodType,
                             bool validCommisions) const;
  bool validateCat35TktDataC(const Indicator& fareType,
                             const Indicator& methodType,
                             bool validCommisions) const;
  bool validateCat35TktDataCT(const Indicator& fareType,
                              const Indicator& methodType,
                              bool anyTicketedFareData) const;
  bool validateCat35TktDataLCT(const Indicator& fareType,
                               const Indicator& methodType,
                               bool anyTicketedFareData) const;

  bool validateCat35TktDataKorea(const Indicator& methodType,
                                 bool ticketOrExchange,
                                 const Agent& agent) const;

  bool validateCat35SegData(bool anyTicketedFareData) const;
  bool checkBetwAndCities() const;

  void doDiag335Collector(const char* failCode) const;
  void doDiag335Collector(const std::string& msg, bool showDiag) const;
  void diagDisplayRemainingRelation(const std::vector<CategoryRuleItemInfo>& seg,
                                    std::vector<CategoryRuleItemInfo>::const_iterator endIt);

  bool isAnyTicketedFareData() const;
  bool isRecurringSegData() const;
  bool isValidCommissions() const;

  virtual bool createFare(PaxTypeFare& ptFare,
                          NegPaxTypeFareRuleData* ruleData,
                          bool isLocationSwapped,
                          MoneyAmount fareAmount,
                          MoneyAmount fareAmountNuc,
                          bool pricingOption,
                          const Indicator* fareDisplayCat35 = nullptr);
  bool checkIfNeedNewFareAmt(PaxTypeFare& ptFare,
                             PaxTypeFare& newPTFare,
                             const Indicator* fareDisplayCat35);
  void printMatchedFare(const NegPaxTypeFareRuleData& ruleData,
                        const PaxTypeFare& newPTFare,
                        const bool forceDisplay = false);
  FareInfo* createFareInfoAndSetFlags(PaxTypeFare& ptFare,
                                      PaxTypeFare& newPTFare,
                                      NegPaxTypeFareRuleData& ruleData,
                                      const NegFareRest& negFareRest,
                                      const Indicator* fareDisplayCat35);
  void setAmounts(PaxTypeFare& ptFare,
                  PaxTypeFare& newPTFare,
                  NegPaxTypeFareRuleData& ruleData,
                  const NegFareRest& negFareRest,
                  FareInfo& fareInfo,
                  MoneyAmount fareAmount,
                  MoneyAmount fareAmountNuc,
                  const Indicator* fareDisplayCat35);

  void fillSegInfo(const PaxTypeFare& ptFare,
                   PaxTypeFare& newPTFare,
                   const NegFareRest& negFareRest) const;

  bool isContextValid(const FareRetailerRuleContext& context);

  bool processPermissionIndicators(const FareRetailerRuleContext& context,
                                   const NegFareSecurityInfo* secRec,
                                   NegPaxTypeFareRuleData* ruleData,
                                   const Indicator& catType);

  bool processPermissionIndicatorsOld(const FareRetailerRuleContext& context,
                                      const NegFareSecurityInfo* secRec,
                                      NegPaxTypeFareRuleData* ruleData,
                                      const Indicator& catType);

  Agent* createAgentSourcePcc(const PseudoCityCode& sourcePcc);

  bool processSourcePccSecurityMatch(const std::vector<NegFareSecurityInfo*>& secList,
                                     const Agent* sourcePcc,
                                     FareRetailerRuleContext& context);

  bool processSecurityMatch(bool& isPosMatch,
                            bool& is979queried,
                            MoneyAmount& amt,
                            MoneyAmount& amtNuc,
                            Money& base,
                            NegPaxTypeFareRuleData* ruleData,
                            PaxTypeFare& ptFare,
                            int32_t& seqNegMatch,
                            NegFareCalc& calcObj);
  virtual bool createFaresFromCalcObj(PaxTypeFare& ptFare,
                                      NegFareCalc& calcObj,
                                      NegFareSecurityInfo* secRec,
                                      NegPaxTypeFareRuleData* ruleData,
                                      Money& base,
                                      MoneyAmount& amt,
                                      MoneyAmount& amtNuc,
                                      Indicator ticketingInd);
  bool processCalcObj(bool& isPosMatch,
                      bool& is979queried,
                      int32_t& seqNegMatch,
                      PaxTypeFare& ptFare,
                      NegFareCalc& calcObj,
                      NegFareSecurityInfo* secRec,
                      NegPaxTypeFareRuleData* ruleData,
                      Money& base,
                      MoneyAmount& amt,
                      MoneyAmount& amtNuc,
                      const NegFareSecurity& secObj,
                      Indicator ticketOrExchange);
  bool checkTktAuthorityAndUpdFlag(const NegFareSecurity& secObj,
                                   NegFareSecurityInfo* secRec,
                                   Indicator isTkt,
                                   PaxTypeFare& ptf);
  void setNationFrance(const LocKey& lc1, const LocKey& lc2);

  virtual void processRedistributions(PaxTypeFare& ptFare,
                                      NegPaxTypeFareRuleData* ruleData,
                                      MoneyAmount& amt,
                                      MoneyAmount& amtNuc,
                                      Money& base,
                                      NegFareCalc& calcObj,
                                      bool is979queried,
                                      int32_t seqNegMatch);

  bool processMarkupForRedistribution(PaxTypeFare& ptFare,
                                      NegPaxTypeFareRuleData* ruleData,
                                      MoneyAmount& amt,
                                      MoneyAmount& amtNuc,
                                      Money& base,
                                      NegFareCalc& calcObj,
                                      bool is979queried,
                                      int32_t seqNegMatch,
                                      std::set<long>& secIds,
                                      bool isTvlPcc);

  bool
  processFareRetailerRuleValidation(PaxTypeFare& ptFare,
                                    NegPaxTypeFareRuleData* ruleData,
                                    MoneyAmount& amt,
                                    MoneyAmount& amtNuc,
                                    Money& base,
                                    NegFareCalc& calcObj,
                                    bool is979queried,
                                    bool isForUpdate,
                                    NegFareSecurityInfo* secRec);

  bool
  processFareRetailerRuleLandT(PaxTypeFare& ptFare,
                               NegPaxTypeFareRuleData* ruleData,
                               MoneyAmount& amt,
                               MoneyAmount& amtNuc,
                               Money& base,
                               NegFareCalc& calcObj,
                               NegFareSecurityInfo* secRec);

  bool
  processFareRetailerRuleTypeCFbrResponsive(PaxTypeFare& ptFare,
                                            NegPaxTypeFareRuleData* ruleData,
                                            MoneyAmount& amt,
                                            MoneyAmount& amtNuc,
                                            Money& base,
                                            NegFareCalc& calcObjReserved,
                                            bool is979queried,
                                            bool isForUpdate,
                                            NegFareSecurityInfo* secRec);

  void
  getRedistWholeSaleCalcPrice(NegFareCalc& calcObj,
                              PaxTypeFare& ptFare,
                              NegPaxTypeFareRuleData* negPaxTypeFare,
                              const Money& base);

  void getSecIdsForRedistribution(int32_t seqNegMatch, std::set<long>& ret) const;

  static bool
  ifMarkupValidForRedistribution(const MarkupControl& markupCntrl, bool ticketOrExchange);

  bool matchMultiCorpIdAccCodeMUC(const MarkupControl& markupRec, const PaxTypeFare& paxTypeFare);

  bool matchMultiCorpIdAccCodeMUCFlexFare(const MarkupControl& markupRec,
                                          const PaxTypeFare& paxTypeFare);
  bool matchMUCAccCodeFlexFare(const MarkupControl& markupRec,
                               const std::set<std::string>& accCodes) const;
  bool checkMultiCorpIdMatrixFlexFare(const MarkupControl& markupRec,
                                      const PaxTypeFare& paxTypeFare,
                                      const std::set<std::string>& corpIdSet);

  bool validateAllCarriers(PaxTypeFare& ptFare);

  bool matchCorpIdAccCodeMUC(const MarkupControl& markupRec, const PaxTypeFare& paxTypeFare);
  bool matchEmptyCorpIdAccCodeMUC(const PaxTypeFare& paxTypeFare);
  void prepareNewSellingAmt(const PaxTypeFare& paxTypeFare, MoneyAmount& retAmt) const;
  void createFaresForFD(PaxTypeFare& ptFare,
                        MoneyAmount newAmt,
                        MoneyAmount newAmtNuc,
                        Money& base,
                        NegFareCalc& calcObj,
                        NegFareSecurityInfo& secRec,
                        const Indicator& ticketInd);

  virtual void invokeCreateFare(PaxTypeFare& ptFare,
                                MoneyAmount fareAmount,
                                MoneyAmount nucFareAmount,
                                NegFareCalc& calcObj,
                                NegFareSecurityInfo* secRec,
                                Indicator fareDisplayCat35,
                                Indicator ticketingInd);

  virtual void saveCandidateFares(MoneyAmount& amt,
                                  MoneyAmount newAmt,
                                  MoneyAmount& amtNuc,
                                  MoneyAmount newAmtNuc,
                                  NegPaxTypeFareRuleData* ruleData,
                                  NegFareCalc& calcObj,
                                  NegFareSecurityInfo* secRec,
                                  PaxTypeFare& ptFare,
                                  Indicator fareDisplayCat35,
                                  Indicator ticketingInd);

  void setCat35Indicator(PaxTypeFare& ptFare);

  bool
  checkTktAuthority(const NegFareSecurity& secObj, NegFareSecurityInfo* secRec, Indicator isTkt);

  bool checkUpdFlag(const NegFareSecurityInfo* secRec, PaxTypeFare& ptf);

  bool checkCorpIdMatrix(const MarkupControl& muc, const PaxTypeFare& paxTypeFare) const;

  bool matchCorpIdMatrix(const MarkupControl& muc,
                         const PaxTypeFare& paxTypeFare,
                         const std::vector<tse::CorpId*>& corpIds) const;

  bool
  matchMUCAccCode(const MarkupControl& markupRec, const std::vector<std::string>& accCodes) const;

  virtual bool
  checkMultiCorpIdMatrix(const MarkupControl& markupRec, const PaxTypeFare& paxTypeFare);
  bool isJalAxessTypeC(PaxTypeFare& paxTypeFare);

  void setRexCreateFaresForPrevAgent();
  bool setRexCreateFaresForPrevAgent(const PaxTypeFare& excItinFare, const FareMarket& newItinFm);
  const Agent* getAgent() const;

  // method accessing database - overriden in tests
  virtual void
  getGeneralFareRuleInfo(const PaxTypeFare& paxTypeFare, GeneralFareRuleInfoVec& gfrInfoVec) const;
  virtual const std::vector<NegFareSecurityInfo*>& getNegFareSecurity() const;
  virtual const std::vector<MarkupControl*>&
  getMarkupBySecurityItemNo(const PaxTypeFare& ptFare) const;
  virtual const std::vector<MarkupControl*>&
  getMarkupBySecondSellerId(const PaxTypeFare& ptFare,
                            uint32_t rec2SeqNum,
                            long secondarySellerId) const;
  virtual const std::vector<MarkupControl*>&
  getMarkupByPcc(const PaxTypeFare& ptFare, bool isTvlPcc) const;
  virtual const std::vector<NegFareCalcInfo*>& getNegFareCalc() const;
  void addPaxTypeFare(const NegPaxTypeFareData& data, std::ostringstream& oss);
  const CarrierCode& valCarrier() const { return _currentValidatingCxr; }
  CarrierCode& validatingCxr() { return _currentValidatingCxr; }

  bool processFrrv(const PaxTypeFare& ptFare);
  bool isPtfValidForCat25Responsive(const PaxTypeFare& ptFare);

  // data members
  DiagCollector* _dc = nullptr;
  bool _dcDisplayRelation = false;
  std::vector<PaxTypeFare*> _negPaxTypeFares;

  FareRetailerRuleValidator _fareRetailerRuleValidator;
  uint64_t _fareRetailerSuccessCount = 0;
  uint64_t _markupSuccessCount = 0;
  std::vector<const FareRetailerRuleLookupInfo*> _frrlVecTypeC;
  std::vector<const FareRetailerRuleLookupInfo*> _frrlVecTypeLT;
  std::vector<const FareRetailerRuleLookupInfo*> _frrlVecTypeCFbr;
  std::vector<FareRetailerRuleContext> _frrcVecTypeC;
  std::vector<FareRetailerRuleContext> _frrcVecTypeLT;
  std::vector<FareRetailerRuleContext> _frrcVecTypeCFbr;

  bool _isNewMinAmt = false;
  bool _isHierarchyApplied = false;
  bool _invalidAgency = false;
  bool _isLocationSwapped = false;
  bool _nationFranceFare = false;
  bool _nationFranceLocalInd = false;
  bool _accCodeMatch = false;
  bool _accCodeMUCMatch = false;
  std::string _accCodeMUCValue;
  std::string _accCodeValue;
  bool _isJalAxessUser = false;
  bool _isJalAxessC35Request = false;
  bool _isJalAxessWPNETT = false;
  bool _needByPassNegFares = false;

  Indicator _fareDisplayMinCat35Type = ' ';
  bool _rexCreateFaresForPrevAgent = false;
  bool _processPrevAgentForRex = false;
  bool _passSecurity = false;

  const GeneralFareRuleInfo* _ruleInfo = nullptr;
  const NegFareRest* _negFareRest = nullptr;
  const CategoryRuleItemInfoSet* _ruleItemInfoSet = nullptr;
  const CategoryRuleItemInfo* _catRuleItemInfo = nullptr;
  const std::vector<CategoryRuleItemInfo>* _segQual = nullptr;
  uint16_t _itinIndex = 0;

  const NegFareRestExt* _negFareRestExt = nullptr;
  bool _negFareRestExtRetrieved = false;
  std::vector<NegFareRestExtSeq*> _negFareRestExtSeq;
  const FareProperties* _fareProperties = nullptr;
  bool _farePropPrintOptRetrieved = false;
  const ValueCodeAlgorithm* _valueCodeAlgorithm = nullptr;
  const PrintOption* _printOption = nullptr;
  NegPaxTypeFareRuleData* _ruleDataFareRetailer = nullptr;

  /* We use NegPaxTypeFareData to collect info for each new fare we create.
   * It is reset after every createFare cal
   **/
  mutable NegPaxTypeFareData _rpData;

  /* Each item in the vector contains collection of negotiated fares we create per base fare.
   * Each inner vector contains fares we create for same R3
   **/
  std::shared_ptr<NegPTFBucketContainer<NegPaxTypeFareDataComparator>> _negPtfBucketContainer;

  CarrierCode _currentValidatingCxr;
  bool _isFDFareCreatedForFareRetailer = false;
  bool _isFDCat25Responsive = false;

private:
  uint16_t _r2SubSetNum = 0;
  PaxTypeFare* _lastCreatedPtf = nullptr;
  std::unique_ptr<CurrencyConversionFacade> _ccf;
  CurrencyConversionCache _cache;
};

} // namespace tse

