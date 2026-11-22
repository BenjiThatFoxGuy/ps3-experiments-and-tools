# Panel probe

> **STATUS: BROKEN** - The `.sprx` builds and passes CI but silently fails to load on hardware (CECH-2504B, Evilnat 4.93 PEX, Cobra 8.5, webMAN 1.47.48n). No toast appears, no error from webMAN, plugin does not appear in VSH slots. Root cause unknown - likely a PS3DK toolchain or NID resolution issue at runtime. Pinned for investigation.

A manually loaded VSH `.sprx` for the CECH-2504B test target on Evilnat 4.93 PEX / Cobra 8.5. It is intended to toast native PS3 notifications for:

- **VSH running mode** changes (`XMB` vs `non-XMB`) and **Cobra disc state** (real/effective/synthetic disc type values) on every change.
- **System Manager events** via `sys_sm_get_ext_event2` (syscall 378, poll mode): power button press/release, reset press/release, thermal alerts. Each fires a separate toast with the raw type, param and extra values.

It does not hook input, exit games, eject discs, write logs, or require a serial connection.

## Known issues

- The plugin does not load. webMAN's `/loadprx.ps3` and `/vshplugin.ps3mapi` both accept the command without error, but the module never appears in a VSH slot and no notification is shown. This suggests the Cobra VSH loader rejects the PRX before `module_start` runs, or `module_start` crashes before the first `vshtask_A02D46E7` call.
- Possible causes to investigate: PS3DK's OPD format vs what Cobra expects, missing or mismatched import NIDs at runtime, PRX module-info layout issues, or memory container constraints.

## Important limitations (once loading is fixed)

- **Eject button** does not appear in the System Manager event stream. Eject goes from Syscon directly to the BD controller. The only observable effect is a disc state change, which IS toasted. An empty-drive eject press produces no observable event from GameOS.
- **Power button event consumption** is unverified. `sys_sm_get_ext_event2` with param=1 is the same call VSH uses. If the probe consumes the event before VSH sees it, the power button may stop triggering shutdown. If that happens, **unload the probe** and the power button will work again. This is exactly what we're testing.

## Load and unload (once working)

1. Download the `panel-probe` artifact from a successful Actions run and extract `panel-probe.sprx`.
2. Upload it as `/dev_hdd0/plugins/panel-probe.sprx`.
3. Load: `http://192.168.0.103/loadprx.ps3?prx=/dev_hdd0/plugins/panel-probe.sprx`
4. Look for `Probe ready`. Then test:
   - **Power button**: short press, watch for `SM event: POWER pressed` toast.
   - **Eject with disc/ISO**: watch for a `State changed` toast with new R/E/F values.
   - **Eject with empty drive**: watch for any toast at all (likely none).
   - **Game launch/exit**: watch for mode changing between 0 (XMB) and nonzero.
5. Unload: `http://192.168.0.103/unloadprx.ps3?prx=/dev_hdd0/plugins/panel-probe.sprx`
6. Plugin manager: `http://192.168.0.103/vshplugin.ps3mapi`

## References

- [webMAN MOD VSH cooperation mode](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshmain.h) and [native notification import](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshtask.h)
- [Evilnat Cobra storage state](https://github.com/xXEvilnatXx/Cobra-PS3/blob/main/8.5/4.93/EVILNAT/PEX/SRC/stage2/storage_ext.c)
- [VSH's System Manager event consumer](https://www.psdevwiki.com/ps3/Talk:Thermal)
- [LV2 syscall 378 documentation](https://www.psdevwiki.com/ps3/LV2_Functions_and_Syscalls)
