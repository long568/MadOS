project('mados', 'c' ,meson_version: '>=1.4.0', version: '3.0.0')

global_config = {}

app = get_option('app')
mcu = get_option('mcu')
message('Build ' + app + ' based on ' + mcu)

subdir('.vscode')
subdir('os')

subdir(join_paths('app', app))
