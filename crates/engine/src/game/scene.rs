use std::path::PathBuf;

use anyhow::Context;
use image::RgbaImage;

use crate::assets::{self, Atlas, Frame, TileMap};
use crate::physics::Aabb;
use crate::render::{DrawList, SpriteDraw};

use super::audio::Audio;
use super::input::InputState;
use super::player::Player;
use super::tuning::{WINDOW_H, WINDOW_W};

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
        Ok(Self {
            map,
            atlas,
            player,
            map_image,
            atlas_image,
            land_frames,
            land_images,
            debug: false,
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

    pub fn update(&mut self, dt_ms: f32, input: &InputState) {
        if input.debug_toggle {
            self.debug = !self.debug;
        }
        let walls: Vec<Aabb> = self.map.walls.iter().map(|wall| wall.bounds).collect();
        self.player.update(
            dt_ms,
            input,
            &walls,
            &self.atlas,
            &self.land_frames,
            &self.audio,
        );
        let (world_w, world_h) = self.map.world_size();
        let (cam_x, cam_top) = clamp_camera(self.player.pos.x, self.player.pos.y, world_w, world_h);
        self.cam_x = cam_x;
        self.cam_top = cam_top;
    }

    pub fn draw(&self) -> DrawList {
        let mut sprites = Vec::new();
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
            push_box(&mut sprites, self.player.debug_box(), [0.1, 1.0, 0.2, 0.85]);
            for wall in &self.map.walls {
                push_box(&mut sprites, wall.bounds, [1.0, 0.1, 1.0, 0.7]);
            }
            draw_text(&mut sprites, 8.0, self.cam_top - 18.0, "DEBUG ON");
        }
        DrawList {
            sprites,
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
    let cam_top = (py + view_h * 0.5).clamp(view_h.min(world_h), world_h.max(view_h));
    (cam_x, cam_top)
}

fn push_box(sprites: &mut Vec<SpriteDraw>, box_: Aabb, color: [f32; 4]) {
    let w = box_.right - box_.left;
    let h = box_.top - box_.bottom;
    let t = 2.0;
    let edges = [
        (box_.left, box_.bottom, w, t),
        (box_.left, box_.top - t, w, t),
        (box_.left, box_.bottom, t, h),
        (box_.right - t, box_.bottom, t, h),
    ];
    for (x, y, ew, eh) in edges {
        sprites.push(SpriteDraw {
            texture: 0,
            x,
            y,
            w: ew,
            h: eh,
            u0: 0.0,
            v0: 0.0,
            u1: 1.0,
            v1: 1.0,
            color,
            flip_x: false,
        });
    }
}

fn draw_text(sprites: &mut Vec<SpriteDraw>, x: f32, y: f32, text: &str) {
    let mut cursor = x;
    for ch in text.chars() {
        if let Some(rows) = glyph(ch) {
            for (row_i, row) in rows.iter().enumerate() {
                for col in 0..5 {
                    if row & (1 << (4 - col)) != 0 {
                        sprites.push(SpriteDraw {
                            texture: 0,
                            x: cursor + col as f32 * 3.0,
                            y: y + (6 - row_i) as f32 * 3.0,
                            w: 2.5,
                            h: 2.5,
                            u0: 0.0,
                            v0: 0.0,
                            u1: 1.0,
                            v1: 1.0,
                            color: [1.0, 1.0, 0.3, 1.0],
                            flip_x: false,
                        });
                    }
                }
            }
        }
        cursor += 18.0;
    }
}

fn glyph(ch: char) -> Option<[u8; 7]> {
    Some(match ch {
        'D' => [
            0b11110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b11110,
        ],
        'E' => [
            0b11111, 0b10000, 0b10000, 0b11110, 0b10000, 0b10000, 0b11111,
        ],
        'B' => [
            0b11110, 0b10001, 0b10001, 0b11110, 0b10001, 0b10001, 0b11110,
        ],
        'U' => [
            0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110,
        ],
        'G' => [
            0b01111, 0b10000, 0b10000, 0b10011, 0b10001, 0b10001, 0b01111,
        ],
        'O' => [
            0b01110, 0b10001, 0b10001, 0b10001, 0b10001, 0b10001, 0b01110,
        ],
        'N' => [
            0b10001, 0b11001, 0b10101, 0b10011, 0b10001, 0b10001, 0b10001,
        ],
        ' ' => [0, 0, 0, 0, 0, 0, 0],
        _ => return None,
    })
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
