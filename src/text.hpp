//
//  text.hpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#ifndef _Text_hpp
#define _Text_hpp

enum ALIGNMENT {
	TOP_LEFT 		= 1 << 0,
	TOP_RIGHT		= 1 << 1,
	BOTTOM_LEFT		= 1 << 2, 
	BOTTOM_RIGHT	= 1 << 3,
	NOT_MONOSPACED	= 1 << 4,
	MONOSPACED		= 1 << 5,
};

struct PackedSpriteMap {
	struct Character {
        glm::vec2 position;
        glm::ivec2 size; 
        glm::ivec2 bearing;	
        unsigned int advance;	
    };

    std::map< unsigned char, Character > characters;

	glm::vec2 texture_dimensions;
	unsigned int text_fontsize;
};

struct Text {

	glm::vec3 position;

	unsigned int vao = 0;
	unsigned int vbo_posuv = 0;
	unsigned int vbo_color = 0;
	unsigned int ebo = 0;
	unsigned int num_indices = 0;

	glm::mat4 model_mtx;

    ~Text() {
    	if ( vao != 0 ) glDeleteBuffers( 1, &vao );
    	if ( vbo_posuv != 0 ) glDeleteVertexArrays( 1, &vbo_posuv );
    	if ( vbo_color != 0 ) glDeleteVertexArrays( 1, &vbo_color );
    	if ( ebo != 0 ) glDeleteBuffers( 1, &ebo );
    }

	void setPosition( glm::vec3 pos );
	void render( unsigned int shader, unsigned int texID );
	void create_text_mesh( PackedSpriteMap *psm, unsigned int shader, const char* txt, int lineSpacing, unsigned int alignment = ALIGNMENT::TOP_LEFT | ALIGNMENT::NOT_MONOSPACED );
};

unsigned int createPackedGlyphTexture( PackedSpriteMap *psm, FT_Library ft, unsigned int fontsize, const char* font );

#endif