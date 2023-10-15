#include "RenderPass.h"

namespace SmolderingEngine
{
    void SpecifySubpassDescriptions(std::vector<SubpassParameters> const& _subpassParameters, std::vector<VkSubpassDescription>& _subpassDescriptions)
    {
        _subpassDescriptions.clear();

        for (auto& subpassDescription : _subpassParameters)
        {
            _subpassDescriptions.push_back
            ({
                0,                                                                      // VkSubpassDescriptionFlags        flags
                subpassDescription.pipelineType,                                       // VkPipelineBindPoint              pipelineBindPoint
                static_cast<uint32_t>(subpassDescription.inputAttachments.size()),     // uint32_t                         inputAttachmentCount
                subpassDescription.inputAttachments.data(),                            // const VkAttachmentReference    * pInputAttachments
                static_cast<uint32_t>(subpassDescription.colorAttachments.size()),     // uint32_t                         colorAttachmentCount
                subpassDescription.colorAttachments.data(),                            // const VkAttachmentReference    * pColorAttachments
                subpassDescription.resolveAttachments.data(),                          // const VkAttachmentReference    * pResolveAttachments
                subpassDescription.depthStencilAttachment,                             // const VkAttachmentReference    * pDepthStencilAttachment
                static_cast<uint32_t>(subpassDescription.preserveAttachments.size()),  // uint32_t                         preserveAttachmentCount
                subpassDescription.preserveAttachments.data()                          // const uint32_t                 * pPreserveAttachments
            });
        }
    }

    bool CreateRenderPass(VkDevice _logicalDevice, std::vector<VkAttachmentDescription> const& _attachmentsDescriptions, std::vector<SubpassParameters> const& _subpassParameters,
        std::vector<VkSubpassDependency> const& _subpassDependencies, VkRenderPass& _renderPass)
	{
        //SpecifyAttachmentsDescriptions(_attachmentsDescriptions);

        std::vector<VkSubpassDescription> subpassDescriptions;
        SpecifySubpassDescriptions(_subpassParameters, subpassDescriptions);

        //SpecifyDependenciesBetweenSubpasses(_subpassDependencies);

        VkRenderPassCreateInfo render_pass_create_info = {
          VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,                // VkStructureType                    sType
          nullptr,                                                  // const void                       * pNext
          0,                                                        // VkRenderPassCreateFlags            flags
          static_cast<uint32_t>(_attachmentsDescriptions.size()),   // uint32_t                           attachmentCount
          _attachmentsDescriptions.data(),                          // const VkAttachmentDescription    * pAttachments
          static_cast<uint32_t>(subpassDescriptions.size()),        // uint32_t                           subpassCount
          subpassDescriptions.data(),                               // const VkSubpassDescription       * pSubpasses
          static_cast<uint32_t>(_subpassDependencies.size()),       // uint32_t                           dependencyCount
          _subpassDependencies.data()                               // const VkSubpassDependency        * pDependencies
        };

        if (vkCreateRenderPass(_logicalDevice, &render_pass_create_info, nullptr, &_renderPass) != VK_SUCCESS)
        {
            std::cout << "Could not create a render pass." << std::endl;
            return false;
        }

        return true;
	}
};