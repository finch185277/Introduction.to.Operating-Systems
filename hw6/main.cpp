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
#include <unistd.h>

#define TAR_BLOCK_SIZE 512
#include <algorithm>
#include <string>
#include <vector>

struct tar_file {
  // header
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char modify_time[12];
  char checksum[8];
  char link_flag;
  char link_name[100];
  char magic[8];
  char user_name[32];
  char group_name[32];
  char major_dev[8];
  char minor_dev[8];
  char padding[167];

  std::vector<char> contents;

  size_t get_content_size() { return std::stoi(size, 0, 8); }

  size_t get_padding_size() {
    auto file_size = get_content_size();
    return TAR_BLOCK_SIZE - (file_size % TAR_BLOCK_SIZE);
  }
};

struct fuse_entry {
  std::string name;
  struct tar_file tfile;
  struct stat *st;
  off_t offset;
  fuse_entry() {}
  fuse_entry(std::string name, struct stat *st, off_t offset)
      : name(name), st(st), offset(offset){};
  fuse_entry(std::string name, struct tar_file tfile, struct stat *st,
             off_t offset)
      : name(name), tfile(tfile), st(st), offset(offset){};
};

struct find_entry : std::unary_function<struct fuse_entry, bool> {
  std::string name;
  find_entry(std::string name) : name(name) {}
  bool operator()(const struct &fuse_entry entry) const {
    return entry.name == name;
  }
};

std::vector<fuse_entry> entries;

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {

  if (offset < entries.size())
    filler(buffer, entries[offset].name.c_str(), entries[offset].st, 0);

  return 0;
}

int my_getattr(const char *path, struct stat *st) {
  std::string file_name(path);
  auto itr =
      std::find_if(entries.begin(), entries.end(), find_entry(file_name));

  st->st_uid = itr->tfile.uid;
  st->st_gid = itr->tfile.gid;

  if (file_name == "/") {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 2;
    st->st_size = 0;
  } else {
    st->st_mode = S_IFREG | 0644;
    st->st_nlink = 1;
    st->st_size = itr->tfile.get_content_size();
  }

  return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset,
            struct fuse_file_info *fi) {
  std::string file_name(path);
  auto itr =
      std::find_if(entries.begin(), entries.end(), find_entry(file_name));
  if (itr = entries.end()) {
    return -1;
  } else {
    int read_size = itr->tfile.contents.size() - (size + offset);
    if (read_size > 0) {
      memcpy(buffer, &itr->tfile.contents[offset], size);
      return size;
    } else {
      memcpy(buffer, &itr->tfile.contents[offset], -read_size);
      return -read_size;
    }
  }
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
  int offset = 1;

  entries.emplace_back(std::string(".."), nullptr, offset++);
  entries.emplace_back(std::string("."), nullptr, offset++);

  int fd = open("test.tar", O_RDONLY);
  char null_block[TAR_BLOCK_SIZE];
  memset(null_block, 0, sizeof(null_block));

  for (;;) {
    struct tar_file tfile;
    read(fd, &tfile, TAR_BLOCK_SIZE);

    if (memcmp(&tfile, null_block, TAR_BLOCK_SIZE) == 0)
      break;

    tfile.contents.resize(tfile.get_content_size());
    read(fd, tfile.contents.begin(), tfile.get_content_size());

    entries.emplace_back(tfile.name, tfile, nullptr, offset++);

    lseek(fd, tfile.get_padding_size(), SEEK_CUR);
  }

  close(fd);

  memset(&op, 0, sizeof(op));
  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;
  return fuse_main(argc, argv, &op, NULL);
}
