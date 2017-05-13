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

#include "Taxes/LegacyTaxes/UtcUtility.h"

#include "Common/LocUtil.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/TaxResponse.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/TaxSpecConfigReg.h"
#include "Diagnostic/Diagnostic.h"
#include "Taxes/LegacyTaxes/TaxDiagnostic.h"

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <vector>

namespace tse
{

namespace utc
{
const std::string FALLBACK{"FALLBACK"};
const std::string YES{"Y"};

const std::string SPECIAL_TAX_ROUND_CENT_NUMBER{"SPECIALTAXROUNDCENT"};
const std::string DISABLE_MIRROR_STOP_LOGIC{"DISABLEMIRRORSTOP"};
const std::string CORSICA_DEFINITION{"CORSICADEFINITION"};
const std::string FR_COAST_DEFINITION{"FRCOASTDEFINITION"};
const std::string START_END_WITHIN_SPEC{"STARTENDWITHINSPEC"};
const std::string PARTITION_ID{"PARTITIONID"};
const std::string ONLY_ORIGINAL_TICKET{"ONLYORIGINALTICKET"};

const std::string TAXEXEMPTFORARUNK{"TAXEXEMPTFORARUNK"};

// TaxOnChangeFee
const std::string TAXONCHANGEFEE{"TAXONCHANGEFEE"};
const std::string PARENTTAX{"PARENTTAX"};
const std::string MATCHPREVTICKET{"MATCHPREVTICKET"};

// OverrideJourneyType
const std::string OVERRIDEJOURNEYTYPE{"JOURNEYTYPE"};
const std::string OVERRIDEJOURNEYTYPEEXCLIND{"JOURNEYTYPEEXCLIND"};

// validateSeqence
const std::string VALIDATE_SEQ_FOR_PERCENTAGE_TAX{"VALIDATESEQ4PTAX"};
const std::string VALIDATE_SEQ_ISSUE_FIX{"VALIDATESEQISSUEFIX"};

// TaxOnTax extensions
const std::string SKIP_TAXONTAX_IF_NO_FARE{"SKIPTAXONTAXIFNOFARE"};
const std::string TAX_ON_TAX_SEG_MATCH{"TAXONTAXSEGMATCH"};
const std::string TAX_ON_DOM_TAX{"TAXONDOMTAX"};
const std::string AY_SPN1800_OPTION_B{"OPTIONB"};

// MirrorImage extensions
const std::string CHECK_CITY_MIRROR{"CHECKCITYMIRROR"};

// TransitValidator extensions
const std::string LAND_TO_AIR_TRANSIT{"LANDTOAIRTRANSIT"};

// ZZ Tax
const std::string TCH{"TCH"};

// AY tax
const std::string AY_TAX_FM_MH{"AYTAXFMMH"};
const std::string AY_TAX_CAP_OPT{"AYTAXCAPOPT"};
const std::string AY_TAX_INT_DOM_INT{"AYTAXINTDOMINT"};
const std::string TREAT_BUS_TRAIN_AS_NON_AIR{"TREATBUSTRAINASNOAIR"};

const std::string TRANSIT_TIME_SKIP_ARUNK{"TRANSTIMESKIPARUNK"};
const std::string MULTICITY_IS_STOP{"MULTICITYISSTOP"};

const std::string NUMBER_OF_SEGMENTS{"NUMBEROFSEGMENTS"};

const std::string MIN_MILAGE{"MINMILAGE"};
const std::string MAX_MILAGE{"MAXMILAGE"};

const std::string FARECLASS_TO_AMOUNT{"FARECLASS2AMOUNT"};
const std::string FARECLASS_CHECK_ALL_FARES{"FARECLASSCHECKALL"};
const std::string FURTHEST_FAREBREAK_1DAY_RT{"FURTHESTFB1DRT"};
const std::string SPECIAL_TRANSIT_AIRPORT{"SPECTRANSITAIRPORT"};
const std::string DISABLE_VIA_LOC_TRANSIT{"DISABLEVIALOCTRANSIT"};
const std::string WITHIN_LATEST_LOC2{"WITHINLATESTLOC2"};
const std::string EXEMPT_IF_ALL_RURAL("EXEMPTIFALLRURAL");

const std::string VALIDATE_EXEMPTION_TABLE{"LATAMEXEMPTION"};
const std::string SKIP_EXEMPTION("SKIPEXEMPTION");

std::string
getSpecConfigParameter(PricingTrx& trx,
                       const std::string& configName,
                       const std::string& paramName,
                       const DateTime& ticketDate)
{

  if (configName.empty())
    return "";
  std::vector<std::string> ret;
  DataHandle dataHandle(ticketDate);
  dataHandle.setParentDataHandle(&trx.dataHandle());
  const std::vector<TaxSpecConfigReg*>& tscv = dataHandle.getTaxSpecConfig(configName);
  std::vector<TaxSpecConfigReg*>::const_iterator cit;

  for (cit = tscv.begin(); cit != tscv.end(); ++cit)
  {

    if (LIKELY((*cit)->effDate() <= ticketDate && (*cit)->discDate().date() >= ticketDate.date()))
    {
      std::vector<TaxSpecConfigReg::TaxSpecConfigRegSeq*>::const_iterator sit;
      for (sit = (*cit)->seqs().begin(); sit != (*cit)->seqs().end(); ++sit)
      {

        if ((*sit)->paramName() == paramName)
        {
          return ((*sit)->paramValue());
        }
      }
    }
  }
  return "";
}

std::string
getSpecConfigParameter(const std::vector<TaxSpecConfigReg*>* tscv,
                       const std::string& paramName,
                       const DateTime& ticketDate)
{
  if (!tscv)
    return "";
  std::vector<std::string> ret;
  std::vector<TaxSpecConfigReg*>::const_iterator cit;

  for (cit = tscv->begin(); cit != tscv->end(); ++cit)
  {

    if (LIKELY((*cit)->effDate() <= ticketDate && (*cit)->discDate().date() >= ticketDate.date()))
    {
      std::vector<TaxSpecConfigReg::TaxSpecConfigRegSeq*>::const_iterator sit;
      for (sit = (*cit)->seqs().begin(); sit != (*cit)->seqs().end(); ++sit)
      {

        if ((*sit)->paramName() == paramName)
        {
          return ((*sit)->paramValue());
        }
      }
    }
  }
  return "";
}

uint16_t
minMilage(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  uint16_t res = 0;
  try
  {
    res = std::stoi( getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
                         MIN_MILAGE, trx.getRequest()->ticketingDT()) );
  }
  catch (const std::invalid_argument&){}
  catch (const std::out_of_range&){}

