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
#include "FreeBagService/AncillaryResultProcessor.h"

#include "Common/FallbackUtil.h"
#include "Common/FreeBaggageUtil.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/BaggageCharge.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "DBAccess/TaxText.h"
#include "FreeBagService/AncillaryResultProcessorUtils.h"
#include "ServiceFees/ServiceFeesGroup.h"
#include "Xform/DataModelMap.h"

#include <boost/bind.hpp>
#include <boost/regex.hpp>

namespace tse
{
FALLBACK_DECL(fallbackAB240);

log4cxx::LoggerPtr
AncillaryResultProcessor::_logger(
    log4cxx::Logger::getLogger("atseintl.FreeBagService.AncillaryResultProcessor"));

AncillaryResultProcessor::AncillaryResultProcessor(AncillaryPricingTrx& trx) : _trx(trx) {}

AncillaryResultProcessor::~AncillaryResultProcessor() {}

std::string
ServiceFeesGroupsToString(const std::vector<ServiceFeesGroup*>& ocFeesGroup)
{
  std::ostringstream dbgMsg;
  dbgMsg << "[" << ocFeesGroup.size() << "]{";
  for (auto sfg : ocFeesGroup)
    dbgMsg << (sfg ? sfg->groupCode() : "NULLPTR") << ",";

  dbgMsg << "}";
  return dbgMsg.str();
}

void
AncillaryResultProcessor::processAllowanceAndCharges(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                                          const BaggageTravel* baggageTravel,
                                          uint32_t baggageTravelIndex)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - entering: ocFeesGroup=" << ServiceFeesGroupsToString(ocFeesGroup));

  if (serviceTypeNeeded(getS5(baggageTravel), 'A'))
  {
    LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " Processing service type A");
    buildOcFees(ocFeesGroup, baggageTravel->_allowance, baggageTravel, baggageTravelIndex);
  }
  for (BaggageCharge* baggageCharge : baggageTravel->_chargeVector)
  {
    if (serviceTypeNeeded(baggageCharge->subCodeInfo(), 'C'))
    {
      LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " Processing service type {" << baggageCharge->subCodeInfo()->serviceGroup() << "," << baggageCharge->subCodeInfo()->serviceSubGroup() << ",C}");
      buildOcFees(ocFeesGroup, baggageCharge, baggageTravel, baggageTravelIndex);
    }
  }
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processAllowanceAndCharges(std::vector<ServiceFeesGroup*>&, const BaggageTravel*, uint32_t) - leaving");
}

void
AncillaryResultProcessor::processEmbargo(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                                          const BaggageTravel* baggageTravel,
                                          uint32_t baggageTravelIndex)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processEmbargo() - entering");
  for(OCFees* fees : baggageTravel->_embargoVector)
  {
    buildOcFees(ocFeesGroup, fees,  baggageTravel, baggageTravelIndex);
  }
}

void
AncillaryResultProcessor::processCarryOn(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                                          const BaggageTravel* baggageTravel,
                                          uint32_t baggageTravelIndex)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processCarryOn() - entering");
  buildOcFees(ocFeesGroup, baggageTravel->_allowance, baggageTravel, baggageTravelIndex);
  for(OCFees* fees : baggageTravel->_chargeVector)
  {
    buildOcFees(ocFeesGroup, fees,  baggageTravel, baggageTravelIndex);
  }
}

void
AncillaryResultProcessor::processAllowanceAndCharges(const std::vector<BaggageTravel*>& baggageTravels)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processAllowanceAndCharges(const std::vector<BaggageTravel*>&) - entering");
  process(baggageTravels.begin(), baggageTravels.end(),
          boost::bind(&AncillaryResultProcessor::processAllowanceAndCharges, this, _1, _2, _3));
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processAllowanceAndCharges(const std::vector<BaggageTravel*>&) - entering");
}

