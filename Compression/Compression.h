#pragma once
#include <vector>
#include <cmath>
#include <fstream>

#define SAFETY 0
#define BYTELEN 8
#define BYTESIZE 256
#define COMPRESSION_HEADER_SIZE sizeof(Compression_Header)

enum ClusterAdressing
{
	BIT_0 = 0,
	BIT_8 = 1,
	BIT_16 = 2,
	BIT_24 = 3,
	BIT_32 = 4,
	BIT_40 = 5,
	BIT_48 = 6,
	BIT_56 = 7,
	BIT_64 = 8
};

constexpr char MaskTemplate[9] = { 0x00, 0x01, 0x03, 0x07, 0x0F, 0x1F, 0x3F, 0x7F, 0xFF };

struct Compression_Header
{
	uint8_t Information;
	unsigned short SizeCluster;
	unsigned long long SizeSource;
};

struct Cluster_Header
{
	uint8_t DataLength : 4;
	uint8_t DecoderLength : 4;
};

template<typename T>
class Compression
{
	std::vector<uint8_t> mData;

public:
	Compression() {}
	Compression( T* data, Compression_Header templ ) { Compress( data, templ ); }

	static inline void Create_Compression_Header( std::vector<uint8_t>& data, Compression_Header templ )
	{
		#if SAFETY
		if ( !data.empty() ) data.clear();
		if ( data.size() < COMPRESSION_HEADER_SIZE ) data.resize( COMPRESSION_HEADER_SIZE );
		#endif
		for ( size_t i = 0; i < COMPRESSION_HEADER_SIZE; i++ )
		{
			uint8_t* ptr = (uint8_t*) &templ;
			data[i] = ptr[i];
		}
	}

	static inline Compression_Header Read_Compression_Header( const std::vector<uint8_t>& data )
	{
		#if SAFETY
		if ( !data.empty() ) return Compression_Header();
		#endif
		Compression_Header templ;
		for ( size_t i = 0; i < COMPRESSION_HEADER_SIZE; i++ )
		{
			uint8_t* ptr = (uint8_t*) &templ;
			ptr[i] = data[i];
		}
		return templ;
	}

	void Compress( T* data, Compression_Header templ )
	{
		if ( templ.SizeCluster != 0 && templ.SizeSource != 0 && data != nullptr )
		{
			unsigned long long TotalCluster = templ.SizeSource / templ.SizeCluster;
			unsigned long long LastClusterSize = templ.SizeSource % templ.SizeCluster;
			if ( LastClusterSize != 0 ) TotalCluster++;

			auto ClusterAddressing = Cluster_Addressing_Size( TotalCluster, templ.SizeSource );
			this->mData.resize( COMPRESSION_HEADER_SIZE + ClusterAddressing * TotalCluster );
			templ.Information = ClusterAddressing;

			uint8_t* pData = (uint8_t*) data;
			for ( unsigned long long i = 0; i < TotalCluster - 1; i++ )
			{
				auto compressed = this->Compress_Cluster( &pData[i * templ.SizeCluster], templ.SizeCluster );
				Store_Cluster_Address( this->mData, ClusterAddressing, i, mData.size() );
				for ( auto byte : compressed ) mData.push_back( byte );
			}
			if ( LastClusterSize != 0 )
			{
				auto compressed = this->Compress_Cluster( &pData[( TotalCluster - 1 ) * templ.SizeCluster], LastClusterSize );
				//store the address of the last cluster
				if ( TotalCluster > 1 )
					Store_Cluster_Address( this->mData, ClusterAddressing, TotalCluster - 1, mData.size() );
				for ( auto byte : compressed ) mData.push_back( byte );
			}
			Compression::Create_Compression_Header( this->mData, templ );
		}
	}

