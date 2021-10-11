#pragma once

#include <stdint.h>
#include <string.h>


#define BG_U32_MAX 0xffffffff

// platform specific includes

#if defined(_WIN32) || defined(_WIN64)
    #ifndef BG_SYSTEM_WINDOWS
        #define BG_SYSTEM_WINDOWS 1
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #ifndef BG_SYSTEM_OSX
        #define BG_SYSTEM_OSX 1
    #endif
#elif defined(__linux__)
    #ifndef BG_SYSTEM_LINUX
        #define BG_SYSTEM_LINUX 1
    #endif
#else
    #error unsupported platform
#endif


#if defined(_MSC_VER) && !defined(__clang__)
    #define BG_COMPILER_MSVC 1
#elif defined(__clang__)
    #define BG_COMPILER_CLANG 1
#elif defined(__GNUC__)
    #define BG_COMPILER_GCC 1
#else
    #error UNKNOWN COMPILER
#endif


#if BG_SYSTEM_WINDOWS
	#define BG_DEBUGBREAK __debugbreak()
#else 
	#define BG_DEBUGBREAK __asm__ volatile("int $0x03")
#endif

#if BG_DEVELOPER
    #define BG_ASSERT(exp) do{if (!(exp)) { BG_DEBUGBREAK; }} while (0);
#else
    #define BG_ASSERT(exp) 
#endif

#if !defined(BG_ARR_BOUNDS_CHECK)
#if defined(BG_DEVELOPER)
#if BG_DEVELOPER
#define BG_ARR_BOUNDS_CHECK 1
#endif
#endif
#endif


// typedefs 


#if BG_SYSTEM_WINDOWS
    typedef uint64_t u64;
    typedef  int64_t s64;
    static_assert(sizeof(s64) == sizeof(u64));

    typedef uint32_t u32;
    typedef  int32_t s32;
    static_assert(sizeof(s32) == sizeof(u32));

    typedef uint16_t u16;
    typedef  int16_t s16;
    static_assert(sizeof(s16) == sizeof(u16));

    typedef  uint8_t s8;
    typedef   int8_t u8;
    static_assert(sizeof(s8) == sizeof(u8));

    typedef wchar_t BgUtf16;
#endif

#if BG_SYSTEM_LINUX
    typedef unsigned long long u64;
    typedef          long long s64;
    static_assert(sizeof(u64) == 8);
    static_assert(sizeof(s64) == sizeof(u64));

    typedef unsigned int u32;
    typedef          int s32;
    static_assert(sizeof(u32) == 4);
    static_assert(sizeof(s32) == sizeof(u32));

    typedef unsigned short u16;
    typedef          short s16;
    static_assert(sizeof(u16) == 2);
    static_assert(sizeof(s16) == sizeof(u16));

    typedef unsigned char  u8;
    typedef          char  s8;
    static_assert(sizeof(u8) ==  1);
    static_assert(sizeof(s8) == sizeof(u8));

    typedef u16 BgUtf16;
#endif


#if !defined(BG_ARR_BOUNDS_CHECK)
    #define BG_ARR_BOUNDS_CHECK 0
#endif

#define bg_sizeof(x)  (u64)sizeof(x)
#define bg_unused(x) ((void)(bg_sizeof(x)))


#define Kilobyte(val) ((val)*1024ll)
#define Megabyte(val) (Kilobyte(val)*1024ll)
#define Gigabyte(val) (Megabyte(val)*1024ll)

#define bg_internal      static 
#define bg_local_persist static



#if BG_SYSTEM_WINDOWS
    #define BG_DLLEXPORT     __declspec(dllexport)
    #define BG_DLLIMPORT     __declspec(dllimport)
#else
    #define BG_DLL_EXPORT    __attribute__((visibility("default")))
    #define BG_DLL_IMPORT 
#endif

#if BG_SYSTEM_WINDOWS
    #if defined(BG_BUILD_AS_DLL)
        #define BG_API BG_DLLEXPORT
    #else
        #define BG_API BG_DLLIMPORT
    #endif
#else
    #define BG_API
#endif

#include <stdio.h>
#if BG_SYSTEM_LINUX
    #include <cstdlib>
#endif


#define LOG(str, ...)          do{char bg__bf[1024 * 4]; snprintf(bg__bf, sizeof(bg__bf), str, ##__VA_ARGS__); fprintf(stdout, "%s", bg__bf); fflush(stdout);} while (0);
#define LOG_INFO(str, ...)     do{char bg__bf[1024 * 4]; snprintf(bg__bf, sizeof(bg__bf), str, ##__VA_ARGS__); fprintf(stdout, "%s", bg__bf); fflush(stdout);} while (0);
#define LOG_ERROR(str, ...)    do{char bg__bf[1024 * 4]; snprintf(bg__bf, sizeof(bg__bf), str, ##__VA_ARGS__); fprintf(stdout, "%s", bg__bf); fflush(stdout);} while (0);
#define LOG_WARNING(str, ...)  do{char bg__bf[1024 * 4]; snprintf(bg__bf, sizeof(bg__bf), str, ##__VA_ARGS__); fprintf(stdout, "%s", bg__bf); fflush(stdout);} while (0);


#define bg_malloc(n)        malloc((u64)(n))
#define bg_realloc(p, sz)   realloc((p), (u64)(sz))
#define bg_free(p)          free((p))
#define bg_calloc(c,s)      calloc((u64)(c), (u64)(s))

static inline void
bg_check_for_memory_leaks() {
    BG_ASSERT(false);
} 

#define BG_MIN(a, b) ((a) > (b) ? (b) : (a))
#define BG_MAX(a, b) ((a) < (b) ? (b) : (a))


#define memory_equal(p1, p2, ps) (!!(memcmp(p1, p2, ps) == 0))
#define zero_memory(p, size)      memset(p, 0, size)
#define copy_memory(p1, p2, ps)   memcpy(p1, p2, ps);


// POOL
struct Pool_Entry {
    Pool_Entry *next;
};


struct Pool_Allocator {
    void *memory;
    Pool_Entry *entries;
    u64 pool_size;
    u64 entry_count;
};


static inline Pool_Allocator
init_pool_allocator(void *mem, u64 msize, u64 psize) {

    if (mem == 0) return {};

    BG_ASSERT(mem);
    BG_ASSERT(msize >= psize);
    BG_ASSERT(msize > 0);
    BG_ASSERT(psize > 0);

    Pool_Allocator result ={};
    result.memory          = mem;
    result.pool_size       = psize;
    result.entry_count     = msize / psize;

    for (u64 i = 0; i < (u64)result.entry_count - 1; i++) {
        Pool_Entry *entry = (Pool_Entry *)((char *)mem + (psize * i));
        entry->next        = (Pool_Entry *)((char *)entry + psize);
    }

    Pool_Entry *entry = (Pool_Entry *)((char *)mem + (psize * (result.entry_count - 1)));

    entry->next     = 0;
    result.entries = (Pool_Entry *)mem;

    return result;
}

static inline void *
pool_allocate(Pool_Allocator *alloc) {
    if (alloc->entries == 0) {
        return 0;
    }
    void *result = alloc->entries;
    alloc->entries = alloc->entries->next;
    return result;
}

