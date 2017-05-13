#include "Xform/FrequentFlyerResponseFormatter.h"

#include "Common/FreqFlyerUtils.h"
#include "Common/XMLConstruct.h"
#include "DataModel/FreqFlyerStatusData.h"
#include "DBAccess/FreqFlyerStatus.h"
#include "Xform/PricingResponseXMLTags.h"


#include <string>

namespace tse
{
static const std::string FF_RESPONSE = "FrequentFlyerResponse";
static const std::string FF_DATA = "FFData";
static const std::string CARRIER_CODE = "CarrierCode";
static const std::string CARRIER_NOT_FOUND = "CarrierNotFound";
static const std::string TIER_DATA = "TierData";

std::string
FrequentFlyerResponseFormatter::formatResponse()
{
  XMLConstruct xml;
  xml.openElement(FF_RESPONSE);
  for (const auto& cxr : _trx.getCxrs())
    formatFFData(xml, cxr);

  xml.closeElement();
  return xml.getXMLData();
}

void
FrequentFlyerResponseFormatter::formatResponse(const ErrorResponseException& ere,
                                               std::string& response)
{
  XMLConstruct xml;
  xml.openElement(FF_RESPONSE);
  prepareMessage(xml, Message::TYPE_ERROR, Message::errCode(ere.code()), ere.message());
  xml.closeElement();

  response = xml.getXMLData();
}

void
FrequentFlyerResponseFormatter::prepareMessage(XMLConstruct& construct,
                                               const char msgType,
                                               const uint16_t lineNum,
                                               const std::string& msgText)
{
  construct.openElement(xml2::MessageInformation);
  construct.addAttributeChar(xml2::MessageType, msgType);
  construct.addAttributeShort(xml2::MessageFailCode, lineNum);
  construct.addAttribute(xml2::MessageText, msgText);
  construct.closeElement();
}

void
FrequentFlyerResponseFormatter::formatFFData(XMLConstruct& xml, const CarrierData& cxr)
{
  xml.openElement(FF_DATA);
  xml.addAttribute(CARRIER_CODE, cxr.first);
  if (!freqflyerutils::checkCarrierPresenceInTableA03(cxr.second))
    xml.addAttribute(CARRIER_NOT_FOUND, "T");

  for (const auto& levelData : cxr.second)
    formatTierData(xml, levelData);
  xml.closeElement();
}

void
FrequentFlyerResponseFormatter::formatTierData(XMLConstruct& xml, const FreqFlyerStatusData& data)
{
  xml.openElement(TIER_DATA);
  xml.addAttributeInteger("Level", data._dbData->_level);
  xml.addAttribute("LevelName", data._dbData->_statusLevel);
  xml.addAttributeInteger("MaxPaxSamePNR", data._dbData->_maxPassengersSamePNR);
  xml.addAttributeInteger("MaxPaxDifferentPNR", data._dbData->_maxPassengersDiffPNR);
  xml.addAttributeInteger("MaxPaxTotal", data._maxPassengersTotal);
  xml.closeElement();
}
}
