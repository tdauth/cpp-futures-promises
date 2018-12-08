#include <folly/init/Init.h>

/**
 * The initialization is required for the delayed tasks.
 */
struct Fixture
{
	Fixture()
	    : argc(boost::unit_test::framework::master_test_suite().argc),
	      argv(boost::unit_test::framework::master_test_suite().argv)
	{
		/*
		 * Does already initialize glog.
		 */
		folly::init(&argc, &argv);
	}

	int argc;
	char **argv;
};

BOOST_GLOBAL_FIXTURE(Fixture);