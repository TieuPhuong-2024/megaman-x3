use std::path::PathBuf;

use anyhow::Context;
use image::RgbaImage;

use crate::assets::{self, Atlas, Frame, TileMap};
use crate::physics::Aabb;
use crate::render::{DrawList, SpriteDraw};

use super::audio::Audio;
use super::debug_draw::{draw_text, push_box_outline};
use super::debug_monitor::{DebugMonitor, FrameStats, GameDebugSnapshot};
use super::debug_session::log_scene_boot;
use super::input::InputState;
use super::player::Player;
use super::tuning::{CAMERA_PLAYER_SCREEN_Y, SPAWN_X, SPAWN_Y, WINDOW_H, WINDOW_W};

pub struct CpuImage {
    pub width: u32,
    pub height: u32,
    pub rgba: Vec<u8>,
}

pub struct PlayScene {
    pub map: TileMap,
    pub atlas: Atlas,
    pub player: Player,
    pub map_image: CpuImage,
    pub atlas_image: CpuImage,
    pub land_frames: Vec<Frame>,
    pub land_images: Vec<CpuImage>,
    pub debug: bool,
    pub monitor: DebugMonitor,
    frame_stats: FrameStats,
    audio: Audio,
    cam_x: f32,
    cam_top: f32,
    map_tex: u32,
    atlas_tex: u32,
    land_tex: Vec<u32>,
}

