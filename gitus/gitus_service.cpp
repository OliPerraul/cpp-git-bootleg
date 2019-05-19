
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
static const size_t EntryLength = 59;
static const size_t EntryHeaderLength = 10;
static const size_t Sha1Size = 20;

bool GitusService::HashObject(const RawData& object, ObjectHashType type, bool write, RawData& sha1)
{
	using namespace std;
	using namespace boost;
	namespace ios = iostreams;

	RawData header;
	string t;
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

	RawData content;

	std::copy(header.begin(), header.end(), std::back_inserter(content));
	std::copy(object.begin(), object.end(), std::back_inserter(content));

	// sha1 string hex used for object path
	// 20 bytes returned as 40 hex characters
	auto sha1String = Utils::Sha1String(content);
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

	// Return full sha binary
	auto sha1 = Utils::Sha1(content);
	return true;
}

//// Not required by the assignment
//std::string GitusService::ReadObject(std::string object)
//{
//
//}

bool GitusService::WriteIndex(const std::unique_ptr<std::map<std::string, IndexEntry>>& entries)
{
	using namespace std;
	using namespace boost;

	RawData entriesData;

	// The c++ standard guarantees that a map is iterated over in order of the keys
	for (auto it = entries->begin(); it != entries->end(); it++)
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

		// Add padding
		auto pathOffset = ((EntryLength + entry->path.size() + 8) / 8) * 8;
		auto paddingLength = pathOffset - EntryLength - entry->path.size();
		for (int j = 0; j < entry->path.size(); j++)
			entriesData.push_back(0);

	}

	RawData data;

	// A 12 byte header
	// Signature
	for (int j = 0; j < 4; j++)
		data.push_back(DirCacheSignature[j]);

	// Format
	Word version; version.n = IndexFormatVersion;
	for (int j = 0; j < 2; j++)
		data.push_back(version.c[j]);

	// Number of entries
	Word2 numEntries; numEntries.n = entries->size();
	for (int j = 0; j < 4; j++)
		data.push_back(numEntries.c[j]);

	stringstream data;

	// Concat entries
	data.insert(data.end(), entriesData.begin(), entriesData.end());

	auto digest = Utils::Sha1(data);

	filesystem::ofstream ofs{ IndexFile() };

	// Concat digest
	data.insert(data.end(), entriesData.begin(), entriesData.end());

	ofs << data.data();

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

		auto contentHash = Utils::Sha1(content);
		if (*contentHash != digest) {
			std::exception("fatal: Index file is corrupted.");
			return false;
		}

		RawData indexEntries(content.begin()+HeaderLength, data.end());

		int i = 0;
		while (i + EntryLength < indexEntries.size())
		{
			RawData entryContent(indexEntries.begin() + i, indexEntries.begin() + i + EntryLength);
			RawData entryHeader(entryContent.begin(), entryContent.begin() + EntryHeaderLength);

			IndexEntry entry(entryHeader);
			entry.sha1 = RawData(
				entryContent.begin() + EntryHeaderLength, 
				entryContent.begin() + EntryHeaderLength + Sha1Size);

			RawData flags(
				entryContent.begin() + 50, 
				entryContent.begin() + 50 + 5);
			memcpy(entry.flags.c, flags.data(), flags.size());

			RawData possiblePathRange(
				indexEntries.begin() + 55,
				indexEntries.end());

			auto pos = std::find(indexEntries.begin() + 55, indexEntries.end(), '\0');
			auto path = RawData(indexEntries.begin() + 55, pos);
			copy(path.begin(), path.end(), back_inserter<string>(entry.path));

			entries.insert(make_pair(entry.path, entry));

			auto currentEntryLength = ((EntryLength + entry.path.size() + 8) / 8) * 8;
			i += currentEntryLength;
		}
	}

	return true;
};


bool GitusService::HashCommitTree(RawData& treeHash) {

	using namespace std;
	using namespace boost;

	auto entries = map<string, IndexEntry>();

	if (GitusService::ReadIndex(entries))
	{
		// Error occured
		return false;
	}
	else if (entries.empty())
	{
		std::cout << "Add file[s] to staging before committing..." << std::endl;
		return false;
	}

	RawData treeEntries;
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

	GitusService::HashObject(treeEntries, GitusService::Tree, true, treeHash);
	return true;
}

bool GitusService::HasParentTree() {
	auto parentTreePath = MasterFile();
	auto masterSize = boost::filesystem::file_size(parentTreePath);
	auto hashParent = masterSize == 0;
	return hashParent;
}

bool GitusService::ParentTreeHash(std::string hash) {
	auto parenTreePath = MasterFile();
	auto parentTreeData = Utils::ReadBytes(parenTreePath.string());
	return parentTreeData;

}

bool GitusService::CheckIfGitObjectExist(std::string hash) {
	std::string first = hash.substr(0, 2);
	std::string last = hash.substr(2, std::string::npos);

	auto filePath = ObjectsDirectory()
		/ first;
	return boost::filesystem::exists(filePath / last);
}


