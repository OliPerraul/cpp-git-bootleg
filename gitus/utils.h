#ifndef GITUS_UTILS_H
#define GITUS_UTILS_H

#include <iostream>
#include <sstream>
#include <bitset>
#include <memory>
#include <vector>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/filesystem.hpp>


#include <boost/iostreams/filter/zlib.hpp>
#include <boost/iostreams/filtering_stream.hpp>
#include <boost/iostreams/copy.hpp>

#define DIR_CACHE_SIGNATURE "DIRC"

static const size_t IndexFormatVersion = 2;
static const size_t HeaderLength = 12;
static const size_t EntryLength = 59;

// 16, and 32 bit null terminated char sequences
union Word2 {
public:
	char    c[4];
	size_t n;
};

union Word {
public:
	char    c[2];
	unsigned short n;
};

//https://mincong-h.github.io/2018/04/28/git-index/
struct IndexEntry
{
	size_t numFields = 10;

	// the last time a file�s metadata changed
	// nanosecond fractions
	// the last time a file�s data changed
	// nanosecond fractions
	// The device (disk) upon which file resides
	// The file inode number. 
	// hexadecimal representation of file permission
	// The user identifier of the current user
	// The group identifier of the current user
	// file size in number of bytes
	std::vector<Word2> fields;

	union Word flags;

	std::string sha1;

	std::string path;

	union Word2 Permission()
	{
		return fields[6];
	}

	IndexEntry()
	{
		Word2 buffer; buffer.n = 0;
		for (int i = 0; i < numFields; i++)
		{
			fields.push_back(buffer);
		}

		flags.n = 0;

	};

	IndexEntry(std::string header) {

		Word2 buffer;
		for (int i = 0; i < numFields; i++)
		{
			memcpy(buffer.c, header.data() + 32 * i, 32);
			fields.push_back(buffer);
		}

		flags.n = 0;
	}
};

class GitusUtils {

private:
public:

	enum  ObjectHashType
	{
		Blob,
		Commit,
		Tree
	};

	static boost::filesystem::path IndexFile()
	{
		return boost::filesystem::current_path() / ".git/index";
	}

	static boost::filesystem::path HeadFile()
	{
		return boost::filesystem::current_path() / ".git/HEAD";
	}

	static boost::filesystem::path GitusDirectory()
	{
		return boost::filesystem::current_path() / ".git/";
	}

	static boost::filesystem::path RefsDirectory()
	{
		return boost::filesystem::current_path() / ".git/refs/";
	}

	static boost::filesystem::path HeadsDirectory()
	{
		return boost::filesystem::current_path() / ".git/refs/heads/";
	}

	static boost::filesystem::path MasterFile()
	{
		return boost::filesystem::current_path() / ".git/refs/heads/master";
	}

	static boost::filesystem::path ObjectsDirectory()
	{
		return boost::filesystem::current_path() / ".git/objects/";
	}


	static std::string Sha1(std::string object)
	{
		boost::uuids::detail::sha1 sha1;

		// I use data() to disregard the null terminated char
		sha1.process_bytes(object.data(), sizeof(char)*object.size());
		unsigned hash[5] = { 0 };
		sha1.get_digest(hash);

		// Back to string
		char buf[41] = { 0 };

		for (int i = 0; i < 5; i++)
		{
			std::sprintf(buf + (i << 3), "%08x", hash[i]);
		}

		return std::string(buf);
	};

	// TODO: cite source 
	// https://stackoverflow.com/questions/27529570/simple-zlib-c-string-compression-and-decompression
	static std::string Compress(const std::string& data)
	{
		using namespace boost::iostreams;

		std::stringstream compressed;
		std::stringstream decompressed;
		decompressed << data;
		filtering_streambuf<input> out;
		out.push(zlib_compressor());
		out.push(decompressed);
		copy(out, compressed);
		return compressed.str();
	}

	static std::string Decompress(const std::string& data)
	{
		using namespace boost::iostreams;

		std::stringstream compressed;
		std::stringstream decompressed;
		compressed << data;
		filtering_streambuf<input> in;
		in.push(zlib_decompressor());
		in.push(compressed);
		copy(in, decompressed);
		return decompressed.str();
	}


	// Simplified hashing
	static std::string HashObject(std::string object, ObjectHashType type, bool write=true)
	{
		using namespace std;
		using namespace boost;
		namespace ios = iostreams;

		std::string header;
		switch (type)
		{
		case GitusUtils::Blob:
			header = "blob";
			break;
		case GitusUtils::Commit:
			header = "commit";
			break;
		case GitusUtils::Tree:
			header = "tree";
			break;
		default:
			break;
		}

		std::string content = header + object;
		auto sha1 = Sha1(content);
		if (write)
		{
			std::string first = sha1.substr(0, 2);
			std::string last = sha1.substr(2, string::npos);

			auto filePath = ObjectsDirectory()
				/ first;

			if (!filesystem::exists(filePath / last)) {
				if (!filesystem::exists(filePath)) {
					filesystem::create_directories(filePath);
				}

				filesystem::ofstream ofs{ filePath / last };
				ofs << Compress(content);
			}
		}

		return sha1;
	}
	static::std::string GenerateEntryPadding(std::string entryPath) {
		auto pathSize = entryPath.size();
		auto pathOffset = ((EntryLength + pathSize + 8) / 8) * 8;
		auto paddingLength = pathOffset - EntryLength - pathSize;
		return std::string(paddingLength, '\0');
	}

