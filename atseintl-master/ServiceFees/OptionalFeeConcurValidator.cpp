//  Copyright Sabre 2009
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
#include "ServiceFees/OptionalFeeConcurValidator.h"

#include "Common/LocUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/OptionalServicesConcur.h"
#include "DBAccess/SubCodeInfo.h"
#include "Diagnostic/DCFactory.h"
#include "Diagnostic/Diag876Collector.h"
#include "Diagnostic/Diagnostic.h"
#include "ServiceFees/OCFees.h"
#include "ServiceFees/OptionalFeeCollector.h"
#include "ServiceFees/ServiceFeesGroup.h"

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/lexical_cast.hpp>

namespace tse
{

const Indicator OptionalFeeConcurValidator::S6_INDCRXIND_OPERATING = 'O';
const Indicator OptionalFeeConcurValidator::S6_INDCRXIND_MARKETING = 'M';
const Indicator OptionalFeeConcurValidator::S6_INDCRXIND_BOTH = 'E';
const Indicator OptionalFeeConcurValidator::S6_INDCRXIND_FAREBUSTER = 'F';
const std::string OptionalFeeConcurValidator::S6_ANY_CARRIER = "$$";
const Indicator OptionalFeeConcurValidator::S6_INDCONCUR = 'Y';
const Indicator OptionalFeeConcurValidator::CONCUR_ALLOWED = '1';
const Indicator OptionalFeeConcurValidator::CONCUR_NOTALLOWED = '2';
const Indicator OptionalFeeConcurValidator::CONCUR_NOCONCURRENCE = 'X';

namespace
{

struct MatchOCToWrapper
{
  bool operator()(const OptionalFeeConcurValidator::S6OCFeesWrapper* w) const
  {
    return w->serviceTypeCode() == _oc->subCodeInfo()->serviceTypeCode() &&
           w->serviceGroup() == _oc->subCodeInfo()->serviceGroup() &&
           w->carrier() == _oc->subCodeInfo()->carrier() &&
           w->vendor() == _oc->subCodeInfo()->vendor() &&
           w->serviceSubTypeCode() == _oc->subCodeInfo()->serviceSubTypeCode() &&
           w->serviceSubGroup() == _oc->subCodeInfo()->serviceSubGroup() &&
           w->concur() == _oc->subCodeInfo()->concur();
  }

