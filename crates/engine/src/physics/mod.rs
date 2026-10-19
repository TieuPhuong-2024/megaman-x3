use glam::Vec2;

/// Axis-aligned box. `top` is the larger Y (world +Y is up).
#[derive(Clone, Copy, Debug, PartialEq)]
pub struct Aabb {
    pub left: f32,
    pub bottom: f32,
    pub right: f32,
    pub top: f32,
}

impl Aabb {
    pub fn from_bottom_left(x: f32, y: f32, w: f32, h: f32) -> Self {
        Self {
            left: x,
            bottom: y,
            right: x + w,
            top: y + h,
        }
    }

    pub fn from_center(center: Vec2, w: f32, h: f32) -> Self {
        Self {
            left: center.x - w * 0.5,
            right: center.x + w * 0.5,
            bottom: center.y - h * 0.5,
            top: center.y + h * 0.5,
        }
    }

    pub fn translated(self, delta: Vec2) -> Self {
        Self {
            left: self.left + delta.x,
            right: self.right + delta.x,
            bottom: self.bottom + delta.y,
            top: self.top + delta.y,
        }
    }
}

#[derive(Clone, Copy, Debug)]
pub struct SweepHit {
    pub time: f32,
    pub normal_x: f32,
    pub normal_y: f32,
}

pub fn overlaps(a: Aabb, b: Aabb) -> bool {
    let left = b.left - a.right;
    let top = b.top - a.bottom;
    let right = b.right - a.left;
    let bottom = b.bottom - a.top;
    !(left > 0.0 || right < 0.0 || top < 0.0 || bottom > 0.0)
}

fn broadphase(mover: Aabb, velocity_px: Vec2) -> Aabb {
    Aabb {
        top: if velocity_px.y > 0.0 {
            mover.top + velocity_px.y
        } else {
            mover.top
        },
        bottom: if velocity_px.y > 0.0 {
            mover.bottom
        } else {
            mover.bottom + velocity_px.y
        },
        left: if velocity_px.x > 0.0 {
            mover.left
        } else {
            mover.left + velocity_px.x
        },
        right: if velocity_px.x > 0.0 {
            mover.right + velocity_px.x
        } else {
            mover.right
        },
    }
}

/// Port of `CCollision::SweptAABB`.
/// `velocity` is pixels per second. `dt_ms` is the frame delta in milliseconds.
/// `time` is 0..1 of this frame. A miss returns time 1 and a zero normal.
pub fn swept_aabb(mover: Aabb, velocity: Vec2, other: Aabb, dt_ms: f32) -> SweepHit {
    let current = velocity * dt_ms / 1000.0;
    let miss = SweepHit {
        time: 1.0,
        normal_x: 0.0,
        normal_y: 0.0,
    };
    if !overlaps(broadphase(mover, current), other) {
        return miss;
    }

    let (dx_entry, dx_exit) = if current.x > 0.0 {
        (other.left - mover.right, other.right - mover.left)
    } else {
        (other.right - mover.left, other.left - mover.right)
    };
    let (dy_entry, dy_exit) = if current.y > 0.0 {
        (other.bottom - mover.top, other.top - mover.bottom)
    } else {
        (other.top - mover.bottom, other.bottom - mover.top)
    };

    let (tx_entry, tx_exit) = if current.x == 0.0 {
        (f32::NEG_INFINITY, f32::INFINITY)
    } else {
        (dx_entry / current.x, dx_exit / current.x)
    };
    let (ty_entry, ty_exit) = if current.y == 0.0 {
        (f32::NEG_INFINITY, f32::INFINITY)
    } else {
        (dy_entry / current.y, dy_exit / current.y)
    };

    let entry = tx_entry.max(ty_entry);
    let exit = tx_exit.min(ty_exit);
    if entry > exit || (tx_entry < 0.0 && ty_entry < 0.0) || tx_entry > 1.0 || ty_entry > 1.0 {
        return miss;
    }

    let (normal_x, normal_y) = if tx_entry > ty_entry {
        if dx_entry < 0.0 {
            (1.0, 0.0)
        } else {
            (-1.0, 0.0)
        }
    } else if dy_entry < 0.0 {
        (0.0, 1.0)
    } else {
        (0.0, -1.0)
    };

    SweepHit {
        time: entry.max(0.0),
        normal_x,
        normal_y,
    }
}

/// Push `mover` out of `solid` along the smallest overlap. Used when a frame starts inside a wall.
pub fn separate(mover: Aabb, solid: Aabb) -> Option<Vec2> {
    if !overlaps(mover, solid) {
        return None;
    }
    let pen_left = mover.right - solid.left;
    let pen_right = solid.right - mover.left;
    let pen_down = mover.top - solid.bottom;
    let pen_up = solid.top - mover.bottom;
    let options = [
        (pen_left, Vec2::new(-pen_left, 0.0)),
        (pen_right, Vec2::new(pen_right, 0.0)),
        (pen_down, Vec2::new(0.0, -pen_down)),
        (pen_up, Vec2::new(0.0, pen_up)),
    ];
    options
        .into_iter()
        .filter(|(pen, _)| *pen > 0.0)
        .min_by(|a, b| a.0.partial_cmp(&b.0).unwrap_or(std::cmp::Ordering::Equal))
        .map(|(_, delta)| delta)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn falling_onto_floor_hits_from_above() {
        let player = Aabb {
            left: 0.0,
            right: 20.0,
            bottom: 40.0,
            top: 70.0,
        };
        let floor = Aabb {
            left: -10.0,
            right: 100.0,
            bottom: 0.0,
            top: 32.0,
        };
        let hit = swept_aabb(player, Vec2::new(0.0, -200.0), floor, 1000.0);
        assert!(hit.time < 1.0, "expected a hit, time {}", hit.time);
        assert_eq!(hit.normal_y, 1.0);
        assert_eq!(hit.normal_x, 0.0);
    }

    #[test]
    fn separated_boxes_do_not_overlap() {
        let a = Aabb::from_bottom_left(0.0, 0.0, 10.0, 10.0);
        let b = Aabb::from_bottom_left(20.0, 0.0, 10.0, 10.0);
        assert!(!overlaps(a, b));
    }
}