static inline void
pool_dealloc(Pool_Allocator *alloc, void *mem) {
    if (mem == 0) {
        return;
    }

    Pool_Entry *e = alloc->entries;
    // edge case
    zero_memory(mem, alloc->pool_size);
    if (alloc->entries == 0) {
        alloc->entries = (Pool_Entry *)mem;
        return;
    }

    while (1) {
        if (e->next == 0) {
            e->next = (Pool_Entry *)mem;
            break;
        }
        e = e->next;
    }
}

// #include "stb_ds.h"
// #include "stb_sprintf.h"


#pragma warning(push)
#pragma warning(disable : 4626)
template <typename F>
struct privDefer {
    F f;
    privDefer(F f) : f(f) {}
    ~privDefer() { f(); }
};

template <typename F>
privDefer<F> defer_func(F f) {
    return privDefer<F>(f);
}
#pragma warning(pop)


#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&]() {code;})

#define for_array(_index, array) for (u64 _index = 0; _index < (array).len; (_index)++)

template<typename T>
struct Array {
    T *data = 0;
    u64 len = 0;
    u64 cap = 0;
    inline T &operator[](u64 i) {

#if BG_ARR_BOUNDS_CHECK
        BG_ASSERT(i < this->len);
#endif

        return this->data[i];
    }
};

template<typename T>
void
arrinit(Array<T> *arr, u64 capacity);

template<typename T>
void
arrput(Array<T> *arr, T val);

template<typename T>
void
arrputn(Array<T> *arr, T *values, u64 n);

template<typename T>
T
arrpop(Array<T> *arr);

template<typename T>
void
arrfree(Array<T> *arr);

template<typename T>
void
arrreserve(Array<T> *arr, u64 n);

template<typename T>
void
arr__grow(Array<T> *arr, u64 new_cap);


template<typename T>
void
arrinit(Array<T> *arr, u64 cap) {
    BG_ASSERT(arr->len == 0);
    BG_ASSERT(arr->data == NULL);
    arr->data = NULL;
    arr__grow(arr, cap);
}

template<typename T>
void
arr__grow(Array<T> *arr, u64 new_cap) {
    if (arr->cap > new_cap)
        return;

    u64 min_cap = 0;
    if (arr->cap <= 8)
        min_cap = 8;
    if (min_cap < new_cap) {
        if (min_cap * 2 + 4 < new_cap)
            min_cap = new_cap;
        else
            min_cap = min_cap * 2 + 4;
    }

    arr->data = (T *)bg_realloc(arr->data, min_cap * bg_sizeof(arr->data[0]));
    arr->cap  = min_cap;

    zero_memory(arr->data + arr->len, (arr->cap - arr->len) * bg_sizeof(arr->data[0]));
}


template<typename T>
void
arrput(Array<T> *arr, T val) {
    if (arr->len == arr->cap) {
        arr__grow(arr, arr->cap + 1);
    }
    arr->data[arr->len] = val;
    arr->len++;
}

template<typename T>
void
arrputn(Array<T> *arr, T *values, u64 n) {
    if (arr->len + n >= arr->cap) {
        arr__grow(arr, arr->len + n);
    }
    BG_ASSERT(arr->len + n <= arr->cap);
    copy_memory(arr->data + arr->len, values, n * sizeof(arr->data[0]));
    arr->len += n;
}

template<typename T>
T
arrpop(Array<T> *arr) {
    if (arr->len > 0) {
        return arr.data[--arr->len];
    }
    return {};
}

template<typename T>
void
arrfree(Array<T> *arr) {
    bg_free(arr->data);
    arr->len = 0;
    arr->cap = 0;
}

template<typename T>
void
arrreserve(Array<T> *arr, u64 n) {
    arr__grow(arr, n);
}

template<typename T>
T *
arrputnempty(Array<T> *arr, u64 count) {
    if (arr->len + count >= arr->cap)
        arr__grow(arr, arr->len + count);

    T *result = &arr->data[arr->len];
    arr->len += count;
    ///zero_memory(result, count * bg_sizeof(arr->data[0]));
    return result;
}

template<typename T>
struct Slice {
    T *data;
    u64 len;
    inline T &operator[](u64 i) {
#if BG_ARR_BOUNDS_CHECK
        BG_ASSERT(i < this->len);
#endif
        return data[i];
    }
};

template<typename T> void
slice_from_array(Slice<T> *slice, Array<T> const &arr) {
    slice->data = arr.data;
    slice->len  = arr.len;
}



#if 1
//
// THREADING , MUTEX, LOCKS ETC 
//
#if BG_SYSTEM_WINDOWS
#include <windows.h>
#endif

struct Mutex {
#if BG_SYSTEM_WINDOWS
    CRITICAL_SECTION critical_section;
#else
    pthread_mutex_t  m;
#endif
};
#endif

Mutex
init_mutex();

void
lock_mutex(Mutex *mutex);

void
unlock_mutex(Mutex *mutex);

bool
try_lock_mutex(Mutex *mutex);


// RANDOM

typedef struct { uint64_t state;  uint64_t inc; } pcg32_random_t;

struct Bg_Random_State {
    pcg32_random_t rng;
};

static inline Bg_Random_State
bg_init_random(u64 init_state) {
    Bg_Random_State result;
    pcg32_random_t result_inner; 
    result_inner.state = init_state;
    result_inner.inc   = 646464;
    
    result.rng = result_inner;
    return result;
}


// msvc doesn't like what pcg does here, but i dont care, didn't even read source code.
#if BG_COMPILER_MSVC 
    #pragma warning(push)
    #pragma warning(disable : 4244)
    #pragma warning(disable : 4146)
#endif

static inline u32 
pcg32_random_r(pcg32_random_t* rng) {
    u64 oldstate = rng->state;
    rng->state = oldstate * 6364136223846793005ULL + rng->inc;
    u32 xorshifted = ((oldstate >> 18u) ^ oldstate) >> 27u;
    u32 rot = oldstate >> 59u;
    return (xorshifted >> rot) | (xorshifted << ((-rot) & 31));
}

#if BG_COMPILER_MSVC
    #pragma warning(pop)
#endif

static inline u64
bg_random(Bg_Random_State *state) {
    return pcg32_random_r(&state->rng);
}

static inline u64
bg_random_between(Bg_Random_State *state, u64 begin, u64 end) {
    u64 rand = bg_random(state);
    BG_ASSERT(end > begin);
    rand = rand % (end - begin);
    rand += begin;
    return rand;
}






// FILE IO
struct File_Read {
    void *data;
    u64 len;
};

enum File_Access_Flags {
    File_Access_Flags_Read_Only,
    File_Access_Flags_Read_And_Write
};

enum File_Share_Flags {
    File_Share_Flags_Share_None,
    File_Share_Flags_Share_Write,
    File_Share_Flags_Share_Read
};

enum File_Open_Flags {
    File_Open_Flags_Existing,
    File_Open_Flags_New
}; 

enum File_Seek_Whence {
    File_Seek_Whence_Begin,
    File_Seek_Whence_End
};

enum IO_Result {
    IO_Result_Error = 0,
    IO_Result_Done = 1,
    IO_Result_Pending = 2
};

struct File {
#if BG_SYSTEM_WINDOWS
    HANDLE     handle;
#else
	int fd;
#endif
	s64 cached_fp = 0;
};

