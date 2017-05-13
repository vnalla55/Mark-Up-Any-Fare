
#include "test/include/CppUnitHelperMacros.h"

#include "Common/TseCodeTypes.h"
#include "DBAccess/TaxCodeReg.h"
#include "DataModel/FarePath.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TaxResponse.h"
#include "Taxes/Common/TaxUtility.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
  class TaxUtility_doesUS2Apply : public CppUnit::TestFixture
  {
    CPPUNIT_TEST_SUITE(TaxUtility_doesUS2Apply);
    CPPUNIT_TEST( doesUS2Apply_isYQ_pass );
    CPPUNIT_TEST( doesUS2Apply_noYQ );
    CPPUNIT_TEST( doesUS2Apply_noYQbutNotZeroFare_pass );
    CPPUNIT_TEST( doesUS2Apply_isYQbutZero );
    CPPUNIT_TEST( doesUS2Apply_isYR );
    CPPUNIT_TEST_SUITE_END();

    TestMemHandle _memHandle;
    PricingTrx* _trx;
    FarePath* _farePath;
    TaxResponse* _taxResponse;
    TaxCodeReg* _taxCodeReg;
    PricingRequest* _pricingRequest;
    TaxItem* _taxItem;

  public:

    void
    setUp()
    {
      _memHandle.create<TestConfigInitializer>();
      _trx = _memHandle.create<PricingTrx>();
      _farePath = _memHandle.create<FarePath>();
      _taxResponse = _memHandle.create<TaxResponse>();
      _taxCodeReg = _memHandle.create<TaxCodeReg>();
      _pricingRequest = _memHandle.create<PricingRequest>();
      _trx->setRequest(_pricingRequest);
      _taxResponse->farePath() = _farePath;
      _taxItem = _memHandle.create<TaxItem>();
    }

    void
    tearDown()
    {
      _memHandle.clear();
    }

    void
    doesUS2Apply_isYQ_pass()
    {
      _taxCodeReg->taxOnTaxCode().push_back("YQI");
      _taxItem->taxCode() = "YQI";
      _taxItem->taxAmount() = 108;
      _taxResponse->taxItemVector().push_back(_taxItem);
      _farePath->setTotalNUCAmount(0);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(taxUtil::doesUS2Apply(0, 0, *_trx, *_taxResponse, *_taxCodeReg));
    }

    void
    doesUS2Apply_noYQ()
    {
      _farePath->setTotalNUCAmount(0);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(!taxUtil::doesUS2Apply(0, 0, *_trx, *_taxResponse, *_taxCodeReg));
    }

    void
    doesUS2Apply_noYQbutNotZeroFare_pass()
    {
      _farePath->setTotalNUCAmount(108);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(taxUtil::doesUS2Apply(0, 0, *_trx, *_taxResponse, *_taxCodeReg));
    }

    void
    doesUS2Apply_isYQonDifferentSegment()
    {
      _taxCodeReg->taxOnTaxCode().push_back("YQI");
      _taxItem->taxCode() = "YQI";
      _taxItem->taxAmount() = 108;
      _taxResponse->taxItemVector().push_back(_taxItem);
      _farePath->setTotalNUCAmount(0);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(!taxUtil::doesUS2Apply(1, 1, *_trx, *_taxResponse, *_taxCodeReg));
    }

    void
    doesUS2Apply_isYQbutZero()
    {
      _taxCodeReg->taxOnTaxCode().push_back("YQI");
      _taxItem->taxCode() = "YQI";
      _taxItem->taxAmount() = 0;
      _taxResponse->taxItemVector().push_back(_taxItem);
      _farePath->setTotalNUCAmount(0);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(!taxUtil::doesUS2Apply(0, 0, *_trx, *_taxResponse, *_taxCodeReg));
    }

    void
    doesUS2Apply_isYR()
    {
      _taxCodeReg->taxOnTaxCode().push_back("YRI");
      _taxItem->taxCode() = "YRI";
      _taxItem->taxAmount() = 108;
      _taxResponse->taxItemVector().push_back(_taxItem);
      _farePath->setTotalNUCAmount(0);
      _pricingRequest->equivAmountOverride() = 0;

      CPPUNIT_ASSERT(taxUtil::doesUS2Apply(0, 0, *_trx, *_taxResponse, *_taxCodeReg));
    }
  };

  CPPUNIT_TEST_SUITE_REGISTRATION(TaxUtility_doesUS2Apply);

}
;
