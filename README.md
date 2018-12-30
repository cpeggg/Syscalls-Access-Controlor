# Syscalls Access Controlor
A Linux file audition-related syscall hook, to do audition and access control work. Altered from X86, tested on Ubuntu 16.04 x64. This is a kernel module that hook the syscall table to achieve the access control, based on the given example posted on FTP, it can also achieve the syscall audit function.
## SYSCALL HOOKS
1. Hook the [syscalls related to fs](http://linasm.sourceforge.net/docs/syscalls/filesystem.php). Simplify the most sensitive syscalls in file audition.
2. Hook execve (No.59)

In a nutshell, the following syscalls are hooked:
- READ 0 `ssize_t read(int fd, void *buf, size_t count);`
- WRITE 1 `ssize_t write(int fd, const void *buf, size_t count);`
- OPEN 2 `int open(const char *pathname, int flags, mode_t mode);` (mode is optional)
- EXECVE 59 `int execve(const char *filename, char *const argv[], char *const envp[]`
- CREAT 85 `int creat(const char *pathname, mode_t mode);`
## Compile and install
To compile the source code and install the kernel module
```bash
make
sudo insmod AuditModule.ko
```
## Test
Here we write a default configuration file and you can just run the shell script:
```bash
./test.sh
```
If you want then alter the config, run the user interface program:
```bash
./user_interface
```
If you want to run the audit deamon, open another terminal in the same path and run:
```bash
./auditdaemon
```
Run `dmesg` and you will find more info from the kernel.
## Uninstall
To remove the kernel module, just type
```bash
sudo rmmod AuditModule
```
## Author and contact
蔡洤朴 cpeggg@gmail.com

徐源
