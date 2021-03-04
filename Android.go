package librga

import (
        "android/soong/android"
        "android/soong/cc"
        "fmt"
        "strings"
)

func init() {
    fmt.Println("librga want to conditional Compile")
    android.RegisterModuleType("cc_librga", DefaultsFactory)
}

func DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, Defaults)
    return module
}

func Defaults(ctx android.LoadHookContext) {
    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    if (sdkVersion >= 29 ) {
        type props struct {
            Srcs []string
            Cflags []string
            Shared_libs []string
            Include_dirs []string
            Double_loadable *bool
        }

        p := &props{}
        p.Srcs = getSrcs(ctx)
        p.Cflags = getCflags(ctx)
        p.Shared_libs = getSharedLibs(ctx)
        p.Include_dirs = getIncludeDirs(ctx)

        double_loadable := true
        p.Double_loadable = &double_loadable

        ctx.AppendProperties(p)
    } else {
        type props struct {
            Srcs []string
            Cflags []string
            Shared_libs []string
            Include_dirs []string
        }

        p := &props{}
        p.Srcs = getSrcs(ctx)
        p.Cflags = getCflags(ctx)
        p.Shared_libs = getSharedLibs(ctx)
        p.Include_dirs = getIncludeDirs(ctx)

        ctx.AppendProperties(p)
    }
}

//条件编译主要修改函数
func getCflags(ctx android.BaseContext) ([]string) {
    var cppflags []string

    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    //该打印输出为: TARGET_PRODUCT:rk3328 fmt.Println("TARGET_PRODUCT:",ctx.AConfig().Getenv("TARGET_PRODUCT")) //通过 strings.EqualFold 比较字符串，可参考go语言字符串对比
    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3368") ) {
    //添加 DEBUG 宏定义
        cppflags = append(cppflags,"-DRK3368=1")
    }

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_RK_GRALLOC_VERSION"),"4") ) {
        if (sdkVersion >= 30 ) {
            cppflags = append(cppflags,"-DUSE_GRALLOC_4")
        }
    }

    //将需要区分的环境变量在此区域添加 //....
    return cppflags
}

func getSharedLibs(ctx android.BaseContext) ([]string) {
    var libs []string

    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_RK_GRALLOC_VERSION"),"4") ) {
        if (sdkVersion >= 30 ) {
            libs = append(libs, "libgralloctypes")
            libs = append(libs, "libhidlbase")
            libs = append(libs, "android.hardware.graphics.mapper@4.0")
        }
    }

    if (sdkVersion < 29 ) {
        libs = append(libs, "libdrm")
    }

    return libs
}

func getIncludeDirs(ctx android.BaseContext) ([]string) {
    var dirs []string

    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_RK_GRALLOC_VERSION"),"4") ) {
        if (sdkVersion >= 30 ) {
            dirs = append(dirs, "hardware/rockchip/libgralloc/bifrost")
            dirs = append(dirs, "hardware/rockchip/libgralloc/bifrost/src")
        }
    }
    // Add libion for RK3368
    if (sdkVersion >= 29) {
        if (sdkVersion >= 30) {
            dirs = append(dirs, "system/memory/libion/original-kernel-headers")
        } else {
            dirs = append(dirs, "system/core/libion/original-kernel-headers")
        }
    }

    return dirs
}

func getSrcs(ctx android.BaseContext) ([]string) {
    var src []string

    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_RK_GRALLOC_VERSION"),"4") ) {
        if (sdkVersion >= 30 ) {
            src = append(src, "core/platform_gralloc4.cpp")
        }
    }

    return src
}
