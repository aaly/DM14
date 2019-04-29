#ifndef	ARRAY_HPP
#define	ARRAY_HPP

class vector;

#include <vector>
//#include <deque>
#include <iostream>
//#include <string>
#include <mutex.hpp>
#include <stdio.h>

using namespace std;


//template < class T, class Alloc = allocator<T> > class vector; // generic template

//typedef MutexLock Mutex;
//typedef Semaphore Mutex;
typedef spinLock Mutex;

//template < class T, class Alloc = allocator<T> > class vector;

template <class T> class Array : public std::vector<T>
{
	public:
	
		//string name;
		int lockCount;
		Array(unsigned long n = 0, const T& val = T())
		{		
			//resize(n,val);
			lockCount = 0;
			for (unsigned long i =0; i < n; i++)
			{
				push_back(val);
			}
		};

		void lock()
		{
			//cerr << ">>>>>>> LOCK" << endl << flush;
			lockCount++;
			if(lockCount > 1)
			{
				//fprintf(stderr, ">>>>>>> LOCK : %d\n", lockCount);
			}
			
			fflush(stderr);
			
			lockBit.lock();
			//volatile int* lockBit2 = &lockBit;
			//while (__sync_lock_test_and_set(lockBit2, 1))
			//{
				//while(*lockBit2);
				//// Do nothing. This GCC builtin instruction
				//// ensures memory barrier.
			//}
		};

		Array(std::vector<T> init) : std::vector<T>(init.begin(), init.end())
		{
			;
		};

		Array<T> cut(uint32_t from, uint32_t to)
		{
			Array<T> result(std::vector<T>(std::vector<T>(this->begin()+from, this->begin()+to)));
			this->erase(this->begin() + from, this->begin() + to);
			return result;
		};

		void copy(Array<T>& value)
		{
			for(uint32_t i =0; i < value.size(); i++)
			{
				push_back(value.at(i));
			}

		};

		void unlock()
		{
			lockCount--;
			if(lockCount > 1)
			{
				//fprintf(stderr, "<<<<<<< UNLOCK : %d\n", lockCount);
			}
			//cerr << "<<<<<<< UNLOCK" << endl << flush;
			
			fflush(stderr);
			lockBit.unlock();
			//volatile int* lockBit2 = &lockBit;
			//__sync_synchronize(); // Memory barrier.
			//*lockBit2 = 0;
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
		int	append_before(const T& value)
		{
			appendBeforeList.push_back(value);
			return 0;
		};
		
		int	remove_before(const T& value)
		{
			for ( unsigned int i =0; i < appendBeforeList.size(); i++)
			{
				if(appendBeforeList.at(i) == value)
				{
					appendBeforeList.erase(appendBeforeList.begin()+i);
				}
			}
			return 0;
		};
		
		int	append_after(const T& value)
		{
			appendAfterList.push_back(value);
			return 0;
		};
		
		int	remove_after(const T& value)
		{
			for ( unsigned int i =0; i < appendAfterList.size(); i++)
			{
				if(appendAfterList.at(i) == value)
				{
					appendAfterList.erase(appendAfterList.begin()+i);
				}
			}
			return 0;
		};
		
		int	push_back(const T& value)
		{
			
			for ( unsigned int i =0; i < appendBeforeList.size(); i++)
			{
				std::vector<T>::push_back(appendBeforeList.at(i));
			}
			appendBeforeList.clear();
			
			std::vector<T>::push_back(value);
			
			for ( unsigned int i =0; i < appendAfterList.size(); i++)
			{
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
		
		int flush()
		{
			for ( unsigned int i =0; i < appendBeforeList.size(); i++)
			{
				std::vector<T>::push_back(appendBeforeList.at(i));
			}

			for ( unsigned int i =0; i < appendAfterList.size(); i++)
			{
				std::vector<T>::push_back(appendAfterList.at(i));
			}
			
			appendBeforeList.clear();
			appendAfterList.clear();
			return 0;
		};
		
				
		int clearQueue()
		{
			appendBeforeList.clear();
			appendAfterList.clear();
			return 0;
		};
		void			remove(unsigned int i){this->erase(this->begin()+i);};
		//int			remove(int i);
	//private:
		std::vector<T> 	appendBeforeList;
		std::vector<T> 	appendAfterList;
		//bool		semaphore;
		//volatile int lockBit;
		Mutex lockBit;
};


#endif //ARRAY_HPP
