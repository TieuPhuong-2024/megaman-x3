use glam::Vec2;

use crate::assets::{frames_with_prefix, Atlas, Frame};
use crate::physics::{overlaps, separate, swept_aabb, Aabb};
use crate::render::SpriteDraw;

use super::audio::{Audio, SoundId};
use super::debug_session::{log_integrate, log_player_tick};
use super::input::InputState;
use super::tuning::*;

#[derive(Clone, Copy, Debug, PartialEq, Eq)]
enum Face {
    Left,
    Right,
}

#[derive(Clone, Copy, Debug)]
enum Body {
    Stand,
    Run,
    Jump,
    Fall,
    Dash { left: f32 },
    Cling,
    Kick { left: f32 },
    Land { left: f32 },
}

impl Body {
    fn label(self) -> &'static str {
        match self {
            Body::Stand => "stand",
            Body::Run => "run",
            Body::Jump => "jump",
            Body::Fall => "fall",
            Body::Dash { .. } => "dash",
            Body::Cling => "cling",
            Body::Kick { .. } => "kick",
            Body::Land { .. } => "land",
        }
    }
}

pub struct Shot {
    pub pos: Vec2,
    pub vel: Vec2,
    pub life: f32,
}

pub struct Player {
    pub pos: Vec2,
    pub vel: Vec2,
    face: Face,
    body: Body,
    shooting: bool,
    anim_key: String,
    anim_frames: Vec<Frame>,
    frame: usize,
    frame_time: f32,
    anim_clock: f32,
    anim_loop: bool,
    shot_cooldown: f32,
    dash_was_down: bool,
    pub shots: Vec<Shot>,
    grounded: bool,
    wall_normal_x: f32,
}

impl Player {
    pub fn new(atlas: &Atlas) -> Self {
        let mut player = Self {
            pos: Vec2::new(SPAWN_X, SPAWN_Y),
            vel: Vec2::ZERO,
            face: Face::Right,
            body: Body::Fall,
            shooting: false,
            anim_key: String::new(),
            anim_frames: Vec::new(),
            frame: 0,
            frame_time: 0.12,
            anim_clock: 0.0,
            anim_loop: true,
            shot_cooldown: 0.0,
            dash_was_down: false,
            shots: Vec::new(),
            grounded: false,
            wall_normal_x: 0.0,
        };
        player.set_anim(atlas, "falling", 0.12, true);
        player
    }

    pub fn bounds(&self) -> Aabb {
        Aabb::from_center(self.pos, HIT_W, HIT_H)
    }

    pub fn update(
        &mut self,
        dt_ms: f32,
        input: &InputState,
        walls: &[Aabb],
        atlas: &Atlas,
        land_frames: &[Frame],
        audio: &Audio,
        cam_top: f32,
        view_h: f32,
    ) {
        let dt = dt_ms / 1000.0;
        self.shot_cooldown = (self.shot_cooldown - dt).max(0.0);
        self.wall_normal_x = 0.0;

        let want_shoot = input.shoot || input.jump;
        if want_shoot && self.shot_cooldown <= 0.0 {
            self.spawn_shot();
            self.shot_cooldown = SHOT_COOLDOWN;
            audio.play(SoundId::Shoot);
        }
        self.shooting = want_shoot;

        self.steer(dt, input, audio);
        self.integrate(dt, walls);
        if self.wall_normal_x == 0.0 {
            self.wall_normal_x = self.probe_wall(walls);
        }
        self.transitions(input, audio);
        self.advance_anim(dt, atlas, land_frames);
        self.update_shots(dt, walls);
        let sprite_wh = self
            .anim_frames
            .get(self.frame)
            .map(|f| (f.w as f32, f.h as f32));
        log_player_tick(
            self.debug_body_label(),
            self.pos.x,
            self.pos.y,
            self.vel.x,
            self.vel.y,
            self.grounded,
            self.anim_frames.len(),
            sprite_wh,
            cam_top,
            cam_top - view_h,
        );
    }

