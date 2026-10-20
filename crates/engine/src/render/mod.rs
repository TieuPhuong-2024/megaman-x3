use std::mem::size_of;

use anyhow::Context;
use bytemuck::{Pod, Zeroable};
use raw_window_handle::{HasDisplayHandle, HasWindowHandle};

use crate::game::CpuImage;

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct CameraUniform {
    cam_x: f32,
    cam_top: f32,
    view_w: f32,
    view_h: f32,
}

#[repr(C)]
#[derive(Clone, Copy, Pod, Zeroable)]
struct Instance {
    pos: [f32; 2],
    size: [f32; 2],
    uv_min: [f32; 2],
    uv_max: [f32; 2],
    color: [f32; 4],
    flip: f32,
    _pad: [f32; 3],
}

#[derive(Clone, Copy, Debug)]
pub struct SpriteDraw {
    pub texture: u32,
    pub x: f32,
    pub y: f32,
    pub w: f32,
    pub h: f32,
    pub u0: f32,
    pub v0: f32,
    pub u1: f32,
    pub v1: f32,
    pub color: [f32; 4],
    pub flip_x: bool,
}

pub struct DrawList {
    pub sprites: Vec<SpriteDraw>,
    /// Screen-space HUD (pixels, origin bottom-left). Drawn after `sprites` with a fixed camera.
    pub overlay: Vec<SpriteDraw>,
    pub cam_x: f32,
    pub cam_top: f32,
    pub view_w: f32,
    pub view_h: f32,
}

const SHADER: &str = r#"
struct Camera {
    cam_x: f32,
    cam_top: f32,
    view_w: f32,
    view_h: f32,
}
@group(0) @binding(0) var<uniform> cam: Camera;

struct VsOut {
    @builtin(position) clip: vec4<f32>,
    @location(0) uv: vec2<f32>,
    @location(1) color: vec4<f32>,
}

struct Instance {
    @location(0) pos: vec2<f32>,
    @location(1) size: vec2<f32>,
    @location(2) uv_min: vec2<f32>,
    @location(3) uv_max: vec2<f32>,
    @location(4) color: vec4<f32>,
    @location(5) flip: f32,
}

@vertex
fn vs_main(@builtin(vertex_index) vi: u32, inst: Instance) -> VsOut {
    var corners = array<vec2<f32>, 6>(
        vec2<f32>(0.0, 0.0),
        vec2<f32>(1.0, 0.0),
        vec2<f32>(0.0, 1.0),
        vec2<f32>(0.0, 1.0),
        vec2<f32>(1.0, 0.0),
        vec2<f32>(1.0, 1.0),
    );
    let c = corners[vi];
    let world = inst.pos + vec2<f32>(c.x * inst.size.x, c.y * inst.size.y);
    let ndc_x = (world.x - cam.cam_x) / cam.view_w * 2.0 - 1.0;
    let ndc_y = (world.y - (cam.cam_top - cam.view_h)) / cam.view_h * 2.0 - 1.0;
    var uv_x = mix(inst.uv_min.x, inst.uv_max.x, c.x);
    if (inst.flip < 0.0) {
        uv_x = mix(inst.uv_max.x, inst.uv_min.x, c.x);
    }
    let uv_y = mix(inst.uv_max.y, inst.uv_min.y, c.y);
    var out: VsOut;
    out.clip = vec4<f32>(ndc_x, ndc_y, 0.0, 1.0);
    out.uv = vec2<f32>(uv_x, uv_y);
    out.color = inst.color;
    return out;
}

@group(1) @binding(0) var sprite_tex: texture_2d<f32>;
@group(1) @binding(1) var sprite_samp: sampler;

@fragment
fn fs_main(in: VsOut) -> @location(0) vec4<f32> {
    let sample = textureSample(sprite_tex, sprite_samp, in.uv);
    let color = sample * in.color;
    if (color.a < 0.05) {
        discard;
    }
    return color;
}
"#;

struct GpuTexture {
    bind: wgpu::BindGroup,
}

pub struct Renderer {
    surface: wgpu::Surface<'static>,
    device: wgpu::Device,
    queue: wgpu::Queue,
    config: wgpu::SurfaceConfiguration,
    pipeline: wgpu::RenderPipeline,
    camera_buf: wgpu::Buffer,
    camera_bind: wgpu::BindGroup,
    sampler: wgpu::Sampler,
    tex_layout: wgpu::BindGroupLayout,
    textures: Vec<GpuTexture>,
    instance_buf: wgpu::Buffer,
}

