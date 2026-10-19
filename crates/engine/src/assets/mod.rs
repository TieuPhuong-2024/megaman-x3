use std::collections::HashMap;
use std::path::{Path, PathBuf};

use anyhow::{anyhow, Context};
use quick_xml::events::Event;
use quick_xml::Reader;

use crate::game::tuning::MAP_SCALE;
use crate::physics::Aabb;

#[derive(Clone, Debug)]
pub struct Frame {
    pub name: String,
    pub x: u32,
    pub y: u32,
    pub w: u32,
    pub h: u32,
}

#[derive(Clone, Debug)]
pub struct Atlas {
    pub image_path: PathBuf,
    pub width: u32,
    pub height: u32,
    pub frames: HashMap<String, Frame>,
}

#[derive(Clone, Debug)]
pub struct Wall {
    pub id: i32,
    pub bounds: Aabb,
}

#[derive(Clone, Debug)]
pub struct TileMap {
    pub width: i32,
    pub height: i32,
    pub tile_w: i32,
    pub tile_h: i32,
    pub columns: i32,
    pub image_path: PathBuf,
    pub gids: Vec<u32>,
    pub walls: Vec<Wall>,
    pub scale: f32,
}

impl TileMap {
    pub fn frame_size(&self) -> (f32, f32) {
        (
            self.tile_w as f32 * self.scale,
            self.tile_h as f32 * self.scale,
        )
    }

    pub fn world_size(&self) -> (f32, f32) {
        let (fw, fh) = self.frame_size();
        (self.width as f32 * fw, self.height as f32 * fh)
    }

    /// Tile gids visible to a viewport whose top-left world corner is `(cam_x, cam_top)`.
    pub fn visible_tiles(
        &self,
        cam_x: f32,
        cam_top: f32,
        view_w: f32,
        view_h: f32,
    ) -> Vec<TileSprite> {
        let (fw, fh) = self.frame_size();
        let left = cam_x;
        let top = cam_top;
        let right = cam_x + view_w;
        let bottom = cam_top - view_h;

        let col_begin = ((left / fw).floor() as i32).clamp(0, self.width);
        let col_end = ((right / fw).floor() as i32 + 1).clamp(0, self.width);
        let row_begin = (self.height - ((top / fh).floor() as i32 + 1)).clamp(0, self.height);
        let row_end = (self.height - (bottom / fh).floor() as i32).clamp(0, self.height);

        let mut out = Vec::new();
        for row in row_begin..row_end {
            for col in col_begin..col_end {
                let gid = self.gids[(row * self.width + col) as usize];
                if gid == 0 {
                    continue;
                }
                let local = gid - 1;
                let src_col = local as i32 % self.columns;
                let src_row = local as i32 / self.columns;
                out.push(TileSprite {
                    dest_x: col as f32 * fw,
                    dest_y: (self.height - row - 1) as f32 * fh,
                    dest_w: fw,
                    dest_h: fh,
                    src_x: src_col as u32 * self.tile_w as u32,
                    src_y: src_row as u32 * self.tile_h as u32,
                    src_w: self.tile_w as u32,
                    src_h: self.tile_h as u32,
                });
            }
        }
        out
    }
}

#[derive(Clone, Copy, Debug)]
pub struct TileSprite {
    pub dest_x: f32,
    pub dest_y: f32,
    pub dest_w: f32,
    pub dest_h: f32,
    pub src_x: u32,
    pub src_y: u32,
    pub src_w: u32,
    pub src_h: u32,
}

pub fn assets_dir() -> PathBuf {
    let root = PathBuf::from(env!("CARGO_MANIFEST_DIR")).join("../..");
    let assets = root.join("assets");
    if assets.join("MAP_1.tmx").exists() {
        assets
    } else {
        root.join("GameEngine/Resource")
    }
}

pub fn load_atlas(path: &Path) -> anyhow::Result<Atlas> {
    let text =
        std::fs::read_to_string(path).with_context(|| format!("read atlas {}", path.display()))?;
    let mut reader = Reader::from_str(&text);
    reader.config_mut().trim_text(true);
    let mut buf = Vec::new();
    let mut atlas = Atlas {
        image_path: path.parent().unwrap_or(Path::new(".")).to_path_buf(),
        width: 1,
        height: 1,
        frames: HashMap::new(),
    };

    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Empty(e)) | Ok(Event::Start(e)) => {
                let name = String::from_utf8_lossy(e.name().as_ref()).into_owned();
                let attrs = attrs(&e);
                if name == "TextureAtlas" {
                    if let Some(image) = attrs.get("imagePath") {
                        atlas.image_path = path.parent().unwrap_or(Path::new(".")).join(image);
                    }
                    atlas.width = attrs.get("width").and_then(|v| v.parse().ok()).unwrap_or(1);
                    atlas.height = attrs
                        .get("height")
                        .and_then(|v| v.parse().ok())
                        .unwrap_or(1);
                } else if name == "sprite" {
                    let frame_name = attrs.get("n").cloned().unwrap_or_default();
                    let frame = Frame {
                        name: frame_name.clone(),
                        x: parse_attr(&attrs, "x")?,
                        y: parse_attr(&attrs, "y")?,
                        w: parse_attr(&attrs, "w")?,
                        h: parse_attr(&attrs, "h")?,
                    };
                    atlas.frames.insert(frame_name, frame);
                }
            }
            Ok(Event::Eof) => break,
            Err(err) => return Err(anyhow!("atlas xml: {err}")),
            _ => {}
        }
        buf.clear();
    }
    Ok(atlas)
}

