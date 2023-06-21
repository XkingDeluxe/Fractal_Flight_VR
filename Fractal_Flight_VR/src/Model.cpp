#include "Model.h"
Model::Model(Vector3f pos, ShaderFill* s) :
    Pos(pos),
    Scale(1.0f),
    Rot(),
    Mat(),
    fill(s),
    lightPos(0.0f, 1.0f, 0.0f),
    vertexBuffer(nullptr)
    //indexBuffer(nullptr)
{}

Model::~Model() {
    FreeBuffers();
}

VertexBuffer::VertexBuffer(void* vertices, void* normals, void* uvs, size_t sizeV, size_t sizeN, size_t sizeU) {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeV+sizeN+sizeU, NULL, GL_STATIC_DRAW);

    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeV, vertices);
    glBufferSubData(GL_ARRAY_BUFFER, sizeV, sizeN, normals);
    glBufferSubData(GL_ARRAY_BUFFER, sizeV + sizeN, sizeU, uvs);
}
VertexBuffer::~VertexBuffer() {
    if (buffer) {
        glDeleteBuffers(1, &buffer);
        buffer = 0;
    }
}

/*IndexBuffer::IndexBuffer(void* indices, size_t size) {
    glGenBuffers(1, &buffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, indices, GL_STATIC_DRAW);
}
IndexBuffer::~IndexBuffer() {
    if (buffer)
    {
        glDeleteBuffers(1, &buffer);
        buffer = 0;
    }
}*/

void Model::AllocateBuffers() {
    vertexBuffer = new VertexBuffer(&vertices[0], &normals[0], &UVs[0], vertices.size() * sizeof(Vector3f), normals.size() * sizeof(Vector3f), UVs.size() * sizeof(Vector3f));
    //indexBuffer = new IndexBuffer(&indices[0], indices.size() * sizeof(GLuint));
}
void Model::FreeBuffers() {
    delete vertexBuffer; vertexBuffer = nullptr;
    //delete indexBuffer; indexBuffer = nullptr;
}

Matrix4f Model::getMatrix() {
    
    Mat = Matrix4f::Scaling(Scale);
    Mat = Matrix4f(Rot) * Mat;
    Mat *= Matrix4f::RotationX(RotOffset.x);
    Mat *= Matrix4f::RotationY(RotOffset.y);
    
    Mat *= Matrix4f::RotationZ(RotOffset.z);
    
    Mat = Matrix4f::Translation(Pos) * Mat;

    return Mat;
}


/*void Model::AddIndex(GLuint i) {
    indices.push_back(i);
}*/

