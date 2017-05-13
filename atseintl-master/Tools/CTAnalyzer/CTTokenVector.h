#ifndef CT_TOKEN_VECTOR_H
#define CT_TOKEN_VECTOR_H

namespace tse
{

class CTTokenVector
{
 public:

  typedef enum ConstructTokenMode
  {
    ConstructTokenNoSpaces = 0,
    ConstructTokenAddSpaces = 1
  };

 private:
  std::vector<std::string> _tokens;

 public:
  CTTokenVector() : _tokens(0)
  {
  }

  ~CTTokenVector()
  {
  }

  const std::string& getToken(const uint32_t& offset)
  {
    return(_tokens[offset]);
  }

  std::string constructMultiToken(
    const std::pair<uint32_t, uint32_t>& range,
    ConstructTokenMode consTokMode);

  std::vector<std::string>& tokens() { return(_tokens);};
  const std::vector<std::string>& tokens() const { return(_tokens);};
};

}//End namespace tse

#endif //CT_TOKEN_VECTOR_H
