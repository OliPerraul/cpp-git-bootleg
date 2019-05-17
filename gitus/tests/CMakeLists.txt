
set(Boost_USE_STATIC_LIBS ON) # operraul: Added this line
# See https://stackoverflow.com/questions/18583800/cmake-error-the-following-boost-libraries-could-not-be-found

set(Boost_USE_STATIC_LIBS ON) 


find_package(Boost REQUIRED COMPONENTS unit_test_framework filesystem zlib iostreams)

add_executable(gittests dummytest.cpp)

target_include_directories(gittests 
    PRIVATE 
        ${Boost_INCLUDE_DIRS}
)

target_link_libraries(gittests
    PRIVATE
        ${Boost_LIBRARIES}
)

add_test(all gittests)
