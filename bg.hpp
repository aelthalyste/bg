#ifndef BG_H_DECLERATIONS
#define BG_H_DECLERATIONS

#include <stdint.h>
#include <string.h>

#include <wctype.h> // for towloer
#include <stdlib.h> // malloc


// presetups
#if BG_RELEASE_SETUP && BG_DEBUG_SETUP
    #error You can't pass debug & release definitions at same time.
#endif

#if BG_RELEASE_SETUP
    #ifndef BG_ENABLE_LEAKCHECK
        #define BG_ENABLE_LEAKCHECK     0
    #endif

    #ifndef BG_DEVELOPER
        #define BG_DEVELOPER            0
    #endif

    #ifndef BG_DISABLE_DEBUGBREAK
        #define BG_DISABLE_DEBUGBREAK   1
    #endif

    #ifndef BG_FLUSH_LOGS_TO_STDOUT
        #define BG_FLUSH_LOGS_TO_STDOUT 0
    #endif
#endif

#if BG_DEBUG_SETUP
    #ifndef BG_ENABLE_LEAKCHECK
        #define BG_ENABLE_LEAKCHECK     1
    #endif

    #ifndef BG_DEVELOPER
        #define BG_DEVELOPER            1
    #endif

    #ifndef BG_DISABLE_DEBUGBREAK
        #define BG_DISABLE_DEBUGBREAK   0
    #endif

    #ifndef BG_FLUSH_LOGS_TO_STDOUT
        #define BG_FLUSH_LOGS_TO_STDOUT 1
    #endif
#endif



// turn this on if you want to merge bg.hpp with stb_leakcheck.hpp to follow allocations.
// this also converts Array<T> functions to macros to ease of reporting @leakcheck, so we know where each
// array expanded and freed or allocated, without worrying about callstack.
#ifndef BG_ENABLE_LEAKCHECK 
    #define BG_ENABLE_LEAKCHECK 0
#endif

// if this sets on, enables range checking on arrays, enables assertions, spits outs logs to stdout
#ifndef BG_DEVELOPER
    #define BG_DEVELOPER 0
#endif

// if this sets on, removes debugbreak on codebase, resulting asserts to just log. 
#ifndef BG_DISABLE_DEBUGBREAK
    #define BG_DISABLE_DEBUGBREAK 0
#endif

// if this sets on, sends logs to stdout alongside with the file.
#ifndef BG_FLUSH_LOGS_TO_STDOUT
    #define BG_FLUSH_LOGS_TO_STDOUT 1
#endif

#ifndef BG_LOG_PATH
    #define BG_LOG_PATH "bg_nar_log_file.txt"
#endif


#define _CRT_SECURE_NO_WARNINGS 1
#define BG_U32_MAX 0xffffffff

// platform specific includes


#if defined(_WIN32) || defined(_WIN64)
    #define BG_SYSTEM_WINDOWS 1
#elif defined(__APPLE__) && defined(__MACH__)
    #define BG_SYSTEM_OSX 1
#elif defined(__linux__)
    #define BG_SYSTEM_LINUX 1
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


#if !BG_DISABLE_DEBUGBREK
    #if BG_SYSTEM_WINDOWS
    	#define BG_DEBUGBREAK __debugbreak()
    #else 
    	#define BG_DEBUGBREAK __asm__ volatile("int $0x03")
    #endif
#else
    #define BG_DEBUGBREAK
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



#define bg_static_assert(exp) static_assert(exp, "error") // to remove c++17 warnings
#define bg_unused(x) ((void)(sizeof(x)))

#define Kilobyte(val) ((val)*1024ll)
#define Megabyte(val) (Kilobyte(val)*1024ll)
#define Gigabyte(val) (Megabyte(val)*1024ll)


// typedefs 


#if BG_SYSTEM_WINDOWS
    typedef uint64_t u64;
    typedef  int64_t s64;
    bg_static_assert(sizeof(s64) == sizeof(u64));

    typedef uint32_t u32;
    typedef  int32_t s32;
    bg_static_assert(sizeof(s32) == sizeof(u32));

    typedef uint16_t u16;
    typedef  int16_t s16;
    bg_static_assert(sizeof(s16) == sizeof(u16));

    typedef  uint8_t u8;
    typedef   int8_t s8;
    bg_static_assert(sizeof(s8) == sizeof(u8));

    #define BgUtf16 wchar_t
#endif

#if BG_SYSTEM_LINUX
    typedef uint64_t u64;
    typedef  int64_t s64;
    bg_static_assert(sizeof(s64) == sizeof(u64));

    typedef uint32_t u32;
    typedef  int32_t s32;
    bg_static_assert(sizeof(s32) == sizeof(u32));

    typedef uint16_t u16;
    typedef  int16_t s16;
    bg_static_assert(sizeof(s16) == sizeof(u16));

    typedef  uint8_t u8;
    typedef   int8_t s8;
    bg_static_assert(sizeof(s8) == sizeof(u8));

    typedef u16 BgUtf16;
#endif

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
    #define BG_API __attribute__((visibility("default")))
#endif


#define LOG(str, ...)          do{BG_INTERNAL_LOG("INFO"   , str, ## __VA_ARGS__);} while (0);
#define LOG_INFO(str, ...)     do{BG_INTERNAL_LOG("INFO"   , str, ## __VA_ARGS__);} while (0);
#define LOG_ERROR(str, ...)    do{BG_INTERNAL_LOG("ERROR"  , str, ## __VA_ARGS__);} while (0);
#define LOG_WARNING(str, ...)  do{BG_INTERNAL_LOG("WARNING", str, ## __VA_ARGS__);} while (0);    
#define LOG_DEBUG(str, ...)    do{BG_INTERNAL_LOG("DEBUG"  , str, ## __VA_ARGS__);} while (0);
#define BG_INTERNAL_LOG(prefix, str, ...) bg__log(prefix, str, ## __VA_ARGS__);



#if BG_DEVELOPER
    #if BG_COMPILER_MSVC
        //#define BG_ASSERT(exp) do{if (!(exp)) { LOG_ERROR("ASSERTION FAILED : \n\tFile : " __FILE__ "\n\tFunction : " __FUNCSIG__ "\n\tLine %d\n\tExpression : " #exp "\n", __LINE__); }} while(0);
        #define BG_ASSERT(exp) do{if (!(exp)) { LOG_ERROR("ASSERTION FAILED : \n\tFile : " __FILE__ "\n\tFunction : " __FUNCSIG__ "\n\tLine %d\n\tExpression : " #exp "\n", __LINE__); BG_DEBUGBREAK; }} while(0);
    #else
        #define BG_ASSERT(exp) do{if (!(exp)) { LOG_ERROR("ASSERTION FAILED : \n\tFile : " __FILE__ "\n\tLine %d", __LINE__);}} while(0);
    #endif
