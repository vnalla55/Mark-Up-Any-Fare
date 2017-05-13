//-------------------------------------------------------------------
//  Copyright Sabre 2016
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

#include "DataModel/PaxTypeFare.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
class PaxTypeFareBuilder
{
public:
  PaxTypeFareBuilder(FareMarket* market, PricingTrx& trx) : _market(market), _trx(trx)
  {
    _ptf = _memHandle.create<PaxTypeFare>();
    _ptf->fareClassAppSegInfo() = _memHandle.create<FareClassAppSegInfo>();

    _fare = _memHandle.create<Fare>();
    _fare->status().set(Fare::FS_ScopeIsDefined);

    _fareInfo = _memHandle.create<FareInfo>();
    _tcrInfo = _memHandle.create<TariffCrossRefInfo>();
    _fcaInfo = _memHandle.create<FareClassAppInfo>();
    _paxType = _memHandle.create<PaxType>();
  }

  PaxTypeFare* build()
  {
    _fare->setFareInfo(_fareInfo);
    _fare->setTariffCrossRefInfo(_tcrInfo);

    _ptf->fareClassAppInfo() = _fcaInfo;
    _ptf->initialize(_fare, _paxType, _market, _trx);

    return _ptf;
  }

  PaxTypeFareBuilder& withNucFareAmount(MoneyAmount amount)
  {
    _fare->_nucFareAmount = amount;
    return *this;
  }

  PaxTypeFareBuilder& withDirectionality(const Directionality& directionality)
  {
    _fareInfo->directionality() = directionality;
    return *this;
  }

  PaxTypeFareBuilder& publicTariff()
  {
    _tcrInfo->tariffCat() = 2;
    return *this;
  }

  PaxTypeFareBuilder& privateTariff()
  {
    _tcrInfo->tariffCat() = RuleConst::PRIVATE_TARIFF;
    return *this;
  }

  PaxTypeFareBuilder& publishedFare()
  {
    _fare->status().set(Fare::FS_PublishedFare);
    return *this;
  }

  PaxTypeFareBuilder& normalFare()
  {
    _fcaInfo->_pricingCatType = 'N';
    return *this;
  }

  PaxTypeFareBuilder& specialFare()
  {
    _fcaInfo->_pricingCatType = 'S';
    return *this;
  }

  PaxTypeFareBuilder& withFareType(const FareType& fareType)
  {
    _fcaInfo->_fareType = fareType;
    return *this;
  }

  PaxTypeFareBuilder& withPaxType(const PaxTypeCode& ptc)
  {
    _paxType->paxType() = ptc;
    return *this;
  }

  PaxTypeFareBuilder& withOwrt(const Indicator owrt)
  {
    _fareInfo->owrt() = owrt;
    return *this;
  }

  PaxTypeFareBuilder& withAmount(const MoneyAmount amount)
  {
    _fareInfo->fareAmount() = amount;
    return *this;
  }

  PaxTypeFareBuilder& withFareClassCode(std::string fareClassCode)
  {
    _fareInfo->fareClass() = fareClassCode;
    return *this;
  }

  PaxTypeFareBuilder& withCarrier(const CarrierCode& carrier)
  {
    _fareInfo->carrier() = carrier;
    return *this;
  }

private:
  TestMemHandle _memHandle;
  PaxTypeFare* _ptf;
  FareInfo* _fareInfo;
  TariffCrossRefInfo* _tcrInfo;
  Fare* _fare;
  FareClassAppInfo* _fcaInfo;
  PaxType* _paxType;
  FareMarket* _market;
  PricingTrx& _trx;
};
}
