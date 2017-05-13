///----------------------------------------------------------------------------
//
//  File:           ThrolltingPred.cpp
//  Created:        10/06/2015
//  Authors:
//
//  Description:    Common predicates required throttle transactions.
//
//  Updates:
//
//  Copyright Sabre 2015
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

#include "Common/ThrottlingPred.h"

#include "Common/Throttling.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/AncillaryPricingTrx.h"
#include "DataModel/Billing.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TaxTrx.h"
#include "DataModel/Trx.h"

#include <boost/tokenizer.hpp>

#include <algorithm>
#include <stdexcept>
#include <string>
#include <vector>

namespace tse
{
bool
Pred::test(const Trx& trx) const
{
  return true;
}

bool
Pred::test(const PricingTrx& trx) const
{
  return true;
}

bool
Pred::test(const AncillaryPricingTrx& trx) const
{
  return test(static_cast<const PricingTrx&>(trx));
}

bool
Pred::test(const TaxTrx& trx) const
{
  return test(static_cast<const PricingTrx&>(trx));
}

bool
Pred::test(const ShoppingTrx& trx) const
{
  return test(static_cast<const PricingTrx&>(trx));
}

void
Pred::parse(std::vector<std::string>& keyValue, const std::string& parseStr, const std::string sep)
{
  boost::tokenizer<boost::char_separator<char>> keyValueTok(
      parseStr, boost::char_separator<char>(sep.c_str()));

  std::copy(keyValueTok.begin(), keyValueTok.end(), std::back_inserter(keyValue));
}

bool
IfThen::test(const Trx& trx) const
{
  return _ifPred->test(trx) ? _thenPred->test(trx) : true;
}

bool
IfThen::test(const PricingTrx& trx) const
{
  return _ifPred->test(trx) ? _thenPred->test(trx) : true;
}

bool
IfThen::test(const AncillaryPricingTrx& trx) const
{
  return _ifPred->test(trx) ? _thenPred->test(trx) : true;
}

bool
IfThen::test(const TaxTrx& trx) const
{
  return _ifPred->test(trx) ? _thenPred->test(trx) : true;
}

bool
IfThen::test(const ShoppingTrx& trx) const
{
  return _ifPred->test(trx) ? _thenPred->test(trx) : true;
}

bool
OrPred::test(const Trx& trx) const
{
  return _oper1.test(trx) || _oper2.test(trx);
}

bool
OrPred::test(const PricingTrx& trx) const
{
  return _oper1.test(trx) || _oper2.test(trx);
}

bool
OrPred::test(const AncillaryPricingTrx& trx) const
{
  return _oper1.test(trx) || _oper2.test(trx);
}

bool
OrPred::test(const TaxTrx& trx) const
{
  return _oper1.test(trx) || _oper2.test(trx);
}

bool
OrPred::test(const ShoppingTrx& trx) const
{
  return _oper1.test(trx) || _oper2.test(trx);
}

bool
AndPred::test(const Trx& trx) const
{
  return _oper1.test(trx) && _oper2.test(trx);
}

bool
AndPred::test(const PricingTrx& trx) const
{
  return _oper1.test(trx) && _oper2.test(trx);
}

bool
AndPred::test(const AncillaryPricingTrx& trx) const
{
  return _oper1.test(trx) && _oper2.test(trx);
}

bool
AndPred::test(const TaxTrx& trx) const
{
  return _oper1.test(trx) && _oper2.test(trx);
}

bool
AndPred::test(const ShoppingTrx& trx) const
{
  return _oper1.test(trx) && _oper2.test(trx);
}

bool
PCCPred::test(const PricingTrx& trx) const
{
  return TrxUtil::getPCC(trx) == _val;
}

bool
PCCPred::test(const AncillaryPricingTrx& trx) const
{
  return TrxUtil::getPCCFromReq(trx) == _val;
}

bool
PCCPred::test(const TaxTrx& trx) const
{
  const Billing* billing = trx.billing();
  return (billing) ? (billing->userPseudoCityCode() == _val) : false;
}

bool
LNIATAPred::test(const PricingTrx& trx) const
{
  return TrxUtil::getLNIATA(trx) == _val;
}

bool
PNRPred::test(const PricingTrx& trx) const
{
  return TrxUtil::getPNR(trx) == _val;
}

bool
PARTPred::test(const PricingTrx& trx) const
{
  return TrxUtil::getHostCarrier(trx) == _val;
}

bool
SEGEqualPred::test(const PricingTrx& trx) const
{
  return std::all_of(trx.itin().cbegin(),
                     trx.itin().cend(),
                     [this](const Itin* const itin)
                     { return itin->travelSeg().size() == _val; });
}

bool
SEGEqualPred::test(const ShoppingTrx& trx) const
{
  return trx.numberOfLegs() == _val;
}

bool
SEGLessPred::test(const PricingTrx& trx) const
{
  return std::all_of(trx.itin().cbegin(),
                     trx.itin().cend(),
                     [this](const Itin* const itin)
                     { return itin->travelSeg().size() < _val; });
}

bool
SEGLessPred::test(const ShoppingTrx& trx) const
{
  return trx.numberOfLegs() < _val;
}

bool
SEGGreaterPred::test(const PricingTrx& trx) const
{
  return std::all_of(trx.itin().cbegin(),
                     trx.itin().cend(),
                     [this](const Itin* const itin)
                     { return itin->travelSeg().size() > _val; });
}

bool
SEGGreaterPred::test(const ShoppingTrx& trx) const
{
  return trx.numberOfLegs() > _val;
}

bool
SVCPred::test(const PricingTrx& trx) const
{
  return TrxUtil::parentServiceName(trx) == _val;
}

MKTPred::MKTPred(const std::string& val)
{
  if (val.length() == 6)
  {
    _origLocCode = val.substr(0, 3);
    _destLocCode = val.substr(3, 3);
  }
}

bool
MKTPred::test(const PricingTrx& trx) const
{
  return std::all_of(trx.itin().cbegin(),
                     trx.itin().cend(),
                     [this](const Itin* const itin)
                     {
    return (itin->firstTravelSeg()->boardMultiCity() == _origLocCode &&
            itin->lastTravelSeg()->offMultiCity() == _destLocCode) ||
           (itin->lastTravelSeg()->offMultiCity() == _origLocCode &&
            itin->firstTravelSeg()->boardMultiCity() == _destLocCode);
  });
}

bool
MKTPred::test(const ShoppingTrx& trx) const
{
  const TravelSeg* first = trx.legs().front().sop().front().itin()->firstTravelSeg();
  const TravelSeg* last = trx.legs().back().sop().front().itin()->lastTravelSeg();

  return (first->boardMultiCity() == _origLocCode && last->offMultiCity() == _destLocCode) ||
         (last->offMultiCity() == _origLocCode && first->boardMultiCity() == _destLocCode);
}

OperPriorSelect::OperPriorSelect(const std::string& str)
{
  size_t pos = str.find('|');
  if (pos != std::string::npos)
  {
    setOperands(str, pos);

    _pred = std::unique_ptr<Pred>(new OrPred(*_oper1, *_oper2));
    return;
  }

  pos = str.find('&');
  if (pos != std::string::npos)
  {
    setOperands(str, pos);

    _pred = std::unique_ptr<Pred>(new AndPred(*_oper1, *_oper2));
    return;
  }

  if (str.find_first_of("<>") != std::string::npos && str.find(THROTTLING_SEG) != std::string::npos)
  {
    std::vector<std::string> keyValue;
    parse(keyValue, str, "><");

    if (keyValue.size() != 2)
      throw std::logic_error("ALLOW TO HAVE ONLY 2 ARGUMENTS DIVIDED BY LESS OR GREATER(<>)");

    if (keyValue[0] == THROTTLING_SEG)
    {
      if (str.find('<') != std::string::npos)
        _pred = std::unique_ptr<Pred>(new SEGLessPred(keyValue[1]));
      else
        _pred = std::unique_ptr<Pred>(new SEGGreaterPred(keyValue[1]));
    }
  }
  else
  {
    std::vector<std::string> keyValue;
    parse(keyValue, str, "-");

    if (keyValue.size() != 2)
      throw std::logic_error("ALLOW TO HAVE ONLY 2 ARGUMENTS DIVIDED BY PAUSE(-)");

    if (keyValue[0] == THROTTLING_PCC)
      _pred = std::unique_ptr<Pred>(new PCCPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_LNIATA)
      _pred = std::unique_ptr<Pred>(new LNIATAPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_PNR)
      _pred = std::unique_ptr<Pred>(new PNRPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_PART)
      _pred = std::unique_ptr<Pred>(new PARTPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_SEG)
      _pred = std::unique_ptr<Pred>(new SEGEqualPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_SVC)
      _pred = std::unique_ptr<Pred>(new SVCPred(keyValue[1]));
    else if (keyValue[0] == THROTTLING_MKT)
      _pred = std::unique_ptr<Pred>(new MKTPred(keyValue[1]));
    else
      throw std::logic_error(
          "WRONG ATTRIBUTE TYPE. TYPE EITHER OF PCC,LNIATA,PNR,PART,SEG,SVC OR MKT");
  }
}

void
OperPriorSelect::setOperands(const std::string& str, size_t& pos)
{
  const std::string& strOper1 = str.substr(0, pos);
  _oper1 = std::unique_ptr<Pred>(new OperPriorSelect(strOper1));
  const std::string& strOper2 = str.substr(pos + 1, str.length() - pos);
  _oper2 = std::unique_ptr<Pred>(new OperPriorSelect(strOper2));
}

bool
OperPriorSelect::test(const Trx& trx) const
{
  return _pred->test(trx);
}

bool
OperPriorSelect::test(const PricingTrx& trx) const
{
  return _pred->test(trx);
}

bool
OperPriorSelect::test(const AncillaryPricingTrx& trx) const
{
  return _pred->test(trx);
}

bool
OperPriorSelect::test(const TaxTrx& trx) const
{
  return _pred->test(trx);
}

bool
OperPriorSelect::test(const ShoppingTrx& trx) const
{
  return _pred->test(trx);
}

IncrementCounter::IncrementCounter(const std::string& counterCondStr, const std::string& keyCondStr)
  : _keyCondStr(keyCondStr)
{
  std::vector<std::string> keyValue;
  parse(keyValue, counterCondStr, "-");

  if (keyValue.size() != 2 || keyValue[0] != THROTTLING_CONCURRENCY)
    throw std::logic_error("WRONG CONCURRENCY ATTRIBUTE LIKE CON-XX");

  _concurrentTrx = boost::lexical_cast<uint16_t>(keyValue[1]);
}

bool
IncrementCounter::test(const Trx& trx) const
{
  CounterTag tag(_concurrentTrx);
  return trx.throttling()->addCounterTag(_keyCondStr, tag).incrementCounter();
}

bool
IncrementCounter::test(const PricingTrx& trx) const
{
  return test(static_cast<const Trx&>(trx));
}

KeyPred::KeyPred(const std::string& str)
{
  std::vector<std::string> attribs;
  parse(attribs, str, "=");

  if (attribs.size() != 2)
    throw std::logic_error("ALLOW TO HAVE ONLY 2 ARGUMENTS DIVIDED BY EQUAL(=)");

  _operSel = std::unique_ptr<Pred>(new OperPriorSelect(attribs[0]));
  _incCounter = std::unique_ptr<Pred>(new IncrementCounter(attribs[1], str));

  _ifThen = IfThen(*_operSel, *_incCounter);
}

bool
KeyPred::test(const Trx& trx) const
{
  return _ifThen.test(trx);
}

bool
KeyPred::test(const PricingTrx& trx) const
{
  return _ifThen.test(trx);
}

bool
KeyPred::test(const AncillaryPricingTrx& trx) const
{
  return _ifThen.test(trx);
}

bool
KeyPred::test(const TaxTrx& trx) const
{
  return _ifThen.test(trx);
}

bool
KeyPred::test(const ShoppingTrx& trx) const
{
  return _ifThen.test(trx);
}

ParseCustomer::ParseCustomer(const std::string& str)
{
  std::vector<std::string> settings;
  parse(settings, str, ":");

  if (settings.empty() || settings.size() > 2)
    throw std::logic_error("ALLOW TO HAVE ONLY 2 ARGUMENTS DIVIDED BY DIVIDE SIGN(:)");

  if (settings.size() == 2)
  {
    _operSel = std::unique_ptr<Pred>(new OperPriorSelect(settings[1]));
    _key = std::unique_ptr<Pred>(new KeyPred(settings[0]));
  }
  else
  {
    _operSel = std::unique_ptr<Pred>(new Pred());
    _key = std::unique_ptr<Pred>(new KeyPred(settings[0]));
  }
  _ifThen = IfThen(*_operSel, *_key);
}

bool
ParseCustomer::test(const Trx& trx) const
{
  if (dynamic_cast<const ShoppingTrx*>(&trx))
    return _ifThen.test(static_cast<const ShoppingTrx&>(trx));
  if (dynamic_cast<const TaxTrx*>(&trx))
    return _ifThen.test(static_cast<const TaxTrx&>(trx));
  if (dynamic_cast<const AncillaryPricingTrx*>(&trx))
    return _ifThen.test(static_cast<const AncillaryPricingTrx&>(trx));
  if (dynamic_cast<const PricingTrx*>(&trx))
    return _ifThen.test(static_cast<const PricingTrx&>(trx));
  return true;
}

} // end tse