bool
AncillaryResultProcessor::isPartOfBaggageRoute(const BaggageTravel* baggageTravel) const
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::isPartOfBaggageRoute() - entering");
  std::vector<tse::TravelSeg*>::const_iterator result = find_if(baggageTravel->getTravelSegBegin(),
                                                                baggageTravel->getTravelSegEnd(),
                                                                CheckPortionOfTravelIndicator('T'));

  return result != baggageTravel->getTravelSegEnd();
}

bool
AncillaryResultProcessor::serviceTypeNeeded(const SubCodeInfo* s5,
                                            const Indicator& ancillaryServiceType) const
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::serviceTypeNeeded("
                << "ancillaryServiceType=\"" << ancillaryServiceType << "\""
                << ") - entering");
  if (!s5)
  {
    LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::serviceTypeNeeded(): s5==0 - returning false");
    return false;
  }

  const std::vector<RequestedOcFeeGroup>& requestedGroups = _trx.getOptions()->serviceGroupsVec();

  if (requestedGroups.empty())
  {
    LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::serviceTypeNeeded() - requestedGroups is empty - returning true");
    return true;
  }

  bool ret = std::find_if(requestedGroups.begin(),
                          requestedGroups.end(),
                          CheckServiceTypeNeeded(s5->serviceGroup(), ancillaryServiceType)) !=
                              requestedGroups.end();
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::serviceTypeNeeded() - returning " << (ret ? "true" : "false"));
  return ret;
}

void
AncillaryResultProcessor::processEmbargoes(const std::vector<const BaggageTravel*>& baggageTravels)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processEmbargoes() - entering");
  process(baggageTravels.begin(), baggageTravels.end(),
          boost::bind(&AncillaryResultProcessor::processEmbargo, this, _1, _2, _3));
}

void
AncillaryResultProcessor::processCarryOn(const std::vector<const BaggageTravel*>& baggageTravels)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::processCarryOn("
                << baggageTravels.size() << ") - entering");
  process(baggageTravels.begin(), baggageTravels.end(),
          boost::bind(&AncillaryResultProcessor::processCarryOn, this, _1, _2, _3));
}

void
AncillaryResultProcessor::buildOcFees(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                                      OCFees* ocFees,
                                      const BaggageTravel* baggageTravel,
                                      uint32_t baggageTravelIndex)
{
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - entering: ocFeesGroup=" << ServiceFeesGroupsToString(ocFeesGroup));

  if (!fallback::fallbackAB240(&_trx))
  {
    if(!ocFees)
    {
      LOG4CXX_ERROR(_logger, "AncillaryResultProcessor::buildOcFees() - ocFees is NULL");
      return;
    }
  }
  ServiceFeesGroup* group = getGroup(ocFeesGroup, ocFees->subCodeInfo());
  if (fallback::fallbackAB240(&_trx))
  {
    if (!group || !ocFees)
      return;
  }
  else
  {
    if (!group)
    {
      LOG4CXX_ERROR(_logger, "AncillaryResultProcessor::buildOcFees() - ServiceFeesGroup is NULL");
      return;
    }
  }

  OCFeesInitializer ocFeesInit(_trx, *baggageTravel, *ocFees, baggageTravelIndex);
  ocFeesInit.init(group);
}

void
AncillaryResultProcessor::OCFeesInitializer::init(ServiceFeesGroup* group) const
{
  TSE_ASSERT(group);
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - entering: group->groupCode=" << group->groupCode());

  std::vector<OCFees*>& ocFeesVector = group->ocFeesMap()[_baggageTravel.farePath()];
  std::vector<OCFees*>::iterator it = find_if(ocFeesVector.begin(), ocFeesVector.end(), *this);

  if (ocFeesVector.end() == it)
    addOCFees(group);
  else
    addOCFeesSeg(*it);

  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - exiting");
}

bool
AncillaryResultProcessor::OCFeesInitializer::
operator()(const OCFees* ocFees) const
{
  return (ocFees->subCodeInfo() == _ocFees.subCodeInfo()) &&
         (ocFees->travelStart() == _ocFees.travelStart()) &&
         (ocFees->travelEnd() == _ocFees.travelEnd());
}

