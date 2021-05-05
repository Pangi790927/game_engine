#ifndef PGE_PIPELINE_H
#define PGE_PIPELINE_H

#include <vector>
#include <fstream>

#include "utils.h"
#include "game_engine_st.h"
#include "pge_window.h"
#include "magic_enum.h"

// this must be rebuilt

namespace pge
{

// general settings regardding pipeline creation
struct base_pipeline_info_t {
    bool use_defaults = false;
};

struct vert_input_info_t {
    bool use_defaults = false;
    std::vector<VkVertexInputBindingDescription> binding_desc;
    std::vector<VkVertexInputAttributeDescription> attr_desc;
};

// VK_PRIMITIVE_TOPOLOGY_POINT_LIST
// VK_PRIMITIVE_TOPOLOGY_LINE_LIST
// VK_PRIMITIVE_TOPOLOGY_LINE_STRIP
// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
// VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP
struct topology_info_t {
    bool use_defaults = false;
    VkPrimitiveTopology topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    bool restart_enable = false;
};

struct viewport_info_t {
    bool use_defaults = false;
    VkViewport viewport;
    VkRect2D scissor;
};

enum ShaderLoadType {
    SHADER_LOAD_SRC,
    SHADER_LOAD_PATH,
    SHADER_LOAD_BYTECODE,
    SHADER_LOAD_BYTECODE_PATH,
};
struct shader_info_t {
    ShaderLoadType load_type = SHADER_LOAD_PATH;
    std::string name;
    std::string code;
    std::string path;
    std::vector<uint32_t> bytecode;
    bool optimize = true;
};
struct vert_shader_info_t {
    // does not have default values
    shader_info_t info;
};

struct frag_shader_info_t {
    // does not have default values
    shader_info_t info;
};


// VK_POLYGON_MODE_FILL
// VK_POLYGON_MODE_LINE
// VK_POLYGON_MODE_POINT
// VK_CULL_MODE_NONE
// VK_CULL_MODE_FRONT_BIT
// VK_CULL_MODE_BACK_BIT
// VK_CULL_MODE_FRONT_AND_BACK
struct rasterizer_info_t {
    bool use_defaults = false;
    bool depth_clamp = false;
    bool raster_discard = false;
    VkPolygonMode poly_mode = VK_POLYGON_MODE_FILL;
    VkCullModeFlags cull_face = VK_CULL_MODE_NONE;
    VkFrontFace front_face = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    float line_width = 1.0f;
};

struct color_blending_info_t {
    bool use_defaults = false;
    bool enabled = false;
};

struct multisample_info_t {
    bool use_defaults = false;
    VkSampleCountFlagBits samples = VK_SAMPLE_COUNT_1_BIT;
    bool enable_sample_shading = false;
    float min_sample_shading = 1.0f;
};

struct layouts_info_t {
    bool use_defaults = false;
    std::vector<VkDescriptorSetLayout> desc_layout;
};

struct render_subpass_info_t {
    // TODO
};

/* should know about:
        - attachments
        - attachemts refs
        - subpass
        - dependencyes
*/
struct RenderPass {
};

struct DrawPipeline {
    // user provided structs
    base_pipeline_info_t _base_pipeline_info;
    vert_input_info_t _vert_info;
    topology_info_t _topology_info;
    viewport_info_t _viewport_info;
    vert_shader_info_t _vert_shader_info;
    rasterizer_info_t _raster_info;
    multisample_info_t _msample_info;
    color_blending_info_t _blend_info;
    frag_shader_info_t _frag_shader_info;
    layouts_info_t _layouts_info;
    std::vector<render_subpass_info_t> render_subpasses;

