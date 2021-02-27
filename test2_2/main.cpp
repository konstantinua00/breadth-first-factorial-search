#define SIZE 21
#define STORAGE_SIZE 70'000'000
#define THREAD_NUM 1
#define PRIME_REP 2

#include <iostream>
#include <gmp.h>
#include <thread>
#include <atomic>
#include <array>
#include <vector>
#include <chrono>

using namespace std;


mpz_t g_fact[SIZE];
array<atomic<int>, 2> Atom{2, 0};
char buff[1000];

void Thread_Foo(mpz_t** Main_Storage,int Layer)
{
	mpz_t HugeNum;
	mpz_init(HugeNum);	//initialization
	
	
	int I_input, I_output;
	do
	{
		I_input = Atom[Layer%2].fetch_sub(1);
		cout << I_input;
		if (I_input < 0) break;              //We got safe stored number to read from
		
		mpz_set(HugeNum, Main_Storage[Layer%2][I_input]);//we read it and store internally
		
		for (int i=1; i <= Layer+1; i++)
		{
			mpz_add(HugeNum, HugeNum, g_fact[Layer]);
			if (!mpz_probab_prime_p(HugeNum, PRIME_REP)) continue; //if we find prime of next level
			
			I_output = Atom[!(Layer%2)].fetch_add(1);              //we get safe number to store in
			mpz_set(Main_Storage[!(Layer%2)][I_output], HugeNum);
		}
		
	}while(I_input);
	
}


int main()
{

	mpz_init_set_ui(g_fact[0], 1);
	for (int i=1; i < SIZE; i++)
	{
		mpz_init(g_fact[i]);
		mpz_mul_ui(g_fact[i], g_fact[i-1], i+1); //Factorials are set
	}
	
	mpz_t* Main_Storage[2];
	Main_Storage[0] = new mpz_t[STORAGE_SIZE];
	Main_Storage[1] = new mpz_t[STORAGE_SIZE];	//Storage is set
	
	mpz_init_set_ui(Main_Storage[0][0], 2);
	mpz_init_set_ui(Main_Storage[0][1], 3);
	mpz_init_set_ui(Main_Storage[0][2], 5);
	for (int i=3; i<STORAGE_SIZE; i++)
	{
		mpz_init(Main_Storage[0][i]);
	}
	for (int i=0; i<STORAGE_SIZE; i++)
	{
		mpz_init(Main_Storage[1][i]);	//Initial values are set
	}
	
	
	/*bool flip = 1;
	for (int Layer = 2; Layer < SIZE; Layer++)
	{
		auto Start = chrono::steady_clock::now();
		bool flip = !flip;
		thread Thread_Stack[THREAD_NUM];
	
		for (int i=0; i < THREAD_NUM; i++)
		{
			Thread_Stack[i] = thread(Thread_Foo, Main_Storage, Layer);
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
	}*/
	
	
	getchar();
    return 0;
}
