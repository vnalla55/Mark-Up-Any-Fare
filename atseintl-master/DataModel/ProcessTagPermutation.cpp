//-------------------------------------------------------------------
//
//  File:        ProcessTagPermutation.cpp
//  Created:
//  Authors:     Artur Krezel
//
//  Description:
//
//  Updates:
//          10/31/07 - VN - file created.
//
//  Copyright Sabre 2007
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

#include "DataModel/ProcessTagPermutation.h"

#include "DataModel/FareUsage.h"
#include "DataModel/ProcessTagInfo.h"
#include "Rules/RuleConst.h"

#include <boost/bind.hpp>

#include <algorithm>
#include <array>

namespace tse
{
const Indicator ProcessTagPermutation::ENDORSEMENT_BLANK;
const Indicator ProcessTagPermutation::ENDORSEMENT_W;
const Indicator ProcessTagPermutation::ENDORSEMENT_X;
const Indicator ProcessTagPermutation::REISSUE_TO_LOWER_BLANK;
const Indicator ProcessTagPermutation::REISSUE_TO_LOWER_F;
const Indicator ProcessTagPermutation::REISSUE_TO_LOWER_R;
const Indicator ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BLANK;
const Indicator ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_B;
const Indicator ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_N;
const Indicator ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BN;
const Indicator ProcessTagPermutation::ELECTRONIC_TICKET_BLANK;
const Indicator ProcessTagPermutation::ELECTRONIC_TICKET_REQUIRED;
const Indicator ProcessTagPermutation::ELECTRONIC_TICKET_NOT_ALLOWED;
const Indicator ProcessTagPermutation::ELECTRONIC_TICKET_MIXED;
const Indicator ProcessTagPermutation::STOP_TRY_KEEP;
const Indicator ProcessTagPermutation::RESIDUAL_BLANK;
const Indicator ProcessTagPermutation::RESIDUAL_I;
const Indicator ProcessTagPermutation::RESIDUAL_N;
const Indicator ProcessTagPermutation::RESIDUAL_R;
const Indicator ProcessTagPermutation::RESIDUAL_S;
const Indicator ProcessTagPermutation::STOPCONN_BLANK;
const Indicator ProcessTagPermutation::STOPCONN_S;
const Indicator ProcessTagPermutation::STOPCONN_C;
const Indicator ProcessTagPermutation::STOPCONN_B;

bool
ProcessTagPermutation::needKeepFare() const
{
  return _fareApplWinnerTags.find(KEEP) != _fareApplWinnerTags.end();
}

const ProcessTagInfo*
ProcessTagPermutation::findProcessTagInfo(const FareMarket& fm) const
{
  std::vector<ProcessTagInfo*>::const_iterator tsiIter = _processTags.begin();
  std::vector<ProcessTagInfo*>::const_iterator tsiIterEnd = _processTags.end();

  for (; tsiIter != tsiIterEnd; ++tsiIter)
  {
    if (&fm == (*tsiIter)->fareMarket())
      return (*tsiIter);
  }
  return nullptr;
}

namespace
{

class Tag1StopY : public std::unary_function<const ProcessTagInfo*, bool>
{
public:
  bool operator()(const ProcessTagInfo* pti) const
  {
    return pti->reissueSequence()->orig() && pti->processTag() == KEEP_THE_FARES &&
           pti->reissueSequence()->stopInd() == ProcessTagPermutation::STOP_TRY_KEEP;
  }
};

class T988Exist : public std::unary_function<const ProcessTagInfo*, bool>
{
public:
  bool operator()(const ProcessTagInfo* pti) const { return pti->reissueSequence()->orig(); }
};

class TagFare : public std::unary_function<const ProcessTagInfo*, bool>
{
  const PaxTypeFare& _ptf;

public:
  TagFare(const PaxTypeFare& ptf) : _ptf(ptf) {}

