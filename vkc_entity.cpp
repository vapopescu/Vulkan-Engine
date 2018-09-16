#include "vkc_entity.h"

/**
 * Create the entity.
 */
VkResult VkcEntity::create(const VkcDevice *pDevice, QString name)
{
    // Save the device.
    this->pDevice = pDevice;

    // Open the model file.
    QFile file("data/models/" + name + ".fbx");

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ')
               .append("- Model \"%1\" not found").arg(name).toStdString().c_str());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Read the data.
    QByteArray fileData = file.readAll();
    file.close();

    // Import assimp scene.
    Assimp::Importer importer;
    QString format = name.remove(0, name.indexOf('.') + 1);

    const aiScene *scene = importer.ReadFileFromMemory(
                fileData.data(),
                static_cast<uint32_t>(fileData.size()),
                aiProcess_JoinIdenticalVertices |
                aiProcess_Triangulate |
                aiProcess_SortByPType |
                aiProcess_ImproveCacheLocality |
                aiProcess_CalcTangentSpace |
                aiProcess_LimitBoneWeights |
                aiProcess_FlipUVs
                ,
                format.toStdString().data());

    if (scene == nullptr)
    {
        qDebug(importer.GetErrorString());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // If the scene has no meshes, return.
    if(!scene->HasMeshes())
    {
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ')
               .append("- Model \"%1\" has no meshes").arg(name).toStdString().c_str());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Select the mesh containing triangles.
    aiMesh *mesh = nullptr;

    for (uint32_t meshIdx = 0; meshIdx < scene->mNumMeshes; meshIdx++)
    {
        if (scene->mMeshes[meshIdx]->mPrimitiveTypes & aiPrimitiveType_TRIANGLE)
        {
            mesh = scene->mMeshes[meshIdx];
        }
    }

    // If no such mesh was found, return.
    if(mesh == nullptr)
    {
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ')
               .append("- Model \"%1\" has no mesh with triangles").arg(name).toStdString().c_str());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    // Load vertices.
    for (uint32_t i = 0; i < mesh->mNumVertices; i++)
    {
        aiVector3D position = mesh->mVertices[i];
        aiVector3D texCoord = mesh->mTextureCoords[0][i];
        aiVector3D normal = mesh->mNormals[i];
        aiVector3D tangent = mesh->mTangents[i];
        aiVector3D bitangent = mesh->mBitangents[i];

        MgVertex vertex =
        {
            position.x, position.y, position.z,
            texCoord.x, texCoord.y, texCoord.z,
            normal.x, normal.y, normal.z,
            tangent.x, tangent.y, tangent.z,
            bitangent.x, bitangent.y, bitangent.z
        };

        vertices.append(vertex);
    }

    // Load indices.
    for (uint32_t i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];

        if (face.mNumIndices == 3)
        {
            for (uint32_t j = 0; j < 3; j++)
            {
                indices.append(face.mIndices[j]);
            }
        }
    }

    // Load position, scale and rotation data.
    // TODO

    // Create buffer.
    mgAssert(buffer.create(static_cast<uint32_t>(vertices.size()) * sizeof(MgVertex) +
            static_cast<uint32_t>(indices.size()) * sizeof(uint32_t),
            VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, pDevice));

    // Map buffer memory to host.
    uint8_t *data;
    mgAssert(vkMapMemory(pDevice->logical, buffer.memory, 0, VK_WHOLE_SIZE, 0, reinterpret_cast<void **>(&data)));

    // Copy data to the buffer.
    uint32_t offset = 0;
    uint32_t size = static_cast<uint32_t>(vertices.size()) * sizeof(MgVertex);
    memcpy(data + offset, vertices.data(), size);

    offset += size;
    size = static_cast<uint32_t>(indices.size()) * sizeof(uint32_t);
    memcpy(data + offset, indices.data(), size);

    // Unmap memory.
    vkUnmapMemory(pDevice->logical, buffer.memory);

    // Fill the diffuse map info.
    MgImageInfo diffuseMapInfo =
    {
        MG_IMAGE_TYPE_TEXTURE_2D,                   // MgImageType                 type;
        "data/textures/" + name + "_d.png",         // QString                     filePath;
        {0, 0, 1},                                  // VkExtent3D                  extent;
        VK_FORMAT_R8G8B8A8_SRGB,                    // VkFormat                    format;

        VK_NULL_HANDLE,                             // const VkImage               image;
        true,                                       // bool                        createView;
        true                                        // bool                        createSampler;
    };

    // Create the diffuse map.
    mgAssert(diffuseMap.create(pDevice, &diffuseMapInfo));

    // Fill the normal map info.
    MgImageInfo normalMapInfo =
    {
        MG_IMAGE_TYPE_TEXTURE_2D,                   // MgImageType                 type;
        "data/textures/" + name + "_n.png",         // QString                     filePath;
        {0, 0, 1},                                  // VkExtent3D                  extent;
        VK_FORMAT_R8G8B8A8_UNORM,                   // VkFormat                    format;

        VK_NULL_HANDLE,                             // const VkImage               image;
        true,                                       // bool                        createView;
        true                                        // bool                        createSampler;
    };

    // Create the normal map.
    mgAssert(normalMap.create(pDevice, &normalMapInfo));

    return VK_SUCCESS;
}

/**
 * Destroy the entity.
 */
void VkcEntity::destroy()
{
    diffuseMap.destroy();
    normalMap.destroy();
    specularMap.destroy();

    buffer.destroy();
}

/**
 * Register the commands that render the entity.
 */
VkResult VkcEntity::render(VkcContext *pContext, VkCommandBuffer commandBuffer, QMatrix4x4 vpMatrix, float delta)
{
    // Wiggle, wiggle, wiggle.
    position += QVector3D(dir * delta, 0.0f, 0.0f);
    float pos = position.x();

    if(pos > 0.5f)
    {
        dir = -1.0f;
    }
    else if (pos < -0.5f)
    {
        dir = 1.0f;
    }

    // Calculate model matrix.
    QMatrix4x4 modelMatrix;
    modelMatrix.translate(position);
    modelMatrix.rotate(rotation);
    modelMatrix.scale(scale);

    // Calculate MVP matrix.
    QMatrix4x4 mvpMatrix = vpMatrix * modelMatrix;

    mgAssert(pContext->pipeline->bindFloatv(0, mvpMatrix.data(), 16 * sizeof(float)));

    // Bind vertex and index bufffer.
    VkDeviceSize vboOffsets[] = {0};
    uint32_t iboOffset = static_cast<uint32_t>(vertices.size()) * sizeof(MgVertex);

    vkCmdBindVertexBuffers(commandBuffer, 0, 1, &buffer.handle, vboOffsets);
    vkCmdBindIndexBuffer(commandBuffer, buffer.handle, iboOffset, VK_INDEX_TYPE_UINT32);

    // Bind the textures.
    pContext->pipeline->bindImage(10, diffuseMap);
    pContext->pipeline->bindImage(11, normalMap);

    // Bind descriptor sets.
    vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pContext->pipeline->layout, 0,
            1, &pContext->pipeline->descriptorSet, 0, nullptr);

    // Draw entity.
    vkCmdDrawIndexed(commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

    return VK_SUCCESS;
}
