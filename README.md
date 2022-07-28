# bg
personal, platform-free c++ standard library  

# features
 - easy to use async io interface(no async support for linux yet)
 - saner string api
 - array<t> type that doesn't tank compile times like std::vector.
 - scope defer (from gingerBill)!
 - platform-compiler detection macros that isn't cryptic. (BG_SYSTEM_WINDOWS, BG_COMPILER_MSVC etc)
 - lots of utility macros. (LOG_.., zero_memory, for_array)
 - platform filesystem interactions(delete, copy, list all files etc)
 - easy to use iterators for arrays and scopes
