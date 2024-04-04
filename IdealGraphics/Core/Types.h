#pragma once

struct PlatformTypes
{
	//~ Unsigned base types

	typedef unsigned char			uint8;
	typedef unsigned short int		uint16;
	typedef unsigned int			uint32;
	typedef unsigned long long		uint64;

	//~ Signed base types

	typedef signed char				int8;
	typedef signed short int		int16;
	typedef signed int				int32;
	typedef signed long long		int64;

	//~ Character Types

	typedef char					ANSICHAR;
	typedef wchar_t					WIDECHAR;
};

typedef PlatformTypes::uint8		uint8;
typedef PlatformTypes::uint16		uint16;
typedef PlatformTypes::uint32		uint32;
typedef PlatformTypes::uint64		uint64;

typedef PlatformTypes::int8			int8;
typedef PlatformTypes::int16		int16;
typedef PlatformTypes::int32		int32;
typedef PlatformTypes::int64		int64;

typedef PlatformTypes::ANSICHAR		ANSICHAR;
typedef PlatformTypes::WIDECHAR		WIDECHAR;