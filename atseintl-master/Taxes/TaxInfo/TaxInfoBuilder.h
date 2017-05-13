//----------------------------------------------------------------------------
//  File:           TaxInfoBuilder.h
//  Authors:        Piotr Lach
//  Created:        12/12/2008
//  Description:    TaxInfoBuilder header file for ATSE V2 PFC Display Project.
//                  Main class for Tax Info Builders.
//
//  Copyright Sabre 2008
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
#ifndef TAX_INFO_BUILDER_H
#define TAX_INFO_BUILDER_H

#include "DataModel/TaxTrx.h"
#include "DataModel/TaxInfoResponse.h"
#include "DBAccess/TaxCodeReg.h"
#include "DBAccess/Customer.h"
#include "DBAccess/Nation.h"
#include "DBAccess/ZoneInfo.h"
#include "Common/Vendor.h"
#include "Common/DateTime.h"

#include <string>
#include <vector>

namespace tse
{
class TaxInfoBuilder
{
public:
  typedef TaxInfoResponse Response;

  static const std::string NO_DATA_ON_FILE;
  static const std::string MISSING_TAX_CODE;
  static const std::string TAX_NOT_FOUND;
  static const std::string MISSING_AIRPORTS;
  static const std::string INVALID_AIRPORT;
  static constexpr char TRUE = 'T';
  static constexpr char FALSE = 'F';
  static constexpr char YES = 'Y';
  static constexpr char NO = 'N';

  TaxInfoBuilder();
  virtual ~TaxInfoBuilder();

  void build(TaxTrx& trx);

  Response& response() { return _response; }
  TaxCode& taxCode() { return _taxCode; }

protected:
  void initTaxDescription(TaxTrx& trx);

  void initTicketingDT(TaxTrx& trx);

  virtual void buildDetails(TaxTrx& trx) = 0;

  virtual bool validateTax(TaxTrx& trx);

  virtual bool validateTravelDate(TaxTrx& trx, TaxCodeReg* taxCodeReg);

  virtual bool validateAirport(TaxTrx& trx);

  std::string dateToString(const DateTime& dt);

  std::string
  amtToString(const MoneyAmount& amount, const CurrencyCode& ccode, const DateTime& ticketingDT);

  std::string percentageToString(MoneyAmount amount);

  TaxCodeReg* taxCodeReg() { return _taxCodeReg; }

  TaxTypeCode& taxType() { return _taxType; }

  NationCode& taxNation() { return _taxNation; }

  std::string& taxDescription() { return _taxDescription; }

  DateTime& ticketingDT() { return _dateTime; }

  std::vector<LocCode>*& airports() { return _airports; }

  bool isAirport(const Loc* loc);

  // DataHandle adapters
  virtual const Loc* getLoc(TaxTrx& trx, const LocCode& locCode);
  virtual const std::vector<TaxCodeReg*>& getTaxCodeReg(TaxTrx& trx);
  virtual const std::vector<Customer*>& getCustomer(TaxTrx& trx, const PseudoCityCode& pcc);
  virtual const Nation* getNation(TaxTrx& trx, const NationCode& countryCode);
  virtual const ZoneInfo* getZone(TaxTrx& trx, const Zone& zone);

  TaxCodeReg* _taxCodeReg;

private:
  TaxCode _taxCode;
  TaxTypeCode _taxType;
  NationCode _taxNation;
  std::string _taxDescription;
  DateTime _dateTime;

  std::vector<LocCode>* _airports;

  Response _response;
  std::vector<TaxCodeReg*> _regListStorage;
};

} // namespace tse
#endif
