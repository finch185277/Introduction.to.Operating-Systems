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
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define TAR_BLOCK_SIZE 512
#include <algorithm>
#include <fstream>
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

  size_t get_content_size() { return std::strtol(size, nullptr, 8); }

  size_t get_padding_size() {
    auto file_size = get_content_size();
    return (TAR_BLOCK_SIZE - (file_size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE;
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
  bool operator()(const struct fuse_entry &entry) const {
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

  st->st_uid = getuid();
  st->st_gid = getgid();

  if (itr == entries.end()) {
    printf("[getattr] no such file: %s\n", path);
    return -1;
  } else {
    printf("[getattr] file exist: %s\n", path);
    if (file_name == "/") {
      st->st_mode = S_IFDIR | 0777;
      st->st_nlink = 2;
      st->st_size = 0;
    } else {
      st->st_mode = S_IFREG | 0777;
      st->st_nlink = 1;
      st->st_size = itr->tfile.get_content_size();
    }
  }

  return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset,
            struct fuse_file_info *fi) {
  std::string file_name(path);
  auto itr =
      std::find_if(entries.begin(), entries.end(), find_entry(file_name));
  if (itr == entries.end()) {
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

  std::ifstream infile;
  infile.open("test.tar");

  char null_block[TAR_BLOCK_SIZE];
  memset(null_block, 0, sizeof(null_block));

  for (;;) {
    struct tar_file tfile;
    infile.read((char *)&tfile.name, sizeof(tfile.name));
    infile.read((char *)&tfile.mode, sizeof(tfile.mode));
    infile.read((char *)&tfile.uid, sizeof(tfile.uid));
    infile.read((char *)&tfile.gid, sizeof(tfile.gid));
    infile.read((char *)&tfile.size, sizeof(tfile.size));
    infile.read((char *)&tfile.modify_time, sizeof(tfile.modify_time));
    infile.read((char *)&tfile.checksum, sizeof(tfile.checksum));
    infile.read((char *)&tfile.link_flag, sizeof(tfile.link_flag));
    infile.read((char *)&tfile.link_name, sizeof(tfile.link_name));
    infile.read((char *)&tfile.magic, sizeof(tfile.magic));
    infile.read((char *)&tfile.user_name, sizeof(tfile.user_name));
    infile.read((char *)&tfile.group_name, sizeof(tfile.group_name));
    infile.read((char *)&tfile.major_dev, sizeof(tfile.major_dev));
    infile.read((char *)&tfile.minor_dev, sizeof(tfile.minor_dev));
    infile.read((char *)&tfile.padding, sizeof(tfile.padding));

    if (memcmp(&tfile, null_block, TAR_BLOCK_SIZE) == 0)
      break;

    int content_size = tfile.get_content_size();
    tfile.contents.resize(content_size);
    while (content_size--) {
      char c;
      infile.get(c);
      tfile.contents.emplace_back(c);
    }

    entries.emplace_back(tfile.name, tfile, nullptr, offset++);
    printf("[main] Get file: %-20s, size: %-5d\n", tfile.name,
           (int)tfile.get_content_size());

    infile.seekg(tfile.get_padding_size(), std::ios_base::cur);
  }

  memset(&op, 0, sizeof(op));
  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;
  return fuse_main(argc, argv, &op, NULL);
}
