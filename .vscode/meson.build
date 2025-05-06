app      = get_option('app')
mcu      = get_option('mcu')
debugger = get_option('debugger')

mcu_np = mcu.substring(0, -2)
mcu_family = mcu.substring(0, -6) + 'x'

launch = configure_file(
    input: 'template_launch.json',
    output: 'launch.json',
    configuration: {
        'APP': app,
        'MCU_NO_PACKAGE': mcu_np,
        'INTERFACE': debugger,
        'MCU_FAMILY': mcu_family
    }
)

configure_file(
    input: launch,
    output: 'dummy',
    command: ['cp', '@INPUT@', meson.project_source_root() + '/.vscode/launch.json']
)
