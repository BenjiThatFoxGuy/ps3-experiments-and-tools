#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <ppu-types.h>
#include <sys/lv2_syscall.h>
#include <sys/prx_module.h>
#include <sys/timer.h>
#include "state.h"

extern int sys_ppu_thread_create(u64 *, void (*)(u64), u64, int, size_t, u64, const char *);
extern int32_t vshtask_A02D46E7(int32_t, const char *);
extern uint32_t vshmain_EB757101(void);

#define PROBE_PRIO  3000   /* webMAN/ps3mon use 3000 */
#define PROBE_STACK 0x2000 /* 8 KiB - ps3mon uses 1500 */
#define JOINABLE    1

static u64 worker_id;
static volatile int stop_requested;
static int started;

/* ---- Cobra disc state ---- */

static int read_disc(struct sample *s)
{
    lv2syscall4(8, 0x7020, (u64)(uintptr_t)&s->real,
                (u64)(uintptr_t)&s->effective, (u64)(uintptr_t)&s->synthetic);
    return_to_user_prog(int);
}

/* ---- System Manager events (power, reset, thermal) ---- */

/*
 * sys_sm_get_ext_event2 (syscall 378).
 * param4=1: VSH's own poll mode ("get queuing events").
 * Returns event type in *v1: 3=power press, 4=power release,
 * 5=reset press, 6=reset release, 7=thermal alert, 8=thermal cleared.
 *
 * WARNING: This may CONSUME the event. If VSH stops responding to power
 * button while this probe is loaded, that confirms consume semantics
 * and we need to remove this call. Unload the probe to restore normal
 * power button handling.
 *
 * The eject button does NOT appear in this event stream. Eject goes
 * from Syscon directly to the BD controller; the only observable
 * effect from GameOS is a disc state change (already covered above).
 */
static int poll_sm_event(struct sm_event *ev)
{
    ev->type = 0;
    ev->param = 0;
    ev->extra = 0;
    {
        lv2syscall4(378, (u64)(uintptr_t)&ev->type,
                    (u64)(uintptr_t)&ev->param,
                    (u64)(uintptr_t)&ev->extra, 1);
        int ret = (int)p1;
        ev->valid = (ret == 0 && ev->type != 0);
        return ret;
    }
}

/* ---- Toast helpers ---- */

static void toast(const char *msg)
{
    vshtask_A02D46E7(0, msg);
}

static void show_sample(struct sample s, int first)
{
    char msg[140];
    const char *mode = s.mode == 0 ? "XMB" : "non-XMB";
    const char *label = first ? "Probe ready" : "State changed";
    if (s.status)
        snprintf(msg, sizeof msg, "%s: %s (mode %u)\nCobra disc: err 0x%08X",
                 label, mode, (unsigned)s.mode, (unsigned)s.status);
    else
        snprintf(msg, sizeof msg, "%s: %s (mode %u)\nDisc R:%08X E:%08X F:%08X",
                 label, mode, (unsigned)s.mode,
                 (unsigned)s.real, (unsigned)s.effective, (unsigned)s.synthetic);
    toast(msg);
}

static void show_sm_event(struct sm_event ev)
{
    char msg[100];
    snprintf(msg, sizeof msg, "SM event: %s (type %u)\nparam=%08X extra=%08X",
             sm_event_name(ev.type), (unsigned)ev.type,
             (unsigned)ev.param, (unsigned)ev.extra);
    toast(msg);
}

/* ---- Main loop ---- */

static void run_probe(u64 arg)
{
    struct sample last = {0};
    int first = 1;
    (void)arg;
    while (!stop_requested) {
        struct sample now = {0};
        now.mode = vshmain_EB757101();
        now.status = read_disc(&now);
        if (first || sample_changed(last, now))
            show_sample(now, first);
        last = now;
        first = 0;

        struct sm_event ev;
        poll_sm_event(&ev);
        if (ev.valid)
            show_sm_event(ev);

        sys_timer_usleep(100000);
    }
    { lv2syscall1(41, 0); }
}

/* ---- PRX lifecycle ---- */

int module_start(size_t args, void *argp)
{
    (void)args;
    (void)argp;
    stop_requested = 0;

    /* Immediate toast so we know the module loaded even if thread fails */
    toast("Panel probe: loading...");

    int ret = sys_ppu_thread_create(&worker_id, run_probe, 0,
                                    PROBE_PRIO, PROBE_STACK,
                                    JOINABLE, "panel_probe");
    if (ret != 0) {
        char msg[80];
        snprintf(msg, sizeof msg, "Panel probe: thread err 0x%08X", (unsigned)ret);
        toast(msg);
        return SYS_PRX_NO_RESIDENT;
    }
    started = 1;
    return SYS_PRX_RESIDENT;
}

int module_stop(size_t args, void *argp)
{
    u64 exit_status = 0;
    (void)args;
    (void)argp;
    if (!started)
        return 0;
    stop_requested = 1;
    lv2syscall2(44, worker_id, (u64)(uintptr_t)&exit_status);
    started = 0;
    return 0;
}
