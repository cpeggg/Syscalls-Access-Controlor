#!/bin/bash
echo 本shell脚本用于测试Access Control及Access Audit的功能
echo 首先拷贝动态权限的配置文件至/root，表示该配置文件只能由系统的根用户创建并修改，内核模块读取的也是该文件
sudo cp ./AccessControl.conf /root/
sudo chown root /root/AccessControl.conf
sudo chmod 644 /root/AccessControl.conf

gcc -Wno-nonnull syscall_test.c -o syscall_test1
gcc -Wno-nonnull syscall_test.c -o syscall_test2
gcc -Wno-nonnull syscall_test.c -o syscall_test3
gcc -Wno-nonnull syscall_test.c -o syscall_test4
gcc -Wno-nonnull syscall_test.c -o syscall_test5
gcc -Wno-nonnull syscall_test.c -o syscall_test6
./syscall_test1 
./syscall_test2  
./syscall_test3 
./syscall_test4 
./syscall_test5 
./syscall_test6 
rm syscall_test1
rm syscall_test2
rm syscall_test3
rm syscall_test4
rm syscall_test5
rm syscall_test6
