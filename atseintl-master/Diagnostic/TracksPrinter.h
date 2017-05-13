//----------------------------------------------------------------------------
//
//  Copyright Sabre 2011
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

#include <map>
#include <string>
#include <vector>

namespace tse
{

class PricingTrx;
class FarePath;
class FareMarket;
class DiagCollector;
class PaxTypeFare;

class TracksPrinter
{
public:
  TracksPrinter(const PricingTrx& trx, DiagCollector& diag) : _trx(trx), _diag(diag) {}

  void process();

private:
  typedef std::map<std::string, std::vector<PaxTypeFare*> > Collection;
  typedef void (TracksPrinter::*PaxTypeFarePrint)(const PaxTypeFare& ptf);

  std::vector<PaxTypeFare*> getPaxTypeFares(const FarePath& fp) const;

  void print(const FarePath& fp);
  void print(const FareMarket& fm);
  void print(const std::vector<FareMarket*>& fms);
  void print(const Collection& collection);
  void print(const Collection::value_type& value);

  void printByLabel(const std::vector<PaxTypeFare*>& ptfs);
  void print(const std::vector<PaxTypeFare*>& ptfs, PaxTypeFarePrint ptfPrint);
  void printWithTracks(const PaxTypeFare& ptf);
  void printWithoutTracks(const PaxTypeFare& ptf);

  bool isParam(const std::string& key) const;
  bool isParamAll(const std::string& key) const;
  const std::string& getParam(const std::string& key) const;

  void recollect(PaxTypeFare* ptf, Collection& collection) const;
  void recollect(const std::vector<PaxTypeFare*>& ptfs, Collection& collection) const;

  const PricingTrx& _trx;
  DiagCollector& _diag;
};

} // tse

