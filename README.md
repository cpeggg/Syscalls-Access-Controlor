# Fileauditor
A Linux file audition-related syscall hook. Altered from X86, getting tested on Ubuntu 16.04 x64.
## TODO
1. Hook the [syscalls related to fs](http://linasm.sourceforge.net/docs/syscalls/filesystem.php). Simplify the most sensitive syscalls in file audition which is included in "File operations", "Directory operations", "Link operations","Basic file attributes" and "Read/Write"
2. Hook execve (No.59)

In a nutshell, the following syscalls are hooked:

READ 0 `ssize_t read(int fd, void *buf, size_t count);`
WRITE 1 `ssize_t write(int fd, const void *buf, size_t count);`
OPEN 2 `int open(const char *pathname, int flags, mode_t mode);` (mode is optional)
CLOSE 3 `int close(int fd);`
LSEEK 8 `off_t lseek(int fd, off_t offset, int whence);`
EXECVE 59 `int execve(const char *filename, char *const argv[], char *const envp[]`
CREAT 85 `int creat(const char *pathname, mode_t mode);`
OPENAT 257 `int openat(int dirfd, const char *pathname, int flags, mode_t mode);` (mode is optional)