struct Async_IO_Handle {
#if BG_SYSTEM_WINDOWS
	OVERLAPPED overlapped;
#else
	int place_holder;
#endif
};


#if BG_SYSTEM_WINDOWS
#include <windows.h>
#endif

static inline bool
is_file_handle_valid(File *file) {
#if BG_SYSTEM_WINDOWS
    // INVALID_HANDLE_VALUE definition copypasta from microsoft headers.
    return file->handle != INVALID_HANDLE_VALUE;
#else
    return file->fd != -1;
#endif
}


// open-create
File
open_file__raw(char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share);

File
open_file__raw(char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share);

File
open_file(char *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);

File
create_file(char *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);

#if BG_SYSTEM_WINDOWS
File
open_file(BgUtf16 *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);

File
create_file(BgUtf16 *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);
#endif



s64
get_fp(File *file);

s64
set_fp(File *file, s64 offset);


IO_Result
check_file_async_io(File *file, Async_IO_Handle *async);

bool
wait_io_completion(File *file, Async_IO_Handle *async_ctx);


// write
bool
write_file(File *file, void *data, u64 n);

Async_IO_Handle
write_file_async(File *file, void *data, u64 n);

IO_Result
write__file(File *file, void *data, u64 n, s64 target_offset, Async_IO_Handle *async_handle);


// read
bool
read_file(File *file, void *data, u64 n);

Async_IO_Handle
read_file_async(File *file, void *data, u64 n);

IO_Result
read__file(File *file, void *buffer, u64 n, s64 target_offset, Async_IO_Handle *async);


// utility, fs

s64
get_file_size(const char *fn);

s64
get_file_size(const BgUtf16 *fn);


bool
delete_file(char *fn);

bool
delete_file(BgUtf16 *fn);

void
close_file(File *file);

File_Read
read_file_all(char *fn);

void
free_file_read(File_Read *fr);

bool
read_filen(char *fn, void *data, u64 n);

bool
dump_file(char *fn, void *d, u64 n);

#if BG_SYSTEM_WINDOWS
bool
copy_file_overwrite(char *file, char *dest);

bool
copy_file_overwrite(BgUtf16 *file, BgUtf16 *dest);
#endif


Array<char *>
get_file_paths_in_directory(char *dir);

void
free_filelist(Array<char *> &list);







u32
crc32(void *memory, u64 size);


/*
    BG STRING
*/

#if BG_SYSTEM_WINDOWS
// some parts of the code deals with very small strings, using stack or temporary arena fits for their usage
u64
memory_needed_for_conversion(const char *str, u64 slen = 0);

u64
memory_needed_for_conversion(const BgUtf16 *str, u64 slen = 0);
#endif

// @@NOTE(Batuhan) those two (for now), needs external functions to operate, we 
// dont want to pull weight of compilation to everywhere 
BgUtf16 *
multibyte_to_widestr(const char *s);

char *
widestr_to_multibyte(const BgUtf16 *ws);


// those two returns null if conversion can't happen(result can not fit given buffer), caller can give more memory or use memory_needed_for_conversion to determine
// exact memory.
char *
widestr_to_multibyte(const BgUtf16 *ws, char *result, u64 max_ch); 

BgUtf16 *
multibyte_to_widestr(const char *s, BgUtf16 *result, u64 max_ch); 



static inline void
string_replace_every_ch(char *src, char to_be_searched, char to_be_replaced) {
    for (; *src != 0; src++) {
        if (*src == to_be_searched) {
            *src = to_be_replaced;
        }
    }
}

static inline void
string_replace_every_ch(BgUtf16 *src, BgUtf16 to_be_searched, BgUtf16 to_be_replaced) {
    for (; *src != 0; src++) {
        if (*src == to_be_searched) {
            *src = to_be_replaced;
        }
    }
}


static inline u64
string_length(const BgUtf16 *ws) {
    auto b = ws;
    for (; *ws != 0; ws++);
    return ws - b;
}

static inline u64
string_length(const char *s) {
    auto b = s;
    for (; *s != 0; s++);
    return s - b;
}



static inline BgUtf16 *
get_file_name_needle(BgUtf16 *fp, BgUtf16 *end = 0) {

    if (end != 0 && end < fp)
        return 0;

    u64 ts_i = 0;
    BgUtf16 *base = fp;
    BG_ASSERT(fp);

    if (end == 0) {
        end = (BgUtf16 *)((u64)-1);
        for (; *fp != 0 && fp < end; fp++) {
            if (*fp == L'\\' || *fp == L'/') {
                ts_i = fp - base;
            }
        }
    }
    else {
        for (fp = --end; fp != base; fp--) {
            if (*fp == L'\\' || *fp == L'/') {
                return ++fp;
            }
        }
    }


    return &base[ts_i ? ++ts_i : 0];
}

static inline char *
get_file_name_needle(char *fp, char *end = 0) {

    if (end != 0 && end < fp)
        return 0;
    //[a-z] \*[a-z]
    s64 ts_i = 0;
    char *base = fp;
    BG_ASSERT(fp);

    if (end == 0) {
        end = (char *)((u64)-1);
        for (; *fp != 0 && fp < end; fp++) {
            if (*fp == '\\' || *fp == '/') {
                ts_i = fp - base;
            }
        }
    }
    else {
        for (fp = --end; fp != base; fp--) {
            if (*fp == '\\' || *fp == '/') {
                return ++fp;
            }
        }
    }


    return &base[ts_i ? ++ts_i : 0];
}


static inline u64
string_copy(char *dest, const char *src) {
    u64 result = 0;
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        dest++;
        src++;
        result++;
    }
    return result;
}

static inline u64
string_copy(BgUtf16 *dest, const BgUtf16 *src) {
    u64 result = 0;
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        dest++;
        src++;
        result++;
    }
    return result;
}

static inline BgUtf16 *
string_concanate(BgUtf16 *dest, u64 dest_max, BgUtf16 *lhs, u64 lhs_len, BgUtf16 *rhs, u64 rhs_len) {

    BG_ASSERT(dest);
    BG_ASSERT(lhs);
    BG_ASSERT(rhs);
    BG_ASSERT(lhs_len + rhs_len + 1 <= dest_max);

    if (dest) {
        if (lhs_len + rhs_len > dest_max) {
            LOG_ERROR("Unable to concanate %S and %S, buffer overrun!\n", (wchar_t*)lhs, (wchar_t*)rhs);
            return dest;
        }
        copy_memory(dest, lhs, lhs_len * 2);
        copy_memory(dest + lhs_len, rhs, rhs_len * 2);
        dest[lhs_len + rhs_len] = '\0';
    }

    return dest;
}

static inline char *
string_concanate(char *dest, u64 dest_max, const char *lhs, u64 lhs_len, const char *rhs, u64 rhs_len) {
    BG_ASSERT(dest);
    BG_ASSERT(lhs);
    BG_ASSERT(rhs);
    BG_ASSERT(lhs_len + rhs_len + 1 <= dest_max);

    if (dest) {
        if (lhs_len + rhs_len > dest_max) {
            LOG_ERROR("Unable to concanate %s and %s, buffer overrun!\n", lhs, rhs);
            return dest;
        }
        copy_memory(dest, lhs, lhs_len);
        copy_memory(dest + lhs_len, rhs, rhs_len);
        dest[lhs_len + rhs_len] = '\0';
    }

    return dest;
}

