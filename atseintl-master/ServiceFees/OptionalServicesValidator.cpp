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

#include "ServiceFees/OptionalServicesValidator.h"

#include "Common/CurrencyConverter.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncRequest.h"
#include "DataModel/CollectedNegFareData.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierFlightSeg.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/SvcFeesCurrencyInfo.h"
#include "DBAccess/SvcFeesCxrResultingFCLInfo.h"
#include "DBAccess/SvcFeesResBkgDesigInfo.h"
#include "DBAccess/TaxCarrierFlightInfo.h"
#include "Diagnostic/Diag877Collector.h"
#include "Rules/PeriodOfStay.h"
#include "Rules/RuleUtil.h"
#include "ServiceFees/OCFees.h"
#include "TicketingFee/SecurityValidator.h"
#include "TicketingFee/SvcFeesAccountCodeValidator.h"
#include "TicketingFee/SvcFeesTktDesigValidator.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <functional>
#include <iterator>
#include <map>

namespace
{
tse::Logger logger("atseintl.ServiceFees.OptionalServicesValidator");

struct IsRBD
{
  IsRBD(tse::BookingCode& bookingCode) : _bookingCode(bookingCode) {}
  bool operator()(tse::SvcFeesResBkgDesigInfo* info,
                  tse::AirSeg& seg,
                  bool isMarketingCxr,
                  tse::Diag877Collector* diag) const
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
    bool res = false;
    tse::StatusT198 status = tse::PASS_T198;
    if (seg.carrier() != seg.operatingCarrierCode() &&
        (!isMarketingCxr ||
         info->mkgOperInd() == tse::OptionalServicesValidator::T198_MKT_OPER_IND_OPER))
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " status=FAIL_RBD_NOT_PROCESSED");
      status = tse::FAIL_RBD_NOT_PROCESSED;
    }
    else if (seg.carrier() != info->carrier())
    {
      LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " status=FAIL_ON_RBD_CXR");
      status = tse::FAIL_ON_RBD_CXR;
    }
    else
    {
      res = info->bookingCode1() == _bookingCode || info->bookingCode2() == _bookingCode ||
            info->bookingCode3() == _bookingCode || info->bookingCode4() == _bookingCode ||
            info->bookingCode5() == _bookingCode;
      if (!res)
      {
        LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " status=FAIL_ON_RBD_CODE");
        status = tse::FAIL_ON_RBD_CODE;
      }
    }
    if (diag)
      diag->printRBDTable198Info(_bookingCode, seg.origAirport(), seg.destAirport(), info, status);
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning " << res << ", status=" << status);
    return res;
  }

private:
  tse::BookingCode& _bookingCode;
};
}

using namespace std;

namespace tse
{
FALLBACK_DECL(fallback_AB240_UpgradeCheckForUPGroupCode);
FALLBACK_DECL(ocFeesAmountRoundingRefactoring);
FALLBACK_DECL(fallbackGoverningCrxForT171);

const Indicator OptionalServicesValidator::T183_SCURITY_PRIVATE;
const Indicator OptionalServicesValidator::T198_MKT_OPER_IND_MKT;
const Indicator OptionalServicesValidator::T198_MKT_OPER_IND_OPER;
const Indicator OptionalServicesValidator::SEC_POR_IND_SECTOR;
const Indicator OptionalServicesValidator::SEC_POR_IND_PORTION;
const Indicator OptionalServicesValidator::SEC_POR_IND_JOURNEY;
const Indicator OptionalServicesValidator::CHAR_BLANK;
const Indicator OptionalServicesValidator::CHAR_BLANK2;
const Indicator OptionalServicesValidator::FTW_FROM;
const Indicator OptionalServicesValidator::FTW_TO;
const Indicator OptionalServicesValidator::FTW_WITHIN;
const Indicator OptionalServicesValidator::FTW_RULE_BUSTER_A;
const Indicator OptionalServicesValidator::FTW_RULE_BUSTER_B;
// Service not available/ no charge
const Indicator OptionalServicesValidator::SERVICE_NOT_AVAILABLE;
const Indicator OptionalServicesValidator::SERVICE_FREE_NO_EMD_ISSUED;
const Indicator OptionalServicesValidator::SERVICE_FREE_EMD_ISSUED;
const Indicator OptionalServicesValidator::SERVICE_FREE_NO_BOOK_NO_EMD;
const Indicator OptionalServicesValidator::SERVICE_FREE_NO_BOOK_EMD_ISSUED;
const Indicator OptionalServicesValidator::SCD_CNX_POINT;
const Indicator OptionalServicesValidator::SCD_NO_FARE_BREAK;
const Indicator OptionalServicesValidator::SCD_NO_FARE_BREAK_OR_STOPOVER;
const Indicator OptionalServicesValidator::SCD_STOPOVER;
const Indicator OptionalServicesValidator::SCD_STOPOVER_WITH_GEO;
const Indicator OptionalServicesValidator::FARE_IND_19_22;
const Indicator OptionalServicesValidator::FARE_IND_25;
const Indicator OptionalServicesValidator::FARE_IND_35;
const ServiceRuleTariffInd OptionalServicesValidator::RULE_TARIFF_IND_PUBLIC = "PUB";
const ServiceRuleTariffInd OptionalServicesValidator::RULE_TARIFF_IND_PRIVATE = "PRI";

bool
PadisComparator::
operator()(const SvcFeesResBkgDesigInfo* padis_1, const SvcFeesResBkgDesigInfo* padis_2) const
{
  if (!padis_1 || !padis_2)
    return false;

  std::set<BookingCode> bookingCode_1;
  std::set<BookingCode> bookingCode_2;

  fill(bookingCode_1, padis_1);
  fill(bookingCode_2, padis_2);

  return bookingCode_1 < bookingCode_2;
}

void
PadisComparator::fill(std::set<BookingCode>& bookingCode, const SvcFeesResBkgDesigInfo* padis) const
{
  if (padis->bookingCode1() != EMPTY_STRING())
    bookingCode.insert(padis->bookingCode1());
  if (padis->bookingCode2() != EMPTY_STRING())
    bookingCode.insert(padis->bookingCode2());
  if (padis->bookingCode3() != EMPTY_STRING())
    bookingCode.insert(padis->bookingCode3());
  if (padis->bookingCode4() != EMPTY_STRING())
    bookingCode.insert(padis->bookingCode4());
  if (padis->bookingCode5() != EMPTY_STRING())
    bookingCode.insert(padis->bookingCode5());
}

OptionalServicesValidator::OptionalServicesValidator(
    const OcValidationContext& ctx,
    const vector<TravelSeg*>::const_iterator segI,
    const vector<TravelSeg*>::const_iterator segIE,
    const std::vector<TravelSeg*>::const_iterator endOfJourney,
    const Ts2ss& ts2ss,
    bool isInternational,
    bool isOneCarrier,
    bool isMarketingCxr,
    Diag877Collector* diag)
  : _trx(ctx.trx),
    _itin(ctx.itin),
    _paxType(ctx.paxType),
    _farePath(ctx.fp),
    _segI(segI),
    _segIE(segIE),
    _endOfJourney(endOfJourney),
    _ts2ss(ts2ss),
    _isIntrnl(isInternational),
    _isOneCarrier(isOneCarrier),
    _isMarketingCxr(isMarketingCxr),
    _diag(diag),
    _portionBG(false),
    _merchCrxStrategy(&_multipleStrategy)
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " Entering ");
  std::vector<TravelSeg*> tvlSegs(_segI, _segIE);

  for (TravelSeg* ts : tvlSegs)
  {
    AirSeg* const as = ts->toAirSeg();
    if (as)
      _airSegs.push_back(as);
    const auto t2ssI = _ts2ss.find(ts);
    if (t2ssI != _ts2ss.end())
      _processedFares.insert(t2ssI->second.second);
  }

  std::vector<FareMarket*> retFareMarket;
  TrxUtil::getFareMarket(_trx, tvlSegs, DateTime::emptyDate(), retFareMarket, &_itin);
  if (!retFareMarket.empty())
    _geoTravelType = retFareMarket.front()->geoTravelType();
  else
    _geoTravelType = GeoTravelType::UnknownGeoTravelType;
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " Exiting ");
}

const vector<OptionalServicesInfo*>&
OptionalServicesValidator::getOptionalServicesInfo(const SubCodeInfo& subCode) const
{
  //  TSELatencyData metrics(_trx, "OC GET S7");
  return _merchCrxStrategy->getOptionalServicesInfo(_trx.dataHandle(),
                                                    subCode.vendor(),
                                                    subCode.carrier(),
                                                    (*_segI)->origAirport(),
                                                    (*_segI)->destAirport(),
                                                    subCode.serviceTypeCode(),
                                                    subCode.serviceSubTypeCode(),
                                                    subCode.fltTktMerchInd(),
                                                    _trx.ticketingDate());
}

const vector<SvcFeesCurrencyInfo*>&
OptionalServicesValidator::getSvcFeesCurrency(const OptionalServicesInfo& optSrvInfo) const
{
  return _trx.dataHandle().getSvcFeesCurrency(optSrvInfo.vendor(),
                                              optSrvInfo.serviceFeesCurrencyTblItemNo());
}

const vector<SvcFeesResBkgDesigInfo*>&
OptionalServicesValidator::getRBDInfo(const VendorCode& vendor, int itemNo) const
{
  return _trx.dataHandle().getSvcFeesResBkgDesig(vendor, itemNo);
}

const std::vector<SvcFeesCxrResultingFCLInfo*>&
OptionalServicesValidator::getResFCInfo(const VendorCode& vendor, int itemNo) const
{
  return _trx.dataHandle().getSvcFeesCxrResultingFCL(vendor, itemNo);
}

bool
OptionalServicesValidator::svcFeesAccountCodeValidate(const SvcFeesAccountCodeValidator& validator,
                                                      int itemNo) const
{
  return validator.validate(itemNo);
}

bool
OptionalServicesValidator::svcFeesTktDesignatorValidate(const SvcFeesTktDesigValidator& validator,
                                                        int itemNo) const
{
  return validator.validate(itemNo);
}

bool
OptionalServicesValidator::inputPtcValidate(const ServiceFeeUtil& util,
                                            const OptionalServicesInfo& optSrvInfo) const
{
  return util.matchPaxType(optSrvInfo.carrier(), _paxType, optSrvInfo.psgType());
}

bool
OptionalServicesValidator::isInLoc(const VendorCode& vendor,
                                   const LocKey& locKey,
                                   const Loc& loc,
                                   CarrierCode carrier) const
{
  return LocUtil::isInLoc(loc,
                          locKey.locType(),
                          locKey.loc(),
                          vendor,
                          RESERVED,
                          LocUtil::OTHER,
                          _isIntrnl ? GeoTravelType::International : GeoTravelType::Domestic,
                          carrier,
                          _trx.ticketingDate());
}

