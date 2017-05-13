#include <xercesc/sax2/DefaultHandler.hpp>

namespace tse
{

class MockXercescAttributes : public xercesc::Attributes
{
  typedef std::basic_string<XMLCh> XMLStringT;

public:
  const XMLCh* getValue(const XMLCh*) const { return 0; }
  const XMLCh* getValue(const XMLCh*, const XMLCh*) const { return 0; }
  const XMLCh* getType(const XMLCh*) const { return 0; }
  const XMLCh* getType(const XMLCh*, const XMLCh*) const { return 0; }
  int getIndex(const XMLCh*) const { return 0; }
  int getIndex(const XMLCh*, const XMLCh*) const { return 0; }
  const XMLCh* getType(unsigned int) const { return 0; }
  const XMLCh* getQName(unsigned int) const { return 0; }
  const XMLCh* getURI(unsigned int) const { return 0; }

  unsigned int getLength() const { return _cont.size(); }
  const XMLCh* getValue(unsigned int i) const { return _cont[i].second.c_str(); }
  const XMLCh* getLocalName(unsigned int i) const { return _cont[i].first.c_str(); }

  void add(const std::string& name, const std::string& value)
  {
    _cont.push_back(std::make_pair(convert(name), convert(value)));
  }

  void clear() {
    _cont.clear();
  }

private:
  static XMLStringT convert(const std::string& str)
  {
    XMLStringT target;
    std::copy(str.begin(), str.end(), std::back_inserter(target));
    return target;
  }

protected:
  std::vector<std::pair<XMLStringT, XMLStringT> > _cont;
};

} //namespace tse

