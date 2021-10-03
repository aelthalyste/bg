#include "bg.h"
#include <stdio.h>
#include <windows.h>

#include <thread>
#include <vector>
#include <iostream>


#define IO_TEST_COUNT 40
#define BUF_SIZE      1024 * 1024 * 32
#define THREAD_COUNT  8

u64 clocks_per_sec;

u64
bg_clock() {
	LARGE_INTEGER li = {};
	QueryPerformanceCounter(&li);
	return li.QuadPart;
}

double
to_ms(u64 ticks) {
	return 1000.0 * ((double)ticks/(double)clocks_per_sec);
}

u64 
do_async_thing() {

	
	u64 id = bg_clock();
	char fn[100];
	snprintf(fn, sizeof(fn), "async_file%llu", id);

	delete_file(fn);
	File file = create_file(fn);
	defer({delete_file(fn);});

	BG_ASSERT(is_file_handle_valid(&file));
	
	u64 bfsize = BUF_SIZE;
	void *bf   = bg_malloc(bfsize);
	defer({free(bf);});

	for (u64 i = 0; i<bfsize/8; i+=8) {
		((u64*)bf)[i] = i * 23 - 535;
	}

	u64 total_issue = 0;
	u64 total_wait  = 0;
	for (u64 i = 0; i < IO_TEST_COUNT; i++) {
		u64 s = 0;
		u64 e = 0;

		
		s = bg_clock();
		set_fp(&file, i * Megabyte(1));
		auto ctx = write_file_async(&file, bf, bfsize);
		e = bg_clock();
		total_issue += (e - s);

		wait_io_completion(&file, &ctx);
		e = bg_clock();
		total_wait += (e - s);
	}


	LOG_INFO("issue ms : %.5f, io ms %.5f, ms : %.5f\n", to_ms(total_issue), to_ms(total_wait), to_ms(total_wait - total_issue));
	close_file(&file);

	return 0;
}

u64 
do_sync_thing() {
	u64 id = bg_clock();
	char fn[100];
	snprintf(fn, sizeof(fn), "async_file%llu", id);

	delete_file(fn);
	File file = create_file(fn);
	defer({delete_file(fn);});
	BG_ASSERT(is_file_handle_valid(&file));
	
	u64 bfsize = BUF_SIZE;
	void *bf   = bg_malloc(bfsize);
	defer({free(bf);});

	for (u64 i = 0; i<bfsize/8; i+=8) {
		((u64*)bf)[i] = i * 23 - 535;
	}

	u64 start = bg_clock();
	for (u64 i = 0; i < IO_TEST_COUNT; i++) {
		set_fp(&file, i * Megabyte(1));
		write_file(&file, bf, bfsize);
	}
	u64 end = bg_clock();

	LOG_INFO("sync io ms : %.5f\n", to_ms(end - start));
	close_file(&file);

	return 0;
} 


u64
compare_conversion_speed() {
	s64 result = 0;
	u64 test_count = 1000 * 1000 * 25; // 4million is sane number to test it;
	
	Array<char *> strings;
	arrreserve(&strings, 100);
	arrput(&strings, (char *)"4092948!£");
	arrput(&strings, (char *)"8278!£");
	arrput(&strings, (char *)"42534");
	arrput(&strings, (char *)"-4982983792");
	arrput(&strings, (char *)"-49828278!£");
	arrput(&strings, (char *)"-49828278!£");
	arrput(&strings, (char *)"49828278!£");
	arrput(&strings, (char *)"49828278!£");

	u64 std_start = bg_clock();
	for (u64 i = 0; i < test_count; i++) {
		for_array (i, strings) {
		 	result += atoll(strings[i]);
		}
	}
	u64 std_end = bg_clock();


	u64 custom_start = bg_clock();
	for (u64 i = 0; i < test_count; i++) {
		for_array (i, strings) {
		 	result += string_to_u64(strings[i]);
		}
	}
	u64 custom_end = bg_clock();

	double custom_ms = to_ms(custom_end - custom_start);
	double std_ms    = to_ms(std_end - std_start);
	LOG_INFO("Cus %.5f\nStd %.5f\nCustom is  %.5fX faster than std\n", custom_ms, std_ms, std_ms/custom_ms);
	return result;

}

int main() {

	char bf16[16]; memset(bf16, 0xcc, bg_sizeof(bf16));
	char bf32[32]; memset(bf32, 0xcc, bg_sizeof(bf32));
	char bf64[64]; memset(bf64, 0xcc, bg_sizeof(bf64));

	wchar_t wbf16[16]; memset(wbf16, 0xdd, bg_sizeof(wbf16));
	wchar_t wbf32[32]; memset(wbf32, 0xdd, bg_sizeof(wbf32));
	wchar_t wbf64[64]; memset(wbf64, 0xdd, bg_sizeof(wbf64));

	auto r1 = widestr_to_multibyte(L"my random textaaa", bf16, bg_sizeof(bf16));
	auto r2 = widestr_to_multibyte(L"my random textaaa", bf16, bg_sizeof(bf32));
	auto r3 = widestr_to_multibyte(L"my random textaaa", bf16, bg_sizeof(bf64));

	auto r4 = multibyte_to_widestr("my random textaaa", wbf16, bg_sizeof(wbf16));
	auto r5 = multibyte_to_widestr("my random textaaa", wbf16, bg_sizeof(wbf32));
	auto r6 = multibyte_to_widestr("my random textaaa", wbf16, bg_sizeof(wbf64));

	LOG_INFO("%p %p %p\n", r1, r2, r3);
	LOG_INFO("%s %s %s\n", r1, r2, r3);

	LOG_INFO("%p %p %p\n", r4, r5, r6);
	LOG_INFO("%S %S %S\n", r4, r5, r6);

	{
		u64 conversion_result = 0;
		conversion_result = string_to_u64("  405230!-43");
		BG_ASSERT(conversion_result == 405230);

		conversion_result = string_to_u64("-405230!-43");
		BG_ASSERT(conversion_result == 0);

		conversion_result = string_to_u64("0593!-43");
		BG_ASSERT(conversion_result == 593);

		conversion_result = string_to_u64("40a5230!-43");
		BG_ASSERT(conversion_result == 40);

		conversion_result = string_to_u64("8948392!-43");
		BG_ASSERT(conversion_result == 8948392);
	}

	{
		s64 conversion_result = 0;
		conversion_result = string_to_s64("  405230!-43");
		BG_ASSERT(conversion_result == 405230);

		conversion_result = string_to_s64("-405230!-43");
		BG_ASSERT(conversion_result == -405230);

		conversion_result = string_to_s64("0593!-43");
		BG_ASSERT(conversion_result == 593);

		conversion_result = string_to_s64("40a5230!-43");
		BG_ASSERT(conversion_result == 40);

		conversion_result = string_to_s64(" -00900!-43");
		BG_ASSERT(conversion_result == -900);

		conversion_result = string_to_s64("- 8948392!-43");
		BG_ASSERT(conversion_result == -8948392);
	}

	LARGE_INTEGER li;
	QueryPerformanceFrequency(&li);
	clocks_per_sec = li.QuadPart;


	compare_conversion_speed();
	return 0;


	std::vector<std::thread> thread_vector;
	thread_vector.reserve(100);

	for (u64 i =0; i<THREAD_COUNT; i++) {
		thread_vector.emplace_back(std::thread(do_async_thing));
		thread_vector.emplace_back(std::thread(do_sync_thing));
		Sleep(50);
	}

	for (u64 i = 0; i<thread_vector.size(); i++) {
		thread_vector[i].join();
	}

	printf("DONE!!! \n");

	return 0;
}