#define SIZE 20
#define ARRAY_SIZE 100'000'000
#define DIGIT_TYPE char
#define THREAD_NUM 4
#define PRIME_REP 2
#include <iostream>
#include <vector>
#include <array>
#include <thread>
#include <atomic>
#include <chrono>
#include <gmp.h>
using namespace std;

mpz_t g_fact[SIZE];
array<atomic<int>, 2> Atom{2, 0};
char buff[1000];


void Thread_Foo (vector<array<DIGIT_TYPE, SIZE>>& V_input, vector<array<DIGIT_TYPE, SIZE>>& V_output, int Layer)
{
	array<DIGIT_TYPE, SIZE> Arr_previous {0};
	mpz_t HugeNum;
	mpz_init(HugeNum);	//initialization
	
	
	int I_input, I_output;
	do
	{
		I_input = Atom[Layer%2].fetch_sub(1);
		if (I_input < 0) break;
		//cout << 'I' << I_input << '\n';
		array<DIGIT_TYPE, SIZE>& Arr_input = V_input[I_input]; //we got valid array representing our number
		for (int i=0; i <= Layer; i++)
		{
			int diff = (int)Arr_input[i] - (int)Arr_previous[i];
			//cout << mpz_get_str(buff, 10, HugeNum) << '+' << diff << '*' << mpz_get_str(buff, 10, g_fact[i]);
			if (diff > 0) mpz_addmul_ui(HugeNum, g_fact[i], diff);
			else mpz_submul_ui(HugeNum, g_fact[i], -diff);
			//cout << '=' << mpz_get_str(buff, 10, HugeNum) << '\n';
		}	                                              //internal mpz is made same as input value
		
		
		Arr_previous = Arr_input;                         //internal array is made same as inout value
		
		for (int i=1; i <= Layer+1; i++)
		{
			mpz_add(HugeNum, HugeNum, g_fact[Layer]);
			Arr_previous[Layer]++;
			if (!mpz_probab_prime_p(HugeNum, PRIME_REP)) continue; //if we find prime of next level
			
			I_output = Atom[!(Layer%2)].fetch_add(1);
			array<DIGIT_TYPE, SIZE>& Arr_output = V_output[I_output];     //we get valid array to put our output
			
			Arr_output = Arr_previous;
			Arr_output[Layer] = i;                                 //output array is made same as internal prime mpz
		}
		
	}while(I_input);
	
	mpz_clear(HugeNum);
}

int main()
{
	for (int i=0; i < SIZE; i++)
	{
		mpz_init(g_fact[i]);
	}
	
	mpz_set_ui(g_fact[0], 1);
	for (int i=1; i < SIZE; i++)
	{
		mpz_mul_ui(g_fact[i], g_fact[i-1], i+1);
	}
	
	vector<array<DIGIT_TYPE, SIZE>> MainStorage[2];
	MainStorage[0].resize(ARRAY_SIZE, array<DIGIT_TYPE, SIZE>{0});
	MainStorage[1].resize(ARRAY_SIZE, array<DIGIT_TYPE, SIZE>{0});
	MainStorage[0][0] = array<DIGIT_TYPE, SIZE> {0, 1};
	MainStorage[0][1] = array<DIGIT_TYPE, SIZE> {1, 1};
	MainStorage[0][2] = array<DIGIT_TYPE, SIZE> {1, 2};
	
	for (int Layer = 2; Layer < SIZE; Layer++)
	{
		auto Start = chrono::steady_clock::now();
		bool flip = Layer%2;
		thread Thread_Stack[THREAD_NUM];
	
		for (int i=0; i < THREAD_NUM; i++)
		{
			Thread_Stack[i] = thread(Thread_Foo, std::ref(MainStorage[flip]), std::ref(MainStorage[!flip]), Layer);
		}
		for (int i=0; i < THREAD_NUM; i++)
		{
			Thread_Stack[i].join();
		}
		Atom[flip].store(0);
		Atom[!flip].fetch_sub(1);
		auto End = chrono::steady_clock::now();
		
		cout << Layer+1 << ' ' << Atom[!flip].load()+1 << ' ' << chrono::duration_cast<chrono::microseconds>(End - Start).count();
		getchar();
	}
	
	return 0;
}