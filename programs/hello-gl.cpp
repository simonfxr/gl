#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "opengl.hpp"
#include "cutil.h"

#include "sys/clock.hpp"
#include "ge/Engine.hpp"

#include "glt/utils.hpp"

using namespace defs;
using namespace math;

struct Anim {
    ge::Engine engine;
    Ref<glt::ShaderProgram> program;

    struct GlobalResources {
	GLuint vertex_buffer, element_buffer;
	Ref<TextureSampler> textures[2];
	GLuint program;
	
	struct {
	    GLint fade_factor;
	    GLint textures[2];
	} uniforms;
	
	struct {
	    GLint position;
	} attributes;

	GLfloat fade_factor;	
    };

    GlobalResources g_resources;

    int resources;


    void init(const ge::Event<ge::InitEvent>&);
    void link();

    void renderScene(const ge::Event<ge::RenderEvent>&);
    GLuint make_buffer(GLenum target, const void *buffer_data, GLsizei buffer_size);
    Ref<glt::TextureSampler> make_texture(const char *filename);
    void show_info_log(GLuint object,
		       PFNGLGETSHADERIVPROC glGet__iv,
		       PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);
    GLuint make_shader(GLenum type, const char *filename);
    GLuint make_program(GLuint vertex_shader, GLuint fragment_shader);
    int make_resources();
};


/*
 * Functions for creating OpenGL objects:
 */
GLuint Anim::make_buffer(
    GLenum target,
    const void *buffer_data,
    GLsizei buffer_size
) {
    GLuint buffer;
    GL_CALL(glGenBuffers, 1, &buffer);
    GL_CALL(glBindBuffer, target, buffer);
    GL_CALL(glBufferData, target, buffer_size, buffer_data, GL_STATIC_DRAW);
    return buffer;
}

Ref<glt::TextureSampler> Anim::make_texture(const char *filename)
{
    int width, height;
    void *pixels = read_tga(filename, &width, &height);

    if (!pixels)
        return 0;

    Ref<glt::TextureSampler> sampler = makeRef(new glt::TextureSampler());
    
    GL_CALL(glGenTextures, 1, &texture);
    GL_CALL(glBindTexture, GL_TEXTURE_2D, texture);
    GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,     GL_CLAMP_TO_EDGE);
    GL_CALL(glTexParameteri, GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,     GL_CLAMP_TO_EDGE);

    
    GL_CALL(glTexImage2D, 
        GL_TEXTURE_2D, 0,           /* target, level */
        GL_RGB8,                    /* internal format */
        width, height, 0,           /* width, height, border */
        GL_BGR, GL_UNSIGNED_BYTE,   /* external format, type */
        pixels                      /* pixels */
    );
    free(pixels);

    return sampler;
}

void Anim::show_info_log(
    GLuint object,
    PFNGLGETSHADERIVPROC glGet__iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog
)
{
    GLint log_length;
    char *log;

    GL_CALL(glGet__iv, object, GL_INFO_LOG_LENGTH, &log_length);
    log = new char[log_length];
    GL_CALL(glGet__InfoLog, object, log_length, NULL, log);
    ERR(log);
    delete log;
}

GLuint Anim::make_shader(GLenum type, const char *filename)
{
    GLint length;
    GLchar *source = (GLchar *) file_contents(filename, &length);
    GLuint shader;
    GLint shader_ok;

    if (!source)
        return 0;

    shader = glCreateShader(type);
    GL_CALL(glShaderSource, shader, 1, (const GLchar**)&source, &length);
    free(source);
    GL_CALL(glCompileShader, shader);

    GL_CALL(glGetShaderiv, shader, GL_COMPILE_STATUS, &shader_ok);
    if (!shader_ok) {
        ERR(std::string("Failed to compile ") + filename);
        show_info_log(shader, glGetShaderiv, glGetShaderInfoLog);
        GL_CALL(glDeleteShader, shader);
        return 0;
    }
    return shader;
}

GLuint Anim::make_program(GLuint vertex_shader, GLuint fragment_shader)
{
    GLint program_ok;

    GLuint program = glCreateProgram();

    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    glGetProgramiv(program, GL_LINK_STATUS, &program_ok);
    if (!program_ok) {
	ERR("Failed to link shader program:");
        show_info_log(program, glGetProgramiv, glGetProgramInfoLog);
        glDeleteProgram(program);
        return 0;
    }
    return program;
}