  return res;
}

uint16_t
maxMilage(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  uint16_t res = 0;
  try
  {
    res = std::stoi( getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
                         MAX_MILAGE, trx.getRequest()->ticketingDT()) );
  }
  catch (const std::invalid_argument&){}
  catch (const std::out_of_range&){}

  return res;
}

bool
doesApplyOnlyOriginalTicket(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      ONLY_ORIGINAL_TICKET, trx.getRequest()->ticketingDT()) == YES;
}

bool
isStartEndWithinSpec(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      START_END_WITHIN_SPEC, trx.getRequest()->ticketingDT()) == YES;
}

bool
skipExemption(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
    SKIP_EXEMPTION, trx.getRequest()->ticketingDT()) == YES;
}

bool
fareClassToAmount(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      FARECLASS_TO_AMOUNT, trx.getRequest()->ticketingDT()) == YES;
}

bool
validateExemptionTable(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      VALIDATE_EXEMPTION_TABLE, trx.getRequest()->ticketingDT()) == YES;
}

std::string
partitionId(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      PARTITION_ID, trx.getRequest()->ticketingDT());
}

std::string
frCorsicaDefinition(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      CORSICA_DEFINITION, trx.getRequest()->ticketingDT());
}

std::string
frCoastDefinition(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      FR_COAST_DEFINITION, trx.getRequest()->ticketingDT());
}

std::string
specialTransitAirport(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      SPECIAL_TRANSIT_AIRPORT, trx.getRequest()->ticketingDT());
}

