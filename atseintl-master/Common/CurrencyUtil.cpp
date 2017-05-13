#include "Common/CurrencyUtil.h"

#include "Common/BSRCollectionResults.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/DateTime.h"
#include "Common/FallbackUtil.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "Common/NUCCurrencyConverter.h"
#include "DataModel/PricingRequest.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/Currency.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Nation.h"

#include <boost/lexical_cast.hpp>

#include <cfloat>
#include <iomanip>
#include <string>

namespace tse
{
FALLBACK_DECL(extractReissueExchangeFormatter);

const std::string CurrencyUtil::NULL_CURRENCY;
Logger CurrencyUtil::_logger("atseintl.Common.CurrencyUtil");

//---------------------------------------------------------------------
//
//   @method getNationCurrency
//   @Description Retrieves the nations primary currency in the following
//                order:
//
//                1. National currency
//                2. Pricing currency
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter of nation prime currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
const bool
CurrencyUtil::getNationCurrency(const NationCode& nationCode,
                                CurrencyCode& nationCurrency,
                                const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getNationCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->alternateCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using pricing currency: " << nation->alternateCur());
    nationCurrency = nation->alternateCur();
    return true;
  }

  if (!(nation->primeCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
    nationCurrency = nation->primeCur();
    return true;
  }

  return false;
}

//---------------------------------------------------------------------
//
//   @method getNationalCurrency
//   @Description Retrieves the nations national currency
//
//                1. National currency
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter of nation prime currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
const bool
CurrencyUtil::getNationalCurrency(const NationCode& nationCode,
                                  CurrencyCode& nationCurrency,
                                  const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getNationCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->primeCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
    nationCurrency = nation->primeCur();
    return true;
  }

  return false;
}
//---------------------------------------------------------------------
//
//   @method getConversionCurrency
//   @Description Retrieves the nations conversion currency in the following
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter of nation conversion currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
const bool
CurrencyUtil::getConversionCurrency(const NationCode& nationCode,
                                    CurrencyCode& conversionCurrency,
                                    const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getConversionCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->conversionCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Conversion Currency : " << nation->conversionCur());
    conversionCurrency = nation->conversionCur();
    return true;
  }

  return false;
}

//--------------------------------------------------------------------------------
//   @method isRestricted
//
//   Description: Determines whether or not the override currency is restricted .
//                The Pricing transaction should fail and an exception should be
//                thrown - RESTRICTED_CURRENCY.
//
//   @param PricingTrx         - transaction object
//   @param Loc                - sales location
//   @param CurrencyCode       - WPM override currency code
//
//   @return bool              - true , currency is restricted, else false.
//
//--------------------------------------------------------------------------------
bool
CurrencyUtil::isRestricted(PricingTrx& trx, const Loc* salesLoc, CurrencyCode& overrideCurrency)
{
  LOG4CXX_INFO(_logger, "Entered CurrencyUtil::isRestricted");

  if (overrideCurrency.empty())
    return false;

  const NationCode& salesNationCode = salesLoc->nation();
  const Currency* currency = nullptr;
  currency = trx.dataHandle().getCurrency( overrideCurrency );

  if (currency)
  {
    std::vector<NationCode>::const_iterator resNationsIter = currency->nationRes().begin();
    std::vector<NationCode>::const_iterator resNationsEnd = currency->nationRes().end();

    for (; resNationsIter != resNationsEnd; resNationsIter++)
    {
      const NationCode& restrictedNation = *resNationsIter;

      if (salesNationCode != restrictedNation)
      {
        // Nations RU/XU are considered same nation
        // NATION_EUR is East Ural Russia
        //
        if ((salesNationCode == NATION_USSR && restrictedNation == NATION_EUR) ||
            (restrictedNation == NATION_USSR && salesNationCode == NATION_EUR))
          continue;

        return true;
      }
    }
  }

  LOG4CXX_INFO(_logger, "Leaving CurrencyUtil::isRestricted");

  return false;
}