	T* Decompress() 
	{
		auto compHeader = Compression::Read_Compression_Header( this->mData );
		uint8_t* decompData = (uint8_t*) malloc( compHeader.SizeSource );

		unsigned long long TotalCluster = compHeader.SizeSource / compHeader.SizeCluster;
		unsigned long long LastClusterSize = compHeader.SizeSource % compHeader.SizeCluster;
		if ( LastClusterSize != 0 ) TotalCluster++;

		for ( unsigned long long i = 0; i < TotalCluster - 1; i++ )
		{
			unsigned long long cAddress = Retrieve_Cluster_Address( this->mData, compHeader.Information, i );
			auto decompressed = Compression::Decompress_Cluster( (uint8_t*) &this->mData[0], cAddress, compHeader.SizeCluster );
			//decompressed vector size is guaranteed to be cluster size bytes
			for ( unsigned short j = 0; j < compHeader.SizeCluster; j++ )
				decompData[i * compHeader.SizeCluster + j] = decompressed[j];
		}
		if ( LastClusterSize != 0 )
		{
			unsigned long long cAddress = COMPRESSION_HEADER_SIZE;
			if ( TotalCluster > 1 ) cAddress = Retrieve_Cluster_Address( this->mData, compHeader.Information, TotalCluster-1 );
			auto decompressed = Compression::Decompress_Cluster( (uint8_t*) &this->mData[0], cAddress, LastClusterSize );
			//decompressed vector size is guaranteed to be last cluster size bytes
			for ( unsigned short j = 0; j < LastClusterSize; j++ )
				decompData[(TotalCluster-1) * compHeader.SizeCluster + j] = decompressed[j];
		}
		return (T*) decompData;
	}
	
	size_t Size() const { return mData.size(); }
	bool isEmpty() const { return mData.empty(); }
	void Delete() { this->mData.clear(); }

private:
	static std::vector<uint8_t> Compress_Cluster( uint8_t* data, unsigned long long size )
	{
		bool byte_value[BYTESIZE];
		for ( unsigned long long i = 0; i < BYTESIZE; i++ ) byte_value[i] = false;
		for ( unsigned long long i = 0; i < size; i++ )		byte_value[data[i]] = true;

		//generate new value
		std::vector<uint8_t> list_decoder;
		std::vector<uint8_t> list_encoded;
		list_encoded.resize( BYTESIZE );
		uint8_t new_value = 0;
		for ( size_t i = 0; i < BYTESIZE; i++ )
		{
			if ( byte_value[i] )
			{
				list_encoded[i] = new_value;
				list_decoder.push_back( i );
				new_value++;
			}
		}

		auto DataLen = Len_Bit( new_value - 1 );
		auto DecoderLen = Len_Bit( list_decoder.back() );

		unsigned long long Allocation_Data = std::ceil( (double) DataLen * size / 8 );
		unsigned long long Allocation_Decoder = std::ceil( (double) DecoderLen * new_value / 8 );
		unsigned long long Allocation_Total = 1 + Allocation_Data + Allocation_Decoder;

		Cluster_Header Header;
		if ( Allocation_Total > size + 1 )
		{
			Allocation_Total = size + 1;
			Header.DataLength = 0;
			Header.DecoderLength = 0;
		}
		else
		{
			Header.DataLength = DataLen;
			Header.DecoderLength = DecoderLen;
		}

		std::vector<uint8_t> ClusterData;
		ClusterData.resize( Allocation_Total );
		ClusterData[0] = Compression::Create_Cluster_Header( Header );

		if ( needCompression( Header ) )
		{
			//generate new compressed data
			std::vector<uint8_t> compressed_data;
			compressed_data.resize( size );
			for ( unsigned long long i = 0; i < size; i++ )
				compressed_data[i] = list_encoded[data[i]];

			//compress data
			for ( unsigned long long i = 0; i < Allocation_Data; i++ )
				ClusterData[i + 1] = Shrink_Byte( i, Header.DataLength, compressed_data );

			//insert decoder table
			for ( unsigned long long i = 0; i < Allocation_Decoder; i++ )
				ClusterData[1 + Allocation_Data + i] = Shrink_Byte( i, Header.DecoderLength, list_decoder );
		}
		else
		{
			for ( unsigned long long i = 0; i < size; i++ )
				ClusterData[i + 1] = data[i];
		}
		return ClusterData;
	}

