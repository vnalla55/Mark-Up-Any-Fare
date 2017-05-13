//----------------------------------------------------------------------------
//  File:           Reissue.h
//  Description:    Reissue header file for ATSE International Project
//  Created:        6/25/2007
//  Authors:        Dean Van Decker
//
//  Description: This Object will be used for Tax Display functionality.
//
//  Updates:
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
//----------------------------------------------------------------------------

#ifndef REISSUE_H
#define REISSUE_H

#include "Common/TseStringTypes.h"
#include <log4cxx/helpers/objectptr.h>

namespace log4cxx
{
class Logger;
typedef helpers::ObjectPtrT<Logger> LoggerPtr;
}

namespace tse
{
class TaxTrx;
class TaxDisplayItem;

class Reissue
{

public:
  Reissue();
  virtual ~Reissue();

  //-----------------------------------------------------------------------------
  // Copy Constructor must be in Public for Pooled Objects
  //-----------------------------------------------------------------------------
  Reissue(const Reissue& item);
  Reissue& operator=(const Reissue& item);

  void build(TaxTrx& taxTrx, TaxDisplayItem& taxDisplayItem);

  std::string& subCat0() { return _subCat0; }
  const std::string& subCat0() const { return _subCat0; }

  std::string& subCat1() { return _subCat1; }
  const std::string& subCat1() const { return _subCat1; }

  std::string& subCat2() { return _subCat2; }
  const std::string& subCat2() const { return _subCat2; }

  std::string& subCat3() { return _subCat3; }
  const std::string& subCat3() const { return _subCat3; }

  std::string& subCat4() { return _subCat4; }
  const std::string& subCat4() const { return _subCat4; }

protected:
private:
  static log4cxx::LoggerPtr _logger;

  std::string _subCat0; // Tax
  std::string _subCat1; // Refundable
  std::string _subCat2; // Sale Location
  std::string _subCat3; // Validating Carrier
  std::string _subCat4; // Currency
};
} // namespace tse
#endif // REISSUE_H_H
