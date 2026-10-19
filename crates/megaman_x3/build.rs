use std::env;
use std::fs;
use std::path::PathBuf;

fn main() {
    let manifest = PathBuf::from(env::var("CARGO_MANIFEST_DIR").unwrap());
    let root = manifest.join("../..");
    let sdl_root = root.join("third_party/SDL2-2.32.8/x86_64-w64-mingw32");
    let link_dir = root.join("third_party/sdl2-link");
    fs::create_dir_all(&link_dir).expect("sdl link dir");
    fs::copy(
        sdl_root.join("lib/libSDL2.dll.a"),
        link_dir.join("libSDL2.dll.a"),
    )
    .expect("copy SDL2 import library");
    let dll_src = link_dir.join("SDL2.dll");
    if !dll_src.exists() {
        fs::copy(sdl_root.join("bin/SDL2.dll"), &dll_src).expect("copy SDL2.dll");
    }
    println!("cargo:rustc-link-search=native={}", link_dir.display());

    let profile = env::var("PROFILE").unwrap_or_else(|_| "debug".into());
    let target_dir = env::var_os("CARGO_TARGET_DIR")
        .map(PathBuf::from)
        .unwrap_or_else(|| root.join("target"));
    let dest_dir = target_dir.join(&profile);
    let _ = fs::create_dir_all(&dest_dir);
    let _ = fs::copy(&dll_src, dest_dir.join("SDL2.dll"));
    println!("cargo:rerun-if-changed=build.rs");
}
