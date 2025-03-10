

    .align 2
    .global trap_entry
    .global debug_check_sp
trap_entry:
    ret


.global rt_hw_interrupt_init
rt_hw_interrupt_init:
    jirl        $zero, $ra, 0



.global rt_hw_interrupt_install
rt_hw_interrupt_install:
    jirl        $zero, $ra, 0



.global rt_hw_interrupt_umask
rt_hw_interrupt_umask:
    jirl        $zero, $ra, 0



.global rt_hw_interrupt_enable
rt_hw_interrupt_enable:
    jirl		$zero, $ra, 0


.global rt_hw_interrupt_disable
rt_hw_interrupt_disable:
    jirl		$zero, $ra, 0



