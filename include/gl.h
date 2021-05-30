//
// Created by grant on 5/29/21.
//

#pragma once

#ifndef GAMETEST_GL_H
#define GAMETEST_GL_H

#include "linalg.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

typedef struct {
    smat4 transform;
    vec2 sampling_bottom_left;
    vec2 sampling_extent;
} draw_instance_t;

typedef struct {
    GLuint tex;
} texture_t;

typedef struct {
    GLuint framebuffer;
    // we aren't using depth or stencil so no render buffer object

    GLuint instance_vbo;
    unsigned num_blits;
    unsigned max_blits;
    GLsizeiptr vbo_size;

    GLuint vao;
} render_ctx_t;

void new_framebuffer(GLuint *buf);

void compile_and_check_shader(GLuint shader);

void render(render_ctx_t *ctx);
void fill_vao_and_vbo(GLuint global_vbo, render_ctx_t *ctx, unsigned max_blits);

#endif //GAMETEST_GL_H
