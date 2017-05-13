//-------------------------------------------------------------------
//
//  File:         PricingRequest.h
//  Created:
//  Authors:
//
//  Description:
//
//  Updates:
//          03/08/04 - VN - file created.
//          04/05/04 - Mike Carroll - Members added
//          08/13/04 - Gerald LePage - Modified taxOverride vector
//                     definition
//          02/14/05 - Quan Ta - Renamed from "Request" to PricingRequest,
//                     Request is now the new base class.
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

#include "Common/Assert.h"
#include "Common/DateTime.h"
#include "Common/TseConsts.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TypeConvert.h"
#include "DataModel/BrandedFaresData.h"
#include "DataModel/FlexFares/GroupsData.h"
#include "DataModel/TaxOverride.h"

#include <string>
#include <vector>
#include <map>

namespace tse
{

class Agent;
class AvailData;
class PricingTrx;
class ReservationData;
class TaxOverride;

struct DiscountAmount
{
  DiscountAmount() = default;
  DiscountAmount(MoneyAmount am, CurrencyCode curCode, int16_t startSegOrder, int16_t endSegOrder)
    : amount(am),
      currencyCode(curCode),
      startSegmentOrder(startSegOrder),
      endSegmentOrder(endSegOrder)
  {
  }

  MoneyAmount amount = 0;
  CurrencyCode currencyCode;
  int16_t startSegmentOrder = 0;
  int16_t endSegmentOrder = 0;
};

class Discounts
{
public:
  // For DP entry with segment select
  void addPercentage(int16_t segmentOrder, Percent percentage);
  void setPercentages(std::map<int16_t, Percent> percentages) { _percentages = std::move(percentages); }
  const std::map<int16_t, Percent>& getPercentages() const { return _percentages; }
  const Percent* getPercentage(const int16_t segmentOrder, const bool isMip) const;

  // For DA/PA entry with segment select
  void addAmount(size_t setNo,
                 const int16_t segmentOrder,
                 const MoneyAmount amount,
                 const std::string& currencyCode);

  void setAmounts(std::vector<DiscountAmount> amounts) { _amounts = std::move(amounts); }
  const std::vector<DiscountAmount>& getAmounts() const { return _amounts; }
  const DiscountAmount* getAmount(const int16_t startSegOrder) const;

  void clearAmountDiscounts() { _amounts.clear(); }
  void clearPercentageDiscounts() { _percentages.clear(); }

  bool isDPEntry() const;
  bool isPPEntry() const;
  bool isDPorPPentry() const { return !_percentages.empty(); }
  bool isDAEntry() const;
  bool isPAEntry() const;
  bool isDAorPAentry() const { return !_amounts.empty(); }

private:
  std::map<int16_t, Percent> _percentages;
  std::vector<DiscountAmount> _amounts;
};

enum class JumpCabinLogic
{
  ENABLED,
  ONLY_MIXED,
  DISABLED
};

enum class spValidator
{
  noSMV_noIEV,
  noSMV_IEV,
  SMV_IEV
};

class PricingRequest
{
  PricingRequest(const PricingRequest&) = delete;

public:
  static const int MAX_NUM_OF_ELEMENT_RCQ = 4;
  static const int MIN_LENGTH_OF_RCQ = 2;
  static const int MAX_LENGTH_OF_RCQ = 20;

protected:
  mutable std::map<uint8_t, Agent*> _ticketingAgents;
  DateTime _ticketingDT = DateTime::localTime(); // TKH
  std::string _corporateID; // COR
  std::string _accountCode; // ACI
  bool _multiAccCorpId = false;
  bool _expAccCorpId = false;
  std::vector<std::string> _incorrectCorpIdVec;
  std::vector<std::string> _corpIdVec;
  std::vector<std::string> _accCodeVec;
  int16_t _diagnosticNumber = 0; // DIA
  CarrierCode _validatingCarrier; // ITZ
  LocCode _ticketPointOverride; // ITU
  LocCode _salePointOverride; // ITV
  std::string _equivAmountCurrencyCode; // ITY
  double _equivAmountOverride = 0.0; // ITX
  double _rateAmountOverride = 0.0; // ITW
  double _secondRateAmountOverride = 0.0; // BSR
  DateTime _ticketDateOverride{0}; // ITT
  char _lowFareNoAvailability = 0; // ITF
  char _lowFareRequested = 0; // ITG
  char _exemptSpecificTaxes = 0; // ITH
  char _exemptAllTaxes = 0; // ITI
  char _priceNullDate = 0; // ITJ
  char _ticketByMail = 0; // ITL
  char _ticketEntry = 0; // ITM
  char _formOfPaymentCash = 'F'; // ITO
  char _formOfPaymentCheck = 'F'; // ITQ
  char _formOfPaymentCard = 'F'; // ITR
  char _formOfPaymentGTR = 'F'; // GTR
  std::string _contractNumber; // IT1
  int16_t _lengthATBFareCalc = 0; // ATB
  int16_t _numberTaxBoxes = 0; // TBX
  char _agencyTaxIsPercent = 0; // ATP
  char _refundPenalty = 0; // RPY
  char _priceNoAvailability = 0; // NCS
  char _ignoreTimeStamp = 0; // IGN
  char _exemptPFC = 0; // PFC
  bool _fareGroupRequested = false; // FGG for IS and MIP
  char _electronicTicket = 0; // ETK
  char _eTktOffAndNoOverride = 0; // ETK default=OFF, no override
  char _ptsOverride = 0; // PCB
  char _wpNettRequested = 0; // PCD
  char _wpSelRequested = 0; // PCE
  char _considerMultiAirport = 0;
  char _wpa50 = 0; // PXA
  char _wpas = 0; // PXB
  char _collectOBFee = false; // SEZ
  bool _collectRTypeOBFee = false; // SE2
  bool _collectTTypeOBFee = false; // SE3
  char _collectOCFees = false; // SEY
  bool _maxOBFeeOnly = false; // MOB
  bool _processVITAData = false; // SEV Process VITA data (default - F)
  char _lowFareNoRebook = false; // OCL
  std::vector<TaxOverride*> _taxOverride; // TVR, TAT
  std::vector<std::string> _taxIdExempted; // TFE
  LocCode _boardPoint; // BCC
  LocCode _offPoint; // OCC
  DateTime _requestedDepartureDT{0}; // DDH DTH
  CarrierCode _governingCarrier; // SGC
  Percent _percentOfLngCnxSolutions = 0; // Q6F

