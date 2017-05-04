//#include <stdafx.h>
#include <boost/lambda/lambda.hpp>
#include <boost/regex.hpp>
#include <iostream>
#include <iterator>
#include <algorithm>


void example0()
{
	using namespace boost::lambda;
	typedef std::istream_iterator<int> in;

	std::for_each(
		in(std::cin), in(), std::cout << (_1 * 3) << " ");
}


void example1()
{
	std::string line;
	boost::regex pat("^Subject: (Re: |Aw: )*(.*)");

	while (std::cin)
	{
		std::getline(std::cin, line);
		boost::smatch matches;
		if (boost::regex_match(line, matches, pat))
			std::cout << matches[2] << std::endl;
	}
}


int main()
{
	example1();
}