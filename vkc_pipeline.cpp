#include "vkc_pipeline.h"


/**
 * Create the graphics pipeline.
 */
VkcPipeline::VkcPipeline(const VkcSwapchain *swapchain, const VkcDevice *device)
{
    pDevice = device;

    // Fill descriptor set binding info.
    QVector<VkDescriptorSetLayoutBinding> descriptorSetBindings =
    {

        {
            0,                                              // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,              // VkDescriptorType      descriptorType;
            1,                                              // uint32_t              descriptorCount;
            VK_SHADER_STAGE_VERTEX_BIT,                     // VkShaderStageFlags    stageFlags;
            nullptr                                         // const VkSampler*      pImmutableSamplers;
        },

        {
            5,                                              // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,              // VkDescriptorType      descriptorType;
            1,                                              // uint32_t              descriptorCount;
            VK_SHADER_STAGE_FRAGMENT_BIT,                   // VkShaderStageFlags    stageFlags;
            nullptr                                         // const VkSampler*      pImmutableSamplers;
        },

        {
            10,                                             // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType      descriptorType;
            1,                                              // uint32_t              descriptorCount;
            VK_SHADER_STAGE_FRAGMENT_BIT,                   // VkShaderStageFlags    stageFlags;
            nullptr                                         // const VkSampler*      pImmutableSamplers;
        },

        {
            11,                                             // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType      descriptorType;
            1,                                              // uint32_t              descriptorCount;
            VK_SHADER_STAGE_FRAGMENT_BIT,                   // VkShaderStageFlags    stageFlags;
            nullptr                                         // const VkSampler*      pImmutableSamplers;
        },

        {
            12,                                             // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,      // VkDescriptorType      descriptorType;
            1,                                              // uint32_t              descriptorCount;
            VK_SHADER_STAGE_FRAGMENT_BIT,                   // VkShaderStageFlags    stageFlags;
            nullptr                                         // const VkSampler*      pImmutableSamplers;
        }

    };

    // Fill descriptor set layout info.
    VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,    // VkStructureType                        sType;
        nullptr,                                                // const void*                            pNext;
        0,                                                      // VkDescriptorSetLayoutCreateFlags       flags;

        static_cast<uint32_t>(descriptorSetBindings.size()),    // uint32_t                               bindingCount;
        descriptorSetBindings.data()                            // const VkDescriptorSetLayoutBinding*    pBindings;
    };

    // Create descriptor set layout.
    vkCreateDescriptorSetLayout(pDevice->logical, &descriptorSetLayoutInfo, nullptr, &descriptorSetLayout);

    // Compute desctriptor set size info.
    uint32_t descriptorCountArray[VK_DESCRIPTOR_TYPE_RANGE_SIZE];
    memset(descriptorCountArray, 0, sizeof(descriptorCountArray));

    for (VkDescriptorSetLayoutBinding binding : descriptorSetBindings)
    {
        descriptorCountArray[binding.descriptorType - VK_DESCRIPTOR_TYPE_BEGIN_RANGE] += binding.descriptorCount;
    }

    // Fill desctriptor set size info.
    QVector<VkDescriptorPoolSize> descriptorPoolSizes = {};

    for (uint32_t i = 0; i < VK_DESCRIPTOR_TYPE_RANGE_SIZE; i++)
    {
        if (descriptorCountArray[i] > 0)
        {
            descriptorPoolSizes.append({VkDescriptorType(i + VK_DESCRIPTOR_TYPE_BEGIN_RANGE), descriptorCountArray[i]});
        }
    }

    // Fill descriptor pool info.
    VkDescriptorPoolCreateInfo descriptorPoolInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,          // VkStructureType                sType;
        nullptr,                                                // const void*                    pNext;
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,      // VkDescriptorPoolCreateFlags    flags;

        1,                                                      // uint32_t                       maxSets;
        static_cast<uint32_t>(descriptorPoolSizes.size()),      // uint32_t                       poolSizeCount;
        descriptorPoolSizes.data()                              // const VkDescriptorPoolSize*    pPoolSizes;
    };

    // Create descriptor pool.
    vkCreateDescriptorPool(pDevice->logical, &descriptorPoolInfo, nullptr, &descriptorPool);

    // Fill descriptor set allocate info.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
    {

        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,         // VkStructureType                 sType;
        nullptr,                                                // const void*                     pNext;

        descriptorPool,                                         // VkDescriptorPool                descriptorPool;
        1,                                                      // uint32_t                        descriptorSetCount;
        &descriptorSetLayout                                    // const VkDescriptorSetLayout*    pSetLayouts;

    };

    // Allocate space for descriptor set.
    vkAllocateDescriptorSets(pDevice->logical, &descriptorSetAllocateInfo, &descriptorSet);

    // Fill pipeline layout info.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,          // VkStructureType                 sType;
        nullptr,                                                // const void*                     pNext;
        0,                                                      // VkPipelineLayoutCreateFlags     flags;

        1,                                                      // uint32_t                        setLayoutCount;
        &descriptorSetLayout,                                   // const VkDescriptorSetLayout*    pSetLayouts;

        0,                                                      // uint32_t                        pushConstantRangeCount;
        nullptr                                                 // const VkPushConstantRange*      pPushConstantRanges;
    };

    // Create pipeline layout.
    vkCreatePipelineLayout(pDevice->logical, &pipelineLayoutInfo, nullptr, &layout);

    // Create shaders.
    createShader(vertShader, "shader.vert.spv", pDevice);
    createShader(fragShader, "shader.frag.spv", pDevice);

    // Fill shader stage info.
    QVector<VkPipelineShaderStageCreateInfo> shaderStages =
    {
        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                     sType;
            nullptr,                                                // const void*                         pNext;
            0,                                                      // VkPipelineShaderStageCreateFlags    flags;

            VK_SHADER_STAGE_VERTEX_BIT,                             // VkShaderStageFlagBits               stage;

            vertShader,                                             // VkShaderModule                      module;
            "main",                                                 // const char*                         pName;
            nullptr                                                 // const VkSpecializationInfo*         pSpecializationInfo;
        },

        {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,    // VkStructureType                     sType;
            nullptr,                                                // const void*                         pNext;
            0,                                                      // VkPipelineShaderStageCreateFlags    flags;

            VK_SHADER_STAGE_FRAGMENT_BIT,                           // VkShaderStageFlagBits               stage;

            fragShader,                                             // VkShaderModule                      module;
            "main",                                                 // const char*                         pName;
            nullptr                                                 // const VkSpecializationInfo*         pSpecializationInfo;
        }
    };

    // Fill vertex input binding description.
    VkVertexInputBindingDescription vertexBinding =
    {
        0,                                                      // uint32_t             binding;
        sizeof(MgVertex),                                       // uint32_t             stride;
        VK_VERTEX_INPUT_RATE_VERTEX,                            // VkVertexInputRate    inputRate;
    };

    // Fill vertex input attribute description.
    QVector<VkVertexInputAttributeDescription> vertexAttributes =
    {
        {
            0,                                                  // uint32_t    location;
            0,                                                  // uint32_t    binding;
            VK_FORMAT_R32G32B32_SFLOAT,                         // VkFormat    format;
            static_cast<uint32_t>(mgOffsetOf(MgVertex, x))      // uint32_t    offset;
        },

        {
            1,                                                  // uint32_t    location;
            0,                                                  // uint32_t    binding;
            VK_FORMAT_R32G32_SFLOAT,                            // VkFormat    format;
            static_cast<uint32_t>(mgOffsetOf(MgVertex, u))      // uint32_t    offset;
        },

        {
            2,                                                  // uint32_t    location;
            0,                                                  // uint32_t    binding;
            VK_FORMAT_R32G32B32_SFLOAT,                         // VkFormat    format;
            static_cast<uint32_t>(mgOffsetOf(MgVertex, nx))     // uint32_t    offset;
        }
    };

    // Fill vertex input state info.
    VkPipelineVertexInputStateCreateInfo vertexInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,      // VkStructureType                             sType;
        nullptr,                                                        // const void*                                 pNext;
        0,                                                              // VkPipelineVertexInputStateCreateFlags       flags;

        1,                                                              // uint32_t                                    vertexBindingDescriptionCount;
        &vertexBinding,                                                 // const VkVertexInputBindingDescription*      pVertexBindingDescriptions;

        static_cast<uint32_t>(vertexAttributes.size()),                 // uint32_t                                    vertexAttributeDescriptionCount;
        vertexAttributes.data()                                         // const VkVertexInputAttributeDescription*    pVertexAttributeDescriptions;
    };


    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,    // VkStructureType                            sType;
        nullptr,                                                        // const void*                                pNext;
        0,                                                              // VkPipelineInputAssemblyStateCreateFlags    flags;

        VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,                            // VkPrimitiveTopology                        topology;
        VK_FALSE,                                                       // VkBool32                                   primitiveRestartEnable;
    };


    /*
     * Space reserved for VkPipelineTessellationStateCreateInfo.
     */


    // Fill viewport state info.
    VkPipelineViewportStateCreateInfo viewportInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,      // VkStructureType                       sType;
        nullptr,                                                    // const void*                           pNext;
        0,                                                          // VkPipelineViewportStateCreateFlags    flags;

        1,                                                          // uint32_t                              viewportCount;
        nullptr,                                                    // const VkViewport*                     pViewports;

        1,                                                          // uint32_t                              scissorCount;
        nullptr                                                     // const VkRect2D*                       pScissors;
    };


    // Fill rasterization state info.
    VkPipelineRasterizationStateCreateInfo rasterisationInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO, // VkStructureType                            sType;
        nullptr,                                                    // const void*                                pNext;
        0,                                                          // VkPipelineRasterizationStateCreateFlags    flags;

        VK_FALSE,                                                   // VkBool32                                   depthClampEnable;
        VK_FALSE,                                                   // VkBool32                                   rasterizerDiscardEnable;
        VK_POLYGON_MODE_FILL,                                       // VkPolygonMode                              polygonMode;
        VK_CULL_MODE_BACK_BIT,                                      // VkCullModeFlags                            cullMode;
        VK_FRONT_FACE_COUNTER_CLOCKWISE,                            // VkFrontFace                                frontFace;

        VK_FALSE,                                                   // VkBool32                                   depthBiasEnable;
        0,                                                          // float                                      depthBiasConstantFactor;
        0,                                                          // float                                      depthBiasClamp;
        0,                                                          // float                                      depthBiasSlopeFactor;

        1,                                                          // float                                      lineWidth;
    };


    // Fill multisample state info.
    VkPipelineMultisampleStateCreateInfo multisampleInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,   // VkStructureType                          sType;
        nullptr,                                                    // const void*                              pNext;
        0,                                                          // VkPipelineMultisampleStateCreateFlags    flags;

        VK_SAMPLE_COUNT_1_BIT,                                      // VkSampleCountFlagBits                    rasterizationSamples;
        VK_FALSE,                                                   // VkBool32                                 sampleShadingEnable;
        0,                                                          // float                                    minSampleShading;

        nullptr,                                                    // const VkSampleMask*                      pSampleMask;
        VK_FALSE,                                                   // VkBool32                                 alphaToCoverageEnable;
        VK_FALSE,                                                   // VkBool32                                 alphaToOneEnable;
    };


    // Fill stencil operation state info.
    VkStencilOpState stencilInfo =
    {
        VK_STENCIL_OP_KEEP,                 // VkStencilOp    failOp;
        VK_STENCIL_OP_REPLACE,              // VkStencilOp    passOp;
        VK_STENCIL_OP_KEEP,                 // VkStencilOp    depthFailOp;

        VK_COMPARE_OP_ALWAYS,               // VkCompareOp    compareOp;
        0,                                  // uint32_t       compareMask;
        0,                                  // uint32_t       writeMask;
        0                                   // uint32_t       reference;
    };

    // Fill depth state info.
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,     // VkStructureType                           sType;
        nullptr,                                                        // const void*                               pNext;
        0,                                                              // VkPipelineDepthStencilStateCreateFlags    flags;

        VK_TRUE,                                                        // VkBool32                                  depthTestEnable;
        VK_TRUE,                                                        // VkBool32                                  depthWriteEnable;
        VK_COMPARE_OP_GREATER_OR_EQUAL,                                 // VkCompareOp                               depthCompareOp;

        VK_FALSE,                                                       // VkBool32                                  depthBoundsTestEnable;
        VK_TRUE,                                                        // VkBool32                                  stencilTestEnable;
        stencilInfo,                                                    // VkStencilOpState                          front;
        stencilInfo,                                                    // VkStencilOpState                          back;
        0.0f,                                                           // float                                     minDepthBounds;
        0.0f                                                            // float                                     maxDepthBounds;
    };

    // Fill color blend attachment state info.
    VkPipelineColorBlendAttachmentState colorBlendState =
    {
        VK_TRUE,                                // VkBool32                 blendEnable;

        VK_BLEND_FACTOR_SRC_ALPHA,              // VkBlendFactor            srcColorBlendFactor;
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // VkBlendFactor            dstColorBlendFactor;
        VK_BLEND_OP_ADD,                        // VkBlendOp                colorBlendOp;

        VK_BLEND_FACTOR_SRC_ALPHA,              // VkBlendFactor            srcAlphaBlendFactor;
        VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,    // VkBlendFactor            dstAlphaBlendFactor;
        VK_BLEND_OP_ADD,                        // VkBlendOp                alphaBlendOp;

        VK_COLOR_COMPONENT_R_BIT |              // VkColorComponentFlags    colorWriteMask;
        VK_COLOR_COMPONENT_G_BIT |
        VK_COLOR_COMPONENT_B_BIT
    };

    // Fill color blend state info.
    VkPipelineColorBlendStateCreateInfo colorBlendInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,   // VkStructureType                               sType;
        nullptr,                                                    // const void*                                   pNext;
        0,                                                          // VkPipelineColorBlendStateCreateFlags          flags;

        VK_FALSE,                                                   // VkBool32                                      logicOpEnable;
        VK_LOGIC_OP_NO_OP,                                          // VkLogicOp                                     logicOp;
        1,                                                          // uint32_t                                      attachmentCount;
        &colorBlendState,                                           // const VkPipelineColorBlendAttachmentState*    pAttachments;
        {1.0f, 1.0f, 1.0f, 1.0f}                                    // float                                         blendConstants[4];
    };


    // Select dynamic states.
    QVector<VkDynamicState> dynamicStates =
    {
        VK_DYNAMIC_STATE_VIEWPORT,
        VK_DYNAMIC_STATE_SCISSOR,
        //VK_DYNAMIC_STATE_STENCIL_REFERENCE
    };

    // Fill dynamic state info.
    VkPipelineDynamicStateCreateInfo dynamicStateInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,       // VkStructureType                      sType;
        nullptr,                                                    // const void*                          pNext;
        0,                                                          // VkPipelineDynamicStateCreateFlags    flags;

        static_cast<uint32_t>(dynamicStates.count()),               // uint32_t                             dynamicStateCount;
        dynamicStates.data()                                        // const VkDynamicState*                pDynamicStates;
    };


    // Fill graphics pipeline info.
    VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                  sType;
        nullptr,                                                    // const void*                                      pNext;
        0,                                                          // VkPipelineCreateFlags                            flags;

        static_cast<uint32_t>(shaderStages.size()),                 // uint32_t                                         stageCount;
        shaderStages.data(),                                        // const VkPipelineShaderStageCreateInfo*           pStages;

        &vertexInfo,                                                // const VkPipelineVertexInputStateCreateInfo*      pVertexInputState;
        &inputAssemblyInfo,                                         // const VkPipelineInputAssemblyStateCreateInfo*    pInputAssemblyState;
        nullptr,                                                    // const VkPipelineTessellationStateCreateInfo*     pTessellationState;
        &viewportInfo,                                              // const VkPipelineViewportStateCreateInfo*         pViewportState;
        &rasterisationInfo,                                         // const VkPipelineRasterizationStateCreateInfo*    pRasterizationState;
        &multisampleInfo,                                           // const VkPipelineMultisampleStateCreateInfo*      pMultisampleState;
        &depthStencilInfo,                                          // const VkPipelineDepthStencilStateCreateInfo*     pDepthStencilState;
        &colorBlendInfo,                                            // const VkPipelineColorBlendStateCreateInfo*       pColorBlendState;
        &dynamicStateInfo,                                          // const VkPipelineDynamicStateCreateInfo*          pDynamicState;

        layout,                                                     // VkPipelineLayout                                 layout;
        swapchain->renderPass,                                      // VkRenderPass                                     renderPass;
        0,                                                          // uint32_t                                         subpass;
        VK_NULL_HANDLE,                                             // VkPipeline                                       basePipelineHandle;
        0                                                           // int32_t                                          basePipelineIndex;
    };

    // Create graphics pipeline.
    vkCreateGraphicsPipelines(pDevice->logical, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle);
}


