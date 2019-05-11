add_library(git
    dummy.h 
    dummy.cpp)

target_include_directories(git 
    PUBLIC 
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
)