static inline u64
string_append(char *str, const char *app, u64 strlen = 0, u64 applen = 0) {
    if (strlen == 0) strlen = string_length(str);
    if (applen == 0) applen = string_length(app);

    copy_memory(str + strlen, app, applen);
    return strlen + applen;
}

static inline bool
string_equal(const char *s1, const char *s2) {
    for (;;) {
        if (*s1 != *s2)
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
}

static inline bool
is_numeric(char c) {
    return (c >= '0' && c <= '9');
}

static inline bool
is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline bool
is_alphanumeric(char c) {
    return is_numeric(c) && is_alpha(c);
}

static inline u64
string_to_u64(const char *str) {

    // with validation enabled, this runs 6x faster std lib, without this it will be 7x faster, which is about %15-18 perf gain
    for (;;str++) {
        if (*str == 0) 
            return 0;
        if (*str == '-') // there shouldnt be sign on unsigned type, :]
            return 0;
        if (*str != ' '  && *str != '0' && is_numeric(*str))
            break;
        if (*str != ' ' && !is_numeric(*str))
            return 0;

    }
    
    u64 result = *str - '0';
    ++str;
    while(is_numeric(*str)) {
        result = result * 10 + (*str - '0');
        ++str;
    }

    return result;
}

static inline s64
string_to_s64(const char *str) {
    
    s64 sign = 1;

    // with validation enabled, this runs 6x faster std lib, without this, it would be 7x faster, which is extra %15-18 perf gain
    for (;;str++) {
        if (*str == 0) 
            return 0;
        if (*str == '-')
            sign = -1;
        if (*str != ' ' && *str != '0' && is_numeric(*str))
            break;
        if (*str != ' ' && *str != '-' && !is_numeric(*str))
            return 0;
    }

    u64 result = *str - '0';
    ++str;
    while(is_numeric(*str)) {
        result *= 10;
        result += (*str - '0');
        ++str;
    }

    return sign * result;
}


static inline char
to_lower(char c) {
    if (c >= 'A' && c <= 'Z')
        return ('Z' - c) + 'a';
    return c;
}

#if BG_SYSTEM_LINUX
    #include <wctype.h>
#endif

static inline BgUtf16
to_lower(BgUtf16 c) {
    return towlower(c);
}

static inline void
lower_string(char *str) {
    for (; *str != 0; str++) {
        *str = to_lower(*str);
    }
}

static inline void
lower_string(BgUtf16 *str) {
    for (; *str != 0; str++) {
        *str = to_lower(*str);
    }
}

static inline bool
string_equal(BgUtf16 *w1, BgUtf16 *w2) {
    for (;;) {
        if (*w1 != *w2)
            return false;
        w1++; w2++;
        if (*w1 == 0 && *w2 == 0)
            return true;
    }
}


static inline bool
string_equal_n(BgUtf16 *w1, BgUtf16 *w2, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (*w1 != *w2)
            return false;
        w1++; w2++;
        if (*w1 == 0 && *w2 == 0)
            return true;
    }
    return true;
    // stupid asan becomes embrassed if memcmp is shown to it
    // return memory_equal(s1, s2, n * 2);
}

static inline bool
string_equal_ignore_case_n(BgUtf16 *s1, BgUtf16 *s2, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (to_lower(*s1) != to_lower(*s2))
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
    return true;
}

static inline BgUtf16 *
string_duplicate(BgUtf16 *ws) {
    static_assert(sizeof(BgUtf16) == 2);

    u64 wlen = string_length(ws);
    BgUtf16 *result = (BgUtf16 *)bg_calloc(wlen + 1, sizeof(BgUtf16));
    copy_memory(result, ws, wlen * 2);
    return result;
}

static inline char *
string_duplicate(char *str) {
    u64 len = string_length(str);
    char *result = (char *)bg_calloc(len + 1, 1);
    copy_memory(result, str, len);
    return result;
}

static inline bool
compare_extension(char *fp, char *ext) {
    auto fn = get_file_name_needle(fp);

    u64 slen   = string_length(fn);
    u64 extlen = string_length(ext);

    if (extlen > slen) {
        return false;
    }

    return (memory_equal(fn + slen - extlen, ext, extlen));
}

static inline bool
compare_extension(BgUtf16 *fp, BgUtf16 *ext) {

    auto fn = get_file_name_needle(fp);

    u64 slen   = string_length(fn);
    u64 extlen = string_length(ext);

    if (extlen > slen) {
        return false;
    }

    return memory_equal(fn + slen - extlen, ext, extlen);
}

/*
    END OF BG STRING
*/




#ifdef BG_IMPLEMENTATION

#if BG_SYSTEM_WINDOWS
    #include <Windows.h>
    #include <debugapi.h>
    #include <WinIoCtl.h>
#endif

#if BG_SYSTEM_LINUX
    #define _LARGEFILE64_SOURCE 1    /* See feature_test_macros(7) */
    #include <sys/types.h>
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <pthread.h>

    

#endif

Mutex
init_mutex() {
#if BG_SYSTEM_WINDOWS
    Mutex result = {};
    InitializeCriticalSection(&result.critical_section);
    return result;
#else
    Mutex result = {};
    pthread_mutex_init(&result.m, NULL);
    return result; 
#endif
}


void
lock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    EnterCriticalSection(&mutex->critical_section);
#else
    pthread_mutex_lock(&mutex->m);
#endif
}

void
unlock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    LeaveCriticalSection(&mutex->critical_section);
#else
    pthread_mutex_unlock(&mutex->m);
#endif
}

bool
try_lock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    return TryEnterCriticalSection(&mutex->critical_section);
#else
    return 0 == pthread_mutex_trylock(&mutex->m);
#endif
}