#else
    #define BG_ASSERT(exp)  
#endif


void bg_init_log_file(const char *file_name);
void bg__log(const char *prefix, const char *fmt, ...);




#if BG_ENABLE_LEAKCHECK
    #include "stb_leakcheck.h"
#endif

#define bg_malloc(n)        (n ? malloc((u64)(n)) : NULL)
#define bg_realloc(p, sz)   realloc((p), (u64)(sz))
#define bg_free(p)          (free((p)), (p)=NULL)
#define bg_calloc(c,s)      (memset(bg_malloc((u64)(c) * (u64)(s)), 0, (u64)(c) * (u64)(s)))  // calloc((u64)c, (u64)s)


#define BG_MIN(a, b) ((a) > (b) ? (b) : (a))
#define BG_MAX(a, b) ((a) < (b) ? (b) : (a))


#define memory_equal(p1, p2, ps) (!!(memcmp(p1, p2, ps) == 0))
#define zero_memory(p, size)      memset(p, 0, size)
#define copy_memory(p1, p2, ps)   memcpy(p1, p2, ps);




#if !defined(BG_SYSTEM_WINDOWS)
    #define BG_SYSTEM_WINDOWS 0
#endif
#if !defined(BG_SYSTEM_LINUX)
    #define BG_SYSTEM_LINUX   0
#endif
#if !defined(BG_SYSTEM_OSX)
    #define BG_SYSTEM_OSX     0
#endif

#if !defined(BG_COMPILER_MSVC)
    #define BG_COMPILER_MSVC  0
#endif
#if !defined(BG_COMPILER_CLANG)
    #define BG_COMPILER_CLANG 0
#endif
#if !defined(BG_COMPILER_GCC)
    #define BG_COMPILER_GCC   0
#endif

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

struct Allocator_Mark {
    u64 internal_mark;
};

struct Linear_Allocator {
    void *memory = NULL;
	u64 size = 0;
	u64 used = 0;
	u64 aligment = 16;

	void * allocate_aligned(u64 s, u64 al) {
		
		u64 al_bonus = (u64)((u8 *)memory + used) % al;
		if (al_bonus)
			al_bonus = al - al_bonus;
		

		void *result = NULL;
		if (s + al_bonus + used < size) {
			result = (u8 *)memory + used + al_bonus;
			used += al_bonus + s;
		}

		return result;
		
	}

	void * allocate(u64 s) {
		return allocate_aligned(s, aligment);	
	}

    void * allocate_zero_aligned(u64 s, u64 al) {
        void *result = allocate_aligned(s, al);
        zero_memory(result, s);
        return result;
    }

    void * allocate_zero(u64 s) {
        void *result = allocate(s);
        zero_memory(result, s);
        return result;         
    }

    Allocator_Mark mark() {
        Allocator_Mark result;
        result.internal_mark = used;
        return result;
    }

    void restore(Allocator_Mark mark) {
        used = mark.internal_mark;
    }

};

static inline Linear_Allocator
init_linear_allocator(void *memory, u64 size, u64 aligment) {
	Linear_Allocator result;
	result.memory = memory;
	result.size   = size;
	result.used   = 0;
	result.aligment = aligment;
	return result;
}


// BG DATE
struct Bg_Date {
    u16 year;
    u16 month;
    u16 day;
    u16 hour;
    u16 minute;
    u16 second;
    u16 millisecond;
};

int64_t bg_get_performance_counter();
double bg_calculate_elapsed_time_ms(int64_t start, int64_t end);

Bg_Date get_local_date();

Bg_Date get_utc_date();



#if BG_COMPILER_MSVC
    #pragma warning(push)
    #pragma warning(disable : 4626)
#endif



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

#if BG_COMPILER_MSVC
    #pragma warning(pop)
#endif

#define DEFER_1(x, y) x##y
#define DEFER_2(x, y) DEFER_1(x, y)
#define DEFER_3(x)    DEFER_2(x, __COUNTER__)
#define defer(code)   auto DEFER_3(_defer_) = defer_func([&]() {code;})

#define for_array(_index, array) for (u64 _index = 0; _index < (array).len; (_index)++)
#define for_array_it(it, arr) for (auto it = &((arr).data[0]); it != (&((arr).data[0]) + (arr).len); ++it)
#define for_n(_index, _ceil) for (u64 _index = 0; _index < _ceil; ++(_index)) 

template<typename T>
struct Array {
    T *data = NULL;
    u64 len = 0;
    u64 cap = 0;
    inline T &operator[](u64 i) {

#if BG_ARR_BOUNDS_CHECK
        BG_ASSERT(i < this->len);
#endif

        return this->data[i];
    }
};




#if BG_ENABLE_LEAKCHECK

#define arr__grow(arr, new_cap) do { \
    if ((arr)->cap >= (new_cap)) \
        break; \
    u64 min_cap = 0; \
    if ((arr)->cap <= 8) \
        min_cap = 8; \
    if (min_cap < (new_cap)) { \
        if (min_cap * 2 + 4 < (new_cap)) \
            min_cap = (new_cap); \
        else \
            min_cap = min_cap * 2 + 4; \
    } \
    (arr)->data = (decltype((arr)->data))bg_realloc((arr)->data, min_cap * sizeof((arr)->data[0])); \
    (arr)->cap  = min_cap; \
    zero_memory((arr)->data + (arr)->len, ((arr)->cap - (arr)->len) * sizeof((arr)->data[0])); \
} while(0)


#define arrinit(arr, cap) do { BG_ASSERT((arr)->len == 0); BG_ASSERT((arr)->data == NULL); (arr)->data = NULL; arr__grow((arr), cap); } while(0)

#define arrput(arr, val) do { if ((arr)->len == (arr)->cap) { arr__grow((arr), (arr)->cap + 1); } (arr)->data[(arr)->len] = val; (arr)->len++; } while(0)

#define arrputn(arr, values, n) do { \
    if ((arr)->len + (n) >= (arr)->cap) { \
        arr__grow((arr), (arr)->len + (n)); \
    } \
    BG_ASSERT((arr)->len + (n) <= (arr)->cap); \
    copy_memory((arr)->data + (arr)->len, values, (n) * sizeof((arr)->data[0])); \
    (arr)->len += (n); \
} while(0)

template<typename T>
T
arrpop(Array<T> *arr) {
    if (arr->len > 0) {
        return arr->data[--arr->len];
    }
    return {};
}

