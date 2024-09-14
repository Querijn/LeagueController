#pragma once

#include <memory>
#include <string_view>

namespace LeagueController
{
	struct SharedMemoryData;
	using SharedMemoryDataPtr = std::shared_ptr<SharedMemoryData>;

	class BaseSharedMemory
	{
	public:
		BaseSharedMemory(std::string_view name, size_t size, bool owner);

		bool IsValid() const;

	protected:
		SharedMemoryDataPtr m_data;

		void* GetBuffer() const;
	};

	template<typename T>
	class SharedMemory : public BaseSharedMemory
	{
	public:
		SharedMemory(std::string_view name, bool owner) :
			BaseSharedMemory(name, sizeof(T), owner)
		{
		}

		T* GetData()
		{
			return reinterpret_cast<T*>(GetBuffer());
		}
	};
}