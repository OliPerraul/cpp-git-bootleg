#ifndef GITUS_SERVICE_H
#define GITUS_SERVICE_H

#include <iostream>
#include <sstream>

#include <memory>
#include <vector>


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/filesystem.hpp>

#define INDEX_FORMAT_VERSION 2

static const char DirCacheSignature[4] = { 'D', 'I', 'R', 'C' };
static const size_t HeaderLength = 12;
static const size_t EntryLength = 59;
// Highly simplified .git/index entry
//(working on windows was difficult to obtain all the info i wanted)
//	We only encode
//	Per Index
//		size
//		digest
//	Per entry
//		path
//		sha1

//https://mincong-h.github.io/2018/04/28/git-index/
struct IndexEntry
{
	// the last time a file’s metadata changed
	size_t ctime = 0;
	// nanosecond fractions
	size_t ctime_frac = 0;
	// the last time a file’s data changed
	size_t mtime = 0;
	// nanosecond fractions
	size_t mtime_frac = 0;
	// The device (disk) upon which file resides
	size_t device = 0;

	// The file inode number. 
	size_t inode_number = 0;

	// hexadecimal representation of file permission
	size_t permission_mode = 0;

	// The user identifier of the current user
	size_t uid = 0;

	// The group identifier of the current user
	size_t gid = 0;

	// file size in number of bytes
	size_t file_size = 0;

	std::string sha1;

	// set of flags which are used during merge operations
	unsigned short flags;

	std::string path;
	void SetEntryHeader(std::string header) {
		this->ctime = header[0];
		this->ctime_frac = header[1];
		this->mtime = header[2];
		this->mtime_frac = header[3];
		this->device = header[4];
		this->inode_number = header[5];
		this->permission_mode = header[6];
		this->uid = header[7];
		this->gid = header[8];
		this->file_size = header[9];
	}

	std::pair<std::string, IndexEntry> GetEntryPair() {
		auto pair = std::pair<std::string, IndexEntry>(this->path, *this);
		return pair;
	}
};

