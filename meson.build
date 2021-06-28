project(
    'rkwifibt',
    'c',
    version : '1.0.0',
    meson_version : '>=0.45.0',
)

executable(
    'brcm_patchram_plus1',
    [ 'brcm_tools/brcm_patchram_plus1.c' ],
    install : true,
)

executable(
    'dhd_priv',
    [ 'brcm_tools/dhd_priv.c' ],
    install : true,
)

executable(
    'rk_wifi_init',
    [ 'src/rk_wifi_init.c' ],
    install : true,
)

install_subdir('firmware', install_dir : '/system/etc/', install_mode : ['rw-r--r--', 'root'])
