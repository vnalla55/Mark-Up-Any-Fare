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

#include "Pricing/Shopping/PQ/SolutionPatternPQItem.h"

#include "Common/ErrorResponseException.h"
#include "Common/Logger.h"
#include "DataModel/DirFMPath.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/Shopping/PQ/BaseLegInfo.h"
#include "Pricing/Shopping/PQ/ConxRoutePQItem.h"
#include "Pricing/Shopping/PQ/SoloPQ.h"
#include "Pricing/Shopping/PQ/SoloTrxData.h"
#include "Pricing/Shopping/PQ/SolutionPattern.h"

#include <limits>

namespace /* anon */
{

struct NullDeleter
{
  void operator()(void*) {}
}; // struct NullDeleter

#define ALL_ITEMS                                                                                  \
  {                                                                                                \
    &_outboundLeg, &_inboundLeg                                                                    \
  }

} // namespace /*anon*/

namespace tse
{
namespace shpq
{

Logger
SolutionPatternPQItem::_logger("atseintl.ShoppingPQ.SolutionPatternPQItem");

SolutionPatternPQItem::SolutionPatternPQItem(const SolutionPattern& solutionPattern,
                                             const DirFMPathListPtr& outboundDFm,
                                             const DirFMPathListPtr& inboundDFm)
  : CommonSoloPQItem(solutionPattern),
    _outboundLeg(outboundDFm, outboundDFm->begin()),
    _inboundLeg(inboundDFm, (inboundDFm ? inboundDFm->begin() : outboundDFm->end())) // avoid
                                                                                     // unassigned
                                                                                     // iterator
    ,
    _score(calculateScore(UNKNOWN_MONEY_AMOUNT))
{
  VALIDATE_OR_THROW(solutionPattern.hasOutboundSol() && _outboundLeg.isValid(),
                    UNKNOWN_EXCEPTION,
                    "Internal error: invalid OB");
  VALIDATE_OR_THROW(!solutionPattern.hasInboundSol() || _inboundLeg.isValid(),
                    UNKNOWN_EXCEPTION,
                    "Internal error: invalid IB");
}

SolutionPatternPQItem::SolutionPatternPQItem(const SolutionPatternPQItem& other,
                                             const LegPosition expandedLegPos)
  : CommonSoloPQItem(other, expandedLegPos),
    _outboundLeg(other._outboundLeg, expandedLegPos == LEG_1ST),
    _inboundLeg(other._inboundLeg, expandedLegPos == LEG_2ND),
    _score(calculateScore(other._score))

{
  VALIDATE_OR_THROW(_outboundLeg.isValid() && _outboundLeg.hasCurrent(),
                    UNKNOWN_EXCEPTION,
                    "Internal error: invalid OB");
  VALIDATE_OR_THROW(!_inboundLeg.isValid() || _inboundLeg.hasCurrent(),
                    UNKNOWN_EXCEPTION,
                    "Internal error: invalid IB");
}

MoneyAmount
SolutionPatternPQItem::calculateScore(const MoneyAmount defaultAmount) const
{
  MoneyAmount amount;
  if (_outboundLeg.hasCurrent())
  {
    amount = _outboundLeg->lowerBound();

    if (_inboundLeg.hasCurrent())
      amount += _inboundLeg->lowerBound();
    return amount;
  }
  return defaultAmount;
}

// virtual
void
SolutionPatternPQItem::expand(SoloTrxData& soloTrxData, SoloPQ& pq)
{

  const LegInfo* const items[] = ALL_ITEMS;

  LegPosition expandedLegPos(LEG_1ST);
  size_t numberOfExpandedItems(0);

  SoloPQItemManager& pqItemMgr = soloTrxData.getPQItemManager();
  for (const LegInfo* const itemPtr : items)
  {
    const LegInfo& item(*itemPtr);
    if (isPlaceholderLeg(expandedLegPos) && item.hasNext())
    {
      const SolutionPatternPQItemPtr pqItem(pqItemMgr.constructSPPQItem(*this, expandedLegPos));
      pq.enqueue(pqItem, this);
      ++numberOfExpandedItems;
    }

    expandedLegPos = CommonSoloPQItem::getNextLegPosition(expandedLegPos);
  }

  // next level expansion
  {
    const ConxRoutePQItemPtr cnxRtPqem(pqItemMgr.constructCRPQItem(
        _solutionPattern, *_outboundLeg, _inboundLeg.hasCurrent() ? *_inboundLeg : DirFMPathPtr()));

    pq.enqueue(cnxRtPqem, this);
    ++numberOfExpandedItems;
  }

  LOG4CXX_TRACE(_logger,
                "SolutionPatternPQItem::expand(): " << this << " expanded into "
                                                    << numberOfExpandedItems << " new items");
}

// virtual
std::string
SolutionPatternPQItem::str(const StrVerbosityLevel strVerbosity /* = SVL_BARE */) const
{
  const char* const FIELD_SEPARTOR(" : ");

  std::ostringstream stream;
  printBasicStr(stream, "SP ");

  if (strVerbosity >= SVL_BARE)
  {
    for (int doInbound = 0; doInbound < 2; ++doInbound)
    {
      stream << FIELD_SEPARTOR;
      const LegPosition currentLegPos(doInbound ? LEG_2ND : LEG_1ST);
      const LegInfo& leg(doInbound ? _inboundLeg : _outboundLeg);
      const char* const& spType(doInbound ? _solutionPattern.getInboundSolCStr()
                                          : _solutionPattern.getOutboundSolCStr());

      if (leg.hasCurrent())
      {
        stream << " " << std::setw(7) << std::left << spType;
        if (strVerbosity >= SVL_NORMAL)
        {
          stream << " [" << leg->str(strVerbosity >= SVL_DETAILS);
          printPlaceHolderStr(stream, currentLegPos, leg.hasNext());
          stream << "]";
        }
      }
      else
      {
        stream << " -";
      }
    }
  }
  return stream.str();
}
}
} // namespace tse::shpq
