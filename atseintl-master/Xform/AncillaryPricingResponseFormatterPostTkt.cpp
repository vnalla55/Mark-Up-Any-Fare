//----------------------------------------------------------------------------
//
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
//----------------------------------------------------------------------------

#include "Xform/AncillaryPricingResponseFormatterPostTkt.h"

#include "Common/Config/ConfigManUtils.h"
#include "Common/Config/ConfigurableValue.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/ServiceFeeUtil.h"
#include "DataModel/AncRequest.h"
#include "DataModel/Billing.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SubCodeInfo.h"
#include "Xform/DataModelMap.h"
#include "Xform/PricingResponseXMLTags.h"

#include <boost/lexical_cast.hpp>
#include <boost/regex.hpp>

namespace
{
// config params
static const std::string SERVICE_FEE = "SERVICE_FEES_SVC";
static const std::string MAX_NUMBER_OF_FEES_KEY = "MAX_NUMBER_OF_FEES";
static const std::string MAX_NUMBER_OF_FEES_AB240_KEY = "MAX_NUMBER_OF_FEES_AB240";
}

namespace tse
{
namespace
{
ConfigurableValue<uint16_t>
maxNumberOfOCFeesAb240Key(SERVICE_FEE, MAX_NUMBER_OF_FEES_AB240_KEY);
ConfigurableValue<uint16_t>
maxNumberOfOCFeesKey(SERVICE_FEE, MAX_NUMBER_OF_FEES_KEY);
}
FIXEDFALLBACK_DECL(AB240_DecoupleServiceFeesAndFreeBag);
FALLBACK_DECL(fallbackAB240);

static Logger
logger("atseintl.Xform.AncillaryPricingResponseFormatterPostTkt");

void
AncillaryPricingResponseFormatterPostTkt::formatAeFeesResponseForGroup(ServiceFeesGroup* sfg,
                                                                       AncillaryPricingTrx& ancTrx,
                                                                       AncRequest* req,
                                                                       uint16_t& groupIndex)
{
  // empty or NA group will be added to first itin only
  bool processOnlyOnce =
      sfg->state() == ServiceFeesGroup::EMPTY || sfg->state() == ServiceFeesGroup::NOT_AVAILABLE;
  bool shouldProcess = true;
  if (_doNotProcess)
    _doNotProcess = false;
  Itin* ocItin = nullptr;

  std::pair<const ServiceFeesGroup*, std::vector<PaxR7OCFeesUsages>>& pair =
      _groupFeesVectorU[groupIndex];

  std::vector<PaxR7OCFeesUsages>::iterator ipt = pair.second.begin();
  std::vector<PaxR7OCFeesUsages>::iterator ipe = pair.second.end();
  if (ipt == ipe)
  {
    // if(_isAnyAll || paxTypes.empty())
    currentItin() = ocItin = req->masterItin();
    _realPaxType = nullptr;
    if (!_isAnyAll && !_paxTypes.empty())
    {
      for (std::vector<Itin*>::iterator iit = ancTrx.itin().begin();
           iit != ancTrx.itin().end() && _realPaxType == nullptr;
           iit++)
      {
        if (_paxTypes.find(*iit) != _paxTypes.end() && !_paxTypes[*iit].empty())
        {
          _realPaxType = *_paxTypes[*iit].begin();
          currentItin() = *iit;
        }
      }
    }
  }
  else
  {
    currentItin() = ocItin = const_cast<Itin*>(ipt->itin());
    _realPaxType = ipt->realPaxType();
  }

  std::map<uint16_t, std::vector<XMLConstruct*>> ocgParts;
  while (shouldProcess) // && !_doNotProcess)
  {
    std::vector<PaxOCFeesUsages> fees;
    while (ipt != ipe)
    {
      // all will be in ITN = 1;
      if (ipt->paxType() == "ALL")
        ocItin = req->masterItin();
      // find correct Itin
      else
        ocItin = const_cast<Itin*>(ipt->itin());

      // Itin changed
      if (currentItin() != ocItin || _realPaxType != ipt->realPaxType())
      {
        break;
      }
      fees.push_back(*ipt);
      ++ipt;
    }
    std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>>
    filteredGroupFeesVector;
    filteredGroupFeesVector.push_back(std::make_pair(pair.first, fees));

    Itin* i = _realPaxType ? req->paxToOriginalItinMap()[_realPaxType] : currentItin();
    // initialize Itin Post tkt data, this will "unmerge" any Itins
    if (_itinXmls.find(i->getItinOrderNum()) == _itinXmls.end())
    {
      _itinXmls.insert(std::make_pair(i->getItinOrderNum(),
                                      ancTrx.dataHandle().create<PostTktResonseItinGrp>()));
      _itinXmls[i->getItinOrderNum()]->_itin = currentItin();
      _itinXmls[i->getItinOrderNum()]->_realItin = i;
    }
    _curXmlConstructData = _itinXmls[i->getItinOrderNum()];
    ocgParts[i->getItinOrderNum()].push_back(ancTrx.dataHandle().create<XMLConstruct>());
    // build ITIN fees section and preformated messages
    LOG4CXX_DEBUG(logger,
                  "currentItin()=" << (size_t)currentItin()
                                   << ", masterItin()=" << (size_t)req->masterItin());
    formatOCFeesGroupsForR7(ancTrx,
                            *ocgParts[i->getItinOrderNum()].back(),
                            filteredGroupFeesVector,
                            _allFeesDisplayOnly[currentItin()],
                            false);
    if (ipt == ipe)
      shouldProcess = false;
    else
    {
      shouldProcess = !processOnlyOnce;
      currentItin() = ocItin;
      _realPaxType = ipt->realPaxType();
    }
  }
  // create common OCG
  std::map<uint16_t, std::vector<XMLConstruct*>>::iterator imt = ocgParts.begin();
  std::map<uint16_t, std::vector<XMLConstruct*>>::iterator ime = ocgParts.end();
  for (; imt != ime; ++imt)
    createOcgSection(_itinXmls[imt->first]->_xmlConstruct, imt->second);

  // next group
  ++groupIndex;
}

void
AncillaryPricingResponseFormatterPostTkt::formatAEFeesResponse(XMLConstruct& construct,
                                                               AncillaryPricingTrx& ancTrx)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::formatAEFeesResponse() - entered");
  // test for any timeout happened before this point
  bool timeOut = false;
  if (nothingShouldBeBack(ancTrx, timeOut))
  {
    // WPAE-BG*Tn was requested but no one fee was collected
    timeOutTrailersForSelectedGroup(ancTrx, construct);
    return;
  }
  // collect all feees for all itins
  if (!getOCFeesForR7U(ancTrx, timeOut))
  {
    // time out trailers because of the incomplete fees collected
    // for all Itins in the request
    timeOutTrailersForSelectedGroup(ancTrx, construct);
    return;
  }
  AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());

  if (ancTrx.itin().size() == 1 && req->noTktRefNumberInR7())
    _useAllForSingelItin = true;

  // loop by group and later by itin
  uint16_t groupIndex = 0;
  for (ServiceFeesGroup* sfg : ancTrx.itin().front()->ocFeesGroup())
    formatAeFeesResponseForGroup(sfg, ancTrx, req, groupIndex);

  if (!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
    for (ServiceFeesGroup* sfg : ancTrx.itin().front()->ocFeesGroupsFreeBag())
      formatAeFeesResponseForGroup(sfg, ancTrx, req, groupIndex);

  // now build whole ITN sections:
  if (_doNotProcess)
    _doNotProcess = false;
  // if any all then add master itin
  if (_isAnyAll || _paxTypes.empty())
  {
    currentItin() = ancTrx.itin().front();
    _curXmlConstructData = _itinXmls[req->masterItin()->getItinOrderNum()];
    // get pax types from all itins
    for (Itin* itin : ancTrx.itin())
    {
      std::vector<PaxType*> thisItinPaxTypes = req->paxType(itin);
      std::copy(thisItinPaxTypes.begin(), thisItinPaxTypes.end(), std::back_inserter(_PnmPaxTypes));
    }
    buildOCFeesResponse(ancTrx, req->masterItin(), construct);
  }
  // now create rermaining itins
  std::map<uint16_t, PostTktResonseItinGrp*>::iterator imt = _itinXmls.begin();
  std::map<uint16_t, PostTktResonseItinGrp*>::iterator ime = _itinXmls.end();
  for (; imt != ime; ++imt)
  {
    // master itin already displayed
    if (imt->first == req->masterItin()->getItinOrderNum())
      continue;

    currentItin() = imt->second->_itin;

    bool isNoServiceGroups;
    if (fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
      isNoServiceGroups = currentItin()->ocFeesGroup().empty();
    else
      isNoServiceGroups =
          currentItin()->ocFeesGroup().empty() && currentItin()->ocFeesGroupsFreeBag().empty();

    if (isNoServiceGroups) // no Service Groups
      continue;

    for (const PaxType* pax : _paxTypes[imt->second->_itin])
    {
      Itin* i = req->paxToOriginalItinMap()[pax];
      if (i != imt->second->_realItin)
        continue;

      _curXmlConstructData = imt->second;
      _PnmPaxTypes.clear();
      _PnmPaxTypes.push_back(pax);
      buildOCFeesResponse(ancTrx, i, construct);
      if (_doNotProcess)
        break;
    }
    if (_doNotProcess)
      break;
  }

  if (_trunkResp)
    _doNotProcess = true;
  // add preformatted data
  if (_agcTagOpened)
    _preformattedMsg.closeElement();
  _agcTagOpened = false;
  construct.openElement(xml2::OCPreformattedMsg);
  if (!_preformattedMsg.getXMLData().empty())
    construct.addElementData(_preformattedMsg.getXMLData().c_str());
  construct.closeElement();
} // end formatAEFeesResponse

void
AncillaryPricingResponseFormatterPostTkt::createOcgSection(XMLConstruct& construct,
                                                           std::vector<XMLConstruct*>& ocgParts)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::createOcgSection() - entered");
  boost::regex xmlDataReg(
      "<([a-zA-Z0-9]*)(\\b[^>]*)>(.*?)</\\1\\s?>|<([a-zA-Z0-9]*)(\\b[^>]*)\\s?/\\s?>");
  boost::regex attrsReg("([\\S]*)=\\\"(\\S*)\\\"");
  std::stringstream contetnt;
  std::map<std::string, std::string> attrMap;
  std::vector<std::string> attrOrder;
  boost::match_results<std::string::const_iterator> m;
  for (XMLConstruct* xmlcon : ocgParts)
  {
    std::string attributeLine;
    if (boost::regex_search(xmlcon->getXMLData(), m, xmlDataReg))
    {
      attributeLine = m[2].matched ? std::string(m[2].first, m[2].second)
                                   : std::string(m[5].first, m[5].second);
      if (m[3].matched)
        contetnt << std::string(m[3].first, m[3].second);
    }
    std::string::const_iterator start = attributeLine.begin(), end = attributeLine.end();
    while (boost::regex_search(start, end, m, attrsReg))
    {
      std::string name = std::string(m[1].first, m[1].second);
      std::string value = std::string(m[2].first, m[2].second);
      if (std::find(attrOrder.begin(), attrOrder.end(), name) == attrOrder.end())
        attrOrder.push_back(name);
      if (name == "Q00" && (attrMap.find(name) != attrMap.end()))
        attrMap[name] = boost::lexical_cast<std::string>(boost::lexical_cast<int>(value) +
                                                         boost::lexical_cast<int>(attrMap[name]));
      else if (name == "ST1")
      {
        for (char& c : value)
          buildST1data(attrMap[name], &c);
      }
      else
        attrMap[name] = value;
      start = m[0].second;
    }
  }
  construct.openElement(xml2::OCGroupInfo); // OCG Open
  for (std::string& key : attrOrder)
    construct.addAttribute(key, attrMap[key]);

  if (!contetnt.str().empty())
    construct.addElementData(contetnt.str().c_str());

  construct.closeElement(); // OCG
}

