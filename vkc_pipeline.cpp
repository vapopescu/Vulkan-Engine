#include "vkc_pipeline.h"


/**
 * Create the graphics pipeline.
 */
VkcPipeline::VkcPipeline(const VkcSwapchain *swapchain, const VkcDevice *device)
{
    // Fill descriptor set binding info.
    QVector<VkDescriptorSetLayoutBinding> setBindings =
    {

        {
            0,                                          // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType      descriptorType;
            1,                                          // uint32_t              descriptorCount;
            VK_SHADER_STAGE_VERTEX_BIT,                 // VkShaderStageFlags    stageFlags;
            nullptr                                     // const VkSampler*      pImmutableSamplers;
        },

        {
            10,                                         // uint32_t              binding;
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType      descriptorType;
            1,                                          // uint32_t              descriptorCount;
            VK_SHADER_STAGE_FRAGMENT_BIT,               // VkShaderStageFlags    stageFlags;
            nullptr                                     // const VkSampler*      pImmutableSamplers;
        }

    };

    // Fill desctriptor set size info.
    QVector<VkDescriptorPoolSize> poolSizes =
    {
        {
            VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,          // VkDescriptorType    type;
            1                                           // uint32_t            descriptorCount;
        },

        {
            VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,  // VkDescriptorType    type;
            1                                           // uint32_t            descriptorCount;
        }
    };

    // Fill descriptor set layout info.
    VkDescriptorSetLayoutCreateInfo setLayoutInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,    // VkStructureType                        sType;
        nullptr,                                                // const void*                            pNext;
        0,                                                      // VkDescriptorSetLayoutCreateFlags       flags;

        (uint32_t)setBindings.size(),                           // uint32_t                               bindingCount;
        setBindings.data()                                      // const VkDescriptorSetLayoutBinding*    pBindings;
    };

    // Create descriptor set layout.
    vkCreateDescriptorSetLayout(device->logical, &setLayoutInfo, nullptr, &setLayout);

    // Fill descriptor pool info.
    VkDescriptorPoolCreateInfo descriptorPoolInfo =
    {
        VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,          // VkStructureType                sType;
        nullptr,                                                // const void*                    pNext;
        VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,      // VkDescriptorPoolCreateFlags    flags;

        1,                                                      // uint32_t                       maxSets;
        (uint32_t)poolSizes.size(),                             // uint32_t                       poolSizeCount;
        poolSizes.data()                                        // const VkDescriptorPoolSize*    pPoolSizes;
    };

    // Create descriptor pool.
    vkCreateDescriptorPool(device->logical, &descriptorPoolInfo, nullptr, &descriptorPool);

    // Fill descriptor set allocate info.
    VkDescriptorSetAllocateInfo descriptorSetAllocateInfo =
    {

        VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,     // VkStructureType                 sType;
        nullptr,                                            // const void*                     pNext;

        descriptorPool,                                     // VkDescriptorPool                descriptorPool;
        1,                                                  // uint32_t                        descriptorSetCount;
        &setLayout                                          // const VkDescriptorSetLayout*    pSetLayouts;

    };

    // Allocate space for descriptor set.
    vkAllocateDescriptorSets(device->logical, &descriptorSetAllocateInfo, &descriptorSet);

    // Fill pipeline layout info.
    VkPipelineLayoutCreateInfo pipelineLayoutInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,      // VkStructureType                 sType;
        nullptr,                                            // const void*                     pNext;
        0,                                                  // VkPipelineLayoutCreateFlags     flags;

        1,                                                  // uint32_t                        setLayoutCount;
        &setLayout,                                         // const VkDescriptorSetLayout*    pSetLayouts;

        0,                                                  // uint32_t                        pushConstantRangeCount;
        nullptr                                             // const VkPushConstantRange*      pPushConstantRanges;
    };

    // Create pipeline layout.
    vkCreatePipelineLayout(device->logical, &pipelineLayoutInfo, nullptr, &layout);

    // Create shaders.
    createShader(vertShader, "shader.vert.spv", device);
    createShader(fragShader, "shader.frag.spv", device);

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
        0,                                      // uint32_t             binding;
        sizeof(VkVertex),                       // uint32_t             stride;
        VK_VERTEX_INPUT_RATE_VERTEX,            // VkVertexInputRate    inputRate;
    };

    // Fill vertex input attribute description.
    QVector<VkVertexInputAttributeDescription> vertexAttributes =
    {
        {
            0,                                  // uint32_t    location;
            0,                                  // uint32_t    binding;
            VK_FORMAT_R32G32B32_SFLOAT,         // VkFormat    format;
            offsetof(VkVertex, x)               // uint32_t    offset;
        },

        {
            1,                                  // uint32_t    location;
            0,                                  // uint32_t    binding;
            VK_FORMAT_R32G32_SFLOAT,            // VkFormat    format;
            offsetof(VkVertex, u)               // uint32_t    offset;
        },

        {
            2,                                  // uint32_t    location;
            0,                                  // uint32_t    binding;
            VK_FORMAT_R32G32B32_SFLOAT,         // VkFormat    format;
            offsetof(VkVertex, nx)              // uint32_t    offset;
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

        (uint32_t)vertexAttributes.size(),                              // uint32_t                                    vertexAttributeDescriptionCount;
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


    // Fill viewport info.
    VkViewport viewport =
    {
        0.0f,                               // float    x;
        0.0f,                               // float    y;
        (float)swapchain->extent.width,     // float    width;
        (float)swapchain->extent.height,    // float    height;
        0.0f,                               // float    minDepth;
        1.0f,                               // float    maxDepth;
    };

    // Fill scissors info.
    VkRect2D scissors =
    {
        {0, 0},                 // VkOffset2D    offset;
        swapchain->extent       // VkExtent2D    extent;
    };

    // Fill viewport state info.
    VkPipelineViewportStateCreateInfo viewportInfo =
    {
        VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,      // VkStructureType                       sType;
        nullptr,                                                    // const void*                           pNext;
        0,                                                          // VkPipelineViewportStateCreateFlags    flags;

        1,                                                          // uint32_t                              viewportCount;
        &viewport,                                                  // const VkViewport*                     pViewports;

        1,                                                          // uint32_t                              scissorCount;
        &scissors                                                   // const VkRect2D*                       pScissors;
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
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,  // VkStencilOp    passOp;
        VK_STENCIL_OP_INCREMENT_AND_CLAMP,  // VkStencilOp    depthFailOp;

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

        (uint32_t)dynamicStates.count(),                            // uint32_t                             dynamicStateCount;
        dynamicStates.data()                                        // const VkDynamicState*                pDynamicStates;
    };


    // Fill graphics pipeline info.
    VkGraphicsPipelineCreateInfo pipelineInfo =
    {
        VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,            // VkStructureType                                  sType;
        nullptr,                                                    // const void*                                      pNext;
        0,                                                          // VkPipelineCreateFlags                            flags;

        (uint32_t)shaderStages.size(),                              // uint32_t                                         stageCount;
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
    vkCreateGraphicsPipelines(device->logical, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &handle);

    this->logicalDevice = device->logical;
}


/**
 * Destroy the graphics pipeline.
 */
VkcPipeline::~VkcPipeline()
{
    if (logicalDevice != VK_NULL_HANDLE)
    {
        if (descriptorSet != VK_NULL_HANDLE)
            vkFreeDescriptorSets(logicalDevice, descriptorPool, 1, &descriptorSet);

        if (setLayout != VK_NULL_HANDLE)
            vkDestroyDescriptorSetLayout(logicalDevice, setLayout, nullptr);

        if (descriptorPool != VK_NULL_HANDLE)
            vkDestroyDescriptorPool(logicalDevice, descriptorPool, nullptr);

        if (vertShader != VK_NULL_HANDLE)
            vkDestroyShaderModule(logicalDevice, vertShader, nullptr);

        if (fragShader != VK_NULL_HANDLE)
            vkDestroyShaderModule(logicalDevice, fragShader, nullptr);

        if (layout != VK_NULL_HANDLE)
            vkDestroyPipelineLayout(logicalDevice, layout, nullptr);

        if (handle != VK_NULL_HANDLE)
            vkDestroyPipeline(logicalDevice, handle, nullptr);
    }
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
        qDebug() << "ERROR:   [@qDebug]              - Shader \"" << fileName << "\" not found.";
        return VK_ERROR_INITIALIZATION_FAILED;
    }

    shaderFile.open(QIODevice::ReadOnly);

    // Read shader data.
    QByteArray shaderData = shaderFile.readAll();

    // Fill shader module info.
    VkShaderModuleCreateInfo shaderInfo =
    {
        VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,    // VkStructureType              sType;
        nullptr,                                        // const void*                  pNext;
        0,                                              // VkShaderModuleCreateFlags    flags;

        (size_t)shaderData.size(),                      // size_t                       codeSize;
        (const uint32_t*)shaderData.data()              // const uint32_t*              pCode;
    };

    // Create shader module.
    mgAssert(vkCreateShaderModule(device->logical, &shaderInfo, nullptr, &shader));

    return VK_SUCCESS;
}
