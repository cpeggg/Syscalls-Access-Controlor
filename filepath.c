/*************************************************************************
	> File Name: filepath.c
	> Author: 
	> Mail: 
	> Created Time: Wed 12 Dec 2018 10:46:04 AM CST
 ************************************************************************/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/string.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/unistd.h>
#include <linux/slab.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("cpegg");
MODULE_DESCRIPTION("A simple example Linux module leart from Robert W. Oliver II");
MODULE_VERSION("0.01")

static int __init audit_init(void){
    :
    char *tmp;
    char *pathname;
    struct file *file;
    struct path *path;

    spin_lock(&files->file_lock);
    file = fcheck_files(files, fd);
    if (!file) {
            spin_unlock(&files->file_lock);
            return -ENOENT;

    }

    path = &file->f_path;
    path_get(path);
    spin_unlock(&files->file_lock);

    tmp = (char *)__get_free_page(GFP_KERNEL);

    if (!tmp) {
            path_put(path);
            return -ENOMEM;

    }

    pathname = d_path(path, tmp, PAGE_SIZE);
    path_put(path);

    if (IS_ERR(pathname)) {
            free_page((unsigned long)tmp);
            return PTR_ERR(pathname);

    }

    /* do something here with pathname */

    free_page((unsigned long)tmp);
}
