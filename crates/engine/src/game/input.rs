#[derive(Clone, Copy, Debug, Default)]
pub struct InputState {
    pub left: bool,
    pub right: bool,
    pub jump: bool,
    pub shoot: bool,
    pub dash: bool,
    pub debug_toggle: bool,
    pub quit: bool,
}
