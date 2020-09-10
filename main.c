/**
 * These variables control if we use the boop device, or the meme
 * device, and if we use register I/O, or interrupt I/O. The mode
 * changes based on if you comment each line out or not.
 */
#define BOOPDEV
#define POLL_IO

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <faux_s.h>

struct faux_s_device *dev;

/* The ISR handler we register with the device! */
void
dev_isr(void)
{
	int ret;
	char data;

	assert(dev);
	/* Read out, and print all of the data from the device! */
	do {
		ret = faux_s_dev_reg_read(dev, &data);
		if (ret < 0) {
			printf("reg_read failure\n");
			exit(EXIT_FAILURE);
		}
		if (ret == 0) {
			printf("%c", data);
			fflush(stdout);
		}
	} while (ret != 1);

	return;
}

#ifdef BOOPDEV
#define DEV_ID FAUX_S_DEV_BOOP
#else
#define DEV_ID FAUX_S_DEV_MEMES
#endif

int
main(void)
{
	faux_s_interrupt_init();
	faux_s_dev_init();

	/* set the interrupt vector entry 0 to our isr handler! */
	faux_s_interrupt_set(0, dev_isr);
	/* Create the device */
	dev = faux_s_dev_create(DEV_ID);
	assert(dev);
#ifndef POLL_IO
	/* Hook up interrupt line 0 to the boot device */
	if (faux_s_interrupt_hook_up(dev, 0)) {
		printf("Hooking up interrupt failed.\n");
		exit(EXIT_FAILURE);
	}
#endif
	/* 60 boops/memes per minute sounds about right */
	if (faux_s_dev_reg_write(dev, 60)) {
 		printf("Register write failure.\n");
		exit(EXIT_FAILURE);
	}

	/**
	 * The work loop where it does useful work, and (if
	 * configured) does polling on the available I/O. Each item of
	 * work done is represented with a `_`.
	 */
	while (1) {
#ifdef POLL_IO
		int ret;
		char data;

		/* Poll on the I/O to get our boops/memes */
		do {
			/* repetitively read all of the data on the device */
			ret = faux_s_dev_reg_read(dev, &data);
			if (ret < 0) {
				printf("Register read failure.\n");
				exit(EXIT_FAILURE);
			}
			if (ret == 0) {
				printf("%c", data);
				fflush(stdout);
			}
		} while (ret != 1);
#endif

		unit_of_work();
		printf("_"); 	/* visual indication that we're doing work! */
		fflush(stdout);
	}

	return 0;
}
