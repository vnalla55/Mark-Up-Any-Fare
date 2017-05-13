//----------------------------------------------------------------------------
//
//  Copyright Sabre 2016
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

#include "Diagnostic/BrandedDiagnosticUtil.h"

#include "BrandedFares/BrandInfo.h"
#include "BrandedFares/BrandProgram.h"
#include "Common/BrandingUtil.h"
#include "Common/FallbackUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/Vendor.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "DataModel/Trx.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag870Collector.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/RuleConst.h"

#include <boost/lexical_cast.hpp>

#include <sstream>
#include <string>

namespace tse
{

std::string
BrandedDiagnosticUtil::displayBrandedFaresFarePathValidation(PricingTrx* trx,
                                                             const FarePath& farePath)
{
  std::ostringstream result;
  result << "********************************************************:\n";
  for (PricingUnit* pricingUnit : farePath.pricingUnit())
  {
    for (FareUsage* fareUsage : pricingUnit->fareUsage())
    {
      PaxTypeFare* fare = fareUsage->paxTypeFare();
      result << fare->createFareBasis(trx, false) << " - ";

      TSE_ASSERT(fare->getBrandStatusVec().size() ==
                 fare->fareMarket()->brandProgramIndexVec().size());

      for (uint16_t i = 0; i < fare->fareMarket()->brandProgramIndexVec().size(); ++i)
      {
        int brandIndex = fare->fareMarket()->brandProgramIndexVec()[i];
        result << ShoppingUtil::getBrandCode(*trx, brandIndex);
        result << "(" << static_cast<char>(fare->getBrandStatusVec()[i].first) << ") ";
      }
      result << "\n";
    }
  }
  result << "\n";
  return result.str();
}

void
BrandedDiagnosticUtil::displayBrand(std::ostringstream& out, const QualifiedBrand& qb)
{
  BrandProgram* brandProgram = qb.first;
  BrandInfo* brand = qb.second;
  out << " PROGRAM ID=" << brandProgram->programID() << "\n"
      << " CODE=" << brand->brandCode() << "\n"
      << " NAME=" << brand->brandName() << "\n";
}

void
BrandedDiagnosticUtil::displayBrandProgram(std::ostringstream& out, const QualifiedBrand& qb,
                                           bool showOnd)
{
  BrandProgram* brandProgram = qb.first;
  out << "PROGRAM ID " << brandProgram->programID() << ":\n"
      << " CODE=" << brandProgram->programCode() << "\n"
      << " SYSTEM=" << brandProgram->systemCode() << "\n"
      << " NAME=" << brandProgram->programName() << "\n";
  if (showOnd)
  {
    std::string originsRequested = DiagnosticUtil::containerToString(brandProgram->originsRequested(),
        " ", DiagnosticUtil::MAX_WIDTH, 25, true);
    std::string destinationsRequested = DiagnosticUtil::containerToString(
        brandProgram->destinationsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 25, true);
    out << " ORIGINS REQUESTED     : " << originsRequested << "\n"
        << " DESTINATIONS REQUESTED: " << destinationsRequested << "\n";
  }
}

void
BrandedDiagnosticUtil::displayBrandIndices(std::ostringstream& out,
                                           const std::vector<QualifiedBrand>& qbs,
                                           const std::vector<int>& indexVector,
                                           bool showOnd)
{
  std::set<ProgramID> programs;
  std::vector<uint16_t> programsIndices;
  for (int index: indexVector)
  {
    const QualifiedBrand& qb = qbs[index];
    if (programs.find(qb.first->programID()) == programs.end())
    {
      programs.insert(qb.first->programID());
      programsIndices.push_back(index);
    }
  }

  for (uint16_t index: programsIndices)
  {
    const QualifiedBrand& qb = qbs[index];
    displayBrandProgram(out, qb, showOnd);
  }
  out << "\n";
  for (uint16_t index = 0; index < qbs.size(); ++index)
  {
    const QualifiedBrand& qb = qbs[index];
    out << "BRAND INDEX " << index << ":\n";
    displayBrand(out, qb);
  }
}

void
BrandedDiagnosticUtil::displayAllBrandIndices(std::ostringstream& out,
                                              const std::vector<QualifiedBrand>& qbs,
                                              bool showOnd)
{
  std::vector<int> indexVector;
  indexVector.reserve(qbs.size());
  for (size_t i = 0; i < qbs.size(); ++i)
  {
    indexVector.push_back(i);
  }
  out << "***** START BRANDS INFORMATION *****\n";
  out << "RECEIVED PROGRAMS AND BRANDS:\n";
  displayBrandIndices(out, qbs, indexVector, showOnd);
  out << "****** END BRANDS INFORMATION ******\n";
}

void
BrandedDiagnosticUtil::displayFareMarketErrorMessagePerBrand(std::ostringstream& out,
                                                             const PricingTrx& trx,
                                                             const FareMarket& fm)
{
  displayErrorMessagesLegend(out);
  std::set<BrandCode> uniqueBrands;

  // Get unique brands
  for (int brandIndex : fm.brandProgramIndexVec())
  {
    uniqueBrands.insert(ShoppingUtil::getBrandCode(trx, brandIndex));
  }
  // Display brand infoa
  std::ostringstream brandStatus;

  const bool useDirectionality = BrandingUtil::isDirectionalityToBeUsed(trx);

  for (const BrandCode& brandCode : uniqueBrands)
  {
    std::ostringstream temp;
    if (trx.isBRAll() && useDirectionality)
    {
      IbfErrorMessage ibfErrorMessage = fm.getStatusForBrand(brandCode, Direction::ORIGINAL);
      temp << " " << brandCode << ":" << ibfErrorMessage << "(O),";
      ibfErrorMessage = fm.getStatusForBrand(brandCode, Direction::REVERSED);
      temp << ibfErrorMessage << "(R) |";
    }
    else
    {
      //IBF or BFS or no directionality
      const IbfErrorMessage ibfErrorMessage = fm.getStatusForBrand(brandCode, Direction::BOTHWAYS);
      temp << " " << brandCode << ":" << ibfErrorMessage << " |";
    }

    if (brandStatus.str().length() + temp.str().length() > DiagnosticUtil::MAX_WIDTH) // Avoid creating too long lines
    {
      out << brandStatus.str() << std::endl;
      brandStatus.str("");
    }

    brandStatus << temp.str();
  }
  out << brandStatus.str() << std::endl;
}

void
BrandedDiagnosticUtil::displayErrorMessagesLegend(std::ostringstream& out)
{
  out << "IBF ERROR MESSAGES PER BRAND: \n";
  out << " N : NOT SET\n";
  out << " X : NO FARE FILED\n";
  out << " E : EARLY DROP - FAILED BEFORE BK VALIDATION\n";
  out << " A : NOT AVAILABLE\n";
  out << " O : NOT OFFERED\n";
  out << " F : PASSED BK VALIDATION\n";
  out << "     IF ITIN IS NOT PRICED - NO COMBINABLE FARES\n\n";
}

void
BrandedDiagnosticUtil::displayBrandRetrievalMode(std::ostringstream& out, BrandRetrievalMode mode)
{
  out << "\nBRAND RETRIEVAL MODE : ";
  switch (mode)
  {
  case BrandRetrievalMode::PER_O_AND_D:
    out << "PER OND";
    break;
  case BrandRetrievalMode::PER_FARE_COMPONENT:
    out << "PER FARE COMPONENT";
    break;
  }
  out << "\n";
}

void
BrandedDiagnosticUtil::displayTripType(std::ostringstream& out, IAIbfUtils::TripType tripType)
{
  out << " \nREQUESTS TO BRANDS BUILT USING TRIP TYPE : " << tripType << "\n";
}

void
BrandedDiagnosticUtil::displayFareMarketsWithBrands(std::ostringstream& out,
                                                    const std::vector<FareMarket*>& fareMarkets,
                                                    const std::vector<QualifiedBrand>& qbs,
                                                    const IAIbfUtils::OdcsForBranding* odcs,
                                                    bool showOnd)
{
  out << "***** START FARE MARKET BRANDS *****\n";
  int index = 1;
  for (FareMarket* fm : fareMarkets)
  {
    displayFareMarket(out, index, fm, odcs, showOnd);
    displayFareMarketBrandIndicies(out, *fm, qbs);
    ++index;
  }
  out << "****** END FARE MARKET BRANDS ******\n";
}

void
BrandedDiagnosticUtil::displayFareMarketBrandIndicies(std::ostringstream& out, const FareMarket& fm,
                                                      const std::vector<QualifiedBrand>& qbs)
{
  if (fm.brandProgramIndexVec().empty())
  {
    out << " NO BRANDS FOUND\n";
    return;
  }
  out << " BRAND INDICES: "; //len = 16
  std::string indicesString =
    DiagnosticUtil::containerToString(fm.brandProgramIndexVec(), ",", DiagnosticUtil::MAX_WIDTH, 16, true);
  out << indicesString << "\n";

  std::map<ProgramID, std::vector<BrandCode> > programBrands;
  std::vector<ProgramID> programs;
  for (int index: fm.brandProgramIndexVec())
  {
    const ProgramID& pid = qbs[index].first->programID();
    const BrandCode& bc = qbs[index].second->brandCode();
    programBrands[pid].push_back(bc);
    if (std::find(programs.begin(), programs.end(), pid) == programs.end())
      programs.push_back(pid);
  }
  bool firstProgram = true;
  std::ostringstream ss;
  for (const ProgramID& pid: programs)
  {
    const std::vector<BrandCode>& brands = programBrands[pid];
    ss.str("");
    ss.clear();
    ss << (firstProgram ? " BRANDS: " : "         ") << pid << ": ";
    out << ss.str()
      << DiagnosticUtil::containerToString(
          brands, ",", DiagnosticUtil::MAX_WIDTH, ss.str().length(), true)
      << "\n";
    firstProgram = false;
  }
}

void BrandedDiagnosticUtil::displayFareMarket(std::ostringstream& out, int index, FareMarket* fm,
                                              const IAIbfUtils::OdcsForBranding* odcs, bool showOnd)
{
  std::string origin;
  std::string destination;
  out << "FARE MARKET " << index << ": "
      << fm->origin()->loc() << "[" << fm->boardMultiCity() << "]"
      << "-" << fm->governingCarrier() << "-"
      << fm->destination()->loc() << "[" << fm->offMultiCity() << "]" << "\n";
  if (odcs)
  {
    auto it = odcs->find(fm);
    if (it != odcs->end())
    {
      out << " BRANDS RETRIEVED USING ODCs:";
      const std::set<IAIbfUtils::OdcTuple>& odcTuples = it->second;
      std::string odcsString =
        DiagnosticUtil::containerToString(odcTuples, " ", DiagnosticUtil::MAX_WIDTH, 29, false, true);
      out << odcsString << "\n";
    }
    else
    {
      out << "! N/A !\n";
    }
  }
  if (showOnd)
  {
    out << " ORIGINS REQUESTED     : " <<
      DiagnosticUtil::containerToString(
          fm->getOriginsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 25, true) << "\n";
    out << " DESTINATIONS REQUESTED: " <<
      DiagnosticUtil::containerToString(
          fm->getDestinationsRequested(), " ", DiagnosticUtil::MAX_WIDTH, 25, true) << "\n";
  }
}

} // end namespace tse
