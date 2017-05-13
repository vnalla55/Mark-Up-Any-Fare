// vim:ts=2:sts=2:sw=2:cin:et
// ----------------------------------------------------------------
//
//   Copyright Sabre 2011
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

#include "Common/TseObjectPool.h"
#include "DataModel/DirFMPathList.h"
#include "Pricing/Shopping/PQ/BaseLegInfo.h"
#include "Pricing/Shopping/PQ/CommonSoloPQItem.h"

#include <memory>

namespace tse
{
// Fwd declarations
class Logger;

} // namespace tse

namespace tse
{
namespace shpq
{

class SolutionPatternPQItem;
typedef std::shared_ptr<SolutionPatternPQItem> SolutionPatternPQItemPtr;

class SolutionPatternPQItem : public CommonSoloPQItem
{
  friend class boost::object_pool<SolutionPatternPQItem>;
  friend class TseObjectPool<SolutionPatternPQItem>;

public:
  virtual MoneyAmount getScore() const override { return _score; }

  virtual SoloPQItemLevel getLevel() const override { return SP_LEVEL; }

  virtual void expand(SoloTrxData& soloTrxData, SoloPQ& pq) override;

  virtual std::string str(const StrVerbosityLevel strVerbosity = SVL_BARE) const override;

private:
  typedef details::BaseLegInfo<DirFMPathListPtr, DirFMPathList::const_iterator> LegInfo;

  SolutionPatternPQItem& operator=(const SolutionPatternPQItem&);

  MoneyAmount calculateScore(const MoneyAmount defaultAmount) const;

  const LegInfo _outboundLeg;
  const LegInfo _inboundLeg;
  const MoneyAmount _score;
  static Logger _logger;

private:
  // TODO: Add arguments to factory method with solution pattern descriptor and actual data to work
  // on
  // Use SoloPQItemManager instead
  SolutionPatternPQItem(const SolutionPattern& solutionPattern,
                        const DirFMPathListPtr& outboundDFm,
                        const DirFMPathListPtr& inboundDFm);

  SolutionPatternPQItem(const SolutionPatternPQItem& other, const LegPosition expandedLegPos);

}; // class SolutionPatterPQItem
}
} // namespace tseshpq