  bool operator()(const ProcessTagInfo* pti) const { return pti->paxTypeFare() == &_ptf; }
};

} // namespace

bool
ProcessTagPermutation::hasTag7only() const
{
  return _processTags.front()->reissueSequence()->orig() &&
         _processTags.front()->processTag() == REISSUE_DOWN_TO_LOWER_FARE;
}

bool
ProcessTagPermutation::tag1StopYonly() const
{
  return std::find_if(_processTags.begin(), _processTags.end(), std::not1(Tag1StopY())) ==
         _processTags.end();
}

const ProcessTagInfo*
ProcessTagPermutation::firstWithT988() const
{
  std::vector<ProcessTagInfo*>::const_iterator firstWithT988 =
      std::find_if(_processTags.begin(), _processTags.end(), T988Exist());

  if (firstWithT988 != _processTags.end())
    return *firstWithT988;

  return nullptr;
}

bool
ProcessTagPermutation::hasZeroT988() const
{
  return std::find_if(_processTags.begin(), _processTags.end(), std::not1(T988Exist())) !=
         _processTags.end();
}

Indicator
ProcessTagPermutation::getEndorsementByte() const
{
  std::set<Indicator> bytes;

  for (ProcessTagInfo* pti : processTags())
    bytes.insert(pti->record3()->endorsement());

  static const std::array<Indicator, 4> endorsmentHierarchy = {
      ENDORSEMENT_BLANK, ENDORSEMENT_X, ENDORSEMENT_W, ENDORSEMENT_Y};

  const auto highestByte = std::find_if(endorsmentHierarchy.cbegin(),
                                        endorsmentHierarchy.cend(),
                                        [&bytes](const Indicator ind)
                                        { return bytes.count(ind) == 1U; });

  return (highestByte == endorsmentHierarchy.cend()) ? ENDORSEMENT_Y : *highestByte;
}

bool
ProcessTagPermutation::isOverriden() const
{
  std::vector<ProcessTagInfo*>::const_iterator pti = processTags().begin();
  for (; pti != processTags().end(); ++pti)
    if ((*pti)->isOverriden())
      return true;
  return false;
}

namespace
{

template<Indicator (ReissueSequenceW::*getMethod)() const>
struct CompareByte : public std::binary_function<const ProcessTagInfo*, Indicator, bool>
{
  bool operator()(const ProcessTagInfo* pti, Indicator i) const
  {
    return pti->reissueSequence()->orig() && (pti->reissueSequence()->*getMethod)() == i;
  }
};

using CompareReissueToLowerByte = CompareByte<&ReissueSequenceW::reissueToLower>;
using CompareStopoverConnectByte = CompareByte<&ReissueSequenceW::stopoverConnectInd>;

} // namsespace

Indicator
ProcessTagPermutation::getReissueToLowerByte() const
{
  static const Indicator t[2] = { REISSUE_TO_LOWER_F, REISSUE_TO_LOWER_R };
  CompareReissueToLowerByte cmp;

  std::vector<ProcessTagInfo*>::const_iterator
    b = processTags().begin(), e = processTags().end(),
    c = std::find_first_of(b, e, t, t+2, cmp);

  if (c == e)
    return REISSUE_TO_LOWER_BLANK;

  if (cmp(*c, REISSUE_TO_LOWER_F))
    return REISSUE_TO_LOWER_F;

  std::vector<ProcessTagInfo*>::const_iterator n =
      std::find_if(c, e, boost::bind(cmp, _1, REISSUE_TO_LOWER_F));

  return n != e ? REISSUE_TO_LOWER_F : REISSUE_TO_LOWER_R;
}

namespace
{

struct CmpByte156
{
  CmpByte156(Indicator ind = ProcessTagPermutation::NEW_TICKET_EQUAL_OR_HIGHER_BLANK) : _ind(ind) {}

  bool operator()(const ProcessTagInfo* pti) const
  {
    return pti->reissueSequence()->orig() ? pti->reissueSequence()->ticketEqualOrHigher() == _ind
                                          : false;
  }

