#include "Pricing/Shopping/PQ/SoloFamilyGrouping.h"

#include "Common/FallbackUtil.h"
#include "Common/ShoppingUtil.h"
#include "DataModel/ShoppingTrx.h"
#include "Diagnostic/DCFactory.h"

namespace tse
{
FALLBACK_DECL(fallbackFareSelction2016);

namespace shpq
{

namespace
{

struct SimilarSOPFinder
{
  SimilarSOPFinder(ShoppingTrx& trx, shpq::SopIdxVec& searchedSOPs, bool dualFMFlag)
    : _trx(trx), _searchedSOPs(searchedSOPs), _dualFMFlag(dualFMFlag)
  {
  }

  bool operator()(shpq::SopIdxVec& sopVec) const
  {
    if (UNLIKELY(sopVec.size() != _searchedSOPs.size()))
      return false;

    for (size_t leg = 0; leg < _searchedSOPs.size(); ++leg)
    {
      const ShoppingTrx::SchedulingOption& sop1 = _trx.legs()[leg].sop()[sopVec[leg]];
      const ShoppingTrx::SchedulingOption& sop2 = _trx.legs()[leg].sop()[_searchedSOPs[leg]];
      bool  atLeastOneHTPM = (sop1.isHighTPM() || sop2.isHighTPM());

      if (_dualFMFlag ||
          (!fallback::fallbackFareSelction2016(&_trx) && atLeastOneHTPM))
      {
        if (!sopSimilarForDualFM(sop1, sop2))
        {
          return false;
        }
      }
      else if (!ShoppingUtil::schedulingOptionsSimilar(_trx, sop1, sop2))
      {
          return false;
      }
    }
    return true;
  }

private:
  bool sopSimilarForDualFM(const ShoppingTrx::SchedulingOption& sop1,
                           const ShoppingTrx::SchedulingOption& sop2) const
  {
    if (sop1.governingCarrier() != sop2.governingCarrier())
      return false;

    const std::vector<TravelSeg*>& tvlSegVec1 = sop1.itin()->travelSeg();
    const std::vector<TravelSeg*>& tvlSegVec2 = sop2.itin()->travelSeg();

    if (tvlSegVec1.size() != tvlSegVec2.size())
      return false;

    for (size_t index = 0; index < tvlSegVec1.size(); ++index)
    {
      if (!travelSegSimilarForDualFM(tvlSegVec1[index], tvlSegVec2[index]))
        return false;
    }

    return true;
  }

