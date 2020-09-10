#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <assert.h>
#include <string.h>

#include <faux_s.h>

static int ticks             = 0;

static int boops_per_minute  = 0;
static volatile int nboops   = 0;
static volatile int prevboop = 0;

static int memes_per_minute  = 0;
static volatile int nmemes   = 0;
static char *current_meme    = NULL;

static int meme_offset       = 0;
static int reading_nmemes    = 0;

static char *memes[] = {
	"I can haz cheeseburger?",
	"trollface",
	"honeybadger don't give a poop",
	"this is fine",
	"#firstworldproblems",
	"Don't tase me, bro",
	"Imma let you finish",
	"double rainbow",
	"Its a trap!",
	"All your base are belong to us",
	"Deal with it.",
	"Never gonna give you up, never gonna let you down..."
};

struct faux_s_device devs[] = {
	{
		.allocated = 0,
		.type = FAUX_S_DEV_BOOP,
		.status = DMA_STATUS_NO_BUF,
		.dma_buffer = NULL,
		.vector_offset = -1
	},
	{
		.allocated = 0,
		.type = FAUX_S_DEV_MEMES,
		.status = DMA_STATUS_NO_BUF,
		.dma_buffer = NULL,
		.vector_offset = -1
	},
	{
		.allocated = 0,
		.type = FAUX_S_DEV_TERM_IN,
		.status = DMA_STATUS_NO_BUF,
		.dma_buffer = NULL,
		.vector_offset = -1
	}
};

int
faux_s_dev_reg_write(struct faux_s_device *d, int mpm)
{
	assert(d);

	if (mpm < 0) return -1;
	switch (d->type) {
	case FAUX_S_DEV_BOOP: {
		boops_per_minute = mpm;

		break;
	}
	case FAUX_S_DEV_MEMES: {
		memes_per_minute = mpm;

		break;
	}
	case FAUX_S_DEV_TERM_IN: {
		assert(0);
		break;
	}
	default:
		assert(0);
	}

	return 0;
}

#define WORK_AMNT 100000000
static volatile unsigned int work = 0;

/**
 * Read data from the device, and place it in `ret`. If multiple data
 * items are available on the device, subsequent calls to this will
 * read them out. The return value indicates if data was retrieved
 * properly, or if there is no ready data.
 *
 * - @d - the device
 * - @ret - the variable in which to return data from the device
 * - @return - The return value can hold one of the following values:
 *     - `0` if data is returned. The device will continue to return `0`
 *       while new data is available. This can be used to iterate
 *       through all of the available data.
 *     - `1` if there is no error, but new data is not available.
 *     - `-1` if there is an error.
 */
int
faux_s_dev_reg_read(struct faux_s_device *d, char *ret)
{
	unsigned int i;
	assert(d && ret);

	/* simulate the cost to *check* for data */
	for (i = 0; i < WORK_AMNT; i++) work++;

	switch(d->type) {
	case FAUX_S_DEV_BOOP: {
		const char *boopstr = "BOOP";

		/* if there is no data to retrieve, return as such */
		if (nboops == prevboop) return 1;
		prevboop = nboops;
		*ret     = boopstr[nboops % strlen(boopstr)];
		break;
	}
	case FAUX_S_DEV_MEMES: {
		char *meme = (char *)current_meme;

		if (reading_nmemes != nmemes) {
			reading_nmemes = nmemes;
			meme_offset    = 0;
		}
		if (meme_offset == (int)(strlen(meme)) + 1) return 1;
		*ret = current_meme[meme_offset];
		meme_offset++;
		break;
	}
	case FAUX_S_DEV_TERM_IN: {
		return -1;
		break;
	}
	default:
		assert(0);
	}

	return 0;
}

/**
 * This is the pretend device processing. Memes generate random memes
 * at a given rate. Boops generates periodic interrupts. The terminal
 * input generates events corresponding to keyboard data.
 */
void
faux_s_dev_process(void)
{
	int ticks_per_min = 60 * 10;

	ticks++;

	/* assuming we activate 10/second */
	if ((memes_per_minute > 0) &&
	    ((ticks_per_min / memes_per_minute) > 0) &&
	    (ticks % (ticks_per_min / memes_per_minute) == 0)) {
		struct faux_s_device *d = &devs[FAUX_S_DEV_MEMES];
		int sz  = sizeof(memes) / sizeof(char *);
		assert(sz > 0);
		char *m = memes[rand() % sz];

		current_meme = m;
		nmemes++;
		if (d->status == DMA_STATUS_RECEIVING) {
			faux_s_dev_dma_populate(d, m, strlen(m) + 1);
		}
		faux_s_interrupt_trigger(d->vector_offset);
	}

	if ((boops_per_minute > 0) &&
	    ((ticks_per_min / boops_per_minute) > 0) &&
	    (ticks % (ticks_per_min / boops_per_minute) == 0)) {
		struct faux_s_device *d = &devs[FAUX_S_DEV_BOOP];

		nboops++;
		faux_s_interrupt_trigger(d->vector_offset);
	}

	return;
}

struct faux_s_device *
faux_s_dev_create(faux_s_dev_t devtype)
{
	struct faux_s_device *d = &devs[devtype];

	if (d->allocated) return NULL;
	d->allocated     = 1;

	return d;
}

int
faux_s_dev_destroy(struct faux_s_device *d)
{
	if (d->vector_offset == -1) return 0;
	if (d->status != DMA_STATUS_NO_BUF) return -1;
	d->vector_offset = -1;
	d->allocated     = 0;

	return 0;
}

void
faux_s_dev_init(void)
{
	current_meme = memes[0];
	srand(time(NULL));
}
