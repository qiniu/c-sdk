#include <gtest/gtest.h>
#include "test.h"
#include "qiniu/http.h"

#ifndef WIN32
#include <time.h>
#endif

const char *Test_bucket = NULL;
const char *Test_Domain = NULL;

int main(int argc, char **argv)
{
	Qiniu_Servend_Init(-1);
	QINIU_ACCESS_KEY = getenv("QINIU_ACCESS_KEY");
	QINIU_SECRET_KEY = getenv("QINIU_SECRET_KEY");
	Test_bucket = getenv("QINIU_TEST_BUCKET");
	Test_Domain = getenv("QINIU_TEST_BUCKET_DOMAIN");

#ifdef WIN32
	srand(GetTickCount());
#else
	srand(time(NULL));
#endif

	::testing::InitGoogleTest(&argc, argv);

	return RUN_ALL_TESTS();
}
