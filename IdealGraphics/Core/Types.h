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

#define MIN_uint8		((uint8)	0x00)
#define	MIN_uint16		((uint16)	0x0000)
#define	MIN_uint32		((uint32)	0x00000000)
#define MIN_uint64		((uint64)	0x0000000000000000)
#define MIN_int8		((int8)		-128)
#define MIN_int16		((int16)	-32768)
#define MIN_int32		((int32)	0x80000000)
#define MIN_int64		((int64)	0x8000000000000000)

#define MAX_uint8		((uint8)	0xff)
#define MAX_uint16		((uint16)	0xffff)
#define MAX_uint32		((uint32)	0xffffffff)
#define MAX_uint64		((uint64)	0xffffffffffffffff)
#define MAX_int8		((int8)		0x7f)
#define MAX_int16		((int16)	0x7fff)
#define MAX_int32		((int32)	0x7fffffff)
#define MAX_int64		((int64)	0x7fffffffffffffff)
