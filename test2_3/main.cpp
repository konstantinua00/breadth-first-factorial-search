#define FACT_NUM_LENGTH 21
#define STORAGE_SIZE 160'000'000
#define THREAD_NUM 1
#define PRIME_REP 1
#define ARRAY_LENGTH 2

#include <iostream>
#include <gmp.h>
#include <thread>
#include <atomic>
#include <array>
#include <vector>
#include <chrono>

using namespace std;

mpz_t g_fact[FACT_NUM_LENGTH+1]; //from 0 factorial to FACT_NUM_LENGTH factorial
array<atomic<int>, 2> Atom{2, 0}; //we start from 3 numbers in input storage and 0 in output storage
char debug_buff[1000];

void Setting_mpz(mpz_t& HugeNum, const array<unsigned long, ARRAY_LENGTH>& Array)
{
	const auto Internals = mpz_limbs_write(HugeNum, ARRAY_LENGTH);  //get address of mpz storage
	for (int i=0; i < ARRAY_LENGTH; i++)
	{
		Internals[i] = Array[i];  //write from array into mpz 
	}
	mpz_limbs_finish(HugeNum, ARRAY_LENGTH);        //Hugenum is set to decoded number
}

void Reading_mpz(const mpz_t& HugeNum, array<unsigned long, ARRAY_LENGTH>& Output_Array)
{
	const auto Output_Internals = mpz_limbs_read(HugeNum);  //get address of mpz storage
	for(int i=0; i < ARRAY_LENGTH; i++)
	{
		Output_Array[i] = Output_Internals[i];  //read from mpz into array
	}
}

void Thread_Foo (vector<array<unsigned long, ARRAY_LENGTH>>* Main_Storage, int Digit, bool flip)
{
	mpz_t HugeNum;
	mpz_init(HugeNum);	//initialization
	
	int I_input, I_output;
	do
	{
		I_input = Atom[flip].fetch_sub(1);
		if (I_input < 0) break;              //we get number of safe array to read from
		
		Setting_mpz(HugeNum, Main_Storage[flip][I_input]); //Hugenum is set to decoded number
		
		for (int i=1; i <= Digit; i++)  //changing left-most digit from 1 to Digit
		{
			mpz_add(HugeNum, HugeNum, g_fact[Digit]);
			if (!mpz_probab_prime_p(HugeNum, PRIME_REP)) continue; //skipping composite numbers
			
			I_output = Atom[!flip].fetch_add(1);  //we get number of safe array to store prime in
			auto& Output_Array = Main_Storage[!flip][I_output];
			
			Reading_mpz(HugeNum, Output_Array);  //Output_Array is set to number from mpz
		}
	}while(I_input);
	
	mpz_clear(HugeNum);  //destruction of mpz
}

int main()
{
    mpz_init_set_ui(g_fact[0], 1); //zero factorial set
	for (int i=1; i <= FACT_NUM_LENGTH; i++)
	{
		mpz_init(g_fact[i]);
		mpz_mul_ui(g_fact[i], g_fact[i-1], i); // i factorial set
	}  //Factorials are set
	
	vector<array<unsigned long, ARRAY_LENGTH>> Main_Storage[2];  //creating storage
	Main_Storage[0].resize(STORAGE_SIZE, array<unsigned long, ARRAY_LENGTH>{0,0});
	Main_Storage[1].resize(STORAGE_SIZE, array<unsigned long, ARRAY_LENGTH>{0,0});  //filling storage with zeros
	Main_Storage[0][0] = array<unsigned long, ARRAY_LENGTH> {2,0};
	Main_Storage[0][1] = array<unsigned long, ARRAY_LENGTH> {3,0};
	Main_Storage[0][2] = array<unsigned long, ARRAY_LENGTH> {5,0};  //setting starting numbers
	
	int Digit;
	bool flip;
	for (Digit = 3, flip = 0; Digit <= FACT_NUM_LENGTH; Digit++, flip = !flip) //starting from 3rd Digit from the right, up to FACT_NUM_lENGTH-th digit
	{
		auto Start = chrono::steady_clock::now();
		
			thread Thread_Stack[THREAD_NUM];
	
			for (int i=0; i < THREAD_NUM; i++)
			{
				Thread_Stack[i] = thread(Thread_Foo, Main_Storage, Digit, flip);
			}
			for (int i=0; i < THREAD_NUM; i++)
			{
				Thread_Stack[i].join();
			}
			Atom[flip].store(0); //reseting counter of soon-to-be-output storage (fetch_sub can push it below 0)
			Atom[!flip].fetch_sub(1); //last output pushed value of atomic above itself
		
		auto End = chrono::steady_clock::now();
		
		cout << Digit << ' ' << Atom[!flip].load()+1 << ' ' << chrono::duration_cast<chrono::microseconds>(End - Start).count();
		getchar();
	}
	
    return 0;
}