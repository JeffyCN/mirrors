package rkaiq_tool_server_defaults

import (
    "android/soong/android"
    "android/soong/cc"
    "fmt"
    "os"
)

func init() {
    android.RegisterModuleType("cc_rkaiq_tool_server_binary", RkaiqToolServerInitModule)
}

func RkaiqToolServerInitModule() android.Module {
    module := cc.BinaryFactory()
    android.AddLoadHook(module, RkaiqToolServerDefaults)
    return module
}

func RkaiqToolServerDefaults(ctx android.LoadHookContext) {
    type props struct {
        Target struct {
            Android struct {
                Enabled *bool
            }
        }
    }

    p := &props{}
    p.Target.Android.Enabled = new(bool)
    board := ctx.Config().Getenv("TARGET_BOARD_PLATFORM")
    if board == "rk3562" {
        fmt.Fprintf(os.Stderr, "%s WILL COMPILE rkaiq_tool_server binary\n", board)
        *p.Target.Android.Enabled = true
    } else {
        fmt.Fprintf(os.Stderr, "%s WILL NOT COMPILE rkaiq_tool_server binary\n", board)
        *p.Target.Android.Enabled = false
    }
    ctx.AppendProperties(p)
}