class GitusService {
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
		return boost::filesystem::current_path()/".git/index";
	}

	static boost::filesystem::path HeadFile()
	{
		return boost::filesystem::current_path()/".git/HEAD";
	}

	static boost::filesystem::path GitusDirectory()
	{
		return boost::filesystem::current_path()/".git/";
	}

	static boost::filesystem::path RefsDirectory()
	{
		return boost::filesystem::current_path()/".git/refs/";
	}

	static boost::filesystem::path HeadsDirectory()
	{
		return boost::filesystem::current_path()/".git/refs/heads/";
	}

	static boost::filesystem::path MasterFile()
	{
		return boost::filesystem::current_path()/".git/refs/heads/master";
	}

	static boost::filesystem::path ObjectsDirectory()
	{
		return boost::filesystem::current_path()/".git/objects/";
	}


	static std::string Sha1(const std::vector<char>& object)
	{
		boost::uuids::detail::sha1 sha1;
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

/*
		"""Compute hash of object data of given type and write to object store
			if "write" is True.Return SHA - 1 object hash as hex string.
				"""
				header = '{} {}'.format(obj_type, len(data)).encode()
				full_data = header + b'\x00' + data
				sha1 = hashlib.sha1(full_data).hexdigest()
				if write:
		path = os.path.join('.git', 'objects', sha1[:2], sha1[2:])
			if not os.path.exists(path) :
				os.makedirs(os.path.dirname(path), exist_ok = True)
				write_file(path, zlib.compress(full_data))
				return sha1
*/

	// Simplified hashing
	static std::string HashObject(std::string object, ObjectHashType type, bool write=true)
	{
		using namespace std;
		using namespace boost;

		std::string header;
		switch (type)
		{
		case GitusService::Blob:
			header = "blob";
			break;
		case GitusService::Commit:
			header = "commit";
			break;
		case GitusService::Tree:
			header = "tree";
			break;
		default:
			break;
		}

		std::string content = header + object;
		auto sha1 = Sha1(*GetChars(content));
		if (write)
		{
			std::string first = sha1.substr(0, 2);
			std::string last = sha1.substr(2, string::npos);

			auto filePath = ObjectsDirectory()
				/ first;

			if (!filesystem::exists(filePath)) {
				filesystem::create_directories(filePath);
				filesystem::ofstream ofs{ filePath / last };
				ofs << content;
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

	static std::string ReadObject(std::string object)
	{

	}

	// Extracts non null chars from null terminated string
	static std::unique_ptr<std::vector<char>> GetChars(const std::string& str)
	{
		auto bytes = std::unique_ptr<std::vector<char>>(new std::vector<char>());
		// Get chars without null-terminated char
		for (int i = 0; i < str.size() - 1; i++)
		{
			bytes->push_back(str.data()[i]);
		}

		return bytes;
	}
	
	
	/*
	def write_index(entries):
    """Write list of IndexEntry objects to git index file."""
    packed_entries = []
    for entry in entries:
        entry_head = struct.pack('!LLLLLLLLLL20sH',
                entry.ctime_s, entry.ctime_n, entry.mtime_s, entry.mtime_n,
                entry.dev, entry.ino, entry.mode, entry.uid, entry.gid,
                entry.size, entry.sha1, entry.flags)
        path = entry.path.encode()
        length = ((62 + len(path) + 8) // 8) * 8
        packed_entry = entry_head + path + b'\x00' * (length - 62 - len(path))
        packed_entries.append(packed_entry)
    header = struct.pack('!4sLL', b'DIRC', 2, len(entries))
    all_data = header + b''.join(packed_entries)
    digest = hashlib.sha1(all_data).digest()
	write_file(os.path.join('.git', 'index'), all_data + digest)
	*/

	// header = struct.pack('!4sLL', b'DIRC', 2, len(entries))
	// TODO do not care about null terminated strings?
	static void WriteIndex(const std::unique_ptr<std::map<std::string, IndexEntry>>& entries)
	{
		using namespace std;
		using namespace boost;

		// number of entries
		string entries_data;

		//content.append(std::to_string(entries->size()));
		// Write entries to file, filling up unknowns with 0s
		char buffer[4];
		size_t fields[10];
		for (auto it = entries->begin(); it != entries->end(); it++)
		{
			auto* entry = &it->second;
			fields[0] = entry->ctime;
			fields[1] = entry->ctime_frac;
			fields[2] = entry->mtime;
			fields[3] = entry->mtime_frac;
			fields[4] = entry->device;
			fields[5] = entry->inode_number;
			fields[6] = entry->permission_mode;
			fields[7] = entry->uid;
			fields[8] = entry->gid;
			fields[9] = entry->file_size;

			for (int i = 0; i < 10; i++)
			{
				auto headerProperty = fields[i];
				entries_data += std::to_string(headerProperty);
			}

			entries_data += entry->sha1;

			entries_data += std::to_string(entry->flags);

			entries_data += entry->path;
			auto entryPadding = GenerateEntryPadding(entry->path);
			entries_data += entryPadding;
		}

		// A 12 - byte header consisting of a 4 - byte signature “DIRC”(0x44495243) 
		// standing for DirCache; 4 - byte version number “2”(0x00000002), which is 
		// the current version of Git index format; 32 - bit number of index entries

		string header;

		// Signature
		copy(
			begin(DirCacheSignature),
			end(DirCacheSignature),
			back_inserter(header));

		// Format
		size_t field = INDEX_FORMAT_VERSION;
		memcpy(buffer, &field, 4);
		copy(
			begin(buffer),
			end(buffer),
			back_inserter(header));

		// Number of entries
		field = entries->size();
		std::memcpy(buffer, &field, 4);
		copy(
			begin(buffer),
			end(buffer),
			back_inserter(header));
	
		string data = header + entries_data;
		auto digest = Sha1(*GetChars(data));
		std::ofstream outfile(IndexFile().string(), std::ofstream::binary);
		auto outputText = data + digest; //TODO null terminated ??
		outfile << outputText;
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

		auto entries = unique_ptr<std::map<std::string, IndexEntry>>(new std::map<std::string, IndexEntry>());

		if (filesystem::exists(IndexFile()))
		{
			auto data = ReadBytes(IndexFile().string());
			std::size_t fileLength = data.size();
			string digest = data.substr(fileLength - 40);
			string content = data.substr(0, fileLength  - 40);
			string contentHash = Sha1(*GetChars(content));
			if (contentHash != digest) {
				throw std::exception("Index file got corrupted...");
			}
			
			string indexEntries = content.substr(HeaderLength);
			int i = 0;
			while (i + EntryLength < indexEntries.size()) {
				auto entryEnd = i + EntryLength;
				auto entryContent = indexEntries.substr(i, entryEnd);
				auto entryHeader = entryContent.substr(0, 10);
				
				IndexEntry* entry = new IndexEntry;
				entry->SetEntryHeader(entryHeader);
				entry->sha1 = entryContent.substr(10, 40);
				//TODO:GET HOW FLAG GENERATED, ADD PADDING IF Required
				auto flagString = entryContent.substr(50, 5);
				entry->flags = std::stoul(flagString);

				auto possiblePathRange = indexEntries.substr(55);
				auto endOfPathIndex = possiblePathRange.find('\0');
				endOfPathIndex = endOfPathIndex == std::string::npos ? 4 : endOfPathIndex;
				entry->path = possiblePathRange.substr(0, endOfPathIndex);

				auto insertPair = entry->GetEntryPair();
				entries->insert(insertPair);

				auto currentEntryLength = ((EntryLength + entry->path.size() + 8) / 8) * 8;
				i += currentEntryLength;
			}
			//auto stream = ifstream(IndexFile().string());
			//string line;
			//getline(stream, line); // TODO: use the size? (first line)
			//istringstream iss(line);
			//int size;
			//iss >> size;

			//for (int i = 0; i < size; i++)
			//{
			//	IndexEntry entry;
			//	getline(stream, line);
			//	iss = istringstream(line);
			//	iss >> entry.sha1;
			//	getline(stream, line);
			//	iss = istringstream(line);
			//	iss >> entry.sha1;

			//	entries->push_back(entry);
			//}

		}

		return entries;
	};
	static std::string CreateCommitTree() {
		auto index = GitusService::ReadIndex();
		std::string treeEntries;
		for (auto it = index->begin(); it != index->end(); it++)
		{
			std::string currentEntry;
			auto* entry = &it->second;
			currentEntry += entry->permission_mode + " ";
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
};


#endif

