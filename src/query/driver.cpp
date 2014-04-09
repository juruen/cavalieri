
#include <fstream>
#include <sstream>

#include "driver.h"
#include "scanner.h"

namespace queryparser {

driver::driver(class query_context& query)
    : trace_scanning(false),
      trace_parsing(false),
      query(query)
{
}

bool driver::parse_stream(std::istream& in, const std::string& sname)
{
    streamname = sname;

    Scanner scanner(&in);
    scanner.set_debug(trace_scanning);
    this->lexer = &scanner;

    Parser parser(*this);
    parser.set_debug_level(trace_parsing);
    return (parser.parse() == 0);
}


bool driver::parse_string(const std::string &input, const std::string& sname)
{
    std::istringstream iss(input);
    return parse_stream(iss, sname);
}

void driver::error(const class location& l,
		   const std::string& m)
{
    std::cerr << l << ": " << m << std::endl;
}

void driver::error(const std::string& m)
{
    std::cerr << m << std::endl;
}

}
