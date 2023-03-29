#include <stdio.h>
#include <linux/can.h>
#include <string.h>
#include <assert.h>
#include "klee.h"
#include "bumper.h"

////////////////////////////// defined here
#define NR_ITERATIONS 3
int nr_iterations = NR_ITERATIONS;
int symbolic_ready[NR_ITERATIONS];
struct can_frame symbolic_data[NR_ITERATIONS];

///////////////////////////// defined in vuln 
extern int* vuln_symbolic_ready;
extern struct can_frame* vuln_symbolic_data;
extern struct Bumper vuln_decoy_bumper;
int vuln_main(int argc, char * argv[]);
extern struct Bumper vuln_decoy_bumper; 

///////////////////////////// defined in patched 
extern int* patched_symbolic_ready;
extern struct can_frame* patched_symbolic_data;
extern struct Bumper patched_decoy_bumper;
int patched_main(int argc, char * argv[]);
extern struct Bumper patched_decoy_bumper; 

int main()
{
	char* argv[2] = {"aaa", "bbb"};

	klee_make_symbolic(&symbolic_ready, sizeof(symbolic_ready), "symbolic_ready");
	klee_make_symbolic(&symbolic_data, sizeof(symbolic_data), "symbolic_data");

	// initialize vuln
	int symbolic_ready_for_vuln[NR_ITERATIONS];
	struct can_frame symbolic_data_for_vuln[NR_ITERATIONS];
	memcpy(symbolic_ready_for_vuln, symbolic_ready, sizeof(symbolic_ready));
	memcpy(symbolic_data_for_vuln, symbolic_data, sizeof(symbolic_data));
	vuln_symbolic_ready = symbolic_ready_for_vuln;
	vuln_symbolic_data = symbolic_data_for_vuln;

	// initialize patched
	int symbolic_ready_for_patched[NR_ITERATIONS];
	struct can_frame symbolic_data_for_patched[NR_ITERATIONS];
	memcpy(symbolic_ready_for_patched, symbolic_ready, sizeof(symbolic_ready));
	memcpy(symbolic_data_for_patched, symbolic_data, sizeof(symbolic_data));
	patched_symbolic_ready = symbolic_ready_for_patched;
	patched_symbolic_data = symbolic_data_for_patched;

	patched_main(2, argv);
	vuln_main(2, argv);

	klee_assert(memcmp(&patched_decoy_bumper, &vuln_decoy_bumper, sizeof(struct Bumper)) == 0);
	//printf("I am superglue!\n");
}
