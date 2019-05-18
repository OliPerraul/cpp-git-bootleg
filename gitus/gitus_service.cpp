
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

std::string GitusService::HashObject(RawData object, ObjectHashType type, bool write)
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

	auto sha1 = Utils::Sha1String(content);
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
			ofs << Utils::Compress(content);
		}
	}

	return sha1;
}

//// Not required by the assignment
//std::string GitusService::ReadObject(std::string object)
//{
//
//}

void GitusService::WriteIndex(const std::unique_ptr<std::map<std::string, IndexEntry>>& entries)
{
	using namespace std;
	using namespace boost;

	RawData entries_data;

	// The c++ standard guarantees that a map is iterated over in order of the keys
	for (auto it = entries->begin(); it != entries->end(); it++)
	{
		auto* entry = &it->second;

		for (int i = 0; i < entry->numFields; i++)
		{
			for(int j = 0; j < 4; j++)
			entries_data.push_back(entry->fields[i].c[j]);
		}

		for (int j = 0; j < Sha1Size; j++)
		entries_data.push_back(entry->sha1[j]);

		// add flags
		for (int j = 0; j < 2; j++)
			entries_data.push_back(entry->flags.c[j]);

		// Add path
		for (int j = 0; j < entry->path.size(); j++)
			entries_data.push_back(entry->path[j]);

		// Add padding
		auto pathOffset = ((EntryLength + entry->path.size() + 8) / 8) * 8;
		auto paddingLength = pathOffset - EntryLength - entry->path.size();
		for (int j = 0; j < entry->path.size(); j++)
			entries_data.push_back(0);

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
	data.insert(data.end(), entries_data.begin(), entries_data.end());

	auto digest = Utils::Sha1(data);

	filesystem::ofstream ofs{ IndexFile() };

	// Concat digest
	data.insert(data.end(), entries_data.begin(), entries_data.end());

	ofs << data.data();

};

std::unique_ptr<std::map<std::string, IndexEntry>> GitusService::ReadIndex()
{
	using namespace std;
	using namespace boost;

	auto entries =
		unique_ptr<map<string, IndexEntry>>(new map<string, IndexEntry>());

	if (filesystem::exists(IndexFile()))
	{
		auto data = Utils::ReadBytes(IndexFile().string());
		RawData digest(data.end() - 20, data.end());
		RawData content(data.begin(), data.end() - 20);

		auto content_hash = Utils::Sha1(content);
		if (*content_hash != digest) {
			throw std::exception("Index file got corrupted...");
		}

		RawData indexEntries(content.begin(), data.end() - HeaderLength);

		int i = 0;
		while (i + EntryLength < indexEntries.size())
		{
			RawData entryContent(indexEntries.begin() + i, indexEntries.begin() + i + EntryLength);
			RawData entryHeader(entryContent.begin(), entryContent.begin() + EntryHeaderLength);

			IndexEntry entry(entryHeader);
			entry.sha1 = RawData(entryContent.begin()+EntryHeaderLength, entryContent.begin()+Sha1Size);

			auto flagString = RawData(entryContent.begin()+50, entryContent.begin()+50+5);
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


std::string GitusService::CreateCommitTree() {
	auto index = GitusService::ReadIndex();
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
	auto treeHash = GitusService::HashObject(treeEntries, GitusService::Tree);
	return treeHash;
}

bool GitusService::HasParentTree() {
	auto parentTreePath = MasterFile();
	auto masterSize = boost::filesystem::file_size(parentTreePath);
	auto hashParent = masterSize == 0;
	return hashParent;
}

std::string GitusService::ParentTreeHash() {
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