#define arrfree(arr) do { \
    bg_free((arr)->data); \
    (arr)->data = 0; \
    (arr)->len = 0; \
    (arr)->cap = 0; \
} while(0)

#define arrreserve(arr, n) do { \
    arr__grow(arr, n); \
} while(0) 


#define arrputnempty(arr, count) do { \
    if ((arr)->len + (count) >= (arr)->cap) { arr__grow((arr), (arr)->len + (count)); } \
    (arr)->len += (count); \
    zero_memory(&((arr)->data[(arr)->len-(count)]), (count) * sizeof((arr)->data[0])); \
} while(0)


    

template<typename T>
T *
arrputptr(Array<T> *arr) {
    T *result = NULL;
    arrputnempty(arr, 1);
    result = &arr->data[arr->len - 1];
    return result;
}

template<typename T>
void
arrdel(Array<T> *arr, u64 indc) {
    BG_ASSERT(indc < arr->len);
    BG_ASSERT(indc < arr->cap);
    
    if (arr->len == 0)
        return;
    
    if (indc == arr->len - 1) {
        --arr->len;
        return;
    }

    if (indc < arr->len) {
        memmove(arr->data + indc, arr->data + indc + 1, (arr->len - indc) * sizeof(T));
    }
    --arr->len;
}

template<typename T>
void
arrins(Array<T> *arr, u64 indc, T val) {
    BG_ASSERT(indc < arr->len);

    arr__grow(arr, arr->len+1);

    memmove(arr->data + indc + 1, arr->data + indc, (arr->len - indc) * sizeof(T));
    
    *(arr->data+indc) = val;

    arr->len++;
}

#else // BG_ENABLE_LEAKCHECK

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
T *
arrputptr(Array<T> *arr);


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
    if (arr->cap >= new_cap)
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

    arr->data = (T *)bg_realloc(arr->data, min_cap * sizeof(arr->data[0]));
    arr->cap  = min_cap;

    zero_memory(arr->data + arr->len, (arr->cap - arr->len) * sizeof(arr->data[0]));
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
        return arr->data[--arr->len];
    }
    return {};
}

template<typename T>
void
arrfree(Array<T> *arr) {
    bg_free(arr->data);
    arr->data = 0;
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
    if (arr->len + count >= arr->cap) // TODO : check this equality
        arr__grow(arr, arr->len + count);

    T *result = &arr->data[arr->len];
    arr->len += count;
    zero_memory(result, count * sizeof(arr->data[0]));
    return result;
}

template<typename T>
T *
arrputptr(Array<T> *arr) {
    T *result = NULL;
    arrputnempty(arr, 1);
    result = &arr->data[arr->len - 1];
    return result;
}

template<typename T>
void
arrdel(Array<T> *arr, u64 indc) {
    BG_ASSERT(indc < arr->len);
    BG_ASSERT(indc < arr->cap);
    
    if (arr->len == 0)
        return;
    
    if (indc == arr->len - 1) {
        --arr->len;
        return;
    }

    if (indc < arr->len) {
        memmove(arr->data + indc, arr->data + indc + 1, (arr->len - indc) * sizeof(T));
    }
    --arr->len;
}

template<typename T>
void
arrins(Array<T> *arr, u64 indc, T val) {
    BG_ASSERT(indc < arr->len);

    arr__grow(arr, arr->len+1);

    memmove(arr->data + indc + 1, arr->data + indc, (arr->len - indc) * sizeof(T));
    
    *(arr->data+indc) = val;

    arr->len++;
}

// end of template versions of array functions.
#endif 


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






//
// THREADING , MUTEX, LOCKS ETC 
//

struct Mutex {
    // sizeof(CRITICAL_SECTION) = sizeof(pthread_mutex_t) = 40 bytes
    uint8_t _internal[40];
//#if BG_SYSTEM_WINDOWS
//    CRITICAL_SECTION critical_section;
//#else
//    pthread_mutex_t  m;
//#endif
};

Mutex
init_mutex();

void
free_mutex(Mutex *m);

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
    u64  len;
};

enum File_Access_Flags {
    File_Access_Flags_Read_Only,
    File_Access_Flags_Read_And_Write
};

enum File_Share_Flags {
    File_Share_Flags_Share_Read,
    File_Share_Flags_Share_Write,
    File_Share_Flags_Share_None
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
    void*     handle;
#else
	int fd;
#endif
	s64 cached_fp = 0;
};


struct Async_IO_Handle {
#if BG_SYSTEM_WINDOWS
    u8 _internal[32];
    //OVERLAPPED overlapped;
#else
	int place_holder;
#endif
};


struct File_View {
    void *data;
    u64  size;
};

File_View
open_file_view(const char *fn);

File_View
open_file_view(const BgUtf16 *fn);

void
close_file_view(File_View *view);


bool 
is_file_handle_valid(File *file);

// open-create
File
open_file__raw(const char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share);

File
open_file__raw(const char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share);

File
open_file(const char *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);

File
create_file(const char *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);


File_View
open_file_view(const char *fn);


#if BG_SYSTEM_WINDOWS
File
open_file(const BgUtf16 *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);

File
create_file(const BgUtf16 *fn, File_Access_Flags access = File_Access_Flags_Read_And_Write, File_Share_Flags share = File_Share_Flags_Share_Read);
#endif



s64
get_fp(File *file);

bool
set_fp(File *file, s64 offset);


IO_Result
check_file_async_io(File *file, Async_IO_Handle *async);

bool
wait_io_completion(File *file, Async_IO_Handle *async_ctx);


// write
bool
write_file(File *file, const void *data, u64 n);

Async_IO_Handle
write_file_async(File *file, const void *data, u64 n);

IO_Result
write__file(File *file, const void *data, u64 n, s64 target_offset, Async_IO_Handle *async_handle);


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

u64
get_file_size(File *file);


bool 
delete_directory(const char *dir);

bool
delete_file(const char *fn);

bool
delete_file(const BgUtf16 *fn);

void
close_file(File *file);

File_Read
read_file_all(const char *fn);

#if BG_SYSTEM_WINDOWS
File_Read
read_file_all(const wchar_t *fn);
#endif

void
free_file_read(File_Read *fr);

bool 
read_filen_last(const char *fn, void *data, u64 n);

bool
read_filen(char *fn, void *data, u64 n);

bool
dump_file(char *fn, const void *d, u64 n);

#if BG_SYSTEM_WINDOWS
bool
copy_file_overwrite(char *file, char *dest);

bool
copy_file_overwrite(BgUtf16 *file, BgUtf16 *dest);
#endif

#if BG_SYSTEM_WINDOWS
Array<BgUtf16 *>
get_file_paths_in_directory(const BgUtf16 *dir);