bool
AncillaryPricingResponseFormatterPostTkt::collectFeesForAllItinsR7(
    std::vector<ServiceFeesGroup*>& ocFeesGroups,
    AncillaryPricingTrx& ancTrx,
    const std::vector<ServiceGroup>& groupCodes,
    const std::vector<PaxType*>& allItinPaxTypes,
    bool timeOutMax,
    uint16_t maxNumberOfOCFees,
    uint16_t& feesCount,
    bool& maxNumOfFeesReached,
    std::map<const Itin*, uint16_t>& dispOnlyFeesCount,
    std::map<const Itin*, uint16_t>& dfeesCount)
{
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = ocFeesGroups.begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = ocFeesGroups.end();
  uint16_t groupIndex = 0;
  for (; sfgIter != sfgIterEnd; ++sfgIter, ++groupIndex)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    std::vector<PaxR7OCFees> paxOcFees;
    std::vector<const ServiceFeesGroup*> allItinSvcGrps;
    switch (sfg->state())
    {
    case ServiceFeesGroup::VALID:
    {
      if (timeOutMax &&
          (find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
      {
        // group code is NOT in the request
        ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
        sfg1->state() = ServiceFeesGroup::EMPTY;
        break;
      }

      if (feesCount >= maxNumberOfOCFees)
      {
        if (!maxNumOfFeesReached)
          checkRequestedGroup(ancTrx, sfgGroupCode);
        maxNumOfFeesReached = true;
        break;
      }
      // collect group by groupIndex from all itins
      for (const Itin* itin : ancTrx.itin())
      {
        allItinSvcGrps.push_back(itin->ocFeesGroup()[groupIndex]);
        if (timeOutMax &&
            !anyFeeForRequestedGroup(ancTrx, itin, sfgGroupCode, itin->ocFeesGroup()[groupIndex]))
          return false;
      }
      paxOcFees = ServiceFeeUtil::getSortedFeesForR7(allItinSvcGrps, allItinPaxTypes);
      // iterate fees
      std::vector<PaxR7OCFees>::iterator paxOcFeesIter = paxOcFees.begin();
      std::vector<PaxR7OCFees>::iterator paxOcFeesIterEnd = paxOcFees.end();
      for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
      {
        if (feesCount >= maxNumberOfOCFees)
        {
          if (!maxNumOfFeesReached)
            checkRequestedGroup(ancTrx, sfgGroupCode);
          maxNumOfFeesReached = true;
          // remove fees exceeding MAX_NUMBER_OF_FEES
          // paxOcFees.erase(paxOcFeesIter, paxOcFeesIterEnd); //Code commented as max number of
          // fees needs to be checked later
          break;
        }

        if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
            (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X')
        {
          dispOnlyFeesCount[paxOcFeesIter->itin()]++;
        }

        if (paxOcFeesIter->paxType() == "ALL")
          _isAnyAll = true;
        else
          _paxTypes[paxOcFeesIter->itin()].insert(paxOcFeesIter->realPaxType());
        dfeesCount[paxOcFeesIter->itin()]++;
      }
    }
    break;

    case ServiceFeesGroup::EMPTY:
    case ServiceFeesGroup::NOT_AVAILABLE:
      break;

    default:
      break;
    }
    _groupFeesVector.push_back(std::make_pair(sfg, paxOcFees));
  }
  return true;
}

bool
AncillaryPricingResponseFormatterPostTkt::getOCFeesForR7(AncillaryPricingTrx& ancTrx,
                                                         bool timeOutMax)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::getOCFeesForR7() - entered");

  // requested group codes
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  // max number of OCFess
  const uint16_t maxNumberOfOCFees = ancTrx.activationFlags().isAB240()
                                         ? maxNumberOfOCFeesAb240Key.getValue()
                                         : maxNumberOfOCFeesKey.getValue();
  // all itin fees count
  uint16_t feesCount = 0;
  std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFees>>> groupFeesVector;

  // max num reached
  bool maxNumOfFeesReached = false;

  AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());

  // all itin pax types
  std::vector<PaxType*> allItinPaxTypes;

  // display count need to be stored separatly by ITN
  std::map<const Itin*, uint16_t> dispOnlyFeesCount;
  std::map<const Itin*, uint16_t> dfeesCount;

  // initialize values
  for (const Itin* itin : ancTrx.itin())
  {
    std::map<const Itin*, bool>::const_iterator guarantPIIter =
        ancReq->ancillNonGuaranteePerItin().find(itin);
    if (guarantPIIter != ancReq->ancillNonGuaranteePerItin().end())
      _ancillariesNonGuaranteeMap[itin] = guarantPIIter->second;
    else
      _ancillariesNonGuaranteeMap[itin] = false;

    dispOnlyFeesCount[itin] = 0;
    dfeesCount[itin] = 0;
    _allFeesDisplayOnly[itin] = false;
    std::vector<PaxType*> itinPaxTypes = ancReq->paxType(itin);
    std::copy(itinPaxTypes.begin(), itinPaxTypes.end(), std::back_inserter(allItinPaxTypes));

    ServiceFeeUtil::createOCFeesUsagesforR7(itin->ocFeesGroup(), static_cast<PricingTrx&>(ancTrx));
    ServiceFeeUtil::createOCFeesUsagesforR7(itin->ocFeesGroupsFreeBag(),
                                            static_cast<PricingTrx&>(ancTrx));
  }

  // for each group collect fees from all itins
  if (!collectFeesForAllItinsR7(ancTrx.itin().front()->ocFeesGroup(),
                                ancTrx,
                                groupCodes,
                                allItinPaxTypes,
                                timeOutMax,
                                maxNumberOfOCFees,
                                feesCount,
                                maxNumOfFeesReached,
                                dispOnlyFeesCount,
                                dfeesCount))
  {
    return false;
  }

  if (!fallback::fixed::AB240_DecoupleServiceFeesAndFreeBag())
  {
    if (!collectFeesForAllItinsR7(ancTrx.itin().front()->ocFeesGroupsFreeBag(),
                                  ancTrx,
                                  groupCodes,
                                  allItinPaxTypes,
                                  timeOutMax,
                                  maxNumberOfOCFees,
                                  feesCount,
                                  maxNumOfFeesReached,
                                  dispOnlyFeesCount,
                                  dfeesCount))
    {
      return false;
    }
  }

  // set display only flags
  for (const Itin* itin : ancTrx.itin())
    if (dfeesCount[itin] && (dispOnlyFeesCount[itin] == dfeesCount[itin]))
      _allFeesDisplayOnly[itin] = true;

  // max fees reached, add to OCF construct resides in AncillaryPricingResponseFormat. object
  if (maxNumOfFeesReached || timeOutMax)
  {
    _trunkResp = true;
    if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
      if (!(ancTrx.itin().size() == 1 && groupCodes.size() == 1) || _allRequestedProcessed)
        formatOCGenericMsg(_ocfXmlConstruct,
                           "MORE AIR EXTRAS AVAILABLE - USE WPAE*T WITH QUALIFIERS");
  }
  return true;
}

