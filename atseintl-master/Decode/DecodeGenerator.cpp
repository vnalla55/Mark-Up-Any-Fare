//----------------------------------------------------------------------------
//
// Copyright Sabre 2014
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------

#include "Decode/DecodeGenerator.h"

#include "Common/CarrierUtil.h"
#include "Common/ErrorResponseException.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "DataModel/DecodeTrx.h"
#include "DBAccess/AirlineAllianceCarrierInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"
#include "DBAccess/State.h"
#include "DBAccess/TaxSpecConfigReg.h"

#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>

#include <sstream>

namespace tse
{
static Logger
logger("atseintl.Decode.DecodeGenerator");

void
DecodeGenerator::generate() const
{
  LOG4CXX_INFO(logger, "Entering DecodeGenerator::generate");

  if (CarrierUtil::isAllianceCode(_locCode))
    generateAlianceCarrierList();
  else if (_locCode == EastCoastCode || _locCode == WestCoastCode)
    generateGenericCityList();
  else
    generateZoneList();
}

void
DecodeGenerator::generateAlianceCarrierList() const
{
  const std::vector<AirlineAllianceCarrierInfo*>& aaCrxInfos =
      _trx.dataHandle().getGenericAllianceCarrier(_locCode);

  if (aaCrxInfos.empty())
    throw ErrorResponseException(ErrorResponseException::NOT_IN_TBL, getErrMsg().c_str());

  std::ostringstream response;

  response << _locCode << " " << aaCrxInfos[0]->genericName() << "\nINCLUDES AIRLINES ";

  std::vector<AirlineAllianceCarrierInfo*>::const_iterator aaCrxInfoIt = aaCrxInfos.begin();

  response << (*aaCrxInfoIt)->carrier();
  ++aaCrxInfoIt;
  for (; aaCrxInfoIt != aaCrxInfos.end(); ++aaCrxInfoIt)
  {
    response << ", " << (*aaCrxInfoIt)->carrier();
  }

  _trx.addToResponse(response);
}

void
DecodeGenerator::generateGenericCityList() const
{
  std::string statesList;
  TaxSpecConfigName configName = "RTW" + _locCode;

  const std::vector<TaxSpecConfigReg*>& taxSpecVec = _trx.dataHandle().getTaxSpecConfig(configName);
  if (taxSpecVec.empty())
    throw ErrorResponseException(ErrorResponseException::NOT_IN_TBL, getErrMsg().c_str());

  for (const TaxSpecConfigReg::TaxSpecConfigRegSeq* taxSpecSeq : taxSpecVec[0]->seqs())
  {
    statesList += taxSpecSeq->paramValue();
  }

  std::string description(taxSpecVec[0]->getDescription());

  boost::algorithm::ireplace_last(description, " INCLUDE", "\nINCLUDE");

  _trx.addToResponse(description);
  _trx.addToResponse(statesList);
}

void
DecodeGenerator::generateZoneState(std::string& nationsList,
                                   std::string& statesList,
                                   const ZoneInfo::ZoneSeg& seg) const
{
  DateTime dt = DateTime::localTime();
  if (seg.locType() == LOCTYPE_NATION)
  {
    const Nation* nation = _trx.dataHandle().getNation(seg.loc(), dt);
    if (!nation)
      throw ErrorResponseException(ErrorResponseException::NOT_IN_TBL, getErrMsg().c_str());

    if (!nationsList.empty())
      nationsList += ", ";
    nationsList += nation->description();
  }
  else if (seg.locType() == LOCTYPE_STATE)
  {
    NationCode nationCode = seg.loc().substr(0, 2);
    StateCode stateCode = seg.loc().substr(2, 2);
    const State* state = _trx.dataHandle().getState(nationCode, stateCode, dt);
    if (!state)
      throw ErrorResponseException(ErrorResponseException::NOT_IN_TBL, getErrMsg().c_str());

    if (!statesList.empty())
      statesList += ", ";
    std::string::size_type pos = state->description().find(',');
    if (pos != std::string::npos)
      statesList += state->description().substr(0, pos);
    else
      statesList += state->description();
  }
}

void
DecodeGenerator::generateZoneList() const
{
  DateTime dt = DateTime::localTime();
  Zone zone = _locCode;

  LocUtil::padZoneNo(zone);
  const VendorCode vendor = ATPCO_VENDOR_CODE;
  const ZoneInfo* zoneInfo = _trx.dataHandle().getZone(vendor, zone, RESERVED, dt);
  if (!zoneInfo || zoneInfo->sets().empty())
    throw ErrorResponseException(ErrorResponseException::NOT_IN_TBL, getErrMsg().c_str());

  NationStatelist nationStatelist;

  for (const std::vector<ZoneInfo::ZoneSeg>& zoneSet : zoneInfo->sets())
  {
    for (const ZoneInfo::ZoneSeg& seg : zoneSet)
    {
      std::string& nationsList = seg.inclExclInd() == 'I' ? std::get<INC_NATION>(nationStatelist)
                                                          : std::get<EXC_NATION>(nationStatelist);
      std::string& statesList = seg.inclExclInd() == 'I' ? std::get<INC_STATE>(nationStatelist)
                                                         : std::get<EXC_STATE>(nationStatelist);

      generateZoneState(nationsList, statesList, seg);
    }
  }

  prepareZoneMessage(zoneInfo->getDescription(), nationStatelist);
}

void
DecodeGenerator::prepareZoneMessage(const std::string& zoneDesc,
                                    const NationStatelist& nationStatelist) const
{
  std::ostringstream response;

  try
  {
    boost::lexical_cast<short>(_locCode);
    response << std::setfill('0') << std::setw(3) << _locCode;
  }
  catch(boost::bad_lexical_cast &)
  {
    response << _locCode;
  }

  response << " ZONE - " << zoneDesc << "\n";
  bool needComma = false;

  if (!std::get<INC_NATION>(nationStatelist).empty())
  {
    if (std::get<INC_NATION>(nationStatelist).find(',') == std::string::npos)
      response << "INCLUDES NATION OF ";
    else
      response << "INCLUDES NATIONS OF ";
    response << std::get<INC_NATION>(nationStatelist);

    needComma = true;
  }

  if (!std::get<EXC_NATION>(nationStatelist).empty())
  {
    if (needComma)
      response << ", ";

    if (std::get<EXC_NATION>(nationStatelist).find(',') == std::string::npos)
      response << "EXCLUDES NATION OF " << std::get<EXC_NATION>(nationStatelist);
    else
      response << "EXCLUDES NATIONS OF " << std::get<EXC_NATION>(nationStatelist);

    needComma = true;
  }
  if (!std::get<INC_STATE>(nationStatelist).empty())
  {
    if (needComma)
      response << ", ";

    if (std::get<INC_STATE>(nationStatelist).find(',') == std::string::npos)
      response << "INCLUDES STATE OF " << std::get<INC_STATE>(nationStatelist);
    else
      response << "INCLUDES STATES OF " << std::get<INC_STATE>(nationStatelist);

    needComma = true;
  }
  if (!std::get<EXC_STATE>(nationStatelist).empty())
  {
    if (needComma)
      response << ", ";

    if (std::get<EXC_STATE>(nationStatelist).find(',') == std::string::npos)
      response << "EXCLUDES STATE OF " << std::get<EXC_STATE>(nationStatelist);
    else
      response << "EXCLUDES STATES OF " << std::get<EXC_STATE>(nationStatelist);
  }

  _trx.addToResponse(response);
}

std::string
DecodeGenerator::getErrMsg() const
{
  return std::string("NOT IN TBL -") + _locCode;
}

} // tse