void
AncillaryResultProcessor::OCFeesInitializer::addOCFeesSeg(OCFees* ocFees) const
{
  if (!_ocFees.optFee())
    return;

  std::vector<OCFees::OCFeesSeg*>::const_iterator it = find_if(
      ocFees->segments().begin(), ocFees->segments().end(), CheckOCFeesSeg(_ocFees.optFee()));
  if (ocFees->segments().end() == it)
  {
    OCFees::OCFeesSeg* ocFeesSeg = _ocFees.segments().front();
    add196(ocFeesSeg);
    ocFees->segments().push_back(ocFeesSeg);
  }
}

std::string
PaxTypesInOcFeesMapToString(const std::map<const FarePath*, std::vector<OCFees*> >& ocFeesMap)
{
  std::ostringstream dbgMsg;
  dbgMsg << "[" << ocFeesMap.size() << "]{";
  for (auto farePathAndOcFees: ocFeesMap)
  {
    TSE_ASSERT(farePathAndOcFees.first);
    TSE_ASSERT(farePathAndOcFees.first->paxType());
    dbgMsg << farePathAndOcFees.first->paxType()->paxType();
    dbgMsg << "=>[" << farePathAndOcFees.second.size() << "],";
  }
  dbgMsg << "}";
  return dbgMsg.str();
}

void
AncillaryResultProcessor::OCFeesInitializer::addOCFees(ServiceFeesGroup* group) const
{
  TSE_ASSERT(group);
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - entering: group->groupCode=" << group->groupCode());

  _ocFees.baggageTravelIndex() = _baggageTravelIndex;
  if (_ocFees.optFee())
  {
    add196(_ocFees.segments().front());
  }
  else
  {
    // there is no S7 record for this OCFees - remove OCFeesSegment
    LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " there is no S7 record for this OCFees - remove OCFeesSegment");
    _ocFees.segments().clear();
  }

  group->ocFeesMap()[_baggageTravel.farePath()].push_back(&_ocFees);

  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << ": group->ocFeesMap()=" << PaxTypesInOcFeesMapToString(group->ocFeesMap()));
  LOG4CXX_DEBUG(_logger, __LOG4CXX_FUNC__ << " - exiting");
}

ServiceFeesGroup*
AncillaryResultProcessor::getGroup(std::vector<ServiceFeesGroup*>& ocFeesGroup,
                                   const SubCodeInfo* subCode)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::getGroup() - entering");
  if (!subCode)
    return nullptr;

  ServiceFeesGroup* group = nullptr;
  std::vector<ServiceFeesGroup*>::iterator it =
      find_if(ocFeesGroup.begin(), ocFeesGroup.end(), CheckServiceGroup(subCode->serviceGroup()));

  if (ocFeesGroup.end() == it)
  {
    group = createGroup(subCode->serviceGroup());
    ocFeesGroup.push_back(group);
  }
  else
    group = (*it);

  return group;
}

ServiceFeesGroup*
AncillaryResultProcessor::createGroup(const ServiceGroup& groupCode)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::createGroup(\"" << groupCode << "\") - entering");
  ServiceFeesGroup* srvFeesGroup;
  _trx.dataHandle().get(srvFeesGroup);
  srvFeesGroup->groupCode() = groupCode;
  return srvFeesGroup;
}

void
AncillaryResultProcessor::OCFeesInitializer::add196(OCFees::OCFeesSeg* ocFeesSeg) const
{
  if (ocFeesSeg->_optFee->fltTktMerchInd() == BAGGAGE_ALLOWANCE
     || (_trx.activationFlags().isAB240() && ocFeesSeg->_optFee->fltTktMerchInd() == CARRY_ON_ALLOWANCE))
  {
    const TaxText* taxText = _trx.dataHandle().getTaxText(ocFeesSeg->_optFee->vendor(),
                                                          ocFeesSeg->_optFee->taxTblItemNo());
    if (taxText)
    {
      for (std::string txtMsg : taxText->txtMsgs())
        addBaggageItemProperty(ocFeesSeg, txtMsg);
    }
  }
}

