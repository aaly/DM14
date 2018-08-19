#ifndef THREAD_HPP
#define THREAD_HPP

//#include "Sleep.hpp"

#ifdef WINDOWS
	#include <windows.h>
#else
	//#warning "don't forget -lpthread !!"
	#include <pthread.h>
	#include <signal.h>
#endif


class threadMember
{
	private:
		void operator=(const threadMember &){}
		threadMember(const threadMember &){}

		void*	par;
		void*	(*func)();

		#ifdef WINDOWS
			HANDLE _handle;
		#else
			pthread_t _thread;
		#endif

		bool _isRunning;

		#ifdef WINDOWS
		static DWORD WINAPI Starter(LPVOID in_thread)
		{
		#else
		static void* Starter(void* in_thread)
		{
		#endif
			threadMember* thread = static_cast< threadMember* >(in_thread);
			
			//(thread->((thread->Caller)->((thread->)*func)))(thread->par);
			//thread->(*(thread->caller)->*func)(thread->par);

			thread->_isRunning = true;
			thread->run();
			thread->_isRunning = false;
			
			return 0x00;
		}

	public:
		int run()
		{
			(*func)();
			return 0;
		}

		threadMember()
		{
			#ifdef WINDOWS
			_handle = 0x00;
			#else
			#endif
			_isRunning = false;
			//runfun = NULL;
		}

		virtual ~threadMember()
		{
			if(!_isRunning)
			{
				return;
			}
			wait();
			#ifdef WINDOWS
				CloseHandle (_handle);
			#else
			#endif
		}
		
		bool start(void* (*func)(), void* par)
		{
			if(_isRunning)
			{
				return false;
			}
			
			if (func == NULL)
			{
				return false;
			}
			
			
			this->par = par;
			this->func = func;
			
			#ifdef WINDOWS
				_handle = CreateThread( 0x00, 0x00,threadMember<T>::Starter, static_cast< void* >(this), 0x00, 0x00);
				return _handle != NULL;
			#else
				//return pthread_create(&_thread, NULL, Thread<T>::Starter, static_cast< void* >(this)) == 0;
				return pthread_create(&_thread, NULL, Starter, static_cast< void* >(this)) == 0;
			#endif
		}

		bool isRunning() const
		{
			return _isRunning;
		}

		bool wait() const
		{
			if(!_isRunning) return false;
			
			#ifdef WINDOWS
				return WaitForSingleObject(_handle,INFINITE) == 0x00000000L;
			#else
				
				return pthread_join(_thread, NULL) == 0;
			#endif
		}
		
		bool kill()
		{
			if(!_isRunning)
			{
				return false;
			}
			_isRunning = false;
			#ifdef WINDOWS
				bool success = TerminateThread(_handle,1) && CloseHandle(_handle);
				_handle = 0x00;
				return success;
			#else
				return pthread_kill( _thread, SIGKILL) == 0;
			#endif
		}
};

template <class T> class Thread
{
	private:
		void operator=(const Thread &){}
		Thread(const Thread &){}

		void*	par;
		void*	(T::*func)(void*);
		T*		caller;

		#ifdef WINDOWS
			HANDLE _handle;
		#else
			pthread_t _thread;
		#endif

		bool _isRunning;

		#ifdef WINDOWS
		static DWORD WINAPI Starter(LPVOID in_thread)
		{
		#else
		static void* Starter(void* in_thread)
		{
		#endif
			Thread<T> * thread = static_cast< Thread<T> * >(in_thread);
			
			//(thread->((thread->Caller)->((thread->)*func)))(thread->par);
			//thread->(*(thread->caller)->*func)(thread->par);

			thread->_isRunning = true;
			thread->run();
			thread->_isRunning = false;
			
			return 0x00;
		}

	public:
		int run()
		{
			(caller->*func)(par);
			return 0;
		}
		
		Thread()
		{
			#ifdef WINDOWS
			_handle = 0x00;
			#else
			#endif
			_isRunning = false;
			//runfun = NULL;
		}

		virtual ~Thread()
		{
			if(!_isRunning)
			{
				return;
			}
			wait();
			#ifdef WINDOWS
				CloseHandle (_handle);
			#else
			#endif
		}
		
		bool start(T* caller, void* (T::*func)(void*), void* par)
		{
			if(_isRunning)
			{
				return false;
			}
			
			if (caller == NULL)
			{
				return false;
			}
			
			if (func == NULL)
			{
				return false;
			}
			
			this->par = par;
			this->caller = caller;
			this->func = func;
			
			#ifdef WINDOWS
				_handle = CreateThread( 0x00, 0x00,Thread<T>::Starter, static_cast< void* >(this), 0x00, 0x00);
				return _handle != NULL;
			#else
				//return pthread_create(&_thread, NULL, Thread<T>::Starter, static_cast< void* >(this)) == 0;
				return pthread_create(&_thread, NULL, Starter, static_cast< void* >(this)) == 0;
			#endif
		}

		bool isRunning() const
		{
			return _isRunning;
		}

		bool wait() const
		{
			if(!_isRunning) return false;
			
			#ifdef WINDOWS
				return WaitForSingleObject(_handle,INFINITE) == 0x00000000L;
			#else
				
				return pthread_join(_thread, NULL) == 0;
			#endif
		}
		
		bool kill()
		{
			if(!_isRunning)
			{
				return false;
			}
			_isRunning = false;
			#ifdef WINDOWS
				bool success = TerminateThread(_handle,1) && CloseHandle(_handle);
				_handle = 0x00;
				return success;
			#else
				return pthread_kill( _thread, SIGKILL) == 0;
			#endif
		}
};


class MutexLock
{
	public:
    /*MutexLock();
    ~MutexLock();

    void Lock();
    void unLock();
    bool tryLock();
    */
    MutexLock()
{
	pthread_mutexattr_settype(&attr,  PTHREAD_MUTEX_NORMAL);
	pthread_mutex_init(&mutex, &attr);
}

~MutexLock()
{
	pthread_mutex_destroy(&mutex);
	pthread_mutexattr_destroy (&attr);
}

void lock()
{
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

void unlock()
{
	pthread_mutex_unlock(&mutex);
	/*lock--;
	if(lock != 0)
	{
		cout << "Multiple thread unlocks : ERROR" << endl << flush;
		//exit(1);
	}*/

}

bool tryLock()
{
		return pthread_mutex_trylock(&mutex);
}
    private:
        pthread_mutex_t  	  mutex;
        pthread_mutexattr_t    attr;
        //int lock;
};

/*
MutexLock::MutexLock()
{
	pthread_mutex_init(&mutex, NULL);
}

MutexLock::~MutexLock()
{
	pthread_mutex_init(&mutex, NULL);
}

void MutexLock::Lock()
{
	pthread_mutex_lock(&mutex);
}

void MutexLock::unLock()
{
		pthread_mutex_unlock(&mutex);
}

bool MutexLock::tryLock()
{
		return pthread_mutex_trylock(&mutex);
}
*/


#endif //THREAD_HPP
