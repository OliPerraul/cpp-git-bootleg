
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
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(gitusDirExist);
	BOOST_CHECK(objDirExist);
	BOOST_CHECK(refDirExist);
	BOOST_CHECK(headDirExist);
	BOOST_CHECK(headFileExist);
	BOOST_CHECK(masterFileExist);
	BOOST_CHECK_EQUAL(headFileSize, 26);
	BOOST_CHECK_EQUAL(masterFileSize, 0);

	CleanUp();
}

BOOST_AUTO_TEST_CASE(InitArleadyInit)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	InitCommand* init = new InitCommand(gitus);

	//Act
	init->Execute();
	auto res = init->Execute();

	//Assert
	BOOST_CHECK(res);

	CleanUp();
}

BOOST_AUTO_TEST_CASE(AddSingleFile)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto filePath1 = GetFileObjPath(fileName);

	AddCommand* add = new AddCommand(gitus, fileName);
	InitCommand* init = new InitCommand(gitus);
	init->Execute();
	//Act
	add->Execute();
	//Assert
	auto fileObjectCreated1 = boost::filesystem::exists(filePath1);
	auto indexCreated = boost::filesystem::file_size(gitus->IndexFile()) > 0;
	BOOST_CHECK(fileObjectCreated1);
	BOOST_CHECK(indexCreated);

	CleanUp();
	DeleteFile(fileName);
}

BOOST_AUTO_TEST_CASE(AddMultipleFiles)
{
	auto gitus = std::shared_ptr<GitusService>(new GitusService);

	//Arrange
	auto fileName = "testFile1.txt";
	auto fileName2 = "testFile21.txt";
	CreateFile(fileName, "random text");
	CreateFile(fileName2, "randodasddsadasdsdsam text");
	auto filePath1 = GetFileObjPath(fileName);
	auto filePath2 = GetFileObjPath(fileName2);
	
	AddCommand* add = new AddCommand(gitus, fileName);
	AddCommand* add2 = new AddCommand(gitus, fileName2);
	InitCommand* init = new InitCommand(gitus);

	init->Execute();
	//Act
	add->Execute();
	add2->Execute();

	//Assert
	auto fileObjectCreated1 = boost::filesystem::exists(filePath1);
	auto fileObjectCreated2 = boost::filesystem::exists(filePath2);
	BOOST_CHECK(fileObjectCreated1);
	BOOST_CHECK(fileObjectCreated2);

	CleanUp();
	DeleteFile(fileName);
	DeleteFile(fileName2);
}

BOOST_AUTO_TEST_CASE(AddSameFile)
{
	auto gitus = std::shared_ptr<GitusService>(new GitusService);

	//Arrange
	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto filePath1 = GetFileObjPath(fileName);

	AddCommand* add = new AddCommand(gitus, fileName);

	InitCommand* init = new InitCommand(gitus);
	init->Execute();
	//Act
	add->Execute();
	auto res =add->Execute();

	//Assert
	BOOST_CHECK(!res);
	
	CleanUp();
	DeleteFile(fileName);
}
BOOST_AUTO_TEST_CASE(AddNotExistingFile)
{
	auto gitus = std::shared_ptr<GitusService>(new GitusService);

	//Arrange
	auto fileName = "testFile1.txt";

	AddCommand* add = new AddCommand(gitus, fileName);
	InitCommand* init = new InitCommand(gitus);

	init->Execute();
	//Act
	auto res = add->Execute();

	//Assert
	BOOST_CHECK(!res);
	
	CleanUp();
	DeleteFile(fileName);
}

BOOST_AUTO_TEST_CASE(CommitBasic)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto gitusDir = GitusService::NewGitusDirectory();
	auto masterFile = gitusDir / "refs/heads/master";

	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto filePath1 = GetFileObjPath(fileName);

	AddCommand* add = new AddCommand(gitus, fileName);
	InitCommand* init = new InitCommand(gitus);
	CommitCommand* commit = new CommitCommand(gitus, "First Commit", "Me", "Me@yahoo.ca");

	init->Execute();
	add->Execute();
	//Act
	auto res = commit->Execute();

	//Assert
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(masterFileSize > 0);
	BOOST_CHECK(res);

	CleanUp();
	DeleteFile(fileName);
}

BOOST_AUTO_TEST_CASE(CommitMultiple)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto gitusDir = GitusService::NewGitusDirectory();
	auto masterFile = gitusDir / "refs/heads/master";

	auto fileName = "testFile1.txt";
	auto fileName2 = "testFile21.txt";
	CreateFile(fileName, "random text");
	CreateFile(fileName2, "randodasddsadasdsdsam text");
	auto filePath1 = GetFileObjPath(fileName);
	auto filePath2 = GetFileObjPath(fileName2);

	AddCommand* add = new AddCommand(gitus, fileName);
	AddCommand* add2 = new AddCommand(gitus, fileName2);

	InitCommand* init = new InitCommand(gitus);
	CommitCommand* commit = new CommitCommand(gitus, "First Commit", "Me", "Me@yahoo.ca");

	init->Execute();
	add->Execute();
	//Act
	commit->Execute();
	add2->Execute();
	auto res = commit->Execute();
	//Assert
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(masterFileSize > 0);
	BOOST_CHECK(res);

	CleanUp();
	DeleteFile(fileName);
	DeleteFile(fileName2);
}

BOOST_AUTO_TEST_CASE(CommitNoFile)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto gitusDir = GitusService::NewGitusDirectory();
	auto masterFile = gitusDir / "refs/heads/master";

	InitCommand* init = new InitCommand(gitus);
	CommitCommand* commit = new CommitCommand(gitus, "First Commit", "Me", "Me@yahoo.ca");

	init->Execute();
	//Act
	auto res = commit->Execute();

	//Assert
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(masterFileSize == 0);
	BOOST_CHECK(!res);

	CleanUp();
}

BOOST_AUTO_TEST_CASE(CommitSameIndexAfterCommit)
{
	//Arrange
	auto gitus = std::shared_ptr<GitusService>(new GitusService);
	auto gitusDir = GitusService::NewGitusDirectory();
	auto masterFile = gitusDir / "refs/heads/master";

	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto filePath1 = GetFileObjPath(fileName);

	AddCommand* add = new AddCommand(gitus, fileName);
	InitCommand* init = new InitCommand(gitus);
	CommitCommand* commit = new CommitCommand(gitus, "First Commit", "Me", "Me@yahoo.ca");

	init->Execute();
	add->Execute();
	//Act
	commit->Execute();
	auto res = commit->Execute();


	//Assert
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(masterFileSize > 0);
	BOOST_CHECK(!res);

	CleanUp();
	DeleteFile(fileName);
}

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

	auto fileBytes = Utils::ReadBytes(fileName);
	RawData fileContent = GitusService::CreateContentData(fileBytes, GitusService::ObjectHashType::Blob);
	std::string sha1String;
	Utils::Sha1String(fileContent, sha1String);
	
	auto objDir = gitus.ObjectsDirectory();

	auto filePath = GitusService::NewGitusDirectory() / objDir / sha1String.substr(0, 2) / sha1String.substr(2);
	return filePath;
}