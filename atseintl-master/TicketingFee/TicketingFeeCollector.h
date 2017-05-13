//-------------------------------------------------------------------
//  Copyright Sabre 2009
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

#include "Common/Logger.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/TicketingFeesInfo.h"

#include <boost/optional.hpp>

namespace tse
{
class PricingTrx;
class FarePath;
class TravelSeg;
class Diag870Collector;
class Loc;
class Money;
class FareUsage;
class PaxTypeFare;
class PaxType;
class LocKey;
class PaxTypeMatrix;
class SvcFeesTktDesigValidator;

class TicketingFeeComparator
{
public:
  TicketingFeeComparator() {};
  virtual ~TicketingFeeComparator() {};
  bool operator()(TicketingFeesInfo* lhs, TicketingFeesInfo* rhs) const
  {
    return (lhs->serviceSubTypeCode() < rhs->serviceSubTypeCode());
  }
};

class TicketingFeeCollector
{
  friend class TicketingFeeCollectorTest;

public:
  TicketingFeeCollector(PricingTrx* trx, FarePath* _farePath);
  virtual ~TicketingFeeCollector();

  void collect();

protected:
  virtual bool setMlgTurnAround();
  virtual bool validateGeoLoc1Loc2(const TicketingFeesInfo* feeInfo) const;
  virtual bool validateGeoVia(const TicketingFeesInfo* feeInfo) const;
  virtual bool validateGeoWhlWithin(const TicketingFeesInfo* feeInfo) const;
  virtual bool
  svcFeesTktDesignatorValidate(const SvcFeesTktDesigValidator& validator, int itemNo) const;

private:
  static const Indicator CREDIT;
  static const Indicator DEBIT;
  static const Indicator BLANK;

  static constexpr int initialInvalidValue = -1;

  TicketingFeeCollector(const TicketingFeeCollector& rhs);
  TicketingFeeCollector& operator=(const TicketingFeeCollector& rhs);

  PricingTrx* _trx;
  FarePath* _farePath;
  Diag870Collector* _diag;
  bool _diagInfo;
  const Loc* _journeyTurnAroundPoint;
  const TravelSeg* _journeyTurnAroundTs;
  const Loc* _journeyDestination;
  int _segmentOrderTurnAround;
  std::map<uint32_t, int, std::greater<uint32_t>> _mlgMap; // Used to determine TurnAround Point
  std::set<TravelSeg*> _validSegs; // Stopovers and FareBreaks
  bool _stopS4Processing;
  bool _stopS4ProcessingFCA;
  bool _stopS4ProcessingFDA;
  bool _binExists;
  bool _processFop;
  bool _process2CC;
  Indicator _cardType;
  Indicator _secondCardType;
  bool _halfRoundTripSplit;
  boost::optional<uint16_t> _frequentFlyerTierLevel;

  static Logger _logger;

  bool serviceFeeActiveForCarrier();
  StatusS4Validation isValidTrx() const;
  OBFeeSubType getOBFeeSubType(const TicketingFeesInfo* feeInfo) const;
  std::vector<TicketingFeesInfo*>& ticketingFeesInfoFromFP(TicketingFeesInfo* feeInfo);
  bool atleastOneSegmentConfirm() const;
  const CarrierCode& validatingCarrier() const;
  virtual const std::vector<TicketingFeesInfo*>& getTicketingFee() const;
  bool validateSequence(const TicketingFeesInfo* feeInfo, StatusS4Validation& rc);
  bool checkTicketingDates(const TicketingFeesInfo* feeInfo) const;
  bool checkServiceType(const TicketingFeesInfo* feeInfo) const;
  bool checkServiceSubType(const TicketingFeesInfo* feeInfo) const;
  void checkFopBinNumber(const TicketingFeesInfo* feeInfo);
  bool checkPaxType(const TicketingFeesInfo* feeInfo) const;
  bool matchSabrePaxList(const PaxTypeCode& farePathPtc, const PaxTypeCode& tktFeePtc) const;
  virtual const std::vector<const PaxTypeMatrix*>&
  getSabrePaxTypes(const PaxTypeCode& farePathPtc) const;
  bool checkFareBasis(const TicketingFeesInfo* feeInfo);
  bool checkFrequentFlyer(const TicketingFeesInfo* tktFeesInfo);
  bool checkFareBasisTktIndAll(const TicketingFeesInfo* tktFeesInfo) const;
  bool checkFareBasisTktIndAny(const TicketingFeesInfo* tktFeesInfo) const;
  virtual bool matchDiffFareBasisTktIndAll(const FareUsage* fu,
                                           std::string tktFareBasis,
                                           CarrierCode tktPrimCxr) const;
  virtual bool matchDiffFareBasisTktIndAny(const FareUsage* fu,
                                           std::string tktFareBasis,
                                           CarrierCode tktPrimCxr) const;
  virtual bool isIndustryFare(const PaxTypeFare& paxTypeFare) const;
  virtual std::string getFareBasis(const PaxTypeFare& paxTypeFare) const;
  virtual bool matchFareClass(const std::string ptfFareBasis, const std::string tktFareBasis) const;
  virtual std::string
  getS1FareBasisCode(const PaxTypeFare& paxTypeFare, const TravelSeg* lastAirSeg) const;
  bool matchFareBasis(std::string tktFareBasis,
                      const PaxTypeFare& paxTypeFare,
                      const std::vector<TravelSeg*>& travelSeg) const;

