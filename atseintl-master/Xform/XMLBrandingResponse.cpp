//-------------------------------------------------------------------
//
//  Authors:     Piotr Bartosik
//
//  Copyright Sabre 2013
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

#include "Xform/XMLBrandingResponse.h"

#include "BrandedFares/BrandedFaresComparator.h"
#include "Common/Assert.h"
#include "Common/Logger.h"
#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "Xform/CommonFormattingUtils.h"
#include "Xform/ShoppingResponseNames.h"

namespace tse
{
FALLBACK_DECL(fallbackMipBForTNShopping);

Logger XMLBrandingResponse::_logger("atseintl.Xform.XMLBrandingResponse");

void
XMLBrandingResponse::setDiagOutput(int diagType, const std::string& diagText)
{
  TSE_ASSERT(!diagText.empty());
  _diagType = diagType;
  _diagText = diagText;
}

void
XMLBrandingResponse::buildXmlBrandingResponse(std::string& outputString,
                                              const BrandingResponseType& response)
{
  XMLWriter w;
  {
    // Node insertion is RAII-controlled -- use additional brackets
    Node shoppingResponseNode(w, ShoppingResponseNames::SHOPPING_RESPONSE_TAG);
    if (!_token.empty())
    {
      shoppingResponseNode.convertAttr(ShoppingResponseNames::TOKEN_ATTR, _token);
    }
    shoppingResponseNode.convertAttr(ShoppingResponseNames::Q0S_ATTR, response.size());

    // If response contains diagnostic, add it to XML
    if (!_diagText.empty())
    {
      createDiagnosticNode(w, _diagType, _diagText);
    }

    responseStructToNodes(w, response);
  }
  w.result(outputString);
}

namespace {

void
printProgramsIntoLegs(
    const BrandingTrx& trx, XMLWriter& w, const Itin& itin, const std::string& brandCode,
    const ShoppingUtil::LegBrandQualifiedIndex& legBrandQualifiedIndex)
{
  for (const auto& tSegs: itin.itinLegs())
  {
    if ((tSegs.front()->segmentType() == Arunk) && (tSegs.size() == 1))
      continue; // Do not display ARUNK legs

    XMLWriter::Node legNode(w, ShoppingResponseNames::LEG_TAG);
    legNode.convertAttr(ShoppingResponseNames::LEG_ID_ATTR, tSegs.front()->legId());

    TSE_ASSERT(legBrandQualifiedIndex.find(tSegs.front()->legId()) !=
               legBrandQualifiedIndex.end());
    const ShoppingUtil::BrandQualifiedIndex& legBrandIndexMap =
      legBrandQualifiedIndex.at(tSegs.front()->legId());

    TSE_ASSERT(legBrandIndexMap.find(brandCode) != legBrandIndexMap.end());
    TSE_ASSERT(legBrandIndexMap.at(brandCode) < trx.brandProgramVec().size());

    const QualifiedBrand& qb = trx.brandProgramVec()[legBrandIndexMap.at(brandCode)];
    xform::formatBrandProgramData(legNode, qb);
  }
}

}

void
XMLBrandingResponse::responseStructToNodes(XMLWriter& w,
                                           const BrandingResponseType& responseStruct)
{
  BrandedFaresComparator comparator(_trx.brandProgramVec(), _logger);

  for (const auto& elem : responseStruct)
  {
    Node itinNode(w, ShoppingResponseNames::ITINERARY_TAG);
    itinNode.convertAttr(ShoppingResponseNames::NUMBER_ATTR, elem.first->itinNum());

    if (!fallback::fallbackMipBForTNShopping(&_trx) && _trx.isBrandsForTnShopping())
    {
      itinNode.convertAttr(ShoppingResponseNames::HAS_ANY_BRAND,
                           (elem.second.empty() ? "F" : "T"));
    }

    const ProgramsForBrandMap& proForB = elem.second;

    std::vector<std::string> sortedBrandCodes;
    for (ProgramsForBrandMap::const_iterator jt = proForB.begin(); jt != proForB.end(); ++jt)
    {
      const BrandCode& brandCode = jt->first;

      if (!fallback::fallbackMipBForTNShopping(&_trx) && brandCode == ANY_BRAND)
          continue;

      sortedBrandCodes.push_back(brandCode);
    }
    std::sort(sortedBrandCodes.begin(), sortedBrandCodes.end(), comparator);

    const ShoppingUtil::LegBrandQualifiedIndex legBrandQualifiedIndex =
      ShoppingUtil::createLegBrandIndexRelation(_trx, *elem.first);

    for (size_t i = 0; i < sortedBrandCodes.size(); ++i)
    {
      Node brandNode(w, ShoppingResponseNames::GROUPFARE_TAG);

      const std::string& brandCode = sortedBrandCodes[i];
      brandNode.convertAttr(xml2::BrandCode, brandCode);

      ProgramsForBrandMap::const_iterator progIt = proForB.find(brandCode);
      TSE_ASSERT(progIt != proForB.end());
      printProgramsIntoBrands(w, progIt->second);

      printProgramsIntoLegs(_trx, w, *elem.first, brandCode, legBrandQualifiedIndex);
    }
  }
}

void
XMLBrandingResponse::printProgramsIntoBrands(XMLWriter& w, const ProgramIdSet& programSet)
{
  for (const auto& elem : programSet)
  {
    Node programNode(w, ShoppingResponseNames::PROGRAM_INFO_TAG);
    programNode.convertAttr(xml2::ProgramCode, elem);
  }
}

void
XMLBrandingResponse::createDiagnosticNode(XMLWriter& w, int diagType, const std::string& diagText)
{
  Node nodeDIA(w, ShoppingResponseNames::DIAGNOSTIC_RESPONSE_TAG);
  nodeDIA.convertAttr(ShoppingResponseNames::DIAGNOSTIC_CODE_ATTR, diagType);
  nodeDIA.addData(diagText);
}

} // namespace tse
