
cmake_minimum_required(VERSION 3.12)
project(GitUS
    VERSION 1.0
    LANGUAGES C CXX
    DESCRIPTION "Client git")

enable_testing()

set(CXX_STANDARD 17)

add_subdirectory(git)
add_subdirectory(tests)

add_executable(gitus gitus.cpp)

target_link_libraries(gitus
    PRIVATE
        git
)

