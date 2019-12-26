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

#define TAR_BLOCK_SIZE 512
#include <fstream>
#include <string>
#include <unordered_map>
#include <vector>

// ref: https://www.onicos.com/staff/iz/formats/tar.html
#define LF_OLDNORMAL '\0' /* Normal disk file, Unix compatible */
#define LF_NORMAL '0'     /* Normal disk file */
#define LF_LINK '1'       /* Link to previously dumped file */
#define LF_SYMLINK '2'    /* Symbolic link */
#define LF_CHR '3'        /* Character special file */
#define LF_BLK '4'        /* Block special file */
#define LF_DIR '5'        /* Directory */
#define LF_FIFO '6'       /* FIFO special file */
#define LF_CONTIG '7'     /* Contiguous file */

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
  char *contents;

  // st attr
  int tar_mode;
  int tar_uid;
  int tar_gid;
  int tar_size;
  int tar_mtime;

  size_t get_content_size() { return std::stoi(size, nullptr, 8); }

  size_t get_padding_size() {
    auto file_size = get_content_size();
    return (TAR_BLOCK_SIZE - (file_size % TAR_BLOCK_SIZE)) % TAR_BLOCK_SIZE;
  }
};

std::unordered_map<std::string, struct tar_file> entries;
std::unordered_map<std::string, std::vector<std::string>> subfiles;

int my_readdir(const char *path, void *buffer, fuse_fill_dir_t filler,
               off_t offset, struct fuse_file_info *fi) {
  std::string file_name(path);

  auto itr = subfiles.find(file_name);
  if (itr != subfiles.end())
    for (auto sub = itr->second.begin(); sub != itr->second.end(); sub++)
      filler(buffer, sub->c_str(), nullptr, 0);

  return 0;
}

int my_getattr(const char *path, struct stat *st) {
  std::string file_name(path);

  if (file_name == "/") {
    st->st_mode = S_IFDIR | 0444;
    st->st_size = 0;
  } else {
    auto itr = entries.find(file_name.substr(1, file_name.size() - 1));
    if (itr == entries.end()) {
      return -ENOENT;
    } else {
      st->st_mode = itr->second.tar_mode;
      st->st_uid = itr->second.tar_uid;
      st->st_gid = itr->second.tar_gid;
      st->st_size = itr->second.tar_size;
      st->st_mtime = itr->second.tar_mtime;
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
    int read_size;
    if (itr->second.get_content_size() >= size + offset)
      read_size = size; // buffer size
    else
      read_size = itr->second.get_content_size() - offset; // remain of contents

    // copy contents into buffer
    memcpy(buffer, &itr->second.contents[offset], read_size);

    return read_size;
  }
}

static struct fuse_operations op;

int main(int argc, char *argv[]) {
  // open the file
  std::ifstream infile;
  infile.open("test.tar");

  // create blank header
  char null_block[TAR_BLOCK_SIZE];
  memset(null_block, 0, sizeof(null_block));

  for (;;) {
    struct tar_file tfile;

    // read header
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

    // check EOF
    if (memcmp(&tfile, null_block, TAR_BLOCK_SIZE) == 0)
      break;

    // read content
    tfile.contents = new char[tfile.get_content_size()];
    infile.read(tfile.contents, tfile.get_content_size());

    // translate char array to int (for st)
    switch (tfile.link_flag) {
    case LF_OLDNORMAL: // regular file
      tfile.tar_mode = S_IFREG | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_NORMAL: // regular file
      tfile.tar_mode = S_IFREG | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_DIR: // directory
      tfile.tar_mode = S_IFDIR | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_SYMLINK: // symbolic link
      tfile.tar_mode = S_IFLNK | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_CHR: // character special
      tfile.tar_mode = S_IFCHR | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_BLK: // block special
      tfile.tar_mode = S_IFBLK | std::stoi(tfile.mode, nullptr, 8);
      break;
    case LF_FIFO: // FIFO special
      tfile.tar_mode = S_IFIFO | std::stoi(tfile.mode, nullptr, 8);
      break;
    }
    tfile.tar_uid = std::stoi(tfile.uid, nullptr, 8);
    tfile.tar_gid = std::stoi(tfile.gid, nullptr, 8);
    tfile.tar_size = tfile.get_content_size();
    tfile.tar_mtime = std::stoi(tfile.modify_time, nullptr, 8);

    std::string file_name(tfile.name);

    // remove slash from back of name
    if (file_name.back() == '/')
      file_name.pop_back();

    // check the entry
    auto entry = entries.find(file_name);
    if (entry == entries.end()) { // file name not found
      // find parent of file
      std::size_t found;
      std::string parent, self;
      found = file_name.rfind("/");
      if (found == std::string::npos) {
        parent = "/";
        self = file_name;
      } else {
        parent = "/" + file_name.substr(0, found);
        self = file_name.substr(found + 1, file_name.size() - (found + 1));
      }

      // record parent of file
      auto itr = subfiles.find(parent);
      if (itr == subfiles.end())
        subfiles.insert(
            std::pair<std::string, std::vector<std::string>>(parent, {self}));
      else
        itr->second.emplace_back(self);

      // add new entry
      entries.insert(std::pair<std::string, struct tar_file>(file_name, tfile));

    } else { // file name exist
      if (tfile.tar_mtime > entry->second.tar_mtime)
        // replace that entry
        entry->second = tfile;
    }

    // move read head to next tar file
    infile.seekg(tfile.get_padding_size(), std::ios_base::cur);
  }

  memset(&op, 0, sizeof(op));
  op.getattr = my_getattr;
  op.readdir = my_readdir;
  op.read = my_read;
  return fuse_main(argc, argv, &op, NULL);
}
