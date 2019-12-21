/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#define FUSE_USE_VERSION 30
#include <fuse.h>
#include <string.h>

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {
  if (strcmp(path, "/") != 0)
    return -1;

  filler(buffer, ".", NULL, 0);
  filler(buffer, "..", NULL, 0);

  char *file_name;
  memcpy(file_name, path, 100);
  filler(buffer, file_name, NULL, 0);

  return 0;
}

int my_getattr(const char *path, struct stat *st) {
  st->st_uid = path + 108;
  st->st_gid = path + 116;
  st->st_mtime = path + 136;

  if (strcmp(path, "/") == 0) {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
  } else {
    st->st_mode = S_IFREG | 0644;
    st->st_nlink = 1;
    st->st_size = path + 124;
  }

  return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset,
            struct fuse_file_info *fi) {
  memcpy(buffer, path + 512, size);
  return size;
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
  memset(&op, 0, sizeof(op));
  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;
  return fuse_main(argc, argv, &op, NULL);
}
