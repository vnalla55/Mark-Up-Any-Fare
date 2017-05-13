//-----------------------------------------------------------------------------
//
//  File:     Diag212CollectorFD.h
//
//  Author :  Kul Shekhar
//
//  Copyright Sabre 2005
//
//          The copyright of the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the agreement/contract
//          under which the program(s) have been supplied.
//
//-----------------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Diagnostic/DiagCollector.h"

#include <map>
#include <set>

namespace tse
{
// Diag212Collector is the diagnostic for currency
class Diag212CollectorFD : public DiagCollector
{
public:
  explicit Diag212CollectorFD(Diagnostic& root)
    : DiagCollector(root),
      _mainPcc(""),
      _agentPcc(""),
      _tjrGroup(0),
      _isMainPcc(false),
      _isAgntPcc(false),
      _isTjrGrNo(false),
      _fDataMatch(false),
      _fFinishProc(false),
      _fMatchFound(PS_MATCH_NOT_FOUND),
      _fHeaderDisplayed(false),
      _fdTrx(nullptr)
  {
  }

  Diag212CollectorFD()
    : _mainPcc(""),
      _agentPcc(""),
      _tjrGroup(0),
      _isMainPcc(false),
      _isAgntPcc(false),
      _isTjrGrNo(false),
      _fDataMatch(false),
      _fFinishProc(false),
      _fMatchFound(PS_MATCH_NOT_FOUND),
      _fHeaderDisplayed(false),
      _fdTrx(nullptr)
  {
  }

  virtual void initParam(Diagnostic& root) override;

  void init(FareDisplayTrx& trx);
  void printHeader() override;
  void printFooter();
  void isMainPcc(bool is) { _isMainPcc = is; }
  void isAgentPcc(bool is) { _isAgntPcc = is; }
  void isTjrGrNo(bool is) { _isTjrGrNo = is; }
  void printAgentPccSuppFares(const std::vector<const FDSuppressFare*>& fares, bool fMatch);
  void printMainPccSuppFares(const std::vector<const FDSuppressFare*>& fares, bool fMatch);
  void printTjrGroupNoSuppFares(const std::vector<const FDSuppressFare*>& fares, bool fMatch);

private:
  enum RecordSource
  {
    SFS_AGENTPCC,
    SFS_MAINPCC,
    SFS_TJRGROUPNO
  };
  enum ProcessingState
  {
    PS_MATCH_NOT_FOUND,
    PS_MATCH_FOUND,
    PS_NOT_PROCESSED
  };
  enum MarketDirectionality
  {
    MD_WORLDWIDE,
    MD_BETWEEN,
    MD_FROM,
    MD_WITHIN
  };

  struct SuppressFareMarket
  {
    MarketDirectionality directionality;
    LocKey loc1;
    LocKey loc2;

    bool operator<(const SuppressFareMarket& second) const
    {
      return directionality < second.directionality || loc1.loc() < second.loc1.loc() ||
             loc1.locType() < second.loc1.locType() || loc2.loc() < second.loc2.loc() ||
             loc1.locType() < second.loc1.locType();
    }
  };

  void
  printSuppresFares(RecordSource src, const std::vector<const FDSuppressFare*>& fares, bool fMatch);
  void addSuppressFares(const std::vector<const FDSuppressFare*>& fares);
  void printHeader(RecordSource src);
  void printSuppressFares();
  void printSuppressFare(const SuppressFareMarket& market, const std::set<CarrierCode>& carriers);
  void printLoc(const LocKey& loc);

  std::map<SuppressFareMarket, std::set<CarrierCode> > _multiCarrierSuppressFares;
  std::map<SuppressFareMarket, std::set<CarrierCode> > _singleCarrierSuppressFares;
  PseudoCityCode _mainPcc;
  PseudoCityCode _agentPcc;
  TJRGroup _tjrGroup;
  bool _isMainPcc;
  bool _isAgntPcc;
  bool _isTjrGrNo;
  bool _fDataMatch;
  bool _fFinishProc;
  ProcessingState _fMatchFound;
  bool _fHeaderDisplayed;
  FareDisplayTrx* _fdTrx;
};

} // namespace tse

