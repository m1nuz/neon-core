#pragma once

#include <stddef.h>
#include <video/gl.h>

typedef struct Renderbuffer {
    GLuint          id;
    GLsizei         width;
    GLsizei         height;
    GLsizei         samples;

    uint32_t        usage;
} RENDERBUFFER;

typedef struct FramebufferAttachment {
    GLuint          attachment;
    GLenum          target;
    GLuint          texture;
} FRAMEBUFFER_ATTACHMENT;


typedef struct FramebufferRead {
    uint32_t mode;
} FRAMEBUFFER_READ;

typedef struct FramebufferDraw {
    size_t n;
    uint32_t bufs[8];
} FRAMEBUFFER_DRAW;

typedef struct FramebufferInfo {
    FRAMEBUFFER_ATTACHMENT  attachments[8]; // TODO: see GL_MAX_COLOR_ATTACHMENTS and video.c max_color_attachments
    uint32_t                attachments_count;
    FRAMEBUFFER_READ        read;
    FRAMEBUFFER_DRAW        draw;
    uint32_t                mask;
    int                     width;
    int                     height;
} FRAMEBUFFER_INFO;

typedef struct Framebuffer {
    GLuint          id;
    GLsizei         width;
    GLsizei         height;
    GLbitfield      mask;
    float           color[4];
    int             viewport[4];
    uint32_t        usage;
} FRAMEBUFFER;

RENDERBUFFER new_renderbuffer(GLenum internalformat, GLsizei width, GLsizei height, GLsizei samples);
void free_renderbuffer(RENDERBUFFER *renderbuffer);

FRAMEBUFFER new_framebuffer(const FRAMEBUFFER_ATTACHMENT *attachments, size_t count, GLsizei width, GLsizei height, GLbitfield mask);
FRAMEBUFFER new_framebuffer_ext(const FRAMEBUFFER_INFO *info);
void free_framebuffer(FRAMEBUFFER *framebuffer);