bool
isTransitViaLocDisabled(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      DISABLE_VIA_LOC_TRANSIT, trx.getRequest()->ticketingDT()) == YES;
}

bool
useWithinLastLoc2Logic(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      WITHIN_LATEST_LOC2, trx.getRequest()->ticketingDT()) == YES;
}

bool
fareClassCheckAllFares(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      FARECLASS_CHECK_ALL_FARES, trx.getRequest()->ticketingDT()) == YES;
}

bool
furthestFareBreak1DayRt(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
      FURTHEST_FAREBREAK_1DAY_RT, trx.getRequest()->ticketingDT()) == YES;
}

bool
isTaxExemptForArunk(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  const std::string& taxExemptForArunk = getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), TAXEXEMPTFORARUNK, trx.getRequest()->ticketingDT());

  return taxExemptForArunk == YES;
}

bool
isTaxExemptForArunk(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  const std::string& taxExemptForArunk =
      getSpecConfigParameter(tscv, TAXEXEMPTFORARUNK, trx.getRequest()->ticketingDT());

  return taxExemptForArunk == YES;
}

/**
 * Tax on change fees indicator
 */
bool
isChangeFeeSeq(PricingTrx& trx, const TaxCodeReg& taxCodeReg, const DateTime& ticketingDT)
{
  const std::string& taxOnChangeFee = getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), TAXONCHANGEFEE, ticketingDT);

  return taxOnChangeFee == YES;
}

bool
isChangeFeeSeq(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  const std::string& taxOnChangeFee =
      getSpecConfigParameter(tscv, TAXONCHANGEFEE, trx.getRequest()->ticketingDT());

  return taxOnChangeFee == YES;
}

std::string
baseTaxCode(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), PARENTTAX, trx.getRequest()->ticketingDT());
}

std::string
baseTaxCode(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, PARENTTAX, trx.getRequest()->ticketingDT());
}

bool
isMatchOrigTicket(PricingTrx& trx, const TaxCodeReg& taxCodeReg, const DateTime& ticketDate)
{

  const std::string& matchOrigTicket = getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), MATCHPREVTICKET, ticketDate);

  return matchOrigTicket == YES;
}

/**
 * validateSeqence params
 */
bool
validateSeqForPercentageTax(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx,
                                taxCodeReg.specConfigName(),
                                VALIDATE_SEQ_FOR_PERCENTAGE_TAX,
                                trx.getRequest()->ticketingDT()) == YES;
}
bool
validateSeqForPercentageTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(
             tscv, VALIDATE_SEQ_FOR_PERCENTAGE_TAX, trx.getRequest()->ticketingDT()) == YES;
}

bool
isMirrorStopLogicDisabled(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx,
                                taxCodeReg.specConfigName(),
                                DISABLE_MIRROR_STOP_LOGIC,
                                trx.getRequest()->ticketingDT()) == YES;
}

uint16_t
specialTaxRoundCentNumber(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  if (getSpecConfigParameter(trx,
                             taxCodeReg.specConfigName(),
                             SPECIAL_TAX_ROUND_CENT_NUMBER,
                             trx.getRequest()->ticketingDT()) == "10")
    return 10;

  return 50;
}

uint16_t
specialTaxRoundCentNumber(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  if (tscv && getSpecConfigParameter(tscv,
                                     SPECIAL_TAX_ROUND_CENT_NUMBER,
                                     trx.getRequest()->ticketingDT()) == "10")
    return 10;

  return 50;
}

bool
validateSeqIssueFix(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx,
                                taxCodeReg.specConfigName(),
                                VALIDATE_SEQ_ISSUE_FIX,
                                trx.getRequest()->ticketingDT()) == YES;
}
bool
validateSeqIssueFix(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, VALIDATE_SEQ_ISSUE_FIX, trx.getRequest()->ticketingDT()) ==
         YES;
}
bool
skipTaxOnTaxIfNoFare(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, SKIP_TAXONTAX_IF_NO_FARE, trx.getRequest()->ticketingDT()) ==
         YES;
}
bool
isTaxOnTaxSeqMatch(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, TAX_ON_TAX_SEG_MATCH, trx.getRequest()->ticketingDT()) == YES;
}

