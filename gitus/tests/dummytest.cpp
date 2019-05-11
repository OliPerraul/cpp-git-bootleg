#define BOOST_TEST_MODULE GitTests

#include <boost/test/unit_test.hpp>

#include <dummy.h>


BOOST_AUTO_TEST_SUITE(Tests)

BOOST_AUTO_TEST_CASE(Dummy)
{
    BOOST_CHECK_EQUAL(GetAnswerToLifeUniverseAndEverything(), 42);
}

BOOST_AUTO_TEST_SUITE_END()