    /* you can bind pipeline creator to a pipeline to create another pipeline
    from it */
    struct PipelineCreator {
        enum PipelineState {
            STATE_NONE,
            STATE_CONSTRUCTOR,
            STATE_INIT_START,
            STATE_VERTEX_INPUT,
            STATE_INPUT_ASSEMBLY,
            STATE_VIEWPORT,
            STATE_VERTEX_SHADER,
            STATE_RASTERIZER,
            STATE_MULTISAMPLER,
            STATE_FRAGMENT_SHADER,
            STATE_COLOR_BLENDING,
            STATE_LAYOUTS,
            STATE_RENDER_SUBPASS,
            STATE_INIT_DONE,
        };
        PipelineState init_state = STATE_NONE;

        struct Scope {
            PipelineCreator &scope;

            Scope (PipelineCreator &scope) : scope(scope) {
                scope.init_state = STATE_CONSTRUCTOR;
            }
            ~Scope() {
                scope.check_init_done();
            }
            PipelineCreator * operator -> () const {
                return &scope;
            } 
        };

        DrawPipeline& pipeline;
        PipelineCreator(DrawPipeline& pipeline) : pipeline(pipeline) {}

        PipelineCreator *start_pipeline(
                base_pipeline_info_t pipeline_info = { .use_defaults = true })
        {
            state_transition({STATE_CONSTRUCTOR}, STATE_INIT_START);
            if (pipeline_info.use_defaults)
                pipeline_info = base_pipeline_info_t{};
            pipeline._base_pipeline_info = pipeline_info;
            return this;
        }

        PipelineCreator *add_vertex_input(
                vert_input_info_t vert_info = { .use_defaults = true})
        {
            state_transition({STATE_INIT_START}, STATE_VERTEX_INPUT);
            if (vert_info.use_defaults)
                vert_info = vert_input_info_t{};
            pipeline._vert_info = vert_info;
            return this;
        }

        PipelineCreator *add_input_assembly(
                topology_info_t topology = { .use_defaults = true})
        {
            state_transition({STATE_VERTEX_INPUT}, STATE_INPUT_ASSEMBLY);
            if (topology.use_defaults)
                topology = topology_info_t{};
            pipeline._topology_info = topology;
            return this;
        }

        PipelineCreator *add_viewport(
                viewport_info_t vp_info = { .use_defaults = true })
        {
            state_transition({STATE_INPUT_ASSEMBLY}, STATE_VIEWPORT);
            if (vp_info.use_defaults)
                vp_info = viewport_info_t{
                    .viewport = {
                        .x = 0.0f,
                        .y = 0.0f,
                        .width = (float)pipeline.window->phydev.extent.width,
                        .height = (float)pipeline.window->phydev.extent.height,
                        .minDepth = 0.0f,
                        .maxDepth = 1.0f,
                    },
                    .scissor = {
                        .offset = {0, 0},
                        .extent = pipeline.window->phydev.extent,
                    }
                };
            pipeline._viewport_info = vp_info;
            return this;
        }

        PipelineCreator *add_vertex_shader(vert_shader_info_t shader_info) {
            state_transition({STATE_VIEWPORT}, STATE_VERTEX_SHADER);
            pipeline._vert_shader_info = shader_info;
            load_shader(pipeline._vert_shader_info.info, VERTEX_SHADER);
            return this;
        }

        PipelineCreator *add_rasterizer(
                rasterizer_info_t raster_info = { .use_defaults = true })
        {
            state_transition({STATE_VERTEX_SHADER}, STATE_RASTERIZER);
            if (raster_info.use_defaults)
                raster_info = rasterizer_info_t{};
            pipeline._raster_info = raster_info;
            return this;
        }

        PipelineCreator *add_multisampler(
                multisample_info_t multi_info = { .use_defaults = true })
        {
            state_transition({STATE_RASTERIZER}, STATE_MULTISAMPLER);
            if (multi_info.use_defaults)
                multi_info = multisample_info_t{};
            pipeline._msample_info = multi_info;
            return this;
        }

        PipelineCreator *add_fragment_shader(frag_shader_info_t shader_info) {
            state_transition({STATE_MULTISAMPLER}, STATE_FRAGMENT_SHADER);
            pipeline._frag_shader_info = shader_info;
            load_shader(pipeline._frag_shader_info.info, FRAGMENT_SHADER);
            return this;
        }