  std::vector<TaxCode> _taxRequestedInfo; // TAX

  std::vector<CarrierCode> _considerOnlyCarriers; // PCA
  std::vector<std::string> _diagArgType; // DI1
  std::vector<std::string> _diagArgData; // DI2
  std::map<int16_t, Percent> _discPercentages; // SDP with Segment order as key
  std::vector<DiscountAmount> _discAmounts; // SDA with Segment Select
  Discounts _discountsNew;
  std::vector<int16_t> _industryFareOverrides; // C-YY with Segment Select
  std::map<int16_t, CarrierCode> _governingCarrierOverrides; // C-xx with Segment Select
  std::map<int16_t, TktDesignator> _tktDesignator; // *Q/TKT DESIGNATOR with Segment Select
  std::map<int16_t, TktDesignator> _specifiedTktDesignator; // *Q/TKT DESIGNATOR - <Specified Ticket
                                                            // Designator>
  FopBinNumber _formOfPayment; // FOP
  FopBinNumber _secondFormOfPayment; // secondary FOP
  MoneyAmount _paymentAmountFop = 0; // PAT  payment amount for OB FOP
  bool _chargeResidualInd = true; // PRS  charge to residual/specified Amt
  bool _multiTicket = false; // MTO
  bool _multiTicketActive = false;
  // attribute BFR in element PRO in ShoppingRequest
  bool _brandedFaresRequest = false;
  bool _settlementTypesRequest = false;
  bool _parityBrandsPath = false;
  bool _dropResultsOnTimeout = false;
  bool _contextShoppingRequest = false;
  // attribute SRL in element PRO in ShoppingRequest - means Schedule Repeat Limit in IS in IBF
  // project
  uint16_t _scheduleRepeatLimit = 0;
  // attribute CAB in element PRO in ShoppingRequest
  bool _catchAllBucketRequest = false;
  // attribute BPL in element PRO in ShoppingRequest
  bool _cheapestWithLegParityPath = false;

  // attribute VCX in element PRO in ShoppingRequest
  bool _validatingCarrierRequest = true;
  // attribute DVL in element PRO in ShoppingRequest
  bool _alternateValidatingCarrierRequest = true;

  std::string _tourCode; // SHC

  bool _changeSoldoutBrand = false;
  bool _useCbsForNoFares = false;
  bool _processParityBrandsOverride = false;
  bool _allFlightsRepresented = false;
  bool _returnIllogicalFlights = false;
  bool _allFlightsData = false;

  // For supporting XformClientString
  std::string _fareClassCode;
  GlobalDirection _globalDirection = GlobalDirection::ZZ;

  // for DC entry
  CurrencyCode _fromCurrency;
  CurrencyCode _toCurrency;
  double _amountToBeConverted = 0.0;

  bool _addOnConstruction = false; // for WDD entry
  bool _rexEntry = false; // For QREX/ARP entries
  ReservationData* _reservationData = nullptr; // for ATAE
  bool _turnOffNoMatch = false; // PBY for no-match turned off
  bool _upSellEntry = false;
  std::vector<AvailData*> _availData; // Availability data passed within UpSell entry
  bool _brandedFareEntry = false; // branded Fare requested
  bool _originBasedRTPricing = false;
  BrandedFaresData _brandedFares;
  ExchRate _roeOverride = 0.0; // Q6B - ROE over ride for exchange pricing
  CurrencyNoDec _roeOverrideNoDec = 0; // number of decimal places entered in ROE override

  // WPA(S) has XM qualifier; the XM requires an addition action for the WPAS
  bool _wpaXM = false;
  ProcessingDirection _processingDirection = ProcessingDirection::ONEWAY;
  bool _owPricingRTTaxProcess = false;
  JumpCabinLogic _jumpCabinLogic = JumpCabinLogic::ENABLED;
  bool _jumpUpCabinAllowed = true;
  bool _suppressSumOfLocals = false; // do only thru fare markets on summ of locals path PRO/PTF
  bool _displayBaseline = false;          // BSL
  bool _specificAgencyText = false;       // SAT
  bool _baggageForMIP = false;            // FBS
  bool _useReducedConstructions = false;  // URC
  bool _isSimpleShoppingRQ = false;
  CarrierCode _cxrOverride{' '};
  SettlementPlanType _settlementMethod;
  uint16_t _majorSchemaVersion = 1;
  uint16_t _minorSchemaVersion = 0;
  uint16_t _revisionSchemaVersion = 0;
  uint16_t _maxFCsPerLeg = 0;

  flexFares::GroupsData _flexFaresGroups;

  //Structured Fare Request
  bool _isSFR = false;

  //Non-PreferredVC
  std::vector<CarrierCode> _nonPreferredVCs;
  //PreferredVC
    std::vector<CarrierCode> _preferredVCs;