/**
 * Destroy the graphics pipeline.
 */
VkcPipeline::~VkcPipeline()
{
    if (pDevice != nullptr)
    {
        QList<MgBuffer> buffers = bindBuffers.values();
        while (buffers.size() > 0)
        {
            buffers.first().destroy();
            buffers.removeFirst();
        }

        if (descriptorSet != VK_NULL_HANDLE)
        {
            vkFreeDescriptorSets(pDevice->logical, descriptorPool, 1, &descriptorSet);
        }

        if (descriptorSetLayout != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorSetLayout(pDevice->logical, descriptorSetLayout, nullptr);
        }

        if (descriptorPool != VK_NULL_HANDLE)
        {
            vkDestroyDescriptorPool(pDevice->logical, descriptorPool, nullptr);
        }

        if (vertShader != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(pDevice->logical, vertShader, nullptr);
        }

        if (fragShader != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(pDevice->logical, fragShader, nullptr);
        }

        if (layout != VK_NULL_HANDLE)
        {
            vkDestroyPipelineLayout(pDevice->logical, layout, nullptr);
        }

        if (handle != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(pDevice->logical, handle, nullptr);
        }
    }
}

VkResult VkcPipeline::bindFloatv(uint32_t binding, float values[], uint32_t size, uint32_t maxSize)
{
    // If max size is not specified, set it to minimum.
    if (maxSize == 0)
    {
        maxSize = size;
    }

    // Assume the descriptor set does not need updating
    VkBool32 updateDescriptorSet = VK_FALSE;

    // If the buffer does not exist, create it.
    MgBuffer buffer = bindBuffers.take(binding);
    if (buffer.handle == VK_NULL_HANDLE)
    {
        mgAssert(buffer.create(maxSize * sizeof(float), VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, pDevice));
        updateDescriptorSet = VK_TRUE;
    }

    // Map buffer memory to host.
    void *data = nullptr;
    mgAssert(vkMapMemory(pDevice->logical, buffer.memory, 0, size * sizeof(float), 0, &data));

    // Copy data to the buffer.
    memcpy(data, values, size * sizeof(float));

    // Unmap memory.
    vkUnmapMemory(pDevice->logical, buffer.memory);

    // Update descriptor set if necessary.
    if (updateDescriptorSet)
    {
        // Fill buffer info.
        VkDescriptorBufferInfo bufferInfo =
        {
            buffer.handle,              // VkBuffer        buffer;
            0,                          // VkDeviceSize    offset;
            maxSize * sizeof(float)     // VkDeviceSize    range;
        };

        // Fill write descriptor set info.
        VkWriteDescriptorSet writeDescriptorSet =
        {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // VkStructureType                  sType;
            nullptr,                                    // const void*                      pNext;

            descriptorSet,                              // VkDescriptorSet                  dstSet;
            binding,                                    // uint32_t                         dstBinding;
            0,                                          // uint32_t                         dstArrayElement;
            1,                                          // uint32_t                         descriptorCount;
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType                 descriptorType;

            nullptr,                                    // const VkDescriptorImageInfo*     pImageInfo;
            &bufferInfo,                                // const VkDescriptorBufferInfo*    pBufferInfo;
            nullptr                                     // const VkBufferView*              pTexelBufferView;
        };

        // Update descriptor set.
        vkUpdateDescriptorSets(pDevice->logical, 1, &writeDescriptorSet, 0, nullptr);
    }

    // Put the buffer back into the hash table.
    bindBuffers.insert(binding, buffer);

    return VK_SUCCESS;
}

