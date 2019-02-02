//
//  sprite.hpp
//  2DProject
//
//  Created by Xavier Slattery.
//  Copyright © 2015 Xavier Slattery. All rights reserved.
//

// TODO(Xavier): Move this elsewhere.
void LoadTexture( unsigned int* tex_id, const char* name );


// --------------------------------------------------------------------------------------------------------


struct Rect_Transform {
	
	glm::vec3 position = glm::vec3(0, 0, 0);
	glm::vec2 scale = glm::vec2(1, 1);
	float rotation = 0;
	
	glm::vec2 size = glm::vec2(1, 1);
	glm::vec2 pivot  = glm::vec2(0, 0);
	
	glm::mat4 matrix = glm::mat4(1);
	
	void calculateMatrix() {
		matrix = glm::scale( glm::mat4(1), glm::vec3(scale, 1) );
		matrix = glm::translate( matrix, position/glm::vec3(scale, 1) );
		matrix = glm::rotate( matrix, rotation, glm::vec3(0, 0, 1) );
		if ( rotation > 2*M_PI ) rotation -= 2*M_PI;
	}
};


// --------------------------------------------------------------------------------------------------------


struct single_colored_mesh {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int ebo = 0;
	unsigned char numIndices = 0;
	unsigned int shaderID = 0;
	glm::vec4 color = glm::vec4(1);
};

struct Colored_Sprite {

	Rect_Transform transform;
	single_colored_mesh mesh;

	Colored_Sprite() {}
	Colored_Sprite( glm::vec3 pos, glm::vec2 scale, float rot, glm::vec2 size, glm::vec2 pivot ) {
		transform.position = pos;
		transform.scale = scale;
		transform.rotation = rot;
		transform.size = size;
		transform.pivot = pivot;
	}

	~Colored_Sprite() {
		if ( mesh.vao != 0 ) glDeleteVertexArrays( 1, &mesh.vao );
		if ( mesh.vbo != 0 ) glDeleteBuffers( 1, &mesh.vbo );
		if ( mesh.ebo != 0 ) glDeleteBuffers( 1, &mesh.ebo );
	}
};

void generateColoredSpriteMesh( Colored_Sprite* colSprite );
void renderColoredSprite( Colored_Sprite* colSprite );


// --------------------------------------------------------------------------------------------------------


struct TexturedMesh {
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int vbo_tex = 0;
	unsigned int ebo = 0;
	unsigned char numIndices = 0;
	unsigned int texID = 0;
	unsigned int shaderID = 0;
};

struct Textured_Sprite {

	Rect_Transform transform;
	TexturedMesh mesh;

	Textured_Sprite() {}
	Textured_Sprite( glm::vec3 pos, glm::vec2 scale, float rot, glm::vec2 size, glm::vec2 pivot ) {
		transform.position = pos;
		transform.scale = scale;
		transform.rotation = rot;
		transform.size = size;
		transform.pivot = pivot;
	}

	~Textured_Sprite() {
		if ( mesh.vao != 0 ) glDeleteVertexArrays( 1, &mesh.vao );
		if ( mesh.vbo != 0 ) glDeleteBuffers( 1, &mesh.vbo );
		if ( mesh.vbo_tex != 0 ) glDeleteBuffers( 1, &mesh.vbo_tex );
		if ( mesh.ebo != 0 ) glDeleteBuffers( 1, &mesh.ebo );
	}
};

void generateTexturedSpriteMesh( Textured_Sprite* texSprite );
void renderTexturedSprite( Textured_Sprite* texSprite );


// --------------------------------------------------------------------------------------------------------


struct ColoredSpriteBatch {
	
	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int vbo_color = 0;
	unsigned int ebo = 0;
	unsigned int shaderID = 0;

	std::vector<GLfloat> vertices;
	std::vector<unsigned char> vertex_colors;
	std::vector<unsigned int> indices;
	unsigned int numIndices = 0;

	~ColoredSpriteBatch() {
		if ( vao != 0 ) glDeleteVertexArrays( 1, &vao );
		if ( vbo != 0 ) glDeleteBuffers( 1, &vbo );
		if ( vbo_color != 0 ) glDeleteBuffers( 1, &vbo_color );
		if ( ebo != 0 ) glDeleteBuffers( 1, &ebo );
	}
};

void prepairColoredSpriteBatchForPush( ColoredSpriteBatch* sb ); // This will clear the batch if it is already made.
void pushToColoredSpriteBatch( ColoredSpriteBatch* sb, glm::vec3 pos, glm::vec2 scale, float rot, glm::vec2 size, glm::vec2 pvt, glm::vec4 color ); // This will add a sprite to the batch.
void buildColoredSpriteBatch( ColoredSpriteBatch* sb ); // This will send off all the data to opengl. And clear the data from the vertex and index vectors.
void renderColoredSpriteBatch( ColoredSpriteBatch* sb ); // This will render the sprite batch to the screen.