  //Carrier without SP and IET check
  std::vector<CarrierCode> _spvCxrsCode;
  std::vector<NationCode> _spvCntyCode;
  spValidator _spvInd = spValidator::SMV_IEV;

  std::vector<FareRetailerCode> _rcqs;
  bool _prm = false;

private:
  std::string& equivAmountCurrencyCode() { return _equivAmountCurrencyCode; }
  const std::string& equivAmountCurrencyCode() const { return _equivAmountCurrencyCode; }

public:
  PricingRequest() = default;
  virtual ~PricingRequest() = default;
  void assign(PricingRequest& src) { *this = src; }

  //--------------------------------------------------------------------------
  // Accessors
  //--------------------------------------------------------------------------
  virtual Agent*& ticketingAgent()
  {
    // WARNING: This function is overridden in AncRequest and changes behavior:
    // it can return one of the ticketing agents or the checkin agent according
    // to how it is configured using AncRequest::setActiveAgent
    // FIXME Refactor this ASAP
    return _ticketingAgents[0];
  };

  virtual const Agent* ticketingAgent() const
  {
    // WARNING: This function is overridden in AncRequest and changes behavior:
    // it can return one of the ticketing agents or the checkin agent according
    // to how it is configured using AncRequest::setActiveAgent
    // FIXME Refactor this ASAP
    return _ticketingAgents[0];
  };

  virtual DateTime& ticketingDT() { return _ticketingDT; }
  virtual const DateTime& ticketingDT() const { return _ticketingDT; }

  bool& isMultiAccCorpId() { return _multiAccCorpId; }
  const bool& isMultiAccCorpId() const { return _multiAccCorpId; }

  bool& isExpAccCorpId() { return _expAccCorpId; }
  const bool& isExpAccCorpId() const { return _expAccCorpId; }

  virtual std::string& corporateID() { return _corporateID; }
  virtual const std::string& corporateID() const { return _corporateID; }

  virtual std::string& accountCode() { return _accountCode; }
  virtual const std::string& accountCode() const { return _accountCode; }

  std::vector<std::string>& corpIdVec() { return _corpIdVec; }
  const std::vector<std::string>& corpIdVec() const { return _corpIdVec; }

  std::vector<std::string>& incorrectCorpIdVec() { return _incorrectCorpIdVec; }
  const std::vector<std::string>& incorrectCorpIdVec() const { return _incorrectCorpIdVec; }

  std::vector<std::string>& accCodeVec() { return _accCodeVec; }
  const std::vector<std::string>& accCodeVec() const { return _accCodeVec; }

  int16_t& diagnosticNumber() { return _diagnosticNumber; }
  const int16_t& diagnosticNumber() const { return _diagnosticNumber; }

  virtual CarrierCode& validatingCarrier() { return _validatingCarrier; }
  virtual const CarrierCode& validatingCarrier() const { return _validatingCarrier; }

  virtual LocCode& ticketPointOverride() { return _ticketPointOverride; }
  virtual const LocCode& ticketPointOverride() const { return _ticketPointOverride; }

  virtual LocCode& salePointOverride() { return _salePointOverride; }
  virtual const LocCode& salePointOverride() const { return _salePointOverride; }

  double& equivAmountOverride() { return _equivAmountOverride; }
  const double& equivAmountOverride() const { return _equivAmountOverride; }

  double& rateAmountOverride() { return _rateAmountOverride; }
  const double& rateAmountOverride() const { return _rateAmountOverride; }

  ExchRate& roeOverride() { return _roeOverride; }
  const ExchRate& roeOverride() const { return _roeOverride; }

  CurrencyNoDec& roeOverrideNoDec() { return _roeOverrideNoDec; }
  const CurrencyNoDec& roeOverrideNoDec() const { return _roeOverrideNoDec; }

  double& secondRateAmountOverride() { return _secondRateAmountOverride; }
  const double& secondRateAmountOverride() const { return _secondRateAmountOverride; }

  DateTime& ticketDateOverride() { return _ticketDateOverride; }
  const DateTime& ticketDateOverride() const { return _ticketDateOverride; }

  char& lowFareNoAvailability() { return _lowFareNoAvailability; }
  const char& lowFareNoAvailability() const { return _lowFareNoAvailability; }
  const bool isLowFareNoAvailability() const
  {
    return TypeConvert::pssCharToBool(_lowFareNoAvailability);
  }

  char& lowFareRequested() { return _lowFareRequested; }
  const char& lowFareRequested() const { return _lowFareRequested; }
  const bool isLowFareRequested() const { return TypeConvert::pssCharToBool(_lowFareRequested); }

  char& lowFareNoRebook() { return _lowFareNoRebook; }
  const char& lowFareNoRebook() const { return _lowFareNoRebook; }
  const bool isLowFareNoRebook() const { return TypeConvert::pssCharToBool(_lowFareNoRebook); }

  char& exemptSpecificTaxes() { return _exemptSpecificTaxes; }
  const char& exemptSpecificTaxes() const { return _exemptSpecificTaxes; }
  const bool isExemptSpecificTaxes() const
  {
    return TypeConvert::pssCharToBool(_exemptSpecificTaxes);
  }

  char& exemptAllTaxes() { return _exemptAllTaxes; }
  const char& exemptAllTaxes() const { return _exemptAllTaxes; }
  const bool isExemptAllTaxes() const { return TypeConvert::pssCharToBool(_exemptAllTaxes); }

  char& priceNullDate() { return _priceNullDate; }
  const char& priceNullDate() const { return _priceNullDate; }
  const bool isPriceNullDate() const { return TypeConvert::pssCharToBool(_priceNullDate); }

