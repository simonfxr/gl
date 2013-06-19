#include <sstream>
#include <cstring>
#include <set>

#include "defs.hpp"
#include "opengl.hpp"

#include "glt/utils.hpp"
#include "err/err.hpp"

namespace glt {

namespace {

bool print_opengl_calls = false;

};

bool printOpenGLCalls() {
    return print_opengl_calls;
}

void printOpenGLCalls(bool yesno) {
    print_opengl_calls = yesno;
}

std::string getGLErrorString(GLenum err) {

    switch (err) {
            
#define err_case(e) case e: return #e
        
        err_case(GL_NO_ERROR);
        err_case(GL_INVALID_ENUM);
        err_case(GL_INVALID_VALUE);
        err_case(GL_INVALID_OPERATION);
        err_case(GL_STACK_OVERFLOW);
        err_case(GL_STACK_UNDERFLOW);
        err_case(GL_OUT_OF_MEMORY);
        err_case(GL_TABLE_TOO_LARGE);
        err_case(GL_INVALID_FRAMEBUFFER_OPERATION);

#undef err_case

    default:

        std::ostringstream rep;
        rep << "Unknown OpenGL error [code = " << err << "]";
        return rep.str();
    }
}

bool printGLErrors(sys::io::OutStream& out) {
    bool was_error = false;
    
    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        out << "OpenGL error occurred: " << getGLErrorString(err) << sys::io::endl;
    
    return was_error;
}

bool isExtensionSupported(const char *extension) {
    GLint ni;
    glGetIntegerv(GL_NUM_EXTENSIONS, &ni);
    GLuint n = GLuint(ni);
    
    for(GLuint i = 0; i < n; i++)
        if(strcmp(extension, gl_unstr(glGetStringi(GL_EXTENSIONS, i))) == 0)
            return true;

    return false;
}

void printGLError(const err::Location& loc, GLenum err);

namespace {

struct GLDebug {

    std::set<GLint> ignored;
    OpenGLVendor vendor;

    GLDebug();
    virtual ~GLDebug() {}
    void init();
    virtual void printDebugMessages(const err::Location&) = 0;
    bool shouldIgnore(GLint);
    void ignoreMessage(OpenGLVendor vendor, GLuint id);

private:
    GLDebug(const GLDebug&);
    GLDebug& operator =(const GLDebug &);
};

GLDebug::GLDebug() {
    vendor = glvendor::Unknown;
}

void GLDebug::init() {
    const GLubyte *gl_vendor_str = glGetString(GL_VENDOR);
    if (gl_vendor_str == 0) {
        vendor = glvendor::Unknown;
        return;
    }

    const char *vendor_str = reinterpret_cast<const char *>(gl_vendor_str);
    if (strcmp(vendor_str, "NVIDIA Corporation") == 0)
        vendor = glvendor::Nvidia;
    else if (strcmp(vendor_str, "ATI Technologies") == 0)
        vendor = glvendor::ATI;
    else if (strcmp(vendor_str, "INTEL") == 0)
        vendor = glvendor::Intel;
    else
        vendor = glvendor::Unknown;
}

bool GLDebug::shouldIgnore(GLint id) {
    return ignored.find(id) != ignored.end();
}

void GLDebug::ignoreMessage(OpenGLVendor id_vendor, GLuint id) {
    INFO("called ignoreMessage");
    if (vendor != glvendor::Unknown && vendor == id_vendor) {
        ignored.insert(id);
    }
}

struct NoDebug : public GLDebug {
    NoDebug() {}
    virtual void printDebugMessages(const err::Location&) FINAL OVERRIDE {}

private:
    NoDebug(const NoDebug&);
    NoDebug& operator =(const NoDebug&);
};

struct ARBDebug : public GLDebug {
    GLsizei message_buffer_length;
    char *message_buffer;

    explicit ARBDebug(GLsizei buf_len) :
        message_buffer_length(buf_len),
        message_buffer(new char[size_t(buf_len)])
        { GLDebug::init(); }
    
    ~ARBDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc) FINAL OVERRIDE;
    
private:
    ARBDebug(const ARBDebug&);
    ARBDebug& operator =(const ARBDebug&);
};

ARBDebug::~ARBDebug() {
    delete[] message_buffer;
}

GLDebug *ARBDebug::init() {
    glDebugMessageControlARB(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, NULL, GL_TRUE);
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(_CURRENT_LOCATION, err);
        return 0;
    }

    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS_ARB);

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_ARB, &max_len);
    
    return new ARBDebug(max_len);
}

