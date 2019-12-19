#define FUSE_USE_VERSION 30

#include <fuse.h>
#include <stdio.h>
#include <string.h>

int my_getattr(const char *path, struct stat *st) { /*do something*/ }
int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler, off_t offset, struct fuse_file_info *fi) { /*do something*/ }
int my_read(const char *path, char *buffer, size_t size, off_t offset, struct fuse_file_info *fi) { /*do something*/ }

static struct fuse_operations op;
int main(int argc, char *argv[])
{
    memset(&op, 0, sizeof(op)); 
    op.getattr = my_getattr;
    op.readdir = my_readdir;
    op.read = my_read;
    return fuse_main(argc, argv, &op, NULL);
}

