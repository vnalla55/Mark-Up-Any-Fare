//-------------------------------------------------------------------
//  Copyright Sabre 2012
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
#include "FreeBagService/AncillaryChargesDataStrategy.h"

#include "Common/EmdInterlineAgreementInfoMapBuilder.h"
#include "Common/EmdValidator.h"
#include "Common/FreeBaggageUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/AncRequest.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/Diag852Collector.h"
#include "FreeBagService/BaggageOcValidationAdapter.h"
#include "FreeBagService/BaggageTravelInfo.h"
#include "ServiceFees/OCEmdDataProvider.h"
#include "ServiceFees/OCFees.h"

#include <algorithm>
#include <functional>

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace tse
{
namespace
{
struct CheckSubCodeInfo : std::unary_function<const SubCodeInfo*, bool>
{
  bool _isInterLine;
  bool _isCarrierOverridden;

  CheckSubCodeInfo(bool isInterLine, bool isCarrierOverridden)
    : _isInterLine(isInterLine), _isCarrierOverridden(isCarrierOverridden)
  {
  }

  bool operator()(const SubCodeInfo* subCodeInfo) const
  {
    bool result = !subCodeInfo->serviceSubGroup().equalToConst("CY") && subCodeInfo->concur() == 'X' &&
                          subCodeInfo->ssimCode() == ' ' &&
                          (subCodeInfo->rfiCode() == 'C' || subCodeInfo->rfiCode() == ' ') &&
                          (subCodeInfo->emdType() == '2' || subCodeInfo->emdType() == '3' || subCodeInfo->emdType() == '4') &&
                          (subCodeInfo->bookingInd() != "2" && subCodeInfo->bookingInd() != "5") &&
                          (subCodeInfo->fltTktMerchInd() == BAGGAGE_CHARGE);

    if (_isInterLine || _isCarrierOverridden)
    {
      result = result && subCodeInfo->industryCarrierInd() == 'I';
    }
    return result;
  }
};

struct S5SortComparator : std::binary_function<const SubCodeInfo*, const SubCodeInfo*, bool>
{
  bool operator()(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    if (!firstS5 || !secondS5)
      return false;

    if (firstS5->serviceGroup() < secondS5->serviceGroup())
      return true;
    else if (firstS5->serviceGroup() > secondS5->serviceGroup())
      return false;
    else
      return compareSubGroup(firstS5, secondS5);
  }

  bool compareSubGroup(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    if (!firstS5->serviceGroup().equalToConst("BG"))
      return false;

    if (firstS5->serviceSubGroup() < secondS5->serviceSubGroup())
      return true;
    else if (firstS5->serviceSubGroup() > secondS5->serviceSubGroup())
      return false;
    else
      return compareDescription1(firstS5, secondS5);
  }

  bool compareDescription1(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    if (firstS5->description1().empty() && !secondS5->description1().empty())
      return true;
    else if (!firstS5->description1().empty() && secondS5->description1().empty())
      return false;
    else
      return compareNumeric(firstS5, secondS5);
  }

  bool compareNumeric(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    const bool isFirstS5Numeric = isNumeric(firstS5);
    const bool isSecondS5Numeric = isNumeric(secondS5);

    if (isFirstS5Numeric && !isSecondS5Numeric)
      return true;
    else if (!isFirstS5Numeric && isSecondS5Numeric)
      return false;
    else if (isFirstS5Numeric && isSecondS5Numeric)
    {
      try
      {
        const short first = boost::lexical_cast<short>(firstS5->description1());
        const short second = boost::lexical_cast<short>(secondS5->description1());

        if (first == second)
          return firstS5->serviceSubTypeCode() < secondS5->serviceSubTypeCode();
        else
          return first < second;
      }
      catch (boost::bad_lexical_cast&) { return compareAlphaNumeric(firstS5, secondS5); }
    }
    else
      return compareAlphaNumeric(firstS5, secondS5);
  }

  bool compareAlphaNumeric(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    const bool isFirstAlphaNumeric = isAlphaNumeric(firstS5);
    const bool isSecondAlphaNumeric = isAlphaNumeric(secondS5);

    if (isFirstAlphaNumeric && !isSecondAlphaNumeric)
      return true;
    else if (!isFirstAlphaNumeric && isSecondAlphaNumeric)
      return false;
    else if (isFirstAlphaNumeric && isSecondAlphaNumeric)
      return compareAlpha(firstS5, secondS5);
    else
      return compareXplusValue(firstS5, secondS5);
  }

  bool compareXplusValue(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    if (!firstS5->serviceSubGroup().empty())
      return compareAlpha(firstS5, secondS5);

    const bool isFirstXplusValue = isXplusValue(firstS5);
    const bool isSecondXplusValue = isXplusValue(secondS5);

    if (isFirstXplusValue && !isSecondXplusValue)
      return true;
    else if (!isFirstXplusValue && isSecondXplusValue)
      return false;
    else
      return compareAlpha(firstS5, secondS5);
  }

  bool compareAlpha(const SubCodeInfo* firstS5, const SubCodeInfo* secondS5) const
  {
    if (firstS5->description1() == secondS5->description1())
      return firstS5->serviceSubTypeCode() < secondS5->serviceSubTypeCode();
    else
      return firstS5->description1() < secondS5->description1();
  }

  bool isNumeric(const SubCodeInfo* s5) const
  {
    try { boost::lexical_cast<int>(s5->description1()); }
    catch (boost::bad_lexical_cast& e) { return false; }
    return true;
  }

  bool isAlphaNumeric(const SubCodeInfo* s5) const
  {
    static const boost::regex expression("^(?=.*[0-9])(?=.*[a-zA-Z])+([a-zA-Z0-9]+)$");
    boost::cmatch what;
    if (boost::regex_match(std::string(s5->description1()).c_str(), what, expression))
    {
      return s5->serviceSubGroup().empty() ? !isXplusValue(s5) : true;
    }
    return false;
  }

  bool isXplusValue(const SubCodeInfo* s5) const
  {
    static const boost::regex expression("^[X|x](\\d)+$");
    boost::cmatch what;
    if (boost::regex_match(std::string(s5->description1()).c_str(), what, expression))
    {
      return true;
    }
    return false;
  }
};
}

// when emdInfoMap is not set, Emd validation will not be performed
AncillaryChargesDataStrategy::AncillaryChargesDataStrategy(PricingTrx& trx, boost::optional<const EmdInterlineAgreementInfoMap&> emdInfoMap)
  : ChargesDataStrategy(trx), _carrierEmdInfoMap(emdInfoMap)
{
}

AncillaryChargesDataStrategy::~AncillaryChargesDataStrategy() {}

void
AncillaryChargesDataStrategy::processBaggageTravel(BaggageTravel* baggageTravel,
                                                   const BaggageTravelInfo& bagInfo,
                                                   const CheckedPoint& furthestCheckedPoint,
                                                   BaggageTripType btt,
                                                   Diag852Collector* dc) const
{
  if (!baggageTravel->_processCharges)
    return;

  std::vector<const SubCodeInfo*> subCodes;
  retrieveS5Records(baggageTravel, subCodes);
  retrieveCharges(baggageTravel, bagInfo, subCodes, furthestCheckedPoint, btt.isUsDot(), dc);

  if (_trx.activationFlags().isEmdForCharges() && _carrierEmdInfoMap)
  {
    AncRequestPath arp = _trx.getOptions()->getAncRequestPath();
    bool emdCalculationResult = true;

    if (shouldvalidateEmd(arp, baggageTravel))
      emdCalculationResult = validateEmd(baggageTravel, bagInfo, btt.isUsDot(), dc);

    propagateEmdValidationResult(emdCalculationResult, arp, baggageTravel, bagInfo, dc);
  }
}

bool
AncillaryChargesDataStrategy::validateEmd(BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo, bool isUsDot, Diag852Collector* dc) const
{
  OCEmdDataProvider ocEmdDataProvider;
  collectEmdData(baggageTravel, bagInfo, ocEmdDataProvider, isUsDot, dc);

  EmdValidator emdValidator(_trx, ocEmdDataProvider, nullptr);
  return emdValidator.validate((*_carrierEmdInfoMap));
}

void
AncillaryChargesDataStrategy::collectEmdData(const BaggageTravel* baggageTravel,
                                             const BaggageTravelInfo& bagInfo,
                                             OCEmdDataProvider& dataForEmdValidation, bool isUsDot,
                                             Diag852Collector* dc) const
{
  const AirSeg* mss = isUsDot ? (*baggageTravel->_MSSJourney)->toAirSeg() : (*baggageTravel->_MSS)->toAirSeg();
  dataForEmdValidation.emdValidatingCarrier() = getEmdValidatingCarrier();

  dataForEmdValidation.setEmdMostSignificantCarrier(mss->marketingCarrierCode());
  for (TravelSegPtrVecCI it = baggageTravel->getTravelSegBegin(); it != baggageTravel->getTravelSegEnd(); ++it)
  {
    const AirSeg* airSeg = static_cast<AirSeg*>(*it);
    dataForEmdValidation.operatingCarriers().insert(airSeg->operatingCarrierCode());
    dataForEmdValidation.marketingCarriers().insert(airSeg->marketingCarrierCode());
  }

  displayEmdDiagnostic(bagInfo, baggageTravel, dataForEmdValidation, dc);
}

void
AncillaryChargesDataStrategy::displayEmdDiagnostic(const BaggageTravelInfo& bagInfo, const BaggageTravel* baggageTravel, const OCEmdDataProvider& dataForEmdValidation, Diag852Collector* dc) const
{
  if (dc && shouldDisplayEmdDaignostic(dc) && checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc))
  {
    NationCode nation;
    const Loc* pointOfSaleLocation = TrxUtil::saleLoc(_trx);
    if(pointOfSaleLocation)
      nation = pointOfSaleLocation->nation();

    CrsCode gds;
    if (_trx.getRequest() && _trx.getRequest()->ticketingAgent())
      gds = _trx.getRequest()->ticketingAgent()->cxrCode();


    dc->printDetailInterlineEmdProcessingS5Info(nation, gds,
                                                dataForEmdValidation.emdValidatingCarrier(),
                                                dataForEmdValidation.marketingCarriers(),
                                                dataForEmdValidation.operatingCarriers());

    auto it = _carrierEmdInfoMap->find(dataForEmdValidation.emdValidatingCarrier());

    if (it == (*_carrierEmdInfoMap).end() || it->second.empty())
      dc->printNoInterlineDataFoundInfo();
    else
      dc->printDetailInterlineEmdAgreementInfo(it->second, dataForEmdValidation.emdValidatingCarrier());
  }
}

