//----------------------------------------------------------------------------
//  File:        Diag867Collector.h
//  Authors:
//  Created:    2015
//
//  Description: Diagnostic 867 formatter for the Commissions Management.
//
//  Updates:
//          date - initials - description.
//
//  Copyright Sabre 2015
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class CommissionLocSegInfo;
class CommissionRuleInfo;
class FarePath;
class FareUsage;
class PricingTrx;

namespace amc
{
class CommissionRuleData;
}

class Diag867Collector : public DiagCollector
{
  friend class Diag867CollectorTest;

public:
  struct DetailedDiag867
  {
    void initialize(PricingTrx& trx);
    bool isDDCC = false; // is contactId present ?
    bool isDDID = false; // is programId present ?
    bool isDDRU = false; // is commission rule Id present?
    bool isDDCX = false; // is validating cxr present?
    bool isDDAMC = false;
    bool isDDINFO = false;
    bool isDDPASSED = false;

    mutable bool nextFu = false;
    mutable bool anyComm = false;

    uint64_t contractId = 0;
    uint64_t programId = 0;
    uint64_t commRuleId = 0;

    LocCode diagBoardCity;
    LocCode diagOffCity;
    std::string diagFareClass;
    std::string diagFareMarket;
    std::string valCxr;
    std::string paxType;
  };

  explicit Diag867Collector(Diagnostic& root)
    : DiagCollector(root), _trx(nullptr) {}

  Diag867Collector() : _trx(nullptr) {}

  void initTrx(PricingTrx& trx);
  void displayFarePathHeader(const FarePath& farePath);
  void printCommissionRuleShortHeader();
  void printCommissionsRuleInfo(const CommissionRuleInfo& cri,
                                CommissionValidationStatus rc);
  void printCommissionsRuleInfoShort(const CommissionRuleInfo& cri,
                                     CommissionValidationStatus rc);
  void printNoSecurityHandShakeForPCC(const FarePath& farePath,
                                      const PseudoCityCode& pcc);
  void printAMCforCat35NotApplicable(const FarePath& farePath);
  void printAMCforASNotApplicable(const FarePath& farePath);
  void printAMCCommandNotSupported(const FarePath& farePath);
  void printAMCforKPandEPRNotApplicable(const FarePath& farePath);
  void printContractNotFound(const CarrierCode& valCxr,
                             const PseudoCityCode& pcc);
  void printValidatingCarrierFCHeader(const CarrierCode& cc);
  void printCommissionFCHeader();
  void printCommissionFC(const PaxTypeFare& paxTypeFare);
  void printCommissionsProgramInfo(const CommissionProgramInfo& cpi,
                                   CommissionValidationStatus rc);
  void printCommissionsProgramInfoShort(const CommissionProgramInfo& cpi,
                                        CommissionValidationStatus rc);

  void printDetailedCommissionContractInfo(const CommissionContractInfo& cci);
  void printCommissionContractInfo(const CommissionContractInfo& cci);
  void printFpCurrencies(const FarePath& fp);
  void printCommonCommissionContractInfo(const CommissionContractInfo& cci);
  void printCommissionContractInfoShort(const CommissionContractInfo& cci);
  bool paxTypeMatched(const FarePath& farePath);
  void printFuClosingLine();
  void printCommissionNotProcessed();
  void printNoCommissionFound();
  void printFuNoCommissionFound();
  void printNoCommissionFound(const PaxTypeFare& paxTypeFare, bool next);
  void printFinalFcCommission(const FarePath& fpath, const FareUsage& fu,
                              MoneyAmount amt, const amc::CommissionRuleData& crd);
  void printFinalAgencyCommission(MoneyAmount amt, bool anyComm);
  void printReuseMessage();

  void printCommissionFC(const FareUsage& fu);

  void printFarePathHeader(const FarePath& fp);

  void printCommissionNotFound(const FareUsage& fu);

  void printFinalAgencyCommission(MoneyAmount amt);

  bool isDetailedDiagnostic() const
  { return _detailedDiag.isDDAMC || _detailedDiag.isDDINFO || _detailedDiag.isDDPASSED; }

  bool isDiagContractMatch(uint64_t ccId) const
  { return _detailedDiag.isDDCC ? _detailedDiag.contractId == ccId : true; }

  bool isDiagProgramMatch(uint64_t progId) const
  { return _detailedDiag.isDDID ? _detailedDiag.programId == progId : true; }

  bool isDiagRuleMatch(uint64_t commRuleId) const
  { return _detailedDiag.isDDRU ? _detailedDiag.commRuleId == commRuleId : true; }

  bool isDiagValCxrMatch(const CarrierCode& cxr) const
  { return _detailedDiag.isDDCX && !_detailedDiag.valCxr.empty() ? cxr == _detailedDiag.valCxr : true; }

  bool isDiagFCMatchFail(const FareUsage& fu) const
  { return (isDetailedDiagnostic() && !diagFcFmMatch(fu)); }

  bool diagFcFmMatch(const FareUsage& fu) const;

  bool isDiagFareClassMatch(const FareUsage& fu) const;
  bool isDiagFareMarketMatch(const FareMarket& fareMarket) const;

  bool isDiagPaxTypeMatch(const FarePath& fp);

  bool isDDAMC() const { return _detailedDiag.isDDAMC; }
  bool isDDINFO() const { return _detailedDiag.isDDINFO;}
  bool isDDPASSED() const { return _detailedDiag.isDDPASSED;}

  void printSingleFcCommission(
      const FarePath& fpath,
      const FareUsage* fu,
      MoneyAmount amt,
      const amc::CommissionRuleData& crd);

  void printCommissionRuleProcess(
        const CommissionRuleInfo& cri,
        CommissionValidationStatus rc);

  void printCommissionProgramProcess(
    const CommissionProgramInfo& cpi,
    CommissionValidationStatus rc);

protected:
  void printAMCNotApplicable();
  void printProgramIdName(const CommissionProgramInfo& cpi);
  void printRuleIdName(const CommissionRuleInfo& cri);

  void displayPaxTypeLine(const PaxTypeCode& paxTypeCode);
  void displayStatus(CommissionValidationStatus status);
  void displayShortFailStatus(CommissionValidationStatus status);

  template <class T>
  void displayAnyString(const std::vector<T>& inputString);
  void printDate(DateTime date);
  void printCommissionTypeIdVerbiage(unsigned commissionTypeId);
  void printCommissionsLocSegInfo(const std::vector<CommissionLocSegInfo*>& logSegs);
  std::string formatPercent(Percent percent) const;
  std::string formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode, bool noCurr=false) const;
  virtual MoneyAmount convertCurrency(const FarePath& fpath,
                                      MoneyAmount sourceAmount,
                                      const CurrencyCode& sourceCurrency,
                                      const CurrencyCode& targetCurrency,
                                      bool doNonIataRounding = false);
private:
  PricingTrx* _trx;
  DetailedDiag867 _detailedDiag;
};

} // namespace tse

