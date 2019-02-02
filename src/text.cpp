//
//  text.cpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#include <OpenGL/gl3.h>
#include <ft2build.h>
#include FT_FREETYPE_H
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <map>
#include <vector>

#include "typedefs.hpp"
#include "shader.hpp"
#include "text.hpp"

unsigned int createPackedGlyphTexture( PackedSpriteMap *psm, FT_Library ft, unsigned int fontsize, const char* font ) {

	if ( fontsize > 200 ) fontsize = 200; // NOTE: The max size will be 200pixels aka 100pt

	psm->text_fontsize = fontsize;
	psm->characters.clear();
	
	FT_Face face;
	if ( FT_New_Face( ft, font, 0, &face ) ) { ERROR( "FREETYPE: Failed to load font.\n" ); return 0; }
	FT_Set_Pixel_Sizes( face, 0, fontsize );
	if ( FT_Load_Char( face, 'X', FT_LOAD_RENDER ) ) { ERROR( "FREETYTPE: Failed to load Glyph\n" ); return 0; }
	
	struct Temp_Character {
		glm::ivec2 size; 
		glm::ivec2 bearing;	
		unsigned int advance;	
		unsigned char* bitmap;
		~Temp_Character() { delete [] bitmap; }
	};
	std::map< unsigned char, Temp_Character > temp_characters;

	unsigned char startCharacter = 0;
	unsigned char endCharacter = 128;

	for ( unsigned char c = startCharacter; c <= endCharacter; c++ ) {
		if ( FT_Load_Char( face, c, FT_LOAD_RENDER ) ) {
			ERROR( "ERROR::FREETYTPE: Failed to load Glyph" );
			continue;
		}

		temp_characters.insert( std::pair<unsigned char, Temp_Character>( c, Temp_Character() ) );
		temp_characters[c].size = glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows );
		temp_characters[c].bearing = glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top );
		temp_characters[c].advance = (unsigned int)face->glyph->advance.x;
		unsigned int dimensions = (unsigned int)face->glyph->bitmap.width * (unsigned int)face->glyph->bitmap.rows;
		temp_characters[c].bitmap = new unsigned char [ dimensions ];
		
		memcpy( temp_characters[c].bitmap, face->glyph->bitmap.buffer, dimensions );

		psm->characters.insert( std::pair<unsigned char, PackedSpriteMap::Character>( c, PackedSpriteMap::Character() ) );
		psm->characters[c].size = glm::ivec2( face->glyph->bitmap.width, face->glyph->bitmap.rows );
		psm->characters[c].bearing = glm::ivec2( face->glyph->bitmap_left, face->glyph->bitmap_top );
		psm->characters[c].advance = (unsigned int)face->glyph->advance.x;
	}

	FT_Done_Face( face );

	unsigned int combined_character_area = 0;
	for ( size_t i = startCharacter; i <= endCharacter; i++ ) { 
		combined_character_area += temp_characters[i].size.x * temp_characters[i].size.y; 
	}

	unsigned int recm_dim = sqrt( combined_character_area ) * 1.5;
	recm_dim = recm_dim + (4 - (recm_dim%4));
	psm->texture_dimensions = glm::vec2( recm_dim, recm_dim );
	unsigned char* combinedBitmap = new unsigned char [ recm_dim * recm_dim ]();
	
	unsigned int xx = 0; unsigned int yy = 0;
	for ( size_t ch = startCharacter; ch <= endCharacter; ch++ ) {
		if ( temp_characters[ch].size.x > 0 ) { 
			if ( xx + temp_characters[ch].size.x + 1 > recm_dim ) { yy += fontsize; xx = 0; }
			if ( yy + temp_characters[ch].size.y + 1 > recm_dim ) { break; }
			psm->characters[ch].position = glm::vec2( xx, yy );
			for ( size_t y = yy; y < yy+temp_characters[ch].size.y; y++ ) {
				for ( size_t x = xx; x < xx+temp_characters[ch].size.x; x++ ) {
					combinedBitmap[ recm_dim*y + x ] = temp_characters[ch].bitmap[ temp_characters[ch].size.x*(y-yy) + x-xx ];
				}
			}
			xx += temp_characters[ch].size.x + 1;
		}
	}	    

	unsigned int tex_id;
	glGenTextures(1, &tex_id);

	glBindTexture(GL_TEXTURE_2D, tex_id);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	glTexImage2D( GL_TEXTURE_2D, 0, GL_RED, recm_dim, recm_dim, 0, GL_RED, GL_UNSIGNED_BYTE, combinedBitmap );
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	delete [] combinedBitmap;

	return tex_id;
}