	static std::vector<uint8_t> Decompress_Cluster( uint8_t* data, unsigned long long cAddress, unsigned long long size )
	{
		if ( data == nullptr ) return std::vector<uint8_t>();

		Cluster_Header Header = Compression::Read_Cluster_Header( data, cAddress );
		std::vector<uint8_t> new_block;
		new_block.resize( size );

		if ( needCompression( Header ) )
		{
			std::vector<uint8_t> compressed_data, decoder_data;
			size_t HighestVal = 0;
			for ( size_t i = 0; i < size; i++ )
			{
				auto val = Compression::Extend_Byte( i, Header.DataLength, &data[cAddress], 1 );
				compressed_data.push_back( val );
				if ( val > HighestVal ) HighestVal = val;
			}

			size_t decoder_jump_byte = 1 + std::ceil( (double) Header.DataLength * size / 8 );
			for ( size_t i = 0; i < HighestVal + 1; i++ )
				decoder_data.push_back( Compression::Extend_Byte( i, Header.DecoderLength, &data[cAddress], decoder_jump_byte ) );

			for ( size_t i = 0; i < size; i++ )
				new_block[i] = decoder_data[compressed_data[i]];
		}
		else
		{
			for ( size_t i = 0; i < size; i++ )
				new_block[i] = data[i + 1 + cAddress];
		}
		return new_block;
	}

	static inline uint8_t Len_Bit( const uint8_t& len )
	{
		return (int) log2( len ) + 1;
	}

	static inline uint8_t Cluster_Addressing_Size( unsigned long long TotalCluster, unsigned long long SizeSource )
	{
		if		( TotalCluster <= 1 )					return ClusterAdressing::BIT_0;
		else if	( SizeSource <= 0xFF )					return ClusterAdressing::BIT_8;
		else if ( SizeSource <= 0xFFFF )				return ClusterAdressing::BIT_16;
		else if ( SizeSource <= 0xFFFFFF )				return ClusterAdressing::BIT_24;
		else if ( SizeSource <= 0xFFFFFFFF )			return ClusterAdressing::BIT_32;
		else if ( SizeSource <= 0xFFFFFFFFFF )			return ClusterAdressing::BIT_40;
		else if ( SizeSource <= 0xFFFFFFFFFFFF )		return ClusterAdressing::BIT_48;
		else if ( SizeSource <= 0xFFFFFFFFFFFFFF )		return ClusterAdressing::BIT_56;
		else if ( SizeSource <= 0xFFFFFFFFFFFFFFFF )	return ClusterAdressing::BIT_64;
	}

	inline void Store_Cluster_Address( std::vector<uint8_t>& data, uint8_t mode, unsigned long long index, unsigned long long address )
	{
		uint8_t* ptr = (uint8_t*) &address;
		for ( uint8_t i = 0; i < mode; i++ )
		{
			data[index * mode + COMPRESSION_HEADER_SIZE + i] = *ptr;
			ptr++;
		}
	}

	inline unsigned long long Retrieve_Cluster_Address( std::vector<uint8_t>& data, uint8_t mode, unsigned long long index )
	{
		unsigned long long address = 0;
		uint8_t* ptr = (uint8_t*) &address;
		for ( uint8_t i = 0; i < mode; i++ )
		{
			*ptr = data[index * mode + COMPRESSION_HEADER_SIZE + i];
			ptr++;
		}
		return address;
	}

