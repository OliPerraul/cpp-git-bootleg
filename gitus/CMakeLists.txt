cmake_minimum_required(VERSION 3.12)
project(GitUS
    VERSION 1.0
    LANGUAGES C CXX
    DESCRIPTION "Client git")

enable_testing()

set(CXX_STANDARD 17)

add_subdirectory(git)
add_subdirectory(tests)

set(Boost_USE_STATIC_LIBS ON) # operraul: Added this line
# See https://stackoverflow.com/questions/18583800/cmake-error-the-following-boost-libraries-could-not-be-found

find_package(Boost REQUIRED COMPONENTS program_options)

add_executable(gitus gitus.cpp)

target_include_directories(gitus 
    PRIVATE 
        ${Boost_INCLUDE_DIRS}
)



target_link_libraries(gitus
    PRIVATE
		Boost::program_options
        git
)