    fn steer(&mut self, dt: f32, input: &InputState, audio: &Audio) {
        let horizontal = |input: &InputState, speed: f32, face: &mut Face| -> f32 {
            if input.left && !input.right {
                *face = Face::Left;
                -speed
            } else if input.right && !input.left {
                *face = Face::Right;
                speed
            } else {
                0.0
            }
        };

        let dash_pressed = input.dash && !self.dash_was_down;
        match self.body {
            Body::Stand => {
                self.vel.x = horizontal(input, RUN_SPEED, &mut self.face);
                self.vel.y = 0.0;
                if dash_pressed {
                    self.start_dash();
                } else if input.jump {
                    self.start_jump(audio);
                } else if self.vel.x != 0.0 {
                    self.body = Body::Run;
                }
            }
            Body::Land { left } => {
                let left = (left - dt).max(0.0);
                self.vel.x = horizontal(input, RUN_SPEED, &mut self.face);
                self.vel.y = 0.0;
                if input.jump {
                    self.start_jump(audio);
                } else if dash_pressed {
                    self.start_dash();
                } else {
                    self.body = Body::Land { left };
                }
            }
            Body::Run => {
                self.vel.x = horizontal(input, RUN_SPEED, &mut self.face);
                self.vel.y = 0.0;
                if dash_pressed {
                    self.start_dash();
                } else if input.jump {
                    self.start_jump(audio);
                } else if self.vel.x == 0.0 {
                    self.body = Body::Stand;
                }
            }
            Body::Jump | Body::Fall => {
                self.vel.x = horizontal(input, AIR_SPEED, &mut self.face);
                self.vel.y += GRAVITY * dt;
            }
            Body::Dash { left } => {
                let left = (left - dt).max(0.0);
                let sign = if self.face == Face::Right { 1.0 } else { -1.0 };
                self.vel.x = sign * DASH_SPEED;
                self.vel.y = 0.0;
                self.body = Body::Dash { left };
            }
            Body::Cling => {
                self.vel.x = 0.0;
                self.vel.y = CLING_SLIDE;
                if input.jump {
                    let away = if self.face == Face::Left { 1.0 } else { -1.0 };
                    self.face = if away > 0.0 { Face::Right } else { Face::Left };
                    self.vel.x = away * KICK_VX;
                    self.vel.y = KICK_VY;
                    self.body = Body::Kick { left: KICK_TIME };
                    audio.play(SoundId::Jump);
                }
            }
            Body::Kick { left } => {
                let left = (left - dt).max(0.0);
                self.vel.y += GRAVITY * dt;
                self.body = Body::Kick { left };
            }
        }
        self.dash_was_down = input.dash;
    }

    fn start_jump(&mut self, audio: &Audio) {
        self.vel.y = JUMP_VELOCITY;
        self.body = Body::Jump;
        self.grounded = false;
        audio.play(SoundId::Jump);
    }

    fn probe_wall(&self, walls: &[Aabb]) -> f32 {
        let bounds = self.bounds();
        let probe = Aabb {
            left: bounds.left - 2.0,
            right: bounds.right + 2.0,
            bottom: bounds.bottom + 4.0,
            top: bounds.top - 4.0,
        };
        for wall in walls {
            if overlaps(probe, *wall) {
                let center = (bounds.left + bounds.right) * 0.5;
                let wall_center = (wall.left + wall.right) * 0.5;
                if wall_center > center {
                    return -1.0;
                }
                return 1.0;
            }
        }
        0.0
    }

    fn start_dash(&mut self) {
        self.body = Body::Dash { left: DASH_TIME };
        self.vel.y = 0.0;
    }

    fn integrate(&mut self, dt: f32, walls: &[Aabb]) {
        self.grounded = false;
        let mut box_now = self.bounds();
        let mut sep_x = 0.0_f32;
        let mut sep_y = 0.0_f32;
        for wall in walls {
            if let Some(push) = separate(box_now, *wall) {
                sep_x += push.x;
                sep_y += push.y;
                self.pos += push;
                box_now = self.bounds();
                if push.y > 0.0 && self.vel.y <= 0.0 {
                    self.grounded = true;
                    self.vel.y = self.vel.y.max(0.0);
                }
            }
        }

        let mut time_left = 1.0;
        let dt_ms = dt * 1000.0;
        if self.vel.y <= 0.0 {
            let support_probe = self.bounds().translated(Vec2::new(0.0, -0.1));
            self.grounded |= walls.iter().any(|wall| overlaps(support_probe, *wall));
        }
        let mut logged_sweep = false;
        for _ in 0..4 {
            if time_left <= 0.0001 {
                break;
            }
            let mut earliest: Option<(f32, f32, f32)> = None;
            let step_ms = dt_ms * time_left;
            for wall in walls {
                let hit = swept_aabb(self.bounds(), self.vel, *wall, step_ms);
                if hit.time < 1.0 && earliest.map(|(t, _, _)| hit.time < t).unwrap_or(true) {
                    earliest = Some((hit.time, hit.normal_x, hit.normal_y));
                }
            }
            let Some((time, nx, ny)) = earliest else {
                if !logged_sweep {
                    log_integrate(sep_x, sep_y, false, 1.0, 0.0, self.pos.y);
                }
                self.pos += self.vel * dt * time_left;
                break;
            };
            if !logged_sweep {
                log_integrate(sep_x, sep_y, true, time, ny, self.pos.y);
                logged_sweep = true;
            }
            let travel = (time * time_left).max(0.0);
            self.pos += self.vel * dt * travel;
            self.pos += Vec2::new(nx, ny) * 0.05;
            if nx != 0.0 {
                self.vel.x = 0.0;
                self.wall_normal_x = nx;
            }
            if ny > 0.0 {
                self.vel.y = 0.0;
                self.grounded = true;
            } else if ny < 0.0 {
                self.vel.y = self.vel.y.min(0.0);
            }
            time_left *= 1.0 - time;
        }
    }