  MatchOCToWrapper(OCFees* oc) : _oc(oc) {}

private:
  OCFees* _oc;
};

struct S6WrapperDiagSorter
{
  enum CmpRes
  {
    LS = -1,
    EQ = 0,
    GT = 1
  };
  S6WrapperDiagSorter(const std::string param)
  {
    if (param == Diagnostic::SRV_GROUP) // SOSG
      _cmpFun = boost::bind(boost::mem_fn(&S6WrapperDiagSorter::sortBySG), this, _1, _2);
    else if (param == Diagnostic::SRV_CODE) // SOSC
      _cmpFun = boost::bind(boost::mem_fn(&S6WrapperDiagSorter::sortBySC), this, _1, _2);
    else // by default SOCX
      _cmpFun = boost::bind(boost::mem_fn(&S6WrapperDiagSorter::sortByCX), this, _1, _2);
  }
  bool operator()(const OptionalFeeConcurValidator::S6OCFeesWrapper* w1,
                  const OptionalFeeConcurValidator::S6OCFeesWrapper* w2) const
  {
    if (w1->serviceTypeCode() < w2->serviceTypeCode())
      return true;
    else if (w1->serviceTypeCode() > w2->serviceTypeCode())
      return false;
    CmpRes res = _cmpFun(w1, w2);
    if (res == LS)
      return true;
    else if (res == GT)
      return false;
    else if (w1->vendor() < w2->vendor())
      return true;
    else if (w1->vendor() > w2->vendor())
      return false;
    else if (w1->serviceSubGroup() < w2->serviceSubGroup())
      return true;
    else if (w1->serviceSubGroup() > w2->serviceSubGroup())
      return false;
    return w1->concur() < w2->concur();
  }

private:
  CmpRes sortBySG(const OptionalFeeConcurValidator::S6OCFeesWrapper* w1,
                  const OptionalFeeConcurValidator::S6OCFeesWrapper* w2) const
  {
    if (w1->serviceGroup() < w2->serviceGroup())
      return LS;
    else if (w1->serviceGroup() > w2->serviceGroup())
      return GT;
    else if (w1->carrier() < w2->carrier())
      return LS;
    else if (w1->carrier() > w2->carrier())
      return GT;
    else if (w1->serviceSubTypeCode() < w2->serviceSubTypeCode())
      return LS;
    else if (w1->serviceSubTypeCode() > w2->serviceSubTypeCode())
      return GT;
    return EQ;
  }
  CmpRes sortByCX(const OptionalFeeConcurValidator::S6OCFeesWrapper* w1,
                  const OptionalFeeConcurValidator::S6OCFeesWrapper* w2) const
  {
    if (w1->carrier() < w2->carrier())
      return LS;
    else if (w1->carrier() > w2->carrier())
      return GT;
    else if (w1->serviceGroup() < w2->serviceGroup())
      return LS;
    else if (w1->serviceGroup() > w2->serviceGroup())
      return GT;
    else if (w1->serviceSubTypeCode() < w2->serviceSubTypeCode())
      return LS;
    else if (w1->serviceSubTypeCode() > w2->serviceSubTypeCode())
      return GT;
    return EQ;
  }
  CmpRes sortBySC(const OptionalFeeConcurValidator::S6OCFeesWrapper* w1,
                  const OptionalFeeConcurValidator::S6OCFeesWrapper* w2) const
  {
    if (w1->serviceSubTypeCode() < w2->serviceSubTypeCode())
      return LS;
    else if (w1->serviceSubTypeCode() > w2->serviceSubTypeCode())
      return GT;
    else if (w1->serviceGroup() < w2->serviceGroup())
      return LS;
    else if (w1->serviceGroup() > w2->serviceGroup())
      return GT;
    else if (w1->carrier() < w2->carrier())
      return LS;
    else if (w1->carrier() > w2->carrier())
      return GT;
    return EQ;
  }
  boost::function<CmpRes(const OptionalFeeConcurValidator::S6OCFeesWrapper* w1,
                         const OptionalFeeConcurValidator::S6OCFeesWrapper* w2)> _cmpFun;
};
}

OptionalFeeConcurValidator::OptionalFeeConcurValidator(PricingTrx& trx, FarePath* farePath)
  : _trx(trx), _farePath(farePath)
{
  createDiag();
}

OptionalFeeConcurValidator::~OptionalFeeConcurValidator()
{
  if (_diag876)
    _diag876->flushMsg();
}

bool
OptionalFeeConcurValidator::shouldDisplayWithSC() const
{
  bool ret = true;
  std::string diagCode = _trx.diagnostic().diagParamMapItem(Diagnostic::SRV_CODE);
  if (!diagCode.empty())
  {
    for (S6OCFeesWrapper* imt : _s5Map)
      if (imt->serviceSubTypeCode() == diagCode)
        return true;

    ret = false;
  }
  return ret;
}

bool
OptionalFeeConcurValidator::checkMarkOperCxrs(const std::set<CarrierCode>& marketingCarriers,
                                              const std::set<CarrierCode>& operatingCarriers,
                                              const TravelSeg* begin,
                                              const TravelSeg* end)
{
  _needValidation = false;

  // one marketing and one operating
  if (marketingCarriers.size() == 1 && operatingCarriers.size() == 1 &&
      *operatingCarriers.begin() != *marketingCarriers.begin())
    _needValidation = true;

  // one marketing and few operating
  if (marketingCarriers.size() == 1 && operatingCarriers.size() > 1)
    _needValidation = true;

  // few marketing and few operating
  if (marketingCarriers.size() > 1 && operatingCarriers.size() > 1)
    _needValidation = true;

  // if diagnostic created and match FM qualifier
  _shouldDiagDisplay = _diag876 && _diag876->shouldDisplay(begin, end);

  // print header
  if (_shouldDiagDisplay)
    _diag876->printHeader(marketingCarriers, operatingCarriers, begin, end, _needValidation, *_farePath);

  // if DDALL then we want to process S6, even if really not needed
  if (isDdAllS5() || isDdAllS6())
    return true;

  return _needValidation;
}

bool
OptionalFeeConcurValidator::initializeOCMap(const CarrierCode& cxr,
                                            const ServiceFeesGroup* srvFeesGrp,
                                            bool checkIndCarrierIndicator,
                                            bool shouldDisplay)
{
  _s5Map.clear();
  // get OCFees for FarePath
  if (srvFeesGrp->ocFeesMap().find(_farePath) == srvFeesGrp->ocFeesMap().end() ||
      srvFeesGrp->ocFeesMap().find(_farePath)->second.empty())
  {
    if (shouldDisplay)
      _diag876->printNoOCFeesFound(cxr, srvFeesGrp->groupCode());

    return false;
  }

  // iterate by all OCFees
  for (OCFees* oc : srvFeesGrp->ocFeesMap().find(_farePath)->second)
  {
    // if this is carrier defined subcode, skip this S5
    if (checkIndCarrierIndicator &&
        (oc->subCodeInfo()->industryCarrierInd() == OptionalFeeCollector::S5_INDCRXIND_CARRIER))
      continue;

    // check if we have same S5 key in vec
    std::vector<S6OCFeesWrapper*>::iterator s5 =
        std::find_if(_s5Map.begin(), _s5Map.end(), MatchOCToWrapper(oc));
    // if found, add thic OC to vector, else create new s6Wrapper
    if (s5 != _s5Map.end())
      (*s5)->ocFees().push_back(oc);
    else
      _s5Map.push_back(S6OCFeesWrapper::getS6OCFeesWrapper(_trx.dataHandle(), oc));
  }
  // sort only for diagnostic
  if (shouldDisplay)
    std::stable_sort(
        _s5Map.begin(),
        _s5Map.end(),
        S6WrapperDiagSorter(_trx.diagnostic().diagParamMapItem(Diagnostic::WPNC_SOLO_TEST)));

  return true;
}

bool
OptionalFeeConcurValidator::collectS6(const std::set<CarrierCode>& cxrs,
                                      S6OCFeesWrapper* s5,
                                      bool shouldDislay)
{
  for (CarrierCode cxr : cxrs)
  {
    // we should have at least 1 record for each carrier
    if (!getConcurs(cxr, s5->subCodeInfo(), s5->matchedS6()[cxr], shouldDislay))
      return false;

    if (shouldDislay)
      _diag876->printS6Found(!s5->matchedS6()[cxr].empty());
  }
  // we have S6
  return true;
}

bool
OptionalFeeConcurValidator::collectS6(const std::set<CarrierCode>& cxrs, bool shouldDiagDisplay)
{
  // for each S5, try to find matching S6 to carrier
  bool anyFound = false;
  bool shouldDislay = false;
  for (S6OCFeesWrapper* s5 : _s5Map)
  {
    shouldDislay = shouldDiagDisplay && _diag876->shouldDisplay(*s5->subCodeInfo());
    // check if S6 need to be processed
    switch (shouldProcessS5(s5->subCodeInfo(), shouldDislay))
    {
    case S5NoConcurence:
      anyFound = true;
      break;

    case S5NotAllowed:
      continue;

    case S5Required:
      anyFound |= collectS6(cxrs, s5, shouldDislay);
    };
  }
  return anyFound;
}

bool
OptionalFeeConcurValidator::getConcurs(const CarrierCode& cxr,
                                       const SubCodeInfo* sci,
                                       std::vector<OptionalServicesConcur*>& rconcurs,
                                       bool shouldDisplayDiag)
{
  // clear previos response
  rconcurs.clear();

  if (!sci)
    return false;

  const std::vector<OptionalServicesConcur*>& concurs = _trx.dataHandle().getOptionalServicesConcur(
      sci->vendor(), cxr, sci->serviceTypeCode(), _trx.ticketingDate());
  if (concurs.empty())
  {
    if (shouldDisplayDiag)
      _diag876->printNoS6InDB();
    return false;
  }

  for (OptionalServicesConcur* it : concurs)
  {
    if (it->serviceTypeCode() != sci->serviceTypeCode())
    {
      if (shouldDisplayDiag && isDdAllS6())
        _diag876->printS6(*it, "*** NO MATCH ON SERVICE TYPE CODE ***");
      continue;
    }

    if (!it->serviceGroup().empty() && it->serviceGroup() != sci->serviceGroup())
    {
      if (shouldDisplayDiag && isDdAllS6())
        _diag876->printS6(*it, "*** NO MATCH ON SERVICE GROUP ***");
      continue;
    }

    if (!it->serviceSubGroup().empty() && it->serviceSubGroup() != sci->serviceSubGroup())
    {
      if (shouldDisplayDiag && isDdAllS6())
        _diag876->printS6(*it, "*** NO MATCH ON SERVICE SUBGROUP ***");
      continue;
    }

    if (!it->serviceSubTypeCode().empty() && it->serviceSubTypeCode() != sci->serviceSubTypeCode())
    {
      if (shouldDisplayDiag && isDdAllS6())
        _diag876->printS6(*it, "*** NO MATCH ON SERVICE SUB TYPE CODE ***");
      continue;
    }

    if (shouldDisplayDiag)
      _diag876->printS6(*it);

    rconcurs.push_back(it);
  }
  return !rconcurs.empty();
}

bool
OptionalFeeConcurValidator::processS6Map(const CarrierCode& cxr,
                                         const std::set<CarrierCode>& cxrs,
                                         bool marketing,
                                         bool shouldDisplay)
{
  bool anyMatch = false;

  for (S6OCFeesWrapper* imt : _s5Map)
  {
    if (imt->concur() == CONCUR_NOCONCURRENCE)
    {
      imt->status() = S6OCFeesWrapper::CONCUR_PASS_NO_CONCUR;
      anyMatch = true;
    }
    else if (imt->concur() == CONCUR_NOTALLOWED)
    {
      imt->status() = S6OCFeesWrapper::FAIL_CONCUR;
    }
    else
    {
      imt->status() = imt->canCarrierBeAssesed(cxr, cxrs, marketing);
      anyMatch |= imt->status() == S6OCFeesWrapper::CONCUR_PASS;
    }
  }

  if (shouldDisplay)
  {
    bool longDisplay = isDdAllS5();
    _diag876->printS6ValidationHeader(marketing);
    if (!anyMatch && !longDisplay)
      _diag876->printS6ValidationNoPass();

    for (S6OCFeesWrapper* imt : _s5Map)
    {
      if (longDisplay || imt->status() == S6OCFeesWrapper::CONCUR_PASS ||
          imt->status() == S6OCFeesWrapper::CONCUR_PASS_NO_CONCUR)
      {
        _diag876->printS6Validation(*imt->subCodeInfo(), imt->statusString());
        std::vector<OptionalServicesConcur*>::const_iterator it = imt->matchedSeq().begin();
        std::vector<OptionalServicesConcur*>::const_iterator ie = imt->matchedSeq().end();
        for (; it != ie; it++)
          _diag876->printS6PassSeqNo(*imt->subCodeInfo(), **it);
      }
    }
    _diag876->addStarLine();
  }
  return anyMatch;
}

bool
OptionalFeeConcurValidator::validateS6(const CarrierCode& cxr,
                                       const std::set<CarrierCode>& carriers,
                                       const ServiceFeesGroup* srvFeesGrp,
                                       bool marketing,
                                       ServiceFeesGroup* retSrvFeesGrp)
{
  // remove processed carrier from set
  std::set<CarrierCode> cxrs;
  std::remove_copy(carriers.begin(), carriers.end(), std::inserter(cxrs, cxrs.end()), cxr);
  bool ret = false;

  bool shouldDisplay =
      _shouldDiagDisplay && _diag876->shouldDisplay(srvFeesGrp) && _diag876->shouldDisplay(cxr);

  // when validate operating with multiple sectors, we should return S5 with carrierIndIndicator =
  // 'C'
  bool checkIndCxrIndicator = !marketing && (carriers.size() > 1);

  // initalize oc wrapping vector map
  if (!initializeOCMap(cxr, srvFeesGrp, checkIndCxrIndicator, shouldDisplay))
    return false;

  // check if something is in map match diag SC qualifier (only if diag requested)
  if (shouldDisplay)
    shouldDisplay = shouldDisplayWithSC();

  // collect s6 for carrier (if diag requested, we will continue process, even there is nothing in
  // map)
  if (!collectS6(cxrs, shouldDisplay) && !(shouldDisplay || _isDiag875))
    return false;

  // process map
  ret = processS6Map(cxr, cxrs, marketing, shouldDisplay);

  if (ret || _isDiag875)
    initResultSrvGroup(cxr, retSrvFeesGrp);

  // if DDALL was used we processed, and we shouldn't validate return false
  return (_needValidation && ret) || _isDiag875;
}

void
OptionalFeeConcurValidator::initResultSrvGroup(const CarrierCode& cxr, ServiceFeesGroup* srvFeesGrp)
{
  for (S6OCFeesWrapper* imt : _s5Map)
  {
    if (imt->carrier() != cxr)
      continue;

    if (imt->status() != S6OCFeesWrapper::CONCUR_PASS &&
        imt->status() != S6OCFeesWrapper::CONCUR_PASS_NO_CONCUR)
    {
      if (_isDiag875)
      {
        for (OCFees* oc : imt->ocFees())
          oc->failS6() = true;
      }
      else
        continue;
    }
    const boost::lock_guard<boost::mutex> guard(srvFeesGrp->mutex());
    // copy OCFees to resulting group
    for (OCFees* oc : imt->ocFees())
      srvFeesGrp->ocFeesMap()[_farePath].push_back(oc);
  }
}

void
OptionalFeeConcurValidator::getOptCarrierVec(
    const std::set<CarrierCode>& marketingCxrs,
    const std::vector<tse::TravelSeg*>::const_iterator& first,
    const std::vector<tse::TravelSeg*>::const_iterator& last,
    std::vector<CarrierCode>& outCarriers) const
{
  // First should be carriers from International segment
  std::vector<TravelSeg*> internationalSegments;
  std::vector<TravelSeg*> itin;
  std::copy(first, last, std::back_inserter(itin));
  LocUtil::getInternationalSegments(itin, internationalSegments);
  for (TravelSeg* ts : internationalSegments)
  {
    if (!ts->isAir())
      continue;

    if (std::find(outCarriers.begin(),
                  outCarriers.end(),
                  static_cast<AirSeg*>(ts)->operatingCarrierCode()) != outCarriers.end())
      continue;

    outCarriers.push_back(static_cast<AirSeg*>(ts)->operatingCarrierCode());
  }
  // and domestic
  for (TravelSeg* ts : itin)
  {
    if (!ts->isAir())
      continue;

    if (std::find(internationalSegments.begin(), internationalSegments.end(), ts) !=
        internationalSegments.end())
      continue;

    if (std::find(outCarriers.begin(),
                  outCarriers.end(),
                  static_cast<AirSeg*>(ts)->operatingCarrierCode()) != outCarriers.end())
      continue;

    outCarriers.push_back(static_cast<AirSeg*>(ts)->operatingCarrierCode());
  }
  // if one marketing, then we don't process it as operating
  if (marketingCxrs.size() == 1)
  {
    std::vector<CarrierCode>::iterator it =
        std::find(outCarriers.begin(), outCarriers.end(), *marketingCxrs.begin());
    if (it != outCarriers.end())
      outCarriers.erase(it);
  }
}

OptionalFeeConcurValidator::S5ConcurValidationResult
OptionalFeeConcurValidator::shouldProcessS5(const SubCodeInfo* sci, bool shouldDisplayDiag)
{
  if (shouldDisplayDiag)
  {
    _diag876->printS5(*sci);
    _diag876->printS5Concur(sci->concur());
  }
  // check only concur field of S5
  switch (sci->concur())
  {
  case CONCUR_NOTALLOWED:
    return S5NotAllowed;
  case CONCUR_NOCONCURRENCE:
    return S5NoConcurence;
  }
  return S5Required;
}

void
OptionalFeeConcurValidator::createDiag()
{
  if (!_trx.diagnostic().isActive())
    return;

  DiagnosticTypes diagType = _trx.diagnostic().diagnosticType();
  if (diagType == Diagnostic876)
  {
    DiagCollector* diag = DCFactory::instance()->create(_trx);
    if (!diag)
      return;

    _diag876 = dynamic_cast<Diag876Collector*>(diag);
    if (!_diag876)
      return;

    _diag876->enable(Diagnostic876);
    _diag876->initTrx(_trx);
  }
  _isDiag875 = (Diagnostic875 == _trx.diagnostic().diagnosticType());
}

bool
OptionalFeeConcurValidator::isDdAllS5() const
{
  return (_shouldDiagDisplay &&
             (_trx.diagnostic().diagParamMapItem(Diagnostic::MISCELLANEOUS) == "ALL")) ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::MISCELLANEOUS) == "ALLS5") ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL") ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLS5");
}

