//  Copyright Sabre 2011

#ifdef CONFIG_HIERARCHY_REFACTOR

#include "Common/Config/DynamicConfigurableFlag.h"
#include "Common/Config/DynamicConfigurableDate.h"
#include "Common/TypeConvert.h"
#include "DataModel/PricingTrx.h"

#include <gtest/gtest.h>
#include "test/include/GtestHelperMacros.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{

namespace
{
const bool DO_NOT_CONFIGURE_CV = false;

const char VALUE_TRUE = 'T';
const char VALUE_FALSE = 'N';

const DateTime DEFAULT_DATE = DateTime(2100, 2, 15);
const DateTime TICKETING_DATE = DateTime(2016, 2, 15);

const std::string TKT_DATE = TypeConvert::valueToString(TICKETING_DATE);
const std::string AFTER_TKT_DATE = TypeConvert::valueToString(DateTime(2016, 3, 15));
const std::string BEFORE_TKT_DATE = TypeConvert::valueToString(DateTime(2016, 1, 15));
}

class ConfigurableValueAdapterTest : public ::testing::Test
{
public:
  void SetUp()
  {
    _dcf = _memHandle(new DynamicConfigurableFlagOn("SECTION", "F_OPT", false));
    _dcd = _memHandle(new DynamicConfigurableEffectiveDate("SECTION", "D_OPT", DEFAULT_DATE));

    _memHandle.create<TestConfigInitializer>();
     Global::configUpdateInProgress() = false;

    _trx = _memHandle(new PricingTrx);

    _trx->setRequest(_memHandle(new PricingRequest));
    _trx->ticketingDate() = TICKETING_DATE;
    _trx->dynamicCfg() = Global::dynamicCfg();
    _trx->setDynamicCfgOverriden(false);
  }

  void TearDown()  { _memHandle.clear(); }

  TestMemHandle _memHandle;
  PricingTrx* _trx;

  DynamicConfigurableFlagOn* _dcf;
  DynamicConfigurableEffectiveDate* _dcd;
};


TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableFlagOn_isValid_FlagOn)
{
  TestConfigInitializer::setValue("F_OPT", VALUE_TRUE, "SECTION");
  ASSERT_TRUE(_dcf->isValid(_trx));
}

TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableFlagOn_isValid_FlagOff)
{
  TestConfigInitializer::setValue("F_OPT", VALUE_FALSE, "SECTION");
  ASSERT_FALSE(_dcf->isValid(_trx));
}

TEST_F(ConfigurableValueAdapterTest,  getConfigMsg_notApplied)
{
  TestConfigInitializer::setValue("F_OPT", VALUE_FALSE, "SECTION");

  ASSERT_STREQ("SECTION/F OPT/N/NOT APPLIED", _dcf->getConfigMsg(_trx, true).c_str());
}

TEST_F(ConfigurableValueAdapterTest, getConfigMsg_Applied)
{
  TestConfigInitializer::setValue("F_OPT", VALUE_TRUE, "SECTION");

  ASSERT_STREQ("SECTION/F OPT/Y/APPLIED", _dcf->getConfigMsg(_trx, false).c_str());
}

TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableEffectiveDate_isValid_NotEfective)
{
  TestConfigInitializer::setValue("D_OPT", AFTER_TKT_DATE, "SECTION");
  ASSERT_FALSE(_dcd->isValid(_trx));
}

TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableEffectiveDate_isValid_Efective)
{
  TestConfigInitializer::setValue("D_OPT", TKT_DATE, "SECTION");

  ASSERT_TRUE(_dcd->isValid(_trx));

  TestConfigInitializer::setValue("D_OPT", BEFORE_TKT_DATE, "SECTION");

  ASSERT_TRUE(_dcd->isValid(_trx));
}

TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableEffectiveDate_getConfigMsg_notApplied)
{
  TestConfigInitializer::setValue("D_OPT", AFTER_TKT_DATE, "SECTION");

  ASSERT_STREQ("SECTION/D OPT/2016-03-15/NOT APPLIED", _dcd->getConfigMsg(_trx, true).c_str());
}

TEST_F(ConfigurableValueAdapterTest, DynamicConfigurableEffectiveDate_getConfigMsg_Applied)
{
  TestConfigInitializer::setValue("D_OPT", BEFORE_TKT_DATE, "SECTION");

  ASSERT_STREQ("SECTION/D OPT/2016-01-15/APPLIED", _dcd->getConfigMsg(_trx, false).c_str());
}

} // ns tse

#endif