        PipelineCreator *add_color_blending(
                color_blending_info_t blending_info = { .use_defaults = true })
        {
            state_transition({STATE_FRAGMENT_SHADER}, STATE_COLOR_BLENDING);
            if (blending_info.use_defaults)
                blending_info = color_blending_info_t{};
            pipeline._blend_info = blending_info;
            return this;
        }

        PipelineCreator *add_layouts(
                layouts_info_t layouts_info = { .use_defaults = true })
        {
            state_transition({STATE_COLOR_BLENDING}, STATE_LAYOUTS);
            if (layouts_info.use_defaults)
                layouts_info = layouts_info_t{};
            pipeline._layouts_info = layouts_info;
            return this;
        }

        PipelineCreator *add_render_subpass(render_subpass_info_t render_pass) {
            state_transition({STATE_LAYOUTS, STATE_RENDER_SUBPASS},
                    STATE_RENDER_SUBPASS);
            /* render subpass is a bit special, as we can have more of them
            and we collect them till the end of the pipeline */
            // TODO: 
            return this;
        }

        PipelineCreator *end_pipeline(bool create_it = true) {
            state_transition({STATE_RENDER_SUBPASS}, STATE_INIT_DONE);
            if (create_it)
                pipeline.create_pipeline();
            return this;
        }

        void load_shader(shader_info_t &info, int type) {
            switch (info.load_type) {
                case SHADER_LOAD_PATH:
                    info.bytecode = compile_shader_path(
                            info.path, type, info.optimize);
                    break;
                case SHADER_LOAD_SRC:
                    info.bytecode = compile_shader_src(info.name,
                            info.path, type, info.optimize);
                    break;
                case SHADER_LOAD_BYTECODE_PATH: {
                    std::ifstream input(info.path, std::ios::binary);

                    std::vector<char> bytes(
                         (std::istreambuf_iterator<char>(input)),
                         (std::istreambuf_iterator<char>()));

                    input.close();
                    info.bytecode.resize(
                            bytes.size() / 4 + !!(bytes.size() % 4));
                    memcpy(info.bytecode.data(), bytes.data(), bytes.size());
                } break;
                case SHADER_LOAD_BYTECODE:
                    // nothing to do
                    break;
                default: EXCEPTION("Unknown shader load type");
            }
        }

        void state_transition(std::initializer_list<PipelineState> prev_states,
                PipelineState new_state)
        {
            bool valid_state = false;
            std::string state_names = "{";
            bool had_prev = false;
            for (auto &&state : prev_states) {
                state_names += (had_prev ? ", " : "") + state_name(state);
                had_prev = true;
                if (init_state == state)
                    valid_state = true;
            }
            state_names += "}";

            if (!valid_state)
                EXCEPTION("Invalid previous state, you must be in one of those "
                        "states: %s before entering %s state, but you are in "
                        "state: %s", state_names, state_name(new_state),
                        state_name(init_state));
            init_state = new_state;
        }

        std::string state_name(PipelineState state) {
            constexpr auto names = magic_enum::enum_names<PipelineState>();
            for (int i = 0; auto &&name : names) {
                if (i++ == state)
                    return std::string(name);
            }
            return "UNKNOWN_STATE";
        }

        void check_init_done() {
            if (init_state != STATE_INIT_DONE)
                EXCEPTION("Pipeline initialization not finished, you are in "
                        "state %s, but should be in %s",
                        state_name(init_state), state_name(STATE_INIT_DONE));
        }
    };

    struct PipelineDataScope{
        PgeWindow *window = nullptr;
        VkRenderPass render_pass = nullptr;
        VkPipeline graphic_pipeline = nullptr;
        VkPipelineLayout pipeline_layout = nullptr;

        PipelineDataScope(PgeWindow *window) : window(window) {}

