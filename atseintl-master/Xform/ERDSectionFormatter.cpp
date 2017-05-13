//-------------------------------------------------------------------------------
// Copyright 2012, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#include "Xform/ERDSectionFormatter.h"

#include "Common/MCPCarrierUtil.h"
#include "Common/TSEException.h"
#include "Common/XMLConstruct.h"
#include "DataModel/Agent.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/DiscountInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/NegFareRest.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "Rules/RuleConst.h"
#include "Xform/PricingResponseFormatter.h"
#include "Xform/PricingResponseXMLTags.h"

namespace tse
{

ERDSectionFormatter::ERDSectionFormatter(PricingTrx& pricingTrx,
                                         CalcTotals& calcTotals,
                                         const FareUsage& fareUsage,
                                         const CalcTotals::FareCompInfo& fareCompInfo,
                                         XMLConstruct& construct)
  : _pricingTrx(pricingTrx),
    _calcTotals(calcTotals),
    _fareUsage(fareUsage),
    _fareCompInfo(fareCompInfo),
    _construct(construct)
{
}

void
ERDSectionFormatter::prepareERD()
{
  const PaxTypeFare* paxTypeFare = _fareUsage.paxTypeFare();
  const PaxTypeFare* baseFare = paxTypeFare;
  const PaxTypeFare* netFare = nullptr;

  try
  {
    baseFare = paxTypeFare->fareWithoutBase();
    if (_pricingTrx.getOptions() && _pricingTrx.getOptions()->isCat35Net())
    {
      netFare = paxTypeFare->baseFare(35);
    }
  }
  catch (TSEException& exc) {}

  _construct.openElement(xml2::ERDInformation);

  _construct.addAttributeInteger(xml2::Link, baseFare->linkNumber()); // "Q46",
  _construct.addAttributeInteger(xml2::Sequence, baseFare->sequenceNumber()); // "Q1K",

  // Base Fare Create date
  if (baseFare->createDate().isValid())
  {
    _construct.addAttribute(xml2::CreateDate,
                            baseFare->createDate().dateToString(YYYYMMDD, "-").c_str()); // "D12",

    // Create time
    std::string createTime = baseFare->createDate().timeToSimpleString();
    createTime[2] = '-';
    createTime[5] = '-';
    _construct.addAttribute(xml2::CreateTime, createTime.c_str()); // "D55",
  }

  _construct.addAttribute(xml2::FCAFareType, paxTypeFare->fcaFareType()); // "S53",
  _construct.addAttributeNoNull(xml2::ERDFareClass, baseFare->fareClass().c_str()); // BJ0
  _construct.addAttribute(xml2::ERDBookingCode, paxTypeFare->bookingCode()); // P72

  if (_pricingTrx.getRequest() && !_pricingTrx.getRequest()->tktDesignator().empty()) // BE0
  {
    _construct.addAttribute(xml2::ERDTicketDesignator,
                            _pricingTrx.getRequest()->tktDesignator().begin()->second.c_str());
  }

  AccountCode corpId = "";

  if (paxTypeFare->isFareByRule())
  {
    // for Cat 25, check record 8
    corpId = paxTypeFare->fbrApp().accountCode();
  }
  else
  {
    if (paxTypeFare->matchedCorpID())
    {
      corpId = paxTypeFare->matchedAccCode();
    }
  }
  if (!corpId.empty())
  {
    _construct.addAttribute(xml2::ERDAccountCode, corpId);
  }

  TravelSeg* firstSeg = _fareUsage.travelSeg().front();
  DateTime time = _pricingTrx.getRequest()->ticketingDT();

  if (firstSeg && firstSeg->departureDT().isValid() && !firstSeg->isOpenWithoutDate())
  {
    time = firstSeg->departureDT();
  }
  else if (_pricingTrx.getOptions() && _pricingTrx.getOptions()->isTicketingDateOverrideEntry())
  {
    time = DateTime::localTime();
    short utcOffset = 0;
    const Loc* hdqLoc = _pricingTrx.dataHandle().getLoc(RuleConst::HDQ_CITY, time);
    const Loc* pccLoc = _pricingTrx.getRequest()->ticketingAgent()->agentLocation();

    if (LocUtil::getUtcOffsetDifference(
            *pccLoc, *hdqLoc, utcOffset, _pricingTrx.dataHandle(), time, time))
    {
      time = time.addSeconds(utcOffset * 60);
    }
  }

  _construct.addAttribute(xml2::ERDFareDepartDate, time.dateToString(YYYYMMDD, "-").c_str()); // D08

  if (netFare)
  {
    std::string fareBasis = netFare->createFareBasis(_pricingTrx, false);

    if (firstSeg && !firstSeg->specifiedFbc().empty())
    {
      _construct.addAttribute(xml2::ERDC35FareDisplayType,
                              NET_QUALIFIER_WITH_UNIQUE_FBC.c_str()); // N1P
      _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                     firstSeg->specifiedFbc().length()); // Q04
    }
    else
    {
      if (netFare->fcasTktCode().length() && netFare->fcasTktCode()[0] != '-')
      {
        _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                       netFare->fcasTktCode().length()); // Q04
      }
      else
      {
        _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                       netFare->fareClass().length()); // Q04
      }