  bool operator()(const ProcessTagInfo* pti, Indicator ind) const
  {
    return pti->reissueSequence()->orig() ? pti->reissueSequence()->ticketEqualOrHigher() == ind
                                          : false;
  }

protected:
  Indicator _ind;
};

} // namespace

Indicator
ProcessTagPermutation::checkTable988Byte156() const
{
  Indicator t[2] = { NEW_TICKET_EQUAL_OR_HIGHER_B, NEW_TICKET_EQUAL_OR_HIGHER_N };
  std::vector<ProcessTagInfo*>::const_iterator b = processTags().begin(), e = processTags().end(),
                                               bit = std::find_first_of(b, e, t, t+2, CmpByte156()),
                                               nit;
  if (bit == e)
    return NEW_TICKET_EQUAL_OR_HIGHER_BLANK;

  if ((*bit)->reissueSequence()->ticketEqualOrHigher() == NEW_TICKET_EQUAL_OR_HIGHER_B)
    nit = std::find_if(bit, e, CmpByte156(NEW_TICKET_EQUAL_OR_HIGHER_N));
  else
    nit = std::find_if(bit, e, CmpByte156(NEW_TICKET_EQUAL_OR_HIGHER_B));

  if (nit != e)
    return NEW_TICKET_EQUAL_OR_HIGHER_BN;

  if ((*bit)->reissueSequence()->ticketEqualOrHigher() == NEW_TICKET_EQUAL_OR_HIGHER_B)
    return NEW_TICKET_EQUAL_OR_HIGHER_B;
  else
    return NEW_TICKET_EQUAL_OR_HIGHER_N;
}

Indicator
ProcessTagPermutation::checkTable988Byte123()
{
  std::vector<ProcessTagInfo*>::const_iterator ptIt;
  _electronicTicket = ELECTRONIC_TICKET_BLANK;
  for (ptIt = processTags().begin(); ptIt != processTags().end(); ++ptIt)
  {
    if (!(*ptIt)->reissueSequence()->orig() ||
        (*ptIt)->reissueSequence()->electronicTktInd() == ELECTRONIC_TICKET_BLANK)
      continue;
    if (_electronicTicket == ELECTRONIC_TICKET_BLANK)
      _electronicTicket = (*ptIt)->reissueSequence()->electronicTktInd();
    else if (_electronicTicket != (*ptIt)->reissueSequence()->electronicTktInd())
      return _electronicTicket = ELECTRONIC_TICKET_MIXED;
  }
  return _electronicTicket;
}

bool
ProcessTagPermutation::isNMostRestrictiveResidualPenaltyInd() const
{
  std::vector<ProcessTagInfo*>::const_iterator ptIter = processTags().begin();
  std::vector<ProcessTagInfo*>::const_iterator ptIterEnd = processTags().end();

  Indicator residualInd;
  bool result = false;
  for (; ptIter != ptIterEnd; ++ptIter)
  {
    residualInd = (*ptIter)->record3()->residualInd();
    if (residualInd == RESIDUAL_I)
      return false;
    if (residualInd == RESIDUAL_N)
      result = true;
  }
  return result;
}

namespace
{

struct CmpByte84
{
  bool operator()(const ProcessTagInfo* lhs, const ProcessTagInfo* rhs) const
  {
    Indicator left = lhs->record3()->residualInd();
    if (left == ProcessTagPermutation::RESIDUAL_BLANK)
      return false;

    Indicator right = rhs->record3()->residualInd();
    if (right == ProcessTagPermutation::RESIDUAL_BLANK)
      return true;

    return left < right;
  }
};

class DifferentResInd : std::unary_function<const ProcessTagInfo*, bool>
{
  Indicator _residualInd;

public:
  DifferentResInd(Indicator residualInd) : _residualInd(residualInd) {}

