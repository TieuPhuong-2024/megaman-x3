//! Movement numbers follow the C++ player, except jump sign.
//! Walls and the viewport use +Y up. The C++ jump wrote `velocity.y = -400`
//! while treating that axis as screen-down, so a literal port falls through the floor.
//! Jump here is +Y (up). Gravity is negative.

pub const WINDOW_W: u32 = 720;
pub const WINDOW_H: u32 = 480;
pub const CAMERA_PLAYER_SCREEN_Y: f32 = 0.1;
pub const FPS: f32 = 60.0;
pub const FRAME_MS: f32 = 1000.0 / FPS;

pub const MAP_SCALE: f32 = 2.0;

pub const RUN_SPEED: f32 = 200.0;
pub const AIR_SPEED: f32 = 150.0;
pub const JUMP_VELOCITY: f32 = 400.0;
pub const GRAVITY: f32 = -1000.0;

pub const DASH_SPEED: f32 = 460.0;
pub const DASH_TIME: f32 = 0.22;

pub const CLING_SLIDE: f32 = -70.0;
pub const KICK_VX: f32 = 240.0;
pub const KICK_VY: f32 = 360.0;
pub const KICK_TIME: f32 = 0.16;
pub const LAND_TIME: f32 = 0.10;

pub const SHOT_SPEED: f32 = 520.0;
pub const SHOT_COOLDOWN: f32 = 0.16;
pub const SHOT_LIFE: f32 = 0.65;
pub const SHOT_W: f32 = 10.0;
pub const SHOT_H: f32 = 6.0;

pub const HIT_W: f32 = 20.0;
pub const HIT_H: f32 = 36.0;

pub const SPAWN_X: f32 = 100.0;
pub const SPAWN_Y: f32 = 200.0;
