#include <ostream>
namespace Markdown {
  enum Code {
    NO_STYLE    = 0,
    NO_BOLD     = 22,
    BOLD        = 1,
    ITALIC      = 3,
    NO_ITALIC   = 23,
    UNDERLINE   = 4,
    NO_UNDERLINE= 24,
    BLINK       = 5,
    FG_RED      = 31,
    FG_GREEN    = 32,
    FG_YELLOW   = 93,
    FG_BLUE     = 94,
    FG_MAGENTA  = 35,
    FG_CYAN     = 36,
    FG_LIGHT_GREEN = 92,
    FG_LIGHT_RED = 91,
    FG_DEFAULT  = 39,
    BG_RED      = 41,
    BG_GREEN    = 42,
    BG_BLUE     = 44,
    BG_LIGHT_YELLOW = 103,
    BG_DEFAULT  = 49
  };
  class Mod {
    Code code;
    public:
      Mod(Code pCode) : code(pCode) {}
      friend std::ostream& operator<<(std::ostream& os, const Mod& mod){
        return os << "\e[" << mod.code << "m";
    }
  };
}