constexpr u32 BG__CRC32_TABLE[256] = {
    0x00000000, 0x77073096, 0xee0e612c, 0x990951ba,
    0x076dc419, 0x706af48f, 0xe963a535, 0x9e6495a3,
    0x0edb8832, 0x79dcb8a4, 0xe0d5e91e, 0x97d2d988,
    0x09b64c2b, 0x7eb17cbd, 0xe7b82d07, 0x90bf1d91,
    0x1db71064, 0x6ab020f2, 0xf3b97148, 0x84be41de,
    0x1adad47d, 0x6ddde4eb, 0xf4d4b551, 0x83d385c7,
    0x136c9856, 0x646ba8c0, 0xfd62f97a, 0x8a65c9ec,
    0x14015c4f, 0x63066cd9, 0xfa0f3d63, 0x8d080df5,
    0x3b6e20c8, 0x4c69105e, 0xd56041e4, 0xa2677172,
    0x3c03e4d1, 0x4b04d447, 0xd20d85fd, 0xa50ab56b,
    0x35b5a8fa, 0x42b2986c, 0xdbbbc9d6, 0xacbcf940,
    0x32d86ce3, 0x45df5c75, 0xdcd60dcf, 0xabd13d59,
    0x26d930ac, 0x51de003a, 0xc8d75180, 0xbfd06116,
    0x21b4f4b5, 0x56b3c423, 0xcfba9599, 0xb8bda50f,
    0x2802b89e, 0x5f058808, 0xc60cd9b2, 0xb10be924,
    0x2f6f7c87, 0x58684c11, 0xc1611dab, 0xb6662d3d,
    0x76dc4190, 0x01db7106, 0x98d220bc, 0xefd5102a,
    0x71b18589, 0x06b6b51f, 0x9fbfe4a5, 0xe8b8d433,
    0x7807c9a2, 0x0f00f934, 0x9609a88e, 0xe10e9818,
    0x7f6a0dbb, 0x086d3d2d, 0x91646c97, 0xe6635c01,
    0x6b6b51f4, 0x1c6c6162, 0x856530d8, 0xf262004e,
    0x6c0695ed, 0x1b01a57b, 0x8208f4c1, 0xf50fc457,
    0x65b0d9c6, 0x12b7e950, 0x8bbeb8ea, 0xfcb9887c,
    0x62dd1ddf, 0x15da2d49, 0x8cd37cf3, 0xfbd44c65,
    0x4db26158, 0x3ab551ce, 0xa3bc0074, 0xd4bb30e2,
    0x4adfa541, 0x3dd895d7, 0xa4d1c46d, 0xd3d6f4fb,
    0x4369e96a, 0x346ed9fc, 0xad678846, 0xda60b8d0,
    0x44042d73, 0x33031de5, 0xaa0a4c5f, 0xdd0d7cc9,
    0x5005713c, 0x270241aa, 0xbe0b1010, 0xc90c2086,
    0x5768b525, 0x206f85b3, 0xb966d409, 0xce61e49f,
    0x5edef90e, 0x29d9c998, 0xb0d09822, 0xc7d7a8b4,
    0x59b33d17, 0x2eb40d81, 0xb7bd5c3b, 0xc0ba6cad,
    0xedb88320, 0x9abfb3b6, 0x03b6e20c, 0x74b1d29a,
    0xead54739, 0x9dd277af, 0x04db2615, 0x73dc1683,
    0xe3630b12, 0x94643b84, 0x0d6d6a3e, 0x7a6a5aa8,
    0xe40ecf0b, 0x9309ff9d, 0x0a00ae27, 0x7d079eb1,
    0xf00f9344, 0x8708a3d2, 0x1e01f268, 0x6906c2fe,
    0xf762575d, 0x806567cb, 0x196c3671, 0x6e6b06e7,
    0xfed41b76, 0x89d32be0, 0x10da7a5a, 0x67dd4acc,
    0xf9b9df6f, 0x8ebeeff9, 0x17b7be43, 0x60b08ed5,
    0xd6d6a3e8, 0xa1d1937e, 0x38d8c2c4, 0x4fdff252,
    0xd1bb67f1, 0xa6bc5767, 0x3fb506dd, 0x48b2364b,
    0xd80d2bda, 0xaf0a1b4c, 0x36034af6, 0x41047a60,
    0xdf60efc3, 0xa867df55, 0x316e8eef, 0x4669be79,
    0xcb61b38c, 0xbc66831a, 0x256fd2a0, 0x5268e236,
    0xcc0c7795, 0xbb0b4703, 0x220216b9, 0x5505262f,
    0xc5ba3bbe, 0xb2bd0b28, 0x2bb45a92, 0x5cb36a04,
    0xc2d7ffa7, 0xb5d0cf31, 0x2cd99e8b, 0x5bdeae1d,
    0x9b64c2b0, 0xec63f226, 0x756aa39c, 0x026d930a,
    0x9c0906a9, 0xeb0e363f, 0x72076785, 0x05005713,
    0x95bf4a82, 0xe2b87a14, 0x7bb12bae, 0x0cb61b38,
    0x92d28e9b, 0xe5d5be0d, 0x7cdcefb7, 0x0bdbdf21,
    0x86d3d2d4, 0xf1d4e242, 0x68ddb3f8, 0x1fda836e,
    0x81be16cd, 0xf6b9265b, 0x6fb077e1, 0x18b74777,
    0x88085ae6, 0xff0f6a70, 0x66063bca, 0x11010b5c,
    0x8f659eff, 0xf862ae69, 0x616bffd3, 0x166ccf45,
    0xa00ae278, 0xd70dd2ee, 0x4e048354, 0x3903b3c2,
    0xa7672661, 0xd06016f7, 0x4969474d, 0x3e6e77db,
    0xaed16a4a, 0xd9d65adc, 0x40df0b66, 0x37d83bf0,
    0xa9bcae53, 0xdebb9ec5, 0x47b2cf7f, 0x30b5ffe9,
    0xbdbdf21c, 0xcabac28a, 0x53b39330, 0x24b4a3a6,
    0xbad03605, 0xcdd70693, 0x54de5729, 0x23d967bf,
    0xb3667a2e, 0xc4614ab8, 0x5d681b02, 0x2a6f2b94,
    0xb40bbe37, 0xc30c8ea1, 0x5a05df1b, 0x2d02ef8d,
};


u32
crc32(void *data, u64 len) {
    u64 remaining;
    u32 result = ~((u32)0);
    u8 const *c = (u8 const *)data;
    for (remaining = len; remaining--; c++) {
        result = (result >> 8) ^ (BG__CRC32_TABLE[(result ^ *c) & 0xff]);
    }
    return ~result;
}

#if BG_SYSTEM_WINDOWS
u64
memory_needed_for_conversion(const char *str, u64 slen) {
    if (slen == 0) slen = string_length(str) + 1;
    int chneeded = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)str, (s32)slen, NULL, NULL);
    BG_ASSERT(chneeded);
    BG_ASSERT(chneeded > 0);
    return (u64)(chneeded * 2);
}

u64
memory_needed_for_conversion(const BgUtf16 *str, u64 slen){
    if (slen == 0) slen = string_length(str) + 1;
    int chneeded = WideCharToMultiByte(CP_UTF8, 0, (wchar_t*)str, (s32)slen, NULL, NULL, NULL, NULL);
    BG_ASSERT(chneeded >= 0);
    return (u64)chneeded;
}
#endif

BgUtf16 *
multibyte_to_widestr(const char *s) {
    BgUtf16 *result = 0;

    int slen = (int)string_length(s) + 1;

#if defined(_WIN32)
    int chneeded = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)s, slen, 0, 0);
    BG_ASSERT(chneeded != 0);
    //chneeded += 1;

    result = (BgUtf16 *)bg_calloc(chneeded, 2);
    BG_ASSERT(result);

    int wr = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)s, slen, (wchar_t *)result, chneeded);
    BG_ASSERT(wr == chneeded);
    bg_unused(wr); // only used in debug build, nothing to worry about 
#else
    result = (BgUtf16 *)bg_calloc(slen + 2, 2);
    mbstowcs((wchar_t *)result, s, slen + 2);
#endif

    return result;
}