      _construct.addAttribute(xml2::ERDC35FareDisplayType, NET_QUALIFIER.c_str()); // N1P
    }

    _construct.addAttribute(xml2::ERDFareBasisCode, fareBasis.c_str()); // B50
    if (netFare->fareDisplayCat35Type() != NET_QUALIFIER[0])
    {
      FareBreakPointInfo& breakPoint = _calcTotals.getFareBreakPointInfo(&_fareUsage);
      _construct.addAttributeDouble(xml2::ERDFareAmount, breakPoint.fareAmount, 9); // C5A
    }
    else
    {
      _construct.addAttributeDouble(xml2::ERDFareAmount, netFare->fareAmount(), 9); // C5A
    }
  }
  else
  {
    if (firstSeg && !firstSeg->specifiedFbc().empty())
    {
      _construct.addAttribute(xml2::ERDFareBasisCode,
                              paxTypeFare->createFareBasis(_pricingTrx, false)); // B50
      _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                     firstSeg->specifiedFbc().length()); // Q04
    }
    else
    {
      if (!_pricingTrx.getRequest()->specifiedTktDesignator().empty() ||
          paxTypeFare->carrier() == INDUSTRY_CARRIER)
      {
        _construct.addAttribute(xml2::ERDFareBasisCode,
                                paxTypeFare->createFareBasis(_pricingTrx, false)); // B50
      }

      if (paxTypeFare->fcasTktCode().length() && paxTypeFare->fcasTktCode()[0] != '-')
      {
        _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                       paxTypeFare->fcasTktCode().length()); // Q04
      }
      else
      {
        _construct.addAttributeInteger(xml2::ERDFareClassLength,
                                       paxTypeFare->fareClass().length()); // Q04
      }
    }

    if (paxTypeFare->fareDisplayCat35Type() != ' ') // "N1P"
    {
      _construct.addAttributeChar(xml2::ERDC35FareDisplayType, paxTypeFare->fareDisplayCat35Type());
    }

    _construct.addAttributeDouble(xml2::ERDFareAmount, _fareCompInfo.fareAmount, 9); // C5A
  }

  if (paxTypeFare->carrier() == INDUSTRY_CARRIER)
  {
    _construct.addAttribute(
        xml2::ERDGoverningCarrier,
        MCPCarrierUtil::swapToPseudo(&_pricingTrx, paxTypeFare->fareMarket()->governingCarrier()));
  }

  _construct.addAttribute(xml2::PassengerType, paxTypeFare->actualPaxType()->paxType()); // B70
  if (paxTypeFare->isCmdPricing())
    _construct.addAttributeChar(xml2::ERDCommandPricing, TRUE_INDICATOR); // CP0
  _construct.closeElement(); // CLOSE ERD.

  prepareERDCat25();
  prepareERDCat35(netFare);
  prepareERDDiscount();

  // Constructed fare information
  if (!paxTypeFare->fare()->isConstructedFareInfoMissing())
  {
    prepareERDConstructedFareInfo();
    prepareERDOriginAddOn();
    prepareERDDestAddOn();
  }
}

void
ERDSectionFormatter::prepareERDCat25()
{
  if (!_fareUsage.paxTypeFare()->isFareByRule())
    return;

  const FareByRuleItemInfo& itemInfo = _fareUsage.paxTypeFare()->fareByRuleInfo();

  // Add CAT25 fare information
  _construct.openElement(xml2::ERDC25Information); // C25
  _construct.addAttributeNoNull(xml2::ERDC25Vendor, itemInfo.vendor()); // "S37",
  _construct.addAttributeInteger(xml2::ERDC25ItemNo, itemInfo.itemNo()); // "Q41",

  std::string dir = PricingResponseFormatter::getDirectionality(
      _fareUsage.paxTypeFare()->paxTypeFareRuleData(RuleConst::FARE_BY_RULE));
  if (!dir.empty())
  {
    _construct.addAttribute(xml2::ERDC25Directionality, dir);
  }

  _construct.closeElement();
}

void
ERDSectionFormatter::prepareERDCat35(const PaxTypeFare* netFare)
{
  const PaxTypeFare& paxTypeFare = *_fareUsage.paxTypeFare();

  if ((paxTypeFare.isNegotiated() && _pricingTrx.getOptions() &&
       !_pricingTrx.getOptions()->isCat35Net()) ||
      (netFare && netFare->fareDisplayCat35Type() != NET_QUALIFIER[0]))
  {
    const NegFareRest& itemInfo = _fareUsage.paxTypeFare()->negotiatedInfo();

    _construct.openElement(xml2::ERDC35Information); // C35
    _construct.addAttributeNoNull(xml2::ERDC35Vendor, itemInfo.vendor()); // "S37",
    _construct.addAttributeInteger(xml2::ERDC35ItemNo, itemInfo.itemNo()); // "Q41",

    std::string dir = PricingResponseFormatter::getDirectionality(
        _fareUsage.paxTypeFare()->paxTypeFareRuleData(RuleConst::NEGOTIATED_RULE));
    if (!dir.empty())
    {
      _construct.addAttribute(xml2::ERDC35Directionality, dir);
    }

    _construct.closeElement();
  }
}

