#ifndef PANEL_PROBE_STATE_H
#define PANEL_PROBE_STATE_H
#include <stdint.h>

struct sample {
    uint32_t mode;
    int status;
    uint32_t real, effective, synthetic;
};

struct sm_event {
    uint64_t type;
    uint64_t param;
    uint64_t extra;
    int valid;
};

static inline int sample_changed(struct sample a, struct sample b)
{
    if (a.mode != b.mode || a.status != b.status)
        return 1;
    return !a.status && (a.real != b.real || a.effective != b.effective || a.synthetic != b.synthetic);
}

static inline const char *sm_event_name(uint64_t type)
{
    switch (type) {
    case 3:  return "POWER pressed";
    case 4:  return "POWER released";
    case 5:  return "RESET pressed";
    case 6:  return "RESET released";
    case 7:  return "THERMAL alert";
    case 8:  return "THERMAL cleared";
    default: return "UNKNOWN";
    }
}
#endif