bool
AncillaryChargesDataStrategy::shouldDisplayEmdDaignostic(const Diag852Collector* dc) const
{
  return dc->hasDisplayChargesOption();
}

void
AncillaryChargesDataStrategy::propagateEmdValidationResult(bool emdValidationResult, AncRequestPath rp, BaggageTravel* baggageTravel, const BaggageTravelInfo& bagInfo, Diag852Collector* dc) const
{
  ChargeVector& charges = baggageTravel->_chargeVector;
  if(dc && shouldDisplayEmdDaignostic(dc) && checkFareLineAndCheckedPortion(baggageTravel, bagInfo, dc))
    dc->printEmdValidationResult(emdValidationResult, rp, baggageTravel, bagInfo);

  if (rp == AncRequestPath::AncCheckInPath)
  {
    for (BaggageCharge* baggageCharge: charges)
    {
      if (isEmdRecord(baggageCharge))
        emdValidationResult ? baggageCharge->setEmdSoftPassChargeIndicator(EmdSoftPassIndicator::EmdPassOrNoEmdValidation) :
                              baggageCharge->setEmdSoftPassChargeIndicator(EmdSoftPassIndicator::EmdSoftPass);
    }
  }
  else if (rp == AncRequestPath::AncReservationPath && !emdValidationResult)
  {
    charges.erase(std::remove_if(charges.begin(), charges.end(),
                                 [this, baggageTravel](const BaggageCharge* bc)
                                 {
                                   return isEmdRecord(bc) &&
                                          !baggageTravel->isBaggageCharge1stOr2ndBag(bc);
                                 }), charges.end());
  }
}

