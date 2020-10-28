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
    }
    p := &props{}
    p.Cflags = globalDefaults(ctx)
    ctx.AppendProperties(p)
}
//条件编译主要修改函数
func globalDefaults(ctx android.BaseContext) ([]string) {
	var cppflags []string
	//该打印输出为: TARGET_PRODUCT:rk3328 fmt.Println("TARGET_PRODUCT:",ctx.AConfig().Getenv("TARGET_PRODUCT")) //通过 strings.EqualFold 比较字符串，可参考go语言字符串对比
	if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3368") ) {
	//添加 DEBUG 宏定义
        cppflags = append(cppflags,"-DRK3368=1")
	}

    if (strings.EqualFold(ctx.AConfig().Getenv("TARGET_BOARD_PLATFORM"),"rk3326") ) {
        cppflags = append(cppflags,"-DUSE_GRALLOC_4")
    }

	//将需要区分的环境变量在此区域添加 //....
	return cppflags
}
