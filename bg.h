#pragma once

#include <stdint.h>
#include <string.h>

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
    #error  implement debugbreak
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

#if defined(BG_BUILD_AS_DLL)
    #define BG_API BG_DLLEXPORT
#else
    #define BG_API BG_DLLIMPORT
#endif

#include <stdio.h>
#define LOG(str, ...)          do{char __bf[1024 * 4]; snprintf(__bf, sizeof(__bf), str, __VA_ARGS__); OutputDebugStringA(__bf); fprintf(stdout, "%s", __bf); fflush(stdout);} while (0);
#define LOG_INFO(str, ...)     do{char __bf[1024 * 4]; snprintf(__bf, sizeof(__bf), str, __VA_ARGS__); OutputDebugStringA(__bf); fprintf(stdout, "%s", __bf); fflush(stdout);} while (0);
#define LOG_ERROR(str, ...)    do{char __bf[1024 * 4]; snprintf(__bf, sizeof(__bf), str, __VA_ARGS__); OutputDebugStringA(__bf); fprintf(stdout, "%s", __bf); fflush(stdout);} while (0);
#define LOG_WARNING(str, ...)  do{char __bf[1024 * 4]; snprintf(__bf, sizeof(__bf), str, __VA_ARGS__); OutputDebugStringA(__bf); fprintf(stdout, "%s", __bf); fflush(stdout);} while (0);


#define bg_malloc(n)        malloc((u64)(n))
#define bg_realloc(p, sz)   realloc((p), (u64)(sz))
#define bg_free(p)          free((p))
#define bg_calloc(c,s)      calloc((u64)(c), (u64)(s))


#define BG_MIN(a, b) ((a) > (b) ? (b) : (a))
#define BG_MAX(a, b) ((a) < (b) ? (b) : (a))


#define memory_equal(p1, p2, ps) (!!(memcmp(p1, p2, ps) == 0))
#define zero_memory(p, size)      memset(p, 0, size)
#define copy_memory(p1, p2, ps)   memcpy(p1, p2, ps);


// POOL
struct PoolEntry{
    PoolEntry *next;
};


struct PoolAllocator{
    void *memory;
    PoolEntry *entries;
    u64 pool_size;
    u64 entry_count;
};


static inline PoolAllocator
init_pool_allocator(void *mem, u64 msize, u64 psize) {
    
    if (mem == 0) return {};
    
    BG_ASSERT(mem);
    BG_ASSERT(msize >= psize);
    BG_ASSERT(msize > 0);
    BG_ASSERT(psize > 0);

    PoolAllocator result = {};
    result.memory          = mem;
    result.pool_size       = psize;
    result.entry_count     = msize / psize;
    
    for (u64 i = 0; i < (u64)result.entry_count - 1; i++) {
        PoolEntry *entry = (PoolEntry*)((char*)mem + (psize * i));
        entry->next        = (PoolEntry*)((char*)entry + psize);
    }
    
    PoolEntry *entry = (PoolEntry*)((char*)mem + (psize * (result.entry_count - 1)));
    
    entry->next     = 0;
    result.entries = (PoolEntry*)mem;
    
    return result;
}

static inline void*
pool_allocate(PoolAllocator *alloc) {
    if (alloc->entries == 0) {
        return 0;
    }
    void* result = alloc->entries;
    alloc->entries = alloc->entries->next;
    return result;
}