  char& ticketByMail() { return _ticketByMail; }
  const char& ticketByMail() const { return _ticketByMail; }
  const bool isTicketByMail() const { return TypeConvert::pssCharToBool(_ticketByMail); }

  char& ticketEntry() { return _ticketEntry; }
  const char& ticketEntry() const { return _ticketEntry; }
  const bool isTicketEntry() const { return TypeConvert::pssCharToBool(_ticketEntry); }

  char& formOfPaymentCash() { return _formOfPaymentCash; }
  const char& formOfPaymentCash() const { return _formOfPaymentCash; }
  const bool isFormOfPaymentCash() const { return TypeConvert::pssCharToBool(_formOfPaymentCash); }

  char& formOfPaymentCheck() { return _formOfPaymentCheck; }
  const char& formOfPaymentCheck() const { return _formOfPaymentCheck; }

  const bool isFormOfPaymentCheck() const
  {
    return TypeConvert::pssCharToBool(_formOfPaymentCheck);
  }

  char& formOfPaymentCard() { return _formOfPaymentCard; }
  const char& formOfPaymentCard() const { return _formOfPaymentCard; }
  const bool isFormOfPaymentCard() const { return TypeConvert::pssCharToBool(_formOfPaymentCard); }

  char& formOfPaymentGTR() { return _formOfPaymentGTR; }
  const char& formOfPaymentGTR() const { return _formOfPaymentGTR; }
  const bool isFormOfPaymentGTR() const { return TypeConvert::pssCharToBool(_formOfPaymentGTR); }

  std::string& contractNumber() { return _contractNumber; }
  const std::string& contractNumber() const { return _contractNumber; }

  int16_t& lengthATBFareCalc() { return _lengthATBFareCalc; }
  const int16_t& lengthATBFareCalc() const { return _lengthATBFareCalc; }

  int16_t& numberTaxBoxes() { return _numberTaxBoxes; }
  const int16_t& numberTaxBoxes() const { return _numberTaxBoxes; }

  char& agencyTaxIsPercent() { return _agencyTaxIsPercent; }
  const char& agencyTaxIsPercent() const { return _agencyTaxIsPercent; }

  char& refundPenalty() { return _refundPenalty; }
  const char& refundPenalty() const { return _refundPenalty; }

  char& priceNoAvailability() { return _priceNoAvailability; }

  const char& priceNoAvailability() const { return _priceNoAvailability; }

  bool& fareGroupRequested() { return _fareGroupRequested; }
  const bool& fareGroupRequested() const { return _fareGroupRequested; }

  char& ignoreTimeStamp() { return _ignoreTimeStamp; }
  const char& ignoreTimeStamp() const { return _ignoreTimeStamp; }

  char& exemptPFC() { return _exemptPFC; }
  const char& exemptPFC() const { return _exemptPFC; }
  const bool isExemptPFC() const { return TypeConvert::pssCharToBool(_exemptPFC); }

  char& electronicTicket() { return _electronicTicket; }
  const char& electronicTicket() const { return _electronicTicket; }
  const bool isElectronicTicket() const { return TypeConvert::pssCharToBool(_electronicTicket); }

  char& eTktOffAndNoOverride() { return _eTktOffAndNoOverride; }
  const char& eTktOffAndNoOverride() const { return _eTktOffAndNoOverride; }
  const bool isETktOffAndNoOverride() const
  {
    return TypeConvert::pssCharToBool(_eTktOffAndNoOverride);
  }

  char& considerMultiAirport() { return _considerMultiAirport; }
  const char& considerMultiAirport() const { return _considerMultiAirport; }

  std::vector<TaxOverride*>& taxOverride() { return _taxOverride; }
  const std::vector<TaxOverride*>& taxOverride() const { return _taxOverride; }

  std::vector<std::string>& taxIdExempted() { return _taxIdExempted; }
  const std::vector<std::string>& taxIdExempted() const { return _taxIdExempted; }

  LocCode& boardPoint() { return _boardPoint; }
  const LocCode& boardPoint() const { return _boardPoint; }

  LocCode& offPoint() { return _offPoint; }
  const LocCode& offPoint() const { return _offPoint; }

  DateTime& requestedDepartureDT() { return _requestedDepartureDT; }
  const DateTime& requestedDepartureDT() const { return _requestedDepartureDT; }

  CarrierCode& governingCarrier() { return _governingCarrier; }
  const CarrierCode& governingCarrier() const { return _governingCarrier; }

  Percent& percentOfLngCnxSolutions() { return _percentOfLngCnxSolutions; }
  const Percent& percentOfLngCnxSolutions() const { return _percentOfLngCnxSolutions; }

  std::vector<CarrierCode>& considerOnlyCarriers() { return _considerOnlyCarriers; }
  const std::vector<CarrierCode>& considerOnlyCarriers() const { return _considerOnlyCarriers; }

  std::vector<std::string>& diagArgType() { return _diagArgType; }
  const std::vector<std::string>& diagArgType() const { return _diagArgType; }

  std::vector<std::string>& diagArgData() { return _diagArgData; }
  const std::vector<std::string>& diagArgData() const { return _diagArgData; }

  // For supporting XformClientString
  std::string& fareClassCode() { return _fareClassCode; }
  const std::string& fareClassCode() const { return _fareClassCode; }

  GlobalDirection& globalDirection() { return _globalDirection; }
  const GlobalDirection& globalDirection() const { return _globalDirection; }

  char& ptsOverride() { return _ptsOverride; }
  const char& ptsOverride() const { return _ptsOverride; }
  const bool isPtsOverride() const { return TypeConvert::pssCharToBool(_ptsOverride); }

  FopBinNumber& formOfPayment() { return _formOfPayment; }
  const FopBinNumber& formOfPayment() const { return _formOfPayment; }

