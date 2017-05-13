//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#ifndef SERIALIZATION_TEST_FLTTRKCNTRYGRP_H
#define SERIALIZATION_TEST_FLTTRKCNTRYGRP_H

#include "DBAccess/test/SerializationTestBase.h"
#include "DBAccess/FltTrkCntryGrp.h"

namespace tse
{
class SerializationTestFltTrkCntryGrp : public SerializationTestBase
{
  CPPUNIT_TEST_SUITE(SerializationTestFltTrkCntryGrp);
  CPPUNIT_TEST(testInfoType<FltTrkCntryGrp>);
  CPPUNIT_TEST(testInfoVectorType<FltTrkCntryGrp>);
  CPPUNIT_TEST_SUITE_END();
};

} // namespace tse

#endif // SERIALIZATION_TEST_FLTTRKCNTRYGRP_H
