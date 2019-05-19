
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

#include "gitus_service.h"
#include "utils.h"

static const char* DirCacheSignature = "DIRC";
static const size_t IndexFormatVersion = 2;
static const size_t HeaderLength = 12;

static const size_t EntryHeaderLength = 40;
static const size_t Sha1Size = 20;
static const size_t FlagsLength = 2;
// total
static const size_t BaseEntryLength = 52;



bool GitusService::HashObject(const RawData& object, ObjectHashType type, bool write, RawData& sha1)
{
	using namespace std;
	using namespace boost;
	namespace ios = iostreams;

	RawData content = CreateContentData(object, type);

	// sha1 string hex used for object path
	// 20 bytes returned as 40 hex characters
	string sha1String;
	Utils::Sha1String(content, sha1String);
	if (write)
	{
		std::string first = sha1String.substr(0, 2);
		std::string last = sha1String.substr(2, string::npos);

		auto filePath = ObjectsDirectory()
			/ first;

		if (!filesystem::exists(filePath / last)) {
			if (!filesystem::exists(filePath)) {
				filesystem::create_directories(filePath);
			}

			filesystem::ofstream ofs{ filePath / last };
			ofs << Utils::Compress(content);
		}
	}

	// populate full sha binary
	Utils::Sha1(content, sha1);
	return true;
}


RawData GitusService::CreateContentData(const RawData& object, ObjectHashType type) {
	auto header = CreateHeaderData(type, object);
	RawData content;
	std::copy(header.begin(), header.end(), std::back_inserter(content));
	std::copy(object.begin(), object.end(), std::back_inserter(content));
	return content;
}

RawData GitusService::CreateHeaderData(GitusService::ObjectHashType type, const RawData & object)
{
	std::string t;
	RawData header;
	switch (type)
	{
	case GitusService::Blob:
		t = "blob";
		break;
	case GitusService::Commit:
		t = "commit";
		break;
	case GitusService::Tree:
		t = "tree";
		break;
	default:
		break;
	}

	// add the type
	copy(t.begin(), t.end(), std::back_inserter(header));

	// add the size
	Word2 size; size.n = object.size();
	copy(&size.c[0], &(size.c[4]), back_inserter(header));
	return header;
}


bool GitusService::WriteIndex(const std::map<std::string, IndexEntry>& entries)
{
	using namespace std;
	using namespace boost;

	RawData entriesData;

	// The c++ standard guarantees that a map is iterated over in order of the keys
	for (auto it = entries.begin(); it != entries.end(); it++)
	{
		auto* entry = &it->second;

		for (int i = 0; i < entry->numFields; i++)
		{
			for(int j = 0; j < 4; j++)
			entriesData.push_back(entry->fields[i].c[j]);
		}

		for (int j = 0; j < Sha1Size; j++)
		entriesData.push_back(entry->sha1[j]);

		// add flags
		for (int j = 0; j < 2; j++)
			entriesData.push_back(entry->flags.c[j]);

		// Add path
		for (int j = 0; j < entry->path.size(); j++)
			entriesData.push_back(entry->path[j]);
		entriesData.push_back(0);//null terminate the path
		
		// Add padding
		size_t pathOffset = ((BaseEntryLength + entry->path.size() + 8) / 8) * 8;
		auto paddingLength = pathOffset - BaseEntryLength - entry->path.size();
		for (int j = 0; j < paddingLength; j++)
			entriesData.push_back(0);
	}

	RawData data;

	// A 12 byte header
	// Signature
	for (int j = 0; j < 4; j++)
		data.push_back(DirCacheSignature[j]);

	// Version number
	Word2 version; version.n = IndexFormatVersion;
	for (int j = 0; j < 4; j++)
		data.push_back(version.c[j]);

	// Number of entries
	Word2 numEntries; numEntries.n = entries.size();
	for (int j = 0; j < 4; j++)
		data.push_back(numEntries.c[j]);

	// Concat entries
	copy(entriesData.begin(), entriesData.end(), back_inserter(data));

	RawData digest;
	Utils::Sha1(data, digest);

	filesystem::ofstream ofs{ IndexFile() };

	// Concat digest
	copy(digest.begin(), digest.end(), back_inserter(data));

	ofs.write(reinterpret_cast<char*>(data.data()), data.size()*sizeof(unsigned char));

	return true;

};

