mcu_specs = {
    'stm32g070rbt6': {'flash_kb': 128, 'ram_kb': 36, 'startup': 'startup_stm32g070xx.s', 'density': 'STM32G070xx', 'isr_ram': 184},
}

mcu = get_option('mcu').to_lower()
startup_file = mcu_specs[mcu]['startup']
mcu_density = '-D' + mcu_specs[mcu]['density']

linker_script = configure_file(
    input : 'template_linker_script.ld',
    output : 'linker_script.ld',
    configuration : {
        'FLASH_ORIGIN': '0x08000000',
        'FLASH_SIZE': mcu_specs[mcu]['flash_kb'],
        'RAM_ORIGIN': '0x20000000',
        'RAM_SIZE': mcu_specs[mcu]['ram_kb'],
        'ISR_RAM_SIZE': mcu_specs[mcu]['isr_ram'],
        'STACK_SIZE': get_option('stack_size'),
        'HEAP_MIN': 0,
    }
).full_path()

global_config += {
    'startup.s': join_paths(meson.current_source_dir(), 'Libraries/Startup', startup_file)
}
message('startup.s: ', global_config['startup.s'])

add_project_arguments(
    [mcu_density, '-DUSE_FULL_LL_DRIVER', '-DUSER_VECT_TAB_ADDRESS', '-DVECT_TAB_SRAM', '-mcpu=cortex-m0plus', '-mthumb'],
    language: ['c', 'cpp']
)

add_project_link_arguments(
    ['-mcpu=cortex-m0plus', '-mthumb', '-T', linker_script],
    language: ['c', 'cpp']
)

arch_inc = include_directories(
    '.',
    'Libraries/CMSIS',
    'Libraries/Include',
    'Libraries/STM32G0xx_HAL_Driver/inc'
)

arch_src = files(
    'MadArchS.s',
    'MadArchC.c',
    'MadArchMem.c',
    'MadISR.c',
    'Libraries/system_stm32g0xx.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_adc.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_comp.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_crc.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_crs.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_dac.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_dma.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_exti.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_gpio.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_i2c.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_lptim.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_lpuart.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_pwr.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_rcc.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_rng.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_rtc.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_spi.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_tim.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_ucpd.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_usart.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_usb.c',
    'Libraries/STM32G0xx_HAL_Driver/src/stm32g0xx_ll_utils.c',
)