bool
AncillaryPricingResponseFormatterPostTkt::collectFeesForAllItinsR7U(
    std::vector<ServiceFeesGroup*>& ocFeesGroups,
    AncillaryPricingTrx& ancTrx,
    const std::vector<ServiceGroup>& groupCodes,
    const std::vector<PaxType*>& allItinPaxTypes,
    bool timeOutMax,
    uint16_t maxNumberOfOCFees,
    uint16_t& feesCount,
    bool& maxNumOfFeesReached,
    std::map<const Itin*, uint16_t>& dispOnlyFeesCount,
    std::map<const Itin*, uint16_t>& dfeesCount)
{
  std::vector<ServiceFeesGroup*>::const_iterator sfgIter = ocFeesGroups.begin();
  std::vector<ServiceFeesGroup*>::const_iterator sfgIterEnd = ocFeesGroups.end();
  uint16_t groupIndex = 0;
  for (; sfgIter != sfgIterEnd; ++sfgIter, ++groupIndex)
  {
    const ServiceFeesGroup* sfg = (*sfgIter);
    const ServiceGroup sfgGroupCode = sfg->groupCode();
    std::vector<PaxR7OCFeesUsages> paxOcFees;
    std::vector<const ServiceFeesGroup*> allItinSvcGrps;
    switch (sfg->state())
    {
    case ServiceFeesGroup::VALID:
    {
      if (timeOutMax &&
          (find(groupCodes.begin(), groupCodes.end(), sfgGroupCode) == groupCodes.end()))
      {
        // group code is NOT in the request
        ServiceFeesGroup* sfg1 = const_cast<ServiceFeesGroup*>(sfg);
        sfg1->state() = ServiceFeesGroup::EMPTY;
        break;
      }

      if (feesCount >= maxNumberOfOCFees)
      {
        if (!maxNumOfFeesReached)
          checkRequestedGroup(ancTrx, sfgGroupCode);
        maxNumOfFeesReached = true;
        break;
      }
      // collect group by groupIndex from all itins
      for (const Itin* itin : ancTrx.itin())
      {
        if (!fallback::fallbackAB240(&ancTrx))
          if (groupIndex < 0 || groupIndex >= itin->ocFeesGroup().size())
            continue;

        allItinSvcGrps.push_back(itin->ocFeesGroup()[groupIndex]);
        if (timeOutMax &&
            !anyFeeForRequestedGroup(ancTrx, itin, sfgGroupCode, itin->ocFeesGroup()[groupIndex]))
          return false;
      }
      paxOcFees = ServiceFeeUtil::getSortedFeesForR7Usages(allItinSvcGrps, allItinPaxTypes);
      // iterate fees
      std::vector<PaxR7OCFeesUsages>::iterator paxOcFeesIter = paxOcFees.begin();
      std::vector<PaxR7OCFeesUsages>::iterator paxOcFeesIterEnd = paxOcFees.end();
      for (; paxOcFeesIter != paxOcFeesIterEnd; ++paxOcFeesIter, ++feesCount)
      {
        if (feesCount >= maxNumberOfOCFees)
        {
          if (!maxNumOfFeesReached)
            checkRequestedGroup(ancTrx, sfgGroupCode);
          maxNumOfFeesReached = true;
          // remove fees exceeding MAX_NUMBER_OF_FEES
          // paxOcFees.erase(paxOcFeesIter, paxOcFeesIterEnd); //Code commented as max number of
          // fees needs to be checked later
          break;
        }

        if ((*paxOcFeesIter).fees()->isDisplayOnly() ||
            (*paxOcFeesIter).fees()->optFee()->notAvailNoChargeInd() == 'X')
          dispOnlyFeesCount[paxOcFeesIter->itin()]++;

        if (paxOcFeesIter->paxType() == "ALL")
          _isAnyAll = true;
        else
          _paxTypes[paxOcFeesIter->itin()].insert(paxOcFeesIter->realPaxType());
        dfeesCount[paxOcFeesIter->itin()]++;
      }
    }
    break;

    case ServiceFeesGroup::EMPTY:
    case ServiceFeesGroup::NOT_AVAILABLE:
      break;

    default:
      break;
    }
    _groupFeesVectorU.push_back(std::make_pair(sfg, paxOcFees));
  }
  return true;
}

