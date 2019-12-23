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
#include <unordered_map>
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

  // content
  std::vector<char> contents;

  size_t get_content_size() { return std::strtol(size, nullptr, 8); }

  size_t get_padding_size() {
    auto file_size = get_content_size();
    return (TAR_BLOCK_SIZE - (file_size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE;
  }
};

std::unordered_map<std::string, struct tar_file> entries;
std::unordered_map<std::string, std::vector<std::string>> subfiles;

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {
  printf("[readdir] {%s}\n", path);
  std::string file_name(path);

  auto itr = subfiles.find(file_name);
  if (itr != subfiles.end())
    for (auto sub = itr->second.begin(); sub != itr->second.end(); sub++) {
      printf("[readdir] get sub: {%s}\n", sub->c_str());
      filler(buffer, sub->c_str(), nullptr, 0);
    }
  else
    printf("no subfile!!!\n");

  return 0;
}

int my_getattr(const char *path, struct stat *st) {
  printf("[readdir] {%s}\n", path);
  std::string file_name(path);

  if (file_name == "/") {
    st->st_mode = S_IFDIR | 0777;
    st->st_nlink = 2;
    st->st_uid = getuid();
    st->st_gid = getgid();
    st->st_size = 0;
  } else {
    auto itr = entries.find(file_name.substr(1, file_name.size() - 1));
    if (itr == entries.end()) {
      // printf("[getattr] %s not found\n", path);
      return -ENOENT;
    } else {
      st->st_mode = std::strtol(itr->second.mode, nullptr, 8);
      st->st_nlink = 1;
      st->st_uid = std::strtol(itr->second.uid, nullptr, 8);
      st->st_gid = std::strtol(itr->second.gid, nullptr, 8);
      st->st_size = itr->second.get_content_size();
      st->st_mtime = std::strtol(itr->second.modify_time, nullptr, 8);
    }
  }

  return 0;
}

int my_read(const char *path, char *buffer, size_t size, off_t offset,
            struct fuse_file_info *fi) {
  std::string file_name(path);

  auto itr = entries.find(file_name.substr(1, file_name.size() - 1));
  if (itr == entries.end()) {
    return -ENOENT;
  } else {
    int read_size = itr->second.contents.size() - (size + offset);
    if (read_size > 0) {
      std::copy(&itr->second.contents.at(offset),
                &itr->second.contents.at(offset + size), buffer);
      return size;
    } else {
      std::copy(&itr->second.contents.at(offset),
                &itr->second.contents.at(offset + (-read_size)), buffer);
      return -read_size;
    }
  }
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
  std::ifstream infile;
  infile.open("test.tar");

  char null_block[TAR_BLOCK_SIZE];
  memset(null_block, 0, sizeof(null_block));

  struct tar_file root;
  sprintf(root.name, "/");
  entries.insert(std::pair<std::string, struct tar_file>(root.name, root));

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

    std::size_t found;
    std::string file_name(tfile.name);
    std::string parent, self;
    if (file_name.back() == '/') { // directory
      found = file_name.substr(0, file_name.size() - 1).rfind("/");
      if (found == std::string::npos) {
        parent = "/";
        self = file_name.substr(0, file_name.size() - 1);
      } else {
        parent = "/" + file_name.substr(0, found);
        self = file_name.substr(found + 1, file_name.size() - (found + 1));
      }
    } else { // regular file
      found = file_name.rfind("/");
      if (found == std::string::npos) {
        parent = "/";
        self = file_name;
      } else {
        parent = "/" + file_name.substr(0, found);
        self = file_name.substr(found + 1, file_name.size() - (found + 1));
      }
    }

    auto itr = subfiles.find(parent);
    if (itr == subfiles.end())
      subfiles.insert(
          std::pair<std::string, std::vector<std::string>>(parent, {self}));
    else
      itr->second.emplace_back(self);

    printf("[main] {%s}, size %d, found: %d, parent: {%s}, self: {%s}\n",
           tfile.name, (int)tfile.get_content_size(), (int)found,
           parent.c_str(), self.c_str());

    entries.insert(std::pair<std::string, struct tar_file>(tfile.name, tfile));
    infile.seekg(tfile.get_padding_size(), std::ios_base::cur);
  }

  memset(&op, 0, sizeof(op));
  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;
  return fuse_main(argc, argv, &op, NULL);
}