        ~PipelineDataScope() {
            if (graphic_pipeline)
                vkDestroyPipeline(window->d->device, graphic_pipeline, nullptr);
            if (render_pass)
                vkDestroyRenderPass(window->d->device, render_pass, nullptr);
            if (pipeline_layout)
                vkDestroyPipelineLayout(window->d->device, pipeline_layout,
                        nullptr);
        }
    };

    std::unique_ptr<PipelineDataScope> p = nullptr;

    PgeWindow *window = nullptr;
    PipelineCreator initer;

    DrawPipeline(PgeWindow *window) : window(window), initer(*this) {
        p = std::make_unique<PipelineDataScope>(window);
    }

    PipelineCreator::Scope begin_pipeline() {
        /* note: destructor of scope is not called here */
        PipelineCreator::Scope scope(initer);
        scope->start_pipeline();
        return scope;
    }

    void create_pipeline() {
        /* Input bindings */
        VkPipelineVertexInputStateCreateInfo vert_input_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
            .vertexBindingDescriptionCount =
                (uint32_t)_vert_info.binding_desc.size(),
            .pVertexBindingDescriptions = _vert_info.binding_desc.data(),
            .vertexAttributeDescriptionCount = 
                (uint32_t)_vert_info.attr_desc.size(),
            .pVertexAttributeDescriptions = _vert_info.attr_desc.data(),
        };

        /* Topology */
        VkPipelineInputAssemblyStateCreateInfo topol_cfg = {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
            .topology = _topology_info.topology,
            .primitiveRestartEnable = _topology_info.restart_enable,
        };

        /* Viewport */
        VkViewport viewport_cfg = _viewport_info.viewport;
        VkRect2D scissor_cfg = _viewport_info.scissor;
        VkPipelineViewportStateCreateInfo viewport_state_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
            .viewportCount = 1,
            .pViewports = &viewport_cfg,
            .scissorCount = 1,
            .pScissors = &scissor_cfg,
        };

