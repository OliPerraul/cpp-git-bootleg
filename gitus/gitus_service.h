#ifndef GITUS_SERVICE_H
#define GITUS_SERVICE_H

#include <iostream>
#include <sstream>

#include <memory>
#include <vector>
//#include <zl>


#include <boost/uuid/uuid.hpp>
#include <boost/uuid/detail/sha1.hpp>
#include <boost/filesystem.hpp>


// Highly simplified .git/index entry
//(working on windows was difficult to obtain all the info i wanted)
//	We only encode 
//	Per Index
//		size
//		digest
//	Per entry
//		path
//		sha1
struct IndexEntry
{
	std::string sha1;
	std::string path;
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
		return boost::filesystem::current_path().concat(".git/index").string();
	}

	static boost::filesystem::path HeadFile()
	{
		return boost::filesystem::current_path().concat(".git/HEAD").string();
	}

	static boost::filesystem::path GitusDirectory()
	{
		return boost::filesystem::current_path().concat(".git/").string();
	}

	static boost::filesystem::path RefsDirectory()
	{
		return boost::filesystem::current_path().concat(".git/refs/").string();
	}

	static boost::filesystem::path HeadsDirectory()
	{
		return boost::filesystem::current_path().concat(".git/refs/heads/").string();		
	}

	static boost::filesystem::path MasterFile()
	{
		return boost::filesystem::current_path().concat(".git/refs/heads/master").string();
	}

	static boost::filesystem::path ObjectsDirectory()
	{
		return boost::filesystem::current_path().concat(".git/objects/").string();
	}


	static std::string Sha1(std::string object)
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

		auto sha1 = Sha1(content);
		if (write)
		{
			auto file = ObjectsDirectory()
				.append(filesystem::path(sha1.substr(0, 2)))
				.append(sha1.substr(2, string::npos)) ;

			auto stream = std::fstream(file.string());
			stream << content;	
		}

		return sha1;
	}


	static std::string ReadObject(std::string object)
	{

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
	static void WriteIndex(const std::unique_ptr<std::vector<IndexEntry>>& entries)
	{
		// number of entries
		std::string content;
		content.append(std::to_string(entries->size()));
		std::string delim("\n");
		for (auto entry = entries->begin(); entry != entries->end(); entry++)
		{
			content.append(delim + entry->sha1);
			content.append(delim + entry->path);
		}

		// 160 bit sha1 over the content (checksum)
		auto digest = Sha1(content);
		content = content.append(delim + digest);

		auto stream = std::fstream(IndexFile().string());
		stream << content;
	};

	static std::string ReadFile(std::string filename)
	{
		std::ifstream ifs(filename);
		std::string content((std::istreambuf_iterator<char>(ifs)),
			(std::istreambuf_iterator<char>()));
		return content;
	}



	static std::unique_ptr<std::vector<IndexEntry>> ReadIndex()
	{
		auto entries = std::unique_ptr<std::vector<IndexEntry>>(new std::vector<IndexEntry>());
		
		if (boost::filesystem::exists(IndexFile()))
		{
			auto stream = std::ifstream(IndexFile().string());
			std::string line;
			std::getline(stream, line); // TODO: use the size? (first line)
			std::istringstream iss(line);
			int size;
			iss >> size;

			for (int i = 0; i < size; i++)
			{
				IndexEntry entry;
				std::getline(stream, line);
				iss = std::istringstream(line);
				iss >> entry.sha1;

				std::getline(stream, line);
				iss = std::istringstream(line);
				iss >> entry.sha1;

				entries->push_back(entry);
			}

		}

		return entries;
	};


};

#endif

