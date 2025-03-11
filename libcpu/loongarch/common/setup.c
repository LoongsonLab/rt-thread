

unsigned long fw_arg0;
unsigned long fw_arg1;
unsigned long fw_arg2;


static unsigned long drdtime(void)
{
	unsigned long val = 0;

	__asm__ __volatile__(
		"rdtime.d %0, $zero\n\t"
		: "=r"(val)
		:
		);
	return val;
}

extern void trap_init(void);

void init_cpu_early(void)
{
	trap_init();
}



