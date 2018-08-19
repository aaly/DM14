//#include "Array.hpp"
#include <iostream>
#include <stdlib.h>
#include <vector>
#include "String.hpp"

using namespace std;



#ifdef WINDOWS
	#include <windows.h>
#else
	//#warning "don't forget -lpthread !!"
	#include <pthread.h>
	#include <signal.h>
#endif

template <class T> class Thread
{
	private:
		void operator=(const Thread &){}
		Thread(const Thread &){}

		void*	par;
		void*	(T::*memberFunc)(void*);
		void*	(*func)(void*);
		T*		caller;
		bool	isMemberFunc;

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
			if(memberFunc)
			{
				(caller->*func)(par);
			}
			else
			{
				(*func)(par);
			}
			return 0;
		}
		
		Thread()
		{
			#ifdef WINDOWS
			_handle = 0x00;
			#else
			#endif
			_isRunning = false;
			memberFunc = false;
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
			
			memberFunc = true;
			
			#ifdef WINDOWS
				_handle = CreateThread( 0x00, 0x00,Thread<T>::Starter, static_cast< void* >(this), 0x00, 0x00);
				return _handle != NULL;
			#else
				//return pthread_create(&_thread, NULL, Thread<T>::Starter, static_cast< void* >(this)) == 0;
				return pthread_create(&_thread, NULL, Starter, static_cast< void* >(this)) == 0;
			#endif
		}
		
		bool start(void* (*func)(void*), void* par)
		{
			if(_isRunning)
			{
				return false;
			}
			
			if (func == NULL)
			{
				return false;
			}
			
			memberFunc = false;
			
			this->par = par;
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



class threadMember
{
	private:
		void operator=(const threadMember &){}
		threadMember(const threadMember &){}

		void*	par;
		void	(*func)();

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
		
		bool start(void (*func)(), void* par)
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


//int x(string& str="test")
void x()
{
	int num = 5;
	cout  << num << endl;
	//return 0;
};

int main()
{
	
	int xx;
	threadMember thread;
	
	thread.start(x, &xx);
	thread.wait();
	
	return 0;
	bool l = -1;
	
	cout << l << endl;
	
	String str;
	str += "123";
	
	cout << str << endl;
	//x(str);

	return 0;
};


