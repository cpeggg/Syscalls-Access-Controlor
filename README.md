# Syscalls Access Controlor
A Linux file audition-related syscall hook. Altered from X86, getting tested on Ubuntu 16.04 x64.
## SYSCALL HOOKS
1. Hook the [syscalls related to fs](http://linasm.sourceforge.net/docs/syscalls/filesystem.php). Simplify the most sensitive syscalls in file audition and [Control Flow Integrity](https://www.cc.gatech.edu/~hhu86/papers/ucfi.pdf)
2. Hook execve (No.59)

In a nutshell, the following syscalls are hooked:

READ 0 `ssize_t read(int fd, void *buf, size_t count);`

WRITE 1 `ssize_t write(int fd, const void *buf, size_t count);`

OPEN 2 `int open(const char *pathname, int flags, mode_t mode);` (mode is optional)

EXECVE 59 `int execve(const char *filename, char *const argv[], char *const envp[]`

CREAT 85 `int creat(const char *pathname, mode_t mode);`

## ACCESS CONTROL

In our demo, we gonna add 2 users and 1 group, so their are 3 users and 2 groups in total:
- cpegg: user, uid=1000(cpegg), gid=1000(cpegg)
- test\_cpegg: user, uid=1001(test\_cpegg), gid=1000(cpegg)
- test\_cpegg2: user, uid=1002(test\_cpegg2) gid=1001(cpegg2)
- cpegg2: group, gid=1000(cpegg2)
