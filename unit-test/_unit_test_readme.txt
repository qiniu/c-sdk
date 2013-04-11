1.用例的添加与删除在c_unit_test_main.c中进行，运行的方式也可在其中修改
2.如果使用生成xml文件的模式，生成的xml文件需要与如下文件凡在同一文件夹下，然后可以是使用那个浏览器查看结果（如firefox）;
  CUnit-List.dtd
  CUnit-List.xsl
  CUnit-Run.dtd
  CUnit-Run.xsl
3.程序的运行方法：
  i  make
  ii ./test
4.将生成的代码覆盖率文件转换为网页的方法：在3之后输入如下指令
  i  lcov --directory .   --capture --output-file rs_app.info
  ii genhtml -o result rs_app.info
  生成的文件在result中查看
5.AddTestsUpDemoResumable用例中
  test_by_up_demo_resumable_err18测试会占用大量时间，可以在下面代码处注释掉
  {"Testing up_resumable expecting err18:", test_by_up_demo_resumable_err18}
