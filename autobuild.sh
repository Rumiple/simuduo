#!/bin/bash
set -e
# 如果没有build目录，创建该目录
if [ ! -d $(pwd)/build ]; then
	mkdir $(pwd)/build
fi
rm -rf $(pwd)/build/* # 把目录下的东西删除
# 编译
cd $(pwd)/build && 
	cmake .. &&
	make
# 回到项目根目录
cd ..
# 把头文件拷贝到 /usr/include/simuduo , so库拷贝到/usr/lib
if [ ! -d /usr/include/simuduo ]; then
	mkdir /usr/include/simuduo
fi
# 拷贝头文件
for header in $(ls *.h)
do 
	cp $header /usr/include/simuduo
done
cp $(pwd)/lib/libsimuduo.so /usr/lib
ldconfig # 刷新动态库缓存