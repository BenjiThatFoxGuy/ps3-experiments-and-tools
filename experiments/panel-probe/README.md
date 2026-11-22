# Panel probe

A manually loaded VSH `.sprx` for the CECH-2504B test target on Evilnat 4.93 PEX / Cobra 8.5. It shows native PS3 notifications when it starts and when the VSH running mode or Cobra disc status changes. It does not hook input, exit games, eject discs, write logs, or require a serial connection.

**This build does not detect raw front-panel button presses.** Disc state is a control measurement: physical eject and software unmount can both change it, and an empty-drive press need not change it at all. The VSH System Manager event read is not used because VSH itself uses that path for shutdown and thermal events, and a non-consuming observer has not been verified.

## Load and unload

1. Download the `panel-probe` artifact from a successful Actions run and extract `panel-probe.sprx`.
2. Upload it as `/dev_hdd0/tmp/panel-probe.sprx`, then open `http://PS3-IP/loadprx_ps3?prx=/dev_hdd0/tmp/panel-probe.sprx` on the LAN. webMAN MOD with plugin loading enabled and a free Cobra VSH slot is required. No boot entry or reboot is needed.
3. Look for `Probe ready`. Launch and exit a game to check `mode 0` (XMB) versus nonzero (non-XMB). Compare ISO mount/unmount, a physical disc eject and an empty-drive press. `R`, `E`, and `F` are raw Cobra real, effective and synthetic disc values; `F` is not proof that an ISO is mounted.
4. Unload with `http://PS3-IP/unloadprx.ps3?prx=/dev_hdd0/tmp/panel-probe.sprx`. If your webMAN edition does not offer those endpoints, use its `/vshplugin.ps3mapi` page to load into or unload from a free VSH slot.

## References

- [webMAN MOD VSH cooperation mode](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshmain.h) and [native notification import](https://github.com/aldostools/webMAN-MOD/blob/master/vsh/vshtask.h)
- [Evilnat Cobra storage state](https://github.com/xXEvilnatXx/Cobra-PS3/blob/main/8.5/4.93/EVILNAT/PEX/SRC/stage2/storage_ext.c)
- [VSH's System Manager event consumer](https://www.psdevwiki.com/ps3/Talk:Thermal)