void VkcPipeline::bindImage(uint32_t binding, MgImage image)
{
    // Fill texture info.
    VkDescriptorImageInfo descriptorImageInfo =
    {
        image.sampler,            // VkSampler        sampler;
        image.view,               // VkImageView      imageView;
        image.layout              // VkImageLayout    imageLayout;
    };

    // Fill write descriptor set info.
    VkWriteDescriptorSet writeDescriptorSet =
    {
        VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,     // VkStructureType                  sType;
        nullptr,                                    // const void*                      pNext;

        descriptorSet,                              // VkDescriptorSet                  dstSet;
        binding,                                    // uint32_t                         dstBinding;
        0,                                          // uint32_t                         dstArrayElement;
        1,                                          // uint32_t                         descriptorCount;
        VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType                 descriptorType;

        &descriptorImageInfo,                       // const VkDescriptorImageInfo*     pImageInfo;
        nullptr,                                    // const VkDescriptorBufferInfo*    pBufferInfo;
        nullptr                                     // const VkBufferView*              pTexelBufferView;
    };

    // Update descriptor set.
    vkUpdateDescriptorSets(pDevice->logical, 1, &writeDescriptorSet, 0, nullptr);
}


/**
 * Load shader module from file.
 */
VkResult VkcPipeline::createShader(VkShaderModule &shader, QString fileName, const VkcDevice *device)
{
    // Open shader file in binary.
    QFile shaderFile("data/shaders/" + fileName);

    if(!shaderFile.exists())
    {
        qDebug(QString("ERROR:   [@qDebug]").leftJustified(32, ' ')
               .append("- Shader \"%1\" not found").arg(fileName).toStdString().c_str());
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    shaderFile.open(QIODevice::ReadOnly);

    // Read shader data.
    QByteArray shaderData = shaderFile.readAll();

    // Fill shader module info.
    VkShaderModuleCreateInfo shaderInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,        // VkStructureType              sType;
        nullptr,                                            // const void*                  pNext;
        0,                                                  // VkShaderModuleCreateFlags    flags;

        static_cast<size_t>(shaderData.size()),             // size_t                       codeSize;
        reinterpret_cast<uint32_t *>(shaderData.data())     // const uint32_t*              pCode;
    };

    // Create shader module.
    mgAssert(vkCreateShaderModule(device->logical, &shaderInfo, nullptr, &shader));

    return VK_SUCCESS;
}