char *
widestr_to_multibyte(const BgUtf16 *ws) {
    char *result = 0;

    int wslen = (int)string_length(ws);

#if defined(_WIN32)
    int chneeded = WideCharToMultiByte(CP_UTF8, 0, ws, wslen, NULL, 0, NULL, NULL);

    result = (char *)bg_calloc((u64)chneeded, 1);

    int wr = WideCharToMultiByte(CP_UTF8, 0, ws, wslen, (LPSTR)result, chneeded, NULL, NULL);
    BG_ASSERT(wr <= chneeded);
    bg_unused(wr); // used only in debug build, nothing to worry about
#else
    result = (char *)bg_calloc(wslen + 2, 1);
    wcstombs(result, (wchar_t *)ws, wslen + 2);
#endif

    return result;
}

char *
widestr_to_multibyte(const BgUtf16 *ws, char *result, u64 max_size) {
#if BG_SYSTEM_WINDOWS
    u64 wslen = string_length(ws) + 1; // for null termination
    u64 memneeded = memory_needed_for_conversion(ws, wslen);
    if (memneeded > max_size)
        return NULL;

    int bw = WideCharToMultiByte(CP_UTF8, 0, ws, (s32)wslen, (LPSTR)result, (s32)max_size, NULL, NULL);
    return ((u64)bw <= max_size ? result : NULL);
#else
    u64 wslen = string_length(ws);
    u64 mn    = (wslen + 1) * 2;
    if (mn > max_size) {
        return NULL;
    }
    size_t rv = wcstombs(result, (wchar_t *)ws, max_size);
    return rv == (size_t)-1 ? NULL : result;
#endif
}

BgUtf16 *
multibyte_to_widestr(const char *s, BgUtf16 *result, u64 max_size) {
#if BG_SYSTEM_WINDOWS
    u64 slen = string_length(s) + 1; // for null termination
    u64 memneeded = memory_needed_for_conversion(s, slen);
    if (memneeded > max_size) 
        return NULL;

    int bw = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)s, (s32)slen, result, (s32)(max_size / 2));
    return ((u64)bw*2 <= max_size ? result : NULL); 
#else
    u64 slen = string_length(s);
    u64 mn   = (slen + 1) * 2;
    if (mn > max_size) {
        return NULL;
    }
    size_t rv = mbstowcs((wchar_t *)result, s, max_size/2);
    return rv == (size_t)-1 ? NULL : result;
#endif
}

s64
get_fp(File *file) {

#if BG_SYSTEM_WINDOWS
    return file->cached_fp;
    #if 0

    LARGE_INTEGER start  ={};
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file->handle, start, &result, FILE_CURRENT);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to get file pointer\n");
    }

    return result.QuadPart;
    #endif
#else
    s64 result = lseek64(file->fd, 0, SEEK_CUR);
    return result;
#endif
}

s64
set_fp(File *file, s64 offset) {
    
#if BG_SYSTEM_WINDOWS
    file->cached_fp = offset;
    return file->cached_fp;
    #if 0
    LARGE_INTEGER start ={};
    start.QuadPart = offset;
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file->handle, start, &result, FILE_BEGIN);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to set file pointer\n");
    }
    return result.QuadPart == offset;
    #endif
#else
    s64 result = lseek64(file->fd, offset, SEEK_SET);
    return result;
#endif
}

bool
write_file(File *file, void *data, u64 n) {

    Async_IO_Handle handle = {};
    s64 current_fp = get_fp(file);

    auto io_result = write__file(file, data, n, current_fp, &handle);

    if (io_result == IO_Result_Pending) {
        return wait_io_completion(file, &handle);
    }

    return io_result == IO_Result_Done;
}

Async_IO_Handle
write_file_async(File *file, void *data, u64 n, u64 write_offset) {
    Async_IO_Handle handle = {};
    write__file(file, data, n, write_offset, &handle);
    return handle;
}

IO_Result
write__file(File *file, void *data, u64 n, s64 write_offset, Async_IO_Handle *async_handle) {
#if BG_SYSTEM_WINDOWS
    BG_ASSERT(n < BG_U32_MAX);
    if (n > BG_U32_MAX) {
        LOG_WARNING("Reading bigger than 4GB is not supported yet\n");
        return IO_Result_Error;
    }
    if (n == 0) {
        return IO_Result_Done;
    }

    {
        async_handle->overlapped.Offset     = (write_offset & (0xffffffff));
        async_handle->overlapped.OffsetHigh = (write_offset >> 32);
   	}

    auto writefile_result = WriteFile(file->handle, data, (DWORD)n, NULL, &async_handle->overlapped);	
    
   	if (writefile_result == TRUE) {
        file->cached_fp += n;
    	return IO_Result_Done;
    }
	
    auto errcode = GetLastError();

	BG_ASSERT(writefile_result == FALSE && errcode == ERROR_IO_PENDING);
	if (errcode != ERROR_IO_PENDING) {
		LOG_ERROR("Writefile returned some weird error code %ld\n", errcode);
        return IO_Result_Error;
	}

    file->cached_fp += n;
	return IO_Result_Done;

#else
    LOG_WARNING("Linux async io is not implemented(yet!)\n"); 
    ssize_t lsr = lseek64(file->fd, write_offset, SEEK_SET);
    BG_ASSERT(lsr == write_offset);
    if (lsr != write_offset) {
        LOG_ERROR("Unable to seek file pointer to %lld, aborting write operation\n", write_offset);
        return IO_Result_Error;
    }

    ssize_t ws = write(file->fd, data, n);
    if (ws == -1) {
        LOG_ERROR("Unable to write %lld bytes to file\n", n);
        return IO_Result_Error;
    } 

    file->cached_fp += n;
    return IO_Result_Done;
#endif

}


bool
read_file(File *file, void *buffer, u64 n) {
    Async_IO_Handle handle = {};
    auto current_fp = get_fp(file);
    auto io_result  = read__file(file, buffer, n, current_fp, &handle);

    if (io_result == IO_Result_Pending) {
        return wait_io_completion(file, &handle);
    }

    return io_result == IO_Result_Done;
}

Async_IO_Handle
read_file_async(File *file, void *buffer, u64 n, s64 read_offset) {
    Async_IO_Handle handle = {};
    read__file(file, buffer, n, read_offset, &handle); 
    return handle;
}

// must return true, false indicating something bad happened, error!!
bool
wait_io_completion(File *file, Async_IO_Handle *async_ctx) {
#if BG_SYSTEM_WINDOWS
    DWORD bytes_transferred = 0;
    auto winapi_result = GetOverlappedResultEx(file->handle, &async_ctx->overlapped, &bytes_transferred, INFINITE, FALSE);
    BG_ASSERT(winapi_result != 0);
    if (winapi_result == 0) {
        LOG_WARNING("IO completion wait failed with result %ld\n", GetLastError());
        return false;
    }
    else {
        // success
        return true;
    }
#else
    LOG_ERROR("Wait io completion(async io in general) for linux not yep implemented\n");
    BG_ASSERT(false);
    return true;
#endif
}