bool
OptionalServicesValidator::isInZone(const VendorCode& vendor,
                                    const LocCode& zone,
                                    const Loc& loc,
                                    CarrierCode carrier) const
{
  return LocUtil::isInZone(loc,
                           vendor,
                           zone,
                           TAX_ZONE,
                           LocUtil::OTHER,
                           _isIntrnl ? GeoTravelType::International : GeoTravelType::Domestic,
                           carrier,
                           _trx.ticketingDate());
}

bool
OptionalServicesValidator::getFeeRounding_old(const CurrencyCode& currencyCode,
                                          RoundingFactor& roundingFactor,
                                          CurrencyNoDec& roundingNoDec,
                                          RoundingRule& roundingRule) const
{
  ServiceFeeUtil util(_trx);
  return util.getFeeRounding_old(currencyCode, roundingFactor, roundingNoDec, roundingRule);
}

bool
OptionalServicesValidator::validate(OCFees& ocFees, bool stopMatch) const
{
  Diag877Collector* diag877 = _diag;
  if (diag877 && !ServiceFeeUtil::isPortionOfTravelSelected(_trx, *_segI, *(_segIE - 1)))
    _diag = nullptr;

  StateRestorer<Diag877Collector*> diagRestorer(_diag, _diag, diag877);

  const vector<OptionalServicesInfo*>& optSrvInfos = getOptionalServicesInfo(*ocFees.subCodeInfo());

  if (optSrvInfos.empty())
  {
    if (!isDDPass())
      printDiagS7NotFound(*ocFees.subCodeInfo());
    return false;
  }
  bool result = false;
  for (OptionalServicesInfo* s7 : optSrvInfos)
  {
    checkDiagS7ForDetail(s7);
    StatusS7Validation rc = validateS7Data(*s7, ocFees);

    if (rc == PASS_S7 || rc == PASS_S7_FREE_SERVICE || rc == PASS_S7_NOT_AVAIL)
    {
      ocFees.farePath() = _farePath;
      if (stopMatch)
      {
        printDiagS7Info(s7, ocFees, rc);
        printStopAtFirstMatchMsg();
        return true;
      }

      result = true;
      if (s7->upgrdServiceFeesResBkgDesigTblItemNo() == 0)
      {
        printDiagS7Info(s7, ocFees, rc);
        break;
      }
      addPadisCodesToOCFees(ocFees, *s7);
      printDiagS7Info(s7, ocFees, rc);
      ocFees.addSeg(_trx.dataHandle());
    }
    else
    {
      printDiagS7Info(s7, ocFees, rc);
      ocFees.cleanOutCurrentSeg();
    }
  }
  ocFees.pointToFirstOCFee();
  if (ocFees.segCount() > 1 && !ocFees.segments().back()->_optFee)
    ocFees.segments().erase(ocFees.segments().end() - 1);

  return result;
}

void
OptionalServicesValidator::addPadisCodesToOCFees(OCFees& ocFees, const OptionalServicesInfo& s7)
    const
{
  const vector<SvcFeesResBkgDesigInfo*>& padisInfos = _trx.dataHandle().getSvcFeesResBkgDesig(
      s7.vendor(), s7.upgrdServiceFeesResBkgDesigTblItemNo());
  if (_diag && isDDInfo())
    _diag->printS7DetailInfoPadis(&s7, ocFees, padisInfos, _itin.travelDate());

  std::set<SvcFeesResBkgDesigInfo*, PadisComparator> uniquePadisSet;
  std::copy(
      padisInfos.begin(), padisInfos.end(), std::inserter(uniquePadisSet, uniquePadisSet.begin()));
  std::copy(uniquePadisSet.begin(), uniquePadisSet.end(), std::back_inserter(ocFees.padisData()));
}

bool
OptionalServicesValidator::validateLocation(const VendorCode& vendor,
                                            const LocKey& locKey,
                                            const Loc& loc,
                                            const LocCode& zone,
                                            bool emptyRet,
                                            CarrierCode carrier,
                                            LocCode* matchLoc) const
{
  if (locKey.isNull())
    return emptyRet;

  if (locKey.locType() == LOCTYPE_USER)
    return isInZone(vendor, zone, loc, carrier);

  return isInLoc(vendor, locKey, loc, carrier);
}

