/* Just add some "fake" work */
#define WORK_AMNT 10000000
static volatile unsigned int work = 0;
void
unit_of_work(void)
{
	unsigned int i;

	for (i = 0; i < WORK_AMNT; i++) work++;

	return;
}