bool GitusService::ReadIndex(std::map<std::string, IndexEntry>& entries)
{
	using namespace std;
	using namespace boost;

	if (filesystem::exists(IndexFile()))
	{
		auto data = Utils::ReadBytes(IndexFile().string());
		RawData digest(data.end() - Sha1Size, data.end());
		RawData content(data.begin(), data.end() - Sha1Size);

		RawData contentHash;
		Utils::Sha1(content, contentHash);
		if (contentHash != digest) {
			std::exception("fatal: Index file is corrupted.");
			return false;
		}

		RawData indexEntries(content.begin() + HeaderLength, content.end());

		int i = 0;
		while (i + BaseEntryLength < indexEntries.size())
		{
			RawData remainingEntriesContent(indexEntries.begin() + i, indexEntries.end());
			RawData entryHeader(remainingEntriesContent.begin(), remainingEntriesContent.begin() + EntryHeaderLength);

			IndexEntry entry(entryHeader);
			size_t shaEndPos = EntryHeaderLength + Sha1Size;
			entry.sha1 = RawData(
				remainingEntriesContent.begin() + EntryHeaderLength, 
				remainingEntriesContent.begin() + shaEndPos);

			size_t flagsEndPos = shaEndPos + FlagsLength;
			RawData flags(
				remainingEntriesContent.begin() + shaEndPos,
				remainingEntriesContent.begin() + flagsEndPos);

			copy(&flags.data()[0], &flags.data()[2], entry.flags.c);

			// Find potential range for a path over remaining bits
			RawData possiblePathRange(
				indexEntries.begin() + flagsEndPos,
				indexEntries.end());

			// Find null terminated string

			auto pos = std::find(indexEntries.begin() + flagsEndPos, indexEntries.end(), '\0');
			
			auto path = RawData(indexEntries.begin() + flagsEndPos, pos);

			copy(path.begin(), path.end(), back_inserter<string>(entry.path));

			entries.insert(make_pair(entry.path, entry));

			auto currentEntryLength = ((BaseEntryLength + entry.path.size() + 8) / 8) * 8;
			i += currentEntryLength;
		}
	}

	return true;
};


RawData GitusService::HashCommitTree() {

	using namespace std;
	using namespace boost;

	auto entries = map<string, IndexEntry>();
	RawData treeEntries;

	if (!GitusService::ReadIndex(entries))
	{
		// Error occured
		return treeEntries;
	}
	else if (entries.empty())
	{
		std::cout << "Your branch is up to date with 'origin/master'" << std::endl;
		return treeEntries;
	}


	// each 'line' in a tree object is in the '<mode><space><path>' format
	// then a NUL byte, then the binary SHA-1 hash.
	for (auto it = entries.begin(); it != entries.end(); it++)
	{
		auto* entry = &it->second;
		
		// mode
		copy(&entry->fields[6].c[0], &entry->fields[6].c[4], back_inserter(treeEntries));
		
		// empty space
		treeEntries.push_back(' ');
		
		// path
		copy(entry->path.begin(), entry->path.end(), back_inserter(treeEntries));
		
		// null byte
		treeEntries.push_back(0);

		// sha1
		copy(entry->sha1.begin(), entry->sha1.end(), back_inserter(treeEntries));	
	}

	return treeEntries;
}

bool GitusService::HasParentTree() {
	auto parentTreePath = MasterFile();
	auto masterSize = boost::filesystem::file_size(parentTreePath);
	auto hashParent = masterSize > 0;
	return hashParent;
}

bool GitusService::LocalMasterHash(RawData& hash) {
	// Get current commit hash of local master branch
	hash = Utils::ReadBytes(MasterFile().string());
	return true;
}

bool GitusService::ObjectExists(std::string sha1String) {

	using namespace std;

	std::string first = sha1String.substr(0, 2);
	std::string last = sha1String.substr(2, std::string::npos);

	auto filePath = ObjectsDirectory() / first;

	return boost::filesystem::exists(filePath / last);
}


