#!/bin/bash
echo 本shell脚本用于测试Access Control及Access Audit的功能
sleep 2
echo 首先拷贝动态权限的配置文件至/root，表示该配置文件只能由系统的根用户创建并修改，内核模块读取的也是该文件
sleep 2
sudo cp ./AccessControl.conf /root/
sudo chown root /root/AccessControl.conf
sudo chmod 644 /root/AccessControl.conf
echo 然后我们将按README中的各个用户所对应的权限分别运行测试程序syscall_test，首先是静态配置了的5个用户
sleep 2
sudo sudo -u cpegg ./syscall_test # change this 'cpegg' to your default system user
sleep 1
echo 
echo
sudo sudo -u test_cpegg ./syscall_test
sleep 1
echo
echo
sudo sudo -u test_cpegg2 ./syscall_test
sleep 1
echo
echo
sudo sudo -u test_cpegg3 ./syscall_test
sleep 1
echo
echo
sudo sudo -u test_cpegg4 ./syscall_test
sleep 1
echo
echo
echo 然后是2个动态配置测试，相应的配置规则在configure文件中有所说明
sleep 2
# test conf
sudo sudo -u test_cpegg5 ./syscall_test
sleep 1
echo
echo
sudo sudo -u test_cpegg6 ./syscall_test
sleep 1

