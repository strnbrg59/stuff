#include <string>
#include <vector>

class Tokenizer
{
  public:
    explicit Tokenizer(std::string str);
    void operator()(std::vector<std::string>&) const;

  private:
    std::string _rep;
};
