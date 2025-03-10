



.global rt_hw_stack_init
rt_hw_stack_init:
    jirl		$zero, $ra, 0


.global rt_hw_context_switch_interrupt
rt_hw_context_switch_interrupt:
    jirl		$zero, $ra, 0


.global rt_hw_context_switch
rt_hw_context_switch:
    jirl		$zero, $ra, 0



.global rt_hw_context_switch_to
rt_hw_context_switch_to:
    jirl		$zero, $ra, 0



.global cpu_idle
cpu_idle:
	idle   0x0
	ret



