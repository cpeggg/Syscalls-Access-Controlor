# Syscalls Access Controlor
A Linux file audition-related syscall hook, to do audition and access control work. Altered from X86, tested on Ubuntu 16.04 x64.
## SYSCALL HOOKS
1. Hook the [syscalls related to fs](http://linasm.sourceforge.net/docs/syscalls/filesystem.php). Simplify the most sensitive syscalls in file audition.
2. Hook execve (No.59)

In a nutshell, the following syscalls are hooked:
- READ 0 `ssize_t read(int fd, void *buf, size_t count);`
- WRITE 1 `ssize_t write(int fd, const void *buf, size_t count);`
- OPEN 2 `int open(const char *pathname, int flags, mode_t mode);` (mode is optional)
- EXECVE 59 `int execve(const char *filename, char *const argv[], char *const envp[]`
- CREAT 85 `int creat(const char *pathname, mode_t mode);`

## ACCESS CONTROL
In our demo, we gonna add 4 users and 1 group, so their are 3 users and 2 groups in test:
- cpegg: user, uid=1000(cpegg), gid=1000(cpegg)
- test\_cpegg: user, uid=1001(test\_cpegg), gid=1000(cpegg)
- test\_cpegg2: user, uid=1002(test\_cpegg2) gid=1001(cpegg2)
- test\_cpegg3: user, uid=1003(test\_cpegg3), gid=1000(cpegg)
- test\_cpegg4: user, uid=1004(test\_cpegg4), gid=1001(cpegg2)
- test\_cpegg5: user, uid=1005(test\_cpegg5), gid=1000(cpegg)
- cpegg: group, gid=1000(cpegg)
- cpegg2: group, gid=1001(cpegg2)
- cpegg3: group, gid=1002(cpegg3)

In this demo, the users will try to open/read/write `/tmp/ioTest`, try to creat `/tmp/creatTest` and try to execve `/tmp/execTest`.

This 3 file are set by `chmod 777 <file>` to make them accessable to all users in advance. Then, the system is deployed and we will check whether the users/groups can have corresponding access to these files

To show the access control deployed on the users and groups, we will apply the following rules:
1. cpegg can open/read/write/creat/execve
2. test\_cpegg cannot open/read/write/creat/execve, to show the access control deployed according to the uid
3. test\_cpegg2 cannot open/read/write/creat/execve, to show the Access control deployed according to the gid
4. test\_cpegg3 cannot read/write given content from/to the file, but can have other controls to these files
5. test\_cpegg4 can open/read/write/creat/execve, to show when the access control has been deployed to both uid and gid, the control deployed on uid will be on primary compared to control deployed on gid.

### Dynamic Access Control
We deployed the AccessControl.conf, a configure file logging the access list deployed to all users. After following the naming rules mentioned in the configure file, we can add dynamic rules to all users.
In our demo, we show that we add ban to execve on uid=1005 and gid=1002, which is not included in our static rules hard-coded in the program. Which is a proof of work.

## How to run the demo?
All command are write to Makefile, to run the demo, type the following command:
```bash
sudo make rmuser
sudo make clean
sudo make adduser
make
sudo insmod AuditModule.ko
./test.sh
sudo rmmod AuditModule.ko
sudo make rmuser
sudo make clean
```
Also, if you want to check the audit function, after insmod the module you can open another terminal and run `./auditdaemon`, then you can see the audit results.

## TODO
READ(0) Control can only ret -1 but the content has been read to the user buffer.

## Author
cpegg@2018
