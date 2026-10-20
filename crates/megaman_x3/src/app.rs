use std::time::Instant;

use anyhow::Context;
use engine::game::tuning::{FRAME_MS, WINDOW_H, WINDOW_W};
use engine::game::{FrameStats, InputState};
use engine::{PlayScene, Renderer};
use sdl2::event::Event;
use sdl2::keyboard::Scancode;

pub fn run() -> anyhow::Result<()> {
    let sdl = sdl2::init().map_err(|err| anyhow::anyhow!(err))?;
    let video = sdl.video().map_err(|err| anyhow::anyhow!(err))?;
    let window = video
        .window("MEGAMAN X3", WINDOW_W, WINDOW_H)
        .position_centered()
        .resizable()
        .build()
        .context("create SDL window")?;
    let mut pump = sdl.event_pump().map_err(|err| anyhow::anyhow!(err))?;

    let (px, py) = window.drawable_size();
    let mut renderer = Renderer::new(&window, px, py)?;
    let mut scene = PlayScene::load()?;
    let ids = renderer.upload_images(&scene.images_for_gpu());
    scene.assign_textures(ids[0]);

    let mut last = Instant::now();
    let mut accumulator = 0.0_f32;
    let mut debug_was_down = false;
    let mut monitor_was_down = false;
    let mut fps = 0.0_f32;
    let mut fps_accum_ms = 0.0_f32;
    let mut fps_frames = 0_u32;
    let mut running = true;

    while running {
        for event in pump.poll_iter() {
            match event {
                Event::Quit { .. } => running = false,
                Event::Window { win_event, .. } => {
                    if let sdl2::event::WindowEvent::Resized(w, h) = win_event {
                        let _ = (w, h);
                        let (dw, dh) = window.drawable_size();
                        renderer.resize(dw, dh);
                    }
                }
                _ => {}
            }
        }

        let keyboard = pump.keyboard_state();
        let debug_down = keyboard.is_scancode_pressed(Scancode::F1);
        let monitor_down = keyboard.is_scancode_pressed(Scancode::F2);
        let input = InputState {
            left: keyboard.is_scancode_pressed(Scancode::Left),
            right: keyboard.is_scancode_pressed(Scancode::Right),
            jump: keyboard.is_scancode_pressed(Scancode::Space)
                || keyboard.is_scancode_pressed(Scancode::X),
            shoot: keyboard.is_scancode_pressed(Scancode::C)
                || keyboard.is_scancode_pressed(Scancode::X)
                || keyboard.is_scancode_pressed(Scancode::Space),
            dash: keyboard.is_scancode_pressed(Scancode::LShift)
                || keyboard.is_scancode_pressed(Scancode::RShift),
            debug_toggle: debug_down && !debug_was_down,
            monitor_toggle: monitor_down && !monitor_was_down,
            quit: keyboard.is_scancode_pressed(Scancode::Escape),
        };
        debug_was_down = debug_down;
        monitor_was_down = monitor_down;
        if input.quit {
            running = false;
        }

        let now = Instant::now();
        let mut elapsed = now.duration_since(last).as_secs_f32() * 1000.0;
        last = now;
        if elapsed > FRAME_MS * 2.0 {
            elapsed = FRAME_MS;
        }
        accumulator += elapsed;
        let mut sim_steps = 0_u32;
        while accumulator >= FRAME_MS {
            scene.update(FRAME_MS, &input);
            accumulator -= FRAME_MS;
            sim_steps += 1;
        }

        fps_frames += 1;
        fps_accum_ms += elapsed;
        if fps_accum_ms >= 500.0 {
            fps = fps_frames as f32 / (fps_accum_ms / 1000.0);
            fps_frames = 0;
            fps_accum_ms = 0.0;
        }
        scene.set_frame_stats(FrameStats {
            fps,
            frame_ms: elapsed,
            sim_steps,
        });

        renderer.draw(&scene.draw())?;
    }
    Ok(())
}
