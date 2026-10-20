use std::fs;
use std::path::PathBuf;
use std::time::{SystemTime, UNIX_EPOCH};

use crate::render::SpriteDraw;

use super::debug_draw::{draw_text, push_filled_rect};

#[derive(Clone, Copy, Debug, Default)]
pub struct FrameStats {
    pub fps: f32,
    pub frame_ms: f32,
    pub sim_steps: u32,
}

#[derive(Clone, Debug)]
pub struct GameDebugSnapshot {
    pub fps: f32,
    pub frame_ms: f32,
    pub sim_steps: u32,
    pub player_x: f32,
    pub player_y: f32,
    pub vel_x: f32,
    pub vel_y: f32,
    pub body: String,
    pub anim: String,
    pub anim_frame: usize,
    pub anim_frames: usize,
    pub grounded: bool,
    pub wall_normal_x: f32,
    pub shooting: bool,
    pub shot_count: usize,
    pub cam_x: f32,
    pub cam_top: f32,
    pub wall_count: usize,
    pub hitbox_debug: bool,
    pub monitor_visible: bool,
}

pub struct DebugMonitor {
    pub visible: bool,
    dump_accum_ms: f32,
}

impl DebugMonitor {
    pub fn new() -> Self {
        Self {
            visible: false,
            dump_accum_ms: 0.0,
        }
    }

    pub fn tick_dump(&mut self, dt_ms: f32, snap: &GameDebugSnapshot) {
        if !dump_enabled() {
            return;
        }
        self.dump_accum_ms += dt_ms;
        if self.dump_accum_ms < 200.0 {
            return;
        }
        self.dump_accum_ms = 0.0;
        if let Err(err) = write_dump(snap) {
            eprintln!("debug dump: {err}");
        }
    }

    pub fn draw_hud(
        &self,
        overlay: &mut Vec<SpriteDraw>,
        view_w: f32,
        _view_h: f32,
        snap: &GameDebugSnapshot,
    ) {
        if !self.visible {
            return;
        }

        let panel_w = 210.0;
        let panel_h = 196.0;
        let margin = 8.0;
        let px = view_w - panel_w - margin;
        let py = margin;

        push_filled_rect(overlay, px, py, panel_w, panel_h, [0.02, 0.04, 0.08, 0.82]);
        push_filled_rect(
            overlay,
            px,
            py + panel_h - 2.0,
            panel_w,
            2.0,
            [0.2, 0.75, 1.0, 0.9],
        );

        let label = [0.55, 0.85, 1.0, 1.0];
        let value = [1.0, 1.0, 0.92, 1.0];
        let line_h = 22.0;
        let tx = px + 6.0;
        let mut ty = py + panel_h - 16.0;

        draw_text(overlay, tx, ty, "DEBUG MONITOR", label);
        ty -= line_h;

        let anim = snap.anim.to_ascii_uppercase();
        let anim = if anim.len() > 8 { anim[..8].to_string() } else { anim };
        let body = snap.body.to_ascii_uppercase();
        let lines = [
            format!("FPS {:.0} MS {:.1}", snap.fps, snap.frame_ms),
            format!("POS {:.0} {:.0}", snap.player_x, snap.player_y),
            format!("VEL {:.1} {:.1}", snap.vel_x, snap.vel_y),
            format!("BODY {}", body),
            format!("ANIM {} {}/{}", anim, snap.anim_frame + 1, snap.anim_frames.max(1)),
            format!(
                "GND {} W {:.0} SH {}",
                if snap.grounded { 1 } else { 0 },
                snap.wall_normal_x,
                snap.shot_count
            ),
            format!("CAM {:.0} {:.0}", snap.cam_x, snap.cam_top),
            format!(
                "F1 {} F2 {}",
                if snap.hitbox_debug { 1 } else { 0 },
                if snap.monitor_visible { 1 } else { 0 }
            ),
        ];

        for line in lines {
            draw_text(overlay, tx, ty, &line, value);
            ty -= line_h;
        }
    }
}

fn dump_enabled() -> bool {
    matches!(
        std::env::var("MEGAMAN_DEBUG_DUMP").as_deref(),
        Ok("1") | Ok("true") | Ok("TRUE")
    )
}

fn dump_path() -> PathBuf {
    std::env::var("MEGAMAN_DEBUG_DUMP_PATH")
        .map(PathBuf::from)
        .unwrap_or_else(|_| PathBuf::from(".debug/state.json"))
}

fn write_dump(snap: &GameDebugSnapshot) -> std::io::Result<()> {
    let path = dump_path();
    if let Some(parent) = path.parent() {
        fs::create_dir_all(parent)?;
    }
    let ts = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.as_millis())
        .unwrap_or(0);
    let json = format!(
        concat!(
            "{{\"ts\":{},\"fps\":{:.2},\"frame_ms\":{:.2},\"sim_steps\":{},",
            "\"player\":{{\"x\":{:.3},\"y\":{:.3},\"vx\":{:.3},\"vy\":{:.3},",
            "\"body\":\"{}\",\"anim\":\"{}\",\"frame\":{},\"frames\":{},",
            "\"grounded\":{},\"wall_normal_x\":{:.3},\"shooting\":{},\"shots\":{}}},",
            "\"camera\":{{\"x\":{:.3},\"top\":{:.3}}},\"walls\":{},",
            "\"hitbox_debug\":{},\"monitor_visible\":{}}}"
        ),
        ts,
        snap.fps,
        snap.frame_ms,
        snap.sim_steps,
        snap.player_x,
        snap.player_y,
        snap.vel_x,
        snap.vel_y,
        escape_json(&snap.body),
        escape_json(&snap.anim),
        snap.anim_frame,
        snap.anim_frames,
        snap.grounded,
        snap.wall_normal_x,
        snap.shooting,
        snap.shot_count,
        snap.cam_x,
        snap.cam_top,
        snap.wall_count,
        snap.hitbox_debug,
        snap.monitor_visible,
    );
    fs::write(path, json)
}

fn escape_json(s: &str) -> String {
    s.replace('\\', "\\\\").replace('"', "\\\"")
}
