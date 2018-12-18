#!/bin/bash
sudo cp ./AccessControl.conf /root/
sudo chown root /root/AccessControl.conf
sudo chmod 644 /root/AccessControl.conf
sudo sudo -u cpegg ./syscall_test
sudo sudo -u test_cpegg ./syscall_test
sudo sudo -u test_cpegg2 ./syscall_test
sudo sudo -u test_cpegg3 ./syscall_test
sudo sudo -u test_cpegg4 ./syscall_test
# test conf
sudo sudo -u test_cpegg5 ./syscall_test
sudo sudo -u test_cpegg6 ./syscall_test