	static inline uint8_t Create_Cluster_Header( const Cluster_Header& header )
	{
		#define DATA_REPOSITION header.DataLength << 4
		#define DECODER_REPOSITION header.DecoderLength << 0
		char compression_header = ( DATA_REPOSITION ) | ( DECODER_REPOSITION );
		return compression_header;
	}

	static inline Cluster_Header Read_Cluster_Header( const uint8_t* data, unsigned long long cAddress )
	{
		Cluster_Header header;
		header.DataLength = data[cAddress] >> 4;
		header.DecoderLength = MaskTemplate[4] & data[cAddress];
		return header;
	}

	static uint8_t Shrink_Byte( size_t index, uint8_t clen, std::vector<uint8_t>& list )
	{
		//index parameter is byte, change to list index. (get value from list)
		index = std::floor( (double) index * BYTELEN / clen );
		uint8_t byte = 0;
		int available = BYTELEN;
		#define PREVIOUS_REMAINDER index * clen % 8
		#define NEXT_REMAINDER (index + 1) * clen % 8
		if ( PREVIOUS_REMAINDER != 0 )
		{
			available -= NEXT_REMAINDER;
			byte = list[index] >> BYTELEN - PREVIOUS_REMAINDER;
			index++;
		}
		while ( available > 0 && index < list.size() )
		{
			uint8_t current = list[index];
			current = current << BYTELEN - available;
			byte = byte | current;
			available -= clen;
			index++;
		}
		return byte;
	}

	static uint8_t Extend_Byte( size_t index, uint8_t clen, uint8_t* data, size_t jump_byte )
	{
		uint8_t byte = 0;
		size_t start_bit_index = ( clen * index % 8 );
		size_t first_byte_index = std::floor( (double) clen * index / 8 ) + jump_byte;
		size_t stop_byte_index = std::floor( (double) ( clen * ( 1 + index ) - 1 ) / 8 ) + jump_byte;

		if ( first_byte_index == stop_byte_index )
		{
			byte = ( data[first_byte_index] >> start_bit_index ) & MaskTemplate[clen];
		}
		else
		{
			#define next_c ( data[stop_byte_index] << 8 - start_bit_index ) & MaskTemplate[clen]
			byte = ( data[first_byte_index] >> start_bit_index ) | next_c;
		}
		return byte;
	}

	static inline bool needCompression( const Cluster_Header& header )
	{
		if ( header.DecoderLength != 0 ) return true;
		return false;
	}

public:
	friend bool Read_Compression_File( std::string filename, Compression& data )
	{
		bool success = false;
		auto myfile = std::fstream( filename, std::ios::in | std::ios::binary );
		if ( myfile.is_open() )
		{
			myfile.seekg( 0, myfile.end );
			size_t FileSize = myfile.tellg();
			if ( FileSize > sizeof( size_t ) )
			{
				if ( !data.isEmpty() ) data.Delete();
				data.mAllocation = FileSize - sizeof( size_t );
				data.mData = (uint8_t*) malloc( data.mAllocation );

				myfile.seekg( 0 );
				myfile.read( reinterpret_cast<char*>( &data.mSize ), sizeof( size_t ) );

				myfile.seekg( sizeof(size_t) );
				myfile.read( reinterpret_cast<char*>( data.mData ), data.mAllocation );
				success = true;
			}
			myfile.close();
		}
		return success;
	}
	friend bool Write_Compression_File( std::string filename, const Compression& data )
	{
		bool success = false;
		//write size
		auto myfile = std::fstream( filename, std::ios::out | std::ios::binary );
		if ( myfile )
		{
			myfile.write( reinterpret_cast<const char*>( &data.mSize ), sizeof( size_t ) );
			myfile.close();
			//write compressed data
			myfile = std::fstream( filename, std::ios::app | std::ios::binary );
			if ( myfile )
			{
				myfile.write( reinterpret_cast<const char*>( data.mData ), data.mAllocation );
				myfile.close();
				success = true;
			}
		}
		return success;
	}
};