  FopBinNumber& secondFormOfPayment() { return _secondFormOfPayment; }
  const FopBinNumber& secondFormOfPayment() const { return _secondFormOfPayment; }

  MoneyAmount& paymentAmountFop() { return _paymentAmountFop; }
  const MoneyAmount& paymentAmountFop() const { return _paymentAmountFop; }

  bool& chargeResidualInd() { return _chargeResidualInd; }
  const bool& chargeResidualInd() const { return _chargeResidualInd; }

  bool& isMultiTicketRequest() { return _multiTicket; }
  const bool& isMultiTicketRequest() const { return _multiTicket; }

  bool& multiTicketActive() { return _multiTicketActive; }
  const bool& multiTicketActive() const { return _multiTicketActive; }

  // JAL/AXESS  entries

  char& wpNettRequested() { return _wpNettRequested; }
  const char& wpNettRequested() const { return _wpNettRequested; }
  const bool isWpNettRequested() const { return TypeConvert::pssCharToBool(_wpNettRequested); }

  char& wpSelRequested() { return _wpSelRequested; }
  const char& wpSelRequested() const { return _wpSelRequested; }
  const bool isWpSelRequested() const { return TypeConvert::pssCharToBool(_wpSelRequested); }

  // WPA 50
  char& wpa50() { return _wpa50; }
  const char& wpa50() const { return _wpa50; }
  const bool isWpa50() const { return TypeConvert::pssCharToBool(_wpa50); }

  // WPAS
  char& wpas() { return _wpas; }
  const char& wpas() const { return _wpas; }
  const bool isWpas() const { return TypeConvert::pssCharToBool(_wpas); }

  // XM for WPA(S)
  bool& wpaXm() { return _wpaXM; }
  const bool& wpaXm() const { return _wpaXM; }

  // OB Fees
  char& collectOBFee() { return _collectOBFee; }
  const char& collectOBFee() const { return _collectOBFee; }
  const bool isCollectOBFee() const { return TypeConvert::pssCharToBool(_collectOBFee); }

  // R Type OB Fees
  void setCollectRTypeOBFee(bool value) { _collectRTypeOBFee = value; }
  bool& getCollectRTypeOBFee() { return _collectRTypeOBFee; }
  const bool isCollectRTypeOBFee() const { return _collectRTypeOBFee; }

  // T Type OB Fees
  void setCollectTTypeOBFee(bool value) { _collectTTypeOBFee = value; }
  bool& getCollectTTypeOBFee() { return _collectTTypeOBFee; }
  const bool isCollectTTypeOBFee() const { return _collectTTypeOBFee; }

  // Return only Max OB Fees for Virtual Payments
  void setReturnMaxOBFeeOnly(bool value) { _maxOBFeeOnly = value; }
  const bool returnMaxOBFeeOnly() const { return _maxOBFeeOnly; }

  // OC Fees
  char& collectOCFees() { return _collectOCFees; }
  const char& collectOCFees() const { return _collectOCFees; }
  const bool isCollectOCFees() const { return TypeConvert::pssCharToBool(_collectOCFees); }

  // VITA processing
  bool& processVITAData() { return _processVITAData; }
  const bool& processVITAData() const { return _processVITAData; }

  // for DC entry
  CurrencyCode& fromCurrency() { return _fromCurrency; }
  const CurrencyCode& fromCurrency() const { return _fromCurrency; }

  // for DC entry
  CurrencyCode& toCurrency() { return _toCurrency; }
  const CurrencyCode& toCurrency() const { return _toCurrency; }

  // for DC entry
  double& amountToBeConverted() { return _amountToBeConverted; }
  const double& amountToBeConverted() const { return _amountToBeConverted; }

  // For DP entry with segment select
  void setDiscountPercentages(std::map<int16_t, Percent> discountPercentages) { _discPercentages = discountPercentages; }
  const std::map<int16_t, Percent>& getDiscountPercentages() const { return _discPercentages; }
  std::map<int16_t, Percent>& discPercentages() { return _discPercentages; }
  void setDiscountPercentagesNew(std::map<int16_t, Percent> discountPercentages) { discountsNew().setPercentages(discountPercentages); }
  const std::map<int16_t, Percent>& getDiscountPercentagesNew() const { return discountsNew().getPercentages(); }
  const Percent discountPercentage(const int16_t segmentOrder) const;
  const Percent* discountPercentageNew(const int16_t segmentOrder, const PricingTrx& trx) const;
  bool isDPEntry() const { return !_discPercentages.empty(); }
  bool isDPEntryNew() const { return discountsNew().isDPEntry(); }
  bool isPPEntry() const { return discountsNew().isPPEntry(); }
  bool isDAorPAentry() const { return discountsNew().isDAorPAentry(); }
  bool isDPorPPentry() const { return discountsNew().isDPorPPentry(); }

  // For DA entry with segment select
  void addDiscAmount(const int16_t setNo,
                     const int16_t segmentOrder,
                     const MoneyAmount discAmount,
                     const std::string& discCurrencyCode);
  // For DA/PA entry with segment select
  void addDiscountAmountNew(size_t setNo,
                            const int16_t segmentOrder,
                            const MoneyAmount discAmount,
                            const std::string& discCurrencyCode)
  {
    discountsNew().addAmount(setNo, segmentOrder, discAmount, discCurrencyCode);
  }

  void addDiscountPercentage(int16_t segmentOrder,
                             Percent discountPercentage)
  {
    discountsNew().addPercentage(segmentOrder, discountPercentage);
  }

  void setDiscountAmounts(std::vector<DiscountAmount> discountAmounts) { _discAmounts = discountAmounts; }
  std::vector<DiscountAmount> getDiscountAmounts() const { return _discAmounts; }
  const DiscountAmount* discountAmount(const int16_t startSegOrder) const;

