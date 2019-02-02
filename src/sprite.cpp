//
//  sprite.cpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <vector>

#include "typedefs.hpp"
#include "shader.hpp"
#include "sprite.hpp"

void LoadTexture( unsigned int* tex_id, const char* name ) {
	int texWidth, texHeight, n;
	unsigned char* bitmap = stbi_load( name, &texWidth, &texHeight, &n, 4 );

	if ( bitmap ) {
		if( *tex_id == 0 ) { glGenTextures(1, tex_id); }

		glBindTexture(GL_TEXTURE_2D, *tex_id);
		glTexImage2D( GL_TEXTURE_2D, 0, GL_RGBA, texWidth, texHeight, 0, GL_RGBA, GL_UNSIGNED_BYTE, bitmap );
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

		stbi_image_free( bitmap );
	} else {
		ERROR( "Failed to load image.\n" );
	}
}


// ---------------------------------------------------------------------------------------------------------------------


void generateColoredSpriteMesh( Colored_Sprite* colSprite ) {

	GLfloat verts [12] = {
		-colSprite->transform.pivot.x * colSprite->transform.size.x, 		-colSprite->transform.pivot.y * colSprite->transform.size.y, 		0, 
		(1.0f-colSprite->transform.pivot.x) * colSprite->transform.size.x,  -colSprite->transform.pivot.y * colSprite->transform.size.y, 		0, 
		-colSprite->transform.pivot.x * colSprite->transform.size.x, 		(1.0f-colSprite->transform.pivot.y) * colSprite->transform.size.y, 	0,
		(1.0f-colSprite->transform.pivot.x) * colSprite->transform.size.x, 	(1.0f-colSprite->transform.pivot.y) * colSprite->transform.size.y, 	0,
	};

	unsigned char indices [6] = {
		0, 1, 2, 
		1, 3, 2
	};
	colSprite->mesh.numIndices = 6;

	if ( colSprite->mesh.vao == 0 ) glGenVertexArrays( 1, &colSprite->mesh.vao );
	if ( colSprite->mesh.vbo == 0 ) glGenBuffers( 1, &colSprite->mesh.vbo );
	if ( colSprite->mesh.ebo == 0 ) glGenBuffers( 1, &colSprite->mesh.ebo );

	glBindVertexArray( colSprite->mesh.vao );

		glBindBuffer( GL_ARRAY_BUFFER, colSprite->mesh.vbo );
			glBufferData( GL_ARRAY_BUFFER, sizeof(verts) * sizeof(GLfloat), verts, GL_STATIC_DRAW );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, colSprite->mesh.ebo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(unsigned char), indices, GL_STATIC_DRAW );

		unsigned int posAttrib = glGetAttribLocation( colSprite->mesh.shaderID, "position" );
		glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
		glEnableVertexAttribArray( posAttrib );
	
	glBindVertexArray( 0 );
}

void renderColoredSprite( Colored_Sprite* colSprite ) {
	setUniformMat4( colSprite->mesh.shaderID, "model", colSprite->transform.matrix );
	setUniform4f( colSprite->mesh.shaderID, "color", colSprite->mesh.color );
	glBindVertexArray( colSprite->mesh.vao );
	glDrawElements( GL_TRIANGLES, colSprite->mesh.numIndices, GL_UNSIGNED_BYTE, 0 );
}


// ---------------------------------------------------------------------------------------------------------------------


void generateTexturedSpriteMesh( Textured_Sprite* texSprite ) {

	GLfloat positions [20] = {
		-texSprite->transform.pivot.x * texSprite->transform.size.x, 		-texSprite->transform.pivot.y * texSprite->transform.size.y, 		0,
		(1.0f-texSprite->transform.pivot.x) * texSprite->transform.size.x,  -texSprite->transform.pivot.y * texSprite->transform.size.y, 		0,
		-texSprite->transform.pivot.x * texSprite->transform.size.x, 		(1.0f-texSprite->transform.pivot.y) * texSprite->transform.size.y, 	0,
		(1.0f-texSprite->transform.pivot.x) * texSprite->transform.size.x, 	(1.0f-texSprite->transform.pivot.y) * texSprite->transform.size.y, 	0,
	};

	GLfloat texcoords [8] = {
		0.0f, 1.0f,
		1.0f, 1.0f,
		0.0f, 0.0f,
		1.0f, 0.0f,
	};

	unsigned char indices [6] = {
		0, 1, 2, 
		1, 3, 2
	};
	texSprite->mesh.numIndices = 6;

	if ( texSprite->mesh.vao == 0 ) glGenVertexArrays( 1, &texSprite->mesh.vao );
	if ( texSprite->mesh.vbo == 0 ) glGenBuffers( 1, &texSprite->mesh.vbo );
	if ( texSprite->mesh.vbo_tex == 0 ) glGenBuffers( 1, &texSprite->mesh.vbo_tex );
	if ( texSprite->mesh.ebo == 0 ) glGenBuffers( 1, &texSprite->mesh.ebo );

	glBindVertexArray( texSprite->mesh.vao );

		glBindBuffer( GL_ARRAY_BUFFER, texSprite->mesh.vbo );
			glBufferData( GL_ARRAY_BUFFER, sizeof(positions) * sizeof(GLfloat), positions, GL_STATIC_DRAW );

			unsigned int posAttrib = glGetAttribLocation( texSprite->mesh.shaderID, "position" );
			glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( posAttrib );

		glBindBuffer( GL_ARRAY_BUFFER, texSprite->mesh.vbo_tex );
			glBufferData( GL_ARRAY_BUFFER, sizeof(texcoords) * sizeof(GLfloat), texcoords, GL_STATIC_DRAW );

			unsigned int texAttrib = glGetAttribLocation( texSprite->mesh.shaderID, "texcoord" );
			glVertexAttribPointer( texAttrib, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( texAttrib );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, texSprite->mesh.ebo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof(indices) * sizeof(unsigned char), indices, GL_STATIC_DRAW );

	
	glBindVertexArray( 0 );
}

