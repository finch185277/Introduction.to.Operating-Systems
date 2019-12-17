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
#include <map>
#include <unordered_map>
#include <vector>

struct Node {
  int id;
  struct Node *prev;
  struct Node *next;
};

void LFU(std::vector<int> &history, int nframe) {
  int hit = 0, miss = 0;
  std::unordered_map<int, std::pair<int, struct Node *>> pages;
  std::map<int, std::pair<struct Node *, struct Node *>> freqs;
  for (auto page = history.begin(); page != history.end(); page++) {
    auto search = pages.find(*page);
    if (search != pages.end()) {
      hit++;

      // re-order current freq-list
      auto freq = freqs.find(search->second.first);
      if (freq->second.first == freq->second.second) {
        // only one node in freq-list
        freqs.erase(search->second.first);
      } else if (search->second.second == freq->second.first) {
        // node is head(MRU) of freq-list
        freq->second.first = freq->second.first->next;
        freq->second.first->prev = nullptr;
      } else if (search->second.second == freq->second.second) {
        // node is tail (LRU) of freq-list
        freq->second.second = freq->second.second->prev;
        freq->second.second->next = nullptr;
      } else { // node in mid of freq-list
        search->second.second->prev->next = search->second.second->next;
        search->second.second->next->prev = search->second.second->prev;
      }

      // check whether upper-freq exist,
      // if not, create new freq
      if (freqs.count(search->second.first + 1) == 0) {
        struct Node *mru = nullptr, *lru = nullptr;
        freqs.insert(std::pair<int, std::pair<struct Node *, struct Node *>>(
            search->second.first + 1,
            std::pair<struct Node *, struct Node *>(mru, lru)));
      }

      // add node to upper-freq
      freq = freqs.find(search->second.first + 1);
      search->second.second->prev = nullptr;
      search->second.second->next = nullptr;
      if (freq->second.first == nullptr && freq->second.second == nullptr) {
        // add new node
        freq->second.first = search->second.second;
        freq->second.second = search->second.second;
      } else {
        // add node to head (MRU)
        freq->second.first->prev = search->second.second;
        search->second.second->next = freq->second.first;
        freq->second.first = freq->second.first->prev;
      }

      // update page frequency
      search->second.first++;
    } else {
      miss++;

      // if frame is full
      if (pages.size() == nframe) {
        // find the victim (lru of botten-freq)
        struct Node *victim = freqs.begin()->second.second;

        if (freqs.begin()->second.first == freqs.begin()->second.second) {
          // only one node in freq
          freqs.erase(freqs.begin()->first);
        } else {
          // update the freq
          freqs.begin()->second.second = freqs.begin()->second.second->prev;
          freqs.begin()->second.second->next = nullptr;
        }

        // remove from pages
        pages.erase(victim->id);

        // remove node
        delete victim;
      }

      // check whether botten-freq exist,
      // if not, create new freq
      if (freqs.count(1) == 0) {
        struct Node *mru = nullptr, *lru = nullptr;
        freqs.insert(std::pair<int, std::pair<struct Node *, struct Node *>>(
            1, std::pair<struct Node *, struct Node *>(mru, lru)));
      }

      // add node to botten-freq
      auto freq = freqs.find(1);
      // create node
      struct Node *node = new struct Node;
      node->id = *page;
      node->prev = nullptr;
      node->next = nullptr;
      if (freq->second.first == nullptr && freq->second.second == nullptr) {
        // add new node
        freq->second.first = node;
        freq->second.second = node;
      } else {
        // add node to head (MRU)
        freq->second.first->prev = node;
        node->next = freq->second.first;
        freq->second.first = freq->second.first->prev;
      }

      // add node to pages
      pages.insert(std::pair<int, std::pair<int, struct Node *>>(
          *page, std::pair<int, struct Node *>(1, node)));
    }
  }
  std::cout << nframe << "\t" << hit << "\t\t" << miss << "\t\t" << std::fixed
            << std::setprecision(10) << (double)miss / (hit + miss) << "\n";
  return;
}

void LRU(std::vector<int> &history, int nframe) {
  int hit = 0, miss = 0;
  struct Node *mru = nullptr, *lru = nullptr;
  std::unordered_map<int, struct Node *> pages;
  for (auto page = history.begin(); page != history.end(); page++) {
    auto search = pages.find(*page);
    if (search != pages.end()) {
      hit++;

      // page is head (MRU): skip
      if (search->second == mru) {
        continue;
      }

      if (search->second == lru) {
        // page is tail (LRU)
        lru = lru->prev;
        lru->next = nullptr;
      } else {
        // page is mid of list
        search->second->prev->next = search->second->next;
        search->second->next->prev = search->second->prev;
      }

      // move page to head (MRU)
      mru->prev = search->second;
      search->second->next = mru;
      mru = mru->prev;
    } else {
      miss++;

      // if frame is full
      if (pages.size() == nframe) {
        struct Node *victim = lru;
        lru = lru->prev;
        lru->next = nullptr;
        pages.erase(victim->id);

        // remove node
        delete victim;
      }

      // add node to list
      // create node
      struct Node *node = new struct Node;
      node->id = *page;
      node->prev = nullptr;
      node->next = nullptr;
      if (mru == nullptr && lru == nullptr) {
        // add new node
        mru = node;
        lru = node;
      } else {
        // add node to head (MRU)
        mru->prev = node;
        node->next = mru;
        mru = mru->prev;
      }

      // add node to pages
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

    // LFU
    auto lfu_start = std::chrono::high_resolution_clock::now();
    std::cout << "LFU policy:\n";
    std::cout << "Frame\tHit\t\tMiss\t\tPage fault ratio\n";
    LFU(history, 64);
    LFU(history, 128);
    LFU(history, 256);
    LFU(history, 512);
    auto lfu_end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double, std::milli> lfu_time = lfu_end - lfu_start;
    std::cout << "Total elapsed time " << std::fixed << std::setprecision(4)
              << lfu_time.count() / 1000 << " sec\n";

    std::cout << '\n';

    // LRU
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
