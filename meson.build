project(
    'rkwifibt',
    'c',
    version : '1.0.0',
    meson_version : '>=0.45.0',
)

executable(
    'brcm_patchram_plus1',
    [ 'tools/brcm_tools/brcm_patchram_plus1.c' ],
    install : true,
)

executable(
    'dhd_priv',
    [ 'tools/brcm_tools/dhd_priv.c' ],
    install : true,
)

executable(
    'rtk_hciattach',
    [ 'tools/rtk_hciattach/hciattach_rtk.c', 'tools/rtk_hciattach/hciattach.c', 'tools/rtk_hciattach/hciattach_h4.c', 'tools/rtk_hciattach/rtb_fwc.c'],
    install : true,
)
