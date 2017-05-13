// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2014
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------

#include "Common/DefaultValidatingCarrierFinder.h"
#include "Common/ErrorResponseException.h"
#include "Common/FallbackUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FarePath.h"
#include "DataModel/NetFarePath.h"
#include "DataModel/NetRemitFarePath.h"
#include "DataModel/RequestResponse/OutputItins.h"
#include "DataModel/RequestResponse/OutputTaxGroup.h"
#include "Taxes/Pfc/PfcItem.h"
#include "Taxes/AtpcoTaxes/Diagnostic/AtpcoDiagnostic.h"
#include "Taxes/LegacyFacades/ConvertCode.h"
#include "Taxes/LegacyFacades/FacadesUtils.h"
#include "Taxes/LegacyFacades/ForEachTaxResponse.h"
#include "Taxes/LegacyFacades/GsaLogic.h"
#include "Taxes/LegacyFacades/V2TrxMappingDetails.h"
#include "Taxes/LegacyTaxes/TaxItem.h"

namespace tse
{
FALLBACK_DECL(fallbackValidatingCxrMultiSp);
}

namespace tax
{

namespace
{

struct FarePathKey
{
  tse::Itin* itin;
  tse::FarePath* mainFarePath;

  FarePathKey(tse::Itin* i, tse::FarePath* mfp) : itin(i), mainFarePath(mfp) {}

  friend bool operator<(const FarePathKey& lhs, const FarePathKey& rhs)
  {
    if (std::less<const tse::FarePath*>()(lhs.mainFarePath, rhs.mainFarePath)) return true;
    if (std::less<const tse::FarePath*>()(rhs.mainFarePath, lhs.mainFarePath)) return false;

    if (std::less<const tse::Itin*>()(lhs.itin, rhs.itin)) return true;
    if (std::less<const tse::Itin*>()(rhs.itin, lhs.itin)) return false;

    return false;
  }
};

struct FarePathRef
{
  tse::FarePath* farePath;
  type::Index    index;

  FarePathRef(tse::FarePath* fp, type::Index i) : farePath(fp), index(i) {}

  friend bool operator==(const FarePathRef& lhs, const FarePathRef& rhs)
  {
    return lhs.farePath == rhs.farePath && lhs.index == rhs.index;
  }
};

struct MappingRef
{
  const tax::ItinFarePathMapping::value_type* mapping;
  type::Index index;

  MappingRef(const tax::ItinFarePathMapping::value_type* m, type::Index i) :
    mapping(m), index(i) {}

  friend bool operator==(const MappingRef& lhs, const MappingRef& rhs)
  {
    return lhs.mapping == rhs.mapping && lhs.index == rhs.index;
  }
};

struct FarePathMapVal
{
  boost::optional<type::MoneyAmount> val;
  std::vector<FarePathRef> farePaths;
  std::shared_ptr<tax::OutputTaxGroup> referenceTaxGroup;
  std::vector<MappingRef> mappings;
  //