bool
AncillaryPricingResponseFormatterPostTkt::getOCFeesForR7U(AncillaryPricingTrx& ancTrx,
                                                          bool timeOutMax)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::getOCFeesForR7U() - entered");
  // requested group codes
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  // max number of OCFess
  const uint16_t maxNumberOfOCFees = ancTrx.activationFlags().isAB240()
                                         ? maxNumberOfOCFeesAb240Key.getValue()
                                         : maxNumberOfOCFeesKey.getValue();

  // all itin fees count
  uint16_t feesCount = 0;
  std::vector<std::pair<const ServiceFeesGroup*, std::vector<PaxOCFeesUsages>>> groupFeesVector;

  // max num reached
  bool maxNumOfFeesReached = false;

  AncRequest* ancReq = static_cast<AncRequest*>(ancTrx.getRequest());

  // all itin pax types
  std::vector<PaxType*> allItinPaxTypes;

  // display count need to be stored separatly by ITN
  std::map<const Itin*, uint16_t> dispOnlyFeesCount;
  std::map<const Itin*, uint16_t> dfeesCount;

  // initialize values
  for (const Itin* itin : ancTrx.itin())
  {
    std::map<const Itin*, bool>::const_iterator guarantPIIter =
        ancReq->ancillNonGuaranteePerItin().find(itin);
    if (guarantPIIter != ancReq->ancillNonGuaranteePerItin().end())
      _ancillariesNonGuaranteeMap[itin] = guarantPIIter->second;
    else
      _ancillariesNonGuaranteeMap[itin] = false;

    dispOnlyFeesCount[itin] = 0;
    dfeesCount[itin] = 0;
    _allFeesDisplayOnly[itin] = false;
    std::vector<PaxType*> itinPaxTypes = ancReq->paxType(itin);
    std::copy(itinPaxTypes.begin(), itinPaxTypes.end(), std::back_inserter(allItinPaxTypes));

    ServiceFeeUtil::createOCFeesUsagesforR7(itin->ocFeesGroup(), static_cast<PricingTrx&>(ancTrx));
    ServiceFeeUtil::createOCFeesUsagesforR7(itin->ocFeesGroupsFreeBag(),
                                            static_cast<PricingTrx&>(ancTrx));
  }

  // for each group collect fees from all itins
  if (!collectFeesForAllItinsR7U(ancTrx.itin().front()->ocFeesGroup(),
                                 ancTrx,
                                 groupCodes,
                                 allItinPaxTypes,
                                 timeOutMax,
                                 maxNumberOfOCFees,
                                 feesCount,
                                 maxNumOfFeesReached,
                                 dispOnlyFeesCount,
                                 dfeesCount))
    return false;

  if (!collectFeesForAllItinsR7U(ancTrx.itin().front()->ocFeesGroupsFreeBag(),
                                 ancTrx,
                                 groupCodes,
                                 allItinPaxTypes,
                                 timeOutMax,
                                 maxNumberOfOCFees,
                                 feesCount,
                                 maxNumOfFeesReached,
                                 dispOnlyFeesCount,
                                 dfeesCount))
    return false;

  // set display only flags
  for (const Itin* itin : ancTrx.itin())
    if (dfeesCount[itin] && (dispOnlyFeesCount[itin] == dfeesCount[itin]))
      _allFeesDisplayOnly[itin] = true;

  // max fees reached, add to OCF construct resides in AncillaryPricingResponseFormat. object
  if (maxNumOfFeesReached || timeOutMax)
  {
    _trunkResp = true;
    if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
      if (!(ancTrx.itin().size() == 1 && groupCodes.size() == 1) || _allRequestedProcessed)
        formatOCGenericMsg(_ocfXmlConstruct,
                           "MORE AIR EXTRAS AVAILABLE - USE WPAE*T WITH QUALIFIERS");
  }
  return true;
}

