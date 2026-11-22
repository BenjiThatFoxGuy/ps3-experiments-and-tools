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

static u64 worker_id;
static int stop_requested;
static int started;

static int read_disc(struct sample *s)
{
    /* Cobra GET_DISC_TYPE only. No drive commands or event queue reads. */
    lv2syscall4(8, 0x7020, (u64)(uintptr_t)&s->real,
                (u64)(uintptr_t)&s->effective, (u64)(uintptr_t)&s->synthetic);
    return_to_user_prog(int);
}

static void show_sample(struct sample s, int first)
{
    char msg[120];
    const char *mode = s.mode == 0 ? "XMB" : "non-XMB";
    const char *label = first ? "Probe ready" : "Probe state";
    if (s.status)
        snprintf(msg, sizeof msg, "%s: %s (mode %u)\nCobra disc query: 0x%08X", label,
                 mode, (unsigned)s.mode, (unsigned)s.status);
    else
        snprintf(msg, sizeof msg, "%s: %s (mode %u)\nDisc R:%08X E:%08X F:%08X", label,
                 mode, (unsigned)s.mode, (unsigned)s.real, (unsigned)s.effective,
                 (unsigned)s.synthetic);
    if (!__atomic_load_n(&stop_requested, __ATOMIC_ACQUIRE))
        vshtask_A02D46E7(0, msg);
}

static void run_probe(u64 arg)
{
    struct sample last = {0};
    int first = 1;
    (void)arg;
    while (!__atomic_load_n(&stop_requested, __ATOMIC_ACQUIRE)) {
        struct sample now = {0};
        now.mode = vshmain_EB757101();
        now.status = read_disc(&now);
        if (first || sample_changed(last, now))
            show_sample(now, first);
        last = now;
        first = 0;
        sys_timer_usleep(250000);
    }
    { lv2syscall1(41, 0); }
}

int module_start(size_t args, void *argp)
{
    (void)args;
    (void)argp;
    __atomic_store_n(&stop_requested, 0, __ATOMIC_RELEASE);
    started = sys_ppu_thread_create(&worker_id, run_probe, 0, 2000, 16384, 1, "panel_probe") == 0;
    if (!started)
        vshtask_A02D46E7(0, "Panel probe: worker start failed");
    return started ? SYS_PRX_RESIDENT : SYS_PRX_NO_RESIDENT;
}

int module_stop(size_t args, void *argp)
{
    u64 exit_status = 0;
    (void)args;
    (void)argp;
    if (!started)
        return 0;
    __atomic_store_n(&stop_requested, 1, __ATOMIC_RELEASE);
    lv2syscall2(44, worker_id, (u64)(uintptr_t)&exit_status);
    int result = (int)p1;
    if (!result)
        started = 0;
    return result;
}
