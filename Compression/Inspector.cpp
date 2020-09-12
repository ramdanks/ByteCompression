#include "Inspector.h"
#include <stdio.h>

#define BINARY_SIZE 8
#define HEX_SIZE 2

constexpr char MaskBit[9] = { 0x00, 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80 };

std::vector<std::vector<unsigned char>> Inspector::Binary( const unsigned char* data, size_t size )
{
	std::vector< std::vector<unsigned char> > inspect;
	inspect.resize( size );	

	for ( size_t i = 0; i < size; i++ )
	{
		inspect[i].resize( BINARY_SIZE );
		for ( size_t j = 0; j < BINARY_SIZE; j++ )
		{
			if ( data[i] & MaskBit[j + 1] )
				inspect[i][j] = true;
		}
	}
	return inspect;
}

std::vector<std::vector<unsigned char>> Inspector::Hex( const unsigned char* data, size_t size )
{
	std::vector< std::vector<unsigned char> > inspect;
	inspect.resize( size );

	for ( size_t i = 0; i < size; i++ )
	{
		inspect[i].resize( HEX_SIZE );
		inspect[i][0] = data[i] & 0x0F;
		inspect[i][1] = data[i] >> 4;
	}
	return inspect;
}

void Inspector::ShowInspector( const std::vector<std::vector<unsigned char>>& data )
{
	for ( int i = data.size() - 1; i >= 0; i-- )
	{
		for ( int j = data[i].size() - 1; j >= 0; j-- )
			printf( "%hhx", data[i][j] );
		printf( " " );
	}
	printf( "\n" );
}