std::string
AncillaryChargesDataStrategy::getEmdValidatingCarrier() const
{
  std::string emdValidatingCarrier;
  if (_trx.billing() && !_trx.billing()->partitionID().empty())
    emdValidatingCarrier = _trx.billing()->partitionID();

  return emdValidatingCarrier;
}

bool
AncillaryChargesDataStrategy::shouldvalidateEmd(AncRequestPath rp, const BaggageTravel* baggageTravel) const
{
  const ChargeVector& charges = baggageTravel->_chargeVector;
  if (AncRequestPath::AncCheckInPath == rp)
  {
    return  std::any_of(charges.begin(), charges.end(),
                        [this](const BaggageCharge* bc)
                        { return isEmdRecord(bc); } );
  }
  else
  {
    return  std::any_of(charges.begin(), charges.end(),
                        [this, baggageTravel](const BaggageCharge* bc)
                        {
                          return isEmdRecord(bc) &&
                                 !baggageTravel->isBaggageCharge1stOr2ndBag(bc);
                        });
  }
}

bool
AncillaryChargesDataStrategy::isEmdRecord(const BaggageCharge* baggageCharge) const
{
  return baggageCharge->subCodeInfo() && (baggageCharge->subCodeInfo()->emdType() == '2' ||
                                          baggageCharge->subCodeInfo()->emdType() == '3');
}

