#pragma once
#include <Windows.h>
#include <functional>
#include "../Raii/Win32Handle.hpp"
#include "Event.hpp"
#include "ThreadStatus.hpp"

namespace Boring32::Async
{
	class Thread
	{
		public:
			virtual void Close();
			virtual ~Thread();

			Thread();
			Thread(void* param, bool destroyOnCompletion);
			Thread(const Thread& other);
			virtual Thread& operator=(const Thread& other);
			Thread(Thread&& other) noexcept;
			virtual Thread& operator=(Thread&& other) noexcept;

			virtual bool operator==(const ThreadStatus status) const noexcept;

		public:
			/// <summary>
			///		Terminates the thread. Be careful when using this
			///		function, as it prevents proper clean up of the 
			///		thread's objects and may leave shared objects in  
			///		an inconsistent state. Note also that if a thread
			///		is waiting on a kernel object, it will not be 
			///		terminated until the wait is finished.
			/// </summary>
			virtual void Terminate();
			virtual void Suspend();
			virtual void Resume();
			virtual bool Join(const DWORD waitTime);
			virtual void Start();
			virtual void Start(int(*simpleFunc)());
			virtual void Start(const std::function<int()>& func);
			virtual ThreadStatus GetStatus() const noexcept;
			virtual UINT GetExitCode() const noexcept;
			virtual Raii::Win32Handle GetHandle() noexcept;
			virtual bool WaitToStart(const DWORD millis);

		protected:
			virtual UINT Run();
			virtual void Copy(const Thread& other);
			virtual void Move(Thread& other) noexcept;
			virtual void InternalStart();
			static UINT WINAPI ThreadProc(void* param);

		protected:
			ThreadStatus m_status;
			UINT m_returnCode;
			UINT m_threadId;
			Raii::Win32Handle m_threadHandle;
			bool m_destroyOnCompletion;
			void* m_threadParam;
			std::function<int()> m_func;
			Event m_started;
	};
}