void
free_filelist(Array<BgUtf16 *> & list);

#endif

Array<char *>
get_file_paths_in_directory(const char *dir);

void
free_filelist(Array<char *> & list);

int bg_check_for_memory_leaks();





u32
bg_crc32(const void *memory, u64 size);


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

#if BG_SYSTEM_WINDOWS
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
#endif


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
path_file_name(const BgUtf16 *fp, BgUtf16 *end = 0) {

    if (end != 0 && end < fp)
        return 0;

    u64 ts_i = 0;
    const BgUtf16 *base = fp;
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
                return (BgUtf16 *)(++fp);
            }
        }
    }


    return (BgUtf16 *)&base[ts_i ? ++ts_i : 0];
}

static inline char *
path_file_name(const char *fp, char *end = 0) {

    if (end != 0 && end < fp)
        return 0;
    //[a-z] \*[a-z]
    s64 ts_i = 0;
    const char *base = fp;
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
                return (char *)(++fp);
            }
        }
    }


    return (char *)&base[ts_i ? ++ts_i : 0];
}

static inline char *
path_extract_directory(const char *p, char *out, u64 out_size) {
    char *e = path_file_name(p);
    BG_ASSERT(out_size > (u64)(e - p));
    if (out_size > (u64)(e - p)) {
        copy_memory(out, p, (u64)(e - p));
        out[e-p] = '\0';
    }
    else {
        out[0] = '\0';
    }
    return out;
}

static inline BgUtf16 *
path_extract_directory(const BgUtf16 *p, BgUtf16 *out, u64 out_size) {
    BgUtf16 *e = path_file_name(p);
    BG_ASSERT(out_size > (u64)(e-p) * 2);
    if (out_size > (u64)(e-p) * 2) {
        copy_memory(out, p, (u64)(e-p) * 2);
        out[e-p] = '\0';
    }
    else {
        out[0] = '\0';
    }
    return out;
} 

// copies at max n bytes, ALWAYS terminates with null!
static inline void
string_copy_n(char *dest, const char *src, u64 n) {
    if (n==0||dest==NULL||src==NULL)
        return;

    char *end=dest+n-1;
    for(;n!=0;) {
        *dest=*src;
        if (*src==0)
            break;
        ++dest;
        ++src;
        --n;
    }
    *end=0;
}

