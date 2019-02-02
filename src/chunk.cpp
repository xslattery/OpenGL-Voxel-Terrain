
#include <OpenGL/gl3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <iostream>

#include "perlin.hpp"
#include "simplex.hpp"
#include "shader.hpp"
#include "chunk.hpp"

float aoAmmount = 0.25f;

extern unsigned long long vertexDataAmmount; 

static float generateHeightmap ( float xx, float zz, float scale, int octaves, float persistance, float lacunarity, bool power ) {

	if ( scale <= 0 ) scale = 0.0001f;
	if ( octaves < 1 ) octaves = 1;
	if ( persistance > 1 ) persistance = 1;
	if ( persistance < 0 ) persistance = 0;
	if ( lacunarity < 1 ) lacunarity = 1;

	float amplitude = 1.0f;
	float frequency = 1.0f;
	float noiseValue = 0.0f;

	for ( int i = 0; i < octaves; ++i ) {

		float sampleX = xx / scale * frequency;
		float sampleZ = zz / scale * frequency;

		// float nv = pow(2.71828182845904523536, noise_2d(sampleX, sampleZ));
		float nv = noise_2d(sampleX, sampleZ);

		noiseValue += nv * amplitude;

		amplitude *= persistance;
		frequency *= lacunarity;

	}

	if ( power ) noiseValue = pow(2.71828182845904523536, noiseValue);

	return noiseValue; 

}

void createChunk ( Chunk *chunk, glm::vec3 pos ) {
	chunk->position = pos;
	chunk->matrix = glm::translate( glm::mat4(1), pos );

	float lowestVal = 100;

	for (int x = 0; x < chunk->SIZE; ++x) {
		for (int z = 0; z < chunk->SIZE; ++z) {
			for (int y = 0; y < chunk->SIZE; ++y) {

				// if ( noise_2d((x+pos.x)/100.0f, (z+pos.z)/100.0f) * 10 + noise_2d((x+pos.x)/25.0f+243, (z+pos.z)/25.0f+78326) * 6 + noise_2d((x+pos.x)/10.0f+63, (z+pos.z)/10.0f+1293) * 3 > (y+pos.y) ) {

				chunk->blocks[x][z][y] = 0;

				float yHeight = y+pos.y;

				// if ( val < lowestVal ) lowestVal = val;

				// (y+pos.y)/50.0f
				// if ( noise( (x+pos.x)/50.0f, (y+pos.y)/50.0f, (z+pos.z)/50.0f ) > 1.25f )
					// yHeight = yHeight + noise( (x+pos.x)/20.0f, (y+pos.y)/50.0f, (z+pos.z)/20.0f ) * 15.0f - 9;
				
				if ( generateHeightmap( x+pos.x, z+pos.z, 350, 4, 0.5f, 2.5f, 1 )*15.0f > yHeight ) {
						chunk->blocks[x][z][y] = 1;
				}

				float val = generateHeightmap( x+pos.x, z+pos.z, 350, 4, 0.5f, 2.5f, 0 );
				if ( val > -1.05f && val < -0.9f ) {
					if ( yHeight > 4 ) chunk->blocks[x][z][y] = 0;
					// yHeight = 64+pos.y-2;
				}

			}
		}
	}

// 	std::cout << lowestVal << "\n";
}

struct ChunkMeshData {
	std::vector<float> vertices;
	std::vector<unsigned char> vertex_colors;
	std::vector<float> normals;
	std::vector<unsigned int> indices;
};