  bool travelSegSimilarForDualFM(const TravelSeg* tvlSeg1, const TravelSeg* tvlSeg2) const
  {
    const AirSeg* air1 = dynamic_cast<const AirSeg*>(tvlSeg1);
    const AirSeg* air2 = dynamic_cast<const AirSeg*>(tvlSeg2);

    if ((air1 == nullptr) || (air2 == nullptr))
      return false;

    return air1->origAirport() == air2->origAirport() &&
           air1->destAirport() == air2->destAirport() &&
           air1->marketingCarrierCode() == air2->marketingCarrierCode() &&
           air1->departureDT().date() == air2->departureDT().date();
  }

private:
  ShoppingTrx& _trx;
  shpq::SopIdxVec& _searchedSOPs;
  bool _dualFMFlag;
};
}

void
SoloFamilyGrouping::process(SoloItinGenerator::SolutionVector& solutionsVector)
{
  if (_groupingOption == DISABLE_GROUPING)
    return;

  if (_diagPtr)
    (*_diagPtr) << " SOPs: \n";

  if (!fallback::fallbackFareSelction2016(&_trx) &&
      !isDualFMFlag() &&
      !compareSops())
  {
    processGroupSolFarePathsOnly(solutionsVector);
    return;
  }

  SoloItinGenerator::VectorOfSopVec sopsAddedToFlightMatrix;
  SoloItinGenerator::VectorOfSopVec::iterator similarSop = sopsAddedToFlightMatrix.end();

  for (auto& sol : solutionsVector)
  {
    shpq::SopIdxVec& sidVec=sol.first;
    if (_diagPtr)
      (*_diagPtr) << " SOP";
    printSopIdVec(sidVec);

    if (isIndustryFareUsed())
    {
      similarSop = sopsAddedToFlightMatrix.end();
    }
    else if (isDualFMFlag() || compareSops())
    {
      similarSop = std::find_if(sopsAddedToFlightMatrix.begin(),
                                sopsAddedToFlightMatrix.end(),
                                SimilarSOPFinder(_trx, sol.first, isDualFMFlag()));
    }

    decideSopRelationship(sopsAddedToFlightMatrix, similarSop, sol);
  }

  printMothersAndChildern();
  if (_diagPtr!=nullptr)
    _diagPtr->flushMsg();
}

void
SoloFamilyGrouping::processGroupSolFarePathsOnly(SoloItinGenerator::SolutionVector& solutionsVector)
{
  SoloItinGenerator::SolutionVector solVecWithHighTPM;
  SoloItinGenerator::SolutionVector solVecWithoutHighTPM;

  for (auto& sol : solutionsVector)
  {
    shpq::SopIdxVec& sopVec = sol.first;
    bool hasHighTPM = false;

    for (size_t leg = 0; leg < sopVec.size(); ++leg)
    {
      const ShoppingTrx::SchedulingOption& sop = _trx.legs()[leg].sop()[sopVec[leg]];
      if (sop.isHighTPM())
      {
        hasHighTPM = true;
        break;
      }
    }
    if (hasHighTPM)
      solVecWithHighTPM.push_back(sol);
    else
      solVecWithoutHighTPM.push_back(sol);
  }

  SoloItinGenerator::VectorOfSopVec sopsAddedToFlightMatrix;
  SoloItinGenerator::VectorOfSopVec::iterator similarSop = sopsAddedToFlightMatrix.end();

  // process the ones without HighTPM first. The
  // first SOP will be mother and the rest will
  //  be set as children of the first SOP
  if (solVecWithoutHighTPM.size()>0)
  {
    if (_diagPtr)
      (*_diagPtr) <<" The following SOPs DONT have highTPM set\n";
  }

  for (auto& sol : solVecWithoutHighTPM)
  {
    shpq::SopIdxVec& sidVec=sol.first;
    if (_diagPtr)
      (*_diagPtr) << " SOP";
    printSopIdVec(sidVec);

    if (isIndustryFareUsed())
    {
      similarSop = sopsAddedToFlightMatrix.end();
    }

    decideSopRelationship(sopsAddedToFlightMatrix, similarSop, sol);
  }
  if (_diagPtr)
   (*_diagPtr) <<"\n";

  // process the ones with HighTPM and decide if
  // each SOP should be mom or child
  sopsAddedToFlightMatrix.clear();
  similarSop = sopsAddedToFlightMatrix.end();
  if (solVecWithHighTPM.size()>0)
  {
    if (_diagPtr)
      (*_diagPtr) <<" The following SOPs have highTPM set\n";
  }

  for (auto& sol : solVecWithHighTPM)
  {
    shpq::SopIdxVec& sidVec=sol.first;
    if (_diagPtr)
      (*_diagPtr) << " SOP";
    printSopIdVec(sidVec);

    if (isIndustryFareUsed())
    {
      similarSop = sopsAddedToFlightMatrix.end();
    }
    else
    {
      similarSop = std::find_if(sopsAddedToFlightMatrix.begin(),
                                sopsAddedToFlightMatrix.end(),
                                SimilarSOPFinder(_trx, sol.first, isDualFMFlag()));
    }

    decideSopRelationship(sopsAddedToFlightMatrix, similarSop, sol);
  }

  if (_diagPtr)
    (*_diagPtr) <<"\n";
  printMothersAndChildern();
  if (_diagPtr!=nullptr)
    _diagPtr->flushMsg();
}

void
SoloFamilyGrouping::setFarePathInfo(const FarePath* const farePath)
{
  for (const FareMarket* const fm : farePath->itin()->fareMarket())
  {
    if (fm->getFmTypeSol() == FareMarket::SOL_FM_LOCAL)
      _farePathInfo._thruFarePath = false;
    if (fm->isDualGoverning())
      _farePathInfo._dualFMFlag = true;
  }

  _farePathInfo._industryFare = ShoppingUtil::isIndustryFareUsed(*farePath);
  printHeader(farePath);
}

void
SoloFamilyGrouping::printHeader(const FarePath* const farePath)
{
  if (UNLIKELY(_trx.diagnostic().diagnosticType() == Diagnostic942 &&
               _trx.diagnostic().diagParamMapItem("DD") == "GROUPING"))
  {
    DCFactory* factory = DCFactory::instance();
    _diagPtr = static_cast<Diag942Collector*>(factory->create(_trx));

    if (_diagPtr==nullptr)
      return;

    _diagPtr->enable(Diagnostic942);
    (*_diagPtr) <<"\n Process SoloFamily Grouping:\n";
    (*_diagPtr) <<" ============================\n";

    for (const FareMarket* const fm : farePath->itin()->fareMarket())
    {
      (*_diagPtr) <<" FM:["<<fm->origin()->loc() <<"-"
            <<fm->governingCarrier()<<"-"
            <<fm->destination()->loc()<<", dualFM="<<fm->isDualGoverning()<<"]. TvlSeg=";

      for (TravelSeg* ts : fm->travelSeg())
      {
        if (ts->isAir())
        { AirSeg* as=ts->toAirSeg();
          if (as)
          {
            (*_diagPtr)<<"["<<as->origin()->loc()<<"-"
                 <<as->carrier()<<"-"
                 <<as->destination()->loc()<<"]";
          }
        }
      }
      (*_diagPtr)<<"\n";
    }

    (*_diagPtr) <<"\n DUAL_FM_FLAG=" << isDualFMFlag()
          <<"\n COMPARE SOPS="<<compareSops()
          <<"\n GroupingOption=";
    switch (_groupingOption)
    {
      case GROUP_ALL_SOLUTIONS:
        (*_diagPtr) << "GROUP_ALL_SOLUTIONS\n ";
        break;
      case GROUP_SOL_FAREPATHS_ONLY:
        (*_diagPtr) << "GROUP_SOL_FAREPATHS_ONLY\n ";
        break;
      default:
        break;
    }
    _diagPtr->flushMsg();
  }
}

void
SoloFamilyGrouping::printSopIdVec(const SopIdVec& sopIdVec)
{
  if (_diagPtr==nullptr)
    return;

  (*_diagPtr) <<"[";
  for (size_t leg = 0; leg < sopIdVec.size(); ++leg)
  {
    const ShoppingTrx::SchedulingOption& sop = _trx.legs()[0].sop()[sopIdVec[leg]];
    uint32_t id = sop.originalSopId();
    (*_diagPtr) << id;
    if (leg<sopIdVec.size()-1)
      (*_diagPtr) << "-";
  }
  (*_diagPtr) <<"]";
}

void
SoloFamilyGrouping::printMothersAndChildern()
{
  if (_diagPtr==nullptr)
    return;

  (*_diagPtr) <<"\n There are "<<_trx.flightMatrix().size()<<" mothers\n ";
  std::map<SopIdVec, GroupFarePath*>::iterator itr=_trx.flightMatrix().begin();
  for (; itr!= _trx.flightMatrix().end(); itr++)
  {
    const SopIdVec& siVec = itr->first;
    printSopIdVec(siVec);
  }
  (*_diagPtr) << "\n";

  (*_diagPtr) <<" There are "<<_trx.estimateMatrix().size()<<" children.\n ";
  std::map<SopIdVec, ShoppingTrx::EstimatedSolution>::iterator itr1=_trx.estimateMatrix().begin();
  int i=0;
  for (; itr1!= _trx.estimateMatrix().end(); itr1++)
  {
    // child sop info
    const SopIdVec& childVec = (itr1->first);
    printSopIdVec(childVec);
    // mom sop info
    (*_diagPtr)<<"M";
    const SopIdVec& momVec = (itr1->second).first;
    printSopIdVec(momVec);
    if (++i%6==0)
      (*_diagPtr) <<"\n ";
    else
      (*_diagPtr) << " ";
  }
  (*_diagPtr) << "\n";
}

void
SoloFamilyGrouping::decideSopRelationship(SoloItinGenerator::VectorOfSopVec& sopsAddedToFlightMatrix,
                                          SoloItinGenerator::VectorOfSopVec::iterator& similarSop,
                                          const SoloItinGenerator::SolutionItem& sol)
// This method decides if the SOP is a mother or a child,
// and add to the mother (trx.FlightMatrix) or the child
// (trx.estimateMatrix) accordlingly
{
  if (similarSop == sopsAddedToFlightMatrix.end())
  {
    if (_diagPtr)
      (*_diagPtr) << "=mother";
    _trx.flightMatrix().insert(sol);
    similarSop = sopsAddedToFlightMatrix.insert(sopsAddedToFlightMatrix.end(), sol.first);
  }
  else
  {
    ShoppingTrx::EstimatedSolution estimate(*similarSop, sol.second);
    _trx.estimateMatrix()[sol.first] = estimate;
    if (_diagPtr)
      (*_diagPtr) <<"=child Mother=";
    shpq::SopIdxVec& similarVec=*similarSop;
    printSopIdVec(similarVec);
   }

  if (_diagPtr)
    (*_diagPtr) << "\n";
}

SoloFamilyGrouping::GroupingOption
SoloFamilyGrouping::getGroupingOption(uint32_t configValue)
{
  switch (configValue)
  {
  case 0:
    return DISABLE_GROUPING;
  case 1:
    return GROUP_ALL_SOLUTIONS;
  case 2:
    return GROUP_SOL_FAREPATHS_ONLY;
  default:
    return GROUP_ALL_SOLUTIONS;
  }
}
}
} // namespace tse::shpq