bool Model::LoadOBJ(std::string filePath) {
    FILE* file = fopen(filePath.c_str(), "r");
    if (file == NULL) {
        printf("unable to read", filePath);
        return false;
    }
    std::vector<GLuint> vertexIndices, uvIndices, normalIndices;
    std::vector<Vector3f> temp_vertices;
    std::vector<Vector2f> temp_uvs;
    std::vector<Vector3f> temp_normals;

    while (true) {
        char lineHeader[128];
        int res = fscanf(file, "%s", lineHeader);
        if (res == EOF)
            break; //quitloop

        if (strcmp(lineHeader, "v") == 0) {
            Vector3f temp_vert;
            fscanf(file, "%f %f %f\n", &temp_vert.x, &temp_vert.y, &temp_vert.z);
            temp_vertices.push_back(temp_vert);
        }
        else if (strcmp(lineHeader, "vt") == 0) {
            Vector2f temp_uv;
            fscanf(file, "%f %f\n", &temp_uv.x, &temp_uv.y);
            temp_uvs.push_back(temp_uv);
        }
        else if (strcmp(lineHeader, "vn") == 0) {
            Vector3f temp_normal;
            fscanf(file, "%f %f %f\n", &temp_normal.x, &temp_normal.y, &temp_normal.z);
            temp_normals.push_back(temp_normal);
        }
        else if (strcmp(lineHeader, "f") == 0) {
            std::string vertex1, vertex2, vertex3;
            GLuint vertexIndex[3], uvIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
            if (matches != 9) {
                printf("File can't be read by our simple parser : ( Try exporting with other options\n");
                return false;
            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            uvIndices.push_back(uvIndex[0]);
            uvIndices.push_back(uvIndex[1]);
            uvIndices.push_back(uvIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        }
    }


    for (int i = 0; i < vertexIndices.size(); i++) {
        unsigned int vertexIndex = vertexIndices[i];
        Vector3f vert = temp_vertices[vertexIndex - 1];
        vertices.push_back(vert);
    }
    for (int i = 0; i < normalIndices.size(); i++) {
        unsigned int normIndex = normalIndices[i];
        Vector3f norm = temp_normals[normIndex - 1];
        normals.push_back(norm);
    }
    for (int i = 0; i < uvIndices.size(); i++) {
        unsigned int uvIndex = uvIndices[i];
        Vector2f uv = temp_uvs[uvIndex - 1];
        UVs.push_back(uv);
    }
    
    std::cout << "s";
}

void Model::Render(Matrix4f view, Matrix4f proj, Vector3f playerPos) {

    glUseProgram(fill->program);
    glUniform3f(glGetUniformLocation(fill->program, "light.ambient"), 1.2f, 1.2f, 1.2f);
    glUniform3f(glGetUniformLocation(fill->program, "light.diffuse"), 5.50f, 5.50f, 5.5f);
    glUniform3f(glGetUniformLocation(fill->program, "light.specular"), 2.0f, 2.0f, 2.0f);

    glUniform3f(glGetUniformLocation(fill->program, "material.ambient"), 1.0f, 0.5f, 0.31f);
    glUniform3f(glGetUniformLocation(fill->program, "material.diffuse"), 1.0f, 0.5f, 0.31f);
    glUniform3f(glGetUniformLocation(fill->program, "material.specular"), 0.5f, 0.5f, 0.5f);
    glUniform1f(glGetUniformLocation(fill->program, "material.shininess"), 30.0f);

    glUniform1f(glGetUniformLocation(fill->program, "time"), ovr_GetTimeInSeconds());

    GLint lightPosLoc = glGetUniformLocation(fill->program, "light.position");
    GLint viewPosLoc = glGetUniformLocation(fill->program, "viewPos");

    glUniform3f(lightPosLoc, lightPos.x, lightPos.y, lightPos.z);
    glUniform3f(viewPosLoc, playerPos.x, playerPos.y, playerPos.z);

    Matrix4f mod = getMatrix();

    glUniformMatrix4fv(glGetUniformLocation(fill->program, "model"), 1, GL_TRUE, (FLOAT*)&mod);
    glUniformMatrix4fv(glGetUniformLocation(fill->program, "view"), 1, GL_TRUE, (FLOAT*)&view);
    glUniformMatrix4fv(glGetUniformLocation(fill->program, "projection"), 1, GL_TRUE, (FLOAT*)&proj);

    glBindTexture(GL_TEXTURE_2D, fill->diffuseMap);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, fill->specularMap);
    glActiveTexture(GL_TEXTURE0);

    glBindBuffer(GL_ARRAY_BUFFER, vertexBuffer->getBuffer());

    GLuint posLoc = glGetAttribLocation(fill->program, "position");
    GLuint normalLoc = glGetAttribLocation(fill->program, "normal");
    GLuint uvLoc = glGetAttribLocation(fill->program, "texCoords");

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)0);
    glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, (const void*)(sizeof(Vector3f) * vertices.size()));
    glVertexAttribPointer(uvLoc, 2, GL_FLOAT, GL_FALSE, 0, (const void*)(sizeof(Vector3f) * vertices.size() + sizeof(Vector3f) * vertices.size()));


    glDrawArrays(GL_TRIANGLES, 0, vertices.size());

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);

    glUseProgram(0);
}

void Model::MoveToPos(Vector3f pos) {
    Pos = pos;
}

void Model::ScaleObj(Vector3f sc) {
    Scale = sc;
}
void Model::RotateObj(Quatf rotation, Vector3f rotOffset) {
    Rot = rotation;
    RotOffset = rotOffset;
}