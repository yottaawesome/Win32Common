#include "pch.hpp"
#include "include/Error/Error.hpp"
#include "include/TaskScheduler/RegisteredTask.hpp"

namespace Boring32::TaskScheduler
{
	using Microsoft::WRL::ComPtr;

	RegisteredTask::~RegisteredTask()
	{
		Close();
	}

	RegisteredTask::RegisteredTask(ComPtr<IRegisteredTask> registeredTask)
	:	m_registeredTask(std::move(registeredTask))
	{
		if (m_registeredTask != nullptr)
		{
			HRESULT hr = m_registeredTask->get_Definition(&m_taskDefinition);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get ITaskDefinition", hr);
		}
	}

	void RegisteredTask::Close() noexcept
	{
		m_registeredTask = nullptr;
		m_taskDefinition = nullptr;
	}

	std::wstring RegisteredTask::GetName() const
	{
		CheckIsValid();

		bstr_t taskName;
		HRESULT hr = m_registeredTask->get_Name(taskName.GetAddress());
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get Task name", hr);

		return std::wstring(taskName, taskName.length());
	}

	void RegisteredTask::SetEnabled(const bool isEnabled)
	{
		CheckIsValid();

		HRESULT hr = m_registeredTask->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to set task enabled property", hr);

		/*std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			HRESULT hr = trigger->put_Enabled(isEnabled ? VARIANT_TRUE : VARIANT_FALSE);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to set trigger enabled", hr);
		}*/
	}

	Microsoft::WRL::ComPtr<IRegisteredTask> RegisteredTask::GetRegisteredTask() const
	{
		return m_registeredTask;
	}

	Microsoft::WRL::ComPtr<ITaskDefinition> RegisteredTask::GetTaskDefinition() const
	{
		return m_taskDefinition;
	}

	void RegisteredTask::SetRepetitionInterval(const DWORD intervalMinutes)
	{
		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		for (auto& trigger : triggers)
		{
			ComPtr<IRepetitionPattern> pattern;
			HRESULT hr = trigger->get_Repetition(&pattern);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get task repetition pattern", hr);

			std::wstring interval = L"PT" + std::to_wstring(intervalMinutes) + L"M";
			hr = pattern->put_Interval(bstr_t(interval.c_str()));
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to set trigger repetition pattern interval", hr);
		}
	}

	void RegisteredTask::Run()
	{
		CheckIsValid();

		// The call to Run() will fail if the task doesn't have AllowDemandStart 
		// set to true or the task has been disabled. We can check for these 
		// conditions, but COM does it for us and the error is descriptive, so no 
		// need to bother.
		ComPtr<IRunningTask> runningTask;
		HRESULT hr = m_registeredTask->Run(_variant_t(VT_NULL), &runningTask);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to start task", hr);
	}
	
	UINT RegisteredTask::SetRandomDelay(const DWORD minutes)
	{
		std::vector<ComPtr<ITrigger>> triggers = GetTriggers();
		const std::wstring delay = L"PT" + std::to_wstring(minutes) + L"M";
		UINT triggersUpdated = 0;

		for (const auto& trigger : triggers)
		{
			TASK_TRIGGER_TYPE2 type = TASK_TRIGGER_TYPE2::TASK_TRIGGER_EVENT;
			HRESULT hr = trigger->get_Type(&type);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get ITrigger type", hr);

			// Some, but not all, triggers support random delays, so we only
			// set the daily one for now, as that's the one of interest.
			switch (type)
			{
				case TASK_TRIGGER_TYPE2::TASK_TRIGGER_DAILY:
				{
					ComPtr<IDailyTrigger> dailyTrigger = (IDailyTrigger*)trigger.Get();
					hr = dailyTrigger->put_RandomDelay(_bstr_t(delay.c_str()));
					if (FAILED(hr))
						throw Error::ComError(__FUNCSIG__ ": failed to set trigger random delay", hr);
					triggersUpdated++;
					break;
				}

				default:
				{
					std::wcerr
						<< L"Did not set random delay for a trigger as it's not supported"
						<< std::endl;
				}
			}
		}

		return triggersUpdated;
	}

	std::vector<ComPtr<ITrigger>> RegisteredTask::GetTriggers()
	{
		CheckIsValid();

		ComPtr<ITriggerCollection> triggers;
		HRESULT hr = m_taskDefinition->get_Triggers(&triggers);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get trigger collection", hr);

		long count = 0;
		hr = triggers->get_Count(&count);
		if (FAILED(hr))
			throw Error::ComError(__FUNCSIG__ ": failed to get trigger collection count", hr);

		std::vector<ComPtr<ITrigger>> returnVal;
		for (int i = 1; i <= count; i++) // Collections start at 1
		{
			ComPtr<ITrigger> trigger = nullptr;
			hr = triggers->get_Item(i, &trigger);
			if (FAILED(hr))
				throw Error::ComError(__FUNCSIG__ ": failed to get trigger", hr);

			returnVal.push_back(std::move(trigger));
		}
		return returnVal;
	}

	void RegisteredTask::CheckIsValid() const
	{
		if (m_registeredTask == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_registeredTask is nullptr");
		if (m_taskDefinition == nullptr)
			throw std::runtime_error(__FUNCSIG__ ": m_taskDefinition is nullptr");
	}
}