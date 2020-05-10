#include "pch.hpp"
#include <stdexcept>
#include "include/Raii.hpp"

namespace Win32Utils::Raii
{
	Win32Handle::Win32Handle()
	:	m_handle(nullptr),
		m_inheritable(false)
	{ }
	
	Win32Handle::Win32Handle(const Win32Handle& otherHandle)
	:	m_handle(nullptr),
		m_inheritable(otherHandle.m_inheritable)
	{
		Duplicate(otherHandle.m_handle, otherHandle.m_inheritable);
	}

	Win32Handle::Win32Handle(Win32Handle&& otherHandle) noexcept
	{
		m_inheritable = otherHandle.m_inheritable;
		m_handle = otherHandle.m_handle;
		otherHandle.m_inheritable = false;
		otherHandle.m_handle = nullptr;
	}

	Win32Handle::Win32Handle(const HANDLE handle, const bool inheritable)
	:	m_handle(handle),
		m_inheritable(inheritable)
	{ }

	Win32Handle::~Win32Handle()
	{
		Close();
	}

	HANDLE Win32Handle::GetHandle()
	{
		return m_handle;
	}

	HANDLE& Win32Handle::GetHandleAddress()
	{
		return m_handle;
	}

	void Win32Handle::Close()
	{
		if (m_handle)
		{
			CloseHandle(m_handle);
			m_handle = nullptr;
		}
	}

	bool Win32Handle::operator==(const HANDLE other)
	{
		return m_handle == other;
	}

	void Win32Handle::operator=(const HANDLE other)
	{
		Close();
		m_handle = other;
	}

	bool Win32Handle::operator==(const Win32Handle& other)
	{
		return m_handle == other.m_handle;
	}

	void Win32Handle::operator=(const Win32Handle& other)
	{
		Close();
		Duplicate(other.m_handle, other.m_inheritable);
	}

	void Win32Handle::operator=(Win32Handle&& other) noexcept
	{
		Close();
		if (other.m_handle != nullptr)
		{
			m_inheritable = other.m_inheritable;
			m_handle = other.m_handle;
			other.m_handle = nullptr;
			other.m_inheritable = false;
		}
	}

	HANDLE* Win32Handle::operator&()
	{
		return &m_handle;
	}

	void Win32Handle::Duplicate(const HANDLE otherHandle, const bool inheritable)
	{
		if (otherHandle != nullptr)
		{
			m_inheritable = inheritable;
			bool succeeded = DuplicateHandle(
				GetCurrentProcess(),
				otherHandle,
				GetCurrentProcess(),
				&m_handle,
				0,
				m_inheritable,
				DUPLICATE_SAME_ACCESS
			);
			if (succeeded == false)
				throw std::runtime_error("Failed to duplicate handle.");
		}
	}
}