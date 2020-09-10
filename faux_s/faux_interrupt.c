/*
 * SPDX-License-Identifier: BSD-2-CLAUSE
 *
 * Copyright by
 * - Bite Ye, 2020
 * - Gabriel Parmer, 2020
 */

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include <assert.h>

#include <faux_s.h>

struct faux_s_interrupt_vector vector;

int
faux_s_interrupt_set(int vect_offset, isr_fn_t fn)
{
	if (vect_offset >= NINTERRUPTS || vect_offset < 0) return -1;
	vector.int_vector[vect_offset] = fn;

	return 0;
}

int
faux_s_interrupt_trigger(int vect_offset)
{
	isr_fn_t fn;

	if (vect_offset >= NINTERRUPTS || vect_offset < 0) return -1;
	fn = vector.int_vector[vect_offset];
	if (fn) fn();

	return 0;
}

int
faux_s_interrupt_hook_up(struct faux_s_device *d, int vect_offset)
{
	assert(d);

	d->vector_offset = vect_offset;

	return 0;
}

extern void faux_s_dev_process(void);

void
sig_handler(int _a)
{
	(void)_a;

	faux_s_dev_process();
}

void
signal_setup(void)
{
	timer_t           timer;
	struct itimerspec ts;
	struct sigaction  sig_action;
	struct sigevent   evp;
	unsigned long long ms = 1000000; /* millisecond = million ns */

	sig_action.sa_handler = sig_handler;
	sigemptyset(&sig_action.sa_mask);
	sig_action.sa_flags = SA_RESTART | SA_NODEFER;
	CHECK(sigaction(SIGUSR1, &sig_action, NULL));

	memset(&evp, 0, sizeof(struct sigevent));
	evp.sigev_value.sival_ptr = &timer;
	evp.sigev_notify          = SIGEV_SIGNAL;
	evp.sigev_signo           = SIGUSR1;

	CHECK(timer_create(CLOCK_REALTIME, &evp, &timer));
	ts.it_interval.tv_sec  = 0;
	ts.it_interval.tv_nsec = ms * 100;
	ts.it_value.tv_sec     = 0;
	ts.it_value.tv_nsec    = ms * 100;
	CHECK(timer_settime(timer, 0, &ts, NULL));

	return;
}

void
faux_s_interrupt_init(void)
{
	signal_setup();

	return;
}