pub fn load_tilemap(path: &Path) -> anyhow::Result<TileMap> {
    let text =
        std::fs::read_to_string(path).with_context(|| format!("read map {}", path.display()))?;
    let mut reader = Reader::from_str(&text);
    reader.config_mut().trim_text(true);
    let mut buf = Vec::new();

    let mut map = TileMap {
        width: 0,
        height: 0,
        tile_w: 32,
        tile_h: 32,
        columns: 1,
        image_path: path.parent().unwrap_or(Path::new(".")).to_path_buf(),
        gids: Vec::new(),
        walls: Vec::new(),
        scale: MAP_SCALE,
    };
    let mut in_wall_group = false;
    let mut saw_tileset = false;

    loop {
        match reader.read_event_into(&mut buf) {
            Ok(Event::Start(e)) | Ok(Event::Empty(e)) => {
                let name = String::from_utf8_lossy(e.name().as_ref()).into_owned();
                let attrs = attrs(&e);
                match name.as_str() {
                    "map" => {
                        map.width = parse_attr(&attrs, "width")?;
                        map.height = parse_attr(&attrs, "height")?;
                    }
                    "tileset" if !saw_tileset => {
                        saw_tileset = true;
                        map.tile_w = parse_attr(&attrs, "tilewidth")?;
                        map.tile_h = parse_attr(&attrs, "tileheight")?;
                        map.columns = parse_attr::<i32>(&attrs, "columns")?.max(1);
                    }
                    "image" if map.gids.is_empty() => {
                        if let Some(source) = attrs.get("source") {
                            map.image_path = path.parent().unwrap_or(Path::new(".")).join(source);
                        }
                    }
                    "layer" => {
                        if let Some(w) = attrs.get("width").and_then(|v| v.parse().ok()) {
                            map.width = w;
                        }
                        if let Some(h) = attrs.get("height").and_then(|v| v.parse().ok()) {
                            map.height = h;
                        }
                    }
                    "tile" => {
                        let gid = attrs.get("gid").and_then(|v| v.parse().ok()).unwrap_or(0);
                        map.gids.push(gid);
                    }
                    "objectgroup" => {
                        in_wall_group = attrs.get("name").map(|n| n == "Wall").unwrap_or(false);
                    }
                    "object" if in_wall_group => {
                        let tmx_x: f32 = parse_attr(&attrs, "x")?;
                        let tmx_y: f32 = parse_attr(&attrs, "y")?;
                        let width: f32 = parse_attr::<f32>(&attrs, "width")? * map.scale;
                        let height: f32 = parse_attr::<f32>(&attrs, "height")? * map.scale;
                        let map_h = map.height as f32 * map.tile_h as f32 * map.scale;
                        let world_x = tmx_x * map.scale;
                        let world_y = map_h - tmx_y * map.scale - height;
                        let id = attrs.get("id").and_then(|v| v.parse().ok()).unwrap_or(0);
                        map.walls.push(Wall {
                            id,
                            bounds: Aabb::from_bottom_left(world_x, world_y, width, height),
                        });
                    }
                    _ => {}
                }
            }
            Ok(Event::End(e)) => {
                if e.name().as_ref() == b"objectgroup" {
                    in_wall_group = false;
                }
            }
            Ok(Event::Eof) => break,
            Err(err) => return Err(anyhow!("tmx xml: {err}")),
            _ => {}
        }
        buf.clear();
    }

    if map.gids.len() != (map.width * map.height) as usize {
        return Err(anyhow!(
            "tile count {} does not match {}x{}",
            map.gids.len(),
            map.width,
            map.height
        ));
    }
    Ok(map)
}

fn attrs(e: &quick_xml::events::BytesStart<'_>) -> HashMap<String, String> {
    let mut map = HashMap::new();
    for attr in e.attributes().flatten() {
        let key = String::from_utf8_lossy(attr.key.as_ref()).into_owned();
        let value = String::from_utf8_lossy(&attr.value).into_owned();
        map.insert(key, value);
    }
    map
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn map_and_atlas_load() {
        let root = assets_dir();
        let map = load_tilemap(&root.join("MAP_1.tmx")).unwrap();
        assert_eq!((map.width, map.height), (24, 15));
        assert_eq!(map.gids.len(), 24 * 15);
        assert_eq!(map.walls.len(), 7);
        let atlas = load_atlas(&root.join("XMan.xml")).unwrap();
        assert!(atlas.frames.contains_key("standing_1"));
        assert!(frames_with_prefix(&atlas, "running").len() >= 9);
        assert!(frames_with_prefix(&atlas, "DashingShoot").len() >= 2);
    }
}

fn parse_attr<T: std::str::FromStr>(
    attrs: &HashMap<String, String>,
    key: &str,
) -> anyhow::Result<T> {
    attrs
        .get(key)
        .ok_or_else(|| anyhow!("missing attribute {key}"))?
        .parse()
        .map_err(|_| anyhow!("bad attribute {key}"))
}

/// Frames named `{prefix}_N`, sorted by N. Prefix match is case-sensitive and requires the underscore.
pub fn frames_with_prefix(atlas: &Atlas, prefix: &str) -> Vec<Frame> {
    let mut frames: Vec<(u32, Frame)> = atlas
        .frames
        .values()
        .filter_map(|frame| {
            let rest = frame.name.strip_prefix(prefix)?;
            let rest = rest.strip_prefix('_')?;
            let index: u32 = rest.parse().ok()?;
            Some((index, frame.clone()))
        })
        .collect();
    frames.sort_by_key(|(index, _)| *index);
    frames.into_iter().map(|(_, frame)| frame).collect()
}