static inline u64
string_copy(char *dest, const char *src) {
    if(!dest||!src) return 0;
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
    if(!dest||!src) return 0;
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


static inline char *
string_find_substring(const char *src, const char *substr) {
    
    for (const char *n = src; *n != 0; ++n) {
        if (*n == *substr) {
            const char *tn = n;
            const char *ts = substr;
            for (; *ts != 0 && *tn != 0; ++tn, ++ts) {
                if (*ts != *tn) {
                    break;
                }
            }

            if (*ts == 0) {
                return (char *)n;
            }
        }
    }

    return NULL;
}


static inline BgUtf16 *
string_concanate(BgUtf16 *dest, u64 dest_max, BgUtf16 *lhs, u64 lhs_len, BgUtf16 *rhs, u64 rhs_len) {

    BG_ASSERT(dest);
    BG_ASSERT(lhs);
    BG_ASSERT(rhs);
    BG_ASSERT(lhs_len + rhs_len + 1 <= dest_max);

    if (dest) {
        if (lhs_len + rhs_len > dest_max) {
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
    str[strlen + applen] = 0;
    return strlen + applen;
}

static inline u64
string_append(BgUtf16 *str, const BgUtf16 *app, u64 strlen = 0, u64 applen = 0) {
    if (strlen == 0) strlen = string_length(str);
    if (applen == 0) applen = string_length(app);

    copy_memory(str + strlen, app, applen * 2);
    str[strlen + applen] = 0;
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
    
    if(!str) return 0;

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
    
    if (!str) return 0;

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
        return (c - 'A') + 'a';
    return c;
}


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

static inline char *
string_rfind_char(char *str, char n) {
    if (!str)
        return NULL;

    u64 sl = string_length(str);
    
    for (s64 i = sl - 1; i >= 0; --i) {
        if (str[i] == n)
            return &str[i];
    }

    return NULL;
}


static inline bool
string_equal_n(const char *s1, const char *s2, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (*s1 != *s2)
            return false;
        ++s1; ++s2;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
    return true;	
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
string_equal_ignore_case_n(const BgUtf16 *s1, const BgUtf16 *s2, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (to_lower(*s1) != to_lower(*s2))
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
    return true;
}

static inline bool
string_equal_ignore_case_n(const char *s1, const char *s2, u64 n) {
    for (u64 i = 0; i < n; i++) {
        if (to_lower(*s1) != to_lower(*s2))
            return false;
        s1++; s2++;
        if (*s1 == 0 && *s2 == 0)
            return true;        
    }
    return true;
}

static inline bool
string_equal_ignore_case(const char *s1, const char *s2) {
    for (;;) {
        if (to_lower(*s1) != to_lower(*s2))
            return false;

        s1++; s2++;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
}

static inline bool
string_equal_ignore_case(const BgUtf16 *s1, const BgUtf16 *s2) {
    for (;;) {
        if (to_lower(*s1) != to_lower(*s2)) 
            return false;

        ++s1; ++s2;
        if (*s1 == 0 && *s2 == 0)
            return true;
    }
} 

#if BG_ENABLE_LEAKCHECK
#include <type_traits>
#define string_duplicate(ws) (std::decay<decltype(ws)>::type)copy_memory(bg_calloc(string_length(ws) + 1, sizeof((ws)[0])), ws, string_length(ws)*sizeof((ws)[0]))
#else
static inline BgUtf16 *
string_duplicate(const BgUtf16 *ws) {
    if (ws == NULL)
        return NULL;
    
    bg_static_assert(sizeof(BgUtf16) == 2);

    u64 wlen = string_length(ws);
    BgUtf16 *result = (BgUtf16 *)bg_calloc(wlen + 1, sizeof(BgUtf16));
    copy_memory(result, ws, wlen * 2);
    return result;
}

static inline char *
string_duplicate(const char *str) {
    if (str == NULL)
        return NULL;

    u64 len = string_length(str);
    char *result = (char *)bg_calloc(len + 1, 1);
    copy_memory(result, str, len);
    return result;
}
#endif

static inline bool
compare_extension(char *fp, char *ext) {
    if (fp == NULL || ext == NULL)
        return false;

    auto fn = path_file_name(fp);

    u64 slen   = string_length(fn);
    u64 extlen = string_length(ext);

    if (extlen > slen) {
        return false;
    }

    return (memory_equal(fn + slen - extlen, ext, extlen));
}

static inline bool
compare_extension_ignore_case(const char *fp, const char *ext) {
    if (fp == NULL || ext == NULL) 
        return false;
    
    auto fn = path_file_name((char *)fp);
    u64 slen   = string_length(fn);
    u64 extlen = string_length(ext);
    if (extlen > slen) 
        return false;

    return string_equal_ignore_case_n(fn + slen - extlen, ext, extlen);
}

static inline bool
compare_extension_ignore_case(const BgUtf16 *fp, const BgUtf16 *ext) {
    if (fp == NULL || ext == NULL)
        return false;

    auto fn = path_file_name((BgUtf16 *)fp);
    u64 slen   = string_length(fn);
    u64 extlen = string_length(ext);
    if (extlen > slen) 
        return false;

    return string_equal_ignore_case_n(fn + slen - extlen, ext, extlen);    
}

static inline bool
compare_extension(const BgUtf16 *fp, const BgUtf16 *ext) {
    if (fp == NULL || ext == NULL)
        return false;

    auto fn = path_file_name((BgUtf16 *)fp);

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

//BG_H_DECLERATIONS
#endif 



#ifdef BG_IMPLEMENTATION

//
// PLATFORM INCLUDES
//

#if BG_SYSTEM_WINDOWS

// shrink down windows.h
    #define WIN32_LEAN_AND_MEAN
    #define NOGDICAPMASKS
    #define NOVIRTUALKEYCODES
    #define NOWINMESSAGES
    #define NOWINSTYLES
    #define NOSYSMETRICS
    #define NOMENUS
    #define NOICONS
    #define NOKEYSTATES
    #define NOSYSCOMMANDS
    #define NORASTEROPS
    #define NOSHOWWINDOW
    #define OEMRESOURCE
    #define NOATOM
    #define NOCLIPBOARD
    #define NOCOLOR
    #define NOCTLMGR
    #define NODRAWTEXT
    #define NOGDI
    #define NOKERNEL
    #define NOUSER
    //#define NONLS -> string conversion stuff
    #define NOMB
    #define NOMEMMGR
    #define NOMETAFILE
    #define NOMINMAX
    #define NOMSG
    #define NOOPENFILE
    #define NOSCROLL
    #define NOSERVICE
    #define NOSOUND
    #define NOTEXTMETRIC
    #define NOWH
    #define NOWINOFFSETS
    #define NOCOMM
    #define NOKANJI
    #define NOHELP
    #define NOPROFILER
    #define NODEFERWINDOWPOS
    #define NOMCX

    #include <windows.h>
    #include <debugapi.h>
    
    // assertions about implementations
    bg_static_assert(sizeof(Async_IO_Handle) == sizeof(OVERLAPPED));
    bg_static_assert(sizeof(Async_IO_Handle) == 32);
    bg_static_assert(sizeof(Mutex) == sizeof(CRITICAL_SECTION));
    bg_static_assert(sizeof(Mutex) == 40);
#endif

#if BG_SYSTEM_LINUX
    #include <pthread.h>
    #define _LARGEFILE64_SOURCE 1    /* See feature_test_macros(7) */
    #include <sys/types.h>
    #include <errno.h>
    #include <sys/stat.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <pthread.h>

    // assertions about implementations
    bg_static_assert(sizeof(Mutex) == sizeof(pthread_mutex_t));
    bg_static_assert(sizeof(Mutex) == 40);
#endif



// STD INCLUDES
#include <time.h>
#include <stdio.h>



static FILE *bg__log__internal_file = NULL;
static Mutex bg__log__internal_mutex;
static bool  bg__log__internal__mutex_initialized = false;

#include<stdio.h>
#include<stdarg.h>

void 
bg_init_log_file(const char *file_name) {
	if (bg__log__internal_file == NULL) {
    	bg__log__internal_file = fopen(file_name, "ab");
    	if (!bg__log__internal__mutex_initialized) {
    		bg__log__internal_mutex = init_mutex();
    		bg__log__internal__mutex_initialized = true;
    	}
	}
}

void
bg__log(const char *log_prefix, const char *fmt, ...) {
    

	if (bg__log__internal_file == NULL)
		bg_init_log_file(BG_LOG_PATH);

    
    lock_mutex(&bg__log__internal_mutex);
    {
        static char buf[32 * 1024];
        zero_memory(buf, sizeof(buf));
        
        Bg_Date cd = get_local_date();
        // prefix size
        s32 ps = 0;
        // total buffer written size
        s32 bs = 0;

        // [dd:mm:yyyy - hh:mm:ss:msms]
        ps += snprintf(buf, sizeof(buf), "[%02d/%02d/%04d - %02d:%02d:%02d:%04d] | ", cd.day, cd.month, cd.year, cd.hour, cd.minute, cd.second, cd.millisecond);

        // ERROR
        // WARNING
        // INFO
        ps += snprintf(buf + ps, sizeof(buf) - ps, "(%-10s) : ", log_prefix); 

        // actual user input
        {
            va_list args;
            va_start(args, fmt);
            bs += vsnprintf(buf + ps, sizeof(buf) - ps, fmt, args);
            va_end(args);
        }
        
        bs += ps;


        // append newline if user didnt provide.
        if (buf[bs - 1] != '\n')
            buf[bs++] = '\n';


        fwrite(buf, bs, 1, bg__log__internal_file);

        
#if BG_DEVELOPER || BG_FLUSH_LOGS_TO_STDOUT
        fprintf(stdout, "%s", buf);
#endif


    }
    unlock_mutex(&bg__log__internal_mutex);
    
    fflush(bg__log__internal_file);

}


#if BG_SYSTEM_WINDOWS

Bg_Date
systemtime_to_bg_date(SYSTEMTIME *w_time) {
    Bg_Date result;
    result.year        = w_time->wYear;
    result.month       = w_time->wMonth;
    result.day         = w_time->wDay;
    result.hour        = w_time->wHour;
    result.minute      = w_time->wMinute;
    result.second      = w_time->wSecond;
    result.millisecond = w_time->wMilliseconds;
    return result;
}

Bg_Date
get_local_date() {
    SYSTEMTIME w_time;
    GetLocalTime(&w_time);
    return systemtime_to_bg_date(&w_time);
}

Bg_Date 
get_utc_date() {
    SYSTEMTIME w_time;
    GetSystemTime(&w_time);
    return systemtime_to_bg_date(&w_time);
}


#else


Bg_Date
tm_time_to_bg_date(struct tm * tm) {
    Bg_Date result;
    result.year   = tm->tm_year + 1900;
    result.month  = tm->tm_mon;
    result.day    = tm->tm_mday;
    result.hour   = tm->tm_hour;
    result.minute = tm->tm_min;
    result.second = tm->tm_sec;
    result.millisecond = 0; //
    return result;
}

Bg_Date
get_utc_date() {
    time_t t = time(NULL);
    struct tm tm;
    gmtime_r(&t, &tm);
    return tm_time_to_bg_date(&tm);
}

Bg_Date
get_local_date() {
    time_t t = time(NULL);
    struct tm tm;
    localtime_r(&t, &tm);
    return tm_time_to_bg_date(&tm); 
}

#endif


int64_t bg_get_performance_counter() {
#if BG_SYSTEM_WINDOWS
    LARGE_INTEGER li;
    QueryPerformanceCounter(&li);
    return li.QuadPart;
#endif
#if BG_SYSTEM_LINUX
    int64_t result = 0;
    struct timespec r;
    clock_gettime(CLOCK_MONOTONIC, &r);
    result += r.tv_sec * 1000000; // convert to us
    result += r.tv_nsec / 1000; // we are losing some precision by doing this divide, but whatever its fine.
    return result;
#endif
}

double bg_calculate_elapsed_time_ms(int64_t start, int64_t end) {
#if BG_SYSTEM_WINDOWS
    int64_t freq=0;
    if (freq==0) {
        LARGE_INTEGER freq_li;
        QueryPerformanceFrequency(&freq_li);
        freq = freq_li.QuadPart;
    }
    double result = (end-start)/(double)freq;
    result *= 1000.0;
    return result;
#endif
#if BG_SYSTEM_LINUX
    return ((double)end - (double)start)/1000.0;
#endif
}


Mutex
init_mutex() {
#if BG_SYSTEM_WINDOWS
    Mutex result = {};
    InitializeCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(&result));
    return result;
#else
    Mutex result = {};
    pthread_mutex_init(reinterpret_cast<pthread_mutex_t *>(&result), NULL);
    return result; 
#endif
}

void
free_mutex(Mutex *m) {
#if BG_SYSTEM_WINDOWS
    DeleteCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(m));
#else
    pthread_mutex_destroy(reinterpret_cast<pthread_mutex_t *>(m));
#endif
}

void
lock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    EnterCriticalSection(reinterpret_cast<CRITICAL_SECTION *>(mutex));
#else
    pthread_mutex_lock(reinterpret_cast<pthread_mutex_t *>(mutex));
#endif
}

void
unlock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    LeaveCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(mutex));
#else
    pthread_mutex_unlock(reinterpret_cast<pthread_mutex_t *>(mutex));
#endif
}

bool
try_lock_mutex(Mutex *mutex) {
#if BG_SYSTEM_WINDOWS
    return TryEnterCriticalSection(reinterpret_cast<CRITICAL_SECTION*>(mutex));
#else
    return 0 == pthread_mutex_trylock(reinterpret_cast<pthread_mutex_t *>(mutex));
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
bg_crc32(const void *data, u64 len) {
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
    int chneeded = WideCharToMultiByte(CP_UTF8, 0, (BgUtf16*)str, (s32)slen, NULL, NULL, NULL, NULL);
    BG_ASSERT(chneeded >= 0);
    return (u64)chneeded;
}
#endif

#if BG_SYSTEM_WINDOWS
BgUtf16 *
multibyte_to_widestr(const char *s) {
    BgUtf16 *result = 0;

    int slen = (int)string_length(s) + 1;

    result = (BgUtf16 *)bg_calloc(slen + 2, 2);
    mbstowcs((BgUtf16 *)result, s, slen + 2);

    return result;
}

char *
widestr_to_multibyte(const BgUtf16 *ws) {
    char *result = 0;

    int wslen = (int)string_length(ws);

    result = (char *)bg_calloc(wslen + 2, 1);
    wcstombs(result, (BgUtf16 *)ws, wslen + 2);

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
    size_t rv = -1;
    // We shouldn't be using this in linux anyway
    LOG_ERROR("utf32 to utf8 conversion not supported on linux(yet)\n");
    BG_ASSERT(false);    
    // wcstombs(result, (BgUtf16 *)ws, max_size);
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
    size_t rv = -1;
    // We shouldn't be using this in linux anyway
    LOG_ERROR("utf32 to utf8 conversion not supported on linux(yet)\n");
    BG_ASSERT(false);
    // mbstowcs((BgUtf16 *)result, s, max_size/2);
    return rv == (size_t)-1 ? NULL : result;
#endif
}
#endif

s64
get_fp(File *file) {

    if (!is_file_handle_valid(file))
        return false;

#if BG_SYSTEM_WINDOWS
    LARGE_INTEGER start  ={};
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file->handle, start, &result, FILE_CURRENT);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to get file pointer\n");
    }
    return result.QuadPart;
#else
    s64 result = lseek64(file->fd, 0, SEEK_CUR);
    return result;
#endif
}