  void setDiscountAmountsNew(std::vector<DiscountAmount> discountAmounts) { discountsNew().setAmounts(discountAmounts); }
  const std::vector<DiscountAmount>& getDiscountAmountsNew() const { return discountsNew().getAmounts(); }
  const DiscountAmount* discountAmountNew(const int16_t startSegOrder) const { return discountsNew().getAmount(startSegOrder); }

  bool isDAEntry() const { return !getDiscountAmounts().empty(); }
  bool isDAEntryNew() const { return discountsNew().isDAEntry(); }
  bool isPAEntry() const { return discountsNew().isPAEntry(); }

  Discounts& discountsNew() { return _discountsNew; }
  const Discounts& discountsNew() const { return _discountsNew; }

  // For WPC-YY entry with segment select
  std::vector<int16_t>& industryFareOverrides() { return _industryFareOverrides; }
  const std::vector<int16_t>& industryFareOverrides() const { return _industryFareOverrides; }
  bool industryFareOverride(const int16_t segmentOrder);
  bool isIndustryFareOverrideEntry() { return !_industryFareOverrides.empty(); }

  // For WPC-xx entry with segment select
  std::map<int16_t, CarrierCode>& governingCarrierOverrides() { return _governingCarrierOverrides; }
  const std::map<int16_t, CarrierCode>& governingCarrierOverrides() const
  {
    return _governingCarrierOverrides;
  }

  const CarrierCode governingCarrierOverride(const int16_t segmentOrder) const;
  bool isGoverningCarrierOverrideEntry() const { return !_governingCarrierOverrides.empty(); }

  // *Q/TKT DESIGNATOR with Segment Select
  virtual std::map<int16_t, TktDesignator>& tktDesignator() { return _tktDesignator; }
  virtual const std::map<int16_t, TktDesignator>& tktDesignator() const { return _tktDesignator; }
  virtual const TktDesignator tktDesignator(const int16_t segmentOrder) const;
  virtual bool isTktDesignatorEntry() { return !_tktDesignator.empty(); }

  // *Q/<TKT DESIGNATOR>-<Specified Tkt Designator> with Segment Select
  std::map<int16_t, TktDesignator>& specifiedTktDesignator() { return _specifiedTktDesignator; }
  const std::map<int16_t, TktDesignator>& specifiedTktDesignator() const
  {
    return _specifiedTktDesignator;
  }
  const TktDesignator specifiedTktDesignator(const int16_t segmentOrder) const;
  bool isSpecifiedTktDesignatorEntry() { return !_specifiedTktDesignator.empty(); }

  // For WDD entry
  bool& addOnConstruction() { return _addOnConstruction; }
  const bool& addOnConstruction() const { return _addOnConstruction; }

  // For QREX/ARP entry
  bool& rexEntry() { return _rexEntry; }
  const bool& rexEntry() const { return _rexEntry; }

  ReservationData*& reservationData() { return _reservationData; }
  const ReservationData* reservationData() const { return _reservationData; }

  bool& turnOffNoMatch() { return _turnOffNoMatch; }
  const bool& turnOffNoMatch() const { return _turnOffNoMatch; }

  bool& upSellEntry() { return _upSellEntry; }
  const bool& upSellEntry() const { return _upSellEntry; }

  std::vector<AvailData*>& availData() { return _availData; }
  const std::vector<AvailData*>& availData() const { return _availData; }

  bool& originBasedRTPricing() { return _originBasedRTPricing; }
  const bool& originBasedRTPricing() const { return _originBasedRTPricing; }

  // branded fare
  bool isAdvancedBrandProcessing() const
  {
    return _brandedFareEntry && !_brandedFares.fareSecondaryBookingCode().empty();
  }

  bool& brandedFareEntry() { return _brandedFareEntry; }
  const bool& brandedFareEntry() const { return _brandedFareEntry; }

  const uint16_t getBrandedFareSize() const { return _brandedFares.getSize(); }

  Code<10>& brandId(const uint16_t index = 0) { return _brandedFares.brandId(index); }
  const Code<10>& brandId(const uint16_t index = 0) const { return _brandedFares.brandId(index); }

  std::string& programId(const uint16_t index = 0) { return _brandedFares.programId(index); }
  const std::string& programId(const uint16_t index = 0) const
  {
    return _brandedFares.programId(index);
  }

  std::vector<BookingCode>& brandedFareBookingCode(const uint16_t index = 0)
  {
    return _brandedFares.fareBookingCode(index);
  }
  const std::vector<BookingCode>& brandedFareBookingCode(const uint16_t index = 0) const
  {
    return _brandedFares.fareBookingCode(index);
  }

  std::vector<BookingCode>& brandedFareSecondaryBookingCode(const uint16_t index = 0)
  {
    return _brandedFares.fareSecondaryBookingCode(index);
  }
  const std::vector<BookingCode>& brandedFareSecondaryBookingCode(const uint16_t index = 0) const
  {
    return _brandedFares.fareSecondaryBookingCode(index);
  }

  void brandedFareAllBookingCode(const uint16_t index, std::vector<BookingCode>& bkg) const;

  std::vector<BookingCode>& brandedFareBookingCodeExclude(const uint16_t index = 0)
  {
    return _brandedFares.fareBookingCodeExclude(index);
  }
  const std::vector<BookingCode>& brandedFareBookingCodeExclude(const uint16_t index = 0) const
  {
    return _brandedFares.fareBookingCodeExclude(index);
  }

