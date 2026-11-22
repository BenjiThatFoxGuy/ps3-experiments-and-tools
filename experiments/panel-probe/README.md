# Panel probe

A manually loaded VSH `.sprx` for the CECH-2504B test target on Evilnat 4.93 PEX / Cobra 8.5. It toasts native PS3 notifications for:

- **VSH running mode** changes (`XMB` vs `non-XMB`) and **Cobra disc state** (real/effective/synthetic disc type values) on every change.
- **System Manager events** via `sys_sm_get_ext_event2` (syscall 378, poll mode): power button press/release, reset press/release, thermal alerts. Each fires a separate toast with the raw type, param and extra values.

It does not hook input, exit games, eject discs, write logs, or require a serial connection.

## Important limitations

- **Eject button** does not appear in the System Manager event stream. Eject goes from Syscon directly to the BD controller. The only observable effect is a disc state change, which IS toasted. An empty-drive eject press produces no observable event from GameOS.
- **Power button event consumption** is unverified. `sys_sm_get_ext_event2` with param=1 is the same call VSH uses. If the probe consumes the event before VSH sees it, the power button may stop triggering shutdown. If that happens, **unload the probe** and the power button will work again. This is exactly what we're testing.

## Load and unload

1. Download the `panel-probe` artifact from a successful Actions run and extract `panel-probe.sprx`.
2. Upload it as `/dev_hdd0/tmp/panel-probe.sprx`, then open `http://PS3-IP/loadprx_ps3?prx=/dev_hdd0/tmp/panel-probe.sprx` on the LAN. webMAN MOD with plugin loading enabled and a free Cobra VSH slot is required. No boot entry or reboot is needed.
3. Look for `Probe ready`. Then test:
   - **Power button**: short press, watch for `SM event: POWER pressed` toast.
   - **Eject with disc/ISO**: watch for a `State changed` toast with new R/E/F values.
   - **Eject with empty drive**: watch for any toast at all (likely none).
   - **Game launch/exit**: watch for mode changing between 0 (XMB) and nonzero.
4. Unload with `http://PS3-IP/unloadprx.ps3?prx=/dev_hdd0/tmp/panel-probe.sprx`. Or use `/vshplugin.ps3mapi` to load/unload by VSH slot.

## References

- [webMAN MOD VSH cooperation mode](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshmain.h) and [native notification import](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshtask.h)
- [Evilnat Cobra storage state](https://github.com/xXEvilnatXx/Cobra-PS3/blob/main/8.5/4.93/EVILNAT/PEX/SRC/stage2/storage_ext.c)
- [VSH's System Manager event consumer](https://www.psdevwiki.com/ps3/Talk:Thermal)
- [LV2 syscall 378 documentation](https://www.psdevwiki.com/ps3/LV2_Functions_and_Syscalls)
