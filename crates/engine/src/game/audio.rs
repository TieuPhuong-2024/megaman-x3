use std::time::Duration;

use rodio::{source::Source, OutputStream, Sink};

#[derive(Clone, Copy, Debug)]
pub enum SoundId {
    Shoot,
    Jump,
    Fall,
}

pub struct Audio {
    _stream: Option<OutputStream>,
    sink: Option<Sink>,
}

impl Audio {
    pub fn new() -> Self {
        match OutputStream::try_default() {
            Ok((stream, handle)) => match Sink::try_new(&handle) {
                Ok(sink) => Self {
                    _stream: Some(stream),
                    sink: Some(sink),
                },
                Err(_) => Self {
                    _stream: Some(stream),
                    sink: None,
                },
            },
            Err(_) => Self {
                _stream: None,
                sink: None,
            },
        }
    }

    pub fn play(&self, id: SoundId) {
        let Some(sink) = &self.sink else {
            return;
        };
        let (freq, ms) = match id {
            SoundId::Shoot => (880.0, 70),
            SoundId::Jump => (520.0, 90),
            SoundId::Fall => (180.0, 80),
        };
        sink.append(Beep::new(freq, ms));
    }
}

impl Default for Audio {
    fn default() -> Self {
        Self::new()
    }
}

struct Beep {
    freq: f32,
    sample_rate: u32,
    left: u32,
    phase: f32,
}

impl Beep {
    fn new(freq: f32, ms: u32) -> Self {
        let sample_rate = 44_100;
        Self {
            freq,
            sample_rate,
            left: sample_rate * ms / 1000,
            phase: 0.0,
        }
    }
}

impl Iterator for Beep {
    type Item = f32;

    fn next(&mut self) -> Option<f32> {
        if self.left == 0 {
            return None;
        }
        self.left -= 1;
        let sample = (self.phase * std::f32::consts::TAU).sin() * 0.15;
        self.phase = (self.phase + self.freq / self.sample_rate as f32) % 1.0;
        Some(sample)
    }
}

impl Source for Beep {
    fn current_frame_len(&self) -> Option<usize> {
        Some(self.left as usize)
    }

    fn channels(&self) -> u16 {
        1
    }

    fn sample_rate(&self) -> u32 {
        self.sample_rate
    }

    fn total_duration(&self) -> Option<Duration> {
        Some(Duration::from_secs_f32(
            self.left as f32 / self.sample_rate as f32,
        ))
    }
}
