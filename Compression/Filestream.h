#pragma once
#include <fstream>

bool Write_File( const char* filename, char* data, size_t size )
{
	bool success = false;
	auto myfile = std::fstream( filename, std::ios::out | std::ios::binary );
	if ( myfile.is_open() )
	{
		if ( size > 0 )
		{
			myfile.write( data, size );
			success = true;
		}
		myfile.close();
	}
	return success;
}

bool Read_File( const char* filename, char*& data, size_t& reported_size )
{
	bool success = false;
	auto myfile = std::fstream( filename, std::ios::in | std::ios::binary );
	if ( myfile.is_open() )
	{
		myfile.seekg( 0, myfile.end );
		reported_size = myfile.tellg();
		myfile.seekg( 0 );
		if ( reported_size > 0 )
		{
			if ( data != nullptr ) free( data );
			data = (char*) malloc( reported_size );
			myfile.read( data, reported_size );
			success = true;
		}
		myfile.close();
	}
	return success;
}