  std::vector<BookingCode>& brandedFareSecondaryBookingCodeExclude(const uint16_t index = 0)
  {
    return _brandedFares.fareSecondaryBookingCodeExclude(index);
  }
  const std::vector<BookingCode>&
  brandedFareSecondaryBookingCodeExclude(const uint16_t index = 0) const
  {
    return _brandedFares.fareSecondaryBookingCodeExclude(index);
  }

  std::map<BookingCode, char>& brandedFareBookingCodeData(const uint16_t index = 0)
  {
    return _brandedFares.fareBookingCodeData(index);
  }
  const std::map<BookingCode, char>& brandedFareBookingCodeData(const uint16_t index = 0) const
  {
    return _brandedFares.fareBookingCodeData(index);
  }

  std::map<BookingCode, char>& brandedFareSecondaryBookingCodeData(const uint16_t index = 0)
  {
    return _brandedFares.fareSecondaryBookingCodeData(index);
  }
  const std::map<BookingCode, char>&
  brandedFareSecondaryBookingCodeData(const uint16_t index = 0) const
  {
    return _brandedFares.fareSecondaryBookingCodeData(index);
  }
  std::vector<BookingCode>& brandedFBCDispDiag499(const uint16_t index = 0)
  {
    return _brandedFares.fareBookingCodeDispDiag499(index);
  }
  const std::vector<BookingCode>& brandedFBCDispDiag499(const uint16_t index = 0) const
  {
    return _brandedFares.fareBookingCodeDispDiag499(index);
  }

  std::vector<FareClassCode>& brandedFareFamily(const uint16_t index = 0)
  {
    return _brandedFares.fareFamily(index);
  }
  const std::vector<FareClassCode>& brandedFareFamily(const uint16_t index = 0) const
  {
    return _brandedFares.fareFamily(index);
  }

  std::vector<FareClassCode>& brandedFareFamilyExclude(const uint16_t index = 0)
  {
    return _brandedFares.fareFamilyExclude(index);
  }
  const std::vector<FareClassCode>& brandedFareFamilyExclude(const uint16_t index = 0) const
  {
    return _brandedFares.fareFamilyExclude(index);
  }

  std::map<FareClassCode, char>& brandedFareFamilyData(const uint16_t index = 0)
  {
    return _brandedFares.fareFamilyData(index);
  }
  const std::map<FareClassCode, char>& brandedFareFamilyData(const uint16_t index = 0) const
  {
    return _brandedFares.fareFamilyData(index);
  }

  std::vector<FareClassCode>& brandedFareBasisCode(const uint16_t index = 0)
  {
    return _brandedFares.fareBasisCode(index);
  }
  const std::vector<FareClassCode>& brandedFareBasisCode(const uint16_t index = 0) const
  {
    return _brandedFares.fareBasisCode(index);
  }

  std::vector<FareClassCode>& brandedFareBasisCodeExclude(const uint16_t index = 0)
  {
    return _brandedFares.fareBasisCodeExclude(index);
  }
  const std::vector<FareClassCode>& brandedFareBasisCodeExclude(const uint16_t index = 0) const
  {
    return _brandedFares.fareBasisCodeExclude(index);
  }

  std::map<FareClassCode, char>& brandedFareBasisCodeData(const uint16_t index = 0)
  {
    return _brandedFares.fareBasisCodeData(index);
  }
  const std::map<FareClassCode, char>& brandedFareBasisCodeData(const uint16_t index = 0) const
  {
    return _brandedFares.fareBasisCodeData(index);
  }

  const BrandedFaresData& getBrandedFaresData() const { return _brandedFares; }

  bool hasConsistenBrandedBookingCodes() const { return _brandedFares.hasConsistentBookingCodes(); }

  void setBrandedFaresRequest(bool value) { _brandedFaresRequest = value; }
  bool isBrandedFaresRequest() const { return _brandedFaresRequest; }

  void setSettlementTypesRequest(bool value) { _settlementTypesRequest = value; }
  bool isSettlementTypesRequest() const { return _settlementTypesRequest; }

  void setParityBrandsPath(bool value) { _parityBrandsPath = value; }
  bool isParityBrandsPath() const { return _parityBrandsPath; }

  void setDropResultsOnTimeout(bool value) { _dropResultsOnTimeout = value; }
  bool isDropResultsOnTimeout() const { return _dropResultsOnTimeout; }

  void setContextShoppingRequest() { _contextShoppingRequest = true; }

  bool isContextShoppingRequest() const
  {
    return isBrandedFaresRequest() && _contextShoppingRequest;
  }

  bool getContextShoppingRequestFlag() const { return _contextShoppingRequest; }

  void setScheduleRepeatLimit(uint16_t value) { _scheduleRepeatLimit = value; }
  uint16_t getScheduleRepeatLimit() const { return _scheduleRepeatLimit; }

  void setCatchAllBucketRequest(bool value) { _catchAllBucketRequest = value; }
  bool isCatchAllBucketRequest() const { return _catchAllBucketRequest; }

  void setCheapestWithLegParityPath(bool value) { _cheapestWithLegParityPath = value; }
  bool isCheapestWithLegParityPath() const { return _cheapestWithLegParityPath; }

  void setValidatingCarrierRequest(bool value) { _validatingCarrierRequest = value; }
  bool isValidatingCarrierRequest() const { return _validatingCarrierRequest; }

  void setAlternateValidatingCarrierRequest(bool value) { _alternateValidatingCarrierRequest = value; }
  bool isAlternateValidatingCarrierRequest() const { return _alternateValidatingCarrierRequest; }

  void setTourCode(const std::string& tourCode) { _tourCode = tourCode; }
  const std::string& getTourCode() const { return _tourCode; }

