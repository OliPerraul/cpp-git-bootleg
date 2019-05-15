
#include <memory>
#include <iostream>

// Linux only
//#include <stat.h>
#include <sys/stat.h>
#ifdef _WIN32

#elif UNIX

#endif

#include <boost/filesystem.hpp>

#include "gitus_service.h"
#include "commands.h"



//-- Init

bool InitCommand::_FileExist(std::string filePath) {
	bool exist = boost::filesystem::exists(filePath);
	return exist;

}

bool InitCommand::_IsArleadyInitialize() {
	boost::filesystem::path p = ".git/";
	return boost::filesystem::is_directory(p);
}


bool InitCommand::Execute() {

	if (_IsArleadyInitialize()) {
		return false;
	}

	boost::filesystem::create_directory(GitusService::GitusDirectory());
	boost::filesystem::create_directory(GitusService::ObjectsDirectory());
	boost::filesystem::create_directory(GitusService::RefsDirectory());
	boost::filesystem::create_directory(GitusService::HeadsDirectory());
	boost::filesystem::ofstream(GitusService::HeadFile().string());
	boost::filesystem::ofstream(GitusService::MasterFile().string());
	return true;
}



//-- Add
/*
	"""Add all file paths to git index."""
	paths = [p.replace('\\', '/') for p in paths]
	all_entries = read_index()
	entries = [e for e in all_entries if e.path not in paths]
	for path in paths:
		sha1 = hash_object(read_file(path), 'blob')
		st = os.stat(path)
		flags = len(path.encode())
		assert flags < (1 << 12)
		entry = IndexEntry(
				int(st.st_ctime), 0, int(st.st_mtime), 0, st.st_dev,
				st.st_ino, st.st_mode, st.st_uid, st.st_gid, st.st_size,
				bytes.fromhex(sha1), flags, path)
		entries.append(entry)
	entries.sort(key=operator.attrgetter('path'))
	write_index(entries)
*/

/*
	for(auto entry = entries->begin(); entry!= entries->end; entry++)
	{

	}
*/

bool AddCommand::Execute() {

	auto entries = GitusService::ReadIndex();

	

// TODO enonce du devoir: Spécification technique: 'code portable'
// TODO IndexEntry representation memoire dificile..
//#ifdef _WIN32
//	struct _stat buf;
//	_stat(_pathspec.c_str(), &buf);
//#elif UNIX
//
//#endif

	// Add new entry
	// TODO Fill up the other fields if IndexEntry?
	IndexEntry entry;
	entry.path = _pathspec;
	entry.sha1 = GitusService::HashObject(GitusService::ReadFile(_pathspec), GitusService::Blob, true);
	if (entries->count(_pathspec) != 0) {
		auto indexedEntry = entries->at(_pathspec);
		if (indexedEntry.sha1 == entry.sha1) {
			std::cout << "The specified file is arleady added" << std::endl;
			return;
		}
		entries->erase(_pathspec);
	}

	auto entryPair = entry.GetEntryPair();
	entries->insert(entryPair);

	// Sort entries
	sort(entries->begin(), entries->end(),
		[](const IndexEntry & a, const IndexEntry & b) -> bool
	{
		return a.path > b.path;
	});

	GitusService::WriteIndex(entries);

	return true;
}




//--- Commit

bool CommitCommand::Execute() {
	int i = 0;
	return true;
}

