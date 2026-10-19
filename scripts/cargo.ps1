# Prepend a GNU assembler so rustc's dlltool can build windows-sys on x86_64-pc-windows-gnu.
$root = Split-Path -Parent $PSScriptRoot
$asBin = Join-Path $root "third_party\as-bin"
$toolchain = Join-Path $env:USERPROFILE ".rustup\toolchains\stable-x86_64-pc-windows-gnu\lib\rustlib\x86_64-pc-windows-gnu\bin\self-contained"
if (Test-Path $asBin) {
    $env:PATH = "$asBin;$env:PATH"
}
if (Test-Path $toolchain) {
    $env:PATH = "$toolchain;$env:PATH"
}
cargo @args
exit $LASTEXITCODE
