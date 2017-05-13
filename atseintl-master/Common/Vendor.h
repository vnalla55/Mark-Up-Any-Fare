#pragma once

//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Common/TseCodeTypes.h"

namespace tse
{

class Vendor
{
public:
  static const VendorCode ATPCO;
  static const VendorCode SITA;
  static const VendorCode SABRE;
  static const VendorCode FMS;
  static const VendorCode WN;
  static const VendorCode POFO;
  static const VendorCode SABD;
  static const VendorCode HIKE;
  static const VendorCode EMPTY;
  static const VendorCode SMFO;

  static const char displayChar(const VendorCode& vendorCode);
};

} // namespace tse

