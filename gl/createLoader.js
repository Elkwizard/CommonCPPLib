const fs = require("fs");

const file = fs.readFileSync("C:\\Program Files (x86)\\Windows Kits\\10\\Include\\10.0.18362.0\\um\\gl\\glcorearb.h", "utf-8");

const fns = Array.from(file.matchAll(/GLAPI (void|GL\w+) APIENTRY gl(\w+) \(/g))
	.map(fn => fn[2]);
const procs = Array.from(file.matchAll(/APIENTRYP PFNGL(\w+)PROC/g))
	.map(fn => fn[1])
	.map(fn => fns.find(name => name.toUpperCase() === fn))
	.filter(n => !!n);
const constants = Array.from(file.matchAll(/\#define GL_(\w+)\s+0x/g))
	.map(fn => fn[1]);

const varType = name => `PFNGL${name.toUpperCase()}PROC`;
const varName = name => name[0].toLowerCase() + name.slice(1);

const decls = procs
	.map(name => `${varType(name)} ${varName(name)};`);

const inits = procs
	.map(name => `${varName(name)} = (${varType(name)})loadProc("gl${name}");`);

const constDecls = constants
	.map(name => `GLenum ${name} = GL_${name};`);

const loaderHPP = `
#pragma once

#include <gl/glcorearb.h>

#pragma comment(lib, "Opengl32")
#pragma comment(lib, "Kernel32")

namespace gl {
	class LoadedGL {
		private:
			static void* loadProc(const char *name) {
				void* p = (void*)wglGetProcAddress(name);
				if (
					p == (void*)-1 ||
					p == (void*)0 ||
					p == (void*)1 ||
					p == (void*)2 ||
					p == (void*)3
				) {
					HMODULE mdl = LoadLibraryA("opengl32.dll");
					p = (void*)GetProcAddress(mdl, name);
				}
		
				return p;
			}

		public:
${decls.map(decl => "\t\t\t" + decl).join("\n")}

			LoadedGL() { }
			void loadProcedures() {
${inits.map(init => "\t\t\t\t" + init).join("\n")}
			}
	};
}`;
fs.writeFile("./loader.hpp", loaderHPP, "utf-8", err => console.error(err));