bool
isTaxOnDomTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, TAX_ON_DOM_TAX, trx.getRequest()->ticketingDT()) == YES;
}

bool
isAySPN1800OptionB(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, AY_SPN1800_OPTION_B, trx.getRequest()->ticketingDT()) == YES;
}

bool
isZZTax(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, TCH, trx.getRequest()->ticketingDT()) == YES;
}

bool
isAYTax_FM_MH_NationInt(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, AY_TAX_FM_MH, trx.getRequest()->ticketingDT()) == YES;
}

bool
isAYTaxCapOption(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, AY_TAX_CAP_OPT, trx.getRequest()->ticketingDT()) == YES;
}

std::string
getAYTaxIntDomIntOption(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, AY_TAX_INT_DOM_INT, trx.getRequest()->ticketingDT());
}

bool
treatBusTrainAsNonAir(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, TREAT_BUS_TRAIN_AS_NON_AIR,
    trx.getRequest()->ticketingDT()) == YES;
}

bool
transitValidatorSkipArunk(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{

  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(), TRANSIT_TIME_SKIP_ARUNK,
      trx.getRequest()->ticketingDT()) == YES;
}

bool
transitValidatorMultiCityIsStop(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  return getSpecConfigParameter(trx, taxCodeReg.specConfigName(), MULTICITY_IS_STOP,
      trx.getRequest()->ticketingDT()) == YES;
}

uint16_t
numberOfSegments(PricingTrx& trx, const TaxCodeReg& taxCodeReg)
{
  uint16_t res = 0;
  try
  {
    res = std::stoi( getSpecConfigParameter(trx, taxCodeReg.specConfigName(),
                         NUMBER_OF_SEGMENTS, trx.getRequest()->ticketingDT()) );
  }
  catch (const std::invalid_argument&){}
  catch (const std::out_of_range&){}

  return res;
}

bool
shouldCheckCityMirror(const DateTime& date,
                      const std::vector<TaxSpecConfigReg*>* const taxSpecConfigRegs)
{
  return getSpecConfigParameter(taxSpecConfigRegs, CHECK_CITY_MIRROR, date) == YES;
}

bool
isLandToAirStopover(PricingTrx& trx,
                    TaxResponse& taxResponse,
                    const std::vector<TaxSpecConfigReg*>* tscv)
{
  static const std::string ALL("ALL");
  const std::string& value =
      getSpecConfigParameter(tscv, LAND_TO_AIR_TRANSIT, trx.getRequest()->ticketingDT());
  if (UNLIKELY(value == ALL))
    return false;

  const CarrierCode& carrier = taxResponse.farePath()->itin()->validatingCarrier();

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> separator(".");
  tokenizer tokens(value, separator);
  tokenizer::iterator tokenI;
  for (tokenI = tokens.begin(); tokenI != tokens.end(); ++tokenI)
    if (carrier == tokenI->data())
      return false;

  return true;
}

bool
shouldExemptIfAllRural(const PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  return getSpecConfigParameter(tscv, EXEMPT_IF_ALL_RURAL, trx.getRequest()->ticketingDT()) == YES;
}

bool
isTaxOnOC(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  TaxBase taxBase(trx, taxCodeReg);
  return taxBase.isTaxOnOC();
}

bool
isTaxOnChangeFee(PricingTrx& trx, TaxCodeReg& taxCodeReg, const DateTime& ticketingDT)
{
  TaxBase taxBase(trx, taxCodeReg, ticketingDT);
  return taxBase.isTaxOnChangeFee();
}

bool
isTaxOnChangeFee(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  TaxBase taxBase(trx, tscv);
  return taxBase.isTaxOnChangeFee();
}

/**
 * Journey Type extension
 */
OverrideJourneyType::OverrideJourneyType(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  std::string journeyType = utc::getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), utc::OVERRIDEJOURNEYTYPE, trx.getRequest()->ticketingDT());

  if (journeyType.size() < 2)
    return;

  _locType = journeyType[0];
  _locCode = journeyType.substr(1);
  _exclInd = getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), OVERRIDEJOURNEYTYPEEXCLIND, trx.getRequest()->ticketingDT())[0];
}