void
AncillaryPricingResponseFormatterPostTkt::formatOCFeesForR7(AncillaryPricingTrx& ancTrx,
                                                            XMLConstruct& construct,
                                                            bool timeOutMax)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::formatOCFeesForR7() - entered");
  // we already have fees sections created, just copy conntent
  if (_curXmlConstructData)
    construct.addElementData(_curXmlConstructData->_xmlConstruct.getXMLData().c_str());

  if (_allFeesDisplayOnly[currentItin()])
    formatOCTrailer(construct, "AL"); // ST0 - AL
}

std::string
AncillaryPricingResponseFormatterPostTkt::getPaxType(const PaxOCFees& paxOcFees)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::getPaxType() - entered");
  // get displeyable pax type/tkt ino
  std::string paxType = _useAllForSingelItin ? PaxTypeCode("ALL") : paxOcFees.paxType();
  if (paxType != "ALL" && _realPaxType && !_realPaxType->psgTktInfo().empty())
  {
    const PaxType::TktInfo* tki = _realPaxType->psgTktInfo().front();
    paxType = "T" + tki->tktRefNumber();
    // adjust to 3
    if (paxType.size() == 2)
      paxType += " ";
  }
  return paxType;
}

std::string
AncillaryPricingResponseFormatterPostTkt::getPaxType(const PaxOCFeesUsages& paxOcFees)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::getPaxType() - entered");
  // get displeyable pax type/tkt ino
  std::string paxType = _useAllForSingelItin ? PaxTypeCode("ALL") : paxOcFees.paxType();
  if (paxType != "ALL" && _realPaxType && !_realPaxType->psgTktInfo().empty())
  {
    const PaxType::TktInfo* tki = _realPaxType->psgTktInfo().front();
    paxType = "T" + tki->tktRefNumber();
    // adjust to 3
    if (paxType.size() == 2)
      paxType += " ";
  }
  return paxType;
}

