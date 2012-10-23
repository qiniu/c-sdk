编译：make
运行：./run_test

注意：编译之前需要确认run_test.c中的QBOX_ACCESS_KEY和QBOX_SECRET_KEY正确。

说明：增加新的测试用例步骤
1. 新建一个任意名称的.c文件或在已有的.c文件中添加测试函数。
2. 测试函数必须符合这种形式：int funcname(QBox_Client* client)。
3. 打开qbox_test.c定位到39行，添加DEFNODE(funcname);和ADDNODE(funcname);即可。
