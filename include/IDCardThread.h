#ifndef IDCardThread_h
#define IDCardThread_h

#include <string>
#include <iostream>
using namespace std;
#include "IDCardInclude.h"

#define THREAD_TERMINATE_TIMEOUT 2000

namespace IDCARD
{

	static void thread_proc(void *param);

	class CIDCardThread
	{
	public:
		CIDCardThread():m_terminated_(false), m_thr_name_(""), m_thr_index_(-1) {
#ifdef WIN32
			m_time_delay_ = THREAD_TERMINATE_TIMEOUT;
			m_thr_handle_ = NULL;
#else
			m_thr_handle_ = 0;
#endif
		}
		virtual ~CIDCardThread() {}

	public:
		INT32			start() {
			m_terminated_ = false;

#ifdef WIN32
			m_thr_handle_ = (HANDLE)_beginthread(thread_proc, 0,this);
#else
			pthread_create(&thr_handle_, NULL, thread_proc, this);
#endif

			return 0;
		}
		virtual INT32	terminate() {
			if(m_terminated_)
			{
				return 0;
			}

			m_terminated_ = true;

#ifdef WIN32
			if(m_thr_handle_ != NULL)
			{
				WaitForSingleObject(m_thr_handle_, m_time_delay_);
			}
#else
			if(m_thr_handle_ != 0)
			{
				pthread_join(m_thr_handle_, NULL);
			}
#endif

			return 0;
		}
		virtual void	execute() {

		}

		bool	get_terminated() const {
#ifdef WIN32
			_endthread();
#else
			pthread_exit(NULL);
#endif
		}
		void	set_terminated(bool terminated) {
			m_terminated_ = terminated;
		}

		bool	set_priority(INT32 pri) {
			return true;
		}
		INT32	get_priority() const {
			return 0;
		}

		void	set_thread_index(INT32 thr_index) {
			m_thr_index_ = thr_index;
		}
		INT32	get_thread_index() const {
			return m_thr_index_;
		}

		void	set_thread_name(const string& thr_name) {
			m_thr_name_= thr_name;
		}
		const string&	get_thread_name() const {
			return m_thr_name_;
		}

		friend std::ostream& operator<<(std::ostream& os, const CIDCardThread& thr) {
			//	os << "thread info, handler = " << (uint32_t)thr.thr_handle_ \
			//		<< ", terminated = " << (thr.terminated_ ? "true" : "false") \
			//		<< ", thr_index = " << thr.thr_index_ \
			//		<< ", thr_name = " << thr.thr_name_ << "\n";

			return os;
		}

	protected:
		void			clear_thread() {
#ifdef WIN32
			_endthread();
#else
			pthread_exit(NULL);
#endif
		}

	protected:
#ifdef WIN32
		HANDLE			m_thr_handle_;
		UINT32			m_time_delay_;
#else
		pthread_t       m_thr_handle_;
		pthread_mutex_t m_thr_mutex_;
#endif
		bool			m_terminated_;
		INT32			m_thr_index_;
		string			m_thr_name_;
	};


#ifdef WIN32
	static void thread_proc(void *param)
	{
		if(param != NULL)
		{
			::CoInitialize(NULL);

			srand((UINT32)time(NULL));

			CIDCardThread* thr = (CIDCardThread *)param;
			thr->execute();

			::CoUninitialize();
		}
	}
#else
	static void* thread_proc(void *param)
	{
		if(param != NULL)
		{
			srand((UINT32)time(NULL));
			srandom((UINT32)time(NULL));

			CIDCardThread* thr = (CIDCardThread *)param;
			thr->execute();
		}

		return NULL;
	}
#endif

#if defined(WIN32)
	class IDCardThreadMutex
	{
	public:
		IDCardThreadMutex()
		{
			InitializeCriticalSection(&m_section_);
		};

		~IDCardThreadMutex()
		{
			DeleteCriticalSection(&m_section_);
		};

		void acquire()
		{
			EnterCriticalSection(&m_section_);
		};

		bool try_acquire()
		{
			return (TryEnterCriticalSection(&m_section_) != FALSE) ? true : false;
		};

		void release()
		{
			LeaveCriticalSection(&m_section_);
		};

	private:
		CRITICAL_SECTION m_section_;
	};

#else

	class IDCardThreadMutex
	{
	public:
		IDCardThreadMutex()
		{
			pthread_mutexattr_t mutex_a;
			pthread_mutexattr_init(&mutex_a);
			pthread_mutexattr_settype(&mutex_a, PTHREAD_MUTEX_RECURSIVE);

			pthread_mutex_init(&m_mutex_, &mutex_a);
			pthread_mutexattr_destroy(&mutex_a);
		};

		~IDCardThreadMutex()
		{
			pthread_mutex_destroy(&m_mutex_);
		};

		void acquire()
		{
			pthread_mutex_lock(&m_mutex_);
		}

		bool try_acquire()
		{
			return pthread_mutex_trylock(&m_mutex_) == 0 ? true : false;
		}

		void release()
		{
			pthread_mutex_unlock(&m_mutex_);
		}

	private:
		pthread_mutex_t m_mutex_;
	};
#endif
}

#endif