/*
 * Data used to seed our vertex array and element array buffers:
 */
static const GLfloat g_vertex_buffer_data[] = { 
    -1.0f, -1.0f,
     1.0f, -1.0f,
    -1.0f,  1.0f,
     1.0f,  1.0f
};
static const GLushort g_element_buffer_data[] = { 0, 1, 2, 3 };

/*
 * Load and create all of our resources:
 */
int Anim::make_resources()
{
    g_resources.vertex_buffer = make_buffer(
        GL_ARRAY_BUFFER,
        g_vertex_buffer_data,
        sizeof(g_vertex_buffer_data)
    );
    g_resources.element_buffer = make_buffer(
        GL_ELEMENT_ARRAY_BUFFER,
        g_element_buffer_data,
        sizeof(g_element_buffer_data)
    );

    g_resources.textures[0] = make_texture("hello1.tga");
    g_resources.textures[1] = make_texture("hello2.tga");

    if (g_resources.textures[0] == 0 || g_resources.textures[1] == 0)
        return 0;

    program = makeRef(new glt::ShaderProgram(engine.shaderManager()));
    program->addShaderFilePair("hello-gl.v.glsl", "hello-gl.f.glsl", true);
    if (!program->tryLink())
	return 0;
    g_resources.program = *program->program();

    GL_ASSIGN_CALL(g_resources.uniforms.fade_factor,
		   glGetUniformLocation, g_resources.program, "fade_factor");
    GL_ASSIGN_CALL(g_resources.uniforms.textures[0],
		   glGetUniformLocation, g_resources.program, "textures[0]");
    GL_ASSIGN_CALL(g_resources.uniforms.textures[1],
		   glGetUniformLocation, g_resources.program, "textures[1]");

    GL_ASSIGN_CALL(g_resources.attributes.position,
		   glGetAttribLocation, g_resources.program, "position");

    return 1;
}

void Anim::init(const ge::Event<ge::InitEvent>& ev) {
    resources = 0;
    ev.info.success = true;
    INFO("resources loaded");
}

void Anim::link() {
    engine.events().render.reg(ge::makeEventHandler(this, &Anim::renderScene));
}

int count = 0;

void Anim::renderScene(const ge::Event<ge::RenderEvent>& ev) {

    // if (count++ > 0)
    // 	for (;;) ;
    
    if (!resources) {
	resources = 1;
	make_resources();
    }

    ge::Engine& engine = ev.info.engine;
    glt::RenderTarget *rt = engine.renderManager().activeRenderTarget();
    
    rt->clearColor(glt::color(vec4(real(1))));
    rt->clear();

    float secs = (float) sys::queryTimer();
    g_resources.fade_factor = sinf(secs * 0.5f + 0.5f);

    program->use();
    glt::Uniforms(program)
	.optional("fade_factor", g_resources.fade_factor)
	.optional("textures[0]", glt::Sampler(g_resources.textures[0], 0))
	.optional("textures[1]", glt::Sampler(g_resources.textures[1], 1));
    
    glBindBuffer(GL_ARRAY_BUFFER, g_resources.vertex_buffer);
    glVertexAttribPointer(
        g_resources.attributes.position,  /* attribute */
        2,                                /* size */
        GL_FLOAT,                         /* type */
        GL_FALSE,                         /* normalized? */
        sizeof(GLfloat)*2,                /* stride */
        (void*)0                          /* array buffer offset */
    );
    glEnableVertexAttribArray(g_resources.attributes.position);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, g_resources.element_buffer);
    glDrawElements(
        GL_TRIANGLE_STRIP,  /* mode */
        4,                  /* count */
        GL_UNSIGNED_SHORT,  /* type */
        (void*)0            /* element array buffer offset */
    );

    glDisableVertexAttribArray(g_resources.attributes.position);
}

int main(int argc, char *argv[]) {
    ge::EngineOptions opts;
    Anim anim;
    anim.resources = 0;
    anim.link();
    opts.parse(&argc, &argv);
    return anim.engine.run(opts);
}
