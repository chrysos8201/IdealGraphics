#include "Misc/Utils/FileUtils.h"

FileUtils::FileUtils()
{

}

FileUtils::~FileUtils()
{
	if (m_handle != INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_handle);
		m_handle = INVALID_HANDLE_VALUE;
	}
}

void FileUtils::Open(std::wstring FilePath, FileMode Mode)
{
	if (Mode == FileMode::Write)
	{
		m_handle = CreateFile(
			FilePath.c_str(),
			GENERIC_WRITE,
			0,
			nullptr,
			CREATE_ALWAYS,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
	}
	else
	{
		m_handle = CreateFile(
			FilePath.c_str(),
			GENERIC_READ,
			FILE_SHARE_READ,
			nullptr,
			OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			nullptr
		);
	}

	assert(m_handle != INVALID_HANDLE_VALUE);
}

void FileUtils::Write(void* Data, uint32 DataSize)
{
	uint32 numOfBytes = 0;
	assert(WriteFile(m_handle, Data, DataSize, reinterpret_cast<LPDWORD>(&numOfBytes), nullptr));
}

void FileUtils::Write(const std::string& Data)
{
	uint32 size = (uint32)Data.size();
	Write(size);
	
	if (Data.size() == 0)
	{
		return;
	}
	Write((void*)Data.data(), size);
}

void FileUtils::Read(void** Data, uint32 DataSize)
{
	uint32 numOfBytes = 0;
	assert(ReadFile(m_handle, *Data, DataSize, reinterpret_cast<LPDWORD>(&numOfBytes), nullptr));
}

void FileUtils::Read(OUT std::string& Data)
{
	uint32 size = Read<uint32>();
	
	if (size == 0)
	{
		return;
	}

	char* temp = new char[size + 1];
	temp[size] = 0;
	Read((void**)&Data, size);
	Data = temp;
	delete[] temp;
}