static void createFace ( Chunk *chunk, ChunkMeshData *cmd, glm::vec3 pos, glm::vec3 dir, glm::vec4 color, float bl, float br, float tl, float tr ) {
	unsigned int vOff = cmd->vertices.size()/3;
	unsigned int tmp_indices [6] = {
		vOff+0, vOff+1, vOff+2, // Z-NEG
		vOff+1, vOff+3, vOff+2,
	};
	cmd->indices.insert( cmd->indices.end(), tmp_indices, tmp_indices + 6 );

	if ( dir.z != 0 ) {
		float tmp_vert_array[ 12 ] = { 
			pos.x+( dir.z == -1 ? 0 : 1 ),		pos.y+0,		pos.z+( dir.z == -1 ? 0 : 1 ),
			pos.x+( dir.z == -1 ? 1 : 0 ),		pos.y+0,		pos.z+( dir.z == -1 ? 0 : 1 ),
			pos.x+( dir.z == -1 ? 0 : 1 ),		pos.y+1,		pos.z+( dir.z == -1 ? 0 : 1 ),
			pos.x+( dir.z == -1 ? 1 : 0 ),		pos.y+1,		pos.z+( dir.z == -1 ? 0 : 1 ),
		};
		cmd->vertices.insert( cmd->vertices.end(), tmp_vert_array, tmp_vert_array + 12 );
	} else if ( dir.x != 0 ) {
		float tmp_vert_array[ 12 ] = { 
			pos.x+( dir.x == -1 ? 0 : 1 ),		pos.y+0,		pos.z+( dir.x == -1 ? 1 : 0 ),
			pos.x+( dir.x == -1 ? 0 : 1 ),		pos.y+0,		pos.z+( dir.x == -1 ? 0 : 1 ),
			pos.x+( dir.x == -1 ? 0 : 1 ),		pos.y+1,		pos.z+( dir.x == -1 ? 1 : 0 ),
			pos.x+( dir.x == -1 ? 0 : 1 ),		pos.y+1,		pos.z+( dir.x == -1 ? 0 : 1 ),
		};
		cmd->vertices.insert( cmd->vertices.end(), tmp_vert_array, tmp_vert_array + 12 );
	} else if ( dir.y != 0 ) {
		float tmp_vert_array[ 12 ] = { 
			pos.x+0,		pos.y+( dir.y == -1 ? 0 : 1 ),		pos.z+( dir.y == -1 ? 1 : 0 ),
			pos.x+1,		pos.y+( dir.y == -1 ? 0 : 1 ),		pos.z+( dir.y == -1 ? 1 : 0 ),
			pos.x+0,		pos.y+( dir.y == -1 ? 0 : 1 ),		pos.z+( dir.y == -1 ? 0 : 1 ),
			pos.x+1,		pos.y+( dir.y == -1 ? 0 : 1 ),		pos.z+( dir.y == -1 ? 0 : 1 ),
		};
		cmd->vertices.insert( cmd->vertices.end(), tmp_vert_array, tmp_vert_array + 12 );
	}

	float tmp_normal_array[ 12 ] = { 
		dir.x, dir.y, dir.z,
		dir.x, dir.y, dir.z,
		dir.x, dir.y, dir.z,
		dir.x, dir.y, dir.z,
	};
	cmd->normals.insert( cmd->normals.end(), tmp_normal_array, tmp_normal_array + 12 );	

	bl = (1-bl) * aoAmmount;
	br = (1-br) * aoAmmount;
	tl = (1-tl) * aoAmmount;
	tr = (1-tr) * aoAmmount;

	unsigned char tmp_color_array[ 16 ] = { 
		color.x*255*(1-bl), color.y*255*(1-bl), color.z*255*(1-bl), color.w*255,
		color.x*255*(1-br), color.y*255*(1-br), color.z*255*(1-br), color.w*255,
		color.x*255*(1-tl), color.y*255*(1-tl), color.z*255*(1-tl), color.w*255,
		color.x*255*(1-tr), color.y*255*(1-tr), color.z*255*(1-tr), color.w*255,
	};
	cmd->vertex_colors.insert( cmd->vertex_colors.end(), tmp_color_array, tmp_color_array + 16 );
}

static float vertexAO(bool side1, bool side2, bool corner) {
	if( side1 && side2 ) {
  		return 0;
  	}
  	return 3.0f - (side1 + side2 + corner);
}

static bool getBlock ( Chunk *chunk, int x, int z, int y ) {
	if ( x >= 0 && z >= 0 && y >= 0 && x < Chunk::SIZE && z < Chunk::SIZE && y < Chunk::SIZE ) {
		return chunk->blocks[x][z][y];
	} else {
		return 0;
	}

}

