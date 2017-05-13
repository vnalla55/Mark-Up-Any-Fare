// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include <boost/format.hpp>

#include "Common/Timestamp.h"
#include "DataModel/Common/Types.h"
#include "Diagnostic/AtpcoDiagnostic.h"

namespace tax
{

class Services;
class BusinessRulesContainer;

class DBDiagnostic : public AtpcoDiagnostic
{
public:
  static const uint32_t NUMBER;

  DBDiagnostic(Services& services,
               const boost::ptr_vector<Parameter>& parameters,
               const type::Timestamp& ticketingDate);
  virtual ~DBDiagnostic();

private:
  static const tax::type::TaxPointTag EMPTY_TAG;

  class Filter
  {
  public:
    Filter();
    bool applyParameters(const boost::ptr_vector<Parameter>& parameters);
    bool nationMatched(const type::Nation& nation_) const;
    bool taxCodeMatched(const type::TaxCode& taxCode_) const;
    bool taxTypeMatched(const type::TaxType& taxType_) const;
    bool seqNoMatched(const type::SeqNo& seqNo_) const;

    const type::Nation& getNation() const;
    const type::TaxPointTag& getTaxPointTag() const;
    bool isDescriptionEnabled() const;

  private:
    type::Nation _nation;
    type::TaxPointTag _taxPointTag;
    type::TaxCode _taxCode;
    type::TaxType _taxType;
    type::SeqNo _seqNo;
    bool _descriptionEnabled;
  };

  // Diagnostic interface
  virtual void runAll() override;
  virtual void printHeader() override;
  virtual void applyParameters() override;

  void printFiltered(const tax::type::TaxPointTag& taxPointTag);
  void printRecordData(const BusinessRulesContainer& rulesRecord);
  void printRecordDescription(const BusinessRulesContainer& rulesRecord);
  void printRecord(const BusinessRulesContainer& rulesRecord, bool showDescription);

  void printHelp();
  void printAllBusinessRules();
  void printRecords();

  void printRecordsContainers(
      const std::vector<std::shared_ptr<BusinessRulesContainer>>& rulesContainers);
  Printer<DBDiagnostic, &DBDiagnostic::printAllBusinessRules> _brPrinter;
  Printer<DBDiagnostic, &DBDiagnostic::printHelp> _helpPrinter;
  Printer<DBDiagnostic, &DBDiagnostic::printRecords> _recordsPrinter;

  const boost::ptr_vector<Parameter>& _parameters;
  type::Timestamp _ticketingDate;
  Services& _services;
  Filter _filter;
  std::vector<tax::type::TaxPointTag> _availableTags;

  boost::format _formatter;
  std::string _columnNames;
  boost::format _brIdNameformatter;
  std::string _brIdNameColumnNames;
};
}

