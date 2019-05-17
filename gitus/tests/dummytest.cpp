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

//
//BOOST_AUTO_TEST_CASE(Add)
//{
//	//Arrange
//	auto fileName = "testFile1.txt";
//	CreateFile(fileName, "random text");
//	AddCommand* add = new AddCommand(fileName);
//	//Act
//	add->Execute();
//
//	//Assert
//}



BOOST_AUTO_TEST_SUITE_END()



/*

	boost::filesystem::create_directory(GitusUtils::GitusDirectory());
	boost::filesystem::create_directory(GitusUtils::ObjectsDirectory());
	boost::filesystem::create_directory(GitusUtils::RefsDirectory());
	boost::filesystem::create_directory(GitusUtils::HeadsDirectory());
	boost::filesystem::ofstream(GitusUtils::HeadFile().string());
	boost::filesystem::ofstream(GitusUtils::MasterFile().string());
	return true;


		BOOST_CHECK(x != 0.0f);
	BOOST_CHECK_EQUAL((int)x, 9);
*/

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