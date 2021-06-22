//
// Using textures with VulkanWindow
//

#include	"VulkanWindow.h"
#include	"Buffer.h"
#include	"DescriptorSet.h"
#include	"Texture.h"
#include	"stb_image_aug.h"
#include	"BasicMesh.h"

struct UniformBufferObject 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

void	loadDds ( Device& device, Texture& texture, Data&& data );

class	TestWindow : public VulkanWindow
{
	std::vector<VkCommandBuffer>	commandBuffers;
	GraphicsPipeline				pipeline;
	Renderpass						renderPass;
	std::vector<Buffer>				uniformBuffers;
	DescriptorPool					descriptorPool;
	std::vector<DescriptorSet> 		descriptorSets;
	Image							image;
	Texture							texture;
	Sampler							sampler;
	BasicMesh                     * mesh = nullptr;
	glm::vec3						eye  = glm::vec3(2.7f, 2.7f, 2.7f);
	
public:
	TestWindow ( int w, int h, const std::string& t ) : VulkanWindow ( w, h, t, true )
	{
		//mesh = createKnot ( device, 1, 4, 120, 30 );
		mesh = createBox  ( device, glm::vec3 ( -1, -1, -1 ), glm::vec3 ( 2, 2, 2 ) );

		sampler.create ( device );		// use default optiona
		
		//loadDds  ( device, texture, Data ( "textures/Fieldstone.dds" ) );
		loadDds  ( device, texture, Data ( "textures/A.dds" ) );
		//loadDds  ( device, texture, Data ( "textures/Default_albedo.dds" ) );
		//loadDds  ( device, texture, Data ( "textures/Habib_House_Med.dds" ) );

		createPipelines ();
	}

	~TestWindow ()
	{
		delete mesh;
	}
	
