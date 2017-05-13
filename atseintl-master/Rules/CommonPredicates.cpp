#include "Rules/CommonPredicates.h"

#include "Common/LocUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/DiagCollector.h"
#include "Rules/Debug.h"
#include "Util/BranchPrediction.h"

using namespace std;

namespace tse
{

//////////////////////////////////

std::string
Predicate::toString(int level) const
{
  std::string s = printTabs(level);
  s += _name + "\n";
  return s;
}

std::string
Predicate::printTabs(int level) const
{
  std::string s;
  for (int i = 0; i < level - 1; ++i)
    s += " |";
  if (level > 0)
    s += " +";
  return s;
}

std::string
And::toString(int level) const
{
  string s = Predicate::toString(level);
  vector<Predicate*>::const_iterator it;

  for (it = _components.begin(); it != _components.end(); ++it)
    s += (*it)->toString(level + 1);

  return s;
}

PredicateReturnType
And::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "And");
  retval.valid = PASS;
  vector<Predicate*>::iterator it;
  retval->matchedSegments.assign(itinerary.size(), false);

  for (it = _components.begin(); it != _components.end(); ++it)
  {
    PredicateReturnType val = (*it)->test(itinerary, trx);
    if (retval.valid != FAIL)
    {
      // If there is any SOFTPASS resulted, we would not
      // let PASS overwrite SOFTPASS
      if (LIKELY(!(val.valid == PASS && retval.valid == SOFTPASS)))
      {
        retval.valid = val.valid;
      }
    }
    if (val.valid == PASS || val.valid == SOFTPASS)
    {
      PredicateReturnType::iterator itr, retitr;
      for (itr = val->matchedSegments.begin(), retitr = retval->matchedSegments.begin();
           itr != val->matchedSegments.end();
           ++itr, ++retitr)
      {
        *retitr |= *itr;
      }
    }
  }

  return retval;
}

////////////////////////

std::string
Or::toString(int level) const
{
  string s = Predicate::toString(level);
  vector<Predicate*>::const_iterator it;

  for (it = components.begin(); it != components.end(); ++it)
    s += (*it)->toString(level + 1);

  return s;
}

PredicateReturnType
Or::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "Or");
  vector<Predicate*>::iterator it;
  retval->matchedSegments.assign(itinerary.size(), false);

  for (it = components.begin(); it != components.end(); ++it)
  {
    PredicateReturnType val = (*it)->test(itinerary, trx);
    if (val.valid == PASS || val.valid == SOFTPASS)
    {
      if (retval.valid != PASS)
      {
        retval.valid = val.valid; // SOFTPASS can not overwrite PASS
      }
      PredicateReturnType::iterator itr, retitr;
      for (itr = val->matchedSegments.begin(), retitr = retval->matchedSegments.begin();
           itr != val->matchedSegments.end();
           ++itr, ++retitr)
      {
        *retitr |= *itr;
      }
      // break; // First passed - 'or' satisfy
    }
  }

  return retval;
}

/////////////////////////////////

std::string
Not::toString(int level) const
{
  string s = Predicate::toString(level);
  if (_pred)
    s += _pred->toString(level + 1);
  else
    s += "Empty Predicate!!";

  return s;
}

PredicateReturnType
Not::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "Not");

  retval = _pred->test(itinerary, trx);
  // retval.valid = !retval.valid;
  if (retval.valid == FAIL)
    retval.valid = PASS;
  else if (LIKELY(retval.valid == PASS))
    retval.valid = FAIL;
  else if (retval.valid == SOFTPASS)
  {
    PredicateReturnType::iterator it = retval->matchedSegments.begin();
    vector<TravelSeg*>::const_iterator tvlSegIt = itinerary.begin();
    const vector<TravelSeg*>::const_iterator tvlSegItEnd = itinerary.end();
    for (; tvlSegIt != tvlSegItEnd; it++, tvlSegIt++)
    {
      if ((*tvlSegIt)->segmentType() != Open)
      {
        if (*it)
          retval.valid = FAIL;

        *it = !*it;
      }
    }

    return retval;
  }

  PredicateReturnType::iterator it = retval->matchedSegments.begin();
  for (; it != retval->matchedSegments.end(); ++it)
    *it = !*it;

  return retval;
}