    fn transitions(&mut self, input: &InputState, audio: &Audio) {
        match self.body {
            Body::Dash { left } if left <= 0.0 => {
                self.body = if self.grounded {
                    if self.vel.x.abs() > 1.0 {
                        Body::Run
                    } else {
                        Body::Stand
                    }
                } else {
                    Body::Fall
                };
            }
            Body::Kick { left } if left <= 0.0 || self.grounded => {
                self.body = if self.grounded {
                    Body::Land { left: LAND_TIME }
                } else {
                    Body::Fall
                };
            }
            Body::Land { left } if left <= 0.0 => {
                self.body = if self.vel.x.abs() > 1.0 {
                    Body::Run
                } else {
                    Body::Stand
                };
            }
            Body::Jump | Body::Fall => {
                if self.grounded {
                    self.body = Body::Land { left: LAND_TIME };
                } else if self.wall_normal_x != 0.0 {
                    let into_left = self.wall_normal_x > 0.0 && input.left;
                    let into_right = self.wall_normal_x < 0.0 && input.right;
                    if into_left || into_right {
                        self.face = if into_left { Face::Left } else { Face::Right };
                        if !matches!(self.body, Body::Cling) {
                            audio.play(SoundId::Fall);
                        }
                        self.body = Body::Cling;
                        self.vel.x = 0.0;
                    } else if matches!(self.body, Body::Jump) && self.vel.y < 80.0 {
                        self.body = Body::Fall;
                        audio.play(SoundId::Fall);
                    }
                } else if matches!(self.body, Body::Jump) && self.vel.y < 40.0 {
                    self.body = Body::Fall;
                    audio.play(SoundId::Fall);
                }
            }
            Body::Cling => {
                let holding = (self.face == Face::Left && input.left)
                    || (self.face == Face::Right && input.right);
                if self.grounded || !holding || self.wall_normal_x == 0.0 {
                    self.body = if self.grounded {
                        Body::Land { left: LAND_TIME }
                    } else {
                        Body::Fall
                    };
                }
            }
            Body::Stand | Body::Run | Body::Land { .. } if !self.grounded && self.vel.y < -1.0 => {
                self.body = Body::Fall;
            }
            _ => {}
        }
    }

    fn spawn_shot(&mut self) {
        let sign = if self.face == Face::Right { 1.0 } else { -1.0 };
        self.shots.push(Shot {
            pos: self.pos + Vec2::new(sign * 16.0, 4.0),
            vel: Vec2::new(sign * SHOT_SPEED, 0.0),
            life: SHOT_LIFE,
        });
    }

    fn update_shots(&mut self, dt: f32, walls: &[Aabb]) {
        for shot in &mut self.shots {
            shot.life -= dt;
            shot.pos += shot.vel * dt;
            let box_ = Aabb::from_center(shot.pos, SHOT_W, SHOT_H);
            if walls.iter().any(|wall| overlaps(box_, *wall)) {
                shot.life = 0.0;
            }
        }
        self.shots.retain(|shot| shot.life > 0.0);
    }

    fn advance_anim(&mut self, dt: f32, atlas: &Atlas, land_frames: &[Frame]) {
        let (key, time, looping) = self.anim_spec();
        if key == "land" {
            if self.anim_key != "land" {
                self.anim_key = "land".to_string();
                self.anim_frames = land_frames.to_vec();
                self.frame = 0;
                self.anim_clock = 0.0;
                self.frame_time = time;
                self.anim_loop = false;
            }
        } else if self.anim_key != key {
            self.set_anim(atlas, key, time, looping);
        }
        if self.anim_frames.is_empty() {
            return;
        }
        self.anim_clock += dt;
        if self.anim_clock >= self.frame_time {
            self.anim_clock = 0.0;
            if self.frame + 1 < self.anim_frames.len() {
                self.frame += 1;
            } else if self.anim_loop {
                self.frame = 0;
            }
        }
    }

