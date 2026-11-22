#include <assert.h>
#include <string.h>
#include "state.h"

int main(void)
{
    /* sample_changed tests */
    struct sample xmb = {0, 0, 0, 0, 0};
    struct sample game = {1, 0, 0, 0, 0};
    struct sample mounted = {1, 0, 0, 0x123, 0x123};
    struct sample failed = {1, -1, 0, 0, 0};
    struct sample failed_dirty = {1, -1, 4, 5, 6};
    assert(!sample_changed(xmb, xmb));
    assert(sample_changed(xmb, game));
    assert(sample_changed(game, mounted));
    assert(sample_changed(mounted, failed));
    assert(!sample_changed(failed, failed_dirty));
    assert(sample_changed(failed, game));

    /* sm_event_name tests */
    assert(strcmp(sm_event_name(3), "POWER pressed") == 0);
    assert(strcmp(sm_event_name(4), "POWER released") == 0);
    assert(strcmp(sm_event_name(5), "RESET pressed") == 0);
    assert(strcmp(sm_event_name(7), "THERMAL alert") == 0);
    assert(strcmp(sm_event_name(99), "UNKNOWN") == 0);
}
