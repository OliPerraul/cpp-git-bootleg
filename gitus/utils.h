#ifndef GITUS_UTILS_H
#define GITUS_UTILS_H

#include <iostream>
#include <memory>
#include <vector>
#include <sstream>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/filesystem.hpp>

#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filtering_streambuf.hpp>


// 16, and 32 bit
union Word2 {
public:
	char    c[4];
	unsigned int n;
};

union Word {
public:
	char    c[2];
	unsigned short n;
};

// To correctly handle null chars and for semantic reasons, 'raw_data' is prefered over 'std::string'
// https://arne-mertz.de/2018/11/string-not-for-raw-data/
typedef std::vector<unsigned char> RawData;
#define BZERO(data) std::fill((data).begin(), (data).end(), 0)

#define SUBSTR(data, pos, len) RawData(data.begin()+pos, data.begin()+pos+len)


class Utils {

private:
public:

	// Returns SHA1 as binrary
	static std::shared_ptr<RawData> Sha1(RawData object)
	{
		using namespace std;

		boost::uuids::detail::sha1 sha1;
		sha1.process_bytes(object.data(), sizeof(char)*object.size());
		unsigned int hash[5];
		sha1.get_digest(hash);

		auto data = shared_ptr<RawData>(new RawData);
		
		for (int i = 0; i < 5; i++)
		{
			Word2 val; val.n = hash[i];
			copy(&val.c[0], &val.c[4], back_inserter(data));
		}

		return data;
	};


	// Returns SHA1 as Hex string
	static std::string Sha1String(RawData object)
	{
		using namespace std;

		boost::uuids::detail::sha1 sha1;

		// I use data() to disregard the null terminated char
		sha1.process_bytes(object.data(), sizeof(char)*object.size());

		unsigned int hash[5];
		sha1.get_digest(hash);

		std::stringstream ss;
		for (int i = 0; i < 5; i++)
		{	
			ss << std::hex << hash[i];
		}

		return ss.str();
	};

	// Compression code from
// https://stackoverflow.com/questions/27529570/simple-zlib-c-string-compression-and-decompression
	static std::string Compress(const RawData& data)
	{
		using namespace boost::iostreams;

		std::stringstream compressed;
		std::stringstream decompressed;
		decompressed << data.data();
		filtering_streambuf<input> out;
		out.push(zlib_compressor());
		out.push(decompressed);
		copy(out, compressed);
		return compressed.str();
	}

	static std::string Decompress(const RawData& data)
	{
		using namespace boost::iostreams;

		std::stringstream compressed;
		std::stringstream decompressed;
		compressed << data.data();
		filtering_streambuf<input> in;
		in.push(zlib_decompressor());
		in.push(compressed);
		copy(in, decompressed);
		return decompressed.str();
	}

	static RawData ReadBytes(std::string filename)
	{
		std::ifstream ifs(filename);
		RawData content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		return content;
	}

	//static std::string ReadFile(std::string filename)
	//{
	//	std::ifstream ifs(filename);
	//	std::string content((std::istreambuf_iterator<char>(ifs)),
	//		(std::istreambuf_iterator<char>()));
	//	return content;
	//}

};


#endif