//---------------------------------------------------------------------
//
//   @method getPricingCurrency
//   @Description Retrieves the nations pricing currency in the following
//                order:
//
//                1. Bank Rate  currency
//                2. Pricing currency
//                3. National currency
//
//   @param NationCode   - nation
//   @param CurrencyCode - return parameter, pricing currency
//
//   @return bool - true - nation currency retrieved , else false
//---------------------------------------------------------------------
const bool
CurrencyUtil::getPricingCurrency(const NationCode& nationCode,
                                 CurrencyCode& pricingCurrency,
                                 const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getPricingCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (UNLIKELY(!nation))
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->conversionCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using bank rate currency: " << nation->conversionCur());
    pricingCurrency = nation->conversionCur();
    return true;
  }

  if (!(nation->alternateCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using pricing currency: " << nation->alternateCur());
    pricingCurrency = nation->alternateCur();
    return true;
  }

  if (LIKELY(!(nation->primeCur().empty())))
  {
    LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
    pricingCurrency = nation->primeCur();
    return true;
  }

  return false;
}

//---------------------------------------------------------------------
//
//   @method getSaleLocOverrideCurrency
//
//   @Description Retrieves the sales location override currency
//                is one.
//
//   @param NationCode   - nation
//
//   @param CurrencyCode - return parameter, sales override location currency
//
//   @return bool - true - currency retrieved , else false
//---------------------------------------------------------------------
const bool
CurrencyUtil::getSaleLocOverrideCurrency(const LocCode& overrideLoc,
                                         CurrencyCode& salesOverrideCurrency,
                                         const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  if (!(overrideLoc.empty()))
  {
    const tse::Loc* loc = dataHandle.getLoc(overrideLoc, ticketingDate);

    if (loc)
    {
      LOG4CXX_DEBUG(_logger, "Sales Override Nation code is: " << loc->nation());
      const NationCode& nationCode = loc->nation();
      bool nationRC = getNationCurrency(nationCode, salesOverrideCurrency, ticketingDate);
      return nationRC;
    }
  }

  return false;
}

bool
CurrencyUtil::formatDoubleWithNoDec(const double value, std::string& target, int noDecimals)
{
  bool ret = true;
  std::string tmpStr = boost::lexical_cast<std::string>(value);
  std::string::size_type pos = tmpStr.find(".");

  if (pos != std::string::npos) // lint !e530
    pos += (noDecimals + 1);
  else
    ret = false;

  target = tmpStr.substr(0, pos);
  return ret;
}

void
CurrencyUtil::formatDouble(const double& value, std::string& target, int noDecimals,
    bool removePeriodWhenInt)
{
  if (noDecimals > 0)
    formatDoubleWithNoDec(value, target, noDecimals);
  else if(removePeriodWhenInt)
    formatDoubleWithNoDec(value, target, -1);
  else
  {
    std::string tmpStr = boost::lexical_cast<std::string>(value);

    target = tmpStr.substr(0, tmpStr.size());
  }
}

