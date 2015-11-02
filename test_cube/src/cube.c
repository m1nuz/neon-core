/*
 * Engine test
 */
#include <stdbool.h>
#include <memtrack.h>

#include <neon.h>
#include <video/gl.h>
#include <video/resources.h>
#include <video/gfx.h>

#ifdef GL_ES_VERSION_2_0
#define SHADER_NAMES "es2.base_model.vert es2.texture_material.frag"
#define glDepthRange _glDepthRangef
#else
#define SHADER_NAMES "base_model.vert texture_material.frag"
#endif

SHADER *shader;
TEXTURE *texture;
SAMPLER *sampler;
VIDEO_VERTICES_INFO cube;
VERTICES_INFO cube_info;

static float rotation_angle = 0.f;

int
gameplay_on_init(void) {
    checkif(asset_open("../assets/test") == -1, "%s\n", "Can\'t open asset");

    cube_info = vertgen_cube((matrix4){IDENTITY_MATRIX4});
    cube = new_vertices_buffers(&cube_info.data, 1, &cube_info.desc);
    shader = store_shader(upload_shader(SHADER_NAMES));
    texture = store_texture(upload_texture("texture.tga"));
    sampler = store_new_sampler(.mag_filter = GL_LINEAR, .min_filter = GL_LINEAR_MIPMAP_LINEAR, .wrap = GL_CLAMP_TO_EDGE, .anisotropy = 16);

    return 0;
}

void
gameplay_on_update(float dt) {
    rotation_angle += 0.005f;
}

void
gameplay_on_present(int w, int h, float alpha) {
    const float angle = MIX(rotation_angle, rotation_angle - 0.005f, alpha);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    glDepthRange(0.f, 100.f);

    glClearColor(0.4f, 0.4f, 0.4f, 1.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, screen.width, screen.height);

    matrix4 projection = {IDENTITY_MATRIX4};
    perspective_matrix4(projection, 45.f, screen.aspect, 0.1f, 100.f);

    matrix4 view = {IDENTITY_MATRIX4};
    float3 eye = {0, 0, -8};
    translate_matrix4(view, eye);
    rotate_z_matrix4(view, 0);
    rotate_y_matrix4(view, 0);
    rotate_x_matrix4(view, 0);

    matrix4 projection_view = {IDENTITY_MATRIX4};
    multiply_matrix4(projection_view, projection);
    multiply_matrix4(projection_view, view);

    matrix4 model = {IDENTITY_MATRIX4};
    rotate_z_matrix4(model, angle);
    rotate_y_matrix4(model, 0);
    rotate_x_matrix4(model, angle);

    gfx_use_shader(shader);
    gfx_uniform1i(shader, "color_map", 0);
    gfx_uniform4f(shader, "color", 0.75, 0.8, 0.75, 1);
    gfx_uniform_matrix4f(shader, "projection_view", &projection_view[0][0]);
    gfx_uniform_matrix4f(shader, "model", &model[0][0]);

    bind_texture(0, texture);
    bind_sampler(0, sampler);

    bind_vertex_arrays(&cube.array);

    glDrawElements(cube.desc.primitive, cube.objects[0].count, GL_UNSIGNED_SHORT, 0);

    unbind_vertex_array(&cube.array);

    unbind_texture(0, GL_TEXTURE_2D);

    glDisable(GL_DEPTH_TEST);
}

void
gameplay_on_event(const SDL_Event *event) {
    if (event->type == SDL_KEYDOWN)
        if(event->key.keysym.sym == SDLK_ESCAPE)
            application_quit();

    if (event->type == SDL_QUIT)
        application_quit();
}

void
gameplay_on_cleanup(void) {
    cleanup_vertices_info(&cube);
    free_vertices_info(&cube);
}

int game_startup(int argc, char *argv[]) {
    screen.width = 1280;
    screen.height = 720;
    screen.srgb_capable = false;
    screen.msaa = 4;

    return 0;
}