void
AncillaryChargesDataStrategy::sort(std::vector<const SubCodeInfo*>& subCodes) const
{
  if (FreeBaggageUtil::isSortNeeded(static_cast<AncillaryPricingTrx*>(&_trx)))
    std::stable_sort(subCodes.begin(), subCodes.end(), S5SortComparator());
}

void
AncillaryChargesDataStrategy::retrieveS5Records(const BaggageTravel* baggageTravel,
                                                std::vector<const SubCodeInfo*>& subCodes) const
{
  ChargesDataStrategy::retrieveS5Records(
      getS5CarrierCode(baggageTravel),
      subCodes,
      CheckSubCodeInfo(!allSegmentsOnTheSameCarrier(*baggageTravel->itin()),
                       isChargesCarrierOverridden()));

  sort(subCodes);
}

void
AncillaryChargesDataStrategy::matchS7s(BaggageTravel& baggageTravel,
                                       const SubCodeInfo* s5,
                                       const CheckedPoint& furthestCheckedPoint,
                                       bool isUsDot,
                                       Diag852Collector* dc,
                                       ChargeVector& charges) const
{
  BaggageOcValidationAdapter::matchS7AncillaryChargesRecords(
      *s5, baggageTravel, furthestCheckedPoint, dc, charges);
}

void
AncillaryChargesDataStrategy::findLowestCharges(
    const std::vector<BaggageTravel*>& /*baggageTravels*/,
    uint32_t /*bgIndex*/,
    ChargeVector& /*charges*/,
    Diag852Collector* /*dc*/) const
{
  return;
}

CarrierCode
AncillaryChargesDataStrategy::getS5CarrierCode(const BaggageTravel* baggageTravel) const
{
  CarrierCode carrier;
  if (isChargesCarrierOverridden())
    carrier = getChargesCarrierOverridden();
  else if (!baggageTravel->_allowanceCxr.empty())
    carrier = baggageTravel->_allowanceCxr;
  else if (getAllowanceS7(baggageTravel))
    carrier = getAllowanceS7(baggageTravel)->carrier();
  else
    carrier = static_cast<const AirSeg*>(*baggageTravel->_MSSJourney)->marketingCarrierCode();
  return carrier;
}
} // tse
