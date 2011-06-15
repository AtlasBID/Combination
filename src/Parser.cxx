//
// Parser.cxx
//
// ATLAS - is currently using spirit 2.2
//

#include "Combination/Parser.h"

//
// All the boost libraries
//
#include <boost/config/warning_disable.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/fusion/include/adapt_struct.hpp>
#include <boost/fusion/include/io.hpp>


#include <iostream>
#include <string>
#include <vector>


namespace qi = boost::spirit::qi;
namespace ascii = boost::spirit::ascii;

///////////////
// The parser code itself
//
//////////////

// A systematic error
struct SystematicError
{
  std::string name;
  double val;
};

//
// Now, in order to make efficient use of boost with this
// thing we need to attribute the structs. We do that here.
// Note: this has to be done in the global namespace/scope!
//
BOOST_FUSION_ADAPT_STRUCT(
			  SystematicError,
			  (std::string, name)
			  (double, val)
			  )

//
// Parse the systematic error
//

template <typename Iterator>
struct SystematicErrorParser : qi::grammar<Iterator, SystematicError(), ascii::space_type>
{
  SystematicErrorParser() : SystematicErrorParser::base_type(start) {
    using ascii::char_;
    using qi::lexeme;
    using qi::lit;
    using qi::double_;

    name_string %= lexeme[+(char_ - ',' - '"')];

    start = lit("sys")
      >> '('
      >> name_string >> ','
      >> double_
      >> ')'
      ;
  }

    qi::rule<Iterator, std::string(), ascii::space_type> name_string;
    qi::rule<Iterator, SystematicError(), ascii::space_type> start;
};

///////////////////////////////////////


using namespace std;

void ParseMe()
{
  string input = "sys( JES, 22%)";
  
  SystematicErrorParser<string::const_iterator> sErrP;
  SystematicError err;
  using ascii::space;

  string::const_iterator iter = input.begin();
  string::const_iterator end = input.end();
  bool r = phrase_parse(iter, end, sErrP, space, err);

  if (!r) {
    cout << "Error parsing the input!" << endl;
  } else {
    cout << "Got a parse" << endl;
    cout << "  name: " << err.name << endl;
    cout << "  value " << err.val << endl;
  }

  cout << "hi" << endl;
}
#ifdef notyet

namespace client
{
    namespace qi = boost::spirit::qi;
    namespace ascii = boost::spirit::ascii;

    ///////////////////////////////////////////////////////////////////////////
    //  Our number list parser
    ///////////////////////////////////////////////////////////////////////////
    //[tutorial_numlist1
    template <typename Iterator>
    bool parse_numbers(Iterator first, Iterator last)
    {
        using qi::double_;
        using qi::phrase_parse;
        using ascii::space;

        bool r = phrase_parse(
            first,                          /*< start iterator >*/
            last,                           /*< end iterator >*/
            double_ >> *(',' >> double_),   /*< the parser >*/
            space                           /*< the skip-parser >*/
        );
        if (first != last) // fail if we did not get a full match
            return false;
        return r;
    }
    //]
}

////////////////////////////////////////////////////////////////////////////
//  Main program
////////////////////////////////////////////////////////////////////////////
int
main()
{
    std::cout << "/////////////////////////////////////////////////////////\n\n";
    std::cout << "\t\tA comma separated list parser for Spirit...\n\n";
    std::cout << "/////////////////////////////////////////////////////////\n\n";

    std::cout << "Give me a comma separated list of numbers.\n";
    std::cout << "Type [q or Q] to quit\n\n";

    std::string str;
    while (getline(std::cin, str))
    {
        if (str.empty() || str[0] == 'q' || str[0] == 'Q')
            break;

        if (client::parse_numbers(str.begin(), str.end()))
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing succeeded\n";
            std::cout << str << " Parses OK: " << std::endl;
        }
        else
        {
            std::cout << "-------------------------\n";
            std::cout << "Parsing failed\n";
            std::cout << "-------------------------\n";
        }
    }

    std::cout << "Bye... :-) \n\n";
    return 0;
}
#endif