  bool operator()(const ProcessTagInfo* pti) const
  {
    return pti->record3()->residualInd() != _residualInd;
  }
};

class IsChanged : std::unary_function<const TravelSeg*, bool>
{
public:
  bool operator()(const TravelSeg* ts) const { return ts->changeStatus() == TravelSeg::CHANGED; }
};

class NotChanged : std::unary_function<const ProcessTagInfo*, bool>
{
public:
  bool operator()(const ProcessTagInfo* pti) const
  {
    return std::find_if(pti->fareMarket()->travelSeg().begin(),
                        pti->fareMarket()->travelSeg().end(),
                        IsChanged()) == pti->fareMarket()->travelSeg().end();
  }
};

class MostRestrictiveHierarchy : std::unary_function<const ProcessTagInfo*, bool>
{
  static const Indicator LAST_RESTRICTIVE_HIERARCHY = 'X';

public:
  bool operator()(const ProcessTagInfo* pti) const
  {
    return pti->record3()->residualHierarchy() != LAST_RESTRICTIVE_HIERARCHY;
  }
};

} // namespace

FareApplication
ProcessTagPermutation::fareTypeSelection(FCChangeStatus cs) const
{
  switch (cs)
  {
  case UU:
    return _uu;
  case UC:
    return _uc;
  case UN:
    return _un;
  case FL:
    return _fl;
  default:
    return UNKNOWN_FA;
  }
}

void
ProcessTagPermutation::setFareTypeSelection(FCChangeStatus cs, FareApplication fa)
{
  switch (cs)
  {
  case UU:
    _uu = fa;
    break;
  case UC:
    _uc = fa;
    break;
  case UN:
    _un = fa;
    break;
  case FL:
    _fl = fa;
    break;
  default:
    ;
  }
}

FareApplication
ProcessTagPermutation::rebookFareTypeSelection(FCChangeStatus cs) const
{
  switch (cs)
  {
  case UN:
    return _rbun;
  case UU:
    return _rbuu;
  case UC:
    return _rbuc;
  case FL:
    return _rbfl;
  default:
    return UNKNOWN_FA;
  }
}

void
ProcessTagPermutation::setRebookFareTypeSelection(FCChangeStatus cs, FareApplication fa)
{
  switch (cs)
  {
  case UN:
    _rbun = fa;
    break;
  case UU:
    _rbuu = fa;
    break;
  case UC:
    _rbuc = fa;
    break;
  case FL:
    _rbfl = fa;
    break;
  default:
    ;
  }
}

Indicator
ProcessTagPermutation::getResidualPenaltyByte() const
{
  if (_processTags.empty())
    return RESIDUAL_BLANK;

  const std::vector<ProcessTagInfo*>* properVec = &_processTags;
  std::vector<ProcessTagInfo*> changed;

  if (residualIndsSame())
    return _processTags.front()->record3()->residualInd();

  std::remove_copy_if(
      _processTags.begin(), _processTags.end(), std::back_inserter(changed), NotChanged());

  if (!changed.empty())
    properVec = &changed;

  if (std::find_if(properVec->begin(), properVec->end(), MostRestrictiveHierarchy()) ==
      properVec->end())
    return (*std::max_element(properVec->begin(), properVec->end(), CmpByte84()))
        ->record3()
        ->residualInd();

  return (*std::min_element(properVec->begin(), properVec->end(), CmpByte84()))
      ->record3()
      ->residualInd();
}

bool
ProcessTagPermutation::residualIndsSame() const
{
  return std::find_if(++_processTags.begin(),
                      _processTags.end(),
                      DifferentResInd(_processTags.front()->record3()->residualInd())) ==
         _processTags.end();
}

Indicator
ProcessTagPermutation::formOfRefundInd() const
{
  std::vector<ProcessTagInfo*>::const_iterator ptIter = _processTags.begin(),
                                               ptIterEnd = _processTags.end();

  char formOfRefundInd = RuleConst::BLANK;
  char mostRestrictiveFormOfRefund = RuleConst::BLANK;

  for (; ptIter != ptIterEnd; ++ptIter)
  {
    formOfRefundInd = (*ptIter)->record3()->formOfRefund();
    if (formOfRefundInd == 'S')
    {
      return formOfRefundInd;
    }
    else if (formOfRefundInd == 'V')
    {
      mostRestrictiveFormOfRefund = 'V';
    }
    else if (formOfRefundInd == 'M')
    {
      if (mostRestrictiveFormOfRefund != 'V')
        mostRestrictiveFormOfRefund = 'M';
    }
  }
  return mostRestrictiveFormOfRefund;
}

bool
ProcessTagPermutation::needExpndKeepFare(const FareMarket& excFm) const
{
  const ProcessTagInfo* pti = findProcessTagInfo(excFm);
  return pti->reissueSequence()->orig() &&
         (pti->reissueSequence()->expndKeep() == ProcessTagInfo::EXPND_A ||
          pti->reissueSequence()->expndKeep() == ProcessTagInfo::EXPND_B);
}

const MoneyAmount&
ProcessTagPermutation::getEstimatedChangeFee() const
{
  return _estimatedChangeFee;
}

void
ProcessTagPermutation::setEstimatedChangeFee(MoneyAmount estimatedChangeFee)
{
  _estimatedChangeFee = estimatedChangeFee;
}


Indicator
ProcessTagPermutation::getStopoverConnectionByte() const
{
  static const Indicator t[3] = { STOPCONN_C, STOPCONN_B, STOPCONN_S };
  CompareStopoverConnectByte cmp;

  std::vector<ProcessTagInfo*>::const_iterator
    end = processTags().end(),
    pos = std::find_first_of(processTags().begin(), end, t, t+3, cmp);

  if (pos == end)
    return STOPCONN_BLANK;

  switch ((*pos)->reissueSequence()->stopoverConnectInd())
  {
  case STOPCONN_B:
    return STOPCONN_B;

  case STOPCONN_C:
    return std::find_first_of(++pos, end, t+1, t+3, cmp) == end ? STOPCONN_C : STOPCONN_B;

  case STOPCONN_S:
    return std::find_first_of(++pos, end, t, t+2, cmp) == end ? STOPCONN_S : STOPCONN_B;

  default:
    ;
  }
  return STOPCONN_BLANK;
}

} // tse