/////////////////////////////////

std::string
IfThenElse::toString(int level) const
{
  string s = Predicate::toString(level);
  s += _if->toString(level + 1);
  s += _then->toString(level + 1);
  if (_else)
    s += _else->toString(level + 1);

  return s;
}

PredicateReturnType
IfThenElse::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  if (_if->test(itinerary, trx).valid == PASS)
    return _then->test(itinerary, trx);
  else if (_else)
    return _else->test(itinerary, trx);

  PredicateReturnType retval;
  return retval;
}

//////////////

std::string
SameSegments::toString(int level) const
{
  std::string s = Predicate::toString(level);
  std::vector<Predicate*>::const_iterator it;

  if (_principal)
    s += _principal->toString(level + 1);
  for (it = _components.begin(); it != _components.end(); ++it)
    s += (*it)->toString(level + 1);

  return s;
}

PredicateReturnType
SameSegments::test(const vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "SameSegments");
  PredicateReturnType principal;
  if (LIKELY(_principal))
    principal = _principal->test(itinerary, trx);
  else
  {
    principal.valid = PASS;
    principal->matchedSegments.assign(itinerary.size(), true);
  }
  retval->matchedSegments.assign(itinerary.size(), false);

  if (principal.valid == FAIL)
  {
    retval.valid = FAIL; // no need to keep going if no PASS on principal
    return retval;
  }

  if (_components.empty())
  {
    retval.valid = principal.valid; // For PredicateOutputPrinter only
    return principal;
  }

  std::vector<Predicate*>::iterator it;

  for (it = _components.begin(); it != _components.end(); ++it)
  {
    PredicateReturnType val = (*it)->test(itinerary, trx);
    if (val.valid == FAIL)
    {
      retval.valid = FAIL;
      break;
    }

    PredicateReturnType::iterator elem, cond, ret;
    for (elem = principal->matchedSegments.begin(),
        cond = val->matchedSegments.begin(),
        ret = retval->matchedSegments.begin();
         elem != principal->matchedSegments.end();
         ++elem, ++cond, ++ret)
    {
      *ret |= *elem & *cond;
      if (*ret && retval.valid != SOFTPASS)
      {
        if (val.valid == SOFTPASS || principal.valid == SOFTPASS)
        {
          retval.valid = SOFTPASS;
        }
        else
        {
          retval.valid = PASS;
        }
      }
    }
    if (retval.valid == FAIL)
      break;
  }

  return retval;
}

string
ContainsInternational::toString(int level) const
{
  return Predicate::toString(level);
}

PredicateReturnType
ContainsInternational::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;
  PredicateOutputPrinter p(retval.valid, "ContainsInternational");
  retval.valid = FAIL;
  retval->matchedSegments.assign(itinerary.size(), false);

  vector<TravelSeg*>::const_iterator it;
  it = itinerary.begin();
  for (unsigned i = 0; it != itinerary.end(); ++it, ++i)
  {
    if (LocUtil::isInternational(*(*it)->origin(), *(*it)->destination()))
    {
      retval.valid = PASS;
      retval->matchedSegments[i] = true;
    }
  }

  return retval;
}

// Text Only

string
TextOnly::toString(int level) const
{
  return Predicate::toString(level);
}

PredicateReturnType
TextOnly::test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx)
{
  PredicateReturnType retval;

  if (UNLIKELY(trx.diagnostic().diagnosticType() == _diag))
  {
    DCFactory* factory = DCFactory::instance();
    DiagCollector* diag = factory->create(const_cast<PricingTrx&>(trx));
    diag->enable(_diag);

    *diag << _text << endl;

    diag->flushMsg();
  }

  if (_isUnavailable)
  {
    retval.valid = FAIL;
  }
  else
  {
    retval.valid = SKIP;
  }

  return retval;
}
} // namespace tse
