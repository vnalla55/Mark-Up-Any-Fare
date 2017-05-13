#pragma once

#include "Diagnostic/Diagnostic.h"

#include <bitset>
#include <deque>
#include <set>
#include <sstream>
#include <string>
#include <vector>

namespace tse
{

class PricingTrx;
class TravelSeg;

class PredicateReturnType
{
public:
  Record3ReturnTypes valid;
  struct Holder
  {
    typedef std::vector<char> Container;
    Container matchedSegments;
  } dummy;

  typedef Holder::Container::iterator iterator;
  typedef Holder::Container::reverse_iterator reverse_iterator;
  typedef Holder::Container::const_iterator const_iterator;

  PredicateReturnType() : valid(FAIL) { dummy.matchedSegments.reserve(8); }

  Holder* operator->()
  {
    return &dummy;
  };
  const Holder* operator->() const
  {
    return &dummy;
  };
};

/// Abstract predicate
class Predicate
{
public:
  Predicate(const std::string& name) : _name(name) {};
  virtual ~Predicate() {}

  /// Method tests if given itinerary satisfied the condition
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) = 0;

  /// Method is used to pretty-print a tree of predicates
  virtual std::string toString(int level = 0) const;

protected:
  std::string printTabs(int level) const;

  std::string _name;
};

/// Predicate tests if itinerary satisfies all predicates it holds.
class And : public Predicate
{
public:
  And() : Predicate("And") {};

  virtual std::string toString(int level = 0) const override;

  void addComponent(Predicate* comp) { _components.push_back(comp); }

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  std::vector<Predicate*> _components;
};

/// Predicate test if itinerary satisfies any of predicates it holds.
class Or : public Predicate
{
public:
  Or() : Predicate("Or") {};

  void addComponent(Predicate* comp) { components.push_back(comp); }

  virtual std::string toString(int level = 0) const override;
  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  std::vector<Predicate*> components;
};

/// Predicate test if itinerary does not satisfy a condition it holds
class Not : public Predicate
{
public:
  Not() : Predicate("Not"), _pred(nullptr) {};

  void initialize(Predicate* pred) { _pred = pred; }

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  Predicate* _pred;
};

/// If-than-else of predicates
class IfThenElse : public Predicate
{
public:
  IfThenElse() : Predicate("IfThenElse"), _if(nullptr), _then(nullptr), _else(nullptr) {};

  void initialize(Predicate* ifPred, Predicate* thenPred)
  {
    _if = ifPred;
    _then = thenPred;
  }

  void addElse(Predicate* elsePred)
  {
    _else = elsePred;
  };

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

protected:
  Predicate* _if;
  Predicate* _then;
  Predicate* _else;
};

/// Tests if the conditions apply to the same segments of itinerary
class SameSegments : public Predicate
{
public:
  SameSegments() : Predicate("SameSegment"), _principal(nullptr) {};

  void initialize(Predicate* princ) { _principal = princ; }

  virtual std::string toString(int level = 0) const override;

  void addComponent(Predicate* comp) { _components.push_back(comp); }

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  std::vector<Predicate*> _components;
  Predicate* _principal;
};

class ContainsInternational : public Predicate
{
public:
  ContainsInternational() : Predicate("ContainsInternational") {};

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;
};

class TextOnly : public Predicate
{
public:
  TextOnly() : Predicate("TextOnly"), _diag(DiagnosticNone), _isUnavailable(false) {};

  void initialize(const std::string& text, DiagnosticTypes diag, bool isUnavailable)
  {
    _text = text;
    _diag = diag;
    _isUnavailable = isUnavailable;
  };

  virtual std::string toString(int level = 0) const override;

  virtual PredicateReturnType
  test(const std::vector<TravelSeg*>& itinerary, const PricingTrx& trx) override;

private:
  std::string _text;
  DiagnosticTypes _diag;
  bool _isUnavailable;
};

} // namespace tse

