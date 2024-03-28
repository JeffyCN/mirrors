package rkaiqdefaults

import (
	"android/soong/android"
	"android/soong/cc"
    "fmt"   // required if we want to print usefull messages on the console
    "os"
    "strings"
)

type Ex_srcs struct {
    Srcs []string
}

type Ex_multilibType struct {
    Lib32 Ex_srcs
    Lib64 Ex_srcs
}

func rkaiqFlags(ctx android.BaseContext) []string {
    var cflags []string
    //fmt.Fprintf(os.Stderr, "%s\n", "deviceFlags called") //example prints
    //fmt.Fprintf(os.Stderr, "%s\n", ctx.AConfig())
    board := ctx.Config().Getenv("TARGET_BOARD_PLATFORM") //currently using this for reference but you can refer to README for other variables.
    fmt.Fprintf(os.Stderr, ">>>>>>>>>>>>>>>>>>>>> %s\n", board)
    if board == "rv1126" {
       cflags = append(cflags, "-DISP_HW_V20")
    }
    if board == "rk356x" {
       cflags = append(cflags, "-DISP_HW_V21")
    }
    if board == "rk3588" {
       cflags = append(cflags, "-DISP_HW_V30")
    }
    if board == "rv1106" {
        cflags = append(cflags, "-DISP_HW_V32")
    }
    if board == "rk3562" {
        cflags = append(cflags, "-DISP_HW_V32_LITE")
    }
    if board == "rk3576" {
        cflags = append(cflags, "-DISP_HW_V39")
    }
    return cflags
}

func rkaiq_get_aiq_version(ctx android.BaseContext) string {
    board := ctx.Config().Getenv("TARGET_BOARD_PLATFORM")
    // fmt.Fprintf(os.Stderr, ">>>>>>>>>>>>>>>>>>>>> %s\n", board)
    return board;
}

func rkaiqDefaults(ctx android.LoadHookContext) {
    type props struct {
        Target struct {
            Android struct {
                Cflags  []string
                Enabled *bool
            }
            Host struct {
                Enabled *bool
            }
            Not_windows struct {
                Cflags []string
            }
        }
        Cflags []string
    }
    p := &props{}

    //p.Cflags = deviceFlags(ctx)

    soc := rkaiq_get_aiq_version(ctx)
    macros_map := rkaiq_macro_switch(soc)
    p.Target.Android.Cflags = rkaiq_getAlgosMacros(macros_map)
    cflags := rkaiqFlags(ctx)
    p.Target.Android.Cflags = append(p.Target.Android.Cflags, cflags...)
    ctx.AppendProperties(p)
}

func rkaiqLibraryShared(ctx android.LoadHookContext) {
    type props struct {
        Target struct {
            Android struct {
                Srcs  []string
                Static_libs []string
                Enabled *bool
            }
            Host struct {
                Enabled *bool
            }
            Not_windows struct {
            }
        }
        Srcs []string
    }
    p := &props{}
    soc := rkaiq_get_aiq_version(ctx)
    macros_map := rkaiq_macro_switch(soc)
    p.Target.Android.Srcs = rkaiq_getSrcsFiles(soc, macros_map)
    p.Target.Android.Static_libs = rkaiq_getAlgosLib(macros_map)
    ctx.AppendProperties(p)
}

func rkaiqLibraryStatic(ctx android.LoadHookContext) {
    type props struct {
        Srcs []string
    }
    p := &props{}
    soc := rkaiq_get_aiq_version(ctx)
    macros_map := rkaiq_macro_switch(soc)
    p.Srcs = rkaiq_getStaticLibSrcs(ctx.ModuleName(), macros_map)
    ctx.AppendProperties(p)
}

func rkaiqPrebuiltStaticLibrary (ctx android.LoadHookContext) {
    type props struct {
        Compile_multilib *string
        Multilib Ex_multilibType
    }
    p := &props{}

    var compile_multilib string = ""
    target_arch := ctx.AConfig().DevicePrimaryArchType().String()
    if (strings.EqualFold(target_arch,"arm64")) {
        compile_multilib = "both"
    } else {
        compile_multilib = "32"
    }
    p.Compile_multilib = &compile_multilib

    // get static lib
    var prefix64 string = ""
    var prefix32 string = ""
    var srcs []string
    var module_name string = ctx.ModuleName()[9:] + ".a"
    os := "android/"
    soc := rkaiq_get_aiq_version(ctx)
    soc +="/"
    prefix64 = os + soc
    prefix32 = os + soc
    prefix64 += "aarch64/clang/"
    prefix32 += "arm/clang/"
    p.Multilib.Lib32.Srcs = append(srcs, prefix32 + module_name)
    p.Multilib.Lib64.Srcs = append(srcs, prefix64 + module_name)
    //fmt.Println("arch.arm.srcs:", p.Multilib.Lib32.Srcs)
    //fmt.Println("arch.arm64.srcs:", p.Multilib.Lib64.Srcs)

    ctx.AppendProperties(p)
}

/* This is called from the boot strap process
 * We register a module here
*/
func init() {
    android.RegisterModuleType("rkaiq_defaults", rkaiqDefaultFactory)
    android.RegisterModuleType("cc_rkaiq_library_shared", cc_rkaiq_library_sharedFactory)
    android.RegisterModuleType("cc_rkaiq_library_static", cc_rkaiq_library_staticFactory)
    android.RegisterModuleType("cc_rkaiq_prebuilt_library_static", cc_rkaiq_prebuilt_library_staticFactory)
}

func rkaiqDefaultFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, rkaiqDefaults)
    return module
}

func cc_rkaiq_library_sharedFactory() android.Module {
    module := cc.LibrarySharedFactory()
    android.AddLoadHook(module, rkaiqLibraryShared)
    return module
}

func cc_rkaiq_library_staticFactory() android.Module {
    module := cc.LibraryStaticFactory()
    android.AddLoadHook(module, rkaiqLibraryStatic)
    return module
}

func cc_rkaiq_prebuilt_library_staticFactory() (android.Module) {
    // Register rockchip_prebuilt_libs factory as PrebuiltSharedLibraryFactory
    module := cc.PrebuiltSharedLibraryFactory()

    // Add Hook for PrebuiltSharedLibraryFactory
    android.AddLoadHook(module, rkaiqPrebuiltStaticLibrary)
    return module
}