
#include "Render/GL/GL_Shader.h"
#include "Render/GL/GL_Shaders.h"

namespace Scaleform { namespace Render { namespace GL {

const char* pSource_FTexTGBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"\n"
"}";

const char* pSource_FTexTGBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"\n"
"}";

const char* pSource_FTexTGBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"\n"
"}";

const char* pSource_FTexTGCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTG = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"\n"
"}";

const char* pSource_FTexTGMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"\n"
"}";

const char* pSource_FVertexBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexBatch = 
"uniform vec4 ffuniforms[60];\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"\n"
"}";

const char* pSource_FVertexBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"\n"
"}";

const char* pSource_FVertexCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertex = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"\n"
"}";

const char* pSource_FVertexMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGTexTGBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGTexTGBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGTexTGBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGTexTGBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGTexTGCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGTexTGCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGTexTG = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGTexTGMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex0;\n"
"uniform sampler2D tex1;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S TexTG\n"
"  fcolor1 = texture2D(tex1,tc1);\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGVertexBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGVertexBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGVertexBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGVertexBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGVertexCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGVertexCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTexTGVertex = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FTexTGVertexMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex0;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"  vec4 fcolor1;\n"
"  vec4 fcolor0;\n"
"// S TexTG\n"
"  fcolor0 = texture2D(tex0,tc0);\n"
"// S Vertex\n"
"  fcolor1 = color;\n"
"  gl_FragColor = mix(fcolor1, fcolor0, factor.r);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexVertexBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FVertexVertexBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexVertexBatch = 
"uniform vec4 ffuniforms[60];\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FVertexVertexBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexVertexCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FVertexVertexCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FVertexVertex = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"\n"
"}";

const char* pSource_FVertexVertexMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"  vec4 fcolor0;\n"
"// S Vertex\n"
"  gl_FragColor = color;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E factora\n"
"  gl_FragColor.a *= factor.a;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_VBatchPosition3dVacolor = 
"uniform mat4 vfmuniforms[30];\n"
"attribute vec4 acolor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E \n"
"  color = acolor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchPosition3dVacolorVafactor = 
"uniform mat4 vfmuniforms[30];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchVacolor = 
"uniform vec4 vfuniforms[60];\n"
"attribute vec4 acolor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E \n"
"  color = acolor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchVacolorVafactor = 
"uniform vec4 vfuniforms[60];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VPosition3dVacolor = 
"uniform mat4 mvp;\n"
"attribute vec4 acolor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E \n"
"  color = acolor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VPosition3dVacolorVafactor = 
"uniform mat4 mvp;\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VTexTGBatchPosition3d = 
"uniform mat4 vfmuniforms[30];\n"
"uniform vec4 vfuniforms[60];\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"\n"
"  tc.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  tc.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGBatchPosition3dVacolorVafactor = 
"uniform mat4 vfmuniforms[30];\n"
"uniform vec4 vfuniforms[60];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  tc0.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGBatch = 
"uniform vec4 vfuniforms[120];\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TexTG\n"
"\n"
"  tc.x = dot(pos, vfuniforms[int(0.1+4*batch+0+(0))]);\n"
"  tc.y = dot(pos, vfuniforms[int(0.1+4*batch+0+(1))]);\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+4*batch+2+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+4*batch+2+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGBatchVacolorVafactor = 
"uniform vec4 vfuniforms[120];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, vfuniforms[int(0.1+4*batch+0+(0))]);\n"
"  tc0.y = dot(pos, vfuniforms[int(0.1+4*batch+0+(1))]);\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+4*batch+2+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+4*batch+2+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGPosition3d = 
"uniform mat4 mvp;\n"
"uniform vec4 texgen[2];\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"\n"
"  tc.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc.y = dot(pos, texgen[int(0.1+1)]);\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VTexTGPosition3dVacolorVafactor = 
"uniform mat4 mvp;\n"
"uniform vec4 texgen[2];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc0.y = dot(pos, texgen[int(0.1+1)]);\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VTexTGTexTGBatchPosition3dVafactor = 
"uniform mat4 vfmuniforms[30];\n"
"uniform vec4 vfuniforms[120];\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, vfuniforms[int(0.1+4*batch+0+(0))]);\n"
"  tc0.y = dot(pos, vfuniforms[int(0.1+4*batch+0+(1))]);\n"
"// S TexTG\n"
"\n"
"  tc1.x = dot(pos, vfuniforms[int(0.1+4*batch+0+(2+0))]);\n"
"  tc1.y = dot(pos, vfuniforms[int(0.1+4*batch+0+(2+1))]);\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGTexTGBatchVafactor = 
"uniform vec4 vfuniforms[180];\n"
"attribute vec4 afactor;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, vfuniforms[int(0.1+6*batch+0+(0))]);\n"
"  tc0.y = dot(pos, vfuniforms[int(0.1+6*batch+0+(1))]);\n"
"// S TexTG\n"
"\n"
"  tc1.x = dot(pos, vfuniforms[int(0.1+6*batch+0+(2+0))]);\n"
"  tc1.y = dot(pos, vfuniforms[int(0.1+6*batch+0+(2+1))]);\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+6*batch+4+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+6*batch+4+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VTexTGTexTGPosition3dVafactor = 
"uniform mat4 mvp;\n"
"uniform vec4 texgen[4];\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc0.y = dot(pos, texgen[int(0.1+1)]);\n"
"// S TexTG\n"
"\n"
"  tc1.x = dot(pos, texgen[int(0.1+2+0)]);\n"
"  tc1.y = dot(pos, texgen[int(0.1+2+1)]);\n"
"// S \n"
"  factor = afactor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VTexTGTexTGVafactor = 
"uniform vec4 mvp[2];\n"
"uniform vec4 texgen[4];\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"varying vec2 tc1;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc0.y = dot(pos, texgen[int(0.1+1)]);\n"
"// S TexTG\n"
"\n"
"  tc1.x = dot(pos, texgen[int(0.1+2+0)]);\n"
"  tc1.y = dot(pos, texgen[int(0.1+2+1)]);\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VTexTG = 
"uniform vec4 mvp[2];\n"
"uniform vec4 texgen[2];\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TexTG\n"
"\n"
"  tc.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc.y = dot(pos, texgen[int(0.1+1)]);\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VTexTGVacolorVafactor = 
"uniform vec4 mvp[2];\n"
"uniform vec4 texgen[2];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"varying vec2 tc0;\n"
"void main() {\n"
"// S TexTG\n"
"\n"
"  tc0.x = dot(pos, texgen[int(0.1+0)]);\n"
"  tc0.y = dot(pos, texgen[int(0.1+1)]);\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VVacolor = 
"uniform vec4 mvp[2];\n"
"attribute vec4 acolor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"void main() {\n"
"// E \n"
"  color = acolor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VVacolorVafactor = 
"uniform vec4 mvp[2];\n"
"attribute vec4 acolor;\n"
"attribute vec4 afactor;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec4 factor;\n"
"void main() {\n"
"// S \n"
"  color = acolor;\n"
"// S \n"
"  factor = afactor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_FSolidBatch = 
"uniform vec4 ffuniforms[30];\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Solid\n"
"  gl_FragColor = ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E none\n"
"  ;\n"
"\n"
"}";

const char* pSource_FSolid = 
"uniform vec4 cxmul;\n"
"void main() {\n"
"// E Solid\n"
"  gl_FragColor = cxmul;\n"
"// E none\n"
"  ;\n"
"\n"
"}";

const char* pSource_FTextBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Text\n"
"  vec4 c = color * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"  c.a = c.a * texture2D(tex, tc).a;\n"
"  gl_FragColor = c;\n"
"// E none\n"
"  ;\n"
"\n"
"}";

const char* pSource_FTextBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Text\n"
"  vec4 c = color * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"  c.a = c.a * texture2D(tex, tc).a;\n"
"  gl_FragColor = c;\n"
"// E none\n"
"  ;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FText = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E Text\n"
"  vec4 c = color * cxmul + cxadd;\n"
"  c.a = c.a * texture2D(tex, tc).a;\n"
"  gl_FragColor = c;\n"
"// E none\n"
"  ;\n"
"\n"
"}";

const char* pSource_FTextMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E Text\n"
"  vec4 c = color * cxmul + cxadd;\n"
"  c.a = c.a * texture2D(tex, tc).a;\n"
"  gl_FragColor = c;\n"
"// E none\n"
"  ;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextColorBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"\n"
"}";

const char* pSource_FTextColorBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextColorBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"\n"
"}";

const char* pSource_FTextColorBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextColorCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"\n"
"}";

const char* pSource_FTextColorCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextColor = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"\n"
"}";

const char* pSource_FTextColorMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextColor\n"
"  gl_FragColor = texture2D(tex,tc);\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVBatchCMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"\n"
"}";

const char* pSource_FTextYUVBatchCMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVBatchTextYUVACMatrix = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"\n"
"}";

const char* pSource_FTextYUVBatchTextYUVACMatrixMul = 
"uniform mat4 ffmuniforms[30];\n"
"uniform vec4 ffuniforms[30];\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,ffmuniforms[int(0.1+1*vbatch+0)]) + ffuniforms[int(0.1+1*vbatch+0)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVBatchTextYUVA = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"\n"
"}";

const char* pSource_FTextYUVBatchTextYUVAMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVBatch = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"\n"
"}";

const char* pSource_FTextYUVBatchMul = 
"uniform vec4 ffuniforms[60];\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * ffuniforms[int(0.1+2*vbatch+0)] + ffuniforms[int(0.1+2*vbatch+1)];\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVCMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"\n"
"}";

const char* pSource_FTextYUVCMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVTextYUVACMatrix = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"\n"
"}";

const char* pSource_FTextYUVTextYUVACMatrixMul = 
"uniform vec4 cxadd;\n"
"uniform mat4 cxmul;\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E CMatrix\n"
"  gl_FragColor = mul(gl_FragColor * gl_FragColor.a,cxmul) + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUVTextYUVA = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"\n"
"}";

const char* pSource_FTextYUVTextYUVAMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex_a;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E TextYUVA\n"
"  gl_FragColor.a = texture2D(tex_a, tc).r;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_FTextYUV = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"\n"
"}";

const char* pSource_FTextYUVMul = 
"uniform vec4 cxadd;\n"
"uniform vec4 cxmul;\n"
"uniform sampler2D tex_u;\n"
"uniform sampler2D tex_v;\n"
"uniform sampler2D tex_y;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E TextYUV\n"
"  vec4 c = vec4((texture2D(tex_y, tc).r - 16./255.) * 1.164);\n"
"  vec4 U = vec4(texture2D(tex_u, tc).r - 128./255.);\n"
"  vec4 V = vec4(texture2D(tex_v, tc).r - 128./255.);\n"
"  c += V * vec4(1.596, -0.813, 0, 0);\n"
"  c += U * vec4(0, -0.392, 2.017, 0);\n"
"  c.a = 1.0;\n"
"  gl_FragColor = c;\n"
"// E cxform\n"
"  gl_FragColor = gl_FragColor * cxmul + cxadd;\n"
"// E Mul\n"
"  gl_FragColor = mix(vec4(1), gl_FragColor, vec4(gl_FragColor.a));\n"
"\n"
"}";

const char* pSource_VBatchPosition3d = 
"uniform mat4 vfmuniforms[30];\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchPosition3dVatc = 
"uniform mat4 vfmuniforms[30];\n"
"attribute vec2 atc;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E \n"
"  tc = atc;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchPosition3dVatcVacolor = 
"uniform mat4 vfmuniforms[30];\n"
"attribute vec4 acolor;\n"
"attribute vec2 atc;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S \n"
"  tc = atc;\n"
"// S \n"
"  color = acolor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, vfmuniforms[int(0.1+1*batch+0)]);\n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatch = 
"uniform vec4 vfuniforms[60];\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchVatc = 
"uniform vec4 vfuniforms[60];\n"
"attribute vec2 atc;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// E \n"
"  tc = atc;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VBatchVatcVacolor = 
"uniform vec4 vfuniforms[60];\n"
"attribute vec4 acolor;\n"
"attribute vec2 atc;\n"
"attribute float batch;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"varying float vbatch;\n"
"void main() {\n"
"// S \n"
"  tc = atc;\n"
"// S \n"
"  color = acolor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, vfuniforms[int(0.1+2*batch+0+(0))]);\n"
"  gl_Position.y = dot(pos, vfuniforms[int(0.1+2*batch+0+(1))]);  \n"
"  vbatch = batch;\n"
"\n"
"}";

const char* pSource_VPosition3d = 
"uniform mat4 mvp;\n"
"attribute vec4 pos;\n"
"void main() {\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VPosition3dVatc = 
"uniform mat4 mvp;\n"
"attribute vec2 atc;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E \n"
"  tc = atc;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_VPosition3dVatcVacolor = 
"uniform mat4 mvp;\n"
"attribute vec4 acolor;\n"
"attribute vec2 atc;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// S \n"
"  tc = atc;\n"
"// S \n"
"  color = acolor;\n"
"// E Position3d\n"
"\n"
"  gl_Position = mul(pos, mvp);\n"
"\n"
"}";

const char* pSource_V = 
"uniform vec4 mvp[2];\n"
"attribute vec4 pos;\n"
"void main() {\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VVatc = 
"uniform vec4 mvp[2];\n"
"attribute vec2 atc;\n"
"attribute vec4 pos;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// E \n"
"  tc = atc;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";

const char* pSource_VVatcVacolor = 
"uniform vec4 mvp[2];\n"
"attribute vec4 acolor;\n"
"attribute vec2 atc;\n"
"attribute vec4 pos;\n"
"varying vec4 color;\n"
"varying vec2 tc;\n"
"void main() {\n"
"// S \n"
"  tc = atc;\n"
"// S \n"
"  color = acolor;\n"
"// E position\n"
"\n"
"\n"
"  gl_Position = pos;\n"
"  gl_Position.x = dot(pos, mvp[int(0.1+0)]);\n"
"  gl_Position.y = dot(pos, mvp[int(0.1+1)]);  \n"
"\n"
"}";


const char* ShaderUniformNames[Uniform::SU_Count] = {
    "cxadd",
    "cxmul",
    "ffmuniforms",
    "ffuniforms",
    "mvp",
    "tex",
    "tex0",
    "tex1",
    "tex_a",
    "tex_u",
    "tex_v",
    "tex_y",
    "texgen",
    "vfmuniforms",
    "vfuniforms"
};

static VertexShaderDesc ShaderDesc_VS_VTexTGVacolorVafactor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTGVacolorVafactor,
  /* Uniforms */      { /* cxadd  */ {-1, 0, 0, 0, 0},
                        /* cxmul  */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {-1, 0, 0, 0, 0},
                        /* mvp    */ {0, 0, 4, 8, 0},
                        /* tex    */ {-1, 0, 0, 0, 0},
                        /* tex0   */ {-1, 0, 0, 0, 0},
                        /* tex1   */ {-1, 0, 0, 0, 0},
                        /* tex_a  */ {-1, 0, 0, 0, 0},
                        /* tex_u  */ {-1, 0, 0, 0, 0},
                        /* tex_v  */ {-1, 0, 0, 0, 0},
                        /* tex_y  */ {-1, 0, 0, 0, 0},
                        /* texgen */ {2, 8, 4, 8, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGPosition3dVacolorVafactor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTGPosition3dVacolorVafactor,
  /* Uniforms */      { /* cxadd  */ {-1, 0, 0, 0, 0},
                        /* cxmul  */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {-1, 0, 0, 0, 0},
                        /* mvp    */ {0, 0, 16, 16, 0},
                        /* tex    */ {-1, 0, 0, 0, 0},
                        /* tex0   */ {-1, 0, 0, 0, 0},
                        /* tex1   */ {-1, 0, 0, 0, 0},
                        /* tex_a  */ {-1, 0, 0, 0, 0},
                        /* tex_u  */ {-1, 0, 0, 0, 0},
                        /* tex_v  */ {-1, 0, 0, 0, 0},
                        /* tex_y  */ {-1, 0, 0, 0, 0},
                        /* texgen */ {4, 16, 4, 8, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGBatchVacolorVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGBatchVacolorVafactor,
  /* Uniforms */      { /* cxadd      */ {-1, 0, 0, 0, 0},
                        /* cxmul      */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {-1, 0, 0, 0, 0},
                        /* mvp        */ {-1, 0, 0, 0, 0},
                        /* tex        */ {-1, 0, 0, 0, 0},
                        /* tex0       */ {-1, 0, 0, 0, 0},
                        /* tex1       */ {-1, 0, 0, 0, 0},
                        /* tex_a      */ {-1, 0, 0, 0, 0},
                        /* tex_u      */ {-1, 0, 0, 0, 0},
                        /* tex_v      */ {-1, 0, 0, 0, 0},
                        /* tex_y      */ {-1, 0, 0, 0, 0},
                        /* texgen     */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {0, 0, 4, 480, 4}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 2, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 2},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGBatchPosition3dVacolorVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGBatchPosition3dVacolorVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {120, 480, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 2},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VVacolor = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VPosition3dVacolor = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VPosition3dVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchVacolor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 0, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchPosition3dVacolor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchPosition3dVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGTexTGVafactor = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTGTexTGVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {2, 8, 4, 16, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGTexTGPosition3dVafactor = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTGTexTGPosition3dVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {4, 16, 4, 16, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGTexTGBatchVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGTexTGBatchVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 720, 6}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 4, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 4},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGTexTGBatchPosition3dVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGTexTGBatchPosition3dVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {120, 480, 4, 480, 4}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 4},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VVacolorVafactor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VVacolorVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VPosition3dVacolorVafactor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VPosition3dVacolorVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchVacolorVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchVacolorVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 0, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchPosition3dVacolorVafactor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"afactor", 4 | VET_Color | (1 << VET_Index_Shift)}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchPosition3dVacolorVafactor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTG = {
  /* Flags */         0,
  /* NumAttribs */    1,
  /* Attributes */    {{"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTG,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {2, 8, 4, 8, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGPosition3d = {
  /* Flags */         0,
  /* NumAttribs */    1,
  /* Attributes */    {{"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VTexTGPosition3d,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {4, 16, 4, 8, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGBatch = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    2,
  /* Attributes */    {{"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 480, 4}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 2, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 2},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VTexTGBatchPosition3d = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    2,
  /* Attributes */    {{"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VTexTGBatchPosition3d,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {120, 480, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_vfuniforms, 0, 2},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VVatcVacolor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VVatcVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VPosition3dVatcVacolor = {
  /* Flags */         0,
  /* NumAttribs */    3,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VPosition3dVatcVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchVatcVacolor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchVatcVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 0, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchPosition3dVatcVacolor = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    4,
  /* Attributes */    {{"acolor", 4 | VET_Color}, {"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchPosition3dVatcVacolor,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_V = {
  /* Flags */         0,
  /* NumAttribs */    1,
  /* Attributes */    {{"pos", 4 | VET_Pos}},
  /* pSource */       pSource_V,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VPosition3d = {
  /* Flags */         0,
  /* NumAttribs */    1,
  /* Attributes */    {{"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VPosition3d,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatch = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    2,
  /* Attributes */    {{"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 0, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchPosition3d = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    2,
  /* Attributes */    {{"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchPosition3d,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VVatc = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VVatc,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 4, 8, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VPosition3dVatc = {
  /* Flags */         0,
  /* NumAttribs */    2,
  /* Attributes */    {{"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}},
  /* pSource */       pSource_VPosition3dVatc,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {0, 0, 16, 16, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchVatc = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchVatc,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {0, 0, 4, 240, 2}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfuniforms, 0, 2},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static VertexShaderDesc ShaderDesc_VS_VBatchPosition3dVatc = {
  /* Flags */         Shader_Batched,
  /* NumAttribs */    3,
  /* Attributes */    {{"atc", 2 | VET_TexCoord}, {"pos", 4 | VET_Pos}, {"batch", VET_Instance8}},
  /* pSource */       pSource_VBatchPosition3dVatc,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {0, 0, 16, 480, 1},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_vfmuniforms, 0, 1},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

const VertexShaderDesc* VertexShaderDesc::Descs[VS_Count] = {
    NULL,
    &ShaderDesc_VS_VTexTGVacolorVafactor,
    &ShaderDesc_VS_VTexTGPosition3dVacolorVafactor,
    &ShaderDesc_VS_VTexTGBatchVacolorVafactor,
    &ShaderDesc_VS_VTexTGBatchPosition3dVacolorVafactor,
    &ShaderDesc_VS_VVacolor,
    &ShaderDesc_VS_VPosition3dVacolor,
    &ShaderDesc_VS_VBatchVacolor,
    &ShaderDesc_VS_VBatchPosition3dVacolor,
    &ShaderDesc_VS_VTexTGTexTGVafactor,
    &ShaderDesc_VS_VTexTGTexTGPosition3dVafactor,
    &ShaderDesc_VS_VTexTGTexTGBatchVafactor,
    &ShaderDesc_VS_VTexTGTexTGBatchPosition3dVafactor,
    &ShaderDesc_VS_VVacolorVafactor,
    &ShaderDesc_VS_VPosition3dVacolorVafactor,
    &ShaderDesc_VS_VBatchVacolorVafactor,
    &ShaderDesc_VS_VBatchPosition3dVacolorVafactor,
    &ShaderDesc_VS_VTexTG,
    &ShaderDesc_VS_VTexTGPosition3d,
    &ShaderDesc_VS_VTexTGBatch,
    &ShaderDesc_VS_VTexTGBatchPosition3d,
    &ShaderDesc_VS_VVatcVacolor,
    &ShaderDesc_VS_VPosition3dVatcVacolor,
    &ShaderDesc_VS_VBatchVatcVacolor,
    &ShaderDesc_VS_VBatchPosition3dVatcVacolor,
    &ShaderDesc_VS_V,
    &ShaderDesc_VS_VPosition3d,
    &ShaderDesc_VS_VBatch,
    &ShaderDesc_VS_VBatchPosition3d,
    &ShaderDesc_VS_VVatc,
    &ShaderDesc_VS_VPosition3dVatc,
    &ShaderDesc_VS_VBatchVatc,
    &ShaderDesc_VS_VBatchPosition3dVatc,
};

VertexShaderType FragShaderDesc::VShaderForFShader[FragShaderDesc::FS_Count] = {
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VTexTGVacolorVafactor,
    VertexShaderDesc::VS_VTexTGVacolorVafactor,
    VertexShaderDesc::VS_VTexTGBatchVacolorVafactor,
    VertexShaderDesc::VS_VTexTGBatchVacolorVafactor,
    VertexShaderDesc::VS_VTexTGVacolorVafactor,
    VertexShaderDesc::VS_VTexTGVacolorVafactor,
    VertexShaderDesc::VS_VTexTGBatchVacolorVafactor,
    VertexShaderDesc::VS_VTexTGBatchVacolorVafactor,
    VertexShaderDesc::VS_VTexTGTexTGVafactor,
    VertexShaderDesc::VS_VTexTGTexTGVafactor,
    VertexShaderDesc::VS_VTexTGTexTGBatchVafactor,
    VertexShaderDesc::VS_VTexTGTexTGBatchVafactor,
    VertexShaderDesc::VS_VTexTGTexTGVafactor,
    VertexShaderDesc::VS_VTexTGTexTGVafactor,
    VertexShaderDesc::VS_VTexTGTexTGBatchVafactor,
    VertexShaderDesc::VS_VTexTGTexTGBatchVafactor,
    VertexShaderDesc::VS_VVacolorVafactor,
    VertexShaderDesc::VS_VVacolorVafactor,
    VertexShaderDesc::VS_VBatchVacolorVafactor,
    VertexShaderDesc::VS_VBatchVacolorVafactor,
    VertexShaderDesc::VS_VVacolorVafactor,
    VertexShaderDesc::VS_VVacolorVafactor,
    VertexShaderDesc::VS_VBatchVacolorVafactor,
    VertexShaderDesc::VS_VBatchVacolorVafactor,
    VertexShaderDesc::VS_VTexTG,
    VertexShaderDesc::VS_VTexTG,
    VertexShaderDesc::VS_VTexTGBatch,
    VertexShaderDesc::VS_VTexTGBatch,
    VertexShaderDesc::VS_VTexTG,
    VertexShaderDesc::VS_VTexTG,
    VertexShaderDesc::VS_VTexTGBatch,
    VertexShaderDesc::VS_VTexTGBatch,
    VertexShaderDesc::VS_VVacolor,
    VertexShaderDesc::VS_VVacolor,
    VertexShaderDesc::VS_VBatchVacolor,
    VertexShaderDesc::VS_VBatchVacolor,
    VertexShaderDesc::VS_VVacolor,
    VertexShaderDesc::VS_VVacolor,
    VertexShaderDesc::VS_VBatchVacolor,
    VertexShaderDesc::VS_VBatchVacolor,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatc,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VVatcVacolor,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatcVacolor,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VVatcVacolor,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatchVatcVacolor,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_V,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_None,
    VertexShaderDesc::VS_VBatch,
};

static FragShaderDesc ShaderDesc_FS_FTexTGVertex = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertex,
  /* Uniforms */      { /* cxadd */ {0, 960, 4, 4, 0},
                        /* cxmul */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {-1, 0, 0, 0, 0},
                        /* mvp   */ {-1, 0, 0, 0, 0},
                        /* tex   */ {-1, 0, 0, 0, 0},
                        /* tex0  */ {0, 0, 0, 0, 0},
                        /* tex1  */ {-1, 0, 0, 0, 0},
                        /* tex_a */ {-1, 0, 0, 0, 0},
                        /* tex_u */ {-1, 0, 0, 0, 0},
                        /* tex_v */ {-1, 0, 0, 0, 0},
                        /* tex_y */ {-1, 0, 0, 0, 0},
                        /* texgen */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexCMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexCMatrix,
  /* Uniforms */      { /* cxadd */ {0, 960, 4, 4, 0},
                        /* cxmul */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {-1, 0, 0, 0, 0},
                        /* mvp   */ {-1, 0, 0, 0, 0},
                        /* tex   */ {-1, 0, 0, 0, 0},
                        /* tex0  */ {0, 0, 0, 0, 0},
                        /* tex1  */ {-1, 0, 0, 0, 0},
                        /* tex_a */ {-1, 0, 0, 0, 0},
                        /* tex_u */ {-1, 0, 0, 0, 0},
                        /* tex_v */ {-1, 0, 0, 0, 0},
                        /* tex_y */ {-1, 0, 0, 0, 0},
                        /* texgen */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexBatch,
  /* Uniforms */      { /* cxadd      */ {-1, 0, 0, 0, 0},
                        /* cxmul      */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms */ {0, 960, 4, 240, 2},
                        /* mvp        */ {-1, 0, 0, 0, 0},
                        /* tex        */ {-1, 0, 0, 0, 0},
                        /* tex0       */ {0, 0, 0, 0, 0},
                        /* tex1       */ {-1, 0, 0, 0, 0},
                        /* tex_a      */ {-1, 0, 0, 0, 0},
                        /* tex_u      */ {-1, 0, 0, 0, 0},
                        /* tex_v      */ {-1, 0, 0, 0, 0},
                        /* tex_y      */ {-1, 0, 0, 0, 0},
                        /* texgen     */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGVertexBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGVertexBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTG = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTG,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGCMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGTexTGBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex0,
                        Uniform::SU_tex1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGTexTGBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {0, 0, 0, 0, 0},
                        /* tex1        */ {1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertex = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertex,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexCMatrix = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexMul = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexVertexBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexVertexBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTG = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTG,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGCMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTexTGBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTexTGBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertex = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertex,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexCMatrix = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexMul = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FVertexBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FVertexBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUV = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUV,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVTextYUVA = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVTextYUVA,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVCMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVTextYUVACMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVTextYUVACMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchTextYUVA = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchTextYUVA,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchTextYUVACMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchTextYUVACMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVTextYUVAMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVTextYUVAMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVTextYUVACMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVTextYUVACMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchTextYUVAMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchTextYUVAMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {0, 0, 0, 0, 0},
                        /* tex_v       */ {1, 0, 0, 0, 0},
                        /* tex_y       */ {2, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextYUVBatchTextYUVACMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex_y,
                        Uniform::SU_tex_u,
                        Uniform::SU_tex_v,
                        Uniform::SU_tex_a,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextYUVBatchTextYUVACMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {0, 0, 0, 0, 0},
                        /* tex_u       */ {1, 0, 0, 0, 0},
                        /* tex_v       */ {2, 0, 0, 0, 0},
                        /* tex_y       */ {3, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColor = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColor,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorCMatrix = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorCMatrix,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorBatchCMatrix = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorBatchCMatrix,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorCMatrixMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 16, 16, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextColorBatchCMatrixMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextColorBatchCMatrixMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {0, 960, 16, 480, 1},
                        /* ffuniforms  */ {120, 1440, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* cxmul       */ {Uniform::SU_ffmuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FText = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FText,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextMul = {
  /* Flags */         0,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextMul,
  /* Uniforms */      { /* cxadd       */ {0, 960, 4, 4, 0},
                        /* cxmul       */ {1, 964, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FTextBatchMul = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { Uniform::SU_tex,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FTextBatchMul,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 240, 2},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {0, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_ffuniforms, 1, 1},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FSolid = {
  /* Flags */         0,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FSolid,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {0, 960, 4, 4, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {-1, 0, 0, 0, 0},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_Count, -1, 0},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

static FragShaderDesc ShaderDesc_FS_FSolidBatch = {
  /* Flags */         Shader_Batched,
  /* TexParams */     { -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1,
                        -1},
  /* pSource */       pSource_FSolidBatch,
  /* Uniforms */      { /* cxadd       */ {-1, 0, 0, 0, 0},
                        /* cxmul       */ {-1, 0, 0, 0, 0},
                        /* ffmuniforms */ {-1, 0, 0, 0, 0},
                        /* ffuniforms  */ {0, 960, 4, 120, 1},
                        /* mvp         */ {-1, 0, 0, 0, 0},
                        /* tex         */ {-1, 0, 0, 0, 0},
                        /* tex0        */ {-1, 0, 0, 0, 0},
                        /* tex1        */ {-1, 0, 0, 0, 0},
                        /* tex_a       */ {-1, 0, 0, 0, 0},
                        /* tex_u       */ {-1, 0, 0, 0, 0},
                        /* tex_v       */ {-1, 0, 0, 0, 0},
                        /* tex_y       */ {-1, 0, 0, 0, 0},
                        /* texgen      */ {-1, 0, 0, 0, 0},
                        /* vfmuniforms */ {-1, 0, 0, 0, 0},
                        /* vfuniforms  */ {-1, 0, 0, 0, 0}},
  /* BatchUniforms */ { /* cxadd       */ {Uniform::SU_Count, -1, 0},
                        /* cxmul       */ {Uniform::SU_ffuniforms, 0, 1},
                        /* ffmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* ffuniforms  */ {Uniform::SU_Count, -1, 0},
                        /* mvp         */ {Uniform::SU_Count, -1, 0},
                        /* tex         */ {Uniform::SU_Count, -1, 0},
                        /* tex0        */ {Uniform::SU_Count, -1, 0},
                        /* tex1        */ {Uniform::SU_Count, -1, 0},
                        /* tex_a       */ {Uniform::SU_Count, -1, 0},
                        /* tex_u       */ {Uniform::SU_Count, -1, 0},
                        /* tex_v       */ {Uniform::SU_Count, -1, 0},
                        /* tex_y       */ {Uniform::SU_Count, -1, 0},
                        /* texgen      */ {Uniform::SU_Count, -1, 0},
                        /* vfmuniforms */ {Uniform::SU_Count, -1, 0},
                        /* vfuniforms  */ {Uniform::SU_Count, -1, 0}}};

const FragShaderDesc* FragShaderDesc::Descs[FS_Count] = {
    NULL,
    &ShaderDesc_FS_FTexTGVertex,
    &ShaderDesc_FS_FTexTGVertexCMatrix,
    &ShaderDesc_FS_FTexTGVertexBatch,
    &ShaderDesc_FS_FTexTGVertexBatchCMatrix,
    &ShaderDesc_FS_FTexTGVertexMul,
    &ShaderDesc_FS_FTexTGVertexCMatrixMul,
    &ShaderDesc_FS_FTexTGVertexBatchMul,
    &ShaderDesc_FS_FTexTGVertexBatchCMatrixMul,
    &ShaderDesc_FS_FTexTGTexTG,
    &ShaderDesc_FS_FTexTGTexTGCMatrix,
    &ShaderDesc_FS_FTexTGTexTGBatch,
    &ShaderDesc_FS_FTexTGTexTGBatchCMatrix,
    &ShaderDesc_FS_FTexTGTexTGMul,
    &ShaderDesc_FS_FTexTGTexTGCMatrixMul,
    &ShaderDesc_FS_FTexTGTexTGBatchMul,
    &ShaderDesc_FS_FTexTGTexTGBatchCMatrixMul,
    &ShaderDesc_FS_FVertexVertex,
    &ShaderDesc_FS_FVertexVertexCMatrix,
    &ShaderDesc_FS_FVertexVertexBatch,
    &ShaderDesc_FS_FVertexVertexBatchCMatrix,
    &ShaderDesc_FS_FVertexVertexMul,
    &ShaderDesc_FS_FVertexVertexCMatrixMul,
    &ShaderDesc_FS_FVertexVertexBatchMul,
    &ShaderDesc_FS_FVertexVertexBatchCMatrixMul,
    &ShaderDesc_FS_FTexTG,
    &ShaderDesc_FS_FTexTGCMatrix,
    &ShaderDesc_FS_FTexTGBatch,
    &ShaderDesc_FS_FTexTGBatchCMatrix,
    &ShaderDesc_FS_FTexTGMul,
    &ShaderDesc_FS_FTexTGCMatrixMul,
    &ShaderDesc_FS_FTexTGBatchMul,
    &ShaderDesc_FS_FTexTGBatchCMatrixMul,
    &ShaderDesc_FS_FVertex,
    &ShaderDesc_FS_FVertexCMatrix,
    &ShaderDesc_FS_FVertexBatch,
    &ShaderDesc_FS_FVertexBatchCMatrix,
    &ShaderDesc_FS_FVertexMul,
    &ShaderDesc_FS_FVertexCMatrixMul,
    &ShaderDesc_FS_FVertexBatchMul,
    &ShaderDesc_FS_FVertexBatchCMatrixMul,
    &ShaderDesc_FS_FTextYUV,
    &ShaderDesc_FS_FTextYUVTextYUVA,
    &ShaderDesc_FS_FTextYUVCMatrix,
    &ShaderDesc_FS_FTextYUVTextYUVACMatrix,
    &ShaderDesc_FS_FTextYUVBatch,
    &ShaderDesc_FS_FTextYUVBatchTextYUVA,
    &ShaderDesc_FS_FTextYUVBatchCMatrix,
    &ShaderDesc_FS_FTextYUVBatchTextYUVACMatrix,
    &ShaderDesc_FS_FTextYUVMul,
    &ShaderDesc_FS_FTextYUVTextYUVAMul,
    &ShaderDesc_FS_FTextYUVCMatrixMul,
    &ShaderDesc_FS_FTextYUVTextYUVACMatrixMul,
    &ShaderDesc_FS_FTextYUVBatchMul,
    &ShaderDesc_FS_FTextYUVBatchTextYUVAMul,
    &ShaderDesc_FS_FTextYUVBatchCMatrixMul,
    &ShaderDesc_FS_FTextYUVBatchTextYUVACMatrixMul,
    &ShaderDesc_FS_FTextColor,
    NULL,
    &ShaderDesc_FS_FTextColorCMatrix,
    NULL,
    &ShaderDesc_FS_FTextColorBatch,
    NULL,
    &ShaderDesc_FS_FTextColorBatchCMatrix,
    NULL,
    &ShaderDesc_FS_FTextColorMul,
    NULL,
    &ShaderDesc_FS_FTextColorCMatrixMul,
    NULL,
    &ShaderDesc_FS_FTextColorBatchMul,
    NULL,
    &ShaderDesc_FS_FTextColorBatchCMatrixMul,
    NULL,
    &ShaderDesc_FS_FText,
    NULL,
    NULL,
    NULL,
    &ShaderDesc_FS_FTextBatch,
    NULL,
    NULL,
    NULL,
    &ShaderDesc_FS_FTextMul,
    NULL,
    NULL,
    NULL,
    &ShaderDesc_FS_FTextBatchMul,
    NULL,
    NULL,
    NULL,
    &ShaderDesc_FS_FSolid,
    NULL,
    NULL,
    NULL,
    &ShaderDesc_FS_FSolidBatch,
};


}}}
