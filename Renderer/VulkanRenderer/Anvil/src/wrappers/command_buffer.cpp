//
// Copyright (c) 2017-2018 Advanced Micro Devices, Inc. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//

#include "misc/callbacks.h"
#include "misc/debug.h"
#include "misc/descriptor_set_create_info.h"
#include "misc/struct_chainer.h"
#include "wrappers/buffer.h"
#include "wrappers/buffer_view.h"
#include "wrappers/command_buffer.h"
#include "wrappers/command_pool.h"
#include "wrappers/compute_pipeline_manager.h"
#include "wrappers/descriptor_set.h"
#include "wrappers/descriptor_set_layout.h"
#include "wrappers/device.h"
#include "wrappers/event.h"
#include "wrappers/framebuffer.h"
#include "wrappers/graphics_pipeline_manager.h"
#include "wrappers/image.h"
#include "wrappers/image_view.h"
#include "wrappers/instance.h"
#include "wrappers/physical_device.h"
#include "wrappers/pipeline_layout.h"
#include "wrappers/query_pool.h"
#include "wrappers/render_pass.h"


/* Command stashing should be enabled by default for builds that care. */
bool Anvil::CommandBufferBase::m_command_stashing_disabled = false;


/** Please see header for specification */
Anvil::CommandBufferBase::BeginQueryCommand::BeginQueryCommand(Anvil::QueryPool*   in_query_pool_ptr,
                                                               Anvil::QueryIndex   in_entry,
                                                               VkQueryControlFlags in_flags)
    :Command(COMMAND_TYPE_BEGIN_QUERY)
{
    entry          = in_entry;
    flags          = in_flags;
    query_pool_ptr = in_query_pool_ptr;
}

