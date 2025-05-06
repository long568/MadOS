mcu = get_option('mcu').to_lower()

arch = 'arch/'
if mcu.startswith('stm32f10')
    arch = join_paths(arch, 'stm32f10x')
elif mcu.startswith('stm32g0')
    arch = join_paths(arch, 'stm32g0xx')
else
    error('Unsupported MCU: ' + mcu)
endif

subdir(arch)
subdir('ext')
subdir('libc')
subdir('kernel')

os_inc = [ arch_inc, ext_inc, libc_inc, kernel_inc ]
os_src = [ arch_src, ext_src, libc_src, kernel_src ]

global_config += {
    'os_inc': os_inc
}

os = static_library('os',
    sources: os_src,
    include_directories: os_inc,
    install: false,
    pic: false,
)
os = os