  void setChangeSoldoutBrand(bool value) { _changeSoldoutBrand = value; }
  bool isChangeSoldoutBrand() const { return _changeSoldoutBrand; }

  void setUseCbsForNoFares(bool value) { _useCbsForNoFares = value;}
  bool isUseCbsForNoFares() const { return _useCbsForNoFares; }

  void setProcessParityBrandsOverride(bool value) { _processParityBrandsOverride = value;}
  bool isProcessParityBrandsOverride() const { return _processParityBrandsOverride; }

  void setAllFlightsRepresented(bool value) { _allFlightsRepresented = value; }
  bool isAllFlightsRepresented() const { return _allFlightsRepresented; }

  void setReturnIllogicalFlights(bool value) { _returnIllogicalFlights = value; }
  bool isReturnIllogicalFlights() const { return _returnIllogicalFlights; }

  void setAllFlightsData(bool value) { _allFlightsData = value; }
  bool isAllFlightsData() const { return _allFlightsData; }

  void setSpecificAgencyText(bool value) { _specificAgencyText = value; }
  bool isSpecificAgencyText() const { return _specificAgencyText; }

  void setBaggageForMIP(bool value) { _baggageForMIP = value; }
  bool isProcessBaggageForMIP() const { return _baggageForMIP; }

  void setUseReducedConstructions(bool value) { _useReducedConstructions = value; }
  bool isUseReducedConstructions() const { return _useReducedConstructions; }

  // Flex Fares Groups
  const flexFares::GroupsData& getFlexFaresGroupsData() const { return _flexFaresGroups; }
  flexFares::GroupsData& getMutableFlexFaresGroupsData() { return _flexFaresGroups; }

  // ASE processing direction
  ProcessingDirection& processingDirection() { return _processingDirection; }
  const ProcessingDirection& processingDirection() const { return _processingDirection; }

  bool& owPricingRTTaxProcess() { return _owPricingRTTaxProcess; }
  const bool& owPricingRTTaxProcess() const { return _owPricingRTTaxProcess; }

  JumpCabinLogic getJumpCabinLogic() const { return _jumpCabinLogic; }
  void setJumpCabinLogic(JumpCabinLogic value) { _jumpCabinLogic = value; }

  void setjumpUpCabinAllowed() { _jumpUpCabinAllowed = false; }
  bool isjumpUpCabinAllowed() const { return _jumpUpCabinAllowed; }

  void setSuppressSumOfLocals(bool value) { _suppressSumOfLocals = value; }
  bool isSuppressedSumOfLocals() const { return _suppressSumOfLocals; }

  void setDisplayBaseline(bool value) { _displayBaseline = value; }
  bool isDisplayBaseline() const { return _displayBaseline; }

  CarrierCode& cxrOverride() { return _cxrOverride; }
  const CarrierCode& cxrOverride() const { return _cxrOverride; }

  uint16_t& majorSchemaVersion() { return _majorSchemaVersion; }
  uint16_t majorSchemaVersion() const { return _majorSchemaVersion; }

  uint16_t& minorSchemaVersion() { return _minorSchemaVersion; }
  uint16_t minorSchemaVersion() const { return _minorSchemaVersion; }

  const SettlementPlanType& getSettlementMethod() const { return _settlementMethod; }
  void setSettlementMethod(SettlementPlanType settlementMethod) { _settlementMethod = settlementMethod; }

  uint16_t& revisionSchemaVersion() { return _revisionSchemaVersion; }
  uint16_t revisionSchemaVersion() const { return _revisionSchemaVersion; }

  const std::vector<TaxCode>& getTaxRequestedInfo() const { return _taxRequestedInfo; }
  void setTaxRequestedInfo(std::vector<TaxCode>&& src) { _taxRequestedInfo=std::move(src); }

  bool checkSchemaVersion(short major, short minor, short revision) const;

  void markAsSFR() { _isSFR = true; }
  bool isSFR() const { return _isSFR; }

  static
  std::string
  jumpCabinLogicStatusToString(const JumpCabinLogic status);

  //Non-PreferredVC
  std::vector<CarrierCode>& nonPreferredVCs() { return _nonPreferredVCs; }
  const std::vector<CarrierCode>& nonPreferredVCs() const { return _nonPreferredVCs; }
  //PreferredVC
    std::vector<CarrierCode>& preferredVCs() { return _preferredVCs; }
    const std::vector<CarrierCode>& preferredVCs() const { return _preferredVCs; }

  spValidator& spvInd() { return _spvInd; }
  const spValidator spvInd() const { return _spvInd; }

  std::vector<CarrierCode>& spvCxrsCode() { return _spvCxrsCode; }
  const std::vector<CarrierCode>& spvCxrsCode() const { return _spvCxrsCode; }

  std::vector<NationCode>& spvCntyCode() { return _spvCntyCode; }
  const std::vector<NationCode>& spvCntyCode() const { return _spvCntyCode; }

  uint8_t setRCQValues(const std::string& xmlString);
  const std::vector<FareRetailerCode>& rcqValues() const { return _rcqs; }

  void setPRM(bool value) { _prm = value; }
  bool prmValue() const { return _prm; }

  bool isMatch(const FareRetailerCode& reailerCode) const;

  bool isNSPCxr(const CarrierCode& mktCxr) const;

  void setSimpleShoppingRQ(bool value) { _isSimpleShoppingRQ = value; }
  bool isSimpleShoppingRQ() const { return _isSimpleShoppingRQ; }

  void setMaxFCsPerLeg(uint16_t maxFCsPerLeg) { _maxFCsPerLeg = maxFCsPerLeg; }
  uint16_t getMaxFCsPerLeg() const { return _maxFCsPerLeg; }

};

} // tse

