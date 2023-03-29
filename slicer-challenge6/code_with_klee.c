#include <assert.h>
#include "klee.h"

int vuln_g_break_status = 0;
int vuln_g_turn_status = 0;

int patched_g_break_status = 0;
int patched_g_turn_status = 0;

////////////////////////////// Turn funcs begin /////////////////////
void vuln_toggle_turn(int x)
{
	vuln_g_turn_status = 1 - vuln_g_turn_status;
}

void patched_toggle_turn(int x)
{
	patched_g_turn_status = 1 - patched_g_turn_status;
}
////////////////////////////// Turn funcs end /////////////////////

////////////////////////////// Break funcs begin /////////////////////
void vuln_toggle_break(int x)
{
	if (x > 0)
		vuln_g_break_status = 1 - vuln_g_break_status;
}

void patched_toggle_break(int x)
{
	if (x != 0)
		patched_g_break_status = 1 - patched_g_break_status;
}
////////////////////////////// Break funcs end /////////////////////

int main()
{
	char input[8] = { 5, 5, 5, 7, 5, 5, 5, 5 }; // TODO: KLEE make symbolic
	klee_make_symbolic(input, sizeof(input), "input");

	vuln_toggle_break(input[3]);
	vuln_toggle_turn(input[2]);

	patched_toggle_break(input[3]);
	patched_toggle_turn(input[2]);

	assert(vuln_g_break_status == patched_g_break_status); // Slicing criteria
}

