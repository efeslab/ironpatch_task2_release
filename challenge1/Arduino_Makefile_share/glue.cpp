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

struct fakeoutput g_fakeoutput_buf[FAKEOUTPUT_SIZE] = { 0 };
int g_fakeoutput_idx = 0;

unsigned long g_timestamp = 0;

unsigned char g_fake_checkReceive_input[NR_ITERATIONS] = { 0 };
unsigned char g_fake_inputs[NR_ITERATIONS][8] = {0};

int g_iteration = 0;
extern elapsedMillis flash_timer; 

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
	memset(g_fakeoutput_buf, 0, sizeof(g_fakeoutput_buf));
	memset(g_fake_checkReceive_input, 0, sizeof(g_fake_checkReceive_input));

	flash_timer = 0; // This is a global variable of a class whose
			// constructor calls millis(). You might not need it
			// here but this caused me a hard bug.

	g_fakeoutput_idx = 0;
	g_timestamp = 0;

	setup();
	for( g_iteration = 0 ; g_iteration < NR_ITERATIONS ; g_iteration ++ ) {
		loop();
	}
}
