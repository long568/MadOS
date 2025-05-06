mcu_specs = {
    'stm32f103c8t6': {'flash_kb':  64, 'ram_kb': 20, 'startup': 'startup_stm32f10x_md.s', 'density': 'STM32F10X_MD', 'isr_ram': 484},
    'stm32f103rct6': {'flash_kb': 256, 'ram_kb': 48, 'startup': 'startup_stm32f10x_hd.s', 'density': 'STM32F10X_HD', 'isr_ram': 484},
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
    [mcu_density, '-DUSE_STDPERIPH_DRIVER', '-DVECT_TAB_SRAM', '-mcpu=cortex-m3', '-mthumb'],
    language: ['c', 'cpp']
)

add_project_link_arguments(
    ['-mcpu=cortex-m3', '-mthumb', '-T', linker_script],
    language: ['c', 'cpp']
)

arch_inc = include_directories(
    '.',
    'Libraries',
    'Libraries/CMSIS',
    'Libraries/STM32F10x_StdPeriph_Driver/inc'
)

arch_src = files(
    'MadArchS.s',
    'MadArchC.c',
    'MadArchMem.c',
    'MadISR.c',
    'Libraries/system_stm32f10x.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_adc.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_bkp.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_can.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_cec.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_crc.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dac.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dbgmcu.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_dma.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_exti.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_flash.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_fsmc.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_gpio.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_i2c.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_iwdg.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_pwr.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rcc.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_rtc.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_sdio.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_spi.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_tim.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_usart.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/stm32f10x_wwdg.c',
    'Libraries/STM32F10x_StdPeriph_Driver/src/misc.c'
)