/*
 OverrideJourneyType::OverrideJourneyType( PricingTrx& trx, const std::vector<TaxSpecConfigReg*>*
 tscv) :
 _locType( ' ')
 {
 std::string journeyType = utc::getSpecConfigParameter( tscv, utc::OVERRIDEJOURNEYTYPE,
 trx.getRequest()->ticketingDT());

 if(journeyType.size() < 2)
 return;

 _locType = journeyType[0];
 _locCode = journeyType.substr( 1);
 _exclInd = getSpecConfigParameter( tscv, OVERRIDEJOURNEYTYPEEXCLIND,
 trx.getRequest()->ticketingDT())[0];
 }*/

// Tax Point Geo Spec - Loc1 Transfer Type
const std::string
Loc1TransferType::LOC1_TRANSFER_TYPE("LOC1TRANSFERTYPE");
const std::string
Loc1TransferType::EQUIPMENT_TO_FLIGHT("EQUIP2FLT");
const std::string
Loc1TransferType::FLIGHT_TO_FLIGHT("FLT2FLT");
const std::string
Loc1TransferType::LOC1_TRANSFER_EQUIP("LOC1TRANSFEREQUIP");
const std::string
Loc1TransferType::PREV_LOC1_FLT_TYPE("PREVLOC1FLTTYPE");
const std::string
Loc1TransferType::PREV_LOC1_FLT_TYPE_EXCL_IND("PREVLOC1FLTTYPEEXCLIND");

// Error/warming messages
const std::string
Loc1TransferType::NOT_APPLIED_ON_FIRST_SEGMENT("NOT APPLIED ON FIRST SEGMENT");
const std::string
Loc1TransferType::NOT_APPLIED_ON_ARUNK_SEGMENT("NOT APPLIED ON ARUNK SEGMENT");
const std::string
Loc1TransferType::INVALID_PREVIOUS_SEGMENT("INVALID PREVIOUS SEGMENT");
const std::string
Loc1TransferType::NOT_APPLIED_FOR_EQUIPMENT("NOT APPLIED FOR EQUIPMENT: ");

/**
 * Transfer Type type extension
 */
Loc1TransferType::Loc1TransferType(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  _loc1TransferType = utc::getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), LOC1_TRANSFER_TYPE, trx.getRequest()->ticketingDT());

  _equipments = utc::getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), LOC1_TRANSFER_EQUIP, trx.getRequest()->ticketingDT());
  std::string prevFltLocType = utc::getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), PREV_LOC1_FLT_TYPE, trx.getRequest()->ticketingDT());

  if (prevFltLocType.size() < 2)
    return;

  _prevFltLocType = prevFltLocType[0];
  _prevFltLocCode = prevFltLocType.substr(1);
  _prevFltLocCodeExclInd = getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), PREV_LOC1_FLT_TYPE_EXCL_IND, trx.getRequest()->ticketingDT())[0];
}

Loc1TransferType::Loc1TransferType(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  _loc1TransferType =
      utc::getSpecConfigParameter(tscv, LOC1_TRANSFER_TYPE, trx.getRequest()->ticketingDT());

  _equipments =
      utc::getSpecConfigParameter(tscv, LOC1_TRANSFER_EQUIP, trx.getRequest()->ticketingDT());
  std::string prevFltLocType =
      utc::getSpecConfigParameter(tscv, PREV_LOC1_FLT_TYPE, trx.getRequest()->ticketingDT());

  if (prevFltLocType.size() < 2)
    return;

  _prevFltLocType = prevFltLocType[0];
  _prevFltLocCode = prevFltLocType.substr(1);
  _prevFltLocCodeExclInd =
      getSpecConfigParameter(tscv, PREV_LOC1_FLT_TYPE_EXCL_IND, trx.getRequest()->ticketingDT())[0];
}