impl Renderer {
    pub fn new<W>(window: &W, width: u32, height: u32) -> anyhow::Result<Self>
    where
        W: HasWindowHandle + HasDisplayHandle,
    {
        let instance = wgpu::Instance::new(&wgpu::InstanceDescriptor {
            backends: wgpu::Backends::all(),
            ..Default::default()
        });
        let target = unsafe { wgpu::SurfaceTargetUnsafe::from_window(window) }
            .context("window handle for wgpu surface")?;
        let surface =
            unsafe { instance.create_surface_unsafe(target) }.context("create wgpu surface")?;
        let adapter = pollster::block_on(instance.request_adapter(&wgpu::RequestAdapterOptions {
            power_preference: wgpu::PowerPreference::HighPerformance,
            compatible_surface: Some(&surface),
            force_fallback_adapter: false,
        }))
        .context("request GPU adapter")?;
        let (device, queue) = pollster::block_on(adapter.request_device(
            &wgpu::DeviceDescriptor {
                label: Some("megaman"),
                required_features: wgpu::Features::empty(),
                required_limits: wgpu::Limits::default(),
                memory_hints: wgpu::MemoryHints::default(),
            },
            None,
        ))
        .context("request GPU device")?;

        let caps = surface.get_capabilities(&adapter);
        let format = caps
            .formats
            .iter()
            .copied()
            .find(|format| format.is_srgb())
            .unwrap_or(caps.formats[0]);
        let config = wgpu::SurfaceConfiguration {
            usage: wgpu::TextureUsages::RENDER_ATTACHMENT,
            format,
            width: width.max(1),
            height: height.max(1),
            present_mode: wgpu::PresentMode::Fifo,
            alpha_mode: caps.alpha_modes[0],
            view_formats: vec![],
            desired_maximum_frame_latency: 2,
        };
        surface.configure(&device, &config);

        let camera_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("camera"),
            entries: &[wgpu::BindGroupLayoutEntry {
                binding: 0,
                visibility: wgpu::ShaderStages::VERTEX,
                ty: wgpu::BindingType::Buffer {
                    ty: wgpu::BufferBindingType::Uniform,
                    has_dynamic_offset: false,
                    min_binding_size: None,
                },
                count: None,
            }],
        });
        let tex_layout = device.create_bind_group_layout(&wgpu::BindGroupLayoutDescriptor {
            label: Some("sprite-tex"),
            entries: &[
                wgpu::BindGroupLayoutEntry {
                    binding: 0,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: wgpu::BindingType::Texture {
                        sample_type: wgpu::TextureSampleType::Float { filterable: true },
                        view_dimension: wgpu::TextureViewDimension::D2,
                        multisampled: false,
                    },
                    count: None,
                },
                wgpu::BindGroupLayoutEntry {
                    binding: 1,
                    visibility: wgpu::ShaderStages::FRAGMENT,
                    ty: wgpu::BindingType::Sampler(wgpu::SamplerBindingType::Filtering),
                    count: None,
                },
            ],
        });
        let pipeline_layout = device.create_pipeline_layout(&wgpu::PipelineLayoutDescriptor {
            label: Some("sprite"),
            bind_group_layouts: &[&camera_layout, &tex_layout],
            push_constant_ranges: &[],
        });
        let shader = device.create_shader_module(wgpu::ShaderModuleDescriptor {
            label: Some("sprite"),
            source: wgpu::ShaderSource::Wgsl(SHADER.into()),
        });
        let pipeline = device.create_render_pipeline(&wgpu::RenderPipelineDescriptor {
            label: Some("sprite"),
            layout: Some(&pipeline_layout),
            vertex: wgpu::VertexState {
                module: &shader,
                entry_point: Some("vs_main"),
                buffers: &[wgpu::VertexBufferLayout {
                    array_stride: size_of::<Instance>() as u64,
                    step_mode: wgpu::VertexStepMode::Instance,
                    attributes: &wgpu::vertex_attr_array![
                        0 => Float32x2,
                        1 => Float32x2,
                        2 => Float32x2,
                        3 => Float32x2,
                        4 => Float32x4,
                        5 => Float32
                    ],
                }],
                compilation_options: Default::default(),
            },
            fragment: Some(wgpu::FragmentState {
                module: &shader,
                entry_point: Some("fs_main"),
                targets: &[Some(wgpu::ColorTargetState {
                    format,
                    blend: Some(wgpu::BlendState::ALPHA_BLENDING),
                    write_mask: wgpu::ColorWrites::ALL,
                })],
                compilation_options: Default::default(),
            }),
            primitive: wgpu::PrimitiveState {
                topology: wgpu::PrimitiveTopology::TriangleList,
                ..Default::default()
            },
            depth_stencil: None,
            multisample: wgpu::MultisampleState::default(),
            multiview: None,
            cache: None,
        });

        let camera_buf = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("camera"),
            size: size_of::<CameraUniform>() as u64,
            usage: wgpu::BufferUsages::UNIFORM | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });
        let camera_bind = device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("camera"),
            layout: &camera_layout,
            entries: &[wgpu::BindGroupEntry {
                binding: 0,
                resource: camera_buf.as_entire_binding(),
            }],
        });
        let sampler = device.create_sampler(&wgpu::SamplerDescriptor {
            label: Some("sprite"),
            mag_filter: wgpu::FilterMode::Nearest,
            min_filter: wgpu::FilterMode::Nearest,
            ..Default::default()
        });
        let instance_buf = device.create_buffer(&wgpu::BufferDescriptor {
            label: Some("instances"),
            size: (size_of::<Instance>() * 4096) as u64,
            usage: wgpu::BufferUsages::VERTEX | wgpu::BufferUsages::COPY_DST,
            mapped_at_creation: false,
        });

        let mut renderer = Self {
            surface,
            device,
            queue,
            config,
            pipeline,
            camera_buf,
            camera_bind,
            sampler,
            tex_layout,
            textures: Vec::new(),
            instance_buf,
        };
        renderer.upload_rgba(1, 1, &[255, 255, 255, 255]);
        Ok(renderer)
    }

    pub fn resize(&mut self, width: u32, height: u32) {
        self.config.width = width.max(1);
        self.config.height = height.max(1);
        self.surface.configure(&self.device, &self.config);
    }

    pub fn upload_rgba(&mut self, width: u32, height: u32, rgba: &[u8]) -> u32 {
        let texture = self.device.create_texture(&wgpu::TextureDescriptor {
            label: Some("sprite"),
            size: wgpu::Extent3d {
                width,
                height,
                depth_or_array_layers: 1,
            },
            mip_level_count: 1,
            sample_count: 1,
            dimension: wgpu::TextureDimension::D2,
            format: wgpu::TextureFormat::Rgba8UnormSrgb,
            usage: wgpu::TextureUsages::TEXTURE_BINDING | wgpu::TextureUsages::COPY_DST,
            view_formats: &[],
        });
        let (padded, bytes) = pad_rows(width, height, rgba);
        self.queue.write_texture(
            wgpu::TexelCopyTextureInfo {
                texture: &texture,
                mip_level: 0,
                origin: wgpu::Origin3d::ZERO,
                aspect: wgpu::TextureAspect::All,
            },
            &bytes,
            wgpu::TexelCopyBufferLayout {
                offset: 0,
                bytes_per_row: Some(padded),
                rows_per_image: Some(height),
            },
            wgpu::Extent3d {
                width,
                height,
                depth_or_array_layers: 1,
            },
        );
        let view = texture.create_view(&wgpu::TextureViewDescriptor::default());
        let bind = self.device.create_bind_group(&wgpu::BindGroupDescriptor {
            label: Some("sprite"),
            layout: &self.tex_layout,
            entries: &[
                wgpu::BindGroupEntry {
                    binding: 0,
                    resource: wgpu::BindingResource::TextureView(&view),
                },
                wgpu::BindGroupEntry {
                    binding: 1,
                    resource: wgpu::BindingResource::Sampler(&self.sampler),
                },
            ],
        });
        self.textures.push(GpuTexture { bind });
        (self.textures.len() - 1) as u32
    }

    pub fn upload_images(&mut self, images: &[&CpuImage]) -> Vec<u32> {
        images
            .iter()
            .map(|image| self.upload_rgba(image.width, image.height, &image.rgba))
            .collect()
    }

    pub fn draw(&mut self, list: &DrawList) -> anyhow::Result<()> {
        let frame = match self.surface.get_current_texture() {
            Ok(frame) => frame,
            Err(wgpu::SurfaceError::Lost) | Err(wgpu::SurfaceError::Outdated) => {
                self.resize(self.config.width, self.config.height);
                return Ok(());
            }
            Err(err) => return Err(err.into()),
        };
        let view = frame
            .texture
            .create_view(&wgpu::TextureViewDescriptor::default());
        let mut encoder = self
            .device
            .create_command_encoder(&wgpu::CommandEncoderDescriptor {
                label: Some("frame"),
            });
        {
            let mut pass = encoder.begin_render_pass(&wgpu::RenderPassDescriptor {
                label: Some("sprites"),
                color_attachments: &[Some(wgpu::RenderPassColorAttachment {
                    view: &view,
                    resolve_target: None,
                    ops: wgpu::Operations {
                        load: wgpu::LoadOp::Clear(wgpu::Color {
                            r: 0.04,
                            g: 0.05,
                            b: 0.09,
                            a: 1.0,
                        }),
                        store: wgpu::StoreOp::Store,
                    },
                })],
                depth_stencil_attachment: None,
                occlusion_query_set: None,
                timestamp_writes: None,
            });
            pass.set_pipeline(&self.pipeline);
            pass.set_bind_group(0, &self.camera_bind, &[]);
            self.draw_sprites(&mut pass, &list.sprites, list.cam_x, list.cam_top, list.view_w, list.view_h);
            if !list.overlay.is_empty() {
                self.draw_sprites(
                    &mut pass,
                    &list.overlay,
                    0.0,
                    list.view_h,
                    list.view_w,
                    list.view_h,
                );
            }
        }
        self.queue.submit(std::iter::once(encoder.finish()));
        frame.present();
        Ok(())
    }

    fn draw_sprites(
        &mut self,
        pass: &mut wgpu::RenderPass<'_>,
        sprites: &[SpriteDraw],
        cam_x: f32,
        cam_top: f32,
        view_w: f32,
        view_h: f32,
    ) {
        let camera = CameraUniform {
            cam_x,
            cam_top,
            view_w,
            view_h,
        };
        self.queue
            .write_buffer(&self.camera_buf, 0, bytemuck::bytes_of(&camera));

        let mut start = 0;
        while start < sprites.len() {
            let texture = sprites[start].texture;
            let mut end = start + 1;
            while end < sprites.len() && sprites[end].texture == texture {
                end += 1;
            }
            let mut batch_start = start;
            while batch_start < end {
                let count = (end - batch_start).min(4096);
                let instances: Vec<Instance> = sprites[batch_start..batch_start + count]
                    .iter()
                    .map(|sprite| Instance {
                        pos: [sprite.x, sprite.y],
                        size: [sprite.w, sprite.h],
                        uv_min: [sprite.u0, sprite.v0],
                        uv_max: [sprite.u1, sprite.v1],
                        color: sprite.color,
                        flip: if sprite.flip_x { -1.0 } else { 1.0 },
                        _pad: [0.0; 3],
                    })
                    .collect();
                self.queue
                    .write_buffer(&self.instance_buf, 0, bytemuck::cast_slice(&instances));
                let index = texture.min(self.textures.len() as u32 - 1) as usize;
                pass.set_bind_group(1, &self.textures[index].bind, &[]);
                pass.set_vertex_buffer(0, self.instance_buf.slice(..));
                pass.draw(0..6, 0..count as u32);
                batch_start += count;
            }
            start = end;
        }
    }
}

fn pad_rows(width: u32, height: u32, rgba: &[u8]) -> (u32, Vec<u8>) {
    let unpadded = width * 4;
    let align = wgpu::COPY_BYTES_PER_ROW_ALIGNMENT;
    let padded = unpadded.div_ceil(align) * align;
    if padded == unpadded {
        return (unpadded, rgba.to_vec());
    }
    let mut out = vec![0u8; (padded * height) as usize];
    for y in 0..height {
        let src = (y * unpadded) as usize;
        let dst = (y * padded) as usize;
        out[dst..dst + unpadded as usize].copy_from_slice(&rgba[src..src + unpadded as usize]);
    }
    (padded, out)
}
