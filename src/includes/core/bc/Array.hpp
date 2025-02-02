#ifndef ARRAY_HPP
#define ARRAY_HPP

class vector;

#include <vector>
// #include <deque>
#include <iostream>

using namespace std;

// template < class T, class Alloc = allocator<T> > class vector; // generic
// template

// template < class T, class Alloc = allocator<T> > class vector;

template <class T> class Array : public std::vector<T> {
public:
  Array(unsigned long n = 0, const T &val = T()) {
    // resize(n,val);
    for (unsigned long i = 0; i < n; i++) {
      push_back(val);
    }
  };

  void lock() {
    volatile int *lockBit2 = &lockBit;
    while (__sync_lock_test_and_set(lockBit2, 1)) {
      while (*lockBit2)
        ;
      // Do nothing. This GCC builtin instruction
      // ensures memory barrier.
    }
  };

  void unlock() {
    volatile int *lockBit2 = &lockBit;
    __sync_synchronize(); // Memory barrier.
    *lockBit2 = 0;
  };

  /*T& at(unsigned long i)
  {
          if(i < this->size())
          {
                  return this[i];
          }
  }*/

  // push DEPS before the statement
  // push MODS after the statement
  int append_before(const T &value) {
    appendBeforeList.push_back(value);
    return 0;
  };

  int remove_before(const T &value) {
    for (unsigned int i = 0; i < appendBeforeList.size(); i++) {
      if (appendBeforeList.at(i) == value) {
        appendBeforeList.erase(appendBeforeList.begin() + i);
      }
    }
    return 0;
  };

  int append_after(const T &value) {
    appendAfterList.push_back(value);
    return 0;
  };

  int remove_after(const T &value) {
    for (unsigned int i = 0; i < appendAfterList.size(); i++) {
      if (appendAfterList.at(i) == value) {
        appendAfterList.erase(appendAfterList.begin() + i);
      }
    }
    return 0;
  };

  int push_back(const T &value) {

    for (unsigned int i = 0; i < appendBeforeList.size(); i++) {
      std::vector<T>::push_back(appendBeforeList.at(i));
    }
    appendBeforeList.clear();

    std::vector<T>::push_back(value);

    for (unsigned int i = 0; i < appendAfterList.size(); i++) {
      std::vector<T>::push_back(appendAfterList.at(i));
    }
    appendAfterList.clear();
    return 0;
  };
  /*
  T& at(size_t n)
  {
          if (n < 0 || n > vector<T>::size())
          {
                  throw "Index out of range";
          }
          return vector<T>::at(n);
  };*/

  int flush() {
    for (unsigned int i = 0; i < appendBeforeList.size(); i++) {
      std::vector<T>::push_back(appendBeforeList.at(i));
    }

    for (unsigned int i = 0; i < appendAfterList.size(); i++) {
      std::vector<T>::push_back(appendAfterList.at(i));
    }

    appendBeforeList.clear();
    appendAfterList.clear();
    return 0;
  };

  int clearQueue() {
    appendBeforeList.clear();
    appendAfterList.clear();
    return 0;
  };
  void remove(unsigned int i) { this->erase(this->begin() + i); };
  // int			remove(int i);
  // private:
  std::vector<T> appendBeforeList;
  std::vector<T> appendAfterList;
  // bool		semaphore;
  volatile int lockBit;
};

#endif // ARRAY_HPP