    fn anim_spec(&self) -> (&'static str, f32, bool) {
        match (self.body, self.shooting) {
            (Body::Stand, false) => ("standing", 0.15, true),
            (Body::Stand, true) => ("StandingShoot", 0.12, true),
            (Body::Run, false) => ("running", 0.08, true),
            (Body::Run, true) => ("RunningShoot", 0.08, true),
            (Body::Jump, false) => ("jumping", 0.10, false),
            (Body::Jump, true) => ("JumpingShoot", 0.10, false),
            (Body::Fall, false) => ("falling", 0.12, true),
            (Body::Fall, true) => ("FallingShoot", 0.12, true),
            (Body::Dash { .. }, false) => ("dashing", 0.08, true),
            (Body::Dash { .. }, true) => ("DashingShoot", 0.08, true),
            (Body::Cling, false) => ("clinging", 0.12, true),
            (Body::Cling, true) => ("ClingingShoot", 0.12, true),
            (Body::Kick { .. }, false) => ("kicking", 0.06, false),
            (Body::Kick { .. }, true) => ("KickingShoot", 0.06, false),
            (Body::Land { .. }, _) => ("land", 0.05, false),
        }
    }

    fn set_anim(&mut self, atlas: &Atlas, prefix: &str, time: f32, looping: bool) {
        self.anim_key = prefix.to_string();
        self.anim_frames = frames_with_prefix(atlas, prefix);
        self.frame = 0;
        self.anim_clock = 0.0;
        self.frame_time = time;
        self.anim_loop = looping;
    }

    pub fn draw(
        &self,
        atlas_tex: u32,
        atlas_w: f32,
        atlas_h: f32,
        land_tex: &[u32],
        out: &mut Vec<SpriteDraw>,
    ) {
        let flip = self.face == Face::Left;
        if self.anim_key == "land" {
            if let Some(tex) = land_tex.get(self.frame.min(land_tex.len().saturating_sub(1))) {
                out.push(SpriteDraw {
                    texture: *tex,
                    x: self.pos.x - 16.0,
                    y: self.pos.y - 18.0,
                    w: 32.0,
                    h: 36.0,
                    u0: 0.0,
                    v0: 0.0,
                    u1: 1.0,
                    v1: 1.0,
                    color: [1.0, 1.0, 1.0, 1.0],
                    flip_x: flip,
                });
            }
        } else if let Some(frame) = self.anim_frames.get(self.frame) {
            let w = frame.w as f32;
            let h = frame.h as f32;
            out.push(SpriteDraw {
                texture: atlas_tex,
                x: self.pos.x - w * 0.5,
                y: self.pos.y - h * 0.5,
                w,
                h,
                u0: frame.x as f32 / atlas_w,
                v0: frame.y as f32 / atlas_h,
                u1: (frame.x + frame.w) as f32 / atlas_w,
                v1: (frame.y + frame.h) as f32 / atlas_h,
                color: [1.0, 1.0, 1.0, 1.0],
                flip_x: flip,
            });
        }

        for shot in &self.shots {
            out.push(SpriteDraw {
                texture: 0,
                x: shot.pos.x - SHOT_W * 0.5,
                y: shot.pos.y - SHOT_H * 0.5,
                w: SHOT_W,
                h: SHOT_H,
                u0: 0.0,
                v0: 0.0,
                u1: 1.0,
                v1: 1.0,
                color: [0.3, 0.85, 1.0, 1.0],
                flip_x: false,
            });
        }
    }

    pub fn debug_box(&self) -> Aabb {
        self.bounds()
    }

    pub fn debug_body_label(&self) -> &'static str {
        self.body.label()
    }

    pub fn debug_anim_key(&self) -> &str {
        &self.anim_key
    }

    pub fn debug_anim_frame(&self) -> usize {
        self.frame
    }

    pub fn debug_anim_frame_count(&self) -> usize {
        self.anim_frames.len()
    }

    pub fn debug_grounded(&self) -> bool {
        self.grounded
    }

    pub fn debug_wall_normal_x(&self) -> f32 {
        self.wall_normal_x
    }

    pub fn debug_shooting(&self) -> bool {
        self.shooting
    }
}
