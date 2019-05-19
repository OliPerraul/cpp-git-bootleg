
#define BOOST_TEST_MODULE GitTests

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>

#include "../gitus_service.h"
#include "../commands.h"
#include "../utils.h"

void CleanUp();
void DeleteFile(std::string fileName);
void CreateFile(std::string fiileName, std::string content);
boost::filesystem::path GetFileObjPath(std::string filePath);

BOOST_AUTO_TEST_SUITE(Tests)


BOOST_AUTO_TEST_CASE(InitNormal)
{
	//Arrange
	auto gitusDir = GitusService::NewGitusDirectory();
	auto objDir = gitusDir / "objects/";
	auto refDir = gitusDir / "refs/";
	auto headDir = gitusDir / "refs/heads/";
	auto masterFile = gitusDir / "refs/heads/master";
	auto headFile = gitusDir / "HEAD";

	//Act
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	InitCommand* init = new InitCommand(gitus);
	init->Execute();
	
	//Assert
	auto gitusDirExist = boost::filesystem::exists(gitusDir);
	auto objDirExist = boost::filesystem::exists(objDir);
	auto refDirExist = boost::filesystem::exists(refDir);
	auto headDirExist = boost::filesystem::exists(headDir);
	auto headFileExist = boost::filesystem::exists(headFile);
	auto masterFileExist = boost::filesystem::exists(masterFile);

	auto headFileSize = boost::filesystem::file_size(headFile);
	auto masterFileSize = boost::filesystem::file_size(headFile);

	BOOST_CHECK(gitusDirExist);
	BOOST_CHECK(objDirExist);
	BOOST_CHECK(refDirExist);
	BOOST_CHECK(headDirExist);
	BOOST_CHECK(headFileExist);
	BOOST_CHECK(masterFileExist);
	BOOST_CHECK_EQUAL(headFileSize, 0);
	BOOST_CHECK_EQUAL(masterFileSize, 0);

	CleanUp();
}

BOOST_AUTO_TEST_CASE(Add)
{
	auto gitus = std::shared_ptr<GitusService>(new GitusService);

	//Arrange
	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto file = Utils::ReadBytes(fileName);
	
	AddCommand* add = new AddCommand(gitus, fileName);
	auto filePath = GetFileObjPath(fileName);

	//Act
	add->Execute();

	//Assert
	auto fileObjectCreated = boost::filesystem::exists(filePath);
	BOOST_CHECK(fileObjectCreated);

	CleanUp();
	DeleteFile(fileName);
}

/*

init
 -chacun de repo sont crer
 -init dans un deja init
 {m_pathname=L"C:\\Users\\zeroeagle1\\CMakeBuilds\\9ea20e7e-e12c-ff34-9a63-2204612524e4\\build\\x64-Debug\\tests\\.git/objects/e2\\b64f6bf69d2916af31f293da31c10d6a575a0b" }

 add
 -/ob/xx/xxxxxxxx bien crer
 - index bien crer
 - add dun file deja ddd
 - add dun file existant pas

commit
 - creation new commit obj/xx/xx
 - commit mais rien en staging
 - second commit

other
--help
- weird param inserted


*/


BOOST_AUTO_TEST_SUITE_END()

void CleanUp() {
	auto gitusPath = GitusService::NewGitusDirectory();
	boost::filesystem::remove_all(gitusPath);
}

void DeleteFile(std::string fileName) {
	boost::filesystem::remove(fileName);
}

void CreateFile(std::string fileName, std::string content) {
	boost::filesystem::ofstream ofs{ fileName };
	ofs << content + "\0";
}

// Retrieve the file path by reading the object file
boost::filesystem::path GetFileObjPath(std::string fileName) {
	
	using namespace std;
	
	GitusService gitus;

	auto fileContent = Utils::ReadBytes(fileName);
	RawData fileHash;
	gitus.HashObject(fileContent, GitusService::Blob, false, fileHash);
	
	stringstream ss;
	ss << hex << fileHash.data();
	string shaString = ss.str();
	
	auto objDir = gitus.ObjectsDirectory();

	auto filePath = objDir / shaString.substr(0, 2) / shaString.substr(2);
	return filePath;
}