bool
OptionalFeeConcurValidator::isDdAllS6() const
{
  return (_shouldDiagDisplay &&
             (_trx.diagnostic().diagParamMapItem(Diagnostic::MISCELLANEOUS) == "ALL")) ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::MISCELLANEOUS) == "ALLS6") ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL") ||
         (_trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALLS6");
}

OptionalFeeConcurValidator::S6OCFeesWrapper::S6OCFeesWrapper() : _status(NOT_ROCESSED) {}

OptionalFeeConcurValidator::S6OCFeesWrapper*
OptionalFeeConcurValidator::S6OCFeesWrapper::getS6OCFeesWrapper(DataHandle& dh, OCFees* oc)
{
  S6OCFeesWrapper* ret(nullptr);
  dh.get(ret);
  ret->ocFees().push_back(oc);
  return ret;
}

OptionalFeeConcurValidator::S6OCFeesWrapper::S6ValidationResult
OptionalFeeConcurValidator::S6OCFeesWrapper::canCarrierBeAssesed(const CarrierCode& s5cxr,
                                                                 const std::set<CarrierCode>& cxrs,
                                                                 bool marketing)
{
  // if no S6 records "match" then concurrence is not permitted and the initiating S5 is not valid.

  S6ValidationResult ret = FAIL_NO_S6_MATCH;
  for (CarrierCode cxr : cxrs)
  {
    if (_matchedS6[cxr].empty())
      return FAIL_NO_S6;

    ret = FAIL_NO_S6_MATCH;
    for (OptionalServicesConcur* concur : _matchedS6[cxr])
    {
      if (concur->mkgOperFareOwner() == S6_INDCRXIND_FAREBUSTER)
        return FAIL_FARE_BUSTER;

      if (marketing && (concur->mkgOperFareOwner() != S6_INDCRXIND_MARKETING) &&
          (concur->mkgOperFareOwner() != S6_INDCRXIND_BOTH))
        continue;

      if (!marketing && (concur->mkgOperFareOwner() != S6_INDCRXIND_OPERATING) &&
          (concur->mkgOperFareOwner() != S6_INDCRXIND_BOTH))
        continue;

      if (concur->accessedCarrier() != S6_ANY_CARRIER && concur->accessedCarrier() != s5cxr)
        continue;

      // of motch on 'N' - then fail this set
      if (concur->concur() != S6_INDCONCUR)
        ret = FAIL_CONCUR_NOT_ALLOWED;
      else
        ret = CONCUR_PASS;

      _matchedSeq.push_back(concur);

      break;
    }

    // if not PASS, then fail whole set
    if (ret != CONCUR_PASS)
      return ret;
  }
  return ret;
}

const char*
OptionalFeeConcurValidator::S6OCFeesWrapper::statusString() const
{
  switch (_status)
  {
  case NOT_ROCESSED:
    return "NOT PROCESSED";
  case FAIL_CONCUR:
    return "FAIL CONCUR";
  case FAIL_NO_S6:
    return "NO S6 FOR CXR";
  case FAIL_NO_S6_MATCH:
    return "NO MATCH ON S6";
  case FAIL_CONCUR_NOT_ALLOWED:
    return "NOT ALLOWED";
  case FAIL_FARE_BUSTER:
    return "FAIL FARE BUSTER";
  case CONCUR_PASS:
    return "PASS";
  case CONCUR_PASS_NO_CONCUR:
    return "NO CONCUR REQUIRED";
  }
  return nullptr;
}
}