void
ERDSectionFormatter::prepareERDDiscount()
{
  if (!_fareUsage.paxTypeFare()->isDiscounted())
    return;

  const DiscountInfo& itemInfo = _fareUsage.paxTypeFare()->discountInfo();

  _construct.openElement(xml2::ERDDiscountInformation); // DFI
  _construct.addAttributeNoNull(xml2::ERDDiscountVendor, itemInfo.vendor()); // "S37",
  _construct.addAttributeInteger(xml2::ERDDiscountItemNo, itemInfo.itemNo()); // "Q41",

  std::string dir = PricingResponseFormatter::getDirectionality(
      _fareUsage.paxTypeFare()->paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE));
  if (!dir.empty())
  {
    _construct.addAttribute(xml2::ERDDiscountDirectionality, dir);
  }

  _construct.closeElement();
}

void
ERDSectionFormatter::prepareERDConstructedFareInfo()
{
  const PaxTypeFare& paxTypeFare = *_fareUsage.paxTypeFare();

  _construct.openElement(xml2::ERDContructedInformation); // "ECN"

  _construct.addAttributeNoNull(xml2::ERDGateway1, paxTypeFare.gateway1()); // "AM0"
  _construct.addAttributeNoNull(xml2::ERDGateway2, paxTypeFare.gateway2()); // "AN0"
  _construct.addAttributeInteger(xml2::ERDConstructionType,
                                 paxTypeFare.constructionType()); // "N1J"

  char tmpBuf[30];
  sprintf(tmpBuf, "%-.*f", 2, paxTypeFare.specifiedFareAmount());
  _construct.addAttribute(xml2::ERDSpecifiedFareAmount, tmpBuf); // "C66"
  std::ostringstream str;
  str.precision(9);
  str << paxTypeFare.constructedNucAmount();
  _construct.addAttribute(xml2::ERDConstructedNucAmount, str.str()); // "C6K"

  _construct.closeElement();
}

void
ERDSectionFormatter::prepareERDOriginAddOn()
{
  const PaxTypeFare& paxTypeFare = *_fareUsage.paxTypeFare();

  _construct.openElement(xml2::OAOInformation); // "OAO"

  _construct.addAttributeNoNull(xml2::OAOFootnote1, paxTypeFare.origAddonFootNote1()); // "S55"
  _construct.addAttributeNoNull(xml2::OAOFootnote2, paxTypeFare.origAddonFootNote2()); // "S64"
  _construct.addAttributeNoNull(xml2::OAOFareClass,
                                paxTypeFare.origAddonFareClass().c_str()); // "BJ0",
  _construct.addAttributeInteger(xml2::OAOTariff, paxTypeFare.origAddonTariff()); // "Q3W",
  _construct.addAttributeNoNull(xml2::OAORouting, paxTypeFare.origAddonRouting()); // "S65",
  // FareDisplay and RuleDisplay always use 2 decimal places for AddonAmount.
  _construct.addAttributeDouble(xml2::OAOAmount, paxTypeFare.origAddonAmount(), 2); // "C50"
  _construct.addAttributeNoNull(xml2::OAOCurrency, paxTypeFare.origAddonCurrency()); // "C40"
  _construct.addAttributeChar(xml2::OAOOWRT, paxTypeFare.origAddonOWRT()); // "P04"

  _construct.closeElement();
}

void
ERDSectionFormatter::prepareERDDestAddOn()
{
  const PaxTypeFare& paxTypeFare = *_fareUsage.paxTypeFare();

  _construct.openElement(xml2::DAOInformation); // "DAO"

  _construct.addAttributeNoNull(xml2::DAOFootnote1, paxTypeFare.destAddonFootNote1()); // "S55"
  _construct.addAttributeNoNull(xml2::DAOFootnote2, paxTypeFare.destAddonFootNote2()); // "S64"
  _construct.addAttributeNoNull(xml2::DAOFareClass,
                                paxTypeFare.destAddonFareClass().c_str()); // "BJ0",
  _construct.addAttributeInteger(xml2::DAOTariff, paxTypeFare.destAddonTariff()); // "Q3W",
  _construct.addAttributeNoNull(xml2::DAORouting, paxTypeFare.destAddonRouting()); // "S65",
  // FareDisplay and RuleDisplay always use 2 decimal places for AddonAmount.
  _construct.addAttributeDouble(xml2::DAOAmount, paxTypeFare.destAddonAmount(), 2); // "C50"
  _construct.addAttributeNoNull(xml2::DAOCurrency, paxTypeFare.destAddonCurrency()); // "C40"
  _construct.addAttributeChar(xml2::DAOOWRT, paxTypeFare.destAddonOWRT()); // "P04"

  _construct.closeElement();
}
}