/** Please see header for specification */
Anvil::BeginRenderPassCommand::BeginRenderPassCommand(uint32_t            in_n_clear_values,
                                                      const VkClearValue* in_clear_value_ptrs,
                                                      Anvil::Framebuffer* in_fbo_ptr,
                                                      const VkRect2D&     in_render_area,
                                                      Anvil::RenderPass*  in_render_pass_ptr,
                                                      VkSubpassContents   in_contents)
    :Command(COMMAND_TYPE_BEGIN_RENDER_PASS)
{
    contents        = in_contents;
    fbo_ptr         = in_fbo_ptr;
    render_area     = in_render_area;
    render_pass_ptr = in_render_pass_ptr;

    for (uint32_t n_clear_value = 0;
                  n_clear_value < in_n_clear_values;
                ++n_clear_value)
    {
        clear_values.push_back(in_clear_value_ptrs[n_clear_value]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindDescriptorSetsCommand::BindDescriptorSetsCommand(VkPipelineBindPoint                in_pipeline_bind_point,
                                                                               Anvil::PipelineLayout*             in_layout_ptr,
                                                                               uint32_t                           in_first_set,
                                                                               uint32_t                           in_set_count,
                                                                               const Anvil::DescriptorSet* const* in_descriptor_set_ptrs,
                                                                               uint32_t                           in_dynamic_offset_count,
                                                                               const uint32_t*                    in_dynamic_offset_ptrs)
    :Command(COMMAND_TYPE_BIND_DESCRIPTOR_SETS)
{
    first_set           = in_first_set;
    layout_ptr          = in_layout_ptr;
    pipeline_bind_point = in_pipeline_bind_point;

    for (uint32_t n_set = 0;
                  n_set < in_set_count;
                ++n_set)
    {
        descriptor_sets.push_back(in_descriptor_set_ptrs[n_set]);
    }

    for (uint32_t n_dynamic_offset = 0;
                  n_dynamic_offset < in_dynamic_offset_count;
                ++n_dynamic_offset)
    {
        dynamic_offsets.push_back(in_dynamic_offset_ptrs[n_dynamic_offset]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindIndexBufferCommand::BindIndexBufferCommand(Anvil::Buffer* in_buffer_ptr,
                                                                         VkDeviceSize   in_offset,
                                                                         VkIndexType    in_index_type)
    :Command(COMMAND_TYPE_BIND_INDEX_BUFFER)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    index_type = in_index_type;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindPipelineCommand::BindPipelineCommand(VkPipelineBindPoint in_pipeline_bind_point,
                                                                   Anvil::PipelineID   in_pipeline_id)
    :Command(COMMAND_TYPE_BIND_PIPELINE)
{
    pipeline_bind_point = in_pipeline_bind_point;
    pipeline_id         = in_pipeline_id;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommand::BindVertexBuffersCommand(uint32_t            in_start_binding,
                                                                             uint32_t            in_binding_count,
                                                                             Anvil::Buffer**     in_buffer_ptrs,
                                                                             const VkDeviceSize* in_offset_ptrs)
    :Command(COMMAND_TYPE_BIND_VERTEX_BUFFER)
{
    start_binding = in_start_binding;
    
    for (uint32_t n_binding = 0;
                  n_binding < in_binding_count;
                ++n_binding)
    {
        bindings.push_back(BindVertexBuffersCommandBinding(in_buffer_ptrs[n_binding],
                                                           in_offset_ptrs[n_binding]) );
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommandBinding::BindVertexBuffersCommandBinding(const BindVertexBuffersCommandBinding& in)
{
    buffer     = in.buffer;
    buffer_ptr = in.buffer_ptr;
    offset     = in.offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BindVertexBuffersCommandBinding::BindVertexBuffersCommandBinding(Anvil::Buffer* in_buffer_ptr,
                                                                                           VkDeviceSize   in_offset)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::BlitImageCommand::BlitImageCommand(Anvil::Image*      in_src_image_ptr,
                                                             VkImageLayout      in_src_image_layout,
                                                             Anvil::Image*      in_dst_image_ptr,
                                                             VkImageLayout      in_dst_image_layout,
                                                             uint32_t           in_region_count,
                                                             const VkImageBlit* in_region_ptrs,
                                                             VkFilter           in_filter)
    :Command(COMMAND_TYPE_BLIT_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    filter           = in_filter;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearAttachmentsCommand::ClearAttachmentsCommand(uint32_t                 in_n_attachments,
                                                                           const VkClearAttachment* in_attachments,
                                                                           uint32_t                 in_n_rects,
                                                                           const VkClearRect*       in_rect_ptrs)
    :Command(COMMAND_TYPE_CLEAR_ATTACHMENTS)
{
    for (uint32_t n_attachment = 0;
                  n_attachment < in_n_attachments;
                ++n_attachment)
    {
        attachments.push_back(ClearAttachmentsCommandAttachment(in_attachments[n_attachment].aspectMask,
                                                                in_attachments[n_attachment].clearValue,
                                                                in_attachments[n_attachment].colorAttachment) );
    }

    for (uint32_t n_rect = 0;
                  n_rect < in_n_rects;
                ++n_rect)
    {
        rects.push_back(in_rect_ptrs[n_rect]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearColorImageCommand::ClearColorImageCommand(Anvil::Image*                  in_image_ptr,
                                                                         VkImageLayout                  in_image_layout,
                                                                         const VkClearColorValue*       in_color_ptr,
                                                                         uint32_t                       in_range_count,
                                                                         const VkImageSubresourceRange* in_range_ptrs)
    :Command(COMMAND_TYPE_CLEAR_COLOR_IMAGE)
{
    color        = *in_color_ptr;
    image        = in_image_ptr->get_image();
    image_layout = in_image_layout;
    image_ptr    = in_image_ptr;

    for (uint32_t n_range = 0;
                  n_range < in_range_count;
                ++n_range)
    {
        ranges.push_back(in_range_ptrs[n_range]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::ClearDepthStencilImageCommand::ClearDepthStencilImageCommand(Anvil::Image*                   in_image_ptr,
                                                                                       VkImageLayout                   in_image_layout,
                                                                                       const VkClearDepthStencilValue* in_depth_stencil_ptr,
                                                                                       uint32_t                        in_range_count,
                                                                                       const VkImageSubresourceRange*  in_range_ptrs)
    :Command(COMMAND_TYPE_CLEAR_DEPTH_STENCIL_IMAGE)
{
    depth_stencil = *in_depth_stencil_ptr;
    image         =  in_image_ptr->get_image();
    image_layout  =  in_image_layout;
    image_ptr     =  in_image_ptr;

    for (uint32_t n_range = 0;
                  n_range < in_range_count;
                ++n_range)
    {
        ranges.push_back(in_range_ptrs[n_range]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyBufferCommand::CopyBufferCommand(Anvil::Buffer*      in_src_buffer_ptr,
                                                               Anvil::Buffer*      in_dst_buffer_ptr,
                                                               uint32_t            in_region_count,
                                                               const VkBufferCopy* in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_BUFFER)
{
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    src_buffer     = in_src_buffer_ptr->get_buffer();
    src_buffer_ptr = in_src_buffer_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyBufferToImageCommand::CopyBufferToImageCommand(Anvil::Buffer*           in_src_buffer_ptr,
                                                                             Anvil::Image*            in_dst_image_ptr,
                                                                             VkImageLayout            in_dst_image_layout,
                                                                             uint32_t                 in_region_count,
                                                                             const VkBufferImageCopy* in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_BUFFER_TO_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_buffer       = in_src_buffer_ptr->get_buffer();
    src_buffer_ptr   = in_src_buffer_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyImageCommand::CopyImageCommand(Anvil::Image*      in_src_image_ptr,
                                                             VkImageLayout      in_src_image_layout,
                                                             Anvil::Image*      in_dst_image_ptr,
                                                             VkImageLayout      in_dst_image_layout,
                                                             uint32_t           in_region_count,
                                                             const VkImageCopy* in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyImageToBufferCommand::CopyImageToBufferCommand(Anvil::Image*            in_src_image_ptr,
                                                                             VkImageLayout            in_src_image_layout,
                                                                             Anvil::Buffer*           in_dst_buffer_ptr,
                                                                             uint32_t                 in_region_count,
                                                                             const VkBufferImageCopy* in_region_ptrs)
    :Command(COMMAND_TYPE_COPY_IMAGE_TO_BUFFER)
{
    dst_buffer       = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr   = in_dst_buffer_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::CopyQueryPoolResultsCommand::CopyQueryPoolResultsCommand(Anvil::QueryPool*  in_query_pool_ptr,
                                                                                   Anvil::QueryIndex  in_start_query,
                                                                                   uint32_t           in_query_count,
                                                                                   Anvil::Buffer*     in_dst_buffer_ptr,
                                                                                   VkDeviceSize       in_dst_offset,
                                                                                   VkDeviceSize       in_dst_stride,
                                                                                   VkQueryResultFlags in_flags)
    :Command(COMMAND_TYPE_COPY_QUERY_POOL_RESULTS)
{
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
    dst_stride     = in_dst_stride;
    flags          = in_flags;
    query_count    = in_query_count;
    query_pool_ptr = in_query_pool_ptr;
    start_query    = in_start_query;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DebugMarkerBeginEXTCommand::DebugMarkerBeginEXTCommand(const std::string& in_marker_name,
                                                                                 const float*       in_color)
    :Command(Anvil::COMMAND_TYPE_DEBUG_MARKER_BEGIN_EXT)
{
    if (in_color != nullptr)
    {
        memcpy(color,
               in_color,
               sizeof(color) );
    }
    else
    {
        memset(color,
               0,
               sizeof(color) );
    }

    marker_name = in_marker_name;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DebugMarkerEndEXTCommand::DebugMarkerEndEXTCommand()
    :Command(Anvil::COMMAND_TYPE_DEBUG_MARKER_END_EXT)
{
    /* Stub */
}

/** Please see header for specification */
Anvil::CommandBufferBase::DebugMarkerInsertEXTCommand::DebugMarkerInsertEXTCommand(const std::string& in_marker_name,
                                                                                   const float*       in_color)
    :Command(Anvil::COMMAND_TYPE_DEBUG_MARKER_INSERT_EXT)
{
    if (in_color != nullptr)
    {
        memcpy(color,
               in_color,
               sizeof(color) );
    }
    else
    {
        memset(color,
               0,
               sizeof(color) );
    }

    marker_name = in_marker_name;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DispatchCommand::DispatchCommand(uint32_t in_x,
                                                           uint32_t in_y,
                                                           uint32_t in_z)
    :Command(COMMAND_TYPE_DISPATCH)
{
    x = in_x;
    y = in_y;
    z = in_z;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DispatchIndirectCommand::DispatchIndirectCommand(Anvil::Buffer* in_buffer_ptr,
                                                                           VkDeviceSize   in_offset)
    :Command(COMMAND_TYPE_DISPATCH_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    offset     = in_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawCommand::DrawCommand(uint32_t in_vertex_count,
                                                   uint32_t in_instance_count,
                                                   uint32_t in_first_vertex,
                                                   uint32_t in_first_instance)
    :Command(COMMAND_TYPE_DRAW)
{
    first_instance = in_first_instance;
    first_vertex   = in_first_vertex;
    instance_count = in_instance_count;
    vertex_count   = in_vertex_count;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedCommand::DrawIndexedCommand(uint32_t in_index_count,
                                                                 uint32_t in_instance_count,
                                                                 uint32_t in_first_index,
                                                                 int32_t  in_vertex_offset,
                                                                 uint32_t in_first_instance)
    :Command(COMMAND_TYPE_DRAW_INDEXED)
{
    first_index    = in_first_index;
    first_instance = in_first_instance;
    index_count    = in_index_count;
    instance_count = in_instance_count;
    vertex_offset  = in_vertex_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedIndirectCommand::DrawIndexedIndirectCommand(Anvil::Buffer* in_buffer_ptr,
                                                                                 VkDeviceSize   in_offset,
                                                                                 uint32_t       in_draw_count,
                                                                                 uint32_t       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDEXED_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    draw_count = in_draw_count;
    offset     = in_offset;
    stride     = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndexedIndirectCountAMDCommand::DrawIndexedIndirectCountAMDCommand(Anvil::Buffer* in_buffer_ptr,
                                                                                                 VkDeviceSize   in_offset,
                                                                                                 Anvil::Buffer* in_count_buffer_ptr,
                                                                                                 VkDeviceSize   in_count_offset,
                                                                                                 uint32_t       in_max_draw_count,
                                                                                                 uint32_t       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDEXED_INDIRECT_COUNT_AMD)
{
    buffer           = in_buffer_ptr->get_buffer();
    buffer_ptr       = in_buffer_ptr;
    count_buffer     = in_count_buffer_ptr->get_buffer();
    count_buffer_ptr = in_count_buffer_ptr;
    count_offset     = in_count_offset;
    max_draw_count   = in_max_draw_count;
    offset           = in_offset;
    stride           = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndirectCommand::DrawIndirectCommand(Anvil::Buffer* in_buffer_ptr,
                                                                   VkDeviceSize   in_offset,
                                                                   uint32_t       in_count,
                                                                   uint32_t       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDIRECT)
{
    buffer     = in_buffer_ptr->get_buffer();
    buffer_ptr = in_buffer_ptr;
    count      = in_count;
    offset     = in_offset;
    stride     = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::DrawIndirectCountAMDCommand::DrawIndirectCountAMDCommand(Anvil::Buffer* in_buffer_ptr,
                                                                                   VkDeviceSize   in_offset,
                                                                                   Anvil::Buffer* in_count_buffer_ptr,
                                                                                   VkDeviceSize   in_count_offset,
                                                                                   uint32_t       in_max_draw_count,
                                                                                   uint32_t       in_stride)
    :Command(COMMAND_TYPE_DRAW_INDIRECT_COUNT_AMD)
{
    buffer           = in_buffer_ptr->get_buffer();
    buffer_ptr       = in_buffer_ptr;
    count_buffer     = in_count_buffer_ptr->get_buffer();
    count_buffer_ptr = in_count_buffer_ptr;
    count_offset     = in_count_offset;
    max_draw_count   = in_max_draw_count;
    offset           = in_offset;
    stride           = in_stride;
}

/** Please see header for specification */
Anvil::CommandBufferBase::EndQueryCommand::EndQueryCommand(Anvil::QueryPool* in_query_pool_ptr,
                                                           Anvil::QueryIndex in_entry)
    :Command(COMMAND_TYPE_END_QUERY)
{
    entry          = in_entry;
    query_pool_ptr = in_query_pool_ptr;
}

/** Please see header for specification */
Anvil::EndRenderPassCommand::EndRenderPassCommand()
    :Command(COMMAND_TYPE_END_RENDER_PASS)
{
}

/** Please see header for specification */
Anvil::CommandBufferBase::ExecuteCommandsCommand::ExecuteCommandsCommand(uint32_t                        in_cmd_buffers_count,
                                                                         Anvil::SecondaryCommandBuffer** in_cmd_buffer_ptrs)
    :Command(COMMAND_TYPE_EXECUTE_COMMANDS)
{
    for (uint32_t n_cmd_buffer = 0;
                  n_cmd_buffer < in_cmd_buffers_count;
                ++n_cmd_buffer)
    {
        command_buffers.push_back    (in_cmd_buffer_ptrs[n_cmd_buffer]->get_command_buffer() );
        command_buffer_ptrs.push_back(in_cmd_buffer_ptrs[n_cmd_buffer]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::FillBufferCommand::FillBufferCommand(Anvil::Buffer* in_dst_buffer_ptr,
                                                               VkDeviceSize   in_dst_offset,
                                                               VkDeviceSize   in_size,
                                                               uint32_t       in_data)
    :Command(COMMAND_TYPE_FILL_BUFFER)
{
    data           = in_data;
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
    size           = in_size;
}

/** Please see header for specification */
Anvil::CommandBufferBase::NextSubpassCommand::NextSubpassCommand(VkSubpassContents in_contents)
    :Command(COMMAND_TYPE_NEXT_SUBPASS)
{
    contents = in_contents;
}

/** Please see header for specification */
Anvil::PipelineBarrierCommand::PipelineBarrierCommand(VkPipelineStageFlags       in_src_stage_mask,
                                                      VkPipelineStageFlags       in_dst_stage_mask,
                                                      VkDependencyFlags          in_flags,
                                                      uint32_t                   in_memory_barrier_count,
                                                      const MemoryBarrier* const in_memory_barrier_ptr_ptr,
                                                      uint32_t                   in_buffer_memory_barrier_count,
                                                      const BufferBarrier* const in_buffer_memory_barrier_ptr_ptr,
                                                      uint32_t                   in_image_memory_barrier_count,
                                                      const ImageBarrier*  const in_image_memory_barrier_ptr_ptr)
    :Command(COMMAND_TYPE_PIPELINE_BARRIER)
{
    dst_stage_mask = static_cast<VkPipelineStageFlagBits>(in_dst_stage_mask);
    flags          = in_flags;
    src_stage_mask = static_cast<VkPipelineStageFlagBits>(in_src_stage_mask);

    for (uint32_t n_buffer_memory_barrier = 0;
                  n_buffer_memory_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_memory_barrier)
    {
        buffer_barriers.push_back(in_buffer_memory_barrier_ptr_ptr[n_buffer_memory_barrier]);
    }

    for (uint32_t n_image_memory_barrier = 0;
                  n_image_memory_barrier < in_image_memory_barrier_count;
                ++n_image_memory_barrier)
    {
        image_barriers.push_back(in_image_memory_barrier_ptr_ptr[n_image_memory_barrier]);
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers.push_back(in_memory_barrier_ptr_ptr[n_memory_barrier]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::PushConstantsCommand::PushConstantsCommand(Anvil::PipelineLayout* in_layout_ptr,
                                                                     VkShaderStageFlags     in_stage_flags,
                                                                     uint32_t               in_offset,
                                                                     uint32_t               in_size,
                                                                     const void*            in_values)
    :Command(COMMAND_TYPE_PUSH_CONSTANTS)
{
    layout_ptr  = in_layout_ptr;
    offset      = in_offset;
    size        = in_size;
    stage_flags = in_stage_flags;
    values      = in_values;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResetEventCommand::ResetEventCommand(Anvil::Event*        in_event_ptr,
                                                               VkPipelineStageFlags in_stage_mask)
    :Command(COMMAND_TYPE_RESET_EVENT)
{
    event      = in_event_ptr->get_event();
    event_ptr  = in_event_ptr;
    stage_mask = in_stage_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResetQueryPoolCommand::ResetQueryPoolCommand(Anvil::QueryPool* in_query_pool_ptr,
                                                                       Anvil::QueryIndex in_start_query,
                                                                       uint32_t          in_query_count)
    :Command(COMMAND_TYPE_RESET_QUERY_POOL)
{
    query_count    = in_query_count;
    query_pool_ptr = in_query_pool_ptr;
    start_query    = in_start_query;
}

/** Please see header for specification */
Anvil::CommandBufferBase::ResolveImageCommand::ResolveImageCommand(Anvil::Image*         in_src_image_ptr,
                                                                   VkImageLayout         in_src_image_layout,
                                                                   Anvil::Image*         in_dst_image_ptr,
                                                                   VkImageLayout         in_dst_image_layout,
                                                                   uint32_t              in_region_count,
                                                                   const VkImageResolve* in_region_ptrs)
    :Command(COMMAND_TYPE_RESOLVE_IMAGE)
{
    dst_image        = in_dst_image_ptr->get_image();
    dst_image_layout = in_dst_image_layout;
    dst_image_ptr    = in_dst_image_ptr;
    src_image        = in_src_image_ptr->get_image();
    src_image_layout = in_src_image_layout;
    src_image_ptr    = in_src_image_ptr;

    for (uint32_t n_region = 0;
                  n_region < in_region_count;
                ++n_region)
    {
        regions.push_back(in_region_ptrs[n_region]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetBlendConstantsCommand::SetBlendConstantsCommand(const float in_blend_constants[4])
    :Command(COMMAND_TYPE_SET_BLEND_CONSTANTS)
{
    memcpy(blend_constants,
           in_blend_constants,
           sizeof(float) * 4);
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetDepthBiasCommand::SetDepthBiasCommand(float in_depth_bias_constant_factor,
                                                                   float in_depth_bias_clamp,
                                                                   float in_slope_scaled_depth_bias)
    :Command(COMMAND_TYPE_SET_DEPTH_BIAS)
{
    depth_bias_clamp           = in_depth_bias_clamp;
    depth_bias_constant_factor = in_depth_bias_constant_factor;
    slope_scaled_depth_bias    = in_slope_scaled_depth_bias;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetDepthBoundsCommand::SetDepthBoundsCommand(float in_min_depth_bounds,
                                                                       float in_max_depth_bounds)
    :Command(COMMAND_TYPE_SET_DEPTH_BOUNDS)
{
    max_depth_bounds = in_max_depth_bounds;
    min_depth_bounds = in_min_depth_bounds;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetEventCommand::SetEventCommand(Anvil::Event*        in_event_ptr,
                                                           VkPipelineStageFlags in_stage_mask)
    :Command(COMMAND_TYPE_SET_EVENT)
{
    event      = in_event_ptr->get_event();
    event_ptr  = in_event_ptr;
    stage_mask = in_stage_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetLineWidthCommand::SetLineWidthCommand(float in_line_width)
    :Command(COMMAND_TYPE_SET_LINE_WIDTH)
{
    line_width = in_line_width;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetScissorCommand::SetScissorCommand(uint32_t        in_first_scissor,
                                                               uint32_t        in_scissor_count,
                                                               const VkRect2D* in_scissor_ptrs)
    :Command(COMMAND_TYPE_SET_SCISSOR)
{
    first_scissor = in_first_scissor;

    for (uint32_t n_scissor = 0;
                  n_scissor < in_scissor_count;
                ++n_scissor)
    {
        scissors.push_back(in_scissor_ptrs[n_scissor]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilCompareMaskCommand::SetStencilCompareMaskCommand(VkStencilFaceFlags in_face_mask,
                                                                                     uint32_t           in_stencil_compare_mask)
    :Command(COMMAND_TYPE_SET_STENCIL_COMPARE_MASK)
{
    face_mask            = in_face_mask;
    stencil_compare_mask = in_stencil_compare_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilReferenceCommand::SetStencilReferenceCommand(VkStencilFaceFlags in_face_mask,
                                                                                 uint32_t           in_stencil_reference)
    :Command(COMMAND_TYPE_SET_STENCIL_REFERENCE)
{
    face_mask         = in_face_mask;
    stencil_reference = in_stencil_reference;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetStencilWriteMaskCommand::SetStencilWriteMaskCommand(VkStencilFaceFlags in_face_mask,
                                                                                 uint32_t           in_stencil_write_mask)
    :Command(COMMAND_TYPE_SET_STENCIL_WRITE_MASK)
{
    face_mask          = in_face_mask;
    stencil_write_mask = in_stencil_write_mask;
}

/** Please see header for specification */
Anvil::CommandBufferBase::SetViewportCommand::SetViewportCommand(uint32_t          in_first_viewport,
                                                                 uint32_t          in_viewport_count,
                                                                 const VkViewport* in_viewport_ptrs)
    :Command(COMMAND_TYPE_SET_VIEWPORT)
{
    first_viewport = in_first_viewport;

    for (uint32_t n_viewport = 0;
                  n_viewport < in_viewport_count;
                ++n_viewport)
    {
        viewports.push_back(in_viewport_ptrs[n_viewport]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::UpdateBufferCommand::UpdateBufferCommand(Anvil::Buffer* in_dst_buffer_ptr,
                                                                   VkDeviceSize   in_dst_offset,
                                                                   VkDeviceSize   in_data_size,
                                                                   const void*    in_data_ptr)
    :Command(COMMAND_TYPE_UPDATE_BUFFER)
{
    data_ptr       = in_data_ptr;
    data_size      = in_data_size;
    dst_buffer     = in_dst_buffer_ptr->get_buffer();
    dst_buffer_ptr = in_dst_buffer_ptr;
    dst_offset     = in_dst_offset;
}

/** Please see header for specification */
Anvil::CommandBufferBase::WaitEventsCommand::WaitEventsCommand(uint32_t                   in_event_count,
                                                               Anvil::Event* const*       in_event_ptrs,
                                                               VkPipelineStageFlags       in_src_stage_mask,
                                                               VkPipelineStageFlags       in_dst_stage_mask,
                                                               uint32_t                   in_memory_barrier_count,
                                                               const MemoryBarrier* const in_memory_barriers_ptr,
                                                               uint32_t                   in_buffer_memory_barrier_count,
                                                               const BufferBarrier* const in_buffer_memory_barriers_ptr,
                                                               uint32_t                   in_image_memory_barrier_count,
                                                               const ImageBarrier* const  in_image_memory_barriers_ptr)
    :Command(COMMAND_TYPE_WAIT_EVENTS)
{
    dst_stage_mask = static_cast<VkPipelineStageFlagBits>(in_dst_stage_mask);
    src_stage_mask = static_cast<VkPipelineStageFlagBits>(in_src_stage_mask);

    for (uint32_t n_event = 0;
                  n_event < in_event_count;
                ++n_event)
    {
        events.push_back    (in_event_ptrs[n_event]->get_event() );
        event_ptrs.push_back(in_event_ptrs[n_event]);
    }

    for (uint32_t n_buffer_memory_barrier = 0;
                  n_buffer_memory_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_memory_barrier)
    {
        buffer_barriers.push_back(in_buffer_memory_barriers_ptr[n_buffer_memory_barrier]);
    }

    for (uint32_t n_image_memory_barrier = 0;
                  n_image_memory_barrier < in_image_memory_barrier_count;
                ++n_image_memory_barrier)
    {
        image_barriers.push_back(in_image_memory_barriers_ptr[n_image_memory_barrier]);
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers.push_back(in_memory_barriers_ptr[n_memory_barrier]);
    }
}

/** Please see header for specification */
Anvil::CommandBufferBase::WriteTimestampCommand::WriteTimestampCommand(VkPipelineStageFlagBits in_pipeline_stage,
                                                                       Anvil::QueryPool*       in_query_pool_ptr,
                                                                       Anvil::QueryIndex       in_entry)
    :Command(COMMAND_TYPE_WRITE_TIMESTAMP)
{
    entry          = in_entry;
    pipeline_stage = in_pipeline_stage;
    query_pool_ptr = in_query_pool_ptr;
}


/** Constructor.
 *
 *  @param device_ptr              Device to use.
 *  @param parent_command_pool_ptr Command pool to allocate the commands from. Must not be nullptr.
 *  @param type                    Command buffer type
 **/
Anvil::CommandBufferBase::CommandBufferBase(const Anvil::BaseDevice* in_device_ptr,
                                            Anvil::CommandPool*      in_parent_command_pool_ptr,
                                            Anvil::CommandBufferType in_type,
                                            bool                     in_mt_safe)
    :MTSafetySupportProvider   (in_mt_safe),
     DebugMarkerSupportProvider(in_device_ptr,
                                VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT),
     CallbacksSupportProvider  (COMMAND_BUFFER_CALLBACK_ID_COUNT),
     m_command_buffer          (VK_NULL_HANDLE),
     m_device_ptr              (in_device_ptr),
     m_is_renderpass_active    (false),
     m_parent_command_pool_ptr (in_parent_command_pool_ptr),
     m_recording_in_progress   (false),
     m_type                    (in_type)
{
    anvil_assert(in_parent_command_pool_ptr != nullptr);
}

/** Destructor.
 *
 *  Releases the underlying Vulkan command buffer instance.
 *
 *  Throws an assertion failure if recording is in progress.
 **/
Anvil::CommandBufferBase::~CommandBufferBase()
{
    anvil_assert(!m_recording_in_progress);

    if (m_command_buffer          != VK_NULL_HANDLE &&
        m_parent_command_pool_ptr != nullptr)
    {
        /* Physically free the command buffer we own */
        m_parent_command_pool_ptr->lock();
        lock();
        {
            vkFreeCommandBuffers(m_device_ptr->get_device_vk(),
                                 m_parent_command_pool_ptr->get_command_pool(),
                                 1, /* commandBufferCount */
                                &m_command_buffer);
        }
        unlock();
        m_parent_command_pool_ptr->unlock();

        m_command_buffer = VK_NULL_HANDLE;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        clear_commands();
    }
    #endif
}

#ifdef STORE_COMMAND_BUFFER_COMMANDS
    /** Clears the command vector by releasing all command descriptors back to the heap memory. */
    void Anvil::CommandBufferBase::clear_commands()
    {
        m_commands.clear();
    }
#endif

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_begin_query(Anvil::QueryPool*   in_query_pool_ptr,
                                                  Anvil::QueryIndex   in_entry,
                                                  VkQueryControlFlags in_flags)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BeginQueryCommand(in_query_pool_ptr,
                                                   in_entry,
                                                   in_flags) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBeginQuery(m_command_buffer,
                        in_query_pool_ptr->get_query_pool(),
                        in_entry,
                        in_flags);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_descriptor_sets(VkPipelineBindPoint                in_pipeline_bind_point,
                                                           Anvil::PipelineLayout*             in_layout_ptr,
                                                           uint32_t                           in_first_set,
                                                           uint32_t                           in_set_count,
                                                           const Anvil::DescriptorSet* const* in_descriptor_set_ptrs,
                                                           uint32_t                           in_dynamic_offset_count,
                                                           const uint32_t*                    in_dynamic_offset_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    auto dss_vk = std::vector<VkDescriptorSet>(in_set_count);
    bool result = false;

    for (uint32_t n_set = 0;
                  n_set < in_set_count;
                ++n_set)
    {
        dss_vk.at(n_set) = in_descriptor_set_ptrs[n_set]->get_descriptor_set_vk();
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindDescriptorSetsCommand(in_pipeline_bind_point,
                                                           in_layout_ptr,
                                                           in_first_set,
                                                           in_set_count,
                                                           in_descriptor_set_ptrs,
                                                           in_dynamic_offset_count,
                                                           in_dynamic_offset_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBindDescriptorSets(m_command_buffer,
                                in_pipeline_bind_point,
                                in_layout_ptr->get_pipeline_layout(),
                                in_first_set,
                                in_set_count,
                                (in_set_count > 0) ? &dss_vk.at(0) : nullptr,
                                in_dynamic_offset_count,
                                in_dynamic_offset_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_index_buffer(Anvil::Buffer* in_buffer_ptr,
                                                        VkDeviceSize   in_offset,
                                                        VkIndexType    in_index_type)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindIndexBufferCommand(in_buffer_ptr,
                                                        in_offset,
                                                        in_index_type) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBindIndexBuffer(m_command_buffer,
                             in_buffer_ptr->get_buffer(),
                             in_offset,
                             in_index_type);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_pipeline(VkPipelineBindPoint in_pipeline_bind_point,
                                                    Anvil::PipelineID   in_pipeline_id)
{
    /* Command supported inside and outside the renderpass. */
    VkPipeline pipeline_vk      (VK_NULL_HANDLE);
    bool       result           (false);

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    anvil_assert(in_pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE ||
                 in_pipeline_bind_point == VK_PIPELINE_BIND_POINT_GRAPHICS);

    pipeline_vk = (in_pipeline_bind_point == VK_PIPELINE_BIND_POINT_COMPUTE)  ? m_device_ptr->get_compute_pipeline_manager()->get_pipeline (in_pipeline_id)
                                                                              : m_device_ptr->get_graphics_pipeline_manager()->get_pipeline(in_pipeline_id);

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindPipelineCommand(in_pipeline_bind_point,
                                                     in_pipeline_id) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBindPipeline(m_command_buffer,
                          in_pipeline_bind_point,
                          pipeline_vk);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_bind_vertex_buffers(uint32_t            in_start_binding,
                                                          uint32_t            in_binding_count,
                                                          Anvil::Buffer**     in_buffer_ptrs,
                                                          const VkDeviceSize* in_offset_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    auto buffers = std::vector<VkBuffer>(in_binding_count);
    bool result  = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BindVertexBuffersCommand(in_start_binding,
                                                          in_binding_count,
                                                          in_buffer_ptrs,
                                                          in_offset_ptrs) );
        }
    }
    #endif

    for (uint32_t n_binding = 0;
                  n_binding < in_binding_count;
                ++n_binding)
    {
        buffers.at(n_binding) = in_buffer_ptrs[n_binding]->get_buffer();
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBindVertexBuffers(m_command_buffer,
                               in_start_binding,
                               in_binding_count,
                               (in_binding_count > 0) ? &buffers.at(0) : nullptr,
                               in_offset_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_blit_image(Anvil::Image*      in_src_image_ptr,
                                                 VkImageLayout      in_src_image_layout,
                                                 Anvil::Image*      in_dst_image_ptr,
                                                 VkImageLayout      in_dst_image_layout,
                                                 uint32_t           in_region_count,
                                                 const VkImageBlit* in_region_ptrs,
                                                 VkFilter           in_filter)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BlitImageCommand(in_src_image_ptr,
                                                  in_src_image_layout,
                                                  in_dst_image_ptr,
                                                  in_dst_image_layout,
                                                  in_region_count,
                                                  in_region_ptrs,
                                                  in_filter) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdBlitImage(m_command_buffer,
                       in_src_image_ptr->get_image(),
                       in_src_image_layout,
                       in_dst_image_ptr->get_image(),
                       in_dst_image_layout,
                       in_region_count,
                       in_region_ptrs,
                       in_filter);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_attachments(uint32_t                 in_n_attachments,
                                                        const VkClearAttachment* in_attachment_ptrs,
                                                        uint32_t                 in_n_rects,
                                                        const VkClearRect*       in_rect_ptrs)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearAttachmentsCommand(in_n_attachments,
                                                         in_attachment_ptrs,
                                                         in_n_rects,
                                                         in_rect_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdClearAttachments(m_command_buffer,
                              in_n_attachments,
                              in_attachment_ptrs,
                              in_n_rects,
                              in_rect_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_color_image(Anvil::Image*                  in_image_ptr,
                                                        VkImageLayout                  in_image_layout,
                                                        const VkClearColorValue*       in_color_ptr,
                                                        uint32_t                       in_range_count,
                                                        const VkImageSubresourceRange* in_range_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearColorImageCommand(in_image_ptr,
                                                        in_image_layout,
                                                        in_color_ptr,
                                                        in_range_count,
                                                        in_range_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdClearColorImage(m_command_buffer,
                             in_image_ptr->get_image(),
                             in_image_layout,
                             in_color_ptr,
                             in_range_count,
                             in_range_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_clear_depth_stencil_image(Anvil::Image*                   in_image_ptr,
                                                                VkImageLayout                   in_image_layout,
                                                                const VkClearDepthStencilValue* in_depth_stencil_ptr,
                                                                uint32_t                        in_range_count,
                                                                const VkImageSubresourceRange*  in_range_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ClearDepthStencilImageCommand(in_image_ptr,
                                                               in_image_layout,
                                                               in_depth_stencil_ptr,
                                                               in_range_count,
                                                               in_range_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdClearDepthStencilImage(m_command_buffer,
                                    in_image_ptr->get_image(),
                                    in_image_layout,
                                    in_depth_stencil_ptr,
                                    in_range_count,
                                    in_range_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_buffer(Anvil::Buffer*      in_src_buffer_ptr,
                                                  Anvil::Buffer*      in_dst_buffer_ptr,
                                                  uint32_t            in_region_count,
                                                  const VkBufferCopy* in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyBufferCommand(in_src_buffer_ptr,
                                                   in_dst_buffer_ptr,
                                                   in_region_count,
                                                   in_region_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdCopyBuffer(m_command_buffer,
                        in_src_buffer_ptr->get_buffer(),
                        in_dst_buffer_ptr->get_buffer(),
                        in_region_count,
                        in_region_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_buffer_to_image(Anvil::Buffer*           in_src_buffer_ptr,
                                                           Anvil::Image*            in_dst_image_ptr,
                                                           VkImageLayout            in_dst_image_layout,
                                                           uint32_t                 in_region_count,
                                                           const VkBufferImageCopy* in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyBufferToImageCommand(in_src_buffer_ptr,
                                                          in_dst_image_ptr,
                                                          in_dst_image_layout,
                                                          in_region_count,
                                                          in_region_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdCopyBufferToImage(m_command_buffer,
                               in_src_buffer_ptr->get_buffer(),
                               in_dst_image_ptr->get_image(),
                               in_dst_image_layout,
                               in_region_count,
                               in_region_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_image(Anvil::Image*      in_src_image_ptr,
                                                 VkImageLayout      in_src_image_layout,
                                                 Anvil::Image*      in_dst_image_ptr,
                                                 VkImageLayout      in_dst_image_layout,
                                                 uint32_t           in_region_count,
                                                 const VkImageCopy* in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyImageCommand(in_src_image_ptr,
                                                  in_src_image_layout,
                                                  in_dst_image_ptr,
                                                  in_dst_image_layout,
                                                  in_region_count,
                                                  in_region_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdCopyImage(m_command_buffer,
                       in_src_image_ptr->get_image(),
                       in_src_image_layout,
                       in_dst_image_ptr->get_image(),
                       in_dst_image_layout,
                       in_region_count,
                       in_region_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_image_to_buffer(Anvil::Image*             in_src_image_ptr,
                                                           VkImageLayout             in_src_image_layout,
                                                           Anvil::Buffer*            in_dst_buffer_ptr,
                                                           uint32_t                  in_region_count,
                                                           const VkBufferImageCopy*  in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyImageToBufferCommand(in_src_image_ptr,
                                                          in_src_image_layout,
                                                          in_dst_buffer_ptr,
                                                          in_region_count,
                                                          in_region_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdCopyImageToBuffer(m_command_buffer,
                               in_src_image_ptr->get_image(),
                               in_src_image_layout,
                               in_dst_buffer_ptr->get_buffer(),
                               in_region_count,
                               in_region_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_copy_query_pool_results(Anvil::QueryPool*  in_query_pool_ptr,
                                                              Anvil::QueryIndex  in_start_query,
                                                              uint32_t           in_query_count,
                                                              Anvil::Buffer*     in_dst_buffer_ptr,
                                                              VkDeviceSize       in_dst_offset,
                                                              VkDeviceSize       in_dst_stride,
                                                              VkQueryResultFlags in_flags)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(CopyQueryPoolResultsCommand(in_query_pool_ptr,
                                                             in_start_query,
                                                             in_query_count,
                                                             in_dst_buffer_ptr,
                                                             in_dst_offset,
                                                             in_dst_stride,
                                                             in_flags) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdCopyQueryPoolResults(m_command_buffer,
                                  in_query_pool_ptr->get_query_pool(),
                                  in_start_query,
                                  in_query_count,
                                  in_dst_buffer_ptr->get_buffer(),
                                  in_dst_offset,
                                  in_dst_stride,
                                  in_flags);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_dispatch(uint32_t in_x,
                                               uint32_t in_y,
                                               uint32_t in_z)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DispatchCommand(in_x,
                                                 in_y,
                                                 in_z) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDispatch(m_command_buffer,
                      in_x,
                      in_y,
                      in_z);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_debug_marker_begin_EXT(const std::string& in_marker_name,
                                                             const float*       in_opt_color)
{
    const auto&                entrypoints (m_device_ptr->get_extension_ext_debug_marker_entrypoints() );
    VkDebugMarkerMarkerInfoEXT marker_info;
    bool                       result      (false);

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DebugMarkerBeginEXTCommand(in_marker_name,
                                                            in_opt_color) );
        }
    }
    #endif

    anvil_assert(m_device_ptr->get_extension_info()->ext_debug_marker() );

    if (in_opt_color != nullptr)
    {
        memcpy(marker_info.color,
               in_opt_color,
               sizeof(float) * 4);
    }
    else
    {
        marker_info.color[0] = 0.0f;
        marker_info.color[1] = 0.0f;
        marker_info.color[2] = 0.0f;
        marker_info.color[3] = 0.0f;
    }

    marker_info.pMarkerName = in_marker_name.c_str();
    marker_info.pNext       = nullptr;
    marker_info.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;

    m_parent_command_pool_ptr->lock();
    {
        entrypoints.vkCmdDebugMarkerBeginEXT(m_command_buffer,
                                            &marker_info);
    }
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_debug_marker_end_EXT()
{
    const auto& entrypoints(m_device_ptr->get_extension_ext_debug_marker_entrypoints() );
    bool        result     (false);

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    anvil_assert(m_device_ptr->get_extension_info()->ext_debug_marker() );


    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DebugMarkerEndEXTCommand() );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    {
        entrypoints.vkCmdDebugMarkerEndEXT(m_command_buffer);
    }
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_debug_marker_insert_EXT(const std::string& in_marker_name,
                                                              const float*       in_opt_color)
{
    const auto&                entrypoints (m_device_ptr->get_extension_ext_debug_marker_entrypoints() );
    VkDebugMarkerMarkerInfoEXT marker_info;
    bool                       result      (false);

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    anvil_assert(m_device_ptr->get_extension_info()->ext_debug_marker() );


    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DebugMarkerInsertEXTCommand(in_marker_name,
                                                             in_opt_color) );
        }
    }
    #endif

    if (in_opt_color != nullptr)
    {
        memcpy(marker_info.color,
               in_opt_color,
               sizeof(float) * 4);
    }
    else
    {
        marker_info.color[0] = 0.0f;
        marker_info.color[1] = 0.0f;
        marker_info.color[2] = 0.0f;
        marker_info.color[3] = 0.0f;
    }

    marker_info.pMarkerName = in_marker_name.c_str();
    marker_info.pNext       = nullptr;
    marker_info.sType       = VK_STRUCTURE_TYPE_DEBUG_MARKER_MARKER_INFO_EXT;

    m_parent_command_pool_ptr->lock();
    {
        entrypoints.vkCmdDebugMarkerInsertEXT(m_command_buffer,
                                             &marker_info);
    }
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_dispatch_indirect(Anvil::Buffer* in_buffer_ptr,
                                                        VkDeviceSize   in_offset)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DispatchIndirectCommand(in_buffer_ptr,
                                                         in_offset) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDispatchIndirect(m_command_buffer,
                              in_buffer_ptr->get_buffer(),
                              in_offset);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw(uint32_t in_vertex_count,
                                           uint32_t in_instance_count,
                                           uint32_t in_first_vertex,
                                           uint32_t in_first_instance)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawCommand(in_vertex_count,
                                             in_instance_count,
                                             in_first_vertex,
                                             in_first_instance) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDraw(m_command_buffer,
                  in_vertex_count,
                  in_instance_count,
                  in_first_vertex,
                  in_first_instance);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed(uint32_t in_index_count,
                                                   uint32_t in_instance_count,
                                                   uint32_t in_first_index,
                                                   int32_t  in_vertex_offset,
                                                   uint32_t in_first_instance)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedCommand(in_index_count,
                                                    in_instance_count,
                                                    in_first_index,
                                                    in_vertex_offset,
                                                    in_first_instance) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDrawIndexed(m_command_buffer,
                         in_index_count,
                         in_instance_count,
                         in_first_index,
                         in_vertex_offset,
                         in_first_instance);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed_indirect(Anvil::Buffer* in_buffer_ptr,
                                                            VkDeviceSize   in_offset,
                                                            uint32_t       in_count,
                                                            uint32_t       in_stride)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedIndirectCommand(in_buffer_ptr,
                                                            in_offset,
                                                            in_count,
                                                            in_stride) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDrawIndexedIndirect(m_command_buffer,
                                 in_buffer_ptr->get_buffer(),
                                 in_offset,
                                 in_count,
                                 in_stride);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indexed_indirect_count_AMD(Anvil::Buffer* in_buffer_ptr,
                                                                      VkDeviceSize   in_offset,
                                                                      Anvil::Buffer* in_count_buffer_ptr,
                                                                      VkDeviceSize   in_count_offset,
                                                                      uint32_t       in_max_draw_count,
                                                                      uint32_t       in_stride)
{
    Anvil::ExtensionAMDDrawIndirectCountEntrypoints entrypoints;
    bool                                            result     (false);

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    anvil_assert(m_device_ptr->get_extension_info()->amd_draw_indirect_count() );


    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndexedIndirectCountAMDCommand(in_buffer_ptr,
                                                                    in_offset,
                                                                    in_count_buffer_ptr,
                                                                    in_count_offset,
                                                                    in_max_draw_count,
                                                                    in_stride) );
        }
    }
    #endif

    entrypoints = m_device_ptr->get_extension_amd_draw_indirect_count_entrypoints();
    
    m_parent_command_pool_ptr->lock();
    lock();
    {
        entrypoints.vkCmdDrawIndexedIndirectCountAMD(m_command_buffer,
                                                     in_buffer_ptr->get_buffer(),
                                                     in_offset,
                                                     in_count_buffer_ptr->get_buffer(),
                                                     in_count_offset,
                                                     in_max_draw_count,
                                                     in_stride);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indirect(Anvil::Buffer* in_buffer_ptr,
                                                    VkDeviceSize   in_offset,
                                                    uint32_t       in_count,
                                                    uint32_t       in_stride)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndirectCommand(in_buffer_ptr,
                                                     in_offset,
                                                     in_count,
                                                     in_stride) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdDrawIndirect(m_command_buffer,
                          in_buffer_ptr->get_buffer(),
                          in_offset,
                          in_count,
                          in_stride);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_draw_indirect_count_AMD(Anvil::Buffer* in_buffer_ptr,
                                                              VkDeviceSize   in_offset,
                                                              Anvil::Buffer* in_count_buffer_ptr,
                                                              VkDeviceSize   in_count_offset,
                                                              uint32_t       in_max_draw_count,
                                                              uint32_t       in_stride)
{
    Anvil::ExtensionAMDDrawIndirectCountEntrypoints entrypoints;
    bool                                            result     (false);

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    anvil_assert(m_device_ptr->get_extension_info()->amd_draw_indirect_count() );


    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(DrawIndirectCountAMDCommand(in_buffer_ptr,
                                                             in_offset,
                                                             in_count_buffer_ptr,
                                                             in_count_offset,
                                                             in_max_draw_count,
                                                             in_stride) );
        }
    }
    #endif

    entrypoints = m_device_ptr->get_extension_amd_draw_indirect_count_entrypoints();

    m_parent_command_pool_ptr->lock();
    lock();
    {
        entrypoints.vkCmdDrawIndirectCountAMD(m_command_buffer,
                                              in_buffer_ptr->get_buffer(),
                                              in_offset,
                                              in_count_buffer_ptr->get_buffer(),
                                              in_count_offset,
                                              in_max_draw_count,
                                              in_stride);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_end_query(Anvil::QueryPool* in_query_pool_ptr,
                                                Anvil::QueryIndex in_entry)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(EndQueryCommand(in_query_pool_ptr,
                                                 in_entry) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdEndQuery(m_command_buffer,
                      in_query_pool_ptr->get_query_pool(),
                      in_entry);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_fill_buffer(Anvil::Buffer* in_dst_buffer_ptr,
                                                  VkDeviceSize   in_dst_offset,
                                                  VkDeviceSize   in_size,
                                                  uint32_t       in_data)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(FillBufferCommand(in_dst_buffer_ptr,
                                                   in_dst_offset,
                                                   in_size,
                                                   in_data) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdFillBuffer(m_command_buffer,
                        in_dst_buffer_ptr->get_buffer(),
                        in_dst_offset,
                        in_size,
                        in_data);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_pipeline_barrier(VkPipelineStageFlags       in_src_stage_mask,
                                                       VkPipelineStageFlags       in_dst_stage_mask,
                                                       VkBool32                   in_by_region,
                                                       uint32_t                   in_memory_barrier_count,
                                                       const MemoryBarrier* const in_memory_barriers_ptr,
                                                       uint32_t                   in_buffer_memory_barrier_count,
                                                       const BufferBarrier* const in_buffer_memory_barriers_ptr,
                                                       uint32_t                   in_image_memory_barrier_count,
                                                       const ImageBarrier*  const in_image_memory_barriers_ptr)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    auto buffer_barriers_vk = std::vector<VkBufferMemoryBarrier>(in_buffer_memory_barrier_count);
    auto image_barriers_vk  = std::vector<VkImageMemoryBarrier> (in_image_memory_barrier_count);
    auto memory_barriers_vk = std::vector<VkMemoryBarrier>      (in_memory_barrier_count);
    bool result             = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(PipelineBarrierCommand(in_src_stage_mask,
                                                        in_dst_stage_mask,
                                                        in_by_region,
                                                        in_memory_barrier_count,
                                                        in_memory_barriers_ptr,
                                                        in_buffer_memory_barrier_count,
                                                        in_buffer_memory_barriers_ptr,
                                                        in_image_memory_barrier_count,
                                                        in_image_memory_barriers_ptr) );
        }
    }
    #endif

    if (get_n_of_callback_subscribers(COMMAND_BUFFER_CALLBACK_ID_PIPELINE_BARRIER_COMMAND_RECORDED) > 0)
    {
        PipelineBarrierCommand                       command_data(in_src_stage_mask,
                                                                  in_dst_stage_mask,
                                                                  in_by_region,
                                                                  in_memory_barrier_count,
                                                                  in_memory_barriers_ptr,
                                                                  in_buffer_memory_barrier_count,
                                                                  in_buffer_memory_barriers_ptr,
                                                                  in_image_memory_barrier_count,
                                                                  in_image_memory_barriers_ptr);
        OnPipelineBarrierCommandRecordedCallbackData callback_data(this,
                                                                  &command_data);

        callback(COMMAND_BUFFER_CALLBACK_ID_PIPELINE_BARRIER_COMMAND_RECORDED,
                &callback_data);
    }

    for (uint32_t n_buffer_barrier = 0;
                  n_buffer_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_barrier)
    {
        buffer_barriers_vk.at(n_buffer_barrier) = in_buffer_memory_barriers_ptr[n_buffer_barrier].get_barrier_vk();
    }

    for (uint32_t n_image_barrier = 0;
                  n_image_barrier < in_image_memory_barrier_count;
                ++n_image_barrier)
    {
        image_barriers_vk.at(n_image_barrier) = in_image_memory_barriers_ptr[n_image_barrier].get_barrier_vk();
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers_vk.at(n_memory_barrier) = in_memory_barriers_ptr[n_memory_barrier].get_barrier_vk();
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdPipelineBarrier(m_command_buffer,
                             in_src_stage_mask,
                             in_dst_stage_mask,
                             in_by_region,
                             in_memory_barrier_count,
                             (in_memory_barrier_count > 0) ? &memory_barriers_vk.at(0) : nullptr,
                             in_buffer_memory_barrier_count,
                             (in_buffer_memory_barrier_count > 0) ? &buffer_barriers_vk.at(0) : nullptr,
                             in_image_memory_barrier_count,
                             (in_image_memory_barrier_count > 0) ? &image_barriers_vk.at(0) : nullptr);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_push_constants(Anvil::PipelineLayout* in_layout_ptr,
                                                     VkShaderStageFlags     in_stage_flags,
                                                     uint32_t               in_offset,
                                                     uint32_t               in_size,
                                                     const void*            in_values)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(PushConstantsCommand(in_layout_ptr,
                                                      in_stage_flags,
                                                      in_offset,
                                                      in_size,
                                                      in_values) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdPushConstants(m_command_buffer,
                           in_layout_ptr->get_pipeline_layout(),
                           in_stage_flags,
                           in_offset,
                           in_size,
                           in_values);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_reset_event(Anvil::Event*        in_event_ptr,
                                                  VkPipelineStageFlags in_stage_mask)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResetEventCommand(in_event_ptr,
                                                   in_stage_mask) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdResetEvent(m_command_buffer,
                        in_event_ptr->get_event(),
                        in_stage_mask);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_reset_query_pool(Anvil::QueryPool* in_query_pool_ptr,
                                                       Anvil::QueryIndex in_start_query,
                                                       uint32_t          in_query_count)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResetQueryPoolCommand(in_query_pool_ptr,
                                                       in_start_query,
                                                       in_query_count) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdResetQueryPool(m_command_buffer,
                            in_query_pool_ptr->get_query_pool(),
                            in_start_query,
                            in_query_count);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_resolve_image(Anvil::Image*         in_src_image_ptr,
                                                    VkImageLayout         in_src_image_layout,
                                                    Anvil::Image*         in_dst_image_ptr,
                                                    VkImageLayout         in_dst_image_layout,
                                                    uint32_t              in_region_count,
                                                    const VkImageResolve* in_region_ptrs)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ResolveImageCommand(in_src_image_ptr,
                                                     in_src_image_layout,
                                                     in_dst_image_ptr,
                                                     in_dst_image_layout,
                                                     in_region_count,
                                                     in_region_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdResolveImage(m_command_buffer,
                          in_src_image_ptr->get_image(),
                          in_src_image_layout,
                          in_dst_image_ptr->get_image(),
                          in_dst_image_layout,
                          in_region_count,
                          in_region_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}


/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_blend_constants(const float in_blend_constants[4])
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetBlendConstantsCommand(in_blend_constants));
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetBlendConstants(m_command_buffer,
                               in_blend_constants);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_depth_bias(float in_depth_bias_constant_factor,
                                                     float in_depth_bias_clamp,
                                                     float in_slope_scaled_depth_bias)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetDepthBiasCommand(in_depth_bias_constant_factor,
                                                     in_depth_bias_clamp,
                                                     in_slope_scaled_depth_bias) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetDepthBias(m_command_buffer,
                          in_depth_bias_constant_factor,
                          in_depth_bias_clamp,
                          in_slope_scaled_depth_bias);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_depth_bounds(float in_min_depth_bounds,
                                                       float in_max_depth_bounds)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetDepthBoundsCommand(in_min_depth_bounds,
                                                       in_max_depth_bounds) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetDepthBounds(m_command_buffer,
                            in_min_depth_bounds,
                            in_max_depth_bounds);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_event(Anvil::Event*        in_event_ptr,
                                                VkPipelineStageFlags in_stage_mask)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetEventCommand(in_event_ptr,
                                                 in_stage_mask) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetEvent(m_command_buffer,
                      in_event_ptr->get_event(),
                      in_stage_mask);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_line_width(float in_line_width)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetLineWidthCommand(in_line_width) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetLineWidth(m_command_buffer,
                          in_line_width);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_scissor(uint32_t        in_first_scissor,
                                                  uint32_t        in_scissor_count,
                                                  const VkRect2D* in_scissor_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetScissorCommand(in_first_scissor,
                                                   in_scissor_count,
                                                   in_scissor_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetScissor(m_command_buffer,
                        in_first_scissor,
                        in_scissor_count,
                        in_scissor_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_compare_mask(VkStencilFaceFlags in_face_mask,
                                                               uint32_t           in_stencil_compare_mask)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilCompareMaskCommand(in_face_mask,
                                                              in_stencil_compare_mask) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetStencilCompareMask(m_command_buffer,
                                   in_face_mask,
                                   in_stencil_compare_mask);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_reference(VkStencilFaceFlags in_face_mask,
                                                            uint32_t           in_stencil_reference)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilReferenceCommand(in_face_mask,
                                                            in_stencil_reference) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetStencilReference(m_command_buffer,
                                 in_face_mask,
                                 in_stencil_reference);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_stencil_write_mask(VkStencilFaceFlags in_face_mask,
                                                             uint32_t           in_stencil_write_mask)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetStencilWriteMaskCommand(in_face_mask,
                                                            in_stencil_write_mask) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetStencilWriteMask(m_command_buffer,
                                 in_face_mask,
                                 in_stencil_write_mask);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_set_viewport(uint32_t          in_first_viewport,
                                                   uint32_t          in_viewport_count,
                                                   const VkViewport* in_viewport_ptrs)
{
    /* Note: Command supported inside and outside the renderpass. */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(SetViewportCommand(in_first_viewport,
                                                    in_viewport_count,
                                                    in_viewport_ptrs) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdSetViewport(m_command_buffer,
                         in_first_viewport,
                         in_viewport_count,
                         in_viewport_ptrs);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_update_buffer(Anvil::Buffer* in_dst_buffer_ptr,
                                                    VkDeviceSize   in_dst_offset,
                                                    VkDeviceSize   in_data_size,
                                                    const void*    in_data_ptr)
{
    bool result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(UpdateBufferCommand(in_dst_buffer_ptr,
                                                     in_dst_offset,
                                                     in_data_size,
                                                     in_data_ptr) );
        }
    }
    #endif


    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdUpdateBuffer(m_command_buffer,
                          in_dst_buffer_ptr->get_buffer(),
                          in_dst_offset,
                          in_data_size,
                          in_data_ptr);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_wait_events(uint32_t                   in_event_count,
                                                  Anvil::Event* const*       in_events,
                                                  VkPipelineStageFlags       in_src_stage_mask,
                                                  VkPipelineStageFlags       in_dst_stage_mask,
                                                  uint32_t                   in_memory_barrier_count,
                                                  const MemoryBarrier* const in_memory_barriers_ptr,
                                                  uint32_t                   in_buffer_memory_barrier_count,
                                                  const BufferBarrier* const in_buffer_memory_barriers_ptr,
                                                  uint32_t                   in_image_memory_barrier_count,
                                                  const ImageBarrier* const  in_image_memory_barriers_ptr)

{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    std::vector<VkEvent>               events            (in_event_count);
    std::vector<VkBufferMemoryBarrier> buffer_barriers_vk(in_buffer_memory_barrier_count);
    std::vector<VkImageMemoryBarrier>  image_barriers_vk (in_image_memory_barrier_count);
    std::vector<VkMemoryBarrier>       memory_barriers_vk(in_memory_barrier_count);
    bool                               result            (false);

    anvil_assert(in_event_count > 0); /* as per spec - easy to miss */

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(WaitEventsCommand(in_event_count,
                                                   in_events,
                                                   in_src_stage_mask,
                                                   in_dst_stage_mask,
                                                   in_memory_barrier_count,
                                                   in_memory_barriers_ptr,
                                                   in_buffer_memory_barrier_count,
                                                   in_buffer_memory_barriers_ptr,
                                                   in_image_memory_barrier_count,
                                                   in_image_memory_barriers_ptr) );
        }
    }
    #endif


    for (uint32_t n_event = 0;
                  n_event < in_event_count;
                ++n_event)
    {
        events.at(n_event) = in_events[n_event]->get_event();
    }

    for (uint32_t n_buffer_barrier = 0;
                  n_buffer_barrier < in_buffer_memory_barrier_count;
                ++n_buffer_barrier)
    {
        buffer_barriers_vk.at(n_buffer_barrier) = in_buffer_memory_barriers_ptr[n_buffer_barrier].get_barrier_vk();
    }

    for (uint32_t n_image_barrier = 0;
                  n_image_barrier < in_image_memory_barrier_count;
                ++n_image_barrier)
    {
        image_barriers_vk.at(n_image_barrier) = in_image_memory_barriers_ptr[n_image_barrier].get_barrier_vk();
    }

    for (uint32_t n_memory_barrier = 0;
                  n_memory_barrier < in_memory_barrier_count;
                ++n_memory_barrier)
    {
        memory_barriers_vk.at(n_memory_barrier) = in_memory_barriers_ptr[n_memory_barrier].get_barrier_vk();
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdWaitEvents(m_command_buffer,
                        in_event_count,
                        (in_event_count > 0) ? &events.at(0) : nullptr,
                        in_src_stage_mask,
                        in_dst_stage_mask,
                        in_memory_barrier_count,
                        (in_memory_barrier_count > 0) ? &memory_barriers_vk.at(0) : nullptr,
                        in_buffer_memory_barrier_count,
                        (in_buffer_memory_barrier_count > 0) ? &buffer_barriers_vk.at(0) : nullptr,
                        in_image_memory_barrier_count,
                        (in_image_memory_barrier_count > 0) ? &image_barriers_vk.at(0) : nullptr);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::record_write_timestamp(VkPipelineStageFlagBits in_pipeline_stage,
                                                      Anvil::QueryPool*       in_query_pool_ptr,
                                                      Anvil::QueryIndex       in_query_index)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    bool result = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(WriteTimestampCommand(in_pipeline_stage,
                                                       in_query_pool_ptr,
                                                       in_query_index) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdWriteTimestamp(m_command_buffer,
                            in_pipeline_stage,
                            in_query_pool_ptr->get_query_pool(),
                            in_query_index);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::reset(bool in_should_release_resources)
{
    bool     result    = false;
    VkResult result_vk;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        result_vk = vkResetCommandBuffer(m_command_buffer,
                                         (in_should_release_resources) ? VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT : 0u);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        clear_commands();
    }
    #endif

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::CommandBufferBase::stop_recording()
{
    bool     result     = false;
    VkResult result_vk;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        result_vk = vkEndCommandBuffer(m_command_buffer);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    if (!is_vk_call_successful(result_vk))
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    m_recording_in_progress = false;
    result                  = true;
end:
    return result;
}

/* Please see header for specification */
Anvil::PrimaryCommandBuffer::PrimaryCommandBuffer(const Anvil::BaseDevice* in_device_ptr,
                                                  Anvil::CommandPool*      in_parent_command_pool_ptr,
                                                  bool                     in_mt_safe)
    :CommandBufferBase(in_device_ptr,
                       in_parent_command_pool_ptr,
                       COMMAND_BUFFER_TYPE_PRIMARY,
                       in_mt_safe)
{
    VkCommandBufferAllocateInfo alloc_info;
    VkResult                    result_vk (VK_ERROR_INITIALIZATION_FAILED);

    ANVIL_REDUNDANT_VARIABLE(result_vk);

    alloc_info.commandBufferCount = 1;
    alloc_info.commandPool        = in_parent_command_pool_ptr->get_command_pool();
    alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.pNext              = nullptr;
    alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    in_parent_command_pool_ptr->lock();
    {
        result_vk = vkAllocateCommandBuffers(m_device_ptr->get_device_vk(),
                                            &alloc_info,
                                            &m_command_buffer);
    }
    in_parent_command_pool_ptr->unlock();

    anvil_assert_vk_call_succeeded(result_vk);
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_begin_render_pass(uint32_t            in_n_clear_values,
                                                           const VkClearValue* in_clear_value_ptrs,
                                                           Anvil::Framebuffer* in_fbo_ptr,
                                                           VkRect2D            in_render_area,
                                                           Anvil::RenderPass*  in_render_pass_ptr,
                                                           VkSubpassContents   in_contents)
{
    Anvil::StructChainer<VkRenderPassBeginInfo> render_pass_begin_info_chain;
    bool                                        result = false;

    if (m_is_renderpass_active)
    {
        anvil_assert(!m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(BeginRenderPassCommand(in_n_clear_values,
                                                        in_clear_value_ptrs,
                                                        in_fbo_ptr,
                                                        in_render_area,
                                                        in_render_pass_ptr,
                                                        in_contents) );
        }
    }
    #endif

    {
        VkRenderPassBeginInfo render_pass_begin_info;

        render_pass_begin_info.clearValueCount = in_n_clear_values;
        render_pass_begin_info.framebuffer     = in_fbo_ptr->get_framebuffer(in_render_pass_ptr);
        render_pass_begin_info.pClearValues    = in_clear_value_ptrs;
        render_pass_begin_info.pNext           = nullptr;
        render_pass_begin_info.renderArea      = in_render_area;
        render_pass_begin_info.renderPass      = in_render_pass_ptr->get_render_pass();
        render_pass_begin_info.sType           = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

        render_pass_begin_info_chain.append_struct(render_pass_begin_info);
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        auto chain_ptr = render_pass_begin_info_chain.create_chain();

        vkCmdBeginRenderPass(m_command_buffer,
                             chain_ptr->get_root_struct(),
                             in_contents);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    m_is_renderpass_active = true;
    result                 = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_end_render_pass()
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(EndRenderPassCommand() );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdEndRenderPass(m_command_buffer);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    m_is_renderpass_active = false;
    result                 = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_execute_commands(uint32_t                        in_cmd_buffers_count,
                                                          Anvil::SecondaryCommandBuffer** in_cmd_buffer_ptrs)
{
    /* NOTE: The command can be executed both inside and outside a renderpass */
    auto cmd_buffers = std::vector<VkCommandBuffer>(in_cmd_buffers_count);
    bool result      = false;

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(ExecuteCommandsCommand(in_cmd_buffers_count,
                                                        in_cmd_buffer_ptrs) );
        }
    }
    #endif

    for (uint32_t n_cmd_buffer = 0;
                  n_cmd_buffer < in_cmd_buffers_count;
                ++n_cmd_buffer)
    {
        cmd_buffers.at(n_cmd_buffer) = in_cmd_buffer_ptrs[n_cmd_buffer]->get_command_buffer();
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdExecuteCommands(m_command_buffer,
                             in_cmd_buffers_count,
                             (in_cmd_buffers_count > 0) ? &cmd_buffers.at(0) : nullptr);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::record_next_subpass(VkSubpassContents in_contents)
{
    bool result = false;

    if (!m_is_renderpass_active)
    {
        anvil_assert(m_is_renderpass_active);

        goto end;
    }

    if (!m_recording_in_progress)
    {
        anvil_assert(m_recording_in_progress);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        if (!m_command_stashing_disabled)
        {
            m_commands.push_back(NextSubpassCommand(in_contents) );
        }
    }
    #endif

    m_parent_command_pool_ptr->lock();
    lock();
    {
        vkCmdNextSubpass(m_command_buffer,
                         in_contents);
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    result = true;
end:
    return result;
}

/* Please see header for specification */
bool Anvil::PrimaryCommandBuffer::start_recording(bool in_one_time_submit,
                                                  bool in_simultaneous_use_allowed)
{
    bool                                           result         (false);
    VkResult                                       result_vk;
    Anvil::StructChainer<VkCommandBufferBeginInfo> struct_chainer;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    {
        VkCommandBufferBeginInfo command_buffer_begin_info;

        command_buffer_begin_info.flags            = ((in_one_time_submit)          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT  : 0u) |
                                                     ((in_simultaneous_use_allowed) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT : 0u);
        command_buffer_begin_info.pNext            = nullptr;
        command_buffer_begin_info.pInheritanceInfo = nullptr;  /* Only relevant for secondary-level command buffers */
        command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        struct_chainer.append_struct(command_buffer_begin_info);
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        auto chain_ptr = struct_chainer.create_chain();

        result_vk = vkBeginCommandBuffer(m_command_buffer,
                                         chain_ptr->get_root_struct() );
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        /* vkBeginCommandBuffer() implicitly resets all commands recorded previously */
        clear_commands();
    }
    #endif

    m_recording_in_progress = true;
    result                  = true;

end:
    return result;
}


/* Please see header for specification */
Anvil::SecondaryCommandBuffer::SecondaryCommandBuffer(const Anvil::BaseDevice* in_device_ptr,
                                                      Anvil::CommandPool*      in_parent_command_pool_ptr,
                                                      bool                     in_mt_safe)
    :CommandBufferBase(in_device_ptr,
                       in_parent_command_pool_ptr,
                       COMMAND_BUFFER_TYPE_SECONDARY,
                       in_mt_safe)
{
    VkCommandBufferAllocateInfo command_buffer_alloc_info;
    VkResult                    result_vk        (VK_ERROR_INITIALIZATION_FAILED);

    ANVIL_REDUNDANT_VARIABLE(result_vk);

    command_buffer_alloc_info.commandBufferCount = 1;
    command_buffer_alloc_info.commandPool        = in_parent_command_pool_ptr->get_command_pool();
    command_buffer_alloc_info.level              = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    command_buffer_alloc_info.pNext              = nullptr;
    command_buffer_alloc_info.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;

    in_parent_command_pool_ptr->lock();
    {
        result_vk = vkAllocateCommandBuffers(m_device_ptr->get_device_vk(),
                                            &command_buffer_alloc_info,
                                            &m_command_buffer);
    }
    in_parent_command_pool_ptr->unlock();

    anvil_assert_vk_call_succeeded(result_vk);
}

/* Please see header for specification */
bool Anvil::SecondaryCommandBuffer::start_recording(bool                                in_one_time_submit,
                                                    bool                                in_simultaneous_use_allowed,
                                                    bool                                in_renderpass_usage_only,
                                                    Framebuffer*                        in_framebuffer_ptr,
                                                    RenderPass*                         in_render_pass_ptr,
                                                    SubPassID                           in_subpass_id,
                                                    OcclusionQuerySupportScope          in_required_occlusion_query_support_scope,
                                                    bool                                in_occlusion_query_used_by_primary_command_buffer,
                                                    VkQueryPipelineStatisticFlags       in_required_pipeline_statistics_scope)
{
    VkCommandBufferInheritanceInfo                 command_buffer_inheritance_info;
    bool                                           result                         (false);
    VkResult                                       result_vk;
    Anvil::StructChainer<VkCommandBufferBeginInfo> struct_chainer;

    if (m_recording_in_progress)
    {
        anvil_assert(!m_recording_in_progress);

        goto end;
    }

    {
        VkCommandBufferBeginInfo command_buffer_begin_info;

        command_buffer_begin_info.flags            = ((in_one_time_submit)          ? VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT      : 0u) |
                                                     ((in_simultaneous_use_allowed) ? VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT     : 0u) |
                                                     ((in_renderpass_usage_only)    ? VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT : 0u);
        command_buffer_begin_info.pInheritanceInfo = &command_buffer_inheritance_info;
        command_buffer_begin_info.pNext            = nullptr;
        command_buffer_begin_info.sType            = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        struct_chainer.append_struct(command_buffer_begin_info);
    }

    {
        command_buffer_inheritance_info.framebuffer          = (in_framebuffer_ptr                                != nullptr) ? in_framebuffer_ptr->get_framebuffer(in_render_pass_ptr) : VK_NULL_HANDLE;
        command_buffer_inheritance_info.occlusionQueryEnable = (in_occlusion_query_used_by_primary_command_buffer)            ? VK_TRUE                                                 : VK_FALSE;
        command_buffer_inheritance_info.pipelineStatistics   = in_required_pipeline_statistics_scope;
        command_buffer_inheritance_info.pNext                = nullptr;
        command_buffer_inheritance_info.queryFlags           = (in_occlusion_query_used_by_primary_command_buffer                                                    &&
                                                                in_required_occlusion_query_support_scope         == OCCLUSION_QUERY_SUPPORT_SCOPE_REQUIRED_PRECISE) ? VK_QUERY_CONTROL_PRECISE_BIT          : 0u;
        command_buffer_inheritance_info.renderPass           = (in_render_pass_ptr                                != nullptr)                                        ? in_render_pass_ptr->get_render_pass() : VK_NULL_HANDLE;
        command_buffer_inheritance_info.sType                = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
        command_buffer_inheritance_info.subpass              = in_subpass_id;
    }

    m_parent_command_pool_ptr->lock();
    lock();
    {
        auto chain_ptr = struct_chainer.create_chain();

        result_vk = vkBeginCommandBuffer(m_command_buffer,
                                         chain_ptr->get_root_struct() );
    }
    unlock();
    m_parent_command_pool_ptr->unlock();

    if (!is_vk_call_successful(result_vk) )
    {
        anvil_assert_vk_call_succeeded(result_vk);

        goto end;
    }

    #ifdef STORE_COMMAND_BUFFER_COMMANDS
    {
        /* vkBeginCommandBuffer() implicitly resets all commands recorded previously */
        clear_commands();
    }
    #endif

    m_is_renderpass_active = in_renderpass_usage_only;

    m_recording_in_progress = true;
    result                  = true;

end:
    return result;
}
