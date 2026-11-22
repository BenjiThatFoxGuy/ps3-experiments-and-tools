#ifndef PANEL_PROBE_STATE_H
#define PANEL_PROBE_STATE_H
#include <stdint.h>

struct sample {
    uint32_t mode;
    int status;
    uint32_t real, effective, synthetic;
};

static inline int sample_changed(struct sample a, struct sample b)
{
    if (a.mode != b.mode || a.status != b.status)
        return 1;
    return !a.status && (a.real != b.real || a.effective != b.effective || a.synthetic != b.synthetic);
}
#endif
