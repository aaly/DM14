#ifndef MUTEXLOCK_HPP
#define MUTEXLOCK_HPP

#include <linux/futex.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/time.h>

#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

class pipeLock {
public:
  pipeLock() {
    socketpair(AF_UNIX, SOCK_STREAM, 0, pollTrigger);
    EPollFD = epoll_create1(EPOLL_CLOEXEC);
    eventsnum = 2;
    events =
        (struct epoll_event *)calloc(eventsnum, sizeof(struct epoll_event));
    static struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = pollTrigger[1];
    epoll_ctl(EPollFD, EPOLL_CTL_ADD, pollTrigger[1], &ev);
  }

  void lock() { epoll_wait(EPollFD, events, eventsnum, -1); }

  void unlock() { write(pollTrigger[0], "w", 1); }

private:
  int pollTrigger[2];
  int EPollFD;
  int eventsnum;
  struct epoll_event *events;
};

class Semaphore {
private:
  sem_t sem;

public:
  /* -- CONSTRUCTOR/DESTRUCTOR */

  Semaphore() { sem_init(&sem, 0, 0); };

  //~Semaphore();

  /* -- SEMAPHORE OPERATIONS */

  void lock() // changed to void: you don't return anything
  {
    sem_wait(&sem);
  };

  void unlock() { sem_post(&sem); };
};

class spinLock {
public:
  spinLock() { lockBit = 0; }
  void lock() {
    while (__sync_lock_test_and_set(&lockBit, 1)) {
      while (lockBit)
        ;
      // Do nothing. This GCC builtin instruction
      // ensures memory barrier.
    }
  }

  void unlock() {
    __sync_synchronize(); // Memory barrier.
    lockBit = 0;
  }

private:
  volatile int lockBit;
};

class MutexLock {
public:
  /*MutexLock();
  ~MutexLock();

  void Lock();
  void unLock();
  bool tryLock();
  */
  MutexLock() {
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_NORMAL);
    // int OldPrio = 0;
    // pthread_mutex_setprioceiling(&mutex, 0, &OldPrio);
    // pthread_mutexattr_settype(&attr,  PTHREAD_MUTEX_ERRORCHECK_NP);
    pthread_mutex_init(&mutex, &attr);
  }

  ~MutexLock() {
    pthread_mutex_destroy(&mutex);
    pthread_mutexattr_destroy(&attr);
  }

  void lock() {
    pthread_mutex_lock(&mutex);
    /*
    lock++;
    if(lock != 1)
    {
            cout << "Multiple thread locks : ERROR" << endl << flush;
            //exit(1);
    }
    //while(!tryLock())
    {
            //cout << "trying to lock" << endl;
    }*/
  }

  void unlock() {
    pthread_mutex_unlock(&mutex);
    /*lock--;
    if(lock != 0)
    {
            cout << "Multiple thread unlocks : ERROR" << endl << flush;
            //exit(1);
    }*/
  }

  bool tryLock() { return pthread_mutex_trylock(&mutex); }

private:
  pthread_mutex_t mutex;
  pthread_mutexattr_t attr;
  // int lock;
};

/*


#define cpu_relax() __builtin_ia32_pause()
#define cmpxchg(P, O, N) __sync_val_compare_and_swap((P), (O), (N))

class Futex
{

void unlock()
{
    unsigned current, wanted;
    do {
        current = _lock;
        if (current == _lock_open) return;
        if (current == _lock_wlocked) {
            wanted = _lock_open;
        } else {
            wanted = current - 1;
        }
    } while (cmpxchg(&_lock, current, wanted) != current);
    syscall(SYS_futex, &_lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL, 0);
}

void _rlock()
{
    unsigned current;
    while ((current = _lock) == _lock_wlocked || cmpxchg(&_lock, current,
current + 1) != current) { while (syscall(SYS_futex, &_lock, FUTEX_WAIT_PRIVATE,
current, NULL, NULL, 0) != 0) { cpu_relax(); if (_lock >= _lock_open) break;
        }
        // will be able to acquire rlock no matter what unlock woke us
    }
}

void lock()
        {
                unsigned current;
                while ((current = cmpxchg(&_lock, _lock_open, _lock_wlocked)) !=
_lock_open) { while (syscall(SYS_futex, &_lock, FUTEX_WAIT_PRIVATE, current,
NULL, NULL, 0) != 0) { cpu_relax(); if (_lock == _lock_open) break;
                        }
                        if (_lock != _lock_open) {
                                // in rlock - won't be able to acquire lock -
wake someone else syscall(SYS_futex, &_lock, FUTEX_WAKE_PRIVATE, 1, NULL, NULL,
0);
                        }
                }
        }

        private:
                unsigned _lock = 1; // read-write lock futex
                const unsigned _lock_open = 1;
                const unsigned _lock_wlocked = 0;
};



class Futex
{

public:
        Futex () : val (0) { }
        void lock ()
        {
                int c;
                if ((c = cmpxchg (val, 0, 1)) != 0)
                {
                        if (c != 2)
                        {
                                c = xchg (val, 2);
                        }
                        while (c != 0)
                        {
                                futex_wait (&val, 2);
                                c = xchg (val, 2);
                        }
                }
        }

        void unlock ()
        {
                if (atomic_dec (val) != 1)
                {
                        val = 0;
                }
                futex_wake (&val, 1);
        }


private:
        int val;
};




class FastMutex
{
public:
    FastMutex() : _word(0) {}
    void lock()
    {
        // try to atimically swap 0 -> 1
        if (CAS(&_word, 0, 1))
            return; // success
        // wasn't zero -- somebody held the lock
        do
        {
            // assume lock is still taken, try to make it 2 and wait
            if (_word == 2 || CAS(&_word, 1, 2))
            {
                // let's wait, but only if the value is still 2
                futex_wait(&_word, 2);
            }
            // try (again) assuming the lock is free
        } while (!CAS(&_word, 0, 2);
        // we are here only if transition 0 -> 2 succeeded
    }
    void unlock()
    {
        // we own the lock, so it's either 1 or 2
        if (atomic_decrement(&_word) != 1)
        {
            // it was 2
            // important: we are still holding the lock!
            _word = 0; // not any more
            // wake up one thread (no fairness assumed)
            futex_wake(&_word, 1);
        }
    }
private:
    int _word;
};
*/

#endif // MUTEXLOCK_HPP
