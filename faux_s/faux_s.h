/**
 * Faux S (pronounced "O S") is a Linux-based emulator of devices and
 * device interactions.
 */

#ifndef FAUX_S_H
#define FAUX_S_H

#include <errno.h>

#define NINTERRUPTS 2

/* Our interrupt vector */
typedef void (*isr_fn_t)(void);
struct faux_s_interrupt_vector {
	isr_fn_t int_vector[NINTERRUPTS];
};

/* The different devices available on the system. */
typedef enum {
	FAUX_S_DEV_BOOP    = 0, /* periodic Penny boops 'r us */
	FAUX_S_DEV_MEMES   = 1,	/* meme generator */
	FAUX_S_DEV_TERM_IN = 2,	/* access keyboard input */
	FAUX_S_DEV_MAX
} faux_s_dev_t;

/* DMA buffer status */
typedef enum {
	DMA_STATUS_NO_BUF,	/* no buffer referenced */
	DMA_STATUS_RECEIVING,	/* buffer that can receive data */
	DMA_STATUS_POPULATED,	/* populated buffer with data */
} dma_status_t;


struct faux_s_device {
	int allocated;
	faux_s_dev_t type;
	dma_status_t status;
	void *dma_buffer;
	int vector_offset;
};

int faux_s_interrupt_set(int vect_offset, isr_fn_t fn);
int faux_s_interrupt_trigger(int vect_offset);
void faux_s_interrupt_init(void);
int faux_s_interrupt_hook_up(struct faux_s_device *d, int vect_offset);

struct faux_s_device *faux_s_dev_create(faux_s_dev_t devtype);
int faux_s_dev_destroy(struct faux_s_device *d);

#define FAUX_S_DMA_BUF_SZ 1024
int faux_s_dev_dma_enqueue(struct faux_s_device *d, void *buf);
int faux_s_dev_dma_dequeue(struct faux_s_device *d, void **buf);
int faux_s_dev_dma_populate(struct faux_s_device *d, void *b, int sz);

/*
 * The meme and boop device exports a register we can write to: to set
 * the memes/boops per minute throughput
 */
int faux_s_dev_reg_write(struct faux_s_device *d, int ops_per_minute);
int faux_s_dev_reg_read(struct faux_s_device *d, char *ret);

void faux_s_dev_init(void);


/* Utilities: */

#define PRINTLN() printf("%d\n", __LINE__)
void unit_of_work(void);
#define CHECK(x)							\
	do {								\
		int retval = (x);					\
		if (retval < 0) {					\
			printf("Return value error: %d in %s failed. retval: %d errno: %d", \
			       __LINE__, __FUNCTION__, retval, errno);	\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)

#endif	/* FAUX_S_H */
