#ifndef MOCK_PAX_TYPE_FARE_WRAPPER_H
#define MOCK_PAX_TYPE_FARE_WRAPPER_H

#include "RexPricing/PaxTypeFareWrapper.h"

#include <memory>

namespace tse
{

class MockPaxTypeFareWrapper : public PaxTypeFareWrapper
{
public:
  MockPaxTypeFareWrapper(const MoneyAmount& amt, bool isIndustry = false)
    : PaxTypeFareWrapper(0),
      ptf_(std::shared_ptr<PaxTypeFare>(new PaxTypeFare)),
      fare_(std::shared_ptr<Fare>(new Fare)),
      fareInfo_(std::shared_ptr<FareInfo>(new FareInfo))
  {
    _ptf = ptf_.get();
    fare_->setFareInfo(fareInfo_.get());
    _ptf->setFare(fare_.get());

    _amount = amt;
    fare_->status().set(Fare::FS_IndustryFare, isIndustry);
  }

  MockPaxTypeFareWrapper(const VCTR& vctr, const MoneyAmount& amt)
    : PaxTypeFareWrapper(0),
      ptf_(std::shared_ptr<PaxTypeFare>(new PaxTypeFare)),
      fare_(std::shared_ptr<Fare>(new Fare)),
      fareInfo_(std::shared_ptr<FareInfo>(new FareInfo))
  {
    _ptf = ptf_.get();

    _amount = amt;
    fareInfo_->vendor() = vctr.vendor();
    fareInfo_->carrier() = vctr.carrier();
    fareInfo_->fareTariff() = vctr.tariff();
    fareInfo_->ruleNumber() = vctr.rule();

    _ptf->_tcrRuleTariff = vctr.tariff() + 6;

    fare_->setFareInfo(fareInfo_.get());
    _ptf->setFare(fare_.get());
  }

protected:
  std::shared_ptr<PaxTypeFare> ptf_;
  std::shared_ptr<Fare> fare_;
  std::shared_ptr<FareInfo> fareInfo_;
};
}

#endif
