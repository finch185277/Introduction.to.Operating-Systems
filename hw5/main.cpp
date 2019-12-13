/*
Student No.: 0616214
Student Name: YU-AN CHEN
Email: 59487andy@gmail.com
SE tag: xnxcxtxuxoxsx
Statement: I am fully aware that this program is not
supposed to be posted to a public server, such as a
public GitHub repository or a public web page.
*/

#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unordered_map>
#include <vector>

struct Node {
  int id;
  struct Node *prev;
  struct Node *next;
};

void LRU(std::vector<int> &history, int nframe) {
  int hit = 0, miss = 0;
  struct Node *lru = nullptr, *mru = nullptr;
  std::unordered_map<int, struct Node *> pages;
  for (auto page = history.begin(); page != history.end(); page++) {
    auto search = pages.find(*page);
    if (search != pages.end()) {
      hit++;
      struct Node *node = search->second;
      if (node == mru) {
        continue;
      }
      if (node == lru) {
        lru = lru->prev;
        lru->next = nullptr;
      } else {
        node->prev->next = node->next;
        node->next->prev = node->prev;
      }
      mru->prev = node;
      node->next = mru;
      mru = mru->prev;
    } else {
      miss++;
      if (pages.size() == nframe) {
        struct Node *victim = lru;
        lru = lru->prev;
        lru->next = nullptr;
        pages.erase(victim->id);
        delete victim;
      }
      struct Node *node = new struct Node;
      node->id = *page;
      node->prev = nullptr;
      node->next = nullptr;
      if (mru == nullptr && lru == nullptr) {
        mru = node;
        lru = node;
      } else {
        mru->prev = node;
        node->next = mru;
        mru = mru->prev;
      }
      pages.insert(std::pair<int, struct Node *>(*page, node));
    }
  }
  std::cout << nframe << "\t" << hit << "\t\t" << miss << "\t\t" << std::fixed
            << std::setprecision(10) << (double)miss / (hit + miss) << "\n";
  return;
}

int main(int argc, char **argv) {
  // open the file
  std::ifstream infile(argv[1]);
  if (infile.good()) { // if file exist
    std::vector<int> history;
    int page;
    while (infile >> page)
      history.push_back(page);

    auto lru_start = std::chrono::high_resolution_clock::now();
    std::cout << "LRU policy:\n";
    std::cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\n";
    LRU(history, 64);
    LRU(history, 128);
    LRU(history, 256);
    LRU(history, 512);
    auto lru_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> lru_time = lru_end - lru_start;
    std::cout << "Total elapsed time " << std::fixed << std::setprecision(4)
              << lru_time.count() / 1000 << " sec\n";

  } else { // if file not exist
    std::cout << "File: " << argv[1] << " does not exist!" << '\n';
  }
  return 0;
}
