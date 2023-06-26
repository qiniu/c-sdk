#include <gtest/gtest.h>
#include "qiniu/recorder_key.h"

TEST(UnitTest, TestRecorderKey_New_Append_Generate)
{
	Qiniu_Recorder_Key_Generator key_gen = Qiniu_Recorder_Key_Generator_New();
	Qiniu_Recorder_Key_Generator_Append(&key_gen, "hello");
	Qiniu_Recorder_Key_Generator_Append(&key_gen, "world");
	char *key = (char *) Qiniu_Recorder_Key_Generator_Generate(&key_gen);
	EXPECT_STREQ("1dd410057b549b65ccce6bc837b23022", key);
	Qiniu_Recorder_Key_Generator_Free(key_gen);
	free(key);
}
