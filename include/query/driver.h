#ifndef QUERY_DRIVER_H
#define QUERY_DRIVER_H

#include <string>
#include <vector>

class query_context;

namespace queryparser {

class driver
{
public:
    driver(query_context & query);

    bool trace_scanning;

    bool trace_parsing;

    std::string streamname;

    bool parse_stream(std::istream& in,
		      const std::string& sname = "stream input");

    bool parse_string(const std::string& input,
		      const std::string& sname = "string stream");

    void error(const class location& l, const std::string& m);

    void error(const std::string& m);

    class Scanner* lexer;

    class query_context & query;
};

}

#endif