// returns true if io completed, for big io stuff it usually returns false.
IO_Result
read__file(File *file, void *buffer, u64 n, s64 read_offset, Async_IO_Handle *async_handle) {

#if BG_SYSTEM_WINDOWS

    BG_ASSERT(n < BG_U32_MAX);
    if (n > BG_U32_MAX) {
        LOG_WARNING("Reading bigger than 4GB is not supported yet\n");
        return IO_Result_Error;
    }


    {
        async_handle->overlapped.Offset     = (read_offset & (0xffffffff));
        async_handle->overlapped.OffsetHigh = (read_offset >> 32);
    }
    auto readfile_result = ReadFile(file->handle, buffer, (DWORD)n, NULL, &async_handle->overlapped);

   	// @NOTE(batuhan): Will we ever hit that?
    if (readfile_result == TRUE) {
    	return IO_Result_Done;
    }
	
    auto errcode = GetLastError();
	BG_ASSERT(readfile_result == FALSE && errcode == ERROR_IO_PENDING);
	if (errcode != ERROR_IO_PENDING) {
		LOG_ERROR("Readfile returned some weird error code %ld\n", errcode);
		return IO_Result_Error;
	}
	
    file->cached_fp += n;
    return IO_Result_Pending;

#else
    s64 sfpr = set_fp(file, read_offset); 
    BG_ASSERT(sfpr == read_offset);
    LOG_WARNING("Async file read for linux not yet implemented\n");
    ssize_t rs = read(file->fd, buffer, n);
    if (rs != n) {
        LOG_ERROR("Unable to read %lld bytes, instead read %ld\n", n, rs);
        return IO_Result_Error;
    }

    file->cached_fp += n;
    return IO_Result_Done;
#endif
}

IO_Result
check_file_async_io(File *file, Async_IO_Handle *async_handle) {
#if BG_SYSTEM_WINDOWS
    DWORD bytes_transferred = 0;
	auto winapi_result = GetOverlappedResultEx(file->handle, &async_handle->overlapped, &bytes_transferred, 0, FALSE);
	if (winapi_result == 0) {
		auto errcode = GetLastError();
		if (errcode == ERROR_IO_INCOMPLETE) {
			// we are ok, io is not ready yet.
			return IO_Result_Pending;
		}
		else {
			if (errcode == ERROR_HANDLE_EOF) {
				LOG_WARNING("Read operation goes beyond EOF, this isn't an error\n");
				return IO_Result_Done;
			}
			else {
				LOG_WARNING("Unknown error code from getoverlappedresultex, %ld\n", errcode);
				return IO_Result_Error;
			}
		}
		
	}
    return IO_Result_Done;
#else
    BG_ASSERT(false);
    return IO_Result_Done;
#endif
}

#if BG_SYSTEM_WINDOWS
File
open_file__raw(BgUtf16 *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {

    DWORD winapi_open = 0;
    // convert open type
    if (File_Open_Flags_New == open_type) {
        winapi_open = CREATE_NEW;
    }
    if (File_Open_Flags_Existing == open_type) {
        winapi_open = OPEN_EXISTING;
    }

    DWORD winapi_share = 0;
    // convert share type
    if (File_Share_Flags_Share_None == share) {
        winapi_share = 0;
    }
    if (File_Share_Flags_Share_Read == share) {
        winapi_share = FILE_SHARE_READ;
    }
    if (File_Share_Flags_Share_Write == share) {
        winapi_share = FILE_SHARE_READ | FILE_SHARE_WRITE;
    }

    DWORD winapi_access = 0;
    // convert access type
    if (File_Access_Flags_Read_Only == access) {
        winapi_access = GENERIC_READ;
    }
    if (File_Access_Flags_Read_And_Write == access) {
        winapi_access = GENERIC_WRITE | GENERIC_READ;
    }

    File result = {};
    result.handle = CreateFileW(fn, winapi_access, winapi_share, 0, winapi_open, FILE_FLAG_OVERLAPPED, 0);
    if (result.handle == INVALID_HANDLE_VALUE) {
        auto errcode = GetLastError();
        LOG_INFO("Unable to open file %S, errcode %ld, access : %ld, share : %ld, open : %ld\n", fn, errcode, winapi_access, winapi_share, winapi_open);
    }

    return result;
}


File
open_file__raw(char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {
    BgUtf16 *wfn = multibyte_to_widestr(fn);
    File result  = open_file__raw(wfn, open_type, access, share);
    bg_free(wfn);
    return result;
}
#endif

#if BG_SYSTEM_LINUX
File
open_file__raw(char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {
    
    int fd    = -1;
    int flags = 0;
    int mode  = 0;

    if (File_Open_Flags_Existing == open_type) {
        flags |= (O_CREAT);
    }
    if (File_Open_Flags_New      == open_type) {
        flags |= (O_CREAT | O_EXCL);
    }


    if (File_Access_Flags_Read_Only == access) {
        flags |= (O_RDONLY);
    } 
    if (File_Access_Flags_Read_And_Write == access) {
        flags |= (O_RDWR);
    }


    mode |= S_IRWXU;
    if (File_Share_Flags_Share_None == share) {

    }
    if (File_Share_Flags_Share_Read == share) {
        mode |= S_IROTH;
    }
    if (File_Share_Flags_Share_Write == share) {
        mode |= (S_IWOTH | S_IROTH);
    }

    fd = open(fn, flags, mode);
    if (fd == -1) {
        LOG_ERROR("Unable to open file %s, flags 0x%X, mode 0x%X\n", fn, flags, mode); 
    }
    File result;
    result.fd = fd;

    return result;
}
#endif


File
open_file(char *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share); 
}

File
create_file(char *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_New, access, share);
}


#if BG_SYSTEM_WINDOWS
File
create_file(BgUtf16 *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share);
}

File
open_file(BgUtf16 *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share); 
}
#endif



bool
delete_file(char *fn) {
#if BG_SYSTEM_WINDOWS
    BOOL winapi_result = DeleteFileA(fn);
    if (winapi_result == 0) {
        LOG_ERROR("Unable to delete file %s, reason %ld\n", fn, GetLastError());
    }
    return winapi_result != 0;
#else
    
    u64 sl = string_length(fn);

    // for small strings, dont heap allocate.
    if (sl + 5 < 1024) {
        char bf[1024];
        zero_memory(bf, sizeof(bf));
        
        u64 u = 0;
        u = string_append(bf, "rm ", u, 0 );
        u = string_append(bf, fn   , u, sl);
        system(bf);
    }
    else {
        u64 bflen = sl + 5;

        char *bf = (char *)bg_calloc(bflen, 1);
        zero_memory(bf, bflen);

        u64 u = 0;
        u = string_append(bf, "rm ", u, 0); 
        u = string_append(bf, fn   , u, sl);
        system(bf);
    }

    return true;

#endif
}


#if BG_SYSTEM_WINDOWS
bool
delete_file(BgUtf16 *fn) {

    BOOL winapi_result = DeleteFileW(fn);
    if (winapi_result == 0) {
        LOG_ERROR("Unable to delete file %S, reason %ld\n", fn, GetLastError());
    }
    return winapi_result != 0;

}
#endif

s64
get_file_size(const char *fn) {
#if BG_SYSTEM_WINDOWS
    s64 result = 0;
    HANDLE file = CreateFileA(fn, FILE_GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
    BG_ASSERT(file != INVALID_HANDLE_VALUE);
    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fsli ={};
        BOOL winapiresult  = GetFileSizeEx(file, &fsli);
        BG_ASSERT(winapiresult);
        bg_unused(winapiresult); // used in debug build. its ok
        result = fsli.QuadPart;
    }
    else {
        LOG_ERROR("Unable to open file %s for file size query operation\n", fn);
    }

    CloseHandle(file);
    return result;
#else

    int flags = O_RDONLY;
    int mode  = 0;
    int fd = open(fn, flags);
    defer({close(fd);});
    LOG_INFO("FD : %d, fn : %s\n", fd, fn);
    if (fd != -1) {
        return lseek64(fd, 0, SEEK_END);
    }
    else {
        LOG_ERROR("Unable to open %s as a file\n", fn);
    }

    return 0;
#endif
}


