#pragma once
#include <vector>

class Inspector
{
public:
	static std::vector< std::vector<unsigned char> > Binary( const unsigned char* data, size_t size );
	static std::vector< std::vector<unsigned char> > Hex( const unsigned char* data, size_t size );

	static void ShowInspector( const std::vector<std::vector<unsigned char>>& data );
};