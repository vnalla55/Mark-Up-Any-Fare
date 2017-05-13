#include "Xform/XMLBaggageResponse.h"

#include "Common/CurrencyUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/BaggageTravel.h"
#include "DataModel/FarePath.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "ServiceFees/OCFees.h"
#include "Util/IteratorRange.h"
#include "Xform/PricingResponseXMLTags.h"

#include <boost/lexical_cast.hpp>
#include <sstream>

namespace tse
{

XMLBaggageResponse::XMLBaggageResponse(XMLWriter& writer)
: _writer(writer)
{
}

void
XMLBaggageResponse::generateBDI(const FarePath& farePath)
{
  for (const BaggageTravel* baggageTravel : farePath.baggageTravels())
  {
    generateAllowanceBDI(*baggageTravel);
    if (TrxUtil::isBaggageChargesInMipActivated(*baggageTravel->_trx))
      generateChargeBDI(*baggageTravel);
  }
}

void
XMLBaggageResponse::generateAllowanceBDI(const BaggageTravel& bt)
{
  if (!bt._allowance || !bt._allowance->optFee())
    return;

  XMLWriter::Node nodeBDI(_writer, xml2::BaggageBDI);
  generateAllowanceAttributes(nodeBDI, *bt._allowance->optFee());
  generateQ00(bt);
}

void
XMLBaggageResponse::generateChargeBDI(const BaggageTravel& bt)
{
  for (const BaggageCharge* bc : bt._charges)
  {
    if (!bc || !bc->optFee())
      continue;

    XMLWriter::Node nodeBDI(_writer, xml2::BaggageBDI);
    generateChargeAttributes(nodeBDI, *bc);
    generateQ00(bt);
  }
}

void
XMLBaggageResponse::generateAllowanceAttributes(XMLWriter::Node& node,
                                                const OptionalServicesInfo& s7) const
{
  if (s7.freeBaggagePcs() > 0 || s7.baggageWeightUnit() == ' ')
    // BPC - Number of Pieces
    node.convertAttr(xml2::BaggageBPC, s7.freeBaggagePcs());

  if (s7.baggageWeightUnit() != ' ')
  {
    // B20 - Weight Limit
    node.convertAttr(xml2::BaggageSizeWeightLimit, s7.baggageWeight());

    // N0D - Units of the Weight Limit
    node.convertAttr(xml2::BaggageSizeWeightUnitType, s7.baggageWeightUnit() == 'K' ? 'K' : 'L');
  }
}

void
XMLBaggageResponse::generateChargeAttributes(XMLWriter::Node& node, const BaggageCharge& bc) const
{
  const OptionalServicesInfo& s7 = *bc.optFee();

  // C52 - Equivalent amount
  node.convertAttr(xml2::SUMEquiBasePrice, CurrencyUtil::toString(bc.feeAmount(), bc.feeNoDec()));

  // C5B - Equivalent currency
  node.convertAttr(xml2::SUMEquiCurCode, bc.feeCurrency());

  // SHK
  std::ostringstream oss;
  oss << s7.vendor() << s7.carrier() << s7.serviceSubTypeCode() << s7.seqNo();
  node.convertAttr(xml2::ExtendedSubCodeKey, oss.str());

  // OC1 - First Occurrence
  if (s7.baggageOccurrenceFirstPc() > 0)
    node.convertAttr(xml2::BaggageOC1, s7.baggageOccurrenceFirstPc());

  // OC2 - Last Occurrence
  if (s7.baggageOccurrenceLastPc() > 0)
    node.convertAttr(xml2::BaggageOC2, s7.baggageOccurrenceLastPc());
}

void
XMLBaggageResponse::generateQ00(const BaggageTravel& bt)
{
  for (const TravelSeg* travelSeg :
       makeIteratorRange(bt.getTravelSegBegin(), bt.getTravelSegEnd()))
  {
    if (travelSeg->pnrSegment() && travelSeg->pnrSegment() != ARUNK_PNR_SEGMENT_ORDER)
    {
      // Q00 - Price and Fulfillment Information
      XMLWriter::Node nodeQ00(_writer, xml2::TravelSegId);
      nodeQ00.addSimpleText(boost::lexical_cast<std::string>(travelSeg->pnrSegment() - 1));
    }
  }
}

}
