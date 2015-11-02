#include <assert.h>
#include "video/framebuffer.h"
#include "core/logerr.h"

extern RENDERBUFFER
new_renderbuffer(GLenum internalformat, GLsizei width, GLsizei height, GLsizei samples) {
    GLuint buffer = 0;
    glGenRenderbuffers(1, &buffer);
    glBindRenderbuffer(GL_RENDERBUFFER, buffer);

#ifdef GL_ES_VERSION_2_0
    glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
#else
    if (samples == 0)
        glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    else
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internalformat, width, height);
#endif // NO GL_ES_VERSION_2_0

    return (RENDERBUFFER){.id = buffer, .width = width, .height = height, .samples = samples};
}

extern void
free_renderbuffer(RENDERBUFFER *renderbuffer) {
    assert(renderbuffer != NULL);

    if (glIsRenderbuffer(renderbuffer->id))
        glDeleteRenderbuffers(1, &renderbuffer->id);

    renderbuffer->id = 0;
}

static uint32_t
framebuffer_check(void) {
    const GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

#ifdef GL_ES_VERSION_2_0
    if (status != GL_FRAMEBUFFER_COMPLETE)
        LOG_ERROR("Framebuffer incomplete %#x\n", status);
#else
    switch (status) {
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE:
    case GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME:
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL:
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE:
    case GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER:
    case GL_FRAMEBUFFER_COMPLETE:
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
        LOG_ERROR("%s\n", "Setup FBO failed. Duplicate attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
        LOG_ERROR("%s\n", "Setup FBO failed. Missing attachment.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER:
        LOG_ERROR("%s\n", "Setup FBO failed. Missing draw buffer.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER:
        LOG_ERROR("%s\n", "Setup FBO failed. Missing read buffer.");
        break;
    case GL_FRAMEBUFFER_UNSUPPORTED:
        LOG_ERROR("%s\n", "Setup FBO failed. Unsupported framebuffer format.");
        break;
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
        LOG_ERROR("%s\n", "Setup FBO failed. Attached images must have the same number of samples.");
        break;
    default:
        LOG_CRITICAL("%s\n", "Setup FBO failed. Fatal error.");
        exit(EXIT_FAILURE);
    }
#endif // NO #ifdef GL_ES_VERSION_2_0

    return status;
}

extern FRAMEBUFFER
new_framebuffer(const FRAMEBUFFER_ATTACHMENT *attachments, size_t count, GLsizei width, GLsizei height, GLbitfield mask) {
    GLuint framebuffer = 0;

    if (attachments) {
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        for (size_t i = 0; i < count; i++) {

            switch (attachments[i].target) {
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
                glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i].attachment, attachments[i].target, attachments[i].texture, 0);
                break;
            case GL_TEXTURE_CUBE_MAP:
                for (int j = 0; j < 6; j++)
                    glFramebufferTexture2D(GL_FRAMEBUFFER, attachments[i].attachment + j, GL_TEXTURE_CUBE_MAP_POSITIVE_X + j, attachments[i].texture, 0);
                break;
#ifndef GL_ES_VERSION_2_0
            case GL_TEXTURE:
                glFramebufferTexture(GL_FRAMEBUFFER, attachments[i].attachment, attachments[i].texture, 0);
                break;
#endif // NO GL_ES_VERSION_2_0
            case GL_RENDERBUFFER:
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachments[i].attachment, GL_RENDERBUFFER, attachments[i].texture);
                break;
            }
        }
    }

    framebuffer_check();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    LOG("NEW FRAMEBUFFER %u\n", framebuffer);

    return (FRAMEBUFFER){.id = framebuffer,
                .width = width,
                .height = height,
                .mask = mask,
                .viewport = {0, 0, width, height}};
}

extern FRAMEBUFFER
new_framebuffer_ext(const FRAMEBUFFER_INFO *info) {
    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    for (uint32_t i = 0; i < info->attachments_count; i++) {
        switch (info->attachments[i].target) {
        case GL_TEXTURE_2D:
            glFramebufferTexture2D(GL_FRAMEBUFFER, info->attachments[i].attachment, GL_TEXTURE_2D, info->attachments[i].texture, 0);
            break;
#ifndef GL_ES_VERSION_2_0
        case GL_TEXTURE:
            glFramebufferTexture(GL_FRAMEBUFFER, info->attachments[i].attachment, info->attachments[i].texture, 0);
            break;
#endif // NO GL_ES_VERSION_2_0
        case GL_RENDERBUFFER:
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, info->attachments[i].attachment, GL_RENDERBUFFER, info->attachments[i].texture);
            break;
        }
    }

    // The initial value is GL_FRONT for single-buffered contexts, and GL_BACK for double-buffered contexts
    //glDrawBuffers(info->draw.n, info->draw.bufs);
    //glReadBuffer(info->read.mode);

    framebuffer_check();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    LOG("NEW FRAMEBUFFER EXT %u\n", framebuffer);

    return (FRAMEBUFFER){.id = framebuffer,
                .width = info->width,
                .height = info->height,
                .mask = info->mask,
                .viewport = {0, 0, info->width, info->height}};
}

extern void
free_framebuffer(FRAMEBUFFER *framebuffer) {
    assert(framebuffer != NULL);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (glIsFramebuffer(framebuffer->id))
        glDeleteFramebuffers(1, &framebuffer->id);

    framebuffer->id = 0;
}
