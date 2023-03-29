#include <HardwareSerial.h>
#include <assert.h>
#include "klee.h"
#include "elapsedMillis.h"

#define NR_ITERATIONS (3)
#define FAKEOUTPUT_SIZE (NR_ITERATIONS * 4)

#define REVERSE_CHECK (0)

#define SIZEOF(X) (sizeof(X)/sizeof(X[0]))

struct fakeoutput {
	uint8_t pin;
	int val;
	int timestamp;
};

struct fakeoutput g_fakeoutput_buf_vuln[FAKEOUTPUT_SIZE] = { 0 };
struct fakeoutput g_fakeoutput_buf_patched[FAKEOUTPUT_SIZE] = { 0 };
struct fakeoutput* g_fakeoutput_buf = NULL;
int g_fakeoutput_idx = 0;

unsigned long g_timestamp = 0;

unsigned char g_fake_checkReceive_input[NR_ITERATIONS] = { 0 };
unsigned char g_fake_inputs[NR_ITERATIONS][8] = {0};

int g_iteration = 0;
extern elapsedMillis patched_flash_timer; 
extern elapsedMillis vuln_flash_timer; 

extern "C"{
	void __assert_fail(const char * assertion, const char * file, unsigned int line, const char * function) { }
	void vuln_setup(void);
	void patched_setup(void);
	void vuln_loop(void);
	void patched_loop(void);
	unsigned long millis() {
		g_timestamp += 240;
		return g_timestamp;
	}
	unsigned long micros() { return 0; }
	void delay(unsigned long ms) {}
	void analogWrite(uint8_t pin, int val) {
		klee_assert(g_fakeoutput_idx < sizeof(struct fakeoutput) * FAKEOUTPUT_SIZE);
		g_fakeoutput_buf[g_fakeoutput_idx].pin = pin;
		g_fakeoutput_buf[g_fakeoutput_idx].val = val;
		g_fakeoutput_buf[g_fakeoutput_idx].timestamp = g_timestamp;
		g_fakeoutput_idx ++;
	}
}
unsigned char fake_checkReceive()
{
	return g_fake_checkReceive_input[g_iteration];
}

unsigned char fake_readMsgBufID(unsigned long* ID, unsigned char* len, unsigned char buf[])
{
	*ID = 42;
	*len = 8;
	memcpy(buf, &(g_fake_inputs[g_iteration][0]), 8);
	return 0;
}

int main()
{
	memset(g_fakeoutput_buf_vuln, 0, sizeof(g_fakeoutput_buf_vuln));
	memset(g_fakeoutput_buf_patched, 0, sizeof(g_fakeoutput_buf_patched));
	memset(g_fake_checkReceive_input, 0, sizeof(g_fake_checkReceive_input));

///////////////////////////////////////////////
	g_fakeoutput_idx = 0;
	g_fakeoutput_buf = g_fakeoutput_buf_vuln;
	g_timestamp = 0;

	vuln_setup();
	for( g_iteration = 0 ; g_iteration < NR_ITERATIONS ; g_iteration ++ ) {
	//	vuln_loop();
	}

	g_fakeoutput_idx = 0;
	g_fakeoutput_buf = g_fakeoutput_buf_vuln;
	g_timestamp = 0;

	patched_setup();
	for( g_iteration = 0 ; g_iteration < NR_ITERATIONS ; g_iteration ++ ) {
	//	patched_loop();
	}
//////////////////////////////////////////////
	memset(g_fakeoutput_buf_vuln, 0, sizeof(g_fakeoutput_buf_vuln));
	memset(g_fakeoutput_buf_patched, 0, sizeof(g_fakeoutput_buf_patched));
	memset(g_fake_checkReceive_input, 0, sizeof(g_fake_checkReceive_input));

	klee_make_symbolic(g_fake_checkReceive_input, sizeof(g_fake_checkReceive_input), "g_fake_checkReceive_input");
	klee_make_symbolic(g_fake_inputs, sizeof(g_fake_inputs), "g_fake_inputs");
	klee_assert(sizeof(g_fake_inputs) == NR_ITERATIONS * 8);

	klee_assert(NR_ITERATIONS <= 3); // temporary
#if REVERSE_CHECK == 0
	#if NR_ITERATIONS > 0
	int16_t speed_value0 = (g_fake_inputs[0][3] << 8) + g_fake_inputs[0][2];
	klee_assume(speed_value0 >= 0);
	#endif
	#if NR_ITERATIONS > 1
	int16_t speed_value1 = (g_fake_inputs[1][3] << 8) + g_fake_inputs[1][2];
	klee_assume(speed_value1 >= 0);
	#endif
	#if NR_ITERATIONS > 2
	int16_t speed_value2 = (g_fake_inputs[2][3] << 8) + g_fake_inputs[2][2];
	klee_assume(speed_value2 >= 0);
	#endif
#else // these are cases that should trigger different behavior
	#if NR_ITERATIONS > 0
	int16_t speed_value0 = (g_fake_inputs[0][3] << 8) + g_fake_inputs[0][2];
	#endif
	#if NR_ITERATIONS > 1
	int16_t speed_value1 = (g_fake_inputs[1][3] << 8) + g_fake_inputs[1][2];
	#endif
	#if NR_ITERATIONS > 2
	int16_t speed_value2 = (g_fake_inputs[2][3] << 8) + g_fake_inputs[2][2];
	#endif
	klee_assume(
		// speed negative        // input received                 // break pressed
		#if NR_ITERATIONS > 0
		(!!(speed_value0 < 0) & !!(g_fake_checkReceive_input[0] == 3) & !!(g_fake_inputs[0][4] & 0b00001100)) |
		#endif
		#if NR_ITERATIONS > 1
		(!!(speed_value1 < 0) & !!(g_fake_checkReceive_input[1] == 3) & !!(g_fake_inputs[1][4] & 0b00001100)) |
		#endif
		#if NR_ITERATIONS > 2
		(!!(speed_value2 < 0) & !!(g_fake_checkReceive_input[2] == 3) & !!(g_fake_inputs[2][4] & 0b00001100)) |
		#endif
		0
		);
#endif

	g_timestamp = 0;
	vuln_flash_timer = 0;
	g_timestamp = 0;
	patched_flash_timer = 0;

	g_fakeoutput_idx = 0;
	g_fakeoutput_buf = g_fakeoutput_buf_vuln;
	g_timestamp = 0;

	//vuln_setup();
	for( g_iteration = 0 ; g_iteration < NR_ITERATIONS ; g_iteration ++ ) {
		vuln_loop();
	}

	g_fakeoutput_idx = 0;
	g_fakeoutput_buf = g_fakeoutput_buf_patched;
	g_timestamp = 0;

	//patched_setup();
	for( g_iteration = 0 ; g_iteration < NR_ITERATIONS ; g_iteration ++ ) {
		patched_loop();
	}

#if REVERSE_CHECK == 0
	klee_assert(memcmp(g_fakeoutput_buf_vuln, g_fakeoutput_buf_patched, sizeof(g_fakeoutput_buf_vuln)) == 0);
#else
	klee_assert(memcmp(g_fakeoutput_buf_vuln, g_fakeoutput_buf_patched, sizeof(g_fakeoutput_buf_vuln)) != 0);
#endif
}
