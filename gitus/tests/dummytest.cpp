#define BOOST_TEST_MODULE GitTests

#include <boost/test/unit_test.hpp>
#include <boost/filesystem.hpp>
#include "../utils.h"
#include "../commands.cpp"

void CleanUp();
void DeleteFile(std::string fileName);
void CreateFile(std::string fiileName, std::string content);

BOOST_AUTO_TEST_SUITE(Tests)

BOOST_AUTO_TEST_CASE(InitNormal)
{
	//Arrange
	InitCommand* init = new InitCommand();
	auto gitusDir = GitusUtils::GitusDirectory();
	auto objDir = GitusUtils::ObjectsDirectory();
	auto refDir = GitusUtils::RefsDirectory();
	auto headDir = GitusUtils::HeadsDirectory();
	auto headFile = GitusUtils::HeadFile();
	auto masterFile = GitusUtils::MasterFile();

	//Act
	init->Execute();
	
	//Assert
	auto gitusDirExist = boost::filesystem::exists(gitusDir);
	auto objDirExist = boost::filesystem::exists(objDir);
	auto refDirExist = boost::filesystem::exists(refDir);
	auto headDirExist = boost::filesystem::exists(headDir);
	auto headFileSize = boost::filesystem::file_size(headFile);
	auto masterFileSize = boost::filesystem::file_size(masterFile);

	BOOST_CHECK(gitusDirExist);
	BOOST_CHECK(objDirExist);
	BOOST_CHECK(refDirExist);
	BOOST_CHECK(headDirExist);
	BOOST_CHECK_EQUAL(headFileSize, 0);
	BOOST_CHECK_EQUAL(masterFileSize, 0);
	
	CleanUp();
}

BOOST_AUTO_TEST_CASE(InitArleadyInitted)
{
	//Arrange
	InitCommand* init = new InitCommand();
	auto path = boost::filesystem::current_path();
	//Act
	init->Execute();
	
	//Assert
	BOOST_CHECK(!init->Execute());
	CleanUp();

}


BOOST_AUTO_TEST_CASE(Add)
{
	//Arrange
	auto fileName = "testFile1.txt";
	CreateFile(fileName, "random text");
	auto file = GitusUtils::ReadFile(fileName);
	
	AddCommand* add = new AddCommand(fileName);
	auto fileHash = GitusUtils::HashObject(file, GitusUtils::Blob, false);
	auto fileDirHead = fileHash.substr(0, 2);
	auto fileDirBody = fileHash.substr(2);
	auto objDir = GitusUtils::ObjectsDirectory();
	auto filePath = objDir / fileDirHead / fileDirBody;
	//Act
	add->Execute();

	//Assert
	auto filePath =
}

/*

init
 -chacun de repo sont crer
 -init dans un deja init

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
	auto gitusPath = GitusUtils::GitusDirectory();
	boost::filesystem::remove_all(gitusPath);
}

void DeleteFile(std::string fileName) {
	boost::filesystem::remove(fileName);
}

void CreateFile(std::string fileName, std::string content) {
	auto ofs = boost::filesystem::ofstream(fileName);
	ofs << content + "\0";
}