bool
Loc1TransferType::validatePrevFltEquip(PricingTrx& trx,
                                       TaxCodeReg& taxCodeReg,
                                       TaxResponse& taxResponse,
                                       uint16_t startIndex)
{

  if (!startIndex)
  {
    collectErrors(trx, taxCodeReg, taxResponse, NOT_APPLIED_ON_FIRST_SEGMENT);
    return false;
  }

  const Itin* itin = taxResponse.farePath()->itin();
  TravelSeg* prevTravelSeg = itin->travelSeg()[startIndex - 1];

  if (!prevTravelSeg->isAir())
  {
    collectErrors(trx, taxCodeReg, taxResponse, NOT_APPLIED_ON_ARUNK_SEGMENT);
    return false;
  }

  if (validateEquip(prevTravelSeg->equipmentType()))
  {
    return true;
  }
  else
  {
    collectErrors(
        trx, taxCodeReg, taxResponse, NOT_APPLIED_FOR_EQUIPMENT + prevTravelSeg->equipmentType());

    return false;
  }
}

bool
Loc1TransferType::validateEquip(std::string equip)
{
  return _equipments.find(equip) != std::string::npos;
}

bool
Loc1TransferType::validatePrevFltLocs(PricingTrx& trx,
                                      TaxCodeReg& taxCodeReg,
                                      TaxResponse& taxResponse,
                                      uint16_t startIndex)
{
  if (!startIndex)
  {
    collectErrors(trx, taxCodeReg, taxResponse, NOT_APPLIED_ON_FIRST_SEGMENT);
    return false;
  }

  const Itin* itin = taxResponse.farePath()->itin();
  TravelSeg* prevTravelSeg = itin->travelSeg()[startIndex - 1];

  if (!prevTravelSeg->isAir())
  {
    prevTravelSeg = itin->travelSeg()[startIndex - 2];
  }

  if (validateLocs(trx, prevTravelSeg))
  {
    return true;
  }
  else
  {
    collectErrors(trx, taxCodeReg, taxResponse, INVALID_PREVIOUS_SEGMENT);
    return false;
  }
}

bool
Loc1TransferType::validateLocs(PricingTrx& trx, TravelSeg* prevTravelSeg)
{
  bool locMatch = isInLoc(*prevTravelSeg->origin(), trx.getRequest()->ticketingDT());

  if ((locMatch && _prevFltLocCodeExclInd == tse::YES) ||
      (!locMatch && _prevFltLocCodeExclInd != tse::YES))
  {
    return false;
  }

  locMatch = isInLoc(*prevTravelSeg->destination(), trx.getRequest()->ticketingDT());

  if ((locMatch && _prevFltLocCodeExclInd == tse::YES) ||
      (!locMatch && _prevFltLocCodeExclInd != tse::YES))
  {
    return false;
  }

  return true;
}

void
Loc1TransferType::collectErrors(PricingTrx& trx,
                                TaxCodeReg& taxCodeReg,
                                TaxResponse& taxResponse,
                                const std::string& info)
{

  TaxDiagnostic::collectErrors(trx,
                               taxCodeReg,
                               taxResponse,
                               tse::TaxDiagnostic::LOC1_TRANSFER_TYPE,
                               tse::Diagnostic816,
                               info);
}

bool
Loc1TransferType::isInLoc(const Loc& loc, const DateTime& ticketingDT)
{
  return LocUtil::isInLoc(loc,
                          _prevFltLocType,
                          _prevFltLocCode,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::TAXES,
                          GeoTravelType::International,
                          EMPTY_STRING(),
                          ticketingDT);
}

const std::string
TaxBase::TAX_BASE("TAXBASE");
const std::string
TaxBase::TAX_BASE_OC("OC");
const std::string
TaxBase::TAX_BASE_CAT31("CAT31");

TaxBase::TaxBase(PricingTrx& trx, TaxCodeReg& taxCodeReg, const DateTime& ticketingDT)
{
  const std::string& base = utc::getSpecConfigParameter(
        trx, taxCodeReg.specConfigName(), TAX_BASE, ticketingDT);

    if (base == TaxBase::TAX_BASE_OC)
    {
      _taxOnOc = true;
    }
    else if (base == TaxBase::TAX_BASE_CAT31 || isChangeFeeSeq(trx, taxCodeReg, ticketingDT))
    {
      _taxOnChangeFee = true;
    }
}