void generateChunkMesh ( Chunk *chunk, unsigned int shaderID ) {

	ChunkMeshData cmd;
	chunk->numIndices = 0;

	for (int x = 0; x < Chunk::SIZE; ++x) {
		for (int z = 0; z < Chunk::SIZE; ++z) {
			for (int y = 0; y < Chunk::SIZE; ++y) {
				
				if ( getBlock(chunk, x, z, y) == 1 ) {

					float nVal = ( noise_2d((x+chunk->position.x)/40.0f+62348, (z+chunk->position.z)/40.0f+1238) + 1.0f ) / 2.0f;
					glm::vec4 bColor = glm::vec4(1, 0.9f, 0.4f, 1)*(nVal) + glm::vec4(1, 0.8f, 0.3f, 1)*(1.0f-nVal);

					// float tmpVal = generateHeightmap( x+chunk->position.x, z+chunk->position.z, 500, 4, 0.6f, 2.5f );
					// if ( tmpVal > 1 ) tmpVal = 1;
					// nVal = ( tmpVal + 1.0f ) / 2.0f;
					// bColor = glm::vec4(0.5f, 0.5f, 0.5f, 1)*(nVal) + bColor*(1.0f-nVal);

					float val = generateHeightmap( x+chunk->position.x, z+chunk->position.z, 350, 4, 0.5f, 2.5f, 0 );

					if ( -val > 0.7f && -val < 0.9f ) {
						float nnv = (-val - 0.7f) * 5.0f;
						// nnv = nnv*nnv;
						nnv *= 0.7f;
						bColor = glm::vec4(59/255.0f, 163/255.0f, 11/255.0f, 1)*(nnv) + bColor*(1.0f-nnv);
					}
					// 91/255.0f, 209/255.0f, 37/255.0f
					//59, 163, 11
					if ( val > -1.25f && val < -1.05f ) {
						float nnv = (val + 1.25f) * 5.0f;
						// nnv = nnv*nnv;
						nnv *= 0.7f;
						bColor = glm::vec4(59/255.0f, 163/255.0f, 11/255.0f, 1)*(nnv) + bColor*(1.0f-nnv);
					}

					if ( val > -1.05f && val < -0.9f ) {
						bColor = glm::vec4(0.3, 0.46, 0.94f, 1); // 69, 171, 239
					}

					if ( getBlock(chunk, x, z, y+1) == 1 ) {
						bColor = glm::vec4(0.7, 0.7, 0.7f, 1);
					}

					if (y+chunk->position.y > 10) {
						nVal = (y+chunk->position.y - 10.0f) / 50.0f;
						if ( nVal > 1 ) nVal = 1;

						float nVal2 = ( noise_2d((x+chunk->position.x)/40.0f+324, (z+chunk->position.z)/40.0f+876) + 1.0f ) / 2.0f;
						glm::vec4 bColor2 = glm::vec4(200/255.0f, 140/255.0f, 70/255.0f, 1)*(nVal2) + glm::vec4(0.7f, 0.5, 0.3f, 1)*(1.0f-nVal2);

						bColor = bColor2*(nVal) + bColor*(1.0f-nVal);

						if (y+chunk->position.y > 50 + noise_2d((x+chunk->position.x)/50.0f+151, (z+chunk->position.z)/50.0f+369) * 5.0f ) {
							nVal = (y+chunk->position.y - (50.0f + noise_2d((x+chunk->position.x)/50.0f+151, (z+chunk->position.z)/50.0f+369) * 5.0f) ) / 20.0f;
							if ( nVal > 1 ) nVal = 1;

							nVal2 = ( noise_2d((x+chunk->position.x)/40.0f+324, (z+chunk->position.z)/40.0f+876) + 1.0f ) / 2.0f;
							bColor2 = glm::vec4(0.6f, 0.6f, 0.6f, 1)*(nVal2) + glm::vec4(0.5f, 0.5, 0.5f, 1)*(1.0f-nVal2);

							bColor = bColor2*(nVal) + bColor*(1.0f-nVal);
						}
					}

					if ( getBlock(chunk, x, z-1, y) == 0 ) {
						float p0 = vertexAO( getBlock(chunk, x-1, z-1, y), getBlock(chunk, x, z, y), getBlock(chunk, x-1, z, y) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x-1, z-1, y+1), getBlock(chunk, x, z, y+1), getBlock(chunk, x-1, z, y+1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x+1, z-1, y), getBlock(chunk, x+1, z, y) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x+1, z-1, y+1), getBlock(chunk, x+1, z, y+1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(0, 0, -1), bColor, p0, p1, p2, p3 );
					}					

					if ( getBlock(chunk, x, z+1, y) == 0 ) {
						float p0 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x+1, z+1, y), getBlock(chunk, x+1, z, y) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x+1, z+1, y+1), getBlock(chunk, x+1, z, y+1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x-1, z+1, y), getBlock(chunk, x-1, z, y) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x-1, z+1, y+1), getBlock(chunk, x-1, z, y+1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(0, 0, 1), bColor, p0, p1, p2, p3 );
					}

					if ( getBlock(chunk, x-1, z, y) == 0 ) {		
						float p0 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x-1, z+1, y), getBlock(chunk, x, z+1, y) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x-1, z+1, y+1), getBlock(chunk, x, z+1, y+1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x-1, z-1, y), getBlock(chunk, x, z-1, y) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x-1, z-1, y+1), getBlock(chunk, x, z-1, y+1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(-1, 0, 0), bColor, p0, p1, p2, p3 );
					}
					
					if ( getBlock(chunk, x+1, z, y) == 0 ) {				
						float p0 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x+1, z-1, y), getBlock(chunk, x, z-1, y) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x+1, z-1, y+1), getBlock(chunk, x, z-1, y+1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x, z, y), getBlock(chunk, x+1, z+1, y), getBlock(chunk, x, z+1, y) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x, z, y+1), getBlock(chunk, x+1, z+1, y+1), getBlock(chunk, x, z+1, y+1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(1, 0, 0), bColor, p0, p1, p2, p3 );
					}
					
					if ( getBlock(chunk, x, z, y-1) == 0 ) {
						float p0 = vertexAO( getBlock(chunk, x-1, z, y-1), getBlock(chunk, x, z+1, y-1), getBlock(chunk, x-1, z+1, y-1) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x-1, z, y-1), getBlock(chunk, x, z-1, y-1), getBlock(chunk, x-1, z-1, y-1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x+1, z, y-1), getBlock(chunk, x, z+1, y-1), getBlock(chunk, x+1, z+1, y-1) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x+1, z, y-1), getBlock(chunk, x, z-1, y-1), getBlock(chunk, x+1, z-1, y-1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(0, -1, 0), bColor, p0, p1, p2, p3 );
					}
					
					if ( getBlock(chunk, x, z, y+1) == 0 ) {
						float p0 = vertexAO( getBlock(chunk, x-1, z, y+1), getBlock(chunk, x, z-1, y+1), getBlock(chunk, x-1, z-1, y+1) ) / 3.0f;
						float p2 = vertexAO( getBlock(chunk, x-1, z, y+1), getBlock(chunk, x, z+1, y+1), getBlock(chunk, x-1, z+1, y+1) ) / 3.0f;
						float p1 = vertexAO( getBlock(chunk, x+1, z, y+1), getBlock(chunk, x, z-1, y+1), getBlock(chunk, x+1, z-1, y+1) ) / 3.0f;
						float p3 = vertexAO( getBlock(chunk, x+1, z, y+1), getBlock(chunk, x, z+1, y+1), getBlock(chunk, x+1, z+1, y+1) ) / 3.0f;
						createFace( chunk, &cmd, glm::vec3(x,y,z), glm::vec3(0, 1, 0), bColor, p0, p1, p2, p3 );
					}
					
				}

			}
		}
	}

	chunk->numIndices = cmd.indices.size();

	// ========================================

	if ( chunk->vao == 0 ) glGenVertexArrays( 1, &chunk->vao );
	if ( chunk->vbo == 0 ) glGenBuffers( 1, &chunk->vbo );
	if ( chunk->vbo_normal == 0 ) glGenBuffers( 1, &chunk->vbo_normal );
	if ( chunk->vbo_color == 0 ) glGenBuffers( 1, &chunk->vbo_color );
	if ( chunk->ebo == 0 ) glGenBuffers( 1, &chunk->ebo );

	glBindVertexArray( chunk->vao );

		glBindBuffer( GL_ARRAY_BUFFER, chunk->vbo );
			glBufferData( GL_ARRAY_BUFFER, cmd.vertices.size() * sizeof(GLfloat), cmd.vertices.data(), GL_DYNAMIC_DRAW );

			unsigned int posAttrib = glGetAttribLocation( shaderID, "position" );
			glVertexAttribPointer( posAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( posAttrib );

		glBindBuffer( GL_ARRAY_BUFFER, chunk->vbo_normal );
			glBufferData( GL_ARRAY_BUFFER, cmd.normals.size() * sizeof(GLfloat), cmd.normals.data(), GL_DYNAMIC_DRAW );

			unsigned int normalAttrib = glGetAttribLocation( shaderID, "normal" );
			glVertexAttribPointer( normalAttrib, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (void*)0 );
			glEnableVertexAttribArray( normalAttrib );

		glBindBuffer( GL_ARRAY_BUFFER, chunk->vbo_color );
			glBufferData( GL_ARRAY_BUFFER, cmd.vertex_colors.size() * sizeof(unsigned char), cmd.vertex_colors.data(), GL_DYNAMIC_DRAW );

			unsigned int colorAttrib = glGetAttribLocation( shaderID, "color" );
			glVertexAttribPointer( colorAttrib, 4, GL_UNSIGNED_BYTE, GL_TRUE, 4 * sizeof(unsigned char), (void*)0 );
			glEnableVertexAttribArray( colorAttrib );

		glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, chunk->ebo );
			glBufferData( GL_ELEMENT_ARRAY_BUFFER, cmd.indices.size() * sizeof(unsigned int), cmd.indices.data(), GL_DYNAMIC_DRAW );
	
	glBindVertexArray( 0 );

	vertexDataAmmount += cmd.vertices.size()*(136-24) + cmd.vertices.size()*24;
}

void renderChunk ( Chunk *chunk, unsigned int shaderID ) {
	setUniformMat4( shaderID, "model", chunk->matrix );
	glBindVertexArray( chunk->vao );
	glDrawElements( GL_TRIANGLES, chunk->numIndices, GL_UNSIGNED_INT, 0 );
}