#pragma once

#include "Common/TseCodeTypes.h"
#include "DBAccess/BookingCodeExceptionSequence.h"
#include "DBAccess/Flattenizable.h"

#include <boost/function.hpp>
#include <boost/noncopyable.hpp>
#include <boost/thread/locks.hpp>
#include <boost/thread/mutex.hpp>

#include <memory>

namespace tse
{

class BookingCodeExceptionIndex;
class BookingCodeExceptionSequence;
class BookingCodeExceptionSequenceList;

typedef std::shared_ptr<const BookingCodeExceptionIndex> BookingCodeExceptionIndexConstPtr;
typedef boost::function<BookingCodeExceptionIndexConstPtr(const BookingCodeExceptionSequenceList&)>
BookingCodeExceptionIndexFactoryFunc;

typedef std::vector<BookingCodeExceptionSequence*> BookingCodeExceptionSeqVec;
typedef BookingCodeExceptionSeqVec::const_iterator BookingCodeExceptionSequenceListCI;
typedef BookingCodeExceptionSeqVec::iterator BookingCodeExceptionSequenceListI;

class BookingCodeExceptionSequenceList : boost::noncopyable
{
public:
  typedef BookingCodeExceptionSeqVec::iterator iterator;
  typedef BookingCodeExceptionSeqVec::const_iterator const_iterator;
  typedef BookingCodeExceptionSeqVec::value_type value_type;
  typedef BookingCodeExceptionSeqVec::const_reference const_reference;

  BookingCodeExceptionSeqVec& getSequences()
  {
    return _sequences;
  };
  const BookingCodeExceptionSeqVec& getSequences() const
  {
    return _sequences;
  };

  const_iterator begin() const { return _sequences.begin(); }
  iterator begin() { return _sequences.begin(); }
  const_iterator end() const { return _sequences.end(); }
  iterator end() { return _sequences.end(); }

  bool empty() const { return _sequences.empty(); }
  size_t size() const { return _sequences.size(); }
  void reserve(size_t s) { _sequences.reserve(s); }
  void push_back(BookingCodeExceptionSequence* seq)
  {
    _sequences.push_back(seq);
  };

  static void dummyData(BookingCodeExceptionSequenceList& obj) {}

  bool operator==(const BookingCodeExceptionSequenceList& rhs) const
  {
    bool eq = _sequences.size() == rhs._sequences.size();

    for (size_t i = 0; (eq && (i < _sequences.size())); ++i)
    {
      eq = *(_sequences[i]) == *(rhs._sequences[i]);
    }

    return eq;
  }

  void flattenize(Flattenizable::Archive& archive) { FLATTENIZE(archive, _sequences); }

  WBuffer& write(WBuffer& os) const { return convert(os, this); }

  RBuffer& read(RBuffer& is) { return convert(is, this); }

  BookingCodeExceptionIndexConstPtr
  getIndex(const BookingCodeExceptionIndexFactoryFunc& factory) const
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    if (!_index)
      _index = factory(*this);

    return _index;
  }

  VendorCode getVendorCode() const
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    return _vendor;
  }

  BookingCodeExceptionSequenceList& setKey(VendorCode vendor)
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    _vendor = vendor;
    return *this;
  }

  const BookingCodeExceptionSequenceList& setKey(VendorCode vendor) const
  {
    boost::lock_guard<boost::mutex> lock(_mutex);
    _vendor = vendor;
    return *this;
  }

private:
  mutable boost::mutex _mutex;
  BookingCodeExceptionSeqVec _sequences;
  mutable BookingCodeExceptionIndexConstPtr _index;
  mutable VendorCode _vendor;

private:
  template <typename B, typename T>
  static B& convert(B& buffer, T ptr)
  {
    return buffer & ptr->_sequences;
  }
};
}