static inline void
pool_dealloc(PoolAllocator *alloc, void *mem) {
    if (mem == 0) {
        return;
    }
    
    PoolEntry* e = alloc->entries;
    // edge case
    zero_memory(mem, alloc->pool_size);
    if (alloc->entries == 0) {
        alloc->entries = (PoolEntry*)mem;
        return;
    }
    
    while (1) {
        if (e->next == 0) {
            e->next = (PoolEntry*)mem;
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
struct Array{
    T *data = 0;
    u64 len = 0;
    u64 cap = 0;
    inline T& operator[](u64 i) { 

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
    BG_ASSERT(arr->len  == 0);
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
        if (min_cap * 2 + 4 < new_cap) {
            min_cap = new_cap;         
        }
        else {
            min_cap = min_cap * 2 + 4;
        }
    }

    arr->data = (T *)bg_realloc(arr->data, min_cap * bg_sizeof(arr[0]));
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
struct Slice{
    T *data;
    u64 len;
    inline T& operator[](u64 i) {
        return data[i];
    }
};

template<typename T> void
slice_from_array(Slice<T> *slice, Array<T> *arr) {
    slice->data = arr->data;
    slice->len  = arr->len;
}



#if 1
//
// THREADING , MUTEX, LOCKS ETC 
//
#if BG_SYSTEM_WINDOWS
#include <windows.h>
#endif

struct BgMutex{
#if BG_SYSTEM_WINDOWS
    CRITICAL_SECTION critical_section;
#else
#error not implemented
#endif
};
#endif

static inline void
lock_mutex(BgMutex *mutex) {
    bg_unused(mutex);
}

static inline void
unlock_mutex(BgMutex *mutex) {
    bg_unused(mutex);
}



// FILE IO
struct FileRead {
    void *data;
    u64 len;
};

typedef void* BgFile;

#if BG_SYSTEM_WINDOWS
    #include <windows.h>
#endif

static inline bool
is_file_handle_valid(BgFile file) {
#if BG_SYSTEM_WINDOWS
    // INVALID_HANDLE_VALUE definition copypasta from microsoft headers.
    return file != INVALID_HANDLE_VALUE;
#else
    #error implement 
#endif
}

s64 
get_fp(BgFile file);

s64 
set_fp(BgFile file, s64 offset);

bool
write_file(BgFile file, void *data, u32 n);

bool
read_file(BgFile file, void *data, u32 n);

s64
get_file_size(char *fn);

s64
get_file_size(wchar_t *fn);

BgFile
open_file(char *fn);

BgFile
open_file(wchar_t *fn);

BgFile
create_file(char *fn);

bool
delete_file(char *fn);

bool
delete_file(wchar_t *fn);

void
close_file(BgFile file);

FileRead
read_file_all(char *fn);

void 
free_file_read(FileRead *fr);

bool
read_filen(char *fn, void *data, u64 n);

bool
dump_file(char *fn, void *d, u64 n);

bool
copy_file_overwrite(char *file, char *dest);

bool
copy_file_overwrite(wchar_t *file, wchar_t *dest);


Array<char*>
get_filelist(char *dir);

void
free_filelist(Array<char*> *list);


u32
crc32(void *memory, u64 size);


/*
    BG STRING 
*/

// @@NOTE(Batuhan) those two (for now), needs external functions to operate, we 
// dont want to pull weight of compilation to everywhere 
wchar_t* 
multibyte_to_widestr(char *s);

char*
widestr_to_multibyte(wchar_t *ws);


static inline void
string_replace_every_ch(char *src, char to_be_searched, char to_be_replaced) {
    for (; *src != 0; src++) {
        if (*src == to_be_searched) {
            *src = to_be_replaced;
        }
    }
}

static inline void
string_replace_every_ch(wchar_t *src, wchar_t to_be_searched, wchar_t to_be_replaced) {
    for (; *src != 0; src++) {
        if (*src == to_be_searched) {
            *src = to_be_replaced;
        }
    }
}


static inline u64 
string_length(wchar_t *ws) {
    auto b = ws;
    for (; *ws != 0; ws++);
    return ws - b;
}

static inline u64 
string_length(char *s) {
    auto b = s;
    for (; *s != 0; s++);
    return s - b;
}



static inline wchar_t*
get_file_name_needle(wchar_t *fp, wchar_t *end = 0) {
    
    if (end != 0 && end < fp)
        return 0;
    
    u64 ts_i = 0;
    wchar_t* base = fp;
    BG_ASSERT(fp);
    
    if (end == 0) {
        end = (wchar_t*)((u64)-1);
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

static inline char*
get_file_name_needle(char *fp, char *end = 0) {
    
    if (end != 0 && end < fp)
        return 0;
    
    s64 ts_i = 0;
    char* base = fp;
    BG_ASSERT(fp);
    
    if (end == 0) {
        end = (char*)((u64)-1);
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

static inline void
string_copy(char *dest, char *src) {
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        dest++;
        src++;
    }
}

static inline void
string_copy(wchar_t *dest, wchar_t *src) {
    for (;;) {
        *dest = *src;
        if (*src == 0)
            break;
        dest++;
        src++;
    }
}

static inline wchar_t *
string_concanate(wchar_t *dest, u64 dest_max, wchar_t *lhs, u64 lhs_len, wchar_t *rhs, u64 rhs_len) {
    
    BG_ASSERT(dest);
    BG_ASSERT(lhs);
    BG_ASSERT(rhs);
    BG_ASSERT(lhs_len + rhs_len + 1 <= dest_max);

    if (dest) {
        if (lhs_len + rhs_len > dest_max) {
            LOG_ERROR("Unable to concanate %S and %S, buffer overrun!\n", lhs, rhs);
            return dest;
        }
        copy_memory(dest, lhs, lhs_len * 2);
        copy_memory(dest + lhs_len, rhs, rhs_len * 2);
        dest[lhs_len + rhs_len] = '\0';
    }

    return dest;
}

static inline char *
string_concanate(char *dest, u64 dest_max, char *lhs, u64 lhs_len, char *rhs, u64 rhs_len) {
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

static inline void
string_append(char *str, char *app, u64 strlen = 0, u64 applen = 0) {
    if (strlen == 0) strlen = string_length(str);
    if (applen == 0) applen = string_length(app);

    copy_memory(str + strlen, app, applen);

}

static inline bool
string_equal(char *s1, char *s2) {
    for (;;) {
        if (*s1 != *s2)       
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0) 
            return true;
    }
}

static inline bool
is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static inline char
to_lower(char c) {
    if (c >= 'A' && c<= 'Z')
         return ('Z' - c) + 'a';
     return c;    
}

static inline wchar_t
to_lower(wchar_t c) {
    return towlower(c);
}

static inline void
lower_string(char  *str) {
    for (; *str != 0; str++) {
        *str = to_lower(*str);
    }
}

static inline void
lower_string(wchar_t *str) {
    for (; *str != 0; str++) {
        *str = to_lower(*str);
    }
}

static inline bool
string_equal(wchar_t *w1, wchar_t *w2) {
    for (;;) {
        if (*w1 != *w2)       
            return false;
        w1++; w2++;
        if (*w1 == 0 && *w2 == 0) 
            return true;
    }
    return false;
}


static inline bool
string_equal_n(wchar_t *w1, wchar_t *w2, u64 n) {
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
string_equal_ignore_case_n(wchar_t *s1, wchar_t *s2, u64 n) {
    for (u64 i = 0; i<n; i++) {
        if (to_lower(*s1) != to_lower(*s2))       
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0) 
            return true;
    }   
    return true;
}

static inline wchar_t *
string_duplicate(wchar_t *ws) {
    static_assert(sizeof(wchar_t) == 2);
    
    u64 wlen = string_length(ws);
    wchar_t *result = (wchar_t*)bg_calloc(wlen + 1, sizeof(wchar_t));
    string_copy(result, ws);
    return result;
}

static inline char *
string_duplicate(char *str) {
    u64 len = string_length(str);
    char *result = (char *)bg_calloc(len + 1, 1);
    string_copy(result, str);
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
compare_extension(wchar_t *fp, wchar_t *ext) {
    
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


wchar_t* 
multibyte_to_widestr(char *s) {
    wchar_t *result = 0;
    
    int slen = (int)string_length(s) + 1;
    
#if defined(_WIN32)
    int chneeded = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)s, slen, 0, 0);
    BG_ASSERT(chneeded != 0);
    //chneeded += 1;
    
    result = (wchar_t*)bg_calloc(chneeded, 2);
    BG_ASSERT(result);
    
    int wr = MultiByteToWideChar(CP_UTF8, 0, (LPCCH)s, slen, result, chneeded);
    BG_ASSERT(wr == chneeded);
#else
#error not implemented
#endif
    
    return result;
}

char*
widestr_to_multibyte(wchar_t *ws) {
    char *result = 0;
    
    int wslen = (int)string_length(ws);
    
#if defined(_WIN32)
    int chneeded = WideCharToMultiByte(CP_UTF8, 0, ws, wslen, NULL, 0, NULL, NULL) ;
    
    result = (char*)bg_calloc((u64)chneeded, 1);
    
    int wr = WideCharToMultiByte(CP_UTF8, 0, ws, wslen, (LPSTR)result, chneeded, NULL, NULL);
    BG_ASSERT(wr <= chneeded);
#else
    #error not implemented
#endif
    
    return result;
}


s64
get_fp(BgFile file) {
#if BG_SYSTEM_WINDOWS
    LARGE_INTEGER start  ={};
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file, start, &result, FILE_CURRENT);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to get file pointer for file handle 0x%p", file);
    }
    
    return result.QuadPart;
#else
    #error not implemented
#endif
}

s64
set_fp(BgFile file, s64 offset) {
#if BG_SYSTEM_WINDOWS
    LARGE_INTEGER start ={};
    start.QuadPart = offset;
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file, start, &result, FILE_BEGIN);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to get file pointer for file handle 0x%p", file);
    }
    return result.QuadPart == offset;
#else
    #error not implemented
#endif
}

bool
write_file(BgFile file, void *data, u32 n) {
#if BG_SYSTEM_WINDOWS
    if (n == 0) {
        return true;
    }
    DWORD br = 0;
    if (!WriteFile(file, data, n, &br, 0) || n != br) {
        LOG_ERROR("Unable to write n(%u) bytes to file, instead written %lu, err %lu\n", n, br, GetLastError());
        return false;
    }
    return true;
#else
#error not implemented
#endif
}

bool
read_file(BgFile file, void *buffer, u32 n) {
#if BG_SYSTEM_WINDOWS
    DWORD br = 0;
    if (!ReadFile(file, buffer, n, &br, 0) || n != br) {
        LOG_ERROR("Unable to read n(%u) bytes from file, instead read %lu, err %lu\n", n, br, GetLastError());
        return false;
    }
    return true;
#else
#error not implemented
#endif
}

BgFile
open_file(char *fn) {
#if BG_SYSTEM_WINDOWS
    wchar_t *wfn = multibyte_to_widestr(fn);
    BgFile result = open_file(wfn);
    bg_free(wfn);
    return result;
#else
#error not implemented
#endif
}

BgFile
open_file(wchar_t *fn) {
#if BG_SYSTEM_WINDOWS
    HANDLE file = CreateFileW(fn, GENERIC_WRITE | GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Unable to open existing file %S, err %lu\n", fn, GetLastError());
    }
    return (void *)file;
#else
#error not implemented
#endif        
}

bool
delete_file(char *fn) {
#if BG_SYSTEM_WINDOWS
    BOOL winapi_result = DeleteFileA(fn);
    if (winapi_result != 0) {
        LOG_ERROR("Unable to delete file %s\n", fn);
    }
    return winapi_result != 0;
#else
    #error implement
#endif
}

bool
delete_file(wchar_t *fn) {
#if BG_SYSTEM_WINDOWS
    BOOL winapi_result = DeleteFileW(fn);
    if (winapi_result != 0) {
        LOG_ERROR("Unable to delete file %S\n", fn);
    }
    return winapi_result != 0;
#else
    #error implement
#endif
}

s64
get_file_size(char *fn) {
    auto wfn = multibyte_to_widestr(fn);
    s64 result = get_file_size(wfn);
    bg_free(wfn);
    return result;
}

s64
get_file_size(wchar_t *fn) {
#if BG_SYSTEM_WINDOWS
    s64 result = 0;
    HANDLE file = CreateFileW(fn, FILE_GENERIC_READ, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, 0, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, 0);
    BG_ASSERT(file != INVALID_HANDLE_VALUE);
    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fsli = {};
        BOOL winapiresult  = GetFileSizeEx(file, &fsli);
        BG_ASSERT(winapiresult);
        result = fsli.QuadPart;
    }
    else {
        LOG_ERROR("Unable to open file %S for file size query operation\n", fn);
    }

    CloseHandle(file);
    return result;
#else
    #error implement
#endif
}

BgFile
create_file(char *fn) {
#if BG_SYSTEM_WINDOWS
    HANDLE file = CreateFileA(fn, GENERIC_WRITE | GENERIC_READ, 0, 0, CREATE_NEW, 0, 0);
    if (file == INVALID_HANDLE_VALUE) {
        LOG_ERROR("Unable to create existing file %s, err %lu\n", fn, GetLastError());
    }
    return (void *)file;
#else
#error not implemented
#endif
}

void
close_file(BgFile file) {
#if BG_SYSTEM_WINDOWS
    CloseHandle((HANDLE)file);
#else
#error not implemented
#endif
}


FileRead
read_file_all(char* fn) {
#if BG_SYSTEM_WINDOWS
    
    FileRead result ={};
    HANDLE file = CreateFileA(fn, GENERIC_READ, 0, 0, OPEN_EXISTING, 0, 0);
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
#error not implemented
#endif
}

void 
free_file_read(FileRead *fr) {
    bg_free(fr->data);
    fr->len = 0;
    fr->data = 0;
}

bool
read_filen(char *fn, void *data, u64 n) {
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
#error not implemented
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
#error not implemented
#endif
}

 
bool
copy_file_overwrite(char *file, char *dest) {
#if BG_SYSTEM_WINDOWS
    return CopyFileA(file, dest, true);
#else
    #error implement!
#endif
}

bool
copy_file_overwrite(wchar_t *file, wchar_t *dest) {
#if BG_SYSTEM_WINDOWS
    return CopyFileW(file, dest, true);
#else
    #error implement!
#endif
}


Array<char*>
get_filelist(char *dir) {
#if defined(_WIN32)
    
    char wildcard_dir[280] = {};
    u64 dirlen = string_length(dir);
    dirlen++; // null termination
    
    BG_ASSERT(sizeof(wildcard_dir) > dirlen);
    
    Array<char*> result = {};
    arrreserve(&result, 20);
    
    string_append(wildcard_dir, "\\*");

    WIN32_FIND_DATAA FDATA;
    HANDLE FileIterator = FindFirstFileA(wildcard_dir, &FDATA);
    
    if (FileIterator != INVALID_HANDLE_VALUE) {
        
        while (FindNextFileA(FileIterator, &FDATA) != 0) {
            
            //@NOTE(Batuhan): Do not search for sub-directories, skip folders.
            if (FDATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }
            
            // @Stupid +4 just to be sure there is enough room for null termination
            u64 fn_len = string_length(FDATA.cFileName);
            fn_len += dirlen + 4;
            
            char *fnbuffer = (char*)bg_calloc(fn_len, 1);

            string_append(fnbuffer, "\\");
            string_append(fnbuffer, FDATA.cFileName);
            
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
    #error not implemented 
#endif
}

void
free_filelist(Array<char*> *list) {
    arrfree(list);
}



#endif