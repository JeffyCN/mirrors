package librga

import (
        "android/soong/android"
        "android/soong/cc"
        "strings"
)

func init() {
	android.RegisterModuleType("cc_librga", DefaultsFactory)
}

func DefaultsFactory() (android.Module) {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, Defaults)
    return module
}

func Defaults(ctx android.LoadHookContext) {
    type props struct {
        Cflags []string
		Shared_libs []string
		Include_dirs []string
    }

    p := &props{}
    p.Cflags = getCflags(ctx)
	p.Shared_libs = getSharedLibs(ctx)
	p.Include_dirs = getIncludeDirs(ctx)

    ctx.AppendProperties(p)
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

	if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3326") ) {
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

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3326") ) {
        if (sdkVersion >= 30 ) {
            libs = append(libs, "libgralloctypes")
            libs = append(libs, "libhidlbase")
			libs = append(libs, "android.hardware.graphics.mapper@4.0")
        }
    }

    return libs
}

func getIncludeDirs(ctx android.BaseContext) ([]string) {
    var dirs []string

    sdkVersion := ctx.AConfig().PlatformSdkVersionInt()

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3326") ) {
        if (sdkVersion >= 30 ) {
			dirs = append(dirs, "hardware/rockchip/libgralloc/bifrost")
			dirs = append(dirs, "hardware/rockchip/libgralloc/bifrost/src")
        }
    }

    return dirs
}

