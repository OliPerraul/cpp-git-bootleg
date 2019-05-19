#ifndef GITUS_SERVICE_H
#define GITUS_SERVICE_H

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

#include "utils.h"


//https://mincong-h.github.io/2018/04/28/git-index/
struct IndexEntry
{
	size_t numFields = 10;

	//	Index entries field packed in vector for easy storage to disk
	//		mtime: the last time a file's metadata changed
	//		mtime_fraction: nanosecond fractions
	//		ctime: the last time a file's data changed
	//		ctime_fraction: nanosecond fractions
	//		device: The device (disk) upon which file resides
	//		inode_number: The file inode number. 
	//		permission: hexadecimal representation of file permission
	//		uid: The user identifier of the current user
	//		gid: The group identifier of the current user
	//		size: file size in number of bytes
	std::vector<Word2> fields;

	// flags: flags used for validation
	union Word flags;

	// sha1: contains the hashed version of the file
	RawData sha1;

	// path: path of the file in the repository
	std::string path;

	// permission: see above
	union Word2 Permission()
	{
		return fields[6];
	}

	IndexEntry()
	{
		Word2 buffer;
		buffer.n = 0;
		for (int i = 0; i < numFields; i++)
		{
			fields.push_back(buffer);
		}

		flags.n = 0;

	};

	IndexEntry(const RawData& header) {

		Word2 buffer;
		for (int i = 0; i < numFields; i++)
		{
			buffer.n = 0;
			std::copy(header.data()+i, header.data()+i+4, buffer.c);
			fields.push_back(buffer);
		}

		flags.n = 0;
	}
};

class GitusService {

private:
	boost::filesystem::path _currentGitusDirectory;

public:

	enum  ObjectHashType
	{
		Blob,
		Commit,
		Tree
	};

	static boost::filesystem::path NewGitusDirectory()
	{
		return boost::filesystem::current_path() / ".git" / "";
	}

	// Iterate over directories from current to find the '.git' directory
	bool CacheCurrentGitusDirectory()
	{
		using namespace boost;

		filesystem::path dir = filesystem::current_path();		
		for (filesystem::recursive_directory_iterator it(dir); it != filesystem::recursive_directory_iterator(); it++)
		{
			if (it->path().filename().string() == ".git")
			{
				_currentGitusDirectory = it->path();
				return true;
			}
		}

		return false;
	}

	boost::filesystem::path RepoDirectory()
	{
		return _currentGitusDirectory.parent_path();
	}

	boost::filesystem::path IndexFile()
	{
		return _currentGitusDirectory / "index";
	}

	boost::filesystem::path HeadFile()
	{
		return _currentGitusDirectory / "HEAD";
	}

	boost::filesystem::path RefsDirectory()
	{
		return _currentGitusDirectory / "refs" / "";
	}

	boost::filesystem::path HeadsDirectory()
	{
		return _currentGitusDirectory / "refs" / "heads" / "";
	}

	boost::filesystem::path MasterFile()
	{
		return _currentGitusDirectory / "refs" / "heads" / "master" / "";
	}

	boost::filesystem::path ObjectsDirectory()
	{
		return _currentGitusDirectory / "objects" / "";
	} 

	bool HashObject(const RawData& object, ObjectHashType type, bool write, RawData& sha1);

	bool WriteIndex(const std::map<std::string, IndexEntry>& entries);

	bool ReadIndex(std::map<std::string, IndexEntry>& entries);

	RawData HashCommitTree();

	bool HasParentTree();

	bool LocalMasterHash(RawData& hash);

	bool ObjectExists(std::string sha1String);
	static RawData CreateContentData(const RawData& object, ObjectHashType type);
	static RawData CreateHeaderData(GitusService::ObjectHashType type, const RawData & object);

};


#endif

