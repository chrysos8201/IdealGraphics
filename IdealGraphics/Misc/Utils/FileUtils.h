#pragma once
#include "Core/Core.h"

enum class FileMode :uint8
{
	Write,
	Read
};

class FileUtils
{
public:
	FileUtils();
	~FileUtils();

public:
	void Open(std::wstring FilePath, FileMode Mode);

	template <typename T>
	void Write(const T& Data)
	{
		DWORD numOfBytes = 0;
		WriteFile(m_handle, &Data, sizeof(T), (LPDWORD)&numOfBytes, nullptr);
		//assert(WriteFile(m_handle, &Data, sizeof(T), (LPDWORD)&numOfBytes, nullptr));
	}

	template<>
	void Write<std::string>(const std::string& Data)
	{
		return Write(Data);
	}

	void Write(void* Data, uint32 DataSize);
	void Write(const std::string& Data);


	template <typename T>
	void Read(OUT T& Data)
	{
		DWORD numOfBytes = 0;
		BOOL isSuccess = ReadFile(m_handle, &Data, sizeof(T), (LPDWORD)&numOfBytes, nullptr);
		if (!isSuccess)
		{
			assert(false);
		}
	}

	template<typename T>
	T Read()
	{
		T data;
		Read(data);
		return data;
	}

	void Read(void** Data, uint32 DataSize);
	void Read(OUT std::string& Data);

private:
	HANDLE m_handle = INVALID_HANDLE_VALUE;
};