StatusS7Validation
OptionalServicesValidator::validateS7Data(OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const
{
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__);
  //  TSELatencyData metrics(_trx, "OC S7 DATA VALIDATION");
  if (_diag && !isDiagSequenceMatch(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_SEQUENCE");
    return FAIL_S7_SEQUENCE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_SEQUENCE");
  }

  if (!skipUpgradeCheck(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_UPGRADE");
    return FAIL_S7_UPGRADE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_UPGRADE");
  }

  if (!checkUpgradeT198(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_UPGRADE_T198");
    return FAIL_S7_UPGRADE_T198;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_UPGRADE_T198");
  }

  if (ocFees.subCodeInfo()->fltTktMerchInd() == PREPAID_BAGGAGE &&
      !checkSectorPortionIndForBG(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_SECTOR_PORTION");
    return FAIL_S7_SECTOR_PORTION;
  }
  else if (!checkSectorPortionInd(optSrvInfo.sectorPortionInd()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_SECTOR_PORTION");
    return FAIL_S7_SECTOR_PORTION;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_SECTOR_PORTION");
  }

  if (!checkInputTravelDate(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_INPUT_TVL_DATE");
    return FAIL_S7_INPUT_TVL_DATE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_INPUT_TVL_DATE");
  }

  if (!checkAdvPurchaseTktInd(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_ADV_PURCHASE_TKT_IND");
    return FAIL_S7_ADV_PURCHASE_TKT_IND;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_ADV_PURCHASE_TKT_IND");
  }

  if (!checkTravelDate(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_TVL_DATE");
    return FAIL_S7_TVL_DATE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_TVL_DATE");
  }

  if (!checkCollectSubtractFee(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_COLLECT_SUBTRACT");
    return FAIL_S7_COLLECT_SUBTRACT;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_COLLECT_SUBTRACT");
  }

  if (!checkNetSellFeeAmount(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_NET_SELL");
    return FAIL_S7_NET_SELL;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_NET_SELL");
  }

  if (!checkInputPtc(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_INPUT_PSG_TYPE");
    return FAIL_S7_INPUT_PSG_TYPE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_INPUT_PSG_TYPE");
  }

  if (!checkFrequentFlyerStatus(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_FREQ_FLYER_STATUS");
    return FAIL_S7_FREQ_FLYER_STATUS;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_FREQ_FLYER_STATUS");
  }

  if (!checkAccountCodes(optSrvInfo.serviceFeesAccountCodeTblItemNo()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_ACCOUNT_CODE");
    return FAIL_S7_ACCOUNT_CODE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_ACCOUNT_CODE");
  }

  if (!checkInputTicketDesignator(optSrvInfo.serviceFeesTktDesigTblItemNo()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_INPUT_TKT_DESIGNATOR");
    return FAIL_S7_INPUT_TKT_DESIGNATOR;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_INPUT_TKT_DESIGNATOR");
  }

  if (!checkSecurity(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_SECUR_T183");
    return FAIL_S7_SECUR_T183;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_SECUR_T183");
  }

  if (!checkGeoFtwInd(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_FROM_TO_WITHIN");
    return FAIL_S7_FROM_TO_WITHIN;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_FROM_TO_WITHIN");
  }

  vector<TravelSeg*> passedLoc3Dest;

  if (!checkIntermediatePoint(optSrvInfo, passedLoc3Dest, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_INTERMEDIATE_POINT");
    return FAIL_S7_INTERMEDIATE_POINT;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_INTERMEDIATE_POINT");
  }

  if (!checkStopCnxDestInd(optSrvInfo, passedLoc3Dest, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_STOP_CNX_DEST");
    return FAIL_S7_STOP_CNX_DEST;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_STOP_CNX_DEST");
  }

  if (!checkCabin(optSrvInfo.cabin(), optSrvInfo.carrier(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_CABIN");
    return FAIL_S7_CABIN;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_CABIN");
  }

  if (!checkRBD(optSrvInfo.vendor(), optSrvInfo.serviceFeesResBkgDesigTblItemNo(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_RBD_T198");
    return FAIL_S7_RBD_T198;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_RBD_T198");
  }

  if (!checkResultingFareClass(
          optSrvInfo.vendor(), optSrvInfo.serviceFeesCxrResultingFclTblItemNo(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_RESULT_FC_T171");
    return FAIL_S7_RESULT_FC_T171;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_RESULT_FC_T171");
  }

  if (!checkOutputTicketDesignator(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_OUTPUT_TKT_DESIGNATOR");
    return FAIL_S7_OUTPUT_TKT_DESIGNATOR;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_OUTPUT_TKT_DESIGNATOR");
  }

  if (!checkRuleTariffInd(optSrvInfo.ruleTariffInd()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_RULE_TARIFF_IND");
    return FAIL_S7_RULE_TARIFF_IND;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_RULE_TARIFF_IND");
  }

  if (!checkRuleTariff(optSrvInfo.ruleTariff(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_RULE_TARIFF");
    return FAIL_S7_RULE_TARIFF;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_RULE_TARIFF");
  }

  if (!checkRule(optSrvInfo.rule(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_RULE");
    return FAIL_S7_RULE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_RULE");
  }

  if (!checkFareInd(optSrvInfo.fareInd()))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_FARE_IND");
    return FAIL_S7_FARE_IND;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_FARE_IND");
  }

  if (!checkTourCode(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_TOURCODE");
    return FAIL_S7_TOURCODE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_TOURCODE");
  }

  bool skipDOWCheck = false;
  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " skipDOWCheck=" << skipDOWCheck);
  if (!checkStartStopTime(optSrvInfo, skipDOWCheck, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_START_STOP_TIME");
    return FAIL_S7_START_STOP_TIME;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_START_STOP_TIME");
  }

  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " skipDOWCheck=" << skipDOWCheck);
  if (!skipDOWCheck && !checkDOW(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_DOW");
    return FAIL_S7_DOW;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_DOW");
  }

  if (!checkCarrierFlightApplT186(optSrvInfo.vendor(), optSrvInfo.carrierFltTblItemNo(), ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_CXR_FLT_T186");
    return FAIL_S7_CXR_FLT_T186;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_CXR_FLT_T186");
  }

  if (!checkEquipmentType(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_EQUIPMENT");
    return FAIL_S7_EQUIPMENT;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_EQUIPMENT");
  }

  if (!checkAdvPur(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_ADVPUR");
    return FAIL_S7_ADVPUR;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_ADVPUR");
  }

  if (!checkInterlineIndicator(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_INTERLINE_IND");
    return FAIL_S7_INTERLINE_IND;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_INTERLINE_IND");
  }

  StatusS7Validation rc = checkServiceNotAvailNoCharge(optSrvInfo, ocFees);
  if (rc != PASS_S7)
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning " << rc << " at line " << __LINE__);
    return rc;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_checkServiceNotAvailNoCharge");
  }

  if (!checkMileageFee(optSrvInfo))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_MILEAGE_FEE");
    return FAIL_S7_MILEAGE_FEE;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_MILEAGE_FEE");
  }

  if (!retrieveSpecifiedFee(optSrvInfo, ocFees)) // Table 170
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_SFC_T170");
    return FAIL_S7_SFC_T170;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_SFC_T170");
  }

  if (!checkFeeApplication(optSrvInfo, ocFees))
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning FAIL_S7_FEE_APPL");
    return FAIL_S7_FEE_APPL;
  }
  else
  {
    LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " PASS_S7_FEE_APPL");
  }

  LOG4CXX_DEBUG(logger, __LOG4CXX_FUNC__ << " returning PASS_S7");
  return PASS_S7;
}

bool
OptionalServicesValidator::checkInputTravelDate(const OptionalServicesInfo& optSrvInfo) const
{
  const DateTime& data = _trx.ticketingDate();

  return ServiceFeeUtil::checkIsDateBetween(optSrvInfo.effDate(), optSrvInfo.discDate(), data) &&
         ServiceFeeUtil::checkIsDateBetween(optSrvInfo.createDate(), optSrvInfo.expireDate(), data);
}

bool
OptionalServicesValidator::checkAdvPurchaseTktInd(const OptionalServicesInfo& optSrvInfo) const
{
  return !_trx.getOptions()->isTicketingInd() || optSrvInfo.advPurchTktIssue() != 'X';
}

bool
OptionalServicesValidator::checkTravelDate(const OptionalServicesInfo& optSrvInfo) const
{
  const DateTime& firstTvlDate = (*_segI)->departureDT();

  if (ServiceFeeUtil::isStartDateSpecified(optSrvInfo) ||
      ServiceFeeUtil::isStopDateSpecified(optSrvInfo))
  {
    uint32_t tvlStartYear, tvlStartMonth, tvlStartDay, tvlStopYear, tvlStopMonth, tvlStopDay;

    if (ServiceFeeUtil::isStartDateSpecified(optSrvInfo))
    {
      tvlStartYear = optSrvInfo.tvlStartYear();
      tvlStartMonth = optSrvInfo.tvlStartMonth();
      tvlStartDay = optSrvInfo.tvlStartDay();
    }
    else
    {
      tvlStartYear = optSrvInfo.ticketEffDate().year();
      tvlStartMonth = optSrvInfo.ticketEffDate().month();
      tvlStartDay = optSrvInfo.ticketEffDate().day();
    }

    if (ServiceFeeUtil::isStopDateSpecified(optSrvInfo))
    {
      tvlStopYear = optSrvInfo.tvlStopYear();
      tvlStopMonth = optSrvInfo.tvlStopMonth();
      tvlStopDay = optSrvInfo.tvlStopDay();
    }
    else
    {
      tvlStopYear = optSrvInfo.ticketDiscDate().year();
      tvlStopMonth = optSrvInfo.ticketDiscDate().month();
      tvlStopDay = optSrvInfo.ticketDiscDate().day();
    }
    return tvlStartMonth && tvlStopMonth && // Months has to be valid number
           firstTvlDate.isBetweenF(
               tvlStartYear, tvlStartMonth, tvlStartDay, tvlStopYear, tvlStopMonth, tvlStopDay);
  }

  return (firstTvlDate.date() <= optSrvInfo.ticketDiscDate().date() &&
          firstTvlDate.date() >= optSrvInfo.ticketEffDate().date());
}

bool
OptionalServicesValidator::checkAccountCodes(uint32_t itemNo) const
{
  if (itemNo == 0)
    return true;

  SvcFeesAccountCodeValidator validator(
      _trx,
      (_diag && _diag->shouldCollectInRequestedContext(Diag877Collector::PROCESSING_ACCOUNT_CODES)
           ? _diag
           : nullptr));

  return svcFeesAccountCodeValidate(validator, itemNo);
}

bool
OptionalServicesValidator::checkFeeApplication(const OptionalServicesInfo& optSrvInfo,
                                               OCFees& ocFees) const
{
  switch (optSrvInfo.frequentFlyerMileageAppl())
  {
  case CHAR_BLANK: // No break
  case CHAR_BLANK2: // No break
  case '3': // No break
  case '4': // No break
  case '5':
    break;
  case '1':
    ocFees.feeAmount() *= _processedFares.size();
    break;
  case '2':
    ocFees.feeAmount() = halfOfAmount(ocFees) * _processedFares.size();
    break;
  default:
    return false;
  }

  return true;
}

MoneyAmount
OptionalServicesValidator::halfOfAmount_old(const OCFees& ocFees) const
{
  CurrencyConverter curConverter;
  RoundingFactor roundingFactor = 0;
  CurrencyNoDec roundingNoDec = 0;
  RoundingRule roundingRule = NONE;

  Money half(ocFees.feeAmount() / 2, ocFees.feeCurrency());
  if (getFeeRounding_old(ocFees.feeCurrency(), roundingFactor, roundingNoDec, roundingRule))
  {
    curConverter.round(half, roundingFactor, roundingRule);
  }

  return half.value();
}

MoneyAmount
OptionalServicesValidator::halfOfAmount(const OCFees& ocFees) const
{
  if (fallback::ocFeesAmountRoundingRefactoring(&_trx))
   return halfOfAmount_old(ocFees);

  Money half(ocFees.feeAmount() / 2, ocFees.feeCurrency());
  OCFees::OcAmountRounder ocAmountRounder(_trx);

  return ocAmountRounder.getRoundedFeeAmount(half);
}

bool
OptionalServicesValidator::retrieveSpecifiedFee(const OptionalServicesInfo& optSrvInfo,
                                                OCFees& ocFees) const
{
  if (optSrvInfo.serviceFeesCurrencyTblItemNo() == 0)
  {
    setSrvInfo(optSrvInfo, ocFees);
    return true; // MATCH
  }

  const vector<SvcFeesCurrencyInfo*>& svcFeeCurInfo = getSvcFeesCurrency(optSrvInfo);
  if (svcFeeCurInfo.empty())
    return false; // NOMATCH

  displaySvcFeeCurInfoHeader(optSrvInfo.serviceFeesCurrencyTblItemNo());

  SvcFeesCurrencyInfo* emptyLocSfcInfo(nullptr);
  for (SvcFeesCurrencyInfo* sfcInfo : svcFeeCurInfo)
  {
    if (validateSfcLocation(optSrvInfo.vendor(), sfcInfo->posLoc(), getLocForSfcValidation()))
    {
      setOCFees(optSrvInfo, ocFees, *sfcInfo); // set OCFees for the matched record with POS
                                               // location
      displaySvcFeeCurInfoDetail(*sfcInfo, true);
      return true; // MATCH
    }
    else
    {
      displaySvcFeeCurInfoDetail(*sfcInfo, false);
      if (!emptyLocSfcInfo && sfcInfo->posLoc().isNull())
        emptyLocSfcInfo = sfcInfo;
    }
  }

  // first check if there are any empty posloc records in DB if yes set the OCfees with this
  if (emptyLocSfcInfo)
  {
    setOCFees(optSrvInfo, ocFees, *emptyLocSfcInfo);
    displaySvcFeeCurInfoHeaderAndDetail(*emptyLocSfcInfo, true, true);
  }
  else // Set OCFees from the first record of SVCFEESCURRENCY
  {
    SvcFeesCurrencyInfo* firtsSfcInfo = *(svcFeeCurInfo.begin());
    setOCFees(optSrvInfo, ocFees, *firtsSfcInfo);
    displaySvcFeeCurInfoHeaderAndDetail(*firtsSfcInfo, true, false);
  }
  return true; // MATCH
}

bool
OptionalServicesValidator::validateSfcLocation(const VendorCode& vendor,
                                               const LocKey& locKey,
                                               const Loc& loc) const
{
  if (locKey.isNull())
    return false;

  return isInLoc(vendor, locKey, loc);
}

const Loc&
OptionalServicesValidator::getLocForSfcValidation() const
{
  return *(_trx.getRequest()->ticketingAgent()->agentLocation());
}

void
OptionalServicesValidator::setOCFees(const OptionalServicesInfo& optSrvInfo,
                                     OCFees& ocFees,
                                     SvcFeesCurrencyInfo& svcInfo) const
{
  ocFees.feeAmount() = svcInfo.feeAmount();
  ocFees.feeCurrency() = svcInfo.currency();
  ocFees.feeNoDec() = svcInfo.noDec();
  setCurrencyNoDec(ocFees);
  setSrvInfo(optSrvInfo, ocFees);
}

void
OptionalServicesValidator::setSrvInfo(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees) const
{
  if (optSrvInfo.vendor() != ATPCO_VENDOR_CODE)
    ocFees.isFeeGuaranteed() = false;

  ocFees.optFee() = &optSrvInfo;
}

void
OptionalServicesValidator::displaySvcFeeCurInfoHeader(const int itemNo) const
{
  if (_diag != nullptr)
  {
    _diag->printSvcFeeCurTable170Header(itemNo);
  }
}

void
OptionalServicesValidator::displaySvcFeeCurInfoDetail(SvcFeesCurrencyInfo& svcInfo, bool status)
    const
{
  if (_diag != nullptr)
  {
    _diag->printSvcFeeCurInfoDetail(svcInfo, status);
  }
}

void
OptionalServicesValidator::displayStartStopTimeOrDOWFailDetail(bool isErrorFromDOW) const
{
  if (_diag != nullptr && isDDInfo())
  {
    _diag->printStartStopTimeHeader(isErrorFromDOW);
  }
}

void
OptionalServicesValidator::displaySvcFeeCurInfoHeaderAndDetail(SvcFeesCurrencyInfo& svcInfo,
                                                               bool status,
                                                               bool isNullHeader) const
{
  if (_diag != nullptr)
  {
    _diag->printSvcFeeCurInfo170Label(isNullHeader);
    _diag->printSvcFeeCurInfoDetail(svcInfo, status);
  }
}

bool
OptionalServicesValidator::checkInputTicketDesignator(uint32_t itemNo) const
{
  if (itemNo == 0)
    return true;

  SvcFeesInputTktDesigValidator validator(
      _trx,
      _itin,
      (_diag && _diag->shouldCollectInRequestedContext(
                    Diag877Collector::PROCESSING_INPUT_TICKET_DESIG)
           ? _diag
           : nullptr));
  return svcFeesTktDesignatorValidate(validator, itemNo);
}

bool
OptionalServicesValidator::checkOutputTicketDesignator(const OptionalServicesInfo& optSrvInfo) const
{
  return checkOutputTicketDesignatorForSegmentsRange(optSrvInfo, _segI, _segIE);
}

bool
OptionalServicesValidator::checkOutputTicketDesignatorForSegmentsRange(
    const OptionalServicesInfo& optSrvInfo,
    const std::vector<TravelSeg*>::const_iterator segBegin,
    const std::vector<TravelSeg*>::const_iterator segEnd) const
{
  if (optSrvInfo.resultServiceFeesTktDesigTblItemNo() == 0) // No T173
    return true;

  if (optSrvInfo.serviceFeesCxrResultingFclTblItemNo() == 0 && // No T171
      optSrvInfo.ruleTariff() != (uint16_t) - 1 &&
      !optSrvInfo.rule().empty() && !_isOneCarrier &&
      (!optSrvInfo.ruleTariffInd().empty() || optSrvInfo.fareInd() != ' '))
  {
    return false;
  }

  SvcFeesOutputTktDesigValidator validator(
      _trx,
      *_farePath,
      segBegin,
      segEnd,
      (_diag && _diag->shouldCollectInRequestedContext(
                    Diag877Collector::PROCESSING_OUTPUT_TICKET_DESIG)
           ? _diag
           : nullptr));

  return svcFeesTktDesignatorValidate(validator, optSrvInfo.resultServiceFeesTktDesigTblItemNo());
}

bool
OptionalServicesValidator::checkInputPtc(const OptionalServicesInfo& optSrvInfo) const
{
  ServiceFeeUtil util(_trx);
  return inputPtcValidate(util, optSrvInfo);
}

bool
OptionalServicesValidator::validateFrom(const OptionalServicesInfo& optSrvInfo,
                                        const Loc& orig,
                                        const Loc& dest,
                                        LocCode* loc1,
                                        LocCode* loc2) const
{
  return validateLocation(optSrvInfo.vendor(),
                          optSrvInfo.loc1(),
                          orig,
                          optSrvInfo.loc1ZoneTblItemNo(),
                          false,
                          optSrvInfo.carrier(),
                          loc1) &&
         validateLocation(optSrvInfo.vendor(),
                          optSrvInfo.loc2(),
                          dest,
                          optSrvInfo.loc2ZoneTblItemNo(),
                          true,
                          optSrvInfo.carrier(),
                          loc2);
}

bool
OptionalServicesValidator::validateBetween(const OptionalServicesInfo& optSrvInfo,
                                           const Loc& orig,
                                           const Loc& dest,
                                           LocCode* loc1,
                                           LocCode* loc2) const
{
  return (optSrvInfo.loc1().isNull() && optSrvInfo.loc2().isNull()) ||
         validateFrom(optSrvInfo, orig, dest, loc1, loc2) ||
         validateFrom(optSrvInfo, dest, orig, loc2, loc1);
}

bool
OptionalServicesValidator::validateWithin(const OptionalServicesInfo& optSrvInfo,
                                          LocCode* loc1,
                                          LocCode* loc2) const
{
  if (!validateLocation(optSrvInfo.vendor(),
                        optSrvInfo.loc1(),
                        *(*_segI)->origin(),
                        optSrvInfo.loc1ZoneTblItemNo(),
                        false,
                        optSrvInfo.carrier(),
                        loc1))
  {
    return false;
  }

  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
  {
    if (!validateLocation(optSrvInfo.vendor(),
                          optSrvInfo.loc1(),
                          *(*i)->destination(),
                          optSrvInfo.loc1ZoneTblItemNo(),
                          false,
                          optSrvInfo.carrier(),
                          (i + 1 == _segIE) ? loc2 : nullptr))
    {
      return false;
    }
  }

  return true;
}

bool
OptionalServicesValidator::checkGeoFtwInd(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
    const
{
  const Loc& orig = *(*_segI)->origin();
  const Loc& dest = *(*(_segIE - 1))->destination();

  switch (optSrvInfo.fromToWithinInd())
  {
  case FTW_FROM:
    return validateFrom(optSrvInfo,
                        orig,
                        dest,
                        &ocFees.matchedOriginAirport(),
                        &ocFees.matchedDestinationAirport());
  case FTW_TO:
    return validateFrom(optSrvInfo,
                        dest,
                        orig,
                        &ocFees.matchedOriginAirport(),
                        &ocFees.matchedDestinationAirport());
  case CHAR_BLANK: // BETWEEN
  case CHAR_BLANK2: // no break here
    return validateBetween(optSrvInfo,
                           orig,
                           dest,
                           &ocFees.matchedOriginAirport(),
                           &ocFees.matchedDestinationAirport());
  case FTW_WITHIN:
    return validateWithin(
        optSrvInfo, &ocFees.matchedOriginAirport(), &ocFees.matchedDestinationAirport());
  case FTW_RULE_BUSTER_A: // no break here
  case FTW_RULE_BUSTER_B:
    return false;
  default:
    break;
    // do nothing
  }
  return false;
}

bool
OptionalServicesValidator::validateViaWithLoc2(const OptionalServicesInfo& optSrvInfo,
                                               vector<TravelSeg*>& passedLoc3Dest) const
{
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; i++)
  {
    // Point of travel matched with S7 VIA point which is the same location as origin
    // or destination of travel should be skipped in validation

    if ((*i)->destination() == (*_segI)->origin() ||
        (*i)->destination() == (*(_segIE-1))->destination())
    {
      continue;
    }

    // continue with intermediates points
    if (validateLocation(optSrvInfo.vendor(), // match via
                         optSrvInfo.viaLoc(),
                         *(*i)->destination(),
                         optSrvInfo.viaLocZoneTblItemNo(),
                         false,
                         optSrvInfo.carrier(),
                         nullptr))
    {
      if (optSrvInfo.stopCnxDestInd() == CHAR_BLANK || optSrvInfo.stopCnxDestInd() == CHAR_BLANK2)
      {
        return true; // we don't need to collect all segments
      }
      passedLoc3Dest.push_back(*i);
    }
  }

  return !passedLoc3Dest.empty();
}

bool
OptionalServicesValidator::validateViaWithStopover(const OptionalServicesInfo& optSrvInfo) const
{
  // via loc can't match end point
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; i++)
  {
    if (i + 1 != _endOfJourney && // we found first stopover, so we fail
        isStopover(optSrvInfo, *i, *(i + 1)))
    {
      return false;
    }

    if (validateLocation(optSrvInfo.vendor(), // match via
                         optSrvInfo.viaLoc(),
                         *(*i)->destination(),
                         optSrvInfo.viaLocZoneTblItemNo(),
                         false,
                         optSrvInfo.carrier(),
                         nullptr))
    {
      return true;
    }
  }

  return false;
}

bool
OptionalServicesValidator::isStopover(const OptionalServicesInfo& optSrvInfo,
                                      const TravelSeg* seg,
                                      const TravelSeg* next) const
{
  if (optSrvInfo.stopoverUnit() != CHAR_BLANK && optSrvInfo.stopoverUnit() != CHAR_BLANK2)
  {
    char unit[2] = { optSrvInfo.stopoverUnit(), '\0' };

    PeriodOfStay maxStay(optSrvInfo.stopoverTime(), unit);
    DateTime startTime = seg->arrivalDT();
    DateTime endTime = next->departureDT();

    // Special treatment for Arunk/Surface
    if (seg->segmentType() == Surface || seg->segmentType() == Arunk)
    {
      startTime = seg->departureDT();
    }
    else if (next->segmentType() == Surface || next->segmentType() == Arunk)
    {
      endTime = next->arrivalDT();
    }

    DateTime calcStartTime;
    if (!optSrvInfo.stopoverTime().empty())
      calcStartTime = RuleUtil::addPeriodToDate(startTime, maxStay);
    else
      calcStartTime = startTime;

    if (optSrvInfo.stopoverUnit() == 'D' || optSrvInfo.stopoverUnit() == 'M')
    {
      if (optSrvInfo.stopoverTime().empty())
      {
        return !(calcStartTime.day() == endTime.day());
      }
      return (!endTime.isBetweenF(startTime.year(),
                                  startTime.month(),
                                  startTime.day(),
                                  calcStartTime.year(),
                                  calcStartTime.month(),
                                  calcStartTime.day()));
    }

    return endTime > calcStartTime;
  }

  if (_geoTravelType == GeoTravelType::International || _geoTravelType == GeoTravelType::ForeignDomestic ||
      _isIntrnl)
    return next->isStopOverWithOutForceCnx(seg, SECONDS_PER_DAY);
  else
    return next->isStopOverWithOutForceCnx(seg, SECONDS_PER_HOUR * 4);
}

bool
OptionalServicesValidator::checkIntermediatePoint(const OptionalServicesInfo& optSrvInfo,
                                                  vector<TravelSeg*>& passedLoc3Dest,
                                                  OCFees& ocFees) const
{
  if (optSrvInfo.viaLoc().isNull())
    return true;

  if ((optSrvInfo.sectorPortionInd() == SEC_POR_IND_PORTION || getPortionBG()) &&
      !optSrvInfo.loc1().isNull())
  {
    if (!optSrvInfo.loc2().isNull())
    {
      return validateViaWithLoc2(optSrvInfo, passedLoc3Dest);
    }
    else if (optSrvInfo.stopCnxDestInd() == SCD_STOPOVER_WITH_GEO)
    {
      return validateViaWithStopover(optSrvInfo);
    }
  }

  return false;
}

bool
OptionalServicesValidator::checkStopCnxDestInd(const OptionalServicesInfo& optSrvInfo,
                                               const std::vector<TravelSeg*>& passedLoc3Dest,
                                               OCFees& ocFees) const
{
  if ((optSrvInfo.sectorPortionInd() == SEC_POR_IND_PORTION || getPortionBG()) &&
      distance(_segI, _segIE) == 1)
    return true;

  switch (optSrvInfo.stopCnxDestInd())
  {
  case SCD_CNX_POINT:
    return validateCnxOrStopover(optSrvInfo, passedLoc3Dest, true);
  case SCD_NO_FARE_BREAK:
    return validateNoFareBreak(optSrvInfo, passedLoc3Dest);
  case SCD_NO_FARE_BREAK_OR_STOPOVER:
    return validateNoFareBreak(optSrvInfo, passedLoc3Dest) &&
           validateCnxOrStopover(optSrvInfo, passedLoc3Dest, true);
  case SCD_STOPOVER:
    return validateCnxOrStopover(optSrvInfo, passedLoc3Dest, false);
  case SCD_STOPOVER_WITH_GEO:
    return (_segIE - 1) == findFirstStopover(optSrvInfo);
  default:
    break; // do nothing
  }

  return true;
}

bool
OptionalServicesValidator::validateCnxOrStopover(const OptionalServicesInfo& optSrvInfo,
                                                 const vector<TravelSeg*>& passedLoc3Dest,
                                                 bool validateCnx) const
{
  if (optSrvInfo.sectorPortionInd() != SEC_POR_IND_PORTION && !getPortionBG())
    return false;

  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; i++)
  {
    if (!passedLoc3Dest.empty() && !findSegmentInVector(passedLoc3Dest, *i))
    {
      continue; // we process only segments withc passed Loc3 validation
    }

    if (isStopover(optSrvInfo, *i, *(i + 1)) == validateCnx)
    {
      return false; // fail when stopover
    }
  }

  return true;
}

bool
OptionalServicesValidator::validateNoFareBreak(const OptionalServicesInfo& optSrvInfo,
                                               const vector<TravelSeg*>& passedLoc3Dest) const
{
  if (optSrvInfo.sectorPortionInd() != SEC_POR_IND_PORTION && !getPortionBG())
    return false;

  vector<TravelSeg*> fareBreaks;
  getFareBreaks(fareBreaks);

  // match only intermediate points
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE - 1; i++)
  {
    if (!passedLoc3Dest.empty() && !findSegmentInVector(passedLoc3Dest, *i))
    {
      continue; // we process only segments withc passed Loc3 validation
    }

    if (findSegmentInVector(fareBreaks, *i))
    {
      return false; // fail if find fare break
    }
  }

  return true;
}

bool
OptionalServicesValidator::findSegmentInVector(const vector<TravelSeg*>& vec,
                                               const TravelSeg* segment) const
{
  if (_trx.getTrxType() == PricingTrx::PRICING_TRX)
  {
    for (const auto elem : vec)
    {
      if (elem->segmentOrder() == segment->segmentOrder())
        return true;
    }
  }
  else
  {
    const Itin* itin = &_itin;
    assert(itin && "Itin is null");

    for (const auto elem : vec)
    {
      if (itin->segmentOrder(elem) == itin->segmentOrder(segment))
        return true;
    }
  }

  return false;
}

void
OptionalServicesValidator::getFareBreaks(vector<TravelSeg*>& fareBreaks) const
{
  for (const PricingUnit* pu : _farePath->pricingUnit())
  {
    if (pu->isSideTripPU())
      continue;

    for (const FareUsage* fu : pu->fareUsage())
      fareBreaks.push_back(fu->travelSeg().back());
  }
}

vector<TravelSeg*>::const_iterator
OptionalServicesValidator::findFirstStopover(const OptionalServicesInfo& optSrvInfo) const
{
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE && i + 1 < _endOfJourney; i++)
  {
    if (isStopover(optSrvInfo, *i, *(i + 1)))
    {
      return i;
    }
  }

  return _endOfJourney;
}

void
OptionalServicesValidator::checkDiagS7ForDetail(const OptionalServicesInfo* optSrvInfo) const
{
  if (UNLIKELY(_diag != nullptr && isDDInfo()))
  {
    if (!isDiagSequenceMatch(*optSrvInfo))
    {
      return;
    }
    _diag->printS7DetailInfo(optSrvInfo, _trx);
  }
}

void
OptionalServicesValidator::printDiagS7Info(const OptionalServicesInfo* optSrvInfo,
                                           const OCFees& ocFees,
                                           const StatusS7Validation& rc,
                                           const bool markAsSelected) const
{
  if (UNLIKELY(_diag != nullptr))
  {
    if (!isDiagSequenceMatch(*optSrvInfo))
    {
      return;
    }

    if (isDDPass() && (rc != PASS_S7 && rc != SOFT_PASS_S7 && rc != PASS_S7_FREE_SERVICE &&
                       rc != PASS_S7_NOT_AVAIL))
    {
      return;
    }

    printT183TableBufferedData();

    if (!isDDInfo())
    {
      _diag->printS7OptionalFeeInfo(optSrvInfo, ocFees, rc, markAsSelected);
    }
    else
    {
      if (ocFees.isAnyS7SoftPass())
        _diag->printS7SoftMatchedFields(optSrvInfo, ocFees);
      _diag->printS7OptionalServiceStatus(rc);
      _diag->lineSkip(0);
      _diag->printS7RecordValidationFooter(*optSrvInfo, _trx);
    }
  }
}
void
OptionalServicesValidator::printStopAtFirstMatchMsg() const
{
  if (_diag != nullptr)
    _diag->printStopAtFirstMatchMsg();
}

void
OptionalServicesValidator::printT183TableBufferedData() const
{
}

void
OptionalServicesValidator::printDiagS7NotFound(const SubCodeInfo& subcode) const
{
  if (_diag != nullptr && !isDDInfo())
    _diag->printS7NotFound(subcode);
}

bool
OptionalServicesValidator::isDiagSequenceMatch(const OptionalServicesInfo& info) const
{
  if (_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).empty())
    return true;

  return info.seqNo() ==
         (uint32_t)atoi(_trx.diagnostic().diagParamMapItem(Diagnostic::SEQ_NUMBER).c_str());
}

bool
OptionalServicesValidator::isDDInfo() const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "INFO";
}

bool
OptionalServicesValidator::isDDPass() const
{
  return _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "PASSED";
}

bool
OptionalServicesValidator::checkSecurity(const OptionalServicesInfo& optSrvInfo, OCFees& ocFees)
    const
{
  if (optSrvInfo.serviceFeesSecurityTblItemNo() <= 0)
  {
    return optSrvInfo.publicPrivateInd() != T183_SCURITY_PRIVATE;
  }
  bool view = false;
  SecurityValidator securityValidator(_trx, _segI, _segIE);

  if (!securityValidator.validate(optSrvInfo.seqNo(),
                                  optSrvInfo.serviceFeesSecurityTblItemNo(),
                                  view,
                                  optSrvInfo.vendor(),
                                  _diag))
    return false;

  if (view)
    ocFees.setDisplayOnly(true);
  return true;
}

bool
OptionalServicesValidator::skipUpgradeCheck(const OptionalServicesInfo& optSrvInfo,
                                            const OCFees& ocFees) const
{
  if (!fallback::fallback_AB240_UpgradeCheckForUPGroupCode(&_trx))
  {
    if (_trx.activationFlags().isAB240() && skipUpgradeForUpGroupCode(ocFees.subCodeInfo()))
      return true;
  }

  return skipUpgradeCheckCommon(optSrvInfo, ocFees);
}

bool
OptionalServicesValidator::skipUpgradeCheckCommon(const OptionalServicesInfo& optSrvInfo,
                                                  const OCFees& ocFees) const
{
  if ((!ocFees.subCodeInfo()->serviceGroup().equalToConst("SA") &&
       optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() != 0) ||
      optSrvInfo.upgradeCabin() != BLANK)
    return false;
  return true;
}

bool
OptionalServicesValidator::skipUpgradeForUpGroupCode(const SubCodeInfo* s5) const
{
  bool path = (_trx.billing() && _trx.billing()->requestPath() == AEBSO_PO_ATSE_PATH) ||
              _trx.activationFlags().isAB240(); // isAB240 already checks the requestPath
  bool isUP = s5 && s5->serviceGroup().equalToConst("UP");

  return isUP && path && TrxUtil::isRequestFromAS(_trx);
}

bool
OptionalServicesValidator::checkUpgradeT198(const OptionalServicesInfo& optSrvInfo,
                                            const OCFees& ocFees) const
{
  if (ocFees.subCodeInfo()->serviceGroup().equalToConst("SA") &&
      optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo() != 0)
  {
    const vector<SvcFeesResBkgDesigInfo*>& padisInfos = _trx.dataHandle().getSvcFeesResBkgDesig(
        optSrvInfo.vendor(), optSrvInfo.upgrdServiceFeesResBkgDesigTblItemNo());

    if (padisInfos.empty())
      return false;
  }
  return true;
}

bool
OptionalServicesValidator::checkSectorPortionInd(const Indicator sectorPortionInd) const
{
  return !(distance(_segI, _segIE) > 1 && sectorPortionInd == SEC_POR_IND_SECTOR);
}

bool
OptionalServicesValidator::checkSectorPortionIndForSectorOnP(const Indicator sectorPortionInd) const
{
  return (sectorPortionInd == SEC_POR_IND_SECTOR && distance(_segI, _segIE) == 1);
}

bool
OptionalServicesValidator::checkSectorPortionIndForBG(const OptionalServicesInfo& optSrvInfo) const
{
  if (optSrvInfo.sectorPortionInd() == SEC_POR_IND_PORTION ||
      optSrvInfo.sectorPortionInd() == SEC_POR_IND_JOURNEY ||
      (optSrvInfo.sectorPortionInd() == CHAR_BLANK && checkAllGeoForBlank(optSrvInfo)))
    setPortionBG(true);
  else if (checkSectorPortionIndForSectorOnP(optSrvInfo.sectorPortionInd()))
    return true;
  else
    setPortionBG();

  return getPortionBG();
}

void
OptionalServicesValidator::setPortionBG(bool value) const
{
  _portionBG = value;
}

bool
OptionalServicesValidator::checkAllGeoForBlank(const OptionalServicesInfo& optSrvInfo) const
{
  return (
      optSrvInfo.loc1().isNull() &&
      (optSrvInfo.loc1ZoneTblItemNo().empty() || optSrvInfo.loc1ZoneTblItemNo() == "0000000") &&
      optSrvInfo.loc2().isNull() &&
      (optSrvInfo.loc2ZoneTblItemNo().empty() || optSrvInfo.loc2ZoneTblItemNo() == "0000000") &&
      optSrvInfo.viaLoc().isNull() &&
      (optSrvInfo.viaLocZoneTblItemNo().empty() || optSrvInfo.viaLocZoneTblItemNo() == "0000000") &&
      (optSrvInfo.stopCnxDestInd() == CHAR_BLANK || optSrvInfo.stopCnxDestInd() == CHAR_BLANK2) &&
      (optSrvInfo.stopoverUnit() == CHAR_BLANK || optSrvInfo.stopoverUnit() == CHAR_BLANK2));
}

bool
OptionalServicesValidator::isRBDValid(AirSeg* seg,
                                      const vector<SvcFeesResBkgDesigInfo*>& rbdInfos,
                                      OCFees& ocFees) const
{
  const auto tsI = _ts2ss.find(seg);
  const PaxTypeFare::SegmentStatus* stat = (tsI != _ts2ss.end()) ? tsI->second.first : nullptr;

  BookingCode bookingCode = (stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
                             !stat->_bkgCodeReBook.empty())
                                ? stat->_bkgCodeReBook
                                : seg->getBookingCode();

  Diag877Collector* const dc =
      (_diag && _diag->shouldCollectInRequestedContext(Diag877Collector::PROCESSING_RBD) ? _diag
                                                                                         : nullptr);

  return find_if(rbdInfos.begin(),
                 rbdInfos.end(),
                 boost::bind<bool>(IsRBD(bookingCode), _1, *seg, _isMarketingCxr, dc)) !=
         rbdInfos.end();
}

bool
OptionalServicesValidator::checkRBD(const VendorCode& vendor,
                                    const uint32_t serviceFeesResBkgDesigTblItemNo,
                                    OCFees& ocFees) const
{
  if (!serviceFeesResBkgDesigTblItemNo)
    return true;

  const vector<SvcFeesResBkgDesigInfo*>& rbdInfos =
      getRBDInfo(vendor, serviceFeesResBkgDesigTblItemNo);
  if (UNLIKELY(rbdInfos.empty()))
    return false;

  if (UNLIKELY(_diag))
    _diag->printRBDTable198Header(serviceFeesResBkgDesigTblItemNo);

  return std::all_of(_airSegs.begin(),
                     _airSegs.end(),
                     [&](AirSeg* as)
                     { return this->isRBDValid(as, rbdInfos, ocFees); });
}

CabinType
OptionalServicesValidator::mapS7CabinType(const Indicator cabin) const
{
  Indicator s7cabin;

  CabinType s7CabinType;
  if(!TrxUtil::isAtpcoRbdByCabinAnswerTableActivated(_trx))
  {
    switch (cabin)
    {
    case 'R':
      s7cabin = 'P';
      break;
    case 'P':
      s7cabin = 'S';
      break;
    default:
      s7CabinType.setClassFromAlphaNum(cabin);
      return s7CabinType;
    }
    s7CabinType.setClassFromAlphaNum(s7cabin);
  }
  else
  {
    switch (cabin)
    {
    case 'P':
      s7cabin = 'W';
      break;
    default:
      s7CabinType.setClassFromAlphaNumAnswer(cabin);
      return s7CabinType;
    }
    s7CabinType.setClassFromAlphaNumAnswer(s7cabin);
  }
  return s7CabinType;
}

bool
OptionalServicesValidator::checkCabinData(AirSeg& seg,
                                          const CabinType& cabin,
                                          const CarrierCode& carrier,
                                          OCFees& ocFees) const
{
  const auto tsI = _ts2ss.find(&seg);
  const PaxTypeFare::SegmentStatus* stat = (tsI != _ts2ss.end()) ? tsI->second.first : nullptr;

  const CabinType cabinType = (stat && stat->_bkgCodeSegStatus.isSet(PaxTypeFare::BKSS_REBOOKED) &&
                               stat->_reBookCabin.isValidCabin())
                                  ? stat->_reBookCabin
                                  : seg.bookedCabin();

  return carrier == (_isMarketingCxr ? seg.carrier() : seg.operatingCarrierCode()) &&
         cabin == cabinType;
}

bool
OptionalServicesValidator::checkCabin(const Indicator cabin,
                                      const CarrierCode& carrier,
                                      OCFees& ocFees) const
{
  if (cabin == BLANK)
    return true;

  typedef vector<AirSeg*>::const_iterator AirSegI;
  for (AirSegI segI = _airSegs.begin(); segI != _airSegs.end(); ++segI)
  {
    CabinType s7CabinType = mapS7CabinType(cabin);
    if (!checkCabinData(**segI, s7CabinType, carrier, ocFees))
      return false;
  }
  return true;
}

bool
OptionalServicesValidator::checkFrequentFlyerStatus(const OptionalServicesInfo& optSrvInfo,
                                                    OCFees& ocFees) const
{
  if (optSrvInfo.frequentFlyerStatus() == 0)
    return true;

  for (const PaxType::FreqFlyerTierWithCarrier* ff : _paxType.freqFlyerTierWithCarrier())
  {
    if (ff->cxr() != optSrvInfo.carrier())
      continue;

    if (ff->freqFlyerTierLevel() > optSrvInfo.frequentFlyerStatus())
      continue;

    ocFees.isFeeGuaranteed() = false;
    return true;
  }

  return false;
}

bool
OptionalServicesValidator::checkTourCode(const OptionalServicesInfo& info) const
{
  if (info.tourCode().empty())
    return true; // When blank match S7

  // check if it is a cat 35 fare and if there is any tour code
  // if so validate it against S7 - tour code
  const CollectedNegFareData* negFareData = _farePath->collectedNegFareData();
  bool cat35 = negFareData && negFareData->indicatorCat35();
  if (cat35)
  {
    const string& cat35TourCode = negFareData->tourCode();
    if (!cat35TourCode.empty())
    {
      if (info.tourCode() == cat35TourCode)
        return true;
    }
  }

  // check if cat 27 tour code is equal to S7 - tour code
  const string& cat27TourCode = _farePath->cat27TourCode();
  if (!cat27TourCode.empty())
  {
    if (info.tourCode() == cat27TourCode)
      return true;
  }
  // since no match to tour code to cat 35 or cat 27 fail tour code match
  // since tour code in S7 exists and didn't match
  // even when there is no tour code in cat 35 and cat 27 fail
  return false;
}

bool
OptionalServicesValidator::checkStartStopTime(const OptionalServicesInfo& info,
                                              bool& skipDOWCheck,
                                              OCFees& ocFees) const
{
  // Start stop time check
  // When TimeApplication is blank return true

  if (info.timeApplication() == BLANK)
    return true; // When blank match S7

  if (info.timeApplication() == 'D')
  {
    skipDOWCheck = true;
    if (info.startTime() == 0 && info.stopTime() == 0) // All Day Allowed
    {
      if (checkDOW(info))
        return true;

      // Fail due to DOW
      displayStartStopTimeOrDOWFailDetail(true);
      return false;
    }
    else
      return validateDOWAndStartAndStopTime(info, ocFees);
  }
  else if (info.timeApplication() == 'R')
  {
    skipDOWCheck = true;
    if (info.startTime() == 0 && info.stopTime() == 0) // All Day Allowed
    {
      if (checkDOWRange(info))
        return true;

      // Fail due to DOW Range
      displayStartStopTimeOrDOWFailDetail(true);
      return false;
    }
    else
      return validateDOWAndStartAndStopTime(info, ocFees);
  }
  displayStartStopTimeOrDOWFailDetail(false);
  return false;
}

uint16_t
OptionalServicesValidator::convertMinutesSinceMidnightToActualTime(const string& strTimeVal) const
{

  uint16_t minutesSinceMidnight;
  minutesSinceMidnight = std::atoi(strTimeVal.c_str());
  uint16_t newHour = minutesSinceMidnight / 60;
  uint16_t newMinute = minutesSinceMidnight % 60;
  uint16_t timeVal = ((newHour * 100) + newMinute);

  return timeVal;
}

bool
OptionalServicesValidator::validateDOWAndStartAndStopTime(const OptionalServicesInfo& info,
                                                          OCFees& ocFees) const
{

  if (info.timeApplication() == 'D')
  {
    if (checkDOW(info))
      return validateStartAndStopTime(info, ocFees);
  }
  else
  {
    return checkDOWRangeAndTime(info, ocFees);
  }

  // Fail due to DOW or DOW Range
  displayStartStopTimeOrDOWFailDetail(true);
  return false;
}

bool
OptionalServicesValidator::validateStartAndStopTime(const OptionalServicesInfo& info,
                                                    OCFees& ocFees) const
{
  uint16_t departTime = convertMinutesSinceMidnightToActualTime((*_segI)->pssDepartureTime());

  if (departTime >= info.startTime() && departTime <= info.stopTime())
    return true;
  else
  {
    displayStartStopTimeOrDOWFailDetail(false);
    return false;
  }
}

bool
OptionalServicesValidator::validateStartAndStopTime(OCFees& ocFees) const
{
  return true;
}

bool
OptionalServicesValidator::isDOWstringValid(const std::string& dow) const
{
  // When single value for Range - Fail the S7
  // When sequence is out of order - Fail the S7
  if (dow.size() < 2 || dow.size() > 7)
    return false;

  std::string dowMask = "1234567123456";
  std::string::const_iterator maskIt = dowMask.begin() + (dow[0] - '1');
  for (std::string::const_iterator dowIt = dow.begin(); dowIt != dow.end(); dowIt++, maskIt++)
  {
    if (*dowIt != *maskIt)
      return false;
  }
  return true;
}

bool
OptionalServicesValidator::checkDOWRangeAndTime(const OptionalServicesInfo& info, OCFees& ocFees)
    const
{
  if (info.dayOfWeek().empty())
    return true; // When blank match S7

  if ((info.fltTktMerchInd() == MERCHANDISE_SERVICE ||
       info.fltTktMerchInd() == RULE_BUSTER_SERVICE))
    return false;

  // validate DOW Range for sector and portion
  if (info.sectorPortionInd() == SEC_POR_IND_SECTOR ||
      info.sectorPortionInd() == SEC_POR_IND_PORTION || getPortionBG())
  {
    string dow = info.dayOfWeek();
    std::string::iterator end_pos = std::remove(dow.begin(), dow.end(), ' ');
    dow.erase(end_pos, dow.end());

    if (!isDOWstringValid(dow))
    {
      displayStartStopTimeOrDOWFailDetail(true);
      return false;
    }

    // validate dow against first travel date of originating segment of the portion when it is
    // portion
    // When it is Segment it will be one flight segment only so validating against just that flight
    // segment is ok
    char dowDep = '0' + (*_segI)->departureDT().dayOfWeek();
    if (dowDep == '0')
      dowDep =
          '7'; // for SUN dayOfWeek is 0 where as for S7 dow data SUN is 7 so doing hard coding here

    if (dow.find(dowDep) != std::string::npos)
    {
      string::iterator itFirst = dow.begin();
      string::iterator itlast = dow.end() - 1;

      if (dowDep == (*itFirst))
      {
        uint16_t departTime = convertMinutesSinceMidnightToActualTime((*_segI)->pssDepartureTime());
        if (departTime >= info.startTime())
          return validateStartAndStopTime(ocFees);
      }
      else if (dowDep == (*itlast))
      {
        uint16_t departTime = convertMinutesSinceMidnightToActualTime((*_segI)->pssDepartureTime());
        if (departTime <= info.stopTime())
          return validateStartAndStopTime(ocFees);
      }
      else
        return validateStartAndStopTime(
            ocFees); // no need to validate time when in between dow value's

      displayStartStopTimeOrDOWFailDetail(false);
      return false;
    }
  }
  // Fail due to DOW Range
  displayStartStopTimeOrDOWFailDetail(true);
  return false;
}

bool
OptionalServicesValidator::checkDOWRange(const OptionalServicesInfo& info) const
{
  if (info.dayOfWeek().empty())
    return true; // When blank match S7

  if ((info.fltTktMerchInd() == MERCHANDISE_SERVICE ||
       info.fltTktMerchInd() == RULE_BUSTER_SERVICE))
    return false;

  // validate DOW Range for sector and portion
  if (info.sectorPortionInd() == SEC_POR_IND_SECTOR ||
      info.sectorPortionInd() == SEC_POR_IND_PORTION || getPortionBG())
  {
    string dow = info.dayOfWeek();
    std::string::iterator end_pos = std::remove(dow.begin(), dow.end(), ' ');
    dow.erase(end_pos, dow.end());

    if (!isDOWstringValid(dow))
    {
      displayStartStopTimeOrDOWFailDetail(true);
      return false;
    }

    // validate dow against first travel date of originating segment of the portion when it is
    // portion
    // When it is Segment it will be one flight segment only so validating against just that flight
    // segment is ok
    char dowDep = '0' + (*_segI)->departureDT().dayOfWeek();
    if (dowDep == '0')
      dowDep =
          '7'; // for SUN dayOfWeek is 0 where as for S7 dow data SUN is 7 so doing hard coding here

    return dow.find(dowDep) != std::string::npos;
  }
  return false;
}

bool
OptionalServicesValidator::checkDOW(const OptionalServicesInfo& info) const
{
  if (info.dayOfWeek().empty())
    return true; // When blank match S7

  if ((info.fltTktMerchInd() == MERCHANDISE_SERVICE ||
       info.fltTktMerchInd() == RULE_BUSTER_SERVICE))
    return false;

  // validate DOW Range for sector and portion
  if (info.sectorPortionInd() == SEC_POR_IND_SECTOR ||
      info.sectorPortionInd() == SEC_POR_IND_PORTION || getPortionBG())
  {
    string dow = info.dayOfWeek();

    // validate dow against first travel date of originating segment of the portion when it is
    // portion
    // When it is Segment it will be one flight segment only so validating against just that flight
    // segment is ok
    char dowDep = '0' + (*_segI)->departureDT().dayOfWeek();
    if (dowDep == '0')
      dowDep =
          '7'; // for SUN dayOfWeek is 0 where as for S7 dow data SUN is 7 so doing hard coding here

    return dow.find(dowDep) != std::string::npos;
  }
  return false;
}

bool
OptionalServicesValidator::checkEquipmentType(const OptionalServicesInfo& info, OCFees& ocFees)
    const
{
  if (info.equipmentCode().empty() || info.equipmentCode() == BLANK_CODE)
    return true;
  if (info.fltTktMerchInd() != FLIGHT_RELATED_SERVICE)
    return false;
  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
  {
    if ((*i)->equipmentType() != info.equipmentCode())
      return false;
  }
  return true;
}

bool
OptionalServicesValidator::checkAdvPur(const OptionalServicesInfo& info, OCFees& ocFees) const
{
  if (info.advPurchPeriod().empty() || info.advPurchPeriod() == BLANK_CODE)
    return true; // When no data in Period match

  if (shouldProcessAdvPur(info))
  {
    DateTime currentLocalTimeOfDepCity;
    getLocalTime(currentLocalTimeOfDepCity);
    ServicePurchaseUnit advPurUnit;
    ServicePurchasePeriod advPurPer;
    bool isAdvPUNHDM = true;

    if (!isAdvPurUnitNHDM(info))
    {
      uint32_t days = 0; // these two are for derived M70
      int advPinDays = 0;
      advPurPer = getAdvPurPeriod(info, currentLocalTimeOfDepCity, days, advPinDays);
      advPurUnit = 'D'; // converting weeks to days above so this indicator
      isAdvPUNHDM = false;
    }
    else
    {
      advPurPer = info.advPurchPeriod();
      advPurUnit = info.advPurchUnit();
    }

    // Do Adv Pur Validation
    PeriodOfStay advPurPeriod(advPurPer, advPurUnit);
    DateTime calcAllowedTravelDateTime =
        RuleUtil::addPeriodToDate(currentLocalTimeOfDepCity, advPurPeriod);

    // According to new acceptance criteria if departure day and adv pur day falls on same day fail
    // the validation
    // when it is dayofweek Adv Pur restrictions
    if (!isAdvPUNHDM && (*_segI)->departureDT().day() == calcAllowedTravelDateTime.day())
      return false;

    if ((*_segI)->departureDT() > calcAllowedTravelDateTime)
      return setPBDate(info, ocFees, calcAllowedTravelDateTime);

    if (advPurUnit == ServicePurchaseUnit('D'))
    {
      if ((*_segI)->departureDT().day() == calcAllowedTravelDateTime.day())
        return setPBDate(info, ocFees, calcAllowedTravelDateTime);
    }
    else if ((info.advPurchUnit() == ServicePurchaseUnit('M')))
    {
      if ((*_segI)->departureDT().month() == calcAllowedTravelDateTime.month() &&
          (*_segI)->departureDT().day() == calcAllowedTravelDateTime.day())
        return setPBDate(info, ocFees, calcAllowedTravelDateTime);
    }
  }
  return false;
  // Checking for ADVPURCHTKTISSUE will be done in Pricing Display Phase
}

bool
OptionalServicesValidator::shouldProcessAdvPur(const OptionalServicesInfo& info) const
{
  return (info.fltTktMerchInd() == FLIGHT_RELATED_SERVICE ||
          info.fltTktMerchInd() == PREPAID_BAGGAGE);
}

bool
OptionalServicesValidator::isAdvPurUnitNHDM(const OptionalServicesInfo& info) const
{
  if (info.advPurchUnit() == ServicePurchaseUnit('N') ||
      info.advPurchUnit() == ServicePurchaseUnit('H') ||
      info.advPurchUnit() == ServicePurchaseUnit('D') ||
      info.advPurchUnit() == ServicePurchaseUnit('M'))
    return true;
  else
    return false;
}

string
OptionalServicesValidator::getAdvPurPeriod(const OptionalServicesInfo& info,
                                           DateTime& calcTime,
                                           uint32_t& days,
                                           int& intRedAdvPurUnitInDays) const
{

  int dowCurTime = calcTime.dayOfWeek();
  int dbDOWDep = getDayOfWeek(info.advPurchPeriod());

  uint16_t intAdvPurUnit;
  istringstream stream(info.advPurchUnit());
  stream >> intAdvPurUnit;

  uint16_t redAdvPurUnit = intAdvPurUnit - 1; // reduce the number of weeks by 1 if 1 becomes 0
  intRedAdvPurUnitInDays = redAdvPurUnit * DAYS_PER_WEEK; // convert weeks to days
  uint32_t calcDays = 0;

  if (dowCurTime <= dbDOWDep)
  {
    calcDays = (dbDOWDep - dowCurTime); // EX MON - TUE
  }
  else
  {
    calcDays = DAYS_PER_WEEK - (dowCurTime - dbDOWDep); // EX THU - TUE
  }
  days = calcDays;
  calcDays += intRedAdvPurUnitInDays;

  stringstream ss;
  ss << calcDays;
  string strAdvPurPer = ss.str();
  return strAdvPurPer;
}

int
OptionalServicesValidator::getDayOfWeek(const std::string& dayOfWeekName) const
{
  int weekDay = -1;
  for (int i = 0; i < 7; i++)
  {
    if (dayOfWeekName == WEEKDAYS_UPPER_CASE[i])
    {
      weekDay = i;
      break;
    }
  }
  return weekDay;
}

void
OptionalServicesValidator::getLocalTime(DateTime& calcTime) const
{
  // get local time of departure city based on ticketing date
  DateTime time = _trx.ticketingDate();
  short utcOffset = getTimeDiff(time);
  if (utcOffset)
    calcTime = time.addSeconds(utcOffset * 60);
  else
    calcTime = time;
}

// get times difference between agent city location and deprature city location
short
OptionalServicesValidator::getTimeDiff(const DateTime& time) const
{
  short utcOffset = 0;
  // since titkceting time is of agent time -- we should send agent location as reference
  const Loc* pccLoc = _trx.getRequest()->ticketingAgent()->agentLocation();
  const Loc* departureOrigLoc = getLocation((*_segI)->origAirport(), time);

  if (pccLoc && departureOrigLoc)
  {
    if (getUTCOffsetDifference(*departureOrigLoc, *pccLoc, utcOffset, time))
      return utcOffset;
  }
  return 0;
}

bool
OptionalServicesValidator::getUTCOffsetDifference(const Loc& loc1,
                                                  const Loc& loc2,
                                                  short& utcoffset,
                                                  const DateTime& time) const
{
  return LocUtil::getUtcOffsetDifference(loc1, loc2, utcoffset, _trx.dataHandle(), time, time);
}

// get loc location
const Loc*
OptionalServicesValidator::getLocation(const LocCode& locCode, const DateTime& refTime) const
{
  return _trx.dataHandle().getLoc(locCode, refTime);
}

bool
OptionalServicesValidator::setPBDate(const OptionalServicesInfo& optSrvInfo,
                                     OCFees& ocFees,
                                     const DateTime& pbDate) const
{
  ocFees.purchaseByDate() = pbDate;
  return true;
}

StatusS7Validation
OptionalServicesValidator::checkServiceNotAvailNoCharge(const OptionalServicesInfo& info,
                                                        OCFees& ocFees) const
{
  if (info.notAvailNoChargeInd() == BLANK)
    return PASS_S7;

  if (info.notAvailNoChargeInd() == SERVICE_NOT_AVAILABLE)
  {
    if (_trx.getTrxType() == PricingTrx::MIP_TRX)
      return FAIL_S7_NOT_AVAIL_NO_CHANGE;
    ocFees.isFeeGuaranteed() = false;
    setSrvInfo(info, ocFees);
    return PASS_S7_NOT_AVAIL;
  }
  if (info.notAvailNoChargeInd() == SERVICE_FREE_NO_EMD_ISSUED ||
      info.notAvailNoChargeInd() == SERVICE_FREE_EMD_ISSUED ||
      info.notAvailNoChargeInd() == SERVICE_FREE_NO_BOOK_NO_EMD ||
      info.notAvailNoChargeInd() == SERVICE_FREE_NO_BOOK_EMD_ISSUED)
    setSrvInfo(info, ocFees);

  return PASS_S7_FREE_SERVICE;
}
bool
OptionalServicesValidator::isValidFareType(const PaxTypeFare* ptf,
                                           SvcFeesCxrResultingFCLInfo& fclInfo) const
{
  if (!fclInfo.fareType().empty() && ptf->fcaFareType() != fclInfo.fareType())
    return false;
  return true;
}

bool
OptionalServicesValidator::isValidFareClass(const PaxTypeFare* ptf,
                                            SvcFeesCxrResultingFCLInfo& fclInfo) const
{
  if (!fclInfo.resultingFCL().empty())
  {
    const TravelSeg* ptfTravelSeg = TravelSegUtil::lastAirSeg(ptf->fareMarket()->travelSeg());

    AncRequest* ancReq = dynamic_cast<AncRequest*>(_trx.getRequest());

    if (LIKELY(!ancReq))
    {
      if ((ptfTravelSeg && !ptfTravelSeg->specifiedFbc().empty() &&
           !RuleUtil::matchFareClass(fclInfo.resultingFCL().c_str(),
                                     ptfTravelSeg->specifiedFbc().c_str())) ||
          ((!ptfTravelSeg || ptfTravelSeg->specifiedFbc().empty()) &&
           !RuleUtil::matchFareClass(
               fclInfo.resultingFCL().c_str(),
               ServiceFeeUtil::getFareBasisRoot(_trx, ptf->createFareBasis(_trx, false)).c_str())))
        return false;
    }
    else
    {
      if ((ptfTravelSeg && !ptfTravelSeg->specifiedFbc().empty() &&
           !RuleUtil::matchFareClass(fclInfo.resultingFCL().c_str(),
                                     ptfTravelSeg->specifiedFbc().c_str())) ||
          ((!ptfTravelSeg || ptfTravelSeg->specifiedFbc().empty()) &&
           !RuleUtil::matchFareClass(
               fclInfo.resultingFCL().c_str(),
               ServiceFeeUtil::getFareBasisRoot(_trx, ptf->fare()->fareInfo()->fareClass().c_str())
                   .c_str())))
        return false;
    }
  }
  return true;
}

StatusT171
OptionalServicesValidator::isValidFareClassFareType(const PaxTypeFare* ptf,
                                                    SvcFeesCxrResultingFCLInfo& fclInfo,
                                                    OCFees& ocFees) const
{
  if (!isValidFareClass(ptf, fclInfo))
    return FAIL_ON_FARE_CLASS;

  if (!isValidFareType(ptf, fclInfo))
    return FAIL_NO_FARE_TYPE;

  return PASS_T171;
}

StatusT171
OptionalServicesValidator::isValidResultingFareClass(const PaxTypeFare* ptf,
                                                     SvcFeesCxrResultingFCLInfo& fclInfo,
                                                     OCFees& ocFees) const
{
  if (!fallback::fallbackGoverningCrxForT171(&_trx))
  {
    if (_trx.activationFlags().isAB240())
    {
      if (!isValidFareClassCarrier(ptf, fclInfo))
        return FAIL_ON_CXR;
    }
    else
    {
      if (ptf->fareMarket()->governingCarrier() != fclInfo.carrier())
        return FAIL_ON_CXR;
    }
  }
  else
  {
    if (ptf->fareMarket()->governingCarrier() != fclInfo.carrier())
      return FAIL_ON_CXR;
  }

  return isValidFareClassFareType(ptf, fclInfo, ocFees);
}


bool
OptionalServicesValidator::isValidFareClassCarrier(const PaxTypeFare* ptf,
                                                   const SvcFeesCxrResultingFCLInfo& fclInfo) const
{
  CarrierCode crx = ptf->carrier();

  if (crx.empty())
    crx = ptf->fareMarket()->governingCarrier();

  if (crx != fclInfo.carrier())
    return false;

  return true;
}

bool
OptionalServicesValidator::checkResultingFareClass(
    const VendorCode& vendor, const uint32_t serviceFeesCxrResultingFclTblItemNo, OCFees& ocFees)
    const
{
  if (!serviceFeesCxrResultingFclTblItemNo)
    return true;

  const std::vector<SvcFeesCxrResultingFCLInfo*>& resFCLInfo =
      getResFCInfo(vendor, serviceFeesCxrResultingFclTblItemNo);

  if (UNLIKELY(_diag))
    _diag->printResultingFareClassTable171Header(serviceFeesCxrResultingFclTblItemNo);

  bool res = false;

  for (const auto fare : _processedFares)
  {
    res = false;

    for (const auto rfi : resFCLInfo)
    {
      StatusT171 resT171 = isValidResultingFareClass(fare, *rfi, ocFees);
      printResultingFareClassInfo(*fare, *rfi, resT171);
      if (resT171 == PASS_T171)
      {
        res = true;
        break;
      }
      else if (UNLIKELY(resT171 == SOFTPASS_FARE_CLASS || resT171 == SOFTPASS_FARE_TYPE))
        res = true;
    }
    if (!res)
      return res;
  }
  return res;
}

bool
OptionalServicesValidator::checkMileageFee(const OptionalServicesInfo& info) const
{
  if (info.andOrInd() == 'A' ||
      (info.applicationFee() != 0 && info.serviceFeesCurrencyTblItemNo() == 0))
    return false;

  return true;
}

bool
OptionalServicesValidator::isValidRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd,
                                                TariffCategory tariffCategory) const
{
  return (ruleTariffInd == RULE_TARIFF_IND_PUBLIC && tariffCategory != RuleConst::PRIVATE_TARIFF) ||
         (ruleTariffInd == RULE_TARIFF_IND_PRIVATE && tariffCategory == RuleConst::PRIVATE_TARIFF);
}

bool
OptionalServicesValidator::checkRuleTariffInd(const ServiceRuleTariffInd& ruleTariffInd) const
{
  return ruleTariffInd.empty() ||
         std::all_of(_processedFares.begin(),
                     _processedFares.end(),
                     [&](PaxTypeFare* ptf)
                     { return this->isValidRuleTariffInd(ruleTariffInd, ptf->tcrTariffCat()); });
}

bool
OptionalServicesValidator::matchRuleTariff(const uint16_t& ruleTariff,
                                           const PaxTypeFare& ptf,
                                           OCFees& ocFees) const
{
  return ruleTariff == ptf.tcrRuleTariff() || ruleTariff == ptf.fareClassAppInfo()->_ruleTariff;
}

bool
OptionalServicesValidator::checkRuleTariff(const uint16_t& ruleTariff, OCFees& ocFees) const
{
  if (ruleTariff == (uint16_t) - 1)
    return true;

  for (const auto fare : _processedFares)
  {
    if (!matchRuleTariff(ruleTariff, *fare, ocFees))
      return false;
  }
  return true;
}

bool
OptionalServicesValidator::checkRule(const RuleNumber& rule, OCFees& ocFees) const
{
  return rule.empty() || std::all_of(_processedFares.begin(),
                                     _processedFares.end(),
                                     [&](PaxTypeFare* ptf)
                                     { return ptf->ruleNumber() == rule; });
}

bool
OptionalServicesValidator::checkFareInd(Indicator fareInd) const
{
  if (fareInd == CHAR_BLANK || fareInd == CHAR_BLANK2)
    return true;

  return std::all_of(_processedFares.begin(),
                     _processedFares.end(),
                     [=](PaxTypeFare* ptf)
                     { return this->checkFareInd(fareInd, *ptf); });
}

bool
OptionalServicesValidator::checkFareInd(Indicator fareInd, const PaxTypeFare& ptf) const
{
  switch (fareInd)
  {
  case FARE_IND_19_22: // but fail for cat 20 && cat 21
    return ServiceFeeUtil::isDiscounted19_22(ptf);
  case FARE_IND_25:
    return ptf.isFareByRule();
  case FARE_IND_35:
    return ptf.isNegotiated();
  default:
    break; // do nothing
  }

  return false;
}

bool
OptionalServicesValidator::checkCarrierFlightApplT186(const VendorCode& vendor,
                                                      const uint32_t itemNo,
                                                      OCFees& ocFees) const
{
  if (itemNo == 0)
    return true;

  if (_diag)
    _diag->printCarrierFlightT186Header(itemNo);

  const TaxCarrierFlightInfo* cxrFltT186 = getTaxCarrierFlight(vendor, itemNo);

  if (!cxrFltT186 || (cxrFltT186->segCnt() == 0) || cxrFltT186->segs().empty())
  {
    if (_diag)
      _diag->printNoCxrFligtT186Data();
    return false;
  }
  bool ret = false;

  for (vector<TravelSeg*>::const_iterator i = _segI; i < _segIE; i++)
  {
    AirSeg* seg = dynamic_cast<AirSeg*>(*i);
    if (!seg)
      continue;

    ret = false;
    vector<CarrierFlightSeg*>::const_iterator carrierFlightI = cxrFltT186->segs().begin();
    vector<CarrierFlightSeg*>::const_iterator carrierFlightEndI = cxrFltT186->segs().end();
    for (int i = 0; i < cxrFltT186->segCnt() && carrierFlightI != carrierFlightEndI;
         i++, carrierFlightI++)
    {
      CarrierFlightSeg* t186 = *carrierFlightI;

      StatusT186 rc186 = isValidCarrierFlight(*seg, *t186);
      if (_diag)
        _diag->printCarrierFlightApplT186Info(*t186, rc186);

      if (rc186 == PASS_T186)
      {
        ret = true;
        break;
      }
      else if (rc186 == SOFTPASS_FLIGHT)
      {
        ret = true;
        ocFees.softMatchS7Status().set(OCFees::S7_CARRIER_FLIGHT_SOFT);
        ocFees.softMatchCarrierFlightT186().push_back(t186);
        break;
      }
    }
    if (!ret)
      return ret;
  }

  return ret;
}

const TaxCarrierFlightInfo*
OptionalServicesValidator::getTaxCarrierFlight(const VendorCode& vendor, uint32_t itemNo) const
{
  return _trx.dataHandle().getTaxCarrierFlight(vendor, itemNo);
}

StatusT186
OptionalServicesValidator::isValidCarrierFlight(const AirSeg& air, CarrierFlightSeg& t186) const
{
  if (air.marketingCarrierCode() != t186.marketingCarrier())
    return FAIL_ON_MARK_CXR;
  if (!t186.operatingCarrier().empty() && air.operatingCarrierCode() != t186.operatingCarrier())
    return FAIL_ON_OPER_CXR;
  if (t186.flt1() == -1) // DB returns '-1' for '****'
    return PASS_T186;

  return isValidCarrierFlightNumber(air, t186);
}
StatusT186
OptionalServicesValidator::isValidCarrierFlightNumber(const AirSeg& air, CarrierFlightSeg& t186)
    const
{
  if ((t186.flt2() == 0 && air.flightNumber() != t186.flt1()) ||
      (t186.flt2() != 0 && (air.flightNumber() < t186.flt1() || air.flightNumber() > t186.flt2())))
    return FAIL_ON_FLIGHT;

  return PASS_T186;
}

bool
OptionalServicesValidator::checkCollectSubtractFee(const OptionalServicesInfo& info) const
{
  if (info.collectSubtractInd() != BLANK)
    return false;

  return true;
}

bool
OptionalServicesValidator::checkNetSellFeeAmount(const OptionalServicesInfo& info) const
{
  if (info.netSellingInd() != BLANK)
    return false;

  return true;
}

void
OptionalServicesValidator::setCurrencyNoDec(OCFees& fee) const
{
  const tse::Currency* currency = nullptr;
  DataHandle dataHandle;
  currency = dataHandle.getCurrency( fee.feeCurrency() );
  if (currency)
    fee.feeNoDec() = currency->noDec();
}

void
OptionalServicesValidator::printResultingFareClassInfo(const PaxTypeFare& ptf,
                                                       const SvcFeesCxrResultingFCLInfo& info,
                                                       StatusT171 status) const
{
  if (UNLIKELY(_diag))
  {
    if (!fallback::fallbackGoverningCrxForT171(&_trx))
    {
      if (_trx.activationFlags().isAB240())
        _diag->printResultingFareClassTable171Info(ptf, info, status, !ptf.carrier().empty());
      else
        _diag->printResultingFareClassTable171Info(ptf, info, status);
    }
    else
    {
      _diag->printResultingFareClassTable171Info(ptf, info, status);
    }
  }
}

bool
OptionalServicesValidator::checkInterlineIndicator(const OptionalServicesInfo& optSrvInfo) const
{
  return true;
}

}