TaxBase::TaxBase(PricingTrx& trx, TaxCodeReg& taxCodeReg)
{
  const std::string& base = utc::getSpecConfigParameter(
      trx, taxCodeReg.specConfigName(), TAX_BASE, trx.getRequest()->ticketingDT());

  if (base == TaxBase::TAX_BASE_OC)
  {
    _taxOnOc = true;
  }
  else if (base == TaxBase::TAX_BASE_CAT31 ||
      isChangeFeeSeq(trx, taxCodeReg, trx.getRequest()->ticketingDT()))
  {
    _taxOnChangeFee = true;
  }
}

TaxBase::TaxBase(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  const std::string& base =
      utc::getSpecConfigParameter(tscv, TAX_BASE, trx.getRequest()->ticketingDT());

  if (UNLIKELY(base == TaxBase::TAX_BASE_OC))
  {
    _taxOnOc = true;
  }
  else if (UNLIKELY(base == TaxBase::TAX_BASE_CAT31 ||
      isChangeFeeSeq(trx, tscv)))
  {
    _taxOnChangeFee = true;
  }
}

const std::string
FareType::FARE_TYPE("FARETYPE");

const std::string FareType::DOMESTIC = "DOM";
const std::string FareType::FOREGIN_DOMESTIC = "FDOM";
const std::string FareType::REAL_DOMESTIC = "RDOM";
const std::string FareType::TRANSBORDER = "TRANS";
const std::string FareType::INTERNATIONAL = "INT";

FareType::FareType(PricingTrx& trx, const std::vector<TaxSpecConfigReg*>* tscv)
{
  _fareTypes = utc::getSpecConfigParameter(tscv, FARE_TYPE, trx.getRequest()->ticketingDT());
}

bool
FareType::validateFareTypes(FareUsage& fareUsage)
{
  if (_fareTypes.find(FareType::DOMESTIC) != std::string::npos)
  {
    if (fareUsage.paxTypeFare()->isDomestic())
      return true;
  }

  if (_fareTypes.find(FareType::FOREGIN_DOMESTIC) != std::string::npos)
  {
    if (fareUsage.paxTypeFare()->isForeignDomestic())
      return true;
  }

  if (_fareTypes.find(FareType::TRANSBORDER) != std::string::npos)
  {
    if (fareUsage.paxTypeFare()->isTransborder())
      return true;
  }

  if (_fareTypes.find(FareType::INTERNATIONAL) != std::string::npos)
  {
    if (fareUsage.paxTypeFare()->isInternational())
      return true;
  }

  return false;
}

bool
FareType::validateFareTypes(FareUsage& fareUsage, NationCode& taxNation)
{
  if (validateFareTypes(fareUsage))
    return true;

  if (_fareTypes.find(FareType::REAL_DOMESTIC) != std::string::npos)
  {
    if ((fareUsage.paxTypeFare()->isDomestic() || fareUsage.paxTypeFare()->isForeignDomestic()) &&
        (fareUsage.paxTypeFare()->fareMarket()->origin()->nation() == taxNation &&
         fareUsage.paxTypeFare()->fareMarket()->destination()->nation() == taxNation))
      return true;
  }

  return false;
}

const std::string TaxOnTaxFilterUtc::TAXONTAX_FILTER_LOC1 = "TAXONTAXFILTERLOC1";
const std::string TaxOnTaxFilterUtc::TAXONTAX_FILTER_TAX_CODE = "TAXONTAXFILTERTAX";

TaxOnTaxFilterUtc::TaxOnTaxFilterUtc(const DateTime& date, const std::vector<TaxSpecConfigReg*>* tscv)
  : tse::TaxOnTaxFilter(date)
{
  if (!tscv)
    return;

  std::string str = utc::getSpecConfigParameter(tscv, TAXONTAX_FILTER_LOC1, _ticketingDT);

  if (str.size() < 2)
    return;

  _loc1Type = str[0];
  _loc1Code = str.substr(1);
  _taxCode = utc::getSpecConfigParameter(tscv, TAXONTAX_FILTER_TAX_CODE, _ticketingDT);

  if (_taxCode.empty())
    return;

  _enable = true;
}

}
}