bool
set_fp(File *file, s64 offset) {
    
    if (!is_file_handle_valid(file))
        return false;

#if BG_SYSTEM_WINDOWS
    file->cached_fp = offset;
    LARGE_INTEGER start ={};
    start.QuadPart = offset;
    LARGE_INTEGER result ={};
    BOOL spresult = SetFilePointerEx(file->handle, start, &result, FILE_BEGIN);
    BG_ASSERT(spresult);
    if (!spresult) {
        LOG_ERROR("Unable to set file pointer\n");
    }
    return result.QuadPart == offset;
#else
    s64 result = lseek64(file->fd, offset, SEEK_SET);
    return result == offset;
#endif
}

bool
write_file(File *file, const void *data, u64 n) {

    Async_IO_Handle handle = {};
    s64 current_fp = get_fp(file);

    auto io_result = write__file(file, data, n, current_fp, &handle);

    if (io_result == IO_Result_Pending) {
        return wait_io_completion(file, &handle);
    }

    return io_result == IO_Result_Done;
}

Async_IO_Handle
write_file_async(File *file, const void *data, u64 n, u64 write_offset) {
    Async_IO_Handle handle = {};
    write__file(file, data, n, write_offset, &handle);
    return handle;
}

IO_Result
write__file(File *file, const void *data, u64 n, s64 write_offset, Async_IO_Handle *async_handle) {
    bg_unused(async_handle);
#if BG_SYSTEM_WINDOWS
    DWORD br = 0;
    set_fp(file, write_offset);
    BOOL wfr = WriteFile(file->handle, data, (DWORD)n, &br, NULL);
    if (wfr && br == n) {
        return IO_Result_Done;
    }
    else {
        LOG_ERROR("WriteFile failed, last error code %d, tried to write %lld bytes, instead, written %d", GetLastError(), n, br);
        return IO_Result_Error;
    }
#if 0
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
#endif

#else
    LOG_WARNING("Linux async io is not implemented(yet!)\n"); 
    ssize_t lsr = lseek64(file->fd, write_offset, SEEK_SET);

    if (lsr != write_offset) {
        LOG_ERROR("Unable to seek file pointer to %lld, result is %lld, aborting write operation\n", write_offset, lsr);
        return IO_Result_Error;
    }

    BG_ASSERT(lsr == write_offset);

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
    auto winapi_result = GetOverlappedResultEx(file->handle, reinterpret_cast<OVERLAPPED*>(async_ctx), &bytes_transferred, INFINITE, FALSE);
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
    bg_unused(async_ctx);
    bg_unused(file);
    return true;
#endif
}

