package rkaiqdefaults

import (
	"android/soong/android"
	"android/soong/cc"
    "fmt"   // required if we want to print usefull messages on the console
    "os"
)

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
    return cflags
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
    p.Target.Android.Cflags = rkaiqFlags(ctx)
    ctx.AppendProperties(p)
}

/* This is called from the boot strap process
 * We register a module here
*/
func init() {
    android.RegisterModuleType("rkaiq_defaults", rkaiqDefaultFactory)
}

func rkaiqDefaultFactory() android.Module {
    module := cc.DefaultsFactory()
    android.AddLoadHook(module, rkaiqDefaults)
    return module
}