void
AncillaryPricingResponseFormatterPostTkt::builPNMData(XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::builPNMData() - entered");
  // create PNM section
  if (_PnmPaxTypes.size())
  {
    construct.openElement(xml2::TicketPsgrNum);
    for (const PaxType* paxType : _PnmPaxTypes)
    {
      for (PaxType::TktInfo* tki : paxType->psgTktInfo())
      {
        construct.openElement(xml2::PsgrNameInfo);
        construct.addAttribute(xml2::NameNum, tki->psgNameNumber());
        construct.addAttribute(xml2::TicketNum, tki->tktNumber());
        construct.addAttribute(xml2::TicketRefNum, tki->tktRefNumber());
        construct.closeElement();
      }
    }
    construct.closeElement();
  }
}
void
AncillaryPricingResponseFormatterPostTkt::buildGroupHeader(const ServiceFeesGroup* sfg,
                                                           XMLConstruct& construct)
{
  LOG4CXX_DEBUG(logger, "AncillaryPricingResponseFormatterPostTkt::buildGroupHeader() - entered");
  // if group header was added in diferent itn - do nothing
  if (std::find(_processedGroups.begin(), _processedGroups.end(), sfg->groupCode()) !=
      _processedGroups.end())
    return;

  if (_agcTagOpened)
    _preformattedMsg.closeElement();

  _preformattedMsg.openElement(xml2::AncillaryGroupCode); // AGC Open
  _preformattedMsg.addAttribute(xml2::OCFeeGroupCode, sfg->groupCode().c_str());
  _agcTagOpened = true;

  _processedGroups.push_back(sfg->groupCode());
  AncillaryPricingResponseFormatter::buildGroupHeader(sfg, _preformattedMsg);
}

