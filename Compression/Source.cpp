#include <iostream>
#include "Utilities/Timer.h"
#include "Compression.h"

int arr[] = { 10,22,1,21,12,11,11,31,41,21,2,1,3,1,21,21,21,1,1,1,2,3,4,1,2,3,4,1,2 };

template<typename T>
bool err_checker( T* src, T* dst, size_t size );

int main( int argc, char* argv[] )
{
	Compression_Header cHeader;
	cHeader.SizeCluster = 0;
	cHeader.SizeSource = sizeof arr;

	std::cout << "Most Efficient, with Cluster Size:" << Compression<int>::Dense_Cluster( arr, sizeof arr ) << std::endl;

	Compression<int> myArr;
	{
		Ramdan::Timer Log( "Compression", ADJUST, true );
		myArr.Compress( arr, cHeader );
	}
	int* result;
	{
		Ramdan::Timer Log( "Decompression", ADJUST, true );
		result = myArr.Decompress();
	}

	std::cout << "Size After Compression:" << myArr.Size() << std::endl;

	if ( err_checker( arr, result, sizeof arr ) ) std::cout << "Pass!" << std::endl;
	else
	{
		std::cout << "Fail!" << std::endl;
		for ( int i = 0; i < sizeof arr / sizeof arr[0]; i++ ) std::cout << result[i] << std::endl;
	}	

	free( result );

	return 0;
}

template<typename T>
bool err_checker( T* src, T* dst, size_t size )
{
	bool identical = true;
	uint8_t* pOne = (uint8_t*) src;
	uint8_t* pTwo = (uint8_t*) dst;

	for ( size_t i = 0; i < size; i++ )
		if ( pOne[i] != pTwo[i] )
		{
			identical = false;
			break;
		}
	return identical;
}