	void	createUniformBuffers ()
	{
		VkDeviceSize bufferSize = sizeof ( UniformBufferObject );

		uniformBuffers.resize ( swapChain.imageCount() );
		
		for ( size_t i = 0; i < swapChain.imageCount (); i++ )
			uniformBuffers [i].create ( device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
	}

	void	freeUniformBuffers ()
	{
		for ( size_t i = 0; i < swapChain.imageCount (); i++ )
			uniformBuffers [i].clean ();
	}

	void	createDescriptorSets ()
	{
		descriptorSets.resize ( swapChain.imageCount () );

		for ( uint32_t i = 0; i < swapChain.imageCount (); i++ )
		{
			descriptorSets  [i]
				.setLayout ( device, pipeline.getDescLayout (), descriptorPool )
				.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [i], 0, sizeof ( UniformBufferObject ) )
				.addImage  ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, texture, sampler )
				.create    ();
		}
	}
	
	virtual	void	createPipelines () override 
	{
		createUniformBuffers ();

		descriptorPool
			.setMaxSets            ( swapChain.imageCount () )
			.setUniformBufferCount ( swapChain.imageCount () )
			.setImageCount         ( swapChain.imageCount () )
			.create                ( device                  );
		
			// current app code
		renderPass.addAttachment   ( swapChain.getFormat (),                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
				  .addAttachment   ( depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
				  .addSubpass      ( 0 )
				  .addDepthSubpass ( 1 )
		          .create          ( device );
		
		mesh->setVertexAttrs ( pipeline )
				.setDevice ( device )
				.setVertexShader   ( "shaders/shader.7.vert.spv" )
				.setFragmentShader ( "shaders/shader.7.frag.spv" )
				.setSize           ( swapChain.getExtent ().width, swapChain.getExtent ().height )
				.addVertexBinding  ( sizeof ( BasicVertex ), 0, VK_VERTEX_INPUT_RATE_VERTEX )
//				.addDescriptor     ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT )
//				.addDescriptor     ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )
				.addDescLayout     ( 0,  DescSetLayout ()
					.add ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT )
					.add ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT ) )
				.setCullMode       ( VK_CULL_MODE_BACK_BIT )
				.setFrontFace      ( VK_FRONT_FACE_COUNTER_CLOCKWISE )
				.setDepthTest      ( true )
				.setDepthWrite     ( true )
				.create            ( renderPass );
			

				// create before command buffers
		swapChain.createFramebuffers ( renderPass.getHandle (), depthTexture.getImageView () );

		createDescriptorSets ();

		createCommandBuffers         ( renderPass.getHandle (), pipeline.getHandle () );
	}

	virtual	void	freePipelines () override
	{
		vkFreeCommandBuffers ( device.getDevice (), device.getCommandPool (), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data () );

		commandBuffers.clear ();
		
		pipeline.clean   ();
		renderPass.clean ();
		freeUniformBuffers ();
		descriptorSets.clear ();

		descriptorPool.clean ();
	}
	
	virtual	void	submit ( uint32_t imageIndex ) override 
	{
		updateUniformBuffer ( imageIndex );

		VkSubmitInfo			submitInfo          = {};
		VkSemaphore				waitSemaphores   [] = { swapChain.currentAvailableSemaphore () };
		VkPipelineStageFlags	waitStages       [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore				signalSemaphores [] = { swapChain.currentRenderFinishedSemaphore () };
		VkFence					currentFence        = swapChain.currentInFlightFence ();

		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = waitSemaphores;
		submitInfo.pWaitDstStageMask    = waitStages;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &commandBuffers [imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores;

		vkResetFences ( device.getDevice (), 1, &currentFence );

		if ( vkQueueSubmit ( device.getGraphicsQueue (), 1, &submitInfo, currentFence ) != VK_SUCCESS )
			fatal () << "failed to submit draw command buffer!";
	}

	void	createCommandBuffers ( VkRenderPass renderPass, VkPipeline pipeline )			// size - swapChain.framebuffers.size ()
	{
		auto	framebuffers = swapChain.getFramebuffers ();

		commandBuffers.resize ( framebuffers.size () );

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool        = device.getCommandPool ();
		allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = (uint32_t) framebuffers.size ();

		if ( vkAllocateCommandBuffers ( device.getDevice (), &allocInfo, commandBuffers.data () ) != VK_SUCCESS )
			fatal () << "VulkanWindow: failed to allocate command buffers!";

		for ( size_t i = 0; i < commandBuffers.size(); i++ )
		{
			VkCommandBufferBeginInfo beginInfo = {};
			beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

			if ( vkBeginCommandBuffer ( commandBuffers[i], &beginInfo ) != VK_SUCCESS )
				fatal () << "VulkanWindow: failed to begin recording command buffer!";

			VkRenderPassBeginInfo	renderPassInfo  = {};
			VkClearValue			clearValues [2] = {};
			
			clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
			clearValues[1].depthStencil = {1.0f, 0};

			renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			renderPassInfo.renderPass        = renderPass;
			renderPassInfo.framebuffer       = framebuffers [i];
			renderPassInfo.renderArea.offset = {0, 0};
			renderPassInfo.renderArea.extent = swapChain.getExtent ();
			renderPassInfo.clearValueCount   = 2;
			renderPassInfo.pClearValues      = clearValues;

			vkCmdBeginRenderPass  ( commandBuffers [i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );

			vkCmdBindPipeline ( commandBuffers [i], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline );

			VkDeviceSize	offsets       [] = { 0 };
			VkDescriptorSet	descSet          = descriptorSets[i].getHandle ();

			vkCmdBindDescriptorSets ( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.getLayout (), 0, 1, &descSet, 0, nullptr );

			mesh->render  ( commandBuffers [i] );

			vkCmdEndRenderPass     ( commandBuffers [i] );

			if ( vkEndCommandBuffer ( commandBuffers [i] ) != VK_SUCCESS )
				fatal () << "VulkanWindow: failed to record command buffer!";
		}
	}

	void updateUniformBuffer ( uint32_t currentImage )
	{
		float				time = (float)getTime ();
		UniformBufferObject ubo  = {};

		ubo.model       = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view        = glm::lookAt(eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj        = glm::perspective(glm::radians(45.0f), getWidth () / (float) getHeight (), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		uniformBuffers [currentImage].copy ( &ubo, sizeof ( ubo ) );
	}
	

	virtual	void	keyTyped ( int key, int scancode, int action, int mods ) override
	{
		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
			glfwSetWindowShouldClose ( window, GLFW_TRUE );		
	}

};

int main ( int argc, const char * argv [] ) 
{
	TestWindow	win ( 1000, 900, "Test window" );

	return win.run ();
}