impl PlayScene {
    pub fn load() -> anyhow::Result<Self> {
        let root = assets::assets_dir();
        let map = assets::load_tilemap(&root.join("MAP_1.tmx"))?;
        let atlas = assets::load_atlas(&root.join("XMan.xml"))?;
        let map_image = load_rgba(&map.image_path)?;
        let atlas_image = load_rgba(&atlas.image_path)?;
        let land_paths = [
            root.join("XMan/fall_on_ground_01.png"),
            root.join("XMan/fall_on_ground_02.png"),
        ];
        let mut land_images = Vec::new();
        let mut land_frames = Vec::new();
        for path in land_paths {
            if path.exists() {
                let image = load_rgba(&path)?;
                land_frames.push(Frame {
                    name: "land".to_string(),
                    x: 0,
                    y: 0,
                    w: image.width,
                    h: image.height,
                });
                land_images.push(image);
            }
        }
        let player = Player::new(&atlas);
        let (world_w, world_h) = map.world_size();
        let (cam_x, cam_top) = clamp_camera(player.pos.x, player.pos.y, world_w, world_h);
        let floor_top_near = map
            .walls
            .iter()
            .filter(|w| w.bounds.top <= player.pos.y + 80.0 && w.bounds.top >= player.pos.y - 120.0)
            .map(|w| w.bounds.top)
            .max_by(|a, b| a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal))
            .unwrap_or(-1.0);
        log_scene_boot(
            SPAWN_X,
            SPAWN_Y,
            map.walls.len(),
            floor_top_near,
            cam_x,
            cam_top,
            world_h,
        );
        Ok(Self {
            map,
            atlas,
            player,
            map_image,
            atlas_image,
            land_frames,
            land_images,
            debug: false,
            monitor: DebugMonitor::new(),
            frame_stats: FrameStats::default(),
            audio: Audio::new(),
            cam_x,
            cam_top,
            map_tex: 1,
            atlas_tex: 2,
            land_tex: Vec::new(),
        })
    }

    pub fn images_for_gpu(&self) -> Vec<&CpuImage> {
        let mut images = vec![&self.map_image, &self.atlas_image];
        images.extend(self.land_images.iter());
        images
    }

    pub fn assign_textures(&mut self, first_dynamic: u32) {
        self.map_tex = first_dynamic;
        self.atlas_tex = first_dynamic + 1;
        self.land_tex = (0..self.land_images.len() as u32)
            .map(|i| first_dynamic + 2 + i)
            .collect();
    }

    pub fn set_frame_stats(&mut self, stats: FrameStats) {
        self.frame_stats = stats;
    }

    pub fn debug_snapshot(&self) -> GameDebugSnapshot {
        GameDebugSnapshot {
            fps: self.frame_stats.fps,
            frame_ms: self.frame_stats.frame_ms,
            sim_steps: self.frame_stats.sim_steps,
            player_x: self.player.pos.x,
            player_y: self.player.pos.y,
            vel_x: self.player.vel.x,
            vel_y: self.player.vel.y,
            body: self.player.debug_body_label().to_string(),
            anim: self.player.debug_anim_key().to_string(),
            anim_frame: self.player.debug_anim_frame(),
            anim_frames: self.player.debug_anim_frame_count(),
            grounded: self.player.debug_grounded(),
            wall_normal_x: self.player.debug_wall_normal_x(),
            shooting: self.player.debug_shooting(),
            shot_count: self.player.shots.len(),
            cam_x: self.cam_x,
            cam_top: self.cam_top,
            wall_count: self.map.walls.len(),
            hitbox_debug: self.debug,
            monitor_visible: self.monitor.visible,
        }
    }

    pub fn update(&mut self, dt_ms: f32, input: &InputState) {
        if input.debug_toggle {
            self.debug = !self.debug;
        }
        if input.monitor_toggle {
            self.monitor.visible = !self.monitor.visible;
        }
        self.monitor.tick_dump(dt_ms, &self.debug_snapshot());
        let walls: Vec<Aabb> = self.map.walls.iter().map(|wall| wall.bounds).collect();
        let view_h = WINDOW_H as f32;
        self.player.update(
            dt_ms,
            input,
            &walls,
            &self.atlas,
            &self.land_frames,
            &self.audio,
            self.cam_top,
            view_h,
        );
        let (world_w, world_h) = self.map.world_size();
        let (cam_x, cam_top) = clamp_camera(self.player.pos.x, self.player.pos.y, world_w, world_h);
        self.cam_x = cam_x;
        self.cam_top = cam_top;
    }

    pub fn draw(&self) -> DrawList {
        let mut sprites = Vec::new();
        let mut overlay = Vec::new();
        let view_w = WINDOW_W as f32;
        let view_h = WINDOW_H as f32;
        for tile in self
            .map
            .visible_tiles(self.cam_x, self.cam_top, view_w, view_h)
        {
            sprites.push(SpriteDraw {
                texture: self.map_tex,
                x: tile.dest_x,
                y: tile.dest_y,
                w: tile.dest_w,
                h: tile.dest_h,
                u0: tile.src_x as f32 / self.map_image.width as f32,
                v0: tile.src_y as f32 / self.map_image.height as f32,
                u1: (tile.src_x + tile.src_w) as f32 / self.map_image.width as f32,
                v1: (tile.src_y + tile.src_h) as f32 / self.map_image.height as f32,
                color: [1.0, 1.0, 1.0, 1.0],
                flip_x: false,
            });
        }
        self.player.draw(
            self.atlas_tex,
            self.atlas.width as f32,
            self.atlas.height as f32,
            &self.land_tex,
            &mut sprites,
        );
        if self.debug {
            push_box_outline(&mut sprites, self.player.debug_box(), [0.1, 1.0, 0.2, 0.85]);
            for wall in &self.map.walls {
                push_box_outline(&mut sprites, wall.bounds, [1.0, 0.1, 1.0, 0.7]);
            }
            draw_text(
                &mut sprites,
                self.cam_x + 8.0,
                self.cam_top - 18.0,
                "HITBOX DEBUG",
                [0.3, 1.0, 0.4, 1.0],
            );
        }
        let snap = self.debug_snapshot();
        self.monitor.draw_hud(&mut overlay, view_w, view_h, &snap);
        DrawList {
            sprites,
            overlay,
            cam_x: self.cam_x,
            cam_top: self.cam_top,
            view_w,
            view_h,
        }
    }
}

fn clamp_camera(px: f32, py: f32, world_w: f32, world_h: f32) -> (f32, f32) {
    let view_w = WINDOW_W as f32;
    let view_h = WINDOW_H as f32;
    let cam_x = (px - view_w * 0.5).clamp(0.0, (world_w - view_w).max(0.0));
    let cam_top =
        (py + view_h * CAMERA_PLAYER_SCREEN_Y).clamp(view_h.min(world_h), world_h.max(view_h));
    (cam_x, cam_top)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn camera_starts_at_red_viewport() {
        let (cam_x, cam_top) = clamp_camera(SPAWN_X, SPAWN_Y, 1536.0, 960.0);

        assert_eq!((cam_x, cam_top), (0.0, 480.0));
    }
}

pub fn load_rgba(path: &PathBuf) -> anyhow::Result<CpuImage> {
    let image = image::open(path)
        .with_context(|| format!("open image {}", path.display()))?
        .to_rgba8();
    let mut image: RgbaImage = image;
    for pixel in image.pixels_mut() {
        if pixel[0] > 240 && pixel[1] < 20 && pixel[2] > 240 {
            pixel[3] = 0;
        }
    }
    Ok(CpuImage {
        width: image.width(),
        height: image.height(),
        rgba: image.into_raw(),
    })
}
