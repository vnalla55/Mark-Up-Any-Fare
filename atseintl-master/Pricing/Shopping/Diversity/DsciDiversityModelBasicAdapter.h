// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
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

#include "Pricing/Shopping/Diversity/DesiredSopCombinationInterpreter.h"

namespace tse
{
class DmcRequirementsFacade;

class DsciDiversityModelBasicAdapter : public DesiredSopCombinationInterpreter<shpq::SOPCombination>
{
public:
  friend class DsciDiversityModelBasicAdapterTest;
  // getCombinationCouldSatisfy result
  typedef int Requirements;

  DsciDiversityModelBasicAdapter(const ShoppingTrx& trx,
                                 const ItinStatistic& stats,
                                 DmcRequirementsFacade* requirements,
                                 MoneyAmount score,
                                 Requirements firstRequirements)
    : DesiredSopCombinationInterpreter<shpq::SOPCombination>(trx, stats),
      _bestRequirements(firstRequirements),
      _lastObtainedCouldSatisfy(-1)
  {
    addRequirementsAndStatusClause(requirements, score, _lastObtainedCouldSatisfy);
    addPairingClause();
    addNonStopClause();
    addTODClause();
  }

  int getLastObtainedCouldSatisfy() { return _lastObtainedCouldSatisfy; }

private:
  Requirements _bestRequirements;
  Requirements _lastObtainedCouldSatisfy;

  int isBetterByRequirementsAndStatus(DmcRequirementsFacade* requirements,
                                      double score,
                                      const Operand& lhs,
                                      const Operand& rhs);

  int isBetterByPairing(const Operand& lhs, const Operand& rhs);
  int isBetterByNonStop(const Operand& lhs, const Operand& rhs);
  int isBetterByTOD(const Operand& lhs, const Operand& rhs);

  void addRequirementsAndStatusClause(DmcRequirementsFacade* requirements,
                                      double score,
                                      int& signalRequirementsTo)
  {
    Clause clause = std::tr1::bind(&DsciDiversityModelBasicAdapter::isBetterByRequirementsAndStatus,
                                   this,
                                   requirements,
                                   score,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  void addPairingClause()
  {
    Clause clause = std::tr1::bind(&DsciDiversityModelBasicAdapter::isBetterByPairing,
                                   this,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  void addNonStopClause()
  {
    Clause clause = std::tr1::bind(&DsciDiversityModelBasicAdapter::isBetterByNonStop,
                                   this,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }

  void addTODClause()
  {
    Clause clause = std::tr1::bind(&DsciDiversityModelBasicAdapter::isBetterByTOD,
                                   this,
                                   std::tr1::placeholders::_1,
                                   std::tr1::placeholders::_2);
    _clauseList.push_back(clause);
  }
};

} // tse