        /* Vertex shader stage */
        VkShaderModuleCreateInfo sh_vert_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = _vert_shader_info.info.bytecode.size() * sizeof(uint32_t),
            .pCode = _vert_shader_info.info.bytecode.data(),
        };
        VkShaderModule vert_module;
        if (vkCreateShaderModule(window->d->device, &sh_vert_info, nullptr,
                &vert_module) != VK_SUCCESS)
            EXCEPTION("failed to create shader module!");
        VkPipelineShaderStageCreateInfo vertex_stage{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert_module,
            .pName = "main",
        };

        /* Fragment shader stage */
        VkShaderModuleCreateInfo sh_frag_info{
            .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = _frag_shader_info.info.bytecode.size() * sizeof(uint32_t),
            .pCode = _frag_shader_info.info.bytecode.data(),
        };
        VkShaderModule frag_module;
        if (vkCreateShaderModule(window->d->device, &sh_frag_info, nullptr,
                &frag_module) != VK_SUCCESS)
            EXCEPTION("failed to create shader module!");
        VkPipelineShaderStageCreateInfo frag_stage{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag_module,
            .pName = "main",
        };

        VkPipelineShaderStageCreateInfo shader_stages[] = {
            vertex_stage, frag_stage
        };

        /* Rasterizer */
        VkPipelineRasterizationStateCreateInfo rasterizer_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .depthClampEnable = _raster_info.depth_clamp,
            .rasterizerDiscardEnable = _raster_info.raster_discard,
            .polygonMode = _raster_info.poly_mode,
            .cullMode = _raster_info.cull_face,
            .frontFace = _raster_info.front_face,
            .depthBiasEnable = VK_FALSE,
            .depthBiasConstantFactor = 0.0f, // Optional
            .depthBiasClamp = 0.0f, // Optional
            .depthBiasSlopeFactor = 0.0f, // Optional
            .lineWidth = _raster_info.line_width,
        };

        // Multisampler
        VkPipelineMultisampleStateCreateInfo multisampler_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
            .rasterizationSamples = _msample_info.samples,
            .sampleShadingEnable = _msample_info.enable_sample_shading,
            .minSampleShading = _msample_info.min_sample_shading,
            .pSampleMask = nullptr, // Optiona,
            .alphaToCoverageEnable = VK_FALSE, // Optiona,
            .alphaToOneEnable = VK_FALSE, // Optiona,
        };

        // Blending
        VkPipelineColorBlendAttachmentState color_blend_attachment{
            .blendEnable = _blend_info.enabled,
            .srcColorBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
            .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
            .colorBlendOp = VK_BLEND_OP_ADD, // Optional
            .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE, // Optional
            .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO, // Optional
            .alphaBlendOp = VK_BLEND_OP_ADD, // Optional
            .colorWriteMask =
                    VK_COLOR_COMPONENT_R_BIT |
                    VK_COLOR_COMPONENT_G_BIT |
                    VK_COLOR_COMPONENT_B_BIT |
                    VK_COLOR_COMPONENT_A_BIT,
        };
        VkPipelineColorBlendStateCreateInfo blending_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
            .logicOpEnable = _blend_info.enabled,
            .logicOp = VK_LOGIC_OP_COPY, // Optional
            .attachmentCount = 1,
            .pAttachments = &color_blend_attachment,
            .blendConstants = { 0.0f, 0.0f, 0.0f, 0.0f },
        };

        // Layouts
        VkPipelineLayoutCreateInfo pipeline_layout_info_cfg{
            .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
            .setLayoutCount = (uint32_t)_layouts_info.desc_layout.size(),
            .pSetLayouts = _layouts_info.desc_layout.data(),
            .pushConstantRangeCount = 0,
            .pPushConstantRanges = nullptr,
        };

        if (vkCreatePipelineLayout(window->d->device, &pipeline_layout_info_cfg,
                nullptr, &p->pipeline_layout) != VK_SUCCESS)
            EXCEPTION("failed to create pipeline layout!");

        // TODO: Make render pass non-fixed

        VkAttachmentDescription color_attacment {
            .format = window->phydev.surf_fmt.format,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
            .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
            .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
            .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        };

        VkAttachmentReference color_attacment_ref{
            .attachment = 0,
            .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        };

        VkSubpassDescription subpass{
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1,
            .pColorAttachments = &color_attacment_ref,
        };

        VkSubpassDependency dependency{
            .srcSubpass = VK_SUBPASS_EXTERNAL,
            .dstSubpass = 0,
            .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
        };

        VkRenderPassCreateInfo render_pass_info{
            .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
            .attachmentCount = 1,
            .pAttachments = &color_attacment,
            .subpassCount = 1,
            .pSubpasses = &subpass,
            .dependencyCount = 1,
            .pDependencies = &dependency,
        };

        if (vkCreateRenderPass(window->d->device, &render_pass_info, nullptr,
                &p->render_pass) != VK_SUCCESS)
        {
            EXCEPTION("failed to create render pass!");
        }

        VkGraphicsPipelineCreateInfo pipeline_cfg{
            .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .stageCount = 2,
            .pStages = shader_stages,
            .pVertexInputState = &vert_input_cfg,
            .pInputAssemblyState = &topol_cfg,
            .pViewportState = &viewport_state_cfg,
            .pRasterizationState = &rasterizer_cfg,
            .pMultisampleState = &multisampler_cfg,
            .pDepthStencilState = nullptr,
            .pColorBlendState = &blending_cfg,
            .pDynamicState = nullptr,
            .layout = p->pipeline_layout,
            .renderPass = p->render_pass,
            .subpass = 0,
            .basePipelineHandle = VK_NULL_HANDLE, // Optional
            .basePipelineIndex = -1, // Optional
        };

        if (vkCreateGraphicsPipelines(window->d->device, VK_NULL_HANDLE, 1,
                &pipeline_cfg, nullptr, &p->graphic_pipeline) != VK_SUCCESS)
            EXCEPTION("failed to create graphics pipeline!");

        vkDestroyShaderModule(window->d->device, frag_module, nullptr);
        vkDestroyShaderModule(window->d->device, vert_module, nullptr);
    }
};
}

#endif