#if BG_SYSTEM_WINDOWS
s64
get_file_size(const BgUtf16 *fn) {
    s64 result = 0;
    HANDLE file = CreateFileW(fn, FILE_GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
    BG_ASSERT(file != INVALID_HANDLE_VALUE);
    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fsli ={};
        BOOL winapiresult  = GetFileSizeEx(file, &fsli);
        BG_ASSERT(winapiresult);
        bg_unused(winapiresult); // used in debug build. its ok
        result = fsli.QuadPart;
    }
    else {
        LOG_ERROR("Unable to open file %S for file size query operation\n", fn);
    }

    CloseHandle(file);
    return result;
}
#endif

void
close_file(File *file) {
#if BG_SYSTEM_WINDOWS
    CloseHandle(file->handle);
    file->handle = INVALID_HANDLE_VALUE;
    // @TODO cancel all pending io's via CancelIO
#else
    close(file->fd);
    file->fd = -1;
#endif
}


File_Read
read_file_all(char *fn) {
#if BG_SYSTEM_WINDOWS

    File_Read result ={};
    HANDLE file = CreateFileA(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, 0);
    defer({ CloseHandle(file); });

    BG_ASSERT(file != INVALID_HANDLE_VALUE);

    if (file != INVALID_HANDLE_VALUE) {
        DWORD BytesRead = 0;
        result.len = (u64)GetFileSize(file, 0); // safe to assume file size < 2 GB
        if (result.len == 0) return result;
        result.data = bg_malloc((u64)result.len);
        BG_ASSERT(result.data);
        if (result.data) {
            if (ReadFile(file, result.data, (DWORD)result.len, &BytesRead, 0) && BytesRead == (DWORD)result.len) {
                // @NOTE success
            }
            else {
                LOG_WARNING("Unable to read %llu bytes from file %s, instead read %lu\n", result.len, fn, BytesRead);
                bg_free(result.data);
            }
        }
        else {
            LOG_WARNING("Unable to allocate memory for read_file_all(fn : %s) size %llu\n", fn, result.len);
        }
    }
    else {
        LOG_WARNING("Can't create file: %s\n", fn);
    }

    return result;
#else
    
    File_Read result = {};
    s64 fs = get_file_size(fn);
    if (fs) {
        result.data = bg_malloc(fs);
        result.len  = fs;
        if (read_filen(fn, result.data, result.len)) {
            // success
        }
        else {
            LOG_ERROR("Unable to read entire contents of file %s\n", fn);
            bg_free(result.data);
            result.data = 0;
            result.len  = 0;
        }

    }

    return result;
#endif
}

void
free_file_read(File_Read *fr) {
    bg_free(fr->data);
    fr->len = 0;
    fr->data = 0;
}

bool
read_filen(char *fn, void *data, u64 n) {
    BG_ASSERT(n < Gigabyte(2));

#if BG_SYSTEM_WINDOWS

    HANDLE file = CreateFileA(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    defer({ CloseHandle(file); });
    BG_ASSERT(file != INVALID_HANDLE_VALUE);

    if (file != INVALID_HANDLE_VALUE) {
        DWORD br = 0;
        if (ReadFile(file, data, (DWORD)n, &br, 0) && br == n) {
            return true;
        }
        else {
            LOG_ERROR("Unable to read %zd bytes from file %s\n", n, fn);
        }
    }
    else {
        LOG_ERROR("Can't open file %s to read %zd bytes\n", fn, n);
    }

    return false;
#else
    
    if (n == 0) {
        return true;
    }

    int mode  = 0;
    int flags = O_RDONLY;
    errno = 0;
    int fd = open(fn, flags);
    defer({close(fd);});
    if (fd != -1) {
        int rs = read(fd, data, n);
        if (rs == n) {
            return true;
        }
        else {
            LOG_ERROR("Unable to read %lld bytes from file %s, instead read %d, errno %d\n", n, fn, rs, errno);
        }
    }
    else {
        LOG_ERROR("Can't open file %s to read %lld bytes, errno %d\n", fn, n, errno);
    }

    return false;
#endif
}

bool
dump_file(char *fn, void *d, u64 n) {
#if BG_SYSTEM_WINDOWS
    HANDLE file = CreateFileA(fn, GENERIC_WRITE, 0, 0, CREATE_NEW, 0, 0);
    BG_ASSERT(file != INVALID_HANDLE_VALUE);
    DWORD br = 0;

    if (WriteFile(file, d, (DWORD)n, &br, 0) && br == n) {

    }
    else {
        LOG_ERROR("Unable to dump memory(0x%p : %zd) to file %s\n", d, n, fn);
    }
    CloseHandle(file);
    return br == n;
#else

    File file   = open_file__raw(fn, File_Open_Flags_New, File_Access_Flags_Read_And_Write, File_Share_Flags_Share_None);
    bool result = write_file(&file, d, n);
    close_file(&file);

    return result;
#endif
}

#if BG_SYSTEM_WINDOWS
bool
copy_file_overwrite(char *file, char *dest) {
    return CopyFileA(file, dest, true);
}

bool
copy_file_overwrite(BgUtf16 *file, BgUtf16 *dest) {
    return CopyFileW((wchar_t *)file, dest, true);
}
#endif

Array<char *>
get_file_paths_in_directory(char *dir) {
#if defined(_WIN32)

    char wildcard_dir[280] ={};
    u64 dirlen = string_length(dir);
    dirlen++; // null termination

    BG_ASSERT(sizeof(wildcard_dir) > dirlen);

    Array<char *> result ={};
    arrreserve(&result, 20);

    string_append(wildcard_dir, dir);
    string_append(wildcard_dir, "\\*");

    WIN32_FIND_DATAA FDATA;
    HANDLE FileIterator = FindFirstFileA(wildcard_dir, &FDATA);

    if (FileIterator != INVALID_HANDLE_VALUE) {

        while (FindNextFileA(FileIterator, &FDATA) != 0) {

            //@NOTE(Batuhan): Do not search for sub-directories, skip folders.
            if (FDATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }

            // @Stupid +5 just to be sure there is enough room for null termination
            u64 fn_len = string_length(FDATA.cFileName);
            fn_len += dirlen + 5;

            char *fnbuffer = (char *)bg_calloc(fn_len, 1);

            u64 slen = 0;
            slen = string_append(fnbuffer, dir, slen);
            slen = string_append(fnbuffer, "\\", slen);
            slen = string_append(fnbuffer, FDATA.cFileName, slen);

            arrput(&result, fnbuffer);

        }


    }
    else {
        printf("Cant iterate directory\n");
    }


    FindClose(FileIterator);

    return result;
#else 
    // linux
    BG_ASSERT(false);
    return {};
#endif
}

void
free_filelist(Array<char *> &list) {
    arrfree(&list);
}



#endif