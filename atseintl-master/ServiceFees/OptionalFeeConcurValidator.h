//-------------------------------------------------------------------
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
#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DBAccess/SubCodeInfo.h"
#include "ServiceFees/OCFees.h"

#include <set>
#include <string>
#include <vector>

namespace tse
{
class PricingTrx;
class Diag876Collector;
class FarePath;
class ServiceFeesGroup;
class OptionalServicesConcur;
class TravelSeg;
class DataHandle;

class OptionalFeeConcurValidator
{
  static const Indicator S6_INDCRXIND_OPERATING;
  static const Indicator S6_INDCRXIND_MARKETING;
  static const Indicator S6_INDCRXIND_BOTH;
  static const Indicator S6_INDCRXIND_FAREBUSTER;
  static const std::string S6_ANY_CARRIER;
  static const Indicator S6_INDCONCUR;

  static const Indicator CONCUR_ALLOWED;
  static const Indicator CONCUR_NOTALLOWED;
  static const Indicator CONCUR_NOCONCURRENCE;

  friend class OptionalFeeConcurValidatorTest;

  enum S5ConcurValidationResult
  {
    S5Required,
    S5NotAllowed,
    S5NoConcurence
  };

public:
  class S6OCFeesWrapper
  {
  public:
    enum S6ValidationResult
    {
      NOT_ROCESSED,
      FAIL_CONCUR,
      FAIL_NO_S6,
      FAIL_NO_S6_MATCH,
      FAIL_CONCUR_NOT_ALLOWED,
      FAIL_FARE_BUSTER,
      CONCUR_PASS,
      CONCUR_PASS_NO_CONCUR
    };
    static S6OCFeesWrapper* getS6OCFeesWrapper(DataHandle& dh, OCFees* oc);

    const VendorCode& vendor() const { return _ocFees.front()->subCodeInfo()->vendor(); }
    const CarrierCode& carrier() const { return _ocFees.front()->subCodeInfo()->carrier(); }
    const ServiceTypeCode& serviceTypeCode() const
    {
      return _ocFees.front()->subCodeInfo()->serviceTypeCode();
    }
    const ServiceSubTypeCode& serviceSubTypeCode() const
    {
      return _ocFees.front()->subCodeInfo()->serviceSubTypeCode();
    }
    const ServiceGroup& serviceGroup() const
    {
      return _ocFees.front()->subCodeInfo()->serviceGroup();
    }
    const ServiceGroup& serviceSubGroup() const
    {
      return _ocFees.front()->subCodeInfo()->serviceSubGroup();
    }
    const Indicator& concur() const { return _ocFees.front()->subCodeInfo()->concur(); }
    const SubCodeInfo* subCodeInfo() const { return _ocFees.front()->subCodeInfo(); }

    std::vector<OCFees*>& ocFees() { return _ocFees; }
    const std::vector<OCFees*>& ocFees() const { return _ocFees; }

    std::map<CarrierCode, std::vector<OptionalServicesConcur*> >& matchedS6() { return _matchedS6; }
    const std::map<CarrierCode, std::vector<OptionalServicesConcur*> >& matchedS6() const
    {
      return _matchedS6;
    }

    S6ValidationResult& status() { return _status; }
    const S6ValidationResult& status() const { return _status; }

    const char* statusString() const;
    S6ValidationResult canCarrierBeAssesed(const CarrierCode& s5cxr,
                                           const std::set<CarrierCode>& cxrs,
                                           bool marketing);

    S6OCFeesWrapper();
    const std::vector<OptionalServicesConcur*>& matchedSeq() const { return _matchedSeq; }

  private:
    std::vector<OCFees*> _ocFees;
    std::map<CarrierCode, std::vector<OptionalServicesConcur*> > _matchedS6;
    S6ValidationResult _status;
    std::vector<OptionalServicesConcur*> _matchedSeq;
  };

  OptionalFeeConcurValidator(PricingTrx& trx, FarePath* farePath);
  virtual ~OptionalFeeConcurValidator();

  bool checkMarkOperCxrs(const std::set<CarrierCode>& marketingCarriers,
                         const std::set<CarrierCode>& operatingCarriers,
                         const TravelSeg* begin,
                         const TravelSeg* end);

  virtual bool validateS6(const CarrierCode& cxr,
                          const std::set<CarrierCode>& cxrs,
                          const ServiceFeesGroup* srvFeesGrp,
                          bool marketing,
                          ServiceFeesGroup* retSrvFeesGrp);

  virtual void getOptCarrierVec(const std::set<CarrierCode>& marketingCxrs,
                                const std::vector<tse::TravelSeg*>::const_iterator& first,
                                const std::vector<tse::TravelSeg*>::const_iterator& last,
                                std::vector<CarrierCode>& outCarriers) const;

protected:
  bool shouldDisplayWithSC() const;
  void initResultSrvGroup(const CarrierCode& cxr, ServiceFeesGroup* srvFeesGrp);
  bool processS6Map(const CarrierCode& cxr,
                    const std::set<CarrierCode>& cxrs,
                    bool marketing,
                    bool shouldDisplay);
  bool initializeOCMap(const CarrierCode& cxr,
                       const ServiceFeesGroup* srvFeesGrp,
                       bool checkIndCarrierIndicator,
                       bool shouldDisplay);
  bool collectS6(const std::set<CarrierCode>& cxrs, bool shouldDiagDisplay);
  bool collectS6(const std::set<CarrierCode>& cxrs, S6OCFeesWrapper* s5, bool shouldDislay);
  bool getConcurs(const CarrierCode& cxr,
                  const SubCodeInfo* sci,
                  std::vector<OptionalServicesConcur*>& concurs,
                  bool shouldDisplayDiag);
  S5ConcurValidationResult shouldProcessS5(const SubCodeInfo* sci, bool shouldDisplayDiag);
  void createDiag();
  void endDiag();
  bool isDdAllS6() const;
  bool isDdAllS5() const;

  PricingTrx& _trx;
  FarePath* _farePath = nullptr;
  Diag876Collector* _diag876 = nullptr;
  VendorCode _vendor;
  bool _needValidation = false;
  bool _shouldDiagDisplay = false;
  std::vector<S6OCFeesWrapper*> _s5Map;
  bool _isDiag875 = false;
};
}

