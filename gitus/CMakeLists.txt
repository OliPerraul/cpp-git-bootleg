cmake_minimum_required(VERSION 3.12)
project(GitUS
    VERSION 1.0
    LANGUAGES C CXX
    DESCRIPTION "Client git")

enable_testing()

set(CXX_STANDARD 17)

add_subdirectory(git)
add_subdirectory(tests)


set(BOOST_INCLUDEDIR C:/boost_1_70_0/)
set(BOOST_LIBRARYDIR C:/boost_1_70_0/stage/lib)

set(Boost_INCLUDE_DIRS C:/boost_1_70_0/)
set(Boost_LIBRARY_DIRS C:/boost_1_70_0/stage/lib)

set(Boost_USE_STATIC_LIBS ON) # operraul: Added this line
set( Boost_USE_MULTITHREADED ON )


# See https://stackoverflow.com/questions/18583800/cmake-error-the-following-boost-libraries-could-not-be-found
add_definitions(-DBOOST_ALL_NO_LIB)

find_package(Boost REQUIRED COMPONENTS program_options filesystem zlib iostreams)
message("boost lib: ${Boost_LIBRARIES}")

add_executable(gitus commands.h utils.h commands.cpp gitus.cpp)

target_include_directories(gitus 
    PRIVATE 
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(gitus
        ${Boost_LIBRARIES}

)


