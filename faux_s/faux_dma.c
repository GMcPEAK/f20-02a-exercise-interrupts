#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <faux_s.h>

/**
 * From the "OS": enqueue a buffer where none is present.  Add a new
 * buffer for the device to transfer into.  Note that there is only a
 * *single* buffer in this DMA "ring".
 *
 * - @d - the device
 * - @buf - The buffer (of size `FAUX_S_DMA_BUF_SZ`) for the device to
 *   transfer into
 * - @return - `0` on non-error, `-1` on error.
 */
int
faux_s_dev_dma_enqueue(struct faux_s_device *d, void *buf)
{
	assert(d && buf);

	if (d->status != DMA_STATUS_NO_BUF) return -1;
	d->dma_buffer = buf;
	d->status = DMA_STATUS_RECEIVING;

	return 0;
}

/**
 * From the "OS": dequeue a buffer from the device that has been
 * populated. It is the caller's job to deallocate that memory. Note
 * that it was the memory that was passed in with enqueue, so the
 * means to deallocate it should be the inverse of how that memory was
 * allocated.
 *
 * - @d - the device to get data from
 * - @buf - The pointer that gets set to the buffer being returned
 * - @return - `0` on non-error, `1` if data is not available, and
 *   `-1` on error.
 */
int
faux_s_dev_dma_dequeue(struct faux_s_device *d, void **buf)
{
	assert(d && buf);

	if (d->status == DMA_STATUS_NO_BUF) return -1;
	if (d->status == DMA_STATUS_RECEIVING) return 1;
	assert(d->status == DMA_STATUS_POPULATED);
	assert(d->dma_buffer);
	*buf = d->dma_buffer;
	d->dma_buffer = NULL;
	d->status = DMA_STATUS_NO_BUF;

	return 0;
}

/* From the Device: populate a buffer that is available, but not populated */
int
faux_s_dev_dma_populate(struct faux_s_device *d, void *m, int sz)
{
	assert(d && m);

	if (d->status != DMA_STATUS_RECEIVING) return -1;
	assert(d->dma_buffer);
	memcpy(d->dma_buffer, m, sz);
	d->status = DMA_STATUS_POPULATED;

	return 0;
}
