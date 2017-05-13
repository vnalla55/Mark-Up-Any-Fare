/*---------------------------------------------------------------------------
 *  Copyright Sabre 2016
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
#include "Pricing/SimilarItin/ValidatingCarrierModule.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/PricingUnit.h"
#include "Diagnostic/DiagCollector.h"
#include "Pricing/SimilarItin/Context.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestFallbackUtil.h"
#include "test/include/TestMemHandle.h"

#include <gtest/gtest.h>

#include <vector>

namespace tse
{
FALLBACKVALUE_DECL(fallbackValidatingCxrMultiSp)
namespace similaritin
{
class ValidatingCarrierModuleTest : public testing::Test
{
public:
  void SetUp() override
  {
    _memHandle.create<TestConfigInitializer>();
    _farePath = _memHandle.create<FarePath>();

    _trx = _memHandle.create<PricingTrx>();
    _trx->setOptions(_memHandle.create<PricingOptions>());
    _trx->setRequest(_memHandle.create<PricingRequest>());

    _itin = _memHandle.create<Itin>();
    _childItin = _memHandle.create<Itin>();
    _context = _memHandle.create<Context>(*_trx, *_itin, _diag);
  }

  void pushToItinGSACarriers(std::vector<CarrierCode>& valCarriers, Itin& itin)
  {
    ValidatingCxrDataMap* valCxrDataMap = _memHandle.create<ValidatingCxrDataMap>();
    vcx::ValidatingCxrData* valCarrierData = _memHandle.create<vcx::ValidatingCxrData>();
    ValidatingCxrGSAData* valCxrGSAData = _memHandle.create<ValidatingCxrGSAData>();

    for (auto carrier : valCarriers)
      (*valCxrDataMap)[carrier] = *valCarrierData;
    valCxrGSAData->validatingCarriersData() = *valCxrDataMap;

    SpValidatingCxrGSADataMap* spGsaDataMap = _memHandle.create<SpValidatingCxrGSADataMap>();
    (*spGsaDataMap)["BSP"] = valCxrGSAData;
    itin.spValidatingCxrGsaDataMap() = spGsaDataMap;
    itin.validatingCxrGsaData() = valCxrGSAData;
  }

  void createPaxTypeFareWithValCxrs(FarePath& farePath, std::vector<CarrierCode>& valCarriers)
  {
    PricingUnit* pu = _memHandle.create<PricingUnit>();
    farePath.pricingUnit().push_back(pu);
    FareUsage* fu = _memHandle.create<FareUsage>();

    pu->fareUsage().push_back(fu);
    PaxTypeFare* ptf = _memHandle.create<PaxTypeFare>();
    fu->paxTypeFare() = ptf;
    ptf->validatingCarriers() = (valCarriers);
  }

  void TearDown() override
  {
    _memHandle.clear();
  }

protected:
  TestMemHandle _memHandle;
  DiagCollector _diag;
  Context* _context;
  FarePath* _farePath;
  Itin* _itin;
  Itin* _childItin;
  PricingTrx* _trx;
};

TEST_F(ValidatingCarrierModuleTest, gsaNotApplicableBuildCarrierLists)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(false);
  FarePath cloneFarePath;
  ASSERT_TRUE(module.buildCarrierLists(*_itin, &cloneFarePath));
}

TEST_F(ValidatingCarrierModuleTest, sameValidatingCarriersBuildCarrierLists)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;
  std::vector<CarrierCode> valCarriers = {"DL", "AA", "AB", "BC"};
  cloneFarePath.validatingCarriers() = valCarriers;

  pushToItinGSACarriers(valCarriers, *_itin);
  ASSERT_TRUE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == valCarriers);
  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  // test when multi settlement plan is not active
  ASSERT_TRUE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == valCarriers);
}

TEST_F(ValidatingCarrierModuleTest, fewDifferentValidatingCarriersIntersectBuildCarrierLists)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;
  std::vector<CarrierCode> valCarriersChild = {"DL", "AA", "AB", "BC"};
  cloneFarePath.validatingCarriers() = valCarriersChild;

  std::vector<CarrierCode> valCarriersMother = {"DH", "AA", "AB", "BA"};

  std::vector<CarrierCode> expected = {"AA", "AB"};
  pushToItinGSACarriers(valCarriersMother, *_itin);
  ASSERT_TRUE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == expected);

  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  cloneFarePath.validatingCarriers() = valCarriersChild;
  // test when multi settlement plan is not active
  ASSERT_TRUE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == expected);
}

TEST_F(ValidatingCarrierModuleTest, allDifferentValidatingCarriersBuildCarrierLists)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;
  std::vector<CarrierCode> valCarriersChild = {"AA", "AB", "AC", "AD"};
  cloneFarePath.validatingCarriers() = valCarriersChild;

  std::vector<CarrierCode> valCarriersMother = {"BA", "BB", "BC", "BD"};

  pushToItinGSACarriers(valCarriersMother, *_itin);
  ASSERT_FALSE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers().empty());

  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  cloneFarePath.validatingCarriers() = valCarriersChild;
  // test when multi settlement plan is not active
  ASSERT_FALSE(module.buildCarrierLists(*_itin, &cloneFarePath));
  ASSERT_TRUE(cloneFarePath.validatingCarriers().empty());
}

TEST_F(ValidatingCarrierModuleTest, sameCarriersProcessValidatingCarriers)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;
  std::vector<CarrierCode> valCarriers = {"AA", "AB", "AC", "AD"};
  cloneFarePath.validatingCarriers() = valCarriers;

  pushToItinGSACarriers(valCarriers, *_itin);

  pushToItinGSACarriers(valCarriers, *_childItin);

  bool revalidationReq = false;
  ASSERT_TRUE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));
  ASSERT_TRUE(revalidationReq);

  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  // test when multi settlement plan is not active
  revalidationReq = false;
  ASSERT_TRUE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));
  ASSERT_TRUE(revalidationReq);
}

TEST_F(ValidatingCarrierModuleTest, someDifferentCarriersProcessValidatingCarriers)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;

  std::vector<CarrierCode> valCarriers = {"AA", "AB", "AC", "AD"};
  cloneFarePath.validatingCarriers() = valCarriers;

  std::vector<CarrierCode> childCarriers = {"XX", "AB", "AC", "YY"};
  cloneFarePath.validatingCarriers() = childCarriers;

  std::vector<CarrierCode> ptfCarriers = {"WW", "YY"};

  createPaxTypeFareWithValCxrs(cloneFarePath, ptfCarriers);
  pushToItinGSACarriers(valCarriers, *_itin);
  pushToItinGSACarriers(childCarriers, *_childItin);
  bool revalidationReq = false;

  std::vector<CarrierCode> expected = {"YY"};
  ASSERT_TRUE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));
  ASSERT_TRUE(revalidationReq);
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == expected);

  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  // test when multi settlement plan is not active

  cloneFarePath.validatingCarriers() = childCarriers;
  revalidationReq = false;
  ASSERT_TRUE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));
  ASSERT_TRUE(revalidationReq);
  ASSERT_TRUE(cloneFarePath.validatingCarriers() == expected);
}

TEST_F(ValidatingCarrierModuleTest, differentCarriersProcessValidatingCarriers)
{
  ValidatingCarrierModule module(*_context, nullptr);
  _trx->setValidatingCxrGsaApplicable(true);
  FarePath cloneFarePath;

  std::vector<CarrierCode> valCarriers = {"AA", "AB", "AC", "AD"};
  cloneFarePath.validatingCarriers() = valCarriers;

  std::vector<CarrierCode> childCarriers = {"XX", "AB", "AC", "YY"};
  cloneFarePath.validatingCarriers() = childCarriers;

  std::vector<CarrierCode> ptfCarriers = {"WW", "ZZ"};

  createPaxTypeFareWithValCxrs(cloneFarePath, ptfCarriers);
  pushToItinGSACarriers(valCarriers, *_itin);
  pushToItinGSACarriers(childCarriers, *_childItin);
  bool revalidationReq = false;

  ASSERT_FALSE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));

  fallback::value::fallbackValidatingCxrMultiSp.set(true);
  // test when multi settlement plan is not active

  cloneFarePath.validatingCarriers() = childCarriers;
  ASSERT_FALSE(
      module.processValidatingCarriers(*_itin, cloneFarePath, *_childItin, revalidationReq));
}
} // similaritin
} // tse