void ARBDebug::printDebugMessages(const err::Location& loc) {

    GLenum source, type, id, severity;
    GLsizei length;

    while (glGetDebugMessageLogARB(1, message_buffer_length,
                                   &source, &type, &id, &severity,
                                   &length, message_buffer) > 0) {

        if (shouldIgnore(id))
            continue;

#define sym_case(v, c) case c: v = #c; break

        const char *ssrc = "unknown";
        switch (source) {
            sym_case(ssrc, GL_DEBUG_SOURCE_API_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_SHADER_COMPILER_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_THIRD_PARTY_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_APPLICATION_ARB);
            sym_case(ssrc, GL_DEBUG_SOURCE_OTHER_ARB);
        }

        const char *stype = "unknown";
        switch (type) {
            sym_case(stype, GL_DEBUG_TYPE_ERROR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PORTABILITY_ARB);
            sym_case(stype, GL_DEBUG_TYPE_PERFORMANCE_ARB);
            sym_case(stype, GL_DEBUG_TYPE_OTHER_ARB);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_ARB);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_ARB);
        }
        
#undef sym_case

        std::stringstream mesg;
        mesg << "source: " << ssrc << ", severity: " << ssev
             << ", type: " << stype << ", id: " << id
             << "  message: " << message_buffer;

        err::printError(sys::io::stdout(), "OpenGL DEBUG", loc, err::Error, mesg.str().c_str());               }
}

struct AMDDebug : public GLDebug {

    GLsizei message_buffer_length;
    char *message_buffer;

    explicit AMDDebug(GLsizei buf_len) :
        message_buffer_length(buf_len),
        message_buffer(new char[size_t(buf_len)])
        { GLDebug::init(); }
    
    ~AMDDebug();
    
    static GLDebug* init();
    virtual void printDebugMessages(const err::Location& loc) FINAL OVERRIDE;

private:
    AMDDebug(const AMDDebug&);
    AMDDebug& operator =(const AMDDebug&);
};

AMDDebug::~AMDDebug() {
    delete[] message_buffer;
}

GLDebug *AMDDebug::init() {
    glDebugMessageEnableAMD(0, 0, 0, NULL, GL_TRUE);

    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        printGLError(_CURRENT_LOCATION, err);
        return 0;
    }

    GLsizei max_len;
    glGetIntegerv(GL_MAX_DEBUG_MESSAGE_LENGTH_AMD, &max_len);
    
    return new AMDDebug(max_len);
}

void AMDDebug::printDebugMessages(const err::Location& loc) {

    GLenum category;
    GLuint severity, id;
    GLsizei length;
        
    while (glGetDebugMessageLogAMD(1, message_buffer_length,
                                   &category, &severity, &id, &length,
                                   message_buffer) > 0) {
        
        if (shouldIgnore(id))
            continue;
        
#define sym_case(v, c) case c: v = #c; break
        
        const char *scat = "unknown";
        switch (category) {
            sym_case(scat, GL_DEBUG_CATEGORY_API_ERROR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_WINDOW_SYSTEM_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_DEPRECATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_UNDEFINED_BEHAVIOR_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_PERFORMANCE_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_SHADER_COMPILER_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_APPLICATION_AMD);
            sym_case(scat, GL_DEBUG_CATEGORY_OTHER_AMD);
        }

        const char *ssev = "unknown";
        switch (severity) {
            sym_case(ssev, GL_DEBUG_SEVERITY_HIGH_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_MEDIUM_AMD);
            sym_case(ssev, GL_DEBUG_SEVERITY_LOW_AMD);
        }

#undef sym_case

        std::stringstream mesg;
        mesg << "category: " << scat << ", severity: " << ssev
             << ", id: " << id << "  message: " << message_buffer;

        err::printError(sys::io::stdout(), "OpenGL DEBUG", loc, err::Error, mesg.str().c_str());
    }
}

GLDebug *glDebug = new NoDebug();

} // namespace anon

void ignoreDebugMessage(OpenGLVendor vendor, GLuint id) {
    if (glDebug)
        glDebug->ignoreMessage(vendor, id);
}

void printGLError(const err::Location& loc, GLenum err) {
    err::printError(sys::io::stdout(), "OpenGL Error", loc, err::Error, getGLErrorString(err).c_str());
}

void printGLTrace(const err::Location& loc) {
    if (print_opengl_calls)
        sys::io::stdout() << "OPENGL " << loc.operation << " " << loc.file << ":" << loc.line << sys::io::endl;
}

