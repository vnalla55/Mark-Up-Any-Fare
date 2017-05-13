// ----------------------------------------------------------------
//
//   Copyright Sabre 2012
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------

#pragma once

#include "Common/DateTime.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/ShoppingTrx.h"

#include <list>
#include <map>
#include <vector>

namespace tse
{
class TravelSeg;

namespace fos
{

struct ShapeSolution
{
  enum Type
  {
    VALID = 0,
    VALID_SNOWMAN,
    VALID_DIAMOND,
    VALID_TRIANGLE,
    INVALID,
    NUM_ELEMENTS
  };
};

typedef std::set<CarrierCode> CarrierSet;

struct SOPWrapper
{
  SOPWrapper(uint32_t sopId) : _sopId(sopId) {}
  int queueRank() const { return _sopId; }
  int _sopId;
};

typedef std::vector<SOPWrapper> Sops;
typedef std::vector<Sops> SopsByLeg;
typedef std::map<CarrierCode, SopsByLeg> SopsByLegByCarrier;

struct SOPDetails
{
  CarrierCode _cxr[2];
  std::vector<TravelSeg*> _tvlSegPortions[2];
  std::string str() const;
};

typedef std::vector<SOPDetails> SOPDetailsVec;
typedef std::map<SOPWrapper, SOPDetailsVec> SOPMap;
typedef std::vector<SOPMap> SopsByLegDetails;
// alt dates
typedef std::map<DateTime, Sops> SopsByDate;
typedef std::vector<SopsByDate> SopsByLegByDate;
typedef std::map<CarrierCode, SopsByLegByDate> SopsByLegByCxrByDate;

class SOPCollections
{
  SopsByLegDetails _interlineSOPsDetails;

  // alt dates
  SopsByLegByDate _sopsByDate;
  SopsByLegByCxrByDate _sopsByCxrByDate;

public:
  SopsByLegDetails& getSopsByLegDetails() { return _interlineSOPsDetails; }

  SopsByLegByDate& getSopsByDate() { return _sopsByDate; }
  SopsByLegByCxrByDate& getSopsByLegByCxrByDate() { return _sopsByCxrByDate; }
  std::pair<Sops, Sops> getSopsByDatePair(const DatePair& datePair);

  void clear()
  {
    _interlineSOPsDetails.clear();
    _sopsByDate.clear();
    _sopsByCxrByDate.clear();
  }
};

typedef std::vector<int> SopsCombination;
typedef std::list<SopsCombination> SolutionsContainer;

class GroupedSolutions
{
public:
  void addFOS(const SolutionsContainer::value_type& item);
  uint32_t solutionsFound() const { return _solutions.size(); }

  SolutionsContainer& getSolutions() { return _solutions; }
  SolutionsContainer& getOnlineSolutions() { return _onlineSolutions; }
  SolutionsContainer& getInterlineSolutions() { return _interlineSolutions; }

private:
  SolutionsContainer _solutions;
  SolutionsContainer _onlineSolutions;
  SolutionsContainer _interlineSolutions;
};

typedef std::map<CarrierCode, GroupedSolutions> SolutionsByCarrier;
typedef std::map<DatePair, SolutionsByCarrier> SolutionsByCarrierByDatePair;

bool operator<(const SOPWrapper& lhs, const SOPWrapper& rhs);

} // namespace fos
} // namespace tse

