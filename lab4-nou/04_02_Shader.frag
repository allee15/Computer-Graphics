//
// ================================================
// | Grafica pe calculator                        |
// ================================================
// | Laboratorul IV - 04_02_Shader.frag |
// ======================================
// 
//  Shaderul de fragment / Fragment shader - afecteaza culoarea pixelilor;
//

#version 330 core

//	Variabile de intrare (dinspre Shader.vert);
in vec4 ex_Color;
in vec2 tex_Coord;		//	Coordonata de texturare;

//	Variabile de iesire	(spre programul principal);
out vec4 out_Color;		//	Culoarea actualizata;

uniform sampler2D myTexture;
uniform int isTexture;

void main(void)
  {
    switch (isTexture) {
        case 0:
            out_Color = ex_Color;
            break;
        case 1:
            out_Color = texture(myTexture, tex_Coord);
            break;
    }
  }