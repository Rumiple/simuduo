#!/bin/bash
set -e
# ���û��buildĿ¼��������Ŀ¼
if [ ! -d $(pwd)/build ]; then
	mkdir $(pwd)/build
fi
rm -rf $(pwd)/build/* # ��Ŀ¼�µĶ���ɾ��
# ����
cd $(pwd)/build && 
	cmake .. &&
	make
# �ص���Ŀ��Ŀ¼
cd ..
# ��ͷ�ļ������� /usr/include/simuduo , so�⿽����/usr/lib
if [ ! -d /usr/include/simuduo ]; then
	mkdir /usr/include/simuduo
fi
# ����ͷ�ļ�
for header in $(ls *.h)
do 
	cp $header /usr/include/simuduo
done
cp $(pwd)/lib/libsimuduo.so /usr/lib
ldconfig # ˢ�¶�̬�⻺��