// returns true if io completed, for big io stuff it usually returns false.
IO_Result
read__file(File *file, void *buffer, u64 n, s64 read_offset, Async_IO_Handle *async_handle) {
    if (!is_file_handle_valid(file))
        return IO_Result_Error;
    bg_unused(async_handle);
#if BG_SYSTEM_WINDOWS
    bg_unused(async_handle);
    DWORD br = 0;
    set_fp(file, read_offset);
    ReadFile(file->handle, buffer, (DWORD)n, &br, NULL);
    return br == n ? IO_Result_Done : IO_Result_Error;
#if 0
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
#endif
#else
    s64 sfpr = set_fp(file, read_offset); 
    bg_unused(sfpr);
    LOG_WARNING("Async file read for linux not yet implemented\n");
    ssize_t rs = read(file->fd, buffer, n);
    if (rs != (s64)n) {
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
	auto winapi_result = GetOverlappedResultEx(file->handle, reinterpret_cast<OVERLAPPED *>(async_handle), &bytes_transferred, 0, FALSE);
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
    bg_unused(async_handle);
    bg_unused(file);
    return IO_Result_Done;
#endif
}

#if BG_SYSTEM_WINDOWS
File
open_file__raw(const BgUtf16 *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {

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
    result.handle = CreateFileW(fn, winapi_access, winapi_share, 0, winapi_open, 0, 0);
    if (result.handle == INVALID_HANDLE_VALUE) {
        auto errcode = GetLastError();
        LOG_INFO("Unable to open file %S, errcode %ld, access : %ld, share : %ld, open : %ld\n", fn, errcode, winapi_access, winapi_share, winapi_open);
    }

    return result;
}


File
open_file__raw(const char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {
    BgUtf16 *wfn = multibyte_to_widestr(fn);
    File result  = open_file__raw(wfn, open_type, access, share);
    bg_free(wfn);
    return result;
}
#endif


#if BG_SYSTEM_LINUX
File
open_file__raw(const char *fn, File_Open_Flags open_type, File_Access_Flags access, File_Share_Flags share) {
    
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
open_file(const char *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share); 
}

File
create_file(const char *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_New, access, share);
}


#if BG_SYSTEM_WINDOWS
File
create_file(const BgUtf16 *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share);
}

File
open_file(const BgUtf16 *fn, File_Access_Flags access, File_Share_Flags share) {
    return open_file__raw(fn, File_Open_Flags_Existing, access, share); 
}
#endif


#if BG_SYSTEM_WINDOWS
File_View
open_file_view(const char *fn) {
    // for small strings, don't heap allocate
    u64 slen = string_length(fn);
    if (slen < 256) {
        BgUtf16 wfn[256];
        zero_memory(wfn, sizeof(wfn));
        multibyte_to_widestr(fn, wfn, sizeof(wfn));
        return open_file_view(wfn);
    }
    else {
        BgUtf16 *wfn = multibyte_to_widestr(fn);
        File_View result = open_file_view(wfn);
        bg_free(wfn);        
        return result;
    }
}

File_View
open_file_view(const BgUtf16 *fn) {
    File_View result = {};
    HANDLE fhandle = CreateFileW(fn, GENERIC_READ, FILE_SHARE_READ, 0, OPEN_ALWAYS, 0, 0);
    if (fhandle != INVALID_HANDLE_VALUE) {
        HANDLE mhandle = CreateFileMappingW(fhandle, NULL, PAGE_READONLY, 0, 0, 0);
        if (mhandle != INVALID_HANDLE_VALUE && mhandle != NULL) {
            LARGE_INTEGER li = {};
            GetFileSizeEx(fhandle, &li);

            result.data = MapViewOfFile(mhandle, FILE_MAP_READ, 0, 0, 0);
            result.size = li.QuadPart;
            CloseHandle(mhandle);
        }   
        else {
            // err
        } 
        CloseHandle(fhandle);
    }
    else {
        // err
    }

    return result;
}

void
close_file_view(File_View *view) {
    UnmapViewOfFile(view->data);
}
#endif

#if BG_SYSTEM_LINUX
#include <sys/mman.h>
File_View
open_file_view(const char *fp) {
    File_View result = {};
    int FD = open(fp, O_RDONLY);
    if(FD != -1){
        u64 file_size = get_file_size(fp);
        void *file_mapped_memory = mmap(0, file_size, PROT_READ, MAP_SHARED_VALIDATE, FD, 0);
        close(FD);        
        if(file_mapped_memory != 0){
            result.data = (uint8_t*)file_mapped_memory;
            result.size = file_size;
        }
        
    }

    return result;
}

void
close_file_view(File_View *fv) {
    munmap(fv->data, fv->size);
}
#endif


bool is_file_handle_valid(File *file) {
#if BG_SYSTEM_WINDOWS
    return file->handle != INVALID_HANDLE_VALUE;
#else
    return file->fd != -1;
#endif
}


bool delete_directory(const char *dir) {
#if BG_SYSTEM_WINDOWS
    bool result = 0 != RemoveDirectoryA(dir);
    return result;
#else
    
    char command[4096] = {};
    BG_ASSERT(string_length(dir) < sizeof(command) - 40);

    snprintf(command, sizeof(command), "rm -rf %s", dir);
    system(command);
    return true;

#endif
}

bool
delete_file(const char *fn) {
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
delete_file(const BgUtf16 *fn) {

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
    // int mode  = 0;
    int fd = open(fn, flags);
    defer({close(fd);});
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
    if (file != INVALID_HANDLE_VALUE) {
        LARGE_INTEGER fsli ={};
        BOOL winapiresult  = GetFileSizeEx(file, &fsli);
        BG_ASSERT(winapiresult);
        LOG_DEBUG("get_file_size result -> (%10lld) %S", fsli.QuadPart, fn);
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


u64
get_file_size(File *file) {
#if BG_SYSTEM_WINDOWS
    LARGE_INTEGER li = {};
    auto gfr = GetFileSizeEx(file->handle, &li);
    BG_ASSERT(gfr != 0);
    bg_unused(gfr);
    return (u64)li.QuadPart;
#else

    s64 cur = lseek64(file->fd, 0, SEEK_CUR);
    s64 r   = lseek64(file->fd, 0, SEEK_END);
    s64 rr  = lseek64(file->fd, 0, SEEK_SET);

    bg_unused(cur);
    bg_unused(r);
    bg_unused(rr);
    // BG_ASSERT(rr == cur);
    return r;
#endif
}


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

#if BG_SYSTEM_WINDOWS
File_Read
read_file_all(const wchar_t *fn) {

    File_Read result = {};    
    File file = open_file(fn);

    if (is_file_handle_valid(&file)) {

        BG_ASSERT(result.len == 0);
        result.len = get_file_size(&file);

        if (result.len > 0) {
            result.data = bg_malloc(result.len);
        }
    
        set_fp(&file, 0);
        bool rfr = read_file(&file, result.data, result.len);
        
        if (!rfr) { 
            LOG_ERROR("Unable to read %llu bytes from file %s\n", result.len, fn);
            bg_free(result.data);
            result = {};
        }
    }

    close_file(&file);
    return result;
}
#endif


File_Read
read_file_all(const char *fn) {

    File_Read result = {};    
    File file = open_file(fn);

    if (is_file_handle_valid(&file)) {

        BG_ASSERT(result.len == 0);
        result.len = get_file_size(&file);

        if (result.len > 0) {
            result.data = bg_malloc(result.len);
        }
	
        set_fp(&file, 0);
        bool rfr = read_file(&file, result.data, result.len);
		
        if (!rfr) { 
            LOG_ERROR("Unable to read %llu bytes from file %s\n", result.len, fn);
            bg_free(result.data);
            result = {};
        }
    }

    close_file(&file);
    return result;
}

void
free_file_read(File_Read *fr) {
    bg_free(fr->data);
    fr->len = 0;
    fr->data = 0;
}


bool 
read_filen_last(const char *fn, void *data, u64 n) {
    auto file      = open_file(fn);
    if (!is_file_handle_valid(&file))
        return false;
    defer({ close_file(&file); });

    auto file_size = get_file_size(&file);
    if (file_size < n) 
        return false;

    set_fp(&file, file_size - n);
    bool rfr = read_file(&file, data, n);
    return rfr;
}

bool
read_filen(char *fn, void *data, u64 n) {
    BG_ASSERT(n < Gigabyte(2));
    if (n == 0)
        return true;

    File file = open_file(fn);
    bool rfr  = read_file(&file, data, n);
    if (!rfr) {
        LOG_ERROR("Unable to read %lld bytes from file %s\n", n, fn);
    }
    close_file(&file);
    return rfr;
}

bool
dump_file(char *fn, const void *d, u64 n) {
    auto file = create_file(fn);
    bool wfr = write_file(&file, d, n);
    if (!wfr) {
        LOG_ERROR("Unable to dump contents to file %s\n", fn);
    }
    close_file(&file);
    return wfr;
}

#if BG_SYSTEM_WINDOWS
bool
copy_file_overwrite(char *file, char *dest) {
    return CopyFileA(file, dest, true);
}

bool
copy_file_overwrite(BgUtf16 *file, BgUtf16 *dest) {
    return CopyFileW((BgUtf16 *)file, dest, true);
}
#endif

#if BG_SYSTEM_WINDOWS
Array<BgUtf16 *>
get_file_paths_in_directory(const BgUtf16 *dir) {

    BgUtf16 wildcard_dir[280] ={};
    u64 dirlen = string_length(dir);
    dirlen++; // null termination

    BG_ASSERT(sizeof(wildcard_dir) > dirlen);

    Array<BgUtf16 *> result ={};
    arrreserve(&result, 20);

    string_append(wildcard_dir, dir);
    string_append(wildcard_dir, L"\\*");

    WIN32_FIND_DATAW FDATA;
    HANDLE FileIterator = FindFirstFileW(wildcard_dir, &FDATA);

    if (FileIterator != INVALID_HANDLE_VALUE) {

        while (FindNextFileW(FileIterator, &FDATA) != 0) {

            //@NOTE(Batuhan): Do not search for sub-directories, skip folders.
            if (FDATA.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                continue;
            }

            // @Stupid +5 just to be sure there is enough room for null termination
            u64 fn_len = string_length(FDATA.cFileName);
            fn_len += dirlen + 5;

            BgUtf16 *fnbuffer = (BgUtf16 *)bg_calloc(fn_len, 2);

            u64 slen = 0;
            slen = string_append(fnbuffer, dir, slen);
            slen = string_append(fnbuffer, L"\\", slen);
            slen = string_append(fnbuffer, FDATA.cFileName, slen);

            arrput(&result, fnbuffer);

        }


    }
    else {
        printf("Cant iterate directory\n");
    }


    FindClose(FileIterator);

    return result;

}
#endif

Array<char *>
get_file_paths_in_directory(const char *dir) {
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
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ls %s -p -A > nar_ls_out.txt", dir);
    system(cmd);
    File_Read fr = read_file_all("nar_ls_out.txt");
    Array<char *> result;
    u64 dirlen = string_length(dir);

    if (fr.data) {
        char *f = (char *)fr.data;
        for(u64 i=0, r=0;i<fr.len;++i) {
            
            if (f[i] == '\n') {
                if (f[i-1] == '/') {
                    r=i+1;
                    continue; // skip directories.
                }
                int sl= (i - r) + dirlen;
                sl+=2;// one for null termination one for seperator

                char *tfn=(char*)bg_calloc(sl, 1);
                string_append(tfn, dir);
                tfn[dirlen]='/';
                copy_memory(tfn + dirlen + 1, &f[r], i - r); 
                arrput(&result, tfn);
                r=i+1;
            }

        }


    }

    free_file_read(&fr);
    return result;
#endif
}

void
free_filelist(Array<char *> &list) {
    for_array (i, list) {
        bg_free(list[i]);        
    } arrfree(&list);
}

#if BG_SYSTEM_WINDOWS
void
free_filelist(Array<wchar_t *> &list) {
    for_array (i, list) {
        bg_free(list[i]);        
    } arrfree(&list);
}
#endif


int bg_check_for_memory_leaks() {
#if BG_ENABLE_LEAKCHECK
    return stb_leakcheck_dumpmem();
#else
    return 0;
#endif
}


#endif