void renderTexturedSprite( Textured_Sprite* texSprite ) {
	setUniformMat4( texSprite->mesh.shaderID, "model", texSprite->transform.matrix );
	glBindTexture( GL_TEXTURE_2D, texSprite->mesh.texID );
	glBindVertexArray( texSprite->mesh.vao );
	glDrawElements( GL_TRIANGLES, texSprite->mesh.numIndices, GL_UNSIGNED_BYTE, 0 );
}


// ---------------------------------------------------------------------------------------------------------------------


void prepairColoredSpriteBatchForPush( ColoredSpriteBatch* sb ) {
	sb->vertices.clear();
	sb->vertex_colors.clear();
	sb->indices.clear();
	sb->numIndices = 0;
}


void pushToColoredSpriteBatch( ColoredSpriteBatch* sb, glm::vec3 pos, glm::vec2 scale, float rot, glm::vec2 size, glm::vec2 pvt, glm::vec4 color ) {

	unsigned int tmp_v = (unsigned int)(sb->vertices.size()/3);
	unsigned int tmp_indices [6] = {
		tmp_v+0, tmp_v+1, tmp_v+2, 
		tmp_v+1, tmp_v+3, tmp_v+2
	};
	sb->numIndices += 6;
	sb->indices.insert( sb->indices.end(), tmp_indices, tmp_indices + 6 );

	GLfloat x1 = -pvt.x * size.x * scale.x;
	GLfloat y1 = -pvt.y * size.y * scale.y;

	GLfloat x2 = (1.0f-pvt.x) * size.x * scale.x;
	GLfloat y2 = (1.0f-pvt.y) * size.y * scale.y;

	glm::vec2 xy1 = glm::rotate( glm::vec2(x1, y1), rot );
	glm::vec2 xy2 = glm::rotate( glm::vec2(x2, y1), rot );
	glm::vec2 xy3 = glm::rotate( glm::vec2(x1, y2), rot );
	glm::vec2 xy4 = glm::rotate( glm::vec2(x2, y2), rot );

	GLfloat tmp_vert_array[ 12 ] = { 
		xy1.x + pos.x,  	xy1.y + pos.y, 		pos.z,
		xy2.x + pos.x,  	xy2.y + pos.y, 		pos.z,
		xy3.x + pos.x, 		xy3.y + pos.y, 		pos.z,
		xy4.x + pos.x, 		xy4.y + pos.y, 		pos.z,
	};
	sb->vertices.insert( sb->vertices.end(), tmp_vert_array, tmp_vert_array + 12 );

	unsigned char tmp_color_array[ 16 ] = { 
		255*color.x, 255*color.y, 255*color.z, 255*color.w,
		255*color.x, 255*color.y, 255*color.z, 255*color.w,
		255*color.x, 255*color.y, 255*color.z, 255*color.w,
		255*color.x, 255*color.y, 255*color.z, 255*color.w,
	};
	sb->vertex_colors.insert( sb->vertex_colors.end(), tmp_color_array, tmp_color_array + 16 );
}

void buildColoredSpriteBatch( ColoredSpriteBatch* sb ) {
	if ( sb->vao == 0 ) glGenVertexArrays( 1, &sb->vao );
	if ( sb->vbo == 0 ) glGenBuffers( 1, &sb->vbo );
	if ( sb->vbo_color == 0 ) glGenBuffers( 1, &sb->vbo_color );
	if ( sb->ebo == 0 ) glGenBuffers( 1, &sb->ebo );

	glBindVertexArray( sb->vao );

		glBindBuffer( GL_ARRAY_BUFFER, sb->vbo );
			glBufferData( GL_ARRAY_BUFFER, sb->vertices.size() * sizeof(GLfloat), sb->vertices.data(), GL_DYNAMIC_DRAW );

			unsigned int posAttrib = glGetAttribLocation( sb->shaderID, "position" );
			glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( posAttrib );

		glBindBuffer( GL_ARRAY_BUFFER, sb->vbo_color );
			glBufferData( GL_ARRAY_BUFFER, sb->vertex_colors.size() * sizeof(unsigned char), sb->vertex_colors.data(), GL_DYNAMIC_DRAW );

			unsigned int colorAttrib = glGetAttribLocation( sb->shaderID, "color" );
			glVertexAttribPointer( colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)0 );
			glEnableVertexAttribArray( colorAttrib );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, sb->ebo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, sb->indices.size() * sizeof(unsigned int), sb->indices.data(), GL_DYNAMIC_DRAW );
	
	glBindVertexArray( 0 );
}

void renderColoredSpriteBatch( ColoredSpriteBatch* sb ) {
	setUniformMat4( sb->shaderID, "model", glm::mat4(1) );
	glBindVertexArray( sb->vao );
	glDrawElements( GL_TRIANGLES, sb->numIndices, GL_UNSIGNED_INT, 0 );
}
