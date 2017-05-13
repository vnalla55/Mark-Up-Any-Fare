//-------------------------------------------------------------------------------
// Copyright 2008, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------

#include "DBAccess/test/SerializationTestAddonCombFareClass.h"

CPPUNIT_TEST_SUITE_REGISTRATION(tse::SerializationTestAddonCombFareClass);

namespace tse
{

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

void
SerializationTestAddonCombFareClass::testDataBlob()
{
  AddonFareClassCombMultiMap original;
  AddonCombFareClassInfo addon;
  AddonCombFareClassInfo::dummyData(addon);
  AddonCombFareClassSpecifiedKey key(addon.fareClass(), addon.owrt());
  std::vector<AddonCombFareClassInfo*> addons(1, &addon);

  original.emplace(key, addons);

  testDataBlobWithOptions(original, DiskCache::DATAFMT_BN2);
}

#else

void
SerializationTestAddonCombFareClass::testDataBlob()
{
  AddonFareClassCombMultiMap original;
  AddonCombFareClassInfo addon;
  AddonCombFareClassInfo::dummyData(addon);
  AddonCombFareClassInfoKey key(
      addon.addonFareClass(), addon.geoAppl(), addon.owrt(), addon.fareClass());

  original.insert(AddonFareClassCombMultiMap::value_type(key, &addon));

  testDataBlobWithOptions(original, DiskCache::DATAFMT_BN2);
}

#endif

}