void Text::create_text_mesh( PackedSpriteMap *psm, unsigned int shader, const char* txt, int lineSpacing,  unsigned int alignment ) {
	
	std::vector<float> verts;
	std::vector<unsigned char> colors;
	std::vector<unsigned int> indis;

	unsigned int vert_ofst = 0;
	float xx = 0;
	float yy = psm->characters[ '`' ].bearing.y; // TODO(Xavier): make it so it chooses the max bearing of all psm->characters.
	for ( size_t i = 0; i < psm->characters.size(); ++i ) {
		if ( yy < psm->characters[i].bearing.y ) {
			yy = psm->characters[i].bearing.y;
		}
	}

	if ( alignment & ALIGNMENT::TOP_LEFT ) {
		for ( size_t i = 0; i < strlen( txt ); ++i ) {
			
			unsigned char ch = txt[i];
			if ( ch == '\n') { xx = 0; yy += psm->text_fontsize + lineSpacing; continue;}
			if ( ch == '\t') { xx += 4*(psm->characters[' '].advance >> 6) - psm->characters[' '].bearing.x; continue; }
			if ( ch == ' ') { xx += (psm->characters[' '].advance >> 6) - psm->characters[' '].bearing.x; continue; }

			float xd = psm->characters[ch].size.x;
			float yd = psm->characters[ch].size.y;
			float yo = psm->characters[ch].bearing.y;
			float uv_x = ( 1.0f/psm->texture_dimensions.x )*psm->characters[ ch ].position.x;
			float uv_y = ( 1.0f/psm->texture_dimensions.y )*psm->characters[ ch ].position.y;
			float uv_xd = ( 1.0f/psm->texture_dimensions.x )*( psm->characters[ ch ].position.x + psm->characters[ ch ].size.x );
			float uv_yd = ( 1.0f/psm->texture_dimensions.y )*( psm->characters[ ch ].position.y + psm->characters[ ch ].size.y );

			float tmp_vert_array[ 20 ] = { 
				xx, 	yy-yo, 		0,		uv_x, 		uv_y,
				xx, 	yy+yd-yo, 	0,		uv_x, 		uv_yd,
				xx+xd, 	yy+yd-yo, 	0,		uv_xd, 		uv_yd,
				xx+xd, 	yy-yo, 		0,		uv_xd, 		uv_y
			};
			verts.insert( verts.end(), tmp_vert_array, tmp_vert_array + 20 );

			// NOTE(Xavier): This is for in the future when
			// individual psm->characters can have their own colors.

			unsigned char tmp_color_array[ 16 ] = { 
				255, 255, 255, 255,
				255, 255, 255, 255,
				255, 255, 255, 255,
				255, 255, 255, 255,
			};
			colors.insert( colors.end(), tmp_color_array, tmp_color_array + 16 );

			unsigned int tmp_indx_array[ 6 ] = {
				vert_ofst, vert_ofst+1, vert_ofst+2, vert_ofst, vert_ofst+2, vert_ofst+3
			};
			indis.insert( indis.end(),  tmp_indx_array, tmp_indx_array+6 );
			vert_ofst += 4;
			
			if ( alignment &&ALIGNMENT::MONOSPACED ) xx += psm->characters[ ' ' ].advance >> 6;
			else xx += ( psm->characters[ ch ].advance >> 6 ) - psm->characters[ ch ].bearing.x;
		}
	} 
	else if ( alignment & ALIGNMENT::TOP_RIGHT ) {
		
		float vo = 0;
		for ( size_t i = 0; i < strlen( txt ); ++i ) {
			if ( txt[i] == '\n') vo += psm->text_fontsize + lineSpacing;
		}

		for ( long long i = strlen(txt)-1; i >= 0; i-- ) {
			
			unsigned char ch = txt[i];
			if ( ch == '\n') { xx = 0; yy -= psm->text_fontsize + lineSpacing; continue;}
			if ( ch == '\t') { xx -= 4*(psm->characters[' '].advance >> 6) - psm->characters[' '].bearing.x; continue; }
			if ( ch == ' ') { xx -= (psm->characters[' '].advance >> 6) - psm->characters[' '].bearing.x; continue; }

			float xb = (psm->characters[ch].advance >> 6) - psm->characters[ch].bearing.x;

			float xd = psm->characters[ch].size.x;
			float yd = psm->characters[ch].size.y;
			float yo = psm->characters[ch].bearing.y;
			float uv_x = ( 1.0f/psm->texture_dimensions.x )*psm->characters[ ch ].position.x;
			float uv_y = ( 1.0f/psm->texture_dimensions.y )*psm->characters[ ch ].position.y;
			float uv_xd = ( 1.0f/psm->texture_dimensions.x )*( psm->characters[ ch ].position.x + psm->characters[ ch ].size.x );
			float uv_yd = ( 1.0f/psm->texture_dimensions.y )*( psm->characters[ ch ].position.y + psm->characters[ ch ].size.y );

			float tmp_vert_array[ 20 ] = { 
				xx-xb, 		yy-yo+vo, 		0,		uv_x, 		uv_y,
				xx-xb, 		yy+yd-yo+vo, 	0,		uv_x, 		uv_yd,
				xx+xd-xb, 	yy+yd-yo+vo, 	0,		uv_xd, 		uv_yd,
				xx+xd-xb, 	yy-yo+vo, 		0,		uv_xd, 		uv_y
			};
			verts.insert( verts.end(), tmp_vert_array, tmp_vert_array + 20 );

			// NOTE(Xavier): This is for in the future when
			// individual psm->characters can have their own colors.

			unsigned char tmp_color_array[ 16 ] = { 
				255, 255, 255, 255,
				255, 255, 255, 255,
				255, 255, 255, 255,
				255, 255, 255, 255,
			};
			colors.insert( colors.end(), tmp_color_array, tmp_color_array + 16 );

			unsigned int tmp_indx_array[ 6 ] = {
				vert_ofst, vert_ofst+1, vert_ofst+2, vert_ofst, vert_ofst+2, vert_ofst+3
			};
			indis.insert( indis.end(),  tmp_indx_array, tmp_indx_array+6 );
			vert_ofst += 4;
			
			if ( alignment & ALIGNMENT::MONOSPACED ) xx -= psm->characters[ ' ' ].advance >> 6;
			else xx -= ( psm->characters[ ch ].advance >> 6 ) - psm->characters[ ch ].bearing.x;
		}
	} 

	if ( verts.size() > 0 ) {
		
		num_indices = indis.size();

		if ( vao == 0 ) glGenVertexArrays(1, &vao);
		if ( vbo_posuv == 0 ) glGenBuffers(1, &vbo_posuv);
		if ( vbo_color == 0 ) glGenBuffers(1, &vbo_color);
		if ( ebo == 0 ) glGenBuffers(1, &ebo);

		glBindVertexArray( vao );
		
			glBindBuffer( GL_ARRAY_BUFFER, vbo_posuv );
				glBufferData( GL_ARRAY_BUFFER, verts.size() * sizeof( float ), verts.data(), GL_DYNAMIC_DRAW );
			
				GLint posAttrib = glGetAttribLocation( shader, "position" );
				glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0 );
				glEnableVertexAttribArray( posAttrib );
				
				GLint texAttrib = glGetAttribLocation( shader, "texcoord" );
				glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)) );
				glEnableVertexAttribArray( texAttrib );
			
			glBindBuffer( GL_ARRAY_BUFFER, vbo_color );
				glBufferData( GL_ARRAY_BUFFER, colors.size() * sizeof( unsigned char ), colors.data(), GL_DYNAMIC_DRAW );
				
				GLint colorAttrib = glGetAttribLocation( shader, "color" );
				glVertexAttribPointer( colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)0 );
				glEnableVertexAttribArray( colorAttrib );

			glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ebo );
				glBufferData( GL_ELEMENT_ARRAY_BUFFER, indis.size() * sizeof( unsigned int ), indis.data(), GL_DYNAMIC_DRAW );
		
		glBindVertexArray( 0 );
	}
}

void Text::setPosition( glm::vec3 pos ) {
	position = pos;
	model_mtx = glm::translate( glm::mat4(1), pos );
}

void Text::render ( unsigned int shader, unsigned int texID ) {
	setUniformMat4( shader, "model", model_mtx );
	glBindTexture( GL_TEXTURE_2D, texID );
	glBindVertexArray( vao );
	glDrawElements( GL_TRIANGLES, num_indices, GL_UNSIGNED_INT, 0 );
}