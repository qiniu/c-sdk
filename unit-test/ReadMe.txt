1.运行程序，请执行
  make
  ./test
2.如果需要生产xml文件，请执行
  make
  ./test -x
  然后将生产的文件"*.xml"与下列文件放在同一文件夹下，即可使用浏览器阅览测试信息
  CUnit-List.dtd
  CUnit-List.xsl
  CUnit-Run.dtd
  CUnit-Run.xsl
3.查看测试覆盖度，请执行
  make output
  或参看makefile内命令执行
  在生成的result文件夹下可以查看代码覆盖度的详细数据。
4.有待完善的测试
  一些测试单元会占用大量的测试时间，正处于有待完善的状态。
  因而默认状态下是没有加入测试集合中的，但是可以通过加入测书-b强制加入。
  ./test -b
5.原始测试只是注重了代码的覆盖程度，现在逐步加入对功能的测试