void
AncillaryResultProcessor::OCFeesInitializer::addBaggageItemProperty(OCFees::OCFeesSeg* ocFeesSeg,
                                                                    const std::string& txtMsg) const
{
  boost::regex expression("^//(\\d{2})/(\\w{3})$");
  boost::cmatch what;
  if (boost::regex_match(txtMsg.c_str(), what, expression))
  {
    const SubCodeInfo* s5 = selectS5Record(ocFeesSeg, what[2].first);
    if (s5)
    {
      ocFeesSeg->_baggageItemProperties.push_back(
          OCFees::BaggageItemProperty(std::atoi(what[1].first), s5));
    }
  }
  else
  {
    ocFeesSeg->_baggageItemProperties.push_back(OCFees::BaggageItemProperty(txtMsg));
  }
}

const SubCodeInfo*
AncillaryResultProcessor::OCFeesInitializer::selectS5Record(
    OCFees::OCFeesSeg* ocFeesSeg, const ServiceSubTypeCode& serviceSubTypeCode) const
{
  const SubCodeInfo* s5 = nullptr;
  std::vector<SubCodeInfo*> allSubCodes;
  retrieveS5Records(ocFeesSeg->_optFee->vendor(), ocFeesSeg->_optFee->carrier(), allSubCodes);

  std::vector<SubCodeInfo*>::const_iterator it =
      find_if(allSubCodes.begin(), allSubCodes.end(), CheckS5(serviceSubTypeCode));

  if (it != allSubCodes.end())
    s5 = *it;

  return s5;
}

void
AncillaryResultProcessor::OCFeesInitializer::retrieveS5Records(
    const VendorCode& vendor, const CarrierCode& carrier, std::vector<SubCodeInfo*>& subCodeVector)
    const
{
  FreeBaggageUtil::getS5Records(vendor, carrier, subCodeVector, _trx);
}

void
AncillaryResultProcessor::addEmptyGroupIfNoDataReturned(Itin* itin)
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::addEmptyGroupIfNoDataReturned() - entering");
  std::vector<ServiceFeesGroup*>& ocFeesGroups =
    fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag() ?
        itin->ocFeesGroup() : itin->ocFeesGroupsFreeBag();

  std::vector<ServiceGroup> groupCodes;
  groupCodes.push_back(ServiceGroup("BG"));

  if(!(_trx.activationFlags().isAB240() &&
       _trx.getOptions()->isOcOrBaggageDataRequested(RequestedOcFeeGroup::DisclosureData)))
    groupCodes.push_back(ServiceGroup("PT"));

  std::vector<RequestedOcFeeGroup>& requestedGroups = _trx.getOptions()->serviceGroupsVec();

  for (const ServiceGroup& groupCode : groupCodes)
  {
    std::vector<ServiceFeesGroup*>::iterator it =
        find_if(ocFeesGroups.begin(), ocFeesGroups.end(), CheckServiceGroup(groupCode));

    bool needThisGroup = requestedGroups.empty() ||
                         find_if(requestedGroups.begin(),
                                 requestedGroups.end(),
                                 CheckRequestedServiceGroup(groupCode)) != requestedGroups.end();
    if (ocFeesGroups.end() == it && needThisGroup)
    {
      ServiceFeesGroup* group = createGroup(groupCode);
      group->state() = ServiceFeesGroup::EMPTY;
      ocFeesGroups.push_back(group);
    }
  }
}

const SubCodeInfo*
AncillaryResultProcessor::getS5(const BaggageTravel* baggageTravel) const
{
  LOG4CXX_DEBUG(_logger, "AncillaryResultProcessor::getS5() - entering");
  return baggageTravel->_allowance ? baggageTravel->_allowance->subCodeInfo() : nullptr;
}
} // tse