void
AncillaryPricingResponseFormatterPostTkt::formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                                                XMLConstruct& construct,
                                                                const PaxOCFees& paxOcFees,
                                                                const uint16_t index,
                                                                const char& indicator)
{
  // use preformated XML construct
  AncillaryPricingResponseFormatter::formatOCFeesLineForR7(
      ancTrx, _preformattedMsg, paxOcFees, index, indicator);
}

void
AncillaryPricingResponseFormatterPostTkt::formatOCFeesLineForR7(AncillaryPricingTrx& ancTrx,
                                                                XMLConstruct& construct,
                                                                const PaxOCFeesUsages& paxOcFees,
                                                                const uint16_t index,
                                                                const char& indicator)
{
  // use preformated XML construct
  AncillaryPricingResponseFormatter::formatOCFeesLineForR7(
      ancTrx, _preformattedMsg, paxOcFees, index, indicator);
}
bool
AncillaryPricingResponseFormatterPostTkt::ancillariesNonGuarantee()
{
  return _ancillariesNonGuaranteeMap[currentItin()];
}

void
AncillaryPricingResponseFormatterPostTkt::buildSegmentsTickeCoupons(AncillaryPricingTrx& ancTrx,
                                                                    XMLConstruct& construct,
                                                                    const PaxOCFees& paxOcFees)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::buildSegmentsTickeCoupons() - entered");
  std::vector<const Itin*> itinsToDisplay;
  if (paxOcFees.paxType() == "ALL")
    std::copy(ancTrx.itin().begin(), ancTrx.itin().end(), std::back_inserter(itinsToDisplay));
  else
    itinsToDisplay.push_back(paxOcFees.fees()->farePath()->itin());

  AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());
  for (const Itin* i : itinsToDisplay)
  {
    for (PaxType* pax : req->paxType(i))
    {
      Itin* itin = req->paxToOriginalItinMap()[pax];

      if (_curXmlConstructData != nullptr && paxOcFees.paxType() != "ALL" &&
          itin != _curXmlConstructData->_realItin)
        continue;

      for (PaxType::TktInfo* tki : pax->psgTktInfo())
      {
        construct.openElement(xml2::OOSTicketCouponAssoc);
        construct.addAttribute(xml2::TicketRefNum, tki->tktRefNumber());
        std::vector<tse::TravelSeg*>::const_iterator segI =
            std::find_if(itin->travelSeg().begin(),
                         itin->travelSeg().end(),
                         FindPointByPnrOrder(paxOcFees.fees()->travelStart()));
        std::vector<tse::TravelSeg*>::const_iterator segE =
            std::find_if(itin->travelSeg().begin(),
                         itin->travelSeg().end(),
                         FindPointByPnrOrder(paxOcFees.fees()->travelEnd()));
        for (; segI != itin->travelSeg().end() && segI != segE + 1; segI++)
        {
          if ((*segI)->pnrSegment() == ARUNK_PNR_SEGMENT_ORDER)
            continue;
          construct.openElement(xml2::OOSFlightCoupon);
          char tmpBuf[10];
          sprintf(tmpBuf, "%d", (*segI)->ticketCouponNumber());
          construct.addElementData(tmpBuf);
          construct.closeElement();
        }
        construct.closeElement();
      }
    }
  }
}

