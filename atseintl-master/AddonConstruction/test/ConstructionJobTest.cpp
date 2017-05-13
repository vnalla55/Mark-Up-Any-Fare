//-------------------------------------------------------------------
//
//  File:        ConstructionJobTest.cpp
//  Created:     Junl 7, 2016
//  Authors:     Luca Stoppa
//
//  Description: Testcase ACDialogCollector refactoring.
//
//  Copyright Sabre 2004
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

#include <gtest/gtest.h>
#include "Diagnostic/Diag251Collector.h"
#include "Diagnostic/Diag252Collector.h"
#include "Diagnostic/Diag253Collector.h"
#include "Diagnostic/Diag254Collector.h"
#include "Diagnostic/Diag255Collector.h"
#include "Diagnostic/Diag257Collector.h"
#include "Diagnostic/Diag259Collector.h"
#include "AddonConstruction/ConstructionJob.h"
#include "DataModel/Agent.h"
#include "DataModel/Billing.h"
#include "DataModel/PricingTrx.h"
#include "test/include/TestConfigInitializer.h"
#include "test/include/TestMemHandle.h"

namespace tse
{
TEST(TestDiagnostic, Diagnostic251CollectorTestCase)
{
  TestMemHandle mh;
  mh.create<TestConfigInitializer>();

  PricingTrx* trx = mh.create<PricingTrx>();
  trx->billing() = mh.create<Billing>();
  trx->setOptions(mh.create<PricingOptions>());

  PricingRequest* req = mh.create<PricingRequest>();
  req->ticketingAgent() = mh.create<Agent>();
  trx->setRequest(req);

  Diagnostic& d = trx->diagnostic();
  d.diagnosticType() = Diagnostic251;
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  SpecifiedFareCache cache;
  ConstructionJob* cj = mh.insert(new ConstructionJob(*trx,
                                                       DateTime{2016, 01, 01},
                                                       DateTime{2016, 01, 01},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       GlobalDirection{},
                                                       false,
                                                       &cache));
#else
  ConstructionJob* cj = mh.insert(new ConstructionJob(*trx,
                                                       DateTime{2016, 01, 01},
                                                       DateTime{2016, 01, 01},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       LocCode{"RIO"},
                                                       false));

#endif
  cj->createDiagCollector();
  ASSERT_TRUE(cj->trx().diagnostic().diagnosticType() == Diagnostic251);

  ASSERT_NE(cj->diagnostic<Diag251Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag251Collector>(), cj->diag251());

  ASSERT_EQ(cj->diagnostic<Diag252Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag252Collector>(), cj->diag252());

  ASSERT_EQ(cj->diagnostic<Diag253Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag253Collector>(), cj->diag253());

  ASSERT_EQ(cj->diagnostic<Diag254Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag254Collector>(), cj->diag254());

  ASSERT_EQ(cj->diagnostic<Diag255Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag255Collector>(), cj->diag255());

  ASSERT_EQ(cj->diagnostic<Diag257Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag257Collector>(), cj->diag257());

  ASSERT_EQ(cj->diagnostic<Diag259Collector>(), nullptr);
  ASSERT_EQ(cj->diagnostic<Diag259Collector>(), cj->diag259());
}
}
