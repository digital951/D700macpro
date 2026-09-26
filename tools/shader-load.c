/* SPDX-License-Identifier: GPL-2.0-only */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

typedef void *EGLDisplay;
typedef void *EGLConfig;
typedef void *EGLSurface;
typedef void *EGLContext;
extern EGLDisplay eglGetDisplay(void *);
extern unsigned eglInitialize(EGLDisplay, int *, int *), eglBindAPI(unsigned);
extern unsigned eglChooseConfig(EGLDisplay, const int *, EGLConfig *, int, int *);
extern unsigned eglMakeCurrent(EGLDisplay, EGLSurface, EGLSurface, EGLContext);
extern unsigned eglDestroySurface(EGLDisplay, EGLSurface);
extern unsigned eglDestroyContext(EGLDisplay, EGLContext), eglTerminate(EGLDisplay);
extern EGLSurface eglCreatePbufferSurface(EGLDisplay, EGLConfig, const int *);
extern EGLContext eglCreateContext(EGLDisplay, EGLConfig, EGLContext, const int *);
extern const unsigned char *glGetString(unsigned);
extern unsigned glCreateShader(unsigned), glCreateProgram(void), glGetError(void);
extern void glShaderSource(unsigned, int, const char *const *, const int *);
extern void glCompileShader(unsigned), glGetShaderiv(unsigned, unsigned, int *);
extern void glGetShaderInfoLog(unsigned, int, int *, char *);
extern void glAttachShader(unsigned, unsigned), glLinkProgram(unsigned);
extern void glGetProgramiv(unsigned, unsigned, int *);
extern void glGetProgramInfoLog(unsigned, int, int *, char *);
extern void glUseProgram(unsigned), glViewport(int, int, int, int);
extern void glDrawArrays(unsigned, int, int), glFinish(void);
extern void glDeleteProgram(unsigned), glDeleteShader(unsigned);

static unsigned compile_shader(unsigned type, const char *source)
{
	unsigned shader = glCreateShader(type);
	glShaderSource(shader, 1, &source, NULL);
	glCompileShader(shader);
	int success = 0;
	glGetShaderiv(shader, 0x8b81, &success);
	if (!success) {
		char log[2048];
		glGetShaderInfoLog(shader, sizeof(log), NULL, log);
		fprintf(stderr, "Shader compilation: %s\n", log);
		exit(1);
	}
	return shader;
}

static double monotonic_seconds(void)
{
	struct timespec now;
	clock_gettime(CLOCK_MONOTONIC, &now);
	return now.tv_sec + now.tv_nsec / 1e9;
}

int main(void)
{
	EGLDisplay display = eglGetDisplay(NULL);
	int major, minor;
	if (!eglInitialize(display, &major, &minor) || !eglBindAPI(0x30a0)) {
		fprintf(stderr, "EGL initialization failed\n");
		return 1;
	}
	const int config_attrs[] = {0x3033, 1, 0x3040, 0x40,
		0x3024, 8, 0x3023, 8, 0x3022, 8, 0x3038};
	EGLConfig config;
	int count;
	if (!eglChooseConfig(display, config_attrs, &config, 1, &count) || count < 1)
		return 1;
	const int surface_attrs[] = {0x3057, 1280, 0x3056, 720, 0x3038};
	const int context_attrs[] = {0x3098, 3, 0x3038};
	EGLSurface surface = eglCreatePbufferSurface(display, config, surface_attrs);
	EGLContext context = eglCreateContext(display, config, NULL, context_attrs);
	if (!surface || !context || !eglMakeCurrent(display, surface, surface, context))
		return 1;
	const char *renderer = (const char *)glGetString(0x1f01);
	if (!renderer || strstr(renderer, "llvmpipe") || strstr(renderer, "softpipe"))
		return 1;
	printf("Renderer: %s\n", renderer);
	fflush(stdout);
	const char *vertex = "#version 300 es\n"
		"void main(){vec2 p=vec2((gl_VertexID<<1)&2,gl_VertexID&2);"
		"gl_Position=vec4(p*2.0-1.0,0,1);}";
	const char *fragment = "#version 300 es\nprecision highp float;"
		"out vec4 color;void main(){vec4 x=vec4(gl_FragCoord.xy*0.001,0.37,0.61);"
		"for(int i=0;i<256;i++){x=sin(x.yzwx*1.01+x*0.99+"
		"vec4(0.01,0.02,0.03,0.04));}color=x*0.5+0.5;}";
	unsigned v = compile_shader(0x8b31, vertex);
	unsigned f = compile_shader(0x8b30, fragment);
	unsigned program = glCreateProgram();
	glAttachShader(program, v);
	glAttachShader(program, f);
	glLinkProgram(program);
	int linked = 0;
	glGetProgramiv(program, 0x8b82, &linked);
	if (!linked) {
		char log[2048];
		glGetProgramInfoLog(program, sizeof(log), NULL, log);
		fprintf(stderr, "Program link: %s\n", log);
		return 1;
	}
	glUseProgram(program);
	glViewport(0, 0, 1280, 720);
	double start = monotonic_seconds();
	unsigned frames = 0;
	while (monotonic_seconds() - start < 10.0) {
		glDrawArrays(4, 0, 3);
		glFinish();
		frames++;
		if (glGetError()) return 1;
	}
	printf("Completed %u shader-heavy frames in %.2f seconds\n",
		frames, monotonic_seconds() - start);
	glDeleteProgram(program);
	glDeleteShader(v);
	glDeleteShader(f);
	eglMakeCurrent(display, NULL, NULL, NULL);
	eglDestroyContext(display, context);
	eglDestroySurface(display, surface);
	eglTerminate(display);
	return 0;
}
