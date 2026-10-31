<h1 align=center>A Kingdom for Keflings — Nintendo Switch port</h1>

A native Nintendo Switch port of the Xbox 360 (Xbox Live Arcade) version of **A Kingdom for Keflings** (title
`584108DB`, with title update 2 and the two Kingdom Packs).

It is not an emulator. The PowerPC code of the game's `default.xex` is statically recompiled to C++ with
[ReXGlue](https://github.com/rexglue/rexglue-sdk) and compiled for the Switch, and the game is drawn by Renut's
Direct3D-level renderer ([reNut](https://github.com/masterspike52/reNut) by masterspike52, as banjo-nx carries it)
directly on the Switch's own graphics API, deko3d.

> [!NOTE]
> You need your own copy of the game: its Xbox Live Arcade package from an Xbox 360
> (`Content/0000000000000000/584108DB/000D0000/`), title update 2 (`000B0000/`, or the `TU_...` file of the
> console's `Cache` folder) and, if you have them, the Kingdom Pack packages (`00000002/`). No game files and no
> shaders made from them are in this repository or its releases.

## How to install

Use the **[keflings-nx installer](https://chanseyisthebest.github.io/keflings-nx-installer/)**
([its source](https://github.com/ChanseyIsTheBest/keflings-nx-installer)): a web page that takes your packages, makes
the game's shaders for the Switch from them in your browser (nothing is uploaded) and gives you a zip to extract to
the root of your SD card. Start `switch/keflings-nx/keflings-nx.nro` from the Homebrew Menu with a title takeover
(hold R while starting a game).

## Settings

`switch/keflings-nx/renut.toml` is written at the first start and holds only the player's options:

| Setting | Default | |
|---|---|---|
| `keflings_language` | `""` | the game's language: en, ja, de, fr, es, it, ko or zh; empty = the Switch's system language |
| `input_xbox_layout` | `true` | face buttons by position, as on an Xbox pad; `false` = by letter |
| `input_joycon_sideways` | `true` | single Joy-Cons held sideways, one per player |
| `input_joycon_sideways_stick` | `true` | the single Joy-Con's stick turned for the sideways grip |
| `keflings_renut_msaa_samples` | `2` | MSAA of the 3D image: 2 = 2x, the game's own; 1 = none; 4 = 4x |
| `keflings_user_picture` | `true` | the Achievement dialog's picture is a Switch user's profile picture |
| `keflings_user_picture_user` | `1` | whose: the user number in the HOME Menu's order (1 = the first user) |
| `[input_p1]` | | button mapping: the Switch buttons that press each Xbox button (keep this section last) |

Everything else is built into the NRO (`app/keflings_switch_defaults.toml`): the picture at 1280x720 handheld and
1920x1080 docked, a frame limiter at 60 fps, the game on cores 0-2 (the fourth is the system's), and no logging. Any
setting written in `renut.toml` overrides the built-in one.

## What is in it

- Title update 2 (Choose your Kingdom, the Kingdom Pack kingdoms); its avatar giant is left out (no avatar data).
- The Kingdom Packs, installed from `dlc/` at the start.
- 346 machine-optimised copies of the game's hottest functions and 16 hand-written natives, each checked against the
  recompiled original (0 differences in 258 million compared calls), and a profile-guided, link-time-optimised build.
- The Achievement dialog's picture from the console user's profile picture.

## Building

The Switch build cross-compiles with devkitA64 (`tools/build_switch.bat`; the release one is PGO + LTO,
`tools/build_switch_pgo.bat`); the PC build (`tools/build_pc.bat`) uses reNut's renderer on Vulkan. Both need the code
generated from your own `default.xex` (ReXGlue's codegen with `app/config/`, then `tools/post_codegen.py`), which is
not in this repository. Notes: `docs/`.

## Credits

- [ReXGlue](https://github.com/rexglue/rexglue-sdk) (the recompiler and runtime).
- reNut's renderer by masterspike52 (its author permits its reuse in these ports), through banjo-nx (this port's
  base).
- sa2-nx, nights-nx, peggle2-nx, bejeweledblitz-nx and the bejeweled3-nx / zuma ports (the built-in settings, the
  language setting, the controls, the gamer pictures, the frame pacing, the exit fix), NetherSX2_nx and battd_nx.

GPL-3.0 where the sources are (LICENSE). A Kingdom for Keflings is NinjaBee's; this project is not affiliated with
NinjaBee or Microsoft.
