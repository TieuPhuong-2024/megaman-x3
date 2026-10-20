use std::fs::OpenOptions;
use std::io::Write;
use std::sync::atomic::{AtomicU32, Ordering};

use super::tuning::WINDOW_H;

const LOG_PATH: &str = r"d:\Workspace\megaman-x3\.cursor\debug-274536.log";
const SESSION: &str = "274536";

static PLAYER_LOG_FRAME: AtomicU32 = AtomicU32::new(0);

pub fn agent_log(hypothesis_id: &str, location: &str, message: &str, data_json: &str) {
    let ts = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_millis())
        .unwrap_or(0);
    let line = format!(
        r#"{{"sessionId":"{session}","hypothesisId":"{hyp}","location":"{loc}","message":"{msg}","data":{data},"timestamp":{ts}}}"#,
        session = SESSION,
        hyp = hypothesis_id,
        loc = location.replace('"', "\\\""),
        msg = message.replace('"', "\\\""),
        data = data_json,
        ts = ts,
    );
    // #region agent log
    if let Ok(mut file) = OpenOptions::new().create(true).append(true).open(LOG_PATH) {
        let _ = writeln!(file, "{line}");
    }
    // #endregion
}

pub fn log_player_tick(
    body: &str,
    pos_x: f32,
    pos_y: f32,
    vel_x: f32,
    vel_y: f32,
    grounded: bool,
    anim_frames: usize,
    sprite_wh: Option<(f32, f32)>,
    cam_top: f32,
    view_bottom: f32,
) {
    let n = PLAYER_LOG_FRAME.fetch_add(1, Ordering::Relaxed);
    if n > 72 || n % 6 != 0 {
        return;
    }
    let (sw, sh) = sprite_wh.unwrap_or((0.0, 0.0));
    agent_log(
        "H1-H3",
        "player.rs:update_tick",
        "player state",
        &format!(
            r#"{{"tick":{n},"body":"{body}","pos_x":{pos_x:.2},"pos_y":{pos_y:.2},"vel_x":{vel_x:.2},"vel_y":{vel_y:.2},"grounded":{grounded},"anim_frames":{anim_frames},"sprite_w":{sw:.1},"sprite_h":{sh:.1},"cam_top":{cam_top:.2},"view_bottom":{view_bottom:.2}}}"#,
        ),
    );
}

pub fn log_integrate(
    push_x: f32,
    push_y: f32,
    hit: bool,
    hit_time: f32,
    hit_ny: f32,
    pos_y_after: f32,
) {
    let n = PLAYER_LOG_FRAME.load(Ordering::Relaxed);
    if n > 24 {
        return;
    }
    agent_log(
        "H3-H4",
        "player.rs:integrate",
        "collision step",
        &format!(
            r#"{{"tick":{n},"separate_push_x":{push_x:.3},"separate_push_y":{push_y:.3},"sweep_hit":{hit},"sweep_time":{hit_time:.4},"sweep_ny":{hit_ny:.3},"pos_y":{pos_y_after:.2}}}"#,
        ),
    );
}

pub fn log_scene_boot(
    spawn_x: f32,
    spawn_y: f32,
    wall_count: usize,
    floor_top_near: f32,
    cam_x: f32,
    cam_top: f32,
    world_h: f32,
) {
    agent_log(
        "H1-H2",
        "scene.rs:load",
        "spawn and camera",
        &format!(
            r#"{{"spawn_x":{spawn_x:.2},"spawn_y":{spawn_y:.2},"walls":{wall_count},"nearest_floor_top":{floor_top_near:.2},"cam_x":{cam_x:.2},"cam_top":{cam_top:.2},"world_h":{world_h:.2},"view_h":{view_h:.1}}}"#,
            view_h = WINDOW_H as f32,
        ),
    );
}
