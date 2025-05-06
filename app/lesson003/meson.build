# 定义应用程序源文件
app_src = files(
    global_config['startup.s'],
    'main.c'
)

# 生成最终可执行文件
executable('app',
    sources: app_src,
    include_directories: 
        global_config['os_inc'] + [
            include_directories('.'),
        ],
    link_with: os
)
