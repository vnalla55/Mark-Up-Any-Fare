#ifndef MOCK_DATA_HANDLE
#define MOCK_DATA_HANDLE

#include "test/DBAccessMock/DataHandleMock.h"
#include "test/include/TestMemHandle.h"
#include <vector>
#include "Common/TseEnums.h"
#include "DBAccess/BankerSellRate.h"
#include "DBAccess/NUCInfo.h"
#include "test/testdata/TestNationFactory.h"
#include "DBAccess/TaxNation.h"
#include <boost/assign/std/vector.hpp>

using namespace boost::assign;

namespace tse
{
class CurrencyDataHandle : public DataHandleMock
{
  TestMemHandle _memHandle;
  BankerSellRate* getBSR(const CurrencyCode& primeCur,
                         const CurrencyCode& cur,
                         ExchRate rate,
                         CurrencyNoDec noDec,
                         Indicator rateType = 'B',
                         std::string agentSine = "FXR")
  {
    BankerSellRate* ret = _memHandle.create<BankerSellRate>();
    ret->primeCur() = primeCur;
    ret->cur() = cur;
    ret->rate() = rate;
    ret->rateNodec() = noDec;
    ret->rateType() = rateType;
    ret->agentSine() = agentSine;
    return ret;
  }
  NUCInfo* createNUC(const CurrencyCode& cur,
                     CarrierCode carrier,
                     CurrencyFactor nucFactor,
                     CurrencyFactor rndFactor,
                     CurrencyNoDec nucNodec,
                     CurrencyNoDec rndNodec,
                     RoundingRule rule)
  {
    NUCInfo* ret = _memHandle.create<NUCInfo>();
    ret->_cur = cur;
    ret->_carrier = carrier;
    ret->_nucFactor = nucFactor;
    ret->_roundingFactor = rndFactor;
    ret->_nucFactorNodec = nucNodec;
    ret->_roundingFactorNodec = rndNodec;
    ret->_roundingRule = rule;
    return ret;
  }

public:
  const std::vector<BankerSellRate*>&
  getBankerSellRate(const CurrencyCode& primeCur, const CurrencyCode& cur, const DateTime& date)
  {
    std::vector<BankerSellRate*>* ret = _memHandle.create<std::vector<BankerSellRate*> >();
    if (primeCur == "BRL" && cur == "CRC")
      ret->push_back(getBSR(primeCur, cur, 292312189, 6));
    else if (primeCur == "BRL" && cur == "CAD")
      ret->push_back(getBSR(primeCur, cur, 586957797, 9));
    else if (primeCur == "BRL" && cur == "GBP")
      ret->push_back(getBSR(primeCur, cur, 378014666, 9));
    else if (primeCur == "BRL" && cur == "USD")
      ret->push_back(getBSR(primeCur, cur, 577834277, 9, 'B', "H-8"));
    else if (primeCur == "JPY" && cur == "BRL")
      ret->push_back(getBSR(primeCur, cur, 1891, 5, 'I'));
    else if (primeCur == "JPY" && cur == "GBP")
      ret->push_back(getBSR(primeCur, cur, 674672783, 11, 'B', "JTA"));
    else if (primeCur == "JPY" && cur == "USD")
      ret->push_back(getBSR(primeCur, cur, 105185652, 10, 'B', "JTA"));
    else if (primeCur == "SOS" && cur == "GBP")
    { /* no SOS-GBP */
    }
    else if (primeCur == "SOS" && cur == "USD")
      ret->push_back(getBSR(primeCur, cur, 65539389, 11));
    else if (primeCur == "USD" && cur == "ARS")
      ret->push_back(getBSR(primeCur, cur, 29, 1, 'B', "H-8"));
    else if (primeCur == "USD" && cur == "CAD")
      ret->push_back(getBSR(primeCur, cur, 100140, 5));
    else if (primeCur == "USD" && cur == "GBP")
      ret->push_back(getBSR(primeCur, cur, 646900, 6));
    else if (primeCur == "USD" && cur == "USD")
      ret->push_back(getBSR(primeCur, cur, 100000, 5, 'B', "LR8"));
    else
      return DataHandleMock::getBankerSellRate(primeCur, cur, date);

    return *ret;
  }

  NUCInfo*
  getNUCFirst(const CurrencyCode& currency, const CarrierCode& carrier, const DateTime& date)
  {
    if (currency == "BRL")
      return createNUC(currency, carrier, 178232000, 1, 8, 2, DOWN);
    else if (currency == "CAD")
      return createNUC(currency, carrier, 102889000, 1, 8, 0, NEAREST);
    else if (currency == "CRC")
      return createNUC(currency, carrier, 1533768, 1, 4, 0, NEAREST);
    else if (currency == "GBP")
      return createNUC(currency, carrier, 66489900, 1, 8, 0, NEAREST);
    else if (currency == "GTQ")
      return createNUC(currency, carrier, 5830660, 0, 6, 0, NONE);
    else if (currency == "INR")
      return createNUC(currency, carrier, 455725000, 5, 7, 0, UP);
    else if (currency == "JPY")
      return createNUC(currency, carrier, 896450000, 100, 7, 0, UP);
    else if (currency == "MXN")
      return createNUC(currency, carrier, 126586700, 1, 7, 2, DOWN);
    else if (currency == "USD")
      return createNUC(currency, carrier, 100000000, 1, 8, 0, NEAREST);
    else if (currency == "SGD")
      return createNUC(currency, carrier, 139907000, 1, 8, 0, UP);
    else if (currency == "MGF")
      return 0;
    return DataHandleMock::getNUCFirst(currency, carrier, date);
  }
  const std::vector<Nation*>& getAllNation(const DateTime& date)
  {
    std::vector<Nation*>* ret = _memHandle.create<std::vector<Nation*> >();
    ret->push_back(TestNationFactory::create("/vobs/atseintl/test/testdata/data/NationGB.xml"));
    ret->push_back(TestNationFactory::create("/vobs/atseintl/test/testdata/data/NationUS.xml"));
    ret->push_back(
        TestNationFactory::create("/vobs/atseintl/test/testdata/data/Nation/NationBR.xml"));
    ret->push_back(
        TestNationFactory::create("/vobs/atseintl/test/testdata/data/Nation/NationES.xml"));
    ret->push_back(
        TestNationFactory::create("/vobs/atseintl/test/testdata/data/Nation/NationJP.xml"));

    return *ret;
  }
  const TaxNation* getTaxNation(const NationCode& nation, const DateTime& date)
  {
    if (nation == "BR")
    {
      TaxNation* ret = _memHandle.create<TaxNation>();
      ret->nation() = nation;
      ret->roundingRule() = NONE;
      ret->taxCollectionInd() = 'A';
      ret->taxFarequoteInd() = 'N';
      ret->taxCodeOrder() += "BR1", "BR2", "BR3", "DU";
      return ret;
    }

    return DataHandleMock::getTaxNation(nation, date);
  }
};
}

#endif