	// Not required by the assignment
	static std::string ReadObject(std::string object)
	{

	}

	// header = struct.pack('!4sLL', b'DIRC', 2, len(entries))
	// TODO do not care about null terminated strings?
	static void WriteIndex(const std::unique_ptr<std::map<std::string, IndexEntry>>& entries)
	{
		using namespace std;
		using namespace boost;

		string entries_data;

		// null terminated buffer to enable string concatenation
		char* buffer4 = new char[5]; buffer4[4] = '\0';
		char* buffer2 = new char[3]; buffer2[2] = '\0';

		// The c++ standard guarantees that a map is iterated over in order of the keys
		for (auto it = entries->begin(); it != entries->end(); it++)
		{
			auto* entry = &it->second;

			for (int i = 0; i < entry->numFields; i++)
			{
				memcpy(buffer4, entry->fields[i].c, 4);
				entries_data += buffer4;
			}

			entries_data += entry->sha1;

			memcpy(buffer2, entry->flags.c, 2);
			entries_data += buffer2;

			entries_data += entry->path;
			auto entryPadding = GenerateEntryPadding(entry->path);
			entries_data += entryPadding;
		}

		// A 12 byte header
		string header;

		// Signature
		header += DIR_CACHE_SIGNATURE;
		
		// Format
		Word version; version.n = IndexFormatVersion;
		memcpy(buffer4, version.c, 4);
		header += buffer4;

		// Number of entries
		Word numEntries; numEntries.n = entries->size();
		memcpy(buffer4, version.c, 4);
		header += buffer4;
	
		delete[] buffer4;
		delete[] buffer2;

		string data = header + entries_data;
		auto digest = Sha1(data);


		filesystem::ofstream ofs{ IndexFile() };

		auto output = data + digest;
		ofs << output;
		

		//std::ofstream outfile(IndexFile().string(), std::ofstream::binary);
		//auto output = data + digest;

		//outfile << output;
	
	
};
	
	static std::string ReadBytes(std::string filename)
	{
		std::ifstream ifs(filename);
		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		return content;
	}

	static std::string ReadFile(std::string filename)
	{
		std::ifstream ifs(filename);
		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		return content;
	}

	static std::unique_ptr<std::map<std::string, IndexEntry>> ReadIndex()
	{
		using namespace std;
		using namespace boost;

		auto entries = 
			unique_ptr<map<string, IndexEntry>>(new map<string, IndexEntry>());

		if (filesystem::exists(IndexFile()))
		{
			auto data = ReadBytes(IndexFile().string());
			std::size_t fileLength = data.size();
			string digest = data.substr(fileLength - 40);
			string content = data.substr(0, fileLength  - 40);
			string contentHash = Sha1(content);
			if (contentHash != digest) {
				throw std::exception("Index file got corrupted...");
			}
			
			string indexEntries = content.substr(HeaderLength);
			int i = 0;
			while (i + EntryLength < indexEntries.size()) {
				auto entryEnd = i + EntryLength;
				auto entryContent = indexEntries.substr(i, entryEnd);
				auto entryHeader = entryContent.substr(0, 10);
				
				IndexEntry entry(entryHeader);
				entry.sha1 = entryContent.substr(10, 40);

				auto flagString = entryContent.substr(50, 5);
				memcpy(entry.flags.c, flagString.data(), flagString.size());

				auto possiblePathRange = indexEntries.substr(55);
				auto endOfPathIndex = possiblePathRange.find('\0');
				endOfPathIndex = endOfPathIndex == std::string::npos ? 4 : endOfPathIndex;
				entry.path = possiblePathRange.substr(0, endOfPathIndex);

				entries->insert(make_pair(entry.path, entry));

				auto currentEntryLength = ((EntryLength + entry.path.size() + 8) / 8) * 8;
				i += currentEntryLength;
			}
		}

		return entries;
	};

	static std::string CreateCommitTree() {
		auto index = GitusUtils::ReadIndex();
		std::string treeEntries;
		for (auto it = index->begin(); it != index->end(); it++)
		{
			std::string currentEntry;
			auto* entry = &it->second;
			currentEntry += entry->Permission().c;
			//TODO::FIND Real way...
			currentEntry += "blob ";
			currentEntry += entry->sha1 + ' ';
			currentEntry += entry->path;
			currentEntry += '\n';

			treeEntries += currentEntry;
		}
		auto treeHash = GitusUtils::HashObject(treeEntries, GitusUtils::Tree);
		return treeHash;
	}

	static bool HasParentTree() {
		auto parentTreePath = MasterFile();
		auto masterSize = boost::filesystem::file_size(parentTreePath);
		auto hashParent = masterSize == 0;
		return hashParent;
	}

	static std::string ParentTreeHash() {
		auto parenTreePath = MasterFile();
		auto parentTreeData = ReadBytes(parenTreePath.string());
		return parentTreeData;

	}

	static bool CheckIfGitObjectExist(std::string hash) {
		std::string first = hash.substr(0, 2);
		std::string last = hash.substr(2, std::string::npos);

		auto filePath = ObjectsDirectory()
			/ first;
		return boost::filesystem::exists(filePath / last);
	}

};


#endif

