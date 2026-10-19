# Mega Man X3 (Rust)

2D platformer ported from the old DirectX 9 / C++ engine. The window and keyboard come from SDL2. Sprites are drawn with wgpu.

Assets (atlas, tile map, landing frames) live in `assets/`.

## Requirements

- Rust stable (`rust-toolchain.toml`)
- A Windows GNU or MSVC host. This repo is set up for `x86_64-pc-windows-gnu`.
- On the GNU host, `rustc` calls `dlltool`, which needs an assembler named `as.exe` on `PATH`. A small copy is in `third_party/as-bin`. `scripts/cargo.ps1` puts that directory on `PATH` before it runs Cargo.
- SDL2 is vendored for MinGW in `third_party/SDL2-2.32.8`. The build script copies `SDL2.dll` next to the executable. DirectX SDK is not required.

## Run

```powershell
powershell -File scripts/cargo.ps1 run -p megaman_x3
```

Or, if `as.exe` is already on `PATH`:

```powershell
cargo run -p megaman_x3
```

Tests:

```powershell
cargo test -p engine
```

In Cursor, the default build task runs `scripts/cargo.ps1 build`.

## Controls

| Key | Action |
| --- | --- |
| Left / Right | Move |
| X or Space | Jump, and shoot while held |
| C | Shoot without jumping |
| Shift | Dash |
| Hold Left or Right into a wall while falling | Wall cling. Jump kicks off the wall |
| F1 | Toggle collision boxes |
| Escape | Quit |

World coordinates use +Y up. Jump velocity is positive. The old C++ code stored a negative jump speed on a Y-up wall system, which disagreed with its own comment, so that sign was not copied.

## Layout

- `crates/megaman_x3` — SDL event loop and fixed 60 Hz timestep
- `crates/engine` — sprite batch, TMX / atlas loading, swept AABB, player states, shots, debug draw, beep audio
- `assets` — `XMan.png`, `XMan.xml`, `MAP_1.tmx`, `MAP_1.png`