void
AncillaryPricingResponseFormatterPostTkt::buildSegmentsTickeCoupons(
    AncillaryPricingTrx& ancTrx, XMLConstruct& construct, const PaxOCFeesUsages& paxOcFees)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::buildSegmentsTickeCoupons() - entered");
  std::vector<const Itin*> itinsToDisplay;
  if (paxOcFees.paxType() == "ALL")
    std::copy(ancTrx.itin().begin(), ancTrx.itin().end(), std::back_inserter(itinsToDisplay));
  else
    itinsToDisplay.push_back(paxOcFees.fees()->farePath()->itin());

  AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());
  for (const Itin* i : itinsToDisplay)
  {
    for (PaxType* pax : req->paxType(i))
    {
      Itin* itin = req->paxToOriginalItinMap()[pax];

      if (_curXmlConstructData != nullptr && paxOcFees.paxType() != "ALL" &&
          itin != _curXmlConstructData->_realItin)
        continue;

      for (PaxType::TktInfo* tki : pax->psgTktInfo())
      {
        construct.openElement(xml2::OOSTicketCouponAssoc);
        construct.addAttribute(xml2::TicketRefNum, tki->tktRefNumber());
        std::vector<tse::TravelSeg*>::const_iterator segI =
            std::find_if(itin->travelSeg().begin(),
                         itin->travelSeg().end(),
                         FindPointByPnrOrder(paxOcFees.fees()->travelStart()));
        std::vector<tse::TravelSeg*>::const_iterator segE =
            std::find_if(itin->travelSeg().begin(),
                         itin->travelSeg().end(),
                         FindPointByPnrOrder(paxOcFees.fees()->travelEnd()));
        for (; segI != itin->travelSeg().end() && segI != segE + 1; segI++)
        {
          if ((*segI)->pnrSegment() == ARUNK_PNR_SEGMENT_ORDER)
            continue;
          construct.openElement(xml2::OOSFlightCoupon);
          char tmpBuf[10];
          sprintf(tmpBuf, "%d", (*segI)->ticketCouponNumber());
          construct.addElementData(tmpBuf);
          construct.closeElement();
        }
        construct.closeElement();
      }
    }
  }
}
void
AncillaryPricingResponseFormatterPostTkt::checkRequestedGroup(AncillaryPricingTrx& ancTrx,
                                                              const ServiceGroup& sg)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::checkRequestedGroup() - entered");
  // requested group codes
  if (!ancTrx.getOptions()->serviceGroupsVec().empty())
  {
    std::vector<ServiceGroup> groupCodes; // requested Group codes
    ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);
    if (!groupCodes.empty() && !_allRequestedProcessed &&
        find(groupCodes.begin(), groupCodes.end(), sg) == groupCodes.end())
      _allRequestedProcessed = true;
  }
}

bool
AncillaryPricingResponseFormatterPostTkt::anyFeeForRequestedGroup(AncillaryPricingTrx& ancTrx,
                                                                  const Itin* itin,
                                                                  const ServiceGroup& sg,
                                                                  ServiceFeesGroup* sfg)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::anyFeeForRequestedGroup() - entered");
  // requested group codes
  if (!ancTrx.getOptions()->serviceGroupsVec().empty())
  {
    AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());
    std::vector<ServiceGroup> groupCodes; // requested Group codes
    ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);
    if (!groupCodes.empty() && find(groupCodes.begin(), groupCodes.end(), sg) != groupCodes.end())
    {
      if (sfg->groupCode() == sg &&
          (sfg->ocFeesMap().size() < 1 ||
           sfg->ocFeesMap().size() != req->pricingItins().find(itin)->second.size()))
        return false;
    }
  }
  return true; // no group selected
}

bool
AncillaryPricingResponseFormatterPostTkt::nothingShouldBeBack(AncillaryPricingTrx& ancTrx,
                                                              bool& timeOutMax)
{
  LOG4CXX_DEBUG(logger,
                "AncillaryPricingResponseFormatterPostTkt::nothingShouldBeBack() - entered");
  // requested group codes
  std::vector<ServiceGroup> groupCodes; // requested Group codes
  ancTrx.getOptions()->getGroupCodes(ancTrx.getOptions()->serviceGroupsVec(), groupCodes);

  std::vector<Itin*>::const_iterator itinI = ancTrx.itin().begin();
  for (; itinI != ancTrx.itin().end(); ++itinI)
  {
    currentItin() = *itinI;
    if ((*itinI)->timeOutForExceededSFGpresent())
    {
      timeOutMax = true;
      return false;
    }
    if ((*itinI)->timeOutForExceeded() && !groupCodes.empty())
      return true;
  }
  return false;
}

void
AncillaryPricingResponseFormatterPostTkt::timeOutTrailersForSelectedGroup(
    AncillaryPricingTrx& ancTrx, XMLConstruct& construct)
{
  LOG4CXX_DEBUG(
      logger,
      "AncillaryPricingResponseFormatterPostTkt::timeOutTrailersForSelectedGroup() - entered");
  Itin* itin = currentItin();
  // time out for WPAE-BG*Tn entry right at OC process starts
  construct.openElement(xml2::ItinInfo);
  construct.addAttributeInteger(xml2::GenId, itin->getItinOrderNum());

  if (ancTrx.billing()->requestPath() != SWS_PO_ATSE_PATH)
  {
    AncRequest* req = static_cast<AncRequest*>(ancTrx.getRequest());
    if (ancTrx.itin().size() == 1 &&
        (req->paxType(itin).size() == 1 || (req->paxType(itin).size() > 1 && samePaxType(ancTrx))))
      formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER");
    else if (itin->timeOutForExceeded() || itin->timeOutForExceededSFGpresent())
      formatOCGenericMsg(construct, "MAX AIR EXTRAS EXCEEDED/USE AE SVC QUALIFIER OR TKT SELECT");
  }

  emptyOCFeeGroups(construct);
  construct.closeElement();
  LOG4CXX_DEBUG(
      logger,
      "AncillaryPricingResponseFormatterPostTkt::timeOutTrailersForSelectedGroup() - leaving");
}
}