  FarePathMapVal() {}
  FarePathMapVal(type::MoneyAmount v,
                 tse::FarePath* fp,
                 type::Index i,
                 const std::shared_ptr<tax::OutputTaxGroup> taxGroup,
                 const tax::ItinFarePathMapping::value_type* mapping)
    : val(v),
      farePaths(1, FarePathRef(fp, i)),
      referenceTaxGroup(taxGroup),
      mappings(1, MappingRef(mapping, i))
  { }
  void addFarePath(tse::FarePath* fp, type::Index i)
  {
    farePaths.push_back(FarePathRef(fp, i));
  }
  void addMapping(const tax::ItinFarePathMapping::value_type* mapping, type::Index i)
  {
    mappings.push_back(MappingRef(mapping, i));
  }
  tse::FarePath* altFare1() const
  {
    assert (!farePaths.empty());
    return farePaths.front().farePath;
  }
};

typedef std::map<FarePathKey, FarePathMapVal> BestFarePath;

tse::FarePath* find1stAltPath(tse::FarePath* mainFP, const BestFarePath& bestFPMap)
{
  for (const BestFarePath::value_type& link : bestFPMap)
  {
    if (link.first.mainFarePath == mainFP)
      return link.second.altFare1();
  }
  assert (false);
  return nullptr;
}

void changeItinFarePaths(const ItinFarePathMapping& mappings, const BestFarePath& bestFarePaths)
{
  std::vector<tse::Itin*> itins;

  for(const tax::ItinFarePathMapping::value_type& mapping : mappings)
  {
    itins.push_back(std::get<0>(mapping));
  }

  std::sort(itins.begin(), itins.end());
  itins.erase(std::unique(itins.begin(), itins.end()), itins.end());

  for (tse::Itin* itin : itins)
  {
    std::vector<tse::FarePath*> substitutes;
    for (tse::FarePath* fp : itin->farePath())
    {
      substitutes.push_back(find1stAltPath(fp, bestFarePaths));
    }
    itin->farePath().swap(substitutes);
  }
}

tse::FarePath* findMainFarePath(const tse::FarePath* altFarePath, const FarePathMap& fpMap)
{
  for(const FarePathLink& link : fpMap)
  {
    if (link.altPath == altFarePath)
      return link.mainPath;
  }

  assert (false);
  return nullptr;
}

std::shared_ptr<tax::OutputTaxGroup>
getTaxGroup(const OutputItin& solution, const tax::OutputItins& solutions)
{
  const boost::optional<type::Index>& taxGroupId = solution.taxGroupId();
  assert (taxGroupId);
  const std::shared_ptr<tax::OutputTaxGroup>& taxGroup = solutions.taxGroupSeq()[*taxGroupId];
  return taxGroup;
}

type::MoneyAmount
totalTax(const std::shared_ptr<tax::OutputTaxGroup>& taxGroup, const tax::OutputItins& solutions)
{
  assert (taxGroup);
  type::MoneyAmount ans;

  for(const OutputTax& outTax : taxGroup->taxSeq())
  {
    for (const OutputTaxDetailsRef& taxDetailsRef : outTax.taxDetailsRef())
    {
      const std::shared_ptr<OutputTaxDetails>& outTaxDetails =
          solutions.taxDetailsSeq()[taxDetailsRef.id()];
      assert (outTaxDetails);
      ans += outTaxDetails->paymentAmt();
    }
  }

  return ans;
}

bool sameTaxStructure(const OutputTax& outTax1, const OutputTax& outTax2, const tax::OutputItins& solutions)
{
  size_t taxDetailSize = outTax1.taxDetailsRef().size();
  if (taxDetailSize != outTax2.taxDetailsRef().size())
    return false;

  const std::vector<std::shared_ptr<OutputTaxDetails>>& taxDetailsSeq = solutions.taxDetailsSeq();

  for (size_t i = 0; i != taxDetailSize; ++i)
  {
    type::Index id1 = outTax1.taxDetailsRef()[i].id();
    type::Index id2 = outTax2.taxDetailsRef()[i].id();

    if (id1 != id2)
    {
      const std::shared_ptr<OutputTaxDetails>& taxDetail1 = taxDetailsSeq[id1];
      const std::shared_ptr<OutputTaxDetails>& taxDetail2 = taxDetailsSeq[id2];
      assert (taxDetail1);
      assert (taxDetail2);
      if (taxDetail1->paymentAmt() != taxDetail2->paymentAmt())
        return false;
    }
  }

  return true;
}

bool sameTaxStructure(const tax::OutputTaxGroup& taxGroup1, const tax::OutputTaxGroup& taxGroup2,
                      const tax::OutputItins& solutions)
{
  // TODO: currently we do not consider permutations as the same structure. The old taxes do
  size_t taxSize = taxGroup1.taxSeq().size();
  if (taxSize != taxGroup2.taxSeq().size())
    return false;

  for (size_t i = 0; i != taxSize; ++i)
  {
    bool same = sameTaxStructure(taxGroup1.taxSeq()[i], taxGroup2.taxSeq()[i], solutions);
    if (!same)
      return false;
  }

  return true;
}

class AltFarePath
{
  FarePathRef farePathRef;

public:
  AltFarePath(FarePathRef fp) : farePathRef(fp) {}
  bool operator()(const tax::FarePathLink& link) const
  {
    return farePathRef.farePath == link.altPath && farePathRef.index == link.index;
  }
};

std::vector<tse::CarrierCode>
getCandidateCarriers(const std::vector<FarePathRef> farePaths, const FarePathMap& fpMap)
{
  std::vector<tse::CarrierCode> ans;
  for (const FarePathRef& fp : farePaths)
  {
    FarePathMap::const_iterator it = std::find_if(fpMap.begin(), fpMap.end(), AltFarePath(fp));
    assert (it != fpMap.end());
    ans.push_back(tse::toTseCarrierCode(it->validatingCarrier));
  }

  return ans;
}

std::string address(const void * addr)
{
  std::stringstream ans;
  ans << addr;
  std::string strans = ans.str();
  return strans.substr(strans.size() - 4);
}

void
fillValCarriers(std::vector<tse::CarrierCode>& bestValCarriers, tse::PricingTrx& trx,
                tse::FarePath* fp, std::ostream* diagnostic)
{
  if (diagnostic)
  {
    *diagnostic << "  FOR " << address(fp) << ":\n";
    *diagnostic << "    CANDIDATES:";
    for (tse::CarrierCode cc : bestValCarriers)
    {
      *diagnostic << " " << cc;
    }
    *diagnostic << "\n";
  }

  bool ok = false;
  tse::CarrierCode defVCxr, marketVCxr;
  if (!tse::fallback::fallbackValidatingCxrMultiSp(&trx) || trx.overrideFallbackValidationCXRMultiSP())
  {
    tse::DefaultValidatingCarrierFinder defValCxrFinder(trx, *(fp->itin()));
    ok = defValCxrFinder.determineDefaultValidatingCarrier(bestValCarriers, defVCxr, marketVCxr);
  }
  else
  {
    tse::ValidatingCarrierUpdater updater(trx);
    ok = updater.determineDefaultValidatingCarrier(*fp->itin(), bestValCarriers, defVCxr, marketVCxr);
  }

  if (ok)
  {
    if (diagnostic) *diagnostic << "    DEF / MKT: " << defVCxr << " / " << marketVCxr << "\n";
    fp->defaultValidatingCarrier() = (defVCxr);
    fp->itin()->validatingCarrier() = (defVCxr);
    if((defVCxr != marketVCxr) && !marketVCxr.empty())
      fp->marketingCxrForDefaultValCxr() = (marketVCxr);
  }
  else // no default found
  {
    if (diagnostic) *diagnostic << "    CANT DETERMINE VAL CXR\n";
    if (trx.getRequest()->isTicketEntry() && !bestValCarriers.empty())
    {
      std::string valCxr;
      for (const tse::CarrierCode& vcr : bestValCarriers)
      {
        valCxr = valCxr + vcr.c_str() + " ";
      }
      std::string message = "MULTIPLE VALIDATING CARRIER OPTIONS - " + valCxr;
      throw tse::ErrorResponseException(tse::ErrorResponseException::VALIDATING_CXR_ERROR, message.c_str());
    }
  }
}

std::string str(const tse::FarePath* tseFarePath, tax::type::Index id)
{
  std::ostringstream ans;
  ans << address(tseFarePath) << '/' << id;
  return ans.str();
}

std::string str(const tse::Itin* tseItin, const tse::FarePath* tseFarePath)
{
  std::ostringstream ans;
  ans << address(tseItin) << '/' << address(tseFarePath);
  return ans.str();
}

void
updateValidatingCarriers(const BestFarePath& fareBestPathMap, const FarePathMap& fpMap,
                         tse::PricingTrx& trx, std::ostream* diagnostic)
{
  if (diagnostic) *diagnostic << std::string(63, '*') << "\nSTAGE: ASSIGN VAL CXRS\n";

  for (const BestFarePath::value_type& fp : fareBestPathMap)
  {
    assert (!fp.second.farePaths.empty());
    std::vector<tse::CarrierCode> bestValCarriers = getCandidateCarriers(fp.second.farePaths, fpMap);
    fillValCarriers(bestValCarriers, trx, fp.first.mainFarePath, diagnostic);
  }

  if (diagnostic)
  {
    *diagnostic << "\n  FPATH MAP CXRS (ALT, MAIN, CXR):\n";
    for (const FarePathMap::value_type& rec : fpMap)
    {
      (*diagnostic) << "    " << str(rec.altPath, rec.index) << " " << address(rec.mainPath) << " "
                    << std::string(rec.validatingCarrier.asString()) << "\n";
    }
  }
}

std::string logSolution(const tse::FarePath* tseFarePath, tax::type::Index id, type::MoneyAmount amount)
{
  std::ostringstream ans;
  ans << str(tseFarePath, id) << " (" << amount << ")";
  return ans.str();
}

void recordFpathMap(const BestFarePath& fpathMap, std::ostream& diag)
{
  diag << " RESULT:\n";
  for (const BestFarePath::value_type& rec : fpathMap)
  {
    diag << "  FPATH VEC FOR " << str(rec.first.itin, rec.first.mainFarePath) << ":\n   ";
    for (const FarePathRef& fp : rec.second.farePaths)
      diag << " " << str(fp.farePath, fp.index);

    diag << "\n";
  }
}

void updateAlternateValidatingCarriers(tse::FarePath* tseFarePath,
                                       const std::vector<FarePathRef>& farePaths,
                                       const FarePathMap& farePathMap,
                                       std::ostream* diag)
{
  assert (farePaths.size() > 1);
  assert (farePaths.front().farePath == tseFarePath);
  assert (tseFarePath);

  tseFarePath->validatingCarriers().clear();

  if (diag) *diag << "  FOR " << address(tseFarePath) << ":";

  for (size_t i = 1; i < farePaths.size(); ++i )
  {
    const FarePathRef& fpRef = farePaths[i];

    for (const FarePathLink& fpLink : farePathMap)
    {
      if (fpLink.mainPath == tseFarePath && fpLink.altPath == fpRef.farePath)
      {
        tse::CarrierCode altValCxr = tse::toTseCarrierCode(fpLink.validatingCarrier); // or primary val cxr
        if (diag) *diag << " " << altValCxr;
        tseFarePath->validatingCarriers().push_back(altValCxr);
        break;
      }
    }
  }

  if (diag) *diag << "\n";
}

const std::string separator = std::string(AtpcoDiagnostic::LINE_LENGTH, '*') + std::string(1, '\n');

} // namespace

ItinFarePathMapping buildBestFarePathMap(const V2TrxMappingDetails& v2TrxMappingDetails,
                                         const tax::OutputItins& solutions,
                                         tse::PricingTrx& trx,
                                         std::ostream* diagnostic /* = 0 */)
{
  BestFarePath bestFor;

  if (diagnostic) *diagnostic << separator <<"STAGE: SCORE SOLUTIONS\n";

  for(const tax::ItinFarePathMapping::value_type& mapping : v2TrxMappingDetails._itinFarePathMapping)
  {
    tse::Itin* tseItin;
    tse::FarePath* tseFarePath;
    tax::type::CarrierCode validatingCarrier;
    tax::type::Index id;
    std::tie(tseItin, tseFarePath, validatingCarrier, id) = mapping;

    bool isNetRemitFp = (dynamic_cast<const tse::NetRemitFarePath*>(tseFarePath) != nullptr);
    bool isNetFp = (dynamic_cast<const tse::NetFarePath*>(tseFarePath) != nullptr);

    // exclude net and adjusted selling from being considered as alternatives for pricing
    if (isNetFp || isNetRemitFp || tseFarePath->isAdjustedSellingFarePath())
      continue;

    const OutputItin& solution = solutions.itinSeq()[id];
    std::shared_ptr<tax::OutputTaxGroup> taxGroup = getTaxGroup(solution, solutions);
    tse::TaxResponse* taxResponse =
        tax::detail::findTaxResponse(tseItin, tseFarePath, validatingCarrier.asString());

    type::MoneyAmount totalAmount = totalTax(taxGroup, solutions);
    tse::MoneyAmount totalYqYrAndPfc = 0;
    if (_LIKELY(taxResponse != nullptr))
    {
      for (auto taxItem : taxResponse->taxItemVector())
      {
        if (tse::FacadesUtils::isYqYr(taxItem->taxCode()))
          totalYqYrAndPfc += taxItem->taxAmount();
      }

      for (auto pfcItem : taxResponse->pfcItemVector())
      {
        totalYqYrAndPfc += pfcItem->pfcAmount();
      }
    }

    totalAmount += doubleToAmount(totalYqYrAndPfc + tseFarePath->getTotalNUCAmount());
    tse::FarePath* mainFarePath = findMainFarePath(tseFarePath, v2TrxMappingDetails._farePathMap);
    FarePathMapVal& val = bestFor[FarePathKey(tseItin, mainFarePath)];

    if (diagnostic) *diagnostic << "  SOLUTION: ";

    if (val.val == boost::none || totalAmount < *val.val)
    {
      val = FarePathMapVal(totalAmount, tseFarePath, id, taxGroup, &mapping);
      if (diagnostic) *diagnostic << "INITIAL " << logSolution(tseFarePath, id, totalAmount) << "\n";
    }
    else if (totalAmount == *val.val && sameTaxStructure(*val.referenceTaxGroup, *taxGroup, solutions))
    {
      val.addFarePath(tseFarePath, id);
      val.addMapping(&mapping, id);
      if (diagnostic) *diagnostic << "EQUIVALENT " << logSolution(tseFarePath, id, totalAmount) << "\n";
    }
    else
    {
      if (diagnostic) *diagnostic << "WORSE" << logSolution(tseFarePath, id, totalAmount) << "\n";
    }
  }

  if (diagnostic) recordFpathMap(bestFor, *diagnostic);
  if (diagnostic) *diagnostic << separator << "STAGE: ASSIGNING ALTERNATE VAL CXRS\n";

  ItinFarePathMapping newMap;
  std::set<std::pair<tse::Itin*, tse::FarePath*> > uniqueMainFP;
  for (auto best : bestFor)
  {
    for (auto mappingRef : best.second.mappings)
    {
      tse::FarePath* tseFarePath;
      tse::Itin* tseItin;
      tax::type::CarrierCode validatingCarrier;
      tax::type::Index id;
      std::tie(tseItin, tseFarePath, validatingCarrier, id) = *mappingRef.mapping;
      tse::FarePath* mainFarePath = findMainFarePath(tseFarePath, v2TrxMappingDetails._farePathMap);

      if (!uniqueMainFP.insert(std::make_pair(tseItin, mainFarePath)).second)
      {
        continue;
      }

      const std::vector<FarePathRef>& farePaths = bestFor[FarePathKey(tseItin, mainFarePath)].farePaths;
      assert (!farePaths.empty());

      if (farePaths.front().farePath == tseFarePath) // main fare path (not a VC alternative)
      {
        newMap.push_back(*mappingRef.mapping);
        if (farePaths.size() > 1) { // a potential alternate carrier
          updateAlternateValidatingCarriers(tseFarePath, farePaths, v2TrxMappingDetails._farePathMap, diagnostic);
        }
      }
    }
  }

  updateValidatingCarriers(bestFor, v2TrxMappingDetails._farePathMap, trx, diagnostic);
  changeItinFarePaths(v2TrxMappingDetails._itinFarePathMapping, bestFor);

  return newMap;
}

const tse::FarePath* GetMainFarePath::operator() (const tse::FarePath* input) const
{
  return findMainFarePath(input, _farePathMap);
}

} // namespace tax
