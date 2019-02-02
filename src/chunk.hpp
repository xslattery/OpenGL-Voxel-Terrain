

struct Chunk {
	glm::vec3 position = glm::vec3(0);
	glm::mat4 matrix = glm::mat4(1);

	static const int SIZE = 64;

	bool blocks[SIZE][SIZE][SIZE];

	unsigned int vao = 0;
	unsigned int vbo = 0;
	unsigned int vbo_color = 0;
	unsigned int vbo_normal = 0;
	unsigned int ebo = 0;

	unsigned int numIndices = 0;

};

void createChunk ( Chunk *chunk, glm::vec3 pos );
void generateChunkMesh ( Chunk *chunk, unsigned int shaderID );
void renderChunk ( Chunk *chunk, unsigned int shaderID );