bool checkForGLError(const err::Location& loc) {

    if (print_opengl_calls && loc.operation != 0) {
        printGLTrace(loc);
    }
    
    bool was_error = false;

    for (GLenum err; (err = glGetError()) != GL_NO_ERROR; was_error = true)
        printGLError(loc, err);

    glDebug->printDebugMessages(loc);
    
    return was_error;
}

bool initDebug() {

    GLDebug *dbg = 0;
    const char *debug_impl = 0;

    if (dbg == 0 && GLEW_AMD_debug_output) {
        debug_impl = "GL_AMD_debug_output";
        sys::io::stdout() << "trying AMD debug" << sys::io::endl;
        dbg = AMDDebug::init();
    }

    if (dbg == 0 && GLEW_ARB_debug_output) {
        debug_impl = "GL_ARB_debug_output";
        sys::io::stdout() << "trying ARB debug" << sys::io::endl;
        dbg = ARBDebug::init();
    }
    
    bool initialized;

    if (dbg == 0) {
        sys::io::stdout() << "couldnt initialize Debug Output, no debug implementaion available" << sys::io::endl;
        dbg = new NoDebug();
        initialized = false;
    } else {
        sys::io::stdout() << "initialized Debug Output using " << debug_impl << sys::io::endl;
        initialized = true;
    }

    delete glDebug;
    glDebug = dbg;
    return initialized;
}

namespace {

bool ati_mem_info_initialized = false;
bool ati_mem_info_available = false;
GLMemInfoATI::GLMemFree initial_free;

enum MemInfoField {
    FREE_TOTAL,
    FREE_MAX_BLOCK,
    FREE_AUX_TOTAL,
    FREE_AUX_MAX_BLOCK,
    FREE_COUNT
};

void queryMemFreeATI(GLMemInfoATI::GLMemFree *free) {
    GLint info[FREE_COUNT];
    GL_CALL(glGetIntegerv, GL_VBO_FREE_MEMORY_ATI, info);
    free->freeVBO = info[FREE_TOTAL];
    GL_CALL(glGetIntegerv, GL_TEXTURE_FREE_MEMORY_ATI, info);
    free->freeTexture = info[FREE_TOTAL];
    GL_CALL(glGetIntegerv, GL_RENDERBUFFER_FREE_MEMORY_ATI, info);
    free->freeRenderbuffer = info[FREE_TOTAL];    
}

} // namespace anon

bool GLMemInfoATI::init() {
    if (ati_mem_info_initialized)
        return ati_mem_info_available;
    ati_mem_info_initialized = true;
    ati_mem_info_available = bool(GLEW_ATI_meminfo);
    if (ati_mem_info_available)
        queryMemFreeATI(&initial_free);
    return ati_mem_info_available;
}

bool GLMemInfoATI::info(GLMemInfoATI *mi) {
    
    if (!ati_mem_info_initialized) {
        ERR("GLMemInfoATI not initialized");
        return false;
    }

    if (!ati_mem_info_available)
        return false;

    queryMemFreeATI(&mi->current);
    mi->initial = initial_free;
    return true;
}

namespace {
bool nv_mem_info_initialized = false;
bool nv_mem_info_available = false;

} // namespace anon

bool GLMemInfoNV::init() {
    if (nv_mem_info_initialized)
        return nv_mem_info_available;
    nv_mem_info_initialized = true;
    nv_mem_info_available = bool(GLEW_NVX_gpu_memory_info);
    return nv_mem_info_available;
}

bool GLMemInfoNV::info(GLMemInfoNV *mi) {
    if (!nv_mem_info_initialized) {
        ERR("GLMemInfoNV not initialized");
        return false;
    }

    if (!nv_mem_info_available)
        return false;

    GLint total, total_dedicated, current, evicted, num_evictions;
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_DEDICATED_VIDMEM_NVX, &total_dedicated);
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_TOTAL_AVAILABLE_MEMORY_NVX, &total);
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_CURRENT_AVAILABLE_VIDMEM_NVX, &current);
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_EVICTION_COUNT_NVX, &num_evictions);
    GL_CALL(glGetIntegerv, GL_GPU_MEMORY_INFO_EVICTED_MEMORY_NVX, &evicted);

    mi->total = total;
    mi->total_dedicated = total_dedicated;
    mi->current = current;
    mi->evicted = evicted;
    mi->num_evictions = num_evictions;
    return true;
}

} // namespace glt
