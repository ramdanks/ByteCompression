#include <iostream>
#include "Utilities/Timer.h"
#include "Compression.h"

int arr[] = { 10,22,1,21,12,11,11,31,41,21,2,1,3,1,21,21,21,1,1,1,2,3,4,1,2,3,4,1,2 };

int main( int argc, char* argv[] )
{
	Compression_Header cHeader;
	cHeader.SizeCluster = 64;
	cHeader.SizeSource = sizeof arr;

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
	for ( int i = 0; i < 29; i++ )
		std::cout << result[i] << " ";
	std::cout << std::endl;
	free( result );

	return 0;
}