//----------------------------------------------------------------------------
//  File:           TaxUtility.h
//  Created:        27/07/2009
//  Authors:        Piotr Badarycz
//
//  Description: Utilitarian functions.
//
//  Updates:
//
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
//----------------------------------------------------------------------------

#pragma once

#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/TaxResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Taxes/Common/TaxOnTaxFilter.h"

#include <string>

namespace tse
{
class FareUsage;
class Loc;
class PricingTrx;
class TaxResponse;
class TravelSeg;

namespace utc
{
extern const std::string FALLBACK;

std::string
getSpecConfigParameter(PricingTrx& trx,
                       const std::string& configName,
                       const std::string& paramName,
                       const DateTime& ticketDate);
std::string
getSpecConfigParameter(const std::vector<TaxSpecConfigReg*>* tscv,
                       const std::string& paramName,
                       const DateTime& ticketDate);

std::string
partitionId(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
doesApplyOnlyOriginalTicket(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

std::string
specialTransitAirport(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

std::string
frCorsicaDefinition(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
std::string
frCoastDefinition(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

uint16_t
minMilage(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

uint16_t
maxMilage(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

uint16_t
specialTaxRoundCentNumber(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
uint16_t
specialTaxRoundCentNumber(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

bool
fareClassToAmount(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
isMirrorStopLogicDisabled(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
fareClassCheckAllFares(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
isStartEndWithinSpec(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
isTransitViaLocDisabled(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
validateExemptionTable(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
skipExemption(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
useWithinLastLoc2Logic(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
furthestFareBreak1DayRt(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

bool
isTaxExemptForArunk(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
bool
isTaxExemptForArunk(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

bool
isChangeFeeSeq(PricingTrx& trx, const TaxCodeReg& taxCodeReg, const DateTime& ticketingDT);
bool
isChangeFeeSeq(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

std::string
baseTaxCode(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
std::string
baseTaxCode(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

bool
isMatchOrigTicket(PricingTrx& trx, const TaxCodeReg& taxCodeReg, const DateTime& ticketDate);

bool
validateSeqForPercentageTax(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
bool
validateSeqForPercentageTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
validateSeqIssueFix(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
bool
validateSeqIssueFix(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
skipTaxOnTaxIfNoFare(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
isTaxOnTaxSeqMatch(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
isTaxOnDomTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
isAySPN1800OptionB(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
shouldCheckCityMirror(const DateTime& date,
                      const std::vector<TaxSpecConfigReg*>* const taxSpecConfigRegs);
bool
isLandToAirStopover(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    const std::vector<TaxSpecConfigReg*>* tscv);

bool
isTaxOnOC(PricingTrx& trx, TaxCodeReg& taxCodeReg);
bool
isTaxOnChangeFee(PricingTrx& trx, TaxCodeReg& taxCodeReg, const DateTime& ticketingDT);
bool
isTaxOnChangeFee(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
isZZTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

bool
isAYTax_FM_MH_NationInt(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
isAYTaxCapOption(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
std::string
getAYTaxIntDomIntOption(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
treatBusTrainAsNonAir(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
shouldExemptIfAllRural(const PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);
bool
transitValidatorSkipArunk(PricingTrx& trx, const TaxCodeReg& taxCodeReg);
bool
transitValidatorMultiCityIsStop(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

uint16_t
numberOfSegments(PricingTrx& trx, const TaxCodeReg& taxCodeReg);

class OverrideJourneyType final
{
public:
  OverrideJourneyType() = default;
  OverrideJourneyType(PricingTrx& trx, TaxCodeReg& taxCodeReg);

  bool isOverrideJourneyType() { return !_locCode.empty(); }

  LocTypeCode getLocType() { return _locType; }

  LocCode getLocCode() { return _locCode; }

  Indicator getExclInd() { return _exclInd; }

private:
  LocTypeCode _locType = ' ';
  LocCode _locCode;
  Indicator _exclInd = 0;
};

class Loc1TransferType
{
  friend class Loc1TransferTypeMock;
  friend class UtcUtilityTest;

public:
  // Tax Point Geo Spec - Loc1 Transfer Type
  static const std::string LOC1_TRANSFER_TYPE;
  static const std::string EQUIPMENT_TO_FLIGHT;
  static const std::string FLIGHT_TO_FLIGHT;
  static const std::string LOC1_TRANSFER_EQUIP;
  static const std::string PREV_LOC1_FLT_TYPE;
  static const std::string PREV_LOC1_FLT_TYPE_EXCL_IND;

  // Error/warming messages
  static const std::string NOT_APPLIED_ON_FIRST_SEGMENT;
  static const std::string NOT_APPLIED_ON_ARUNK_SEGMENT;
  static const std::string INVALID_PREVIOUS_SEGMENT;
  static const std::string NOT_APPLIED_FOR_EQUIPMENT;

  Loc1TransferType(PricingTrx& trx, TaxCodeReg& taxCodeReg);
  Loc1TransferType(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

  virtual ~Loc1TransferType() = default;

  bool isEquipmentToFlt() { return _loc1TransferType == EQUIPMENT_TO_FLIGHT; }

  bool isFltToFlt() { return _loc1TransferType == FLIGHT_TO_FLIGHT; }

  bool validatePrevFltEquip(PricingTrx& trx,
                            TaxCodeReg& taxCodeReg,
                            TaxResponse& taxResponse,
                            uint16_t startIndex);

  bool validatePrevFltLocs(PricingTrx& trx,
                           TaxCodeReg& taxCodeReg,
                           TaxResponse& taxResponse,
                           uint16_t startIndex);

  bool validateEquip(std::string equip);

  bool validateLocs(PricingTrx& trx, TravelSeg* prevTravelSeg);

  virtual void collectErrors(PricingTrx& trx,
                             TaxCodeReg& taxCodeReg,
                             TaxResponse& taxResponse,
                             const std::string& info);

  virtual bool isInLoc(const Loc& loc, const DateTime& ticketingDT);

  void setEquipments(std::string& equip) { _equipments = equip; }

protected:
  Loc1TransferType() {}

  std::string _loc1TransferType;
  std::string _equipments;
  LocTypeCode _prevFltLocType = 0;
  LocCode _prevFltLocCode;
  Indicator _prevFltLocCodeExclInd = 0;
};

class TaxBase final
{

public:
  static const std::string TAX_BASE;
  static const std::string TAX_BASE_OC;
  static const std::string TAX_BASE_CAT31;
  TaxBase() = default;
  TaxBase(PricingTrx& trx, TaxCodeReg& taxCodeReg);
  TaxBase(PricingTrx& trx, TaxCodeReg& taxCodeReg, const DateTime& ticketingDT);
  TaxBase(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

  bool isTaxOnOC() const { return _taxOnOc; }
  bool isTaxOnChangeFee() const { return _taxOnChangeFee; }

private:
  bool _taxOnOc = false;
  bool _taxOnChangeFee = false;
};

class FareType final
{
  friend class UtcUtilityTest;

public:
  static const std::string FARE_TYPE;

  static const std::string DOMESTIC;
  static const std::string FOREGIN_DOMESTIC;
  static const std::string REAL_DOMESTIC;
  static const std::string TRANSBORDER;
  static const std::string INTERNATIONAL;

  FareType(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv);

  bool mustBeValidate() { return !_fareTypes.empty(); }

  virtual bool validateFareTypes(FareUsage& fareUsage);
  virtual bool validateFareTypes(FareUsage& fareUsage, NationCode& taxNation);

private:
  std::string _fareTypes;

  FareType() {}
  void setFareType(std::string& ft) { _fareTypes = ft; }
};

class TaxOnTaxFilterUtc : public tse::TaxOnTaxFilter
{
public:
  TaxOnTaxFilterUtc(const DateTime& date, const std::vector<TaxSpecConfigReg*>* tscv);

private:
  static const std::string TAXONTAX_FILTER_LOC1;
  static const std::string TAXONTAX_FILTER_TAX_CODE;
};

}
}