  bool paxTypeMappingMatch(const PaxType& paxChk, const PaxTypeCode paxRef) const;
  bool checkAccountCode(const TicketingFeesInfo* feeInfo) const;
  bool checkTktDesignator(const TicketingFeesInfo* feeInfo);
  bool checkSecurity(const TicketingFeesInfo* feeInfo) const;
  virtual bool roundTripJouneyType(void);
  virtual bool internationalJouneyType(void);
  virtual void getValidSegs();
  void setJourneyDestination();
  bool checkDuplicatedRecords(TicketingFeesInfo* feeI);
  virtual GlobalDirection getGlobalDirection(std::vector<TravelSeg*>& travelSegs) const;
  virtual uint32_t getTPM(const Loc& market1,
                          const Loc& market2,
                          const GlobalDirection& glbDir,
                          const DateTime& tvlDate) const;
  virtual bool isInLoc(const LocKey& locKey, const Loc& loc, const LocCode& zone) const;
  bool isStopOverPoint(const TravelSeg* travelSeg,
                       const TravelSeg* travelSegTo,
                       const FareUsage* fareUsage) const;
  bool isStopOver(const TravelSeg* travelSeg,
                  const TravelSeg* travelSegTo,
                  const FareUsage* fareUsage) const;

  void createDiag(void);
  void printDiagHeader(void);
  bool isProcessFOP(std::vector<FopBinNumber>& fopBin);
  void getFopBinNumber(std::vector<FopBinNumber>& fopBinNumberVector);
  void printDiagHeaderFopBinNumber(const std::vector<FopBinNumber>& fopBinNumber);
  void printFopBinNotFound();
  void printDiagHeader1(void);
  bool isFopBinMatch(TicketingFeesInfo& feeInfo, std::vector<FopBinNumber>& fopBins) const;
  bool addFopBinIfMatch(TicketingFeesInfo& feeInfo, std::vector<TicketingFeesInfo*>& fopMatched);
  void printCxrNotValid(const CarrierCode& cxr);
  void printCanNotCollect(const StatusS4Validation rc) const;
  void printDiagS4NotFound(void) const;
  void printDiagS4Info(const TicketingFeesInfo* feeInfo, const StatusS4Validation& status);
  void endDiag(void) const;
  bool diagActive(void) const { return _diag ? true : false; }

  bool isDiagSequenceMatch(const TicketingFeesInfo* feeInfo);
  bool isDiagServiceCodeMatch(const TicketingFeesInfo* feeInfo);
  void checkDiagDetailedRequest(const TicketingFeesInfo* feeInfo);
  void printDiagFirstPartDetailedRequest(const TicketingFeesInfo* feeInfo);
  void printDiagFinalPartDetailedRequest(const StatusS4Validation& rc);
  virtual void setCurrencyNoDec(TicketingFeesInfo& fee) const;
  typedef std::vector<TravelSeg*>::const_iterator TravelSegVecIC;
  const static Indicator T183_SCURITY_PRIVATE;
  bool isArunkPartOfSideTrip(const TravelSeg* travelSeg);

  bool isAnyNonCreditCardFOP() const;
  Indicator getCardType(const FopBinNumber& bin);
  void printDiagHeaderAmountFopResidualInd();
  void processFType(StatusS4Validation& rc,
                    bool isShopping,
                    Money& highestAmount,
                    TicketingFeesInfo* feeInfo,
                    std::vector<FopBinNumber>& fopBinNumberVector,
                    std::vector<TicketingFeesInfo*>& fopMatched);
  void processRTType(StatusS4Validation& rc, TicketingFeesInfo* feeInfo);
  void sortOBFeeVectors(std::vector<TicketingFeesInfo*>& sortVector);
  bool checkFeeTypesEnabled(const TicketingFeesInfo* feeInfo, StatusS4Validation& rc);

  bool isSvcTypeMatchCardType(const ServiceSubTypeCode& svcSubCode, Indicator cardType) const;
  void setCardType();
  void initializeFor2CreditCards(std::vector<TicketingFeesInfo*>& fopMatched) const;
  bool isTTypeSkip(TicketingFeesInfo* feeInfo);
  void adjustAnyCreditCardType(Indicator& cardType);
  void swapObFeesVecFor2CC(std::vector<TicketingFeesInfo*>& fopMatched);
};
} // tse namespace