const bool
CurrencyUtil::getDomesticPricingCurrency(const NationCode& nationCode,
                                         CurrencyCode& pricingCurrency,
                                         const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getPricingCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->alternateCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using pricing currency: " << nation->alternateCur());
    pricingCurrency = nation->alternateCur();
    return true;
  }

  if (!(nation->primeCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
    pricingCurrency = nation->primeCur();
    return true;
  }

  if (!(nation->conversionCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using bank rate currency: " << nation->conversionCur());
    pricingCurrency = nation->conversionCur();
    return true;
  }

  return false;
}

const bool
CurrencyUtil::getInternationalPricingCurrency(const NationCode& nationCode,
                                              CurrencyCode& pricingCurrency,
                                              const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  LOG4CXX_INFO(_logger, " Entered CurrencyUtil::getPricingCurrency()");

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->conversionCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using bank rate currency: " << nation->conversionCur());
    pricingCurrency = nation->conversionCur();
    return true;
  }

  if (!(nation->alternateCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using pricing currency: " << nation->alternateCur());
    pricingCurrency = nation->alternateCur();
    return true;
  }

  if (!(nation->primeCur().empty()))
  {
    LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
    pricingCurrency = nation->primeCur();
    return true;
  }

  return false;
}

std::string
CurrencyUtil::toString(const MoneyAmount amt, const CurrencyNoDec noDec)
{
  std::string target;

  try
  {
    target = boost::lexical_cast<std::string>(amt);
    size_t pos = target.find(".");

    if (noDec > 0)
    {
      if (pos != std::string::npos) // lint !e530
        pos += (noDec + 1);
    }
    else
    {
      if (pos != std::string::npos) // lint !e530
        pos += (noDec);
    }

    target = target.substr(0, pos);
  }
  catch (boost::bad_lexical_cast& bc)
  {
    LOG4CXX_ERROR(_logger, "Boost Bad Lexical cast: " << bc.what());
  }

  return target;
}

//-----------------------------------------------------------------------------
//   @method getControllingNationDesc
//
//   Description: Retrieves  Controlling Nation Description
//
//   @param   CurrencyCode  - key to Currency cache
//
//   @return  string        - description controlling nation
//
//-----------------------------------------------------------------------------
const std::string&
CurrencyUtil::getControllingNationDesc(Trx& trx, const CurrencyCode& code, const DateTime& dcDate)
{
  LOG4CXX_INFO(_logger, "Entering CurrencyUtil::getControllingNationDesc");
  LOG4CXX_INFO(_logger, "Currency : " << code);

  const Currency* currency = nullptr;
  currency = trx.dataHandle().getCurrency( code );

  if (!currency)
  {
    LOG4CXX_DEBUG(_logger, "Currency cache pointer returned NULL");
    return NULL_CURRENCY;
  }

  LOG4CXX_DEBUG(_logger, "Country description : " << currency->controllingEntityDesc());
  LOG4CXX_INFO(_logger, "Leaving CurrencyUtil::getControllingNationDesc");

  return currency->controllingEntityDesc();
}

//-----------------------------------------------------------------------------
//   @method getControllingNationCode
//
//   Description: Retrieves the controlling Nation
//
//   @param   string         - Country Description
//   @param   NationCode     - return param
//   @param   bool           - found nation
//   @param   bool           - found nation's national currency
//   @param   NationCode     - return param
//
//   @return  void
//-----------------------------------------------------------------------------
void
CurrencyUtil::getControllingNationCode(Trx& trx,
                                       const std::string& countryDescription,
                                       NationCode& nationCode,
                                       bool& foundNation,
                                       bool& foundNationalCurrency,
                                       NationCode& nationWithMatchingNationalCurrency,
                                       const DateTime& dcDate,
                                       const CurrencyCode& targetCurrency)
{
  LOG4CXX_INFO(_logger, "Entered CurrencyUtil::getControllingNation");
  LOG4CXX_DEBUG(_logger, "DC DATE: " << dcDate.toSimpleString());
  LOG4CXX_DEBUG(_logger, "NATION CODE: " << nationCode);
  LOG4CXX_DEBUG(_logger, "TRX TARGET CUR: " << targetCurrency);

  const std::vector<Nation*> allNationList = trx.dataHandle().getAllNation(dcDate);

  std::vector<Nation*>::const_iterator nationsListIter = allNationList.begin();
  std::vector<Nation*>::const_iterator nationsListEnd = allNationList.end();

  if (UNLIKELY(allNationList.empty()))
  {
    LOG4CXX_ERROR(_logger, "Nations vector is empty");
    return;
  }

  for (; nationsListIter != nationsListEnd; nationsListIter++)
  {
    const Nation* nation = *nationsListIter;

    if (!(targetCurrency.empty()) && (!nation->primeCur().empty()))
    {
      if (targetCurrency == nation->primeCur())
      {
        if (foundNationalCurrency == false)
        {
          nationWithMatchingNationalCurrency = nation->nation();
          foundNationalCurrency = true;
          LOG4CXX_DEBUG(_logger, "FOUND NATION WITH MATCHING NATIONAL CUR: " << nation->nation());
        }
      }
    }

    if (countryDescription == nation->description())
    {
      nationCode = nation->nation();
      foundNation = true;
      break;
    }
  }

  LOG4CXX_INFO(_logger, "Leaving CurrencyUtil::getControllingNation");
}

//-----------------------------------------------------------------------------
//    @method matchNationCurrency
//
//    @Description Determines whether or not the source currency
//                 matches the nation currency.
//
//     @param NationCode   - nation
//
//     @param CurrencyCode - input parameter, source currency
//
//     @return bool - true - nation currency matched , else false
//
//-----------------------------------------------------------------------------
const bool
CurrencyUtil::matchNationCurrency(const NationCode& nationCode,
                                  const CurrencyCode& sourceCurrency,
                                  const DateTime& ticketingDate)
{
  DataHandle dataHandle(ticketingDate);

  const tse::Nation* nation = dataHandle.getNation(nationCode, ticketingDate);

  if (!nation)
  {
    LOG4CXX_DEBUG(_logger, "Unable to retrieve Nation from cache " << nationCode);
    return false;
  }

  if (!(nation->alternateCur().empty()))
  {
    if (nation->alternateCur() == sourceCurrency)
    {
      LOG4CXX_DEBUG(_logger, "Using pricing currency: " << nation->alternateCur());
      return true;
    }
  }

  if (LIKELY(!(nation->primeCur().empty())))
  {
    if (nation->primeCur() == sourceCurrency)
    {
      LOG4CXX_DEBUG(_logger, "Using national currency: " << nation->primeCur());
      return true;
    }
  }

  if (!(nation->conversionCur().empty()))
  {
    if (nation->conversionCur() == sourceCurrency)
    {
      LOG4CXX_DEBUG(_logger, "Using bank rate currency: " << nation->conversionCur());
      return true;
    }
  }

  return false;
}

void
CurrencyUtil::truncateNUCAmount(MoneyAmount& amount)
{
  LOG4CXX_DEBUG(_logger, "amount: " << amount);
  int e;
  double mantissa = frexp(amount, &e);
  mantissa += DBL_EPSILON;
  amount = trunc(100 * (ldexp(mantissa, e))) / 100;
  LOG4CXX_DEBUG(_logger, "truncated amount: " << amount);
}

void
CurrencyUtil::truncateNonNUCAmount(MoneyAmount& amount, unsigned noDec)
{
  LOG4CXX_DEBUG(_logger, "amount: " << amount);
  int e;
  double mantissa = frexp(amount, &e);
  double powerOf10 = pow(10, noDec);
  mantissa += DBL_EPSILON;
  amount = trunc(powerOf10 * (ldexp(mantissa, e))) / powerOf10;
  LOG4CXX_DEBUG(_logger, "truncated amount: " << amount);
}

void
CurrencyUtil::halveNUCAmount(MoneyAmount& amount)
{
  LOG4CXX_DEBUG(_logger, "amount: " << amount);
  static const double EPS = 0.00001;
  amount = trunc(amount * 50 + EPS) / 100;
  LOG4CXX_DEBUG(_logger, "halved and truncated amount: " << amount);
}

MoneyAmount
CurrencyUtil::convertMoneyAmount(const MoneyAmount& sourceAmount,
                                 const CurrencyCode& sourceCurrency,
                                 const CurrencyCode& targetCurrency,
                                 const DateTime& conversionDate,
                                 PricingTrx& trx,
                                 ConversionType type)
{
  if (sourceCurrency == targetCurrency)
  {
    return sourceAmount;
  }

  if (targetCurrency == NUC)
  {
    return CurrencyUtil::getMoneyAmountInNUC(sourceAmount, sourceCurrency,
        conversionDate, trx, type);
  }

  const Money sourceMoney(sourceAmount, sourceCurrency);
  Money targetMoney(targetCurrency);
  CurrencyConversionRequest request(targetMoney,
                                    sourceMoney,
                                    conversionDate,
                                    *(trx.getRequest()),
                                    trx.dataHandle(),
                                    false,
                                    type,
                                    false,
                                    trx.getOptions(),
                                    false,
                                    true,
                                    &trx);
  BSRCollectionResults bsrResults;
  //     bsrResults.collect() = false;
  // * Is there a reason to collect the results when this object is
  // * local and doesn't invoke messaging, unless collect() is true?

  bsrResults.collect() = true;

  try
  {
    BSRCurrencyConverter bsrConverter;
    if (!bsrConverter.convert(request, &bsrResults))
    {
      LOG4CXX_FATAL(_logger, "BSR RATE FROM " << sourceCurrency << " TO "
                              << targetCurrency << " NOT AVAILABLE");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_FATAL(_logger, "BSR Converter exception: " << ex.what());
    throw ex;
  }

  return targetMoney.value();
}

MoneyAmount
CurrencyUtil::convertMoneyAmount_OLD(const MoneyAmount& sourceAmount,
                                 const CurrencyCode& sourceCurrency,
                                 const CurrencyCode& targetCurrency,
                                 PricingTrx& _trx,
                                 ConversionType type)
{
  if (sourceCurrency == targetCurrency)
  {
    return sourceAmount;
  }

  if (targetCurrency == NUC)
  {
    return CurrencyUtil::getMoneyAmountInNUC(sourceAmount, sourceCurrency,
                                             _trx.ticketingDate(), _trx, type);
  }

  const Money sourceMoney(sourceAmount, sourceCurrency);
  Money targetMoney(targetCurrency);
  CurrencyConversionRequest request(targetMoney,
                                    sourceMoney,
                                    _trx.ticketingDate(),
                                    *(_trx.getRequest()),
                                    _trx.dataHandle(),
                                    false,
                                    type,
                                    false,
                                    _trx.getOptions(),
                                    false,
                                    true,
                                    &_trx);
  BSRCollectionResults bsrResults;
  //     bsrResults.collect() = false;
  // * Is there a reason to collect the results when this object is
  // * local and doesn't invoke messaging, unless collect() is true?

  bsrResults.collect() = true;

  try
  {
    BSRCurrencyConverter bsrConverter;
    if (!bsrConverter.convert(request, &bsrResults))
    {
      LOG4CXX_FATAL(_logger, "BSR RATE FROM " << sourceCurrency << " TO "
                              << targetCurrency << " NOT AVAILABLE");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_FATAL(_logger, "BSR Converter exception: " << ex.what());
    throw ex;
  }

  return targetMoney.value();

}

MoneyAmount
CurrencyUtil::convertMoneyAmount(const MoneyAmount& sourceAmount,
                                 const CurrencyCode& sourceCurrency,
                                 const CurrencyCode& targetCurrency,
                                 PricingTrx& trx,
                                 ConversionType type)
{
  if (fallback::extractReissueExchangeFormatter(&trx))
    return convertMoneyAmount_OLD(sourceAmount, sourceCurrency, targetCurrency,
        trx, type);
  else
    return convertMoneyAmount(sourceAmount, sourceCurrency, targetCurrency,
        trx.ticketingDate(), trx, type);
}

MoneyAmount
CurrencyUtil::getMoneyAmountInNUC(const MoneyAmount& sourceAmount,
                                  const CurrencyCode& sourceCurrency,
                                  const DateTime& conversionDate,
                                  PricingTrx& trx,
                                  ConversionType type)
{
  const Money sourceMoney(sourceAmount, sourceCurrency);
  Money targetMoney(NUC);
  CurrencyConversionRequest request(targetMoney,
                                   sourceMoney,
                                   conversionDate,
                                   *(trx.getRequest()),
                                   trx.dataHandle(),
                                   false,
                                   type,
                                   false,
                                   trx.getOptions(),
                                   false,
                                   true,
                                   &trx);

  BSRCollectionResults bsrResults;

  try
  {
    NUCCurrencyConverter ncc;
    if (!ncc.convert(request, &bsrResults))
    {
      LOG4CXX_FATAL(_logger, "RATE FROM " << sourceCurrency << " TO NUC" << " NOT AVAILABLE");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    LOG4CXX_FATAL(_logger, "BSR Converter exception: " << ex.what());
    throw ex;
  }

  return targetMoney.value();
}

void
CurrencyUtil::validateCurrencyCode(const PricingTrx& trx, const CurrencyCode& cur)
{
  if (!cur.empty() && nullptr == trx.dataHandle().getCurrency(cur))
  {
    throw ErrorResponseException(ErrorResponseException::INVALID_FORMAT,
        "INVALID CURRENCY CODE");
  }
}

} //tse

