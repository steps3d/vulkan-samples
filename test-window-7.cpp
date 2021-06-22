//
// Using textures with VulkanWindow
//

#include	"VulkanWindow.h"
#include	"Buffer.h"
#include	"DescriptorSet.h"
#include	"Texture.h"
#include	"stb_image_aug.h"

struct UniformBufferObject 
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

struct Vertex 
{
    glm::vec3 pos;
    glm::vec3 color;
    glm::vec2 texCoord;
};

const std::vector<Vertex> vertices = 
{
    {{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f},  {1.0f, 0.0f}},
    {{0.5f, -0.5f,  0.0f}, {0.0f, 1.0f, 0.0f},  {0.0f, 0.0f}},
    {{0.5f, 0.5f,   0.0f},  {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f,  0.0f}, {1.0f, 1.0f, 1.0f},  {1.0f, 1.0f}},

    {{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f,  -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f,   -0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f,  -0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
};

const std::vector<uint16_t> indices = 
{
    0, 1, 2, 2, 3, 0,
    4, 5, 6, 6, 7, 4
};

class	TestWindow : public VulkanWindow
{
	std::vector<VkCommandBuffer>	commandBuffers;
	GraphicsPipeline				pipeline;
	Renderpass						renderPass;
	Buffer							vertexBuffer;
	Buffer							indexBuffer;
	std::vector<Buffer>				uniformBuffers;
	DescriptorPool					descriptorPool;
	std::vector<DescriptorSet> 		descriptorSets;
	Image							image;
	Texture							texture;
	Sampler							sampler;
	Texture							depthTexture;

	
public:
	TestWindow ( int w, int h, const std::string& t ) : VulkanWindow ( w, h, t, false )
	{
		size_t	verticesSize = sizeof(vertices[0]) * vertices.size();
		size_t	indicesSize  = sizeof(indices[0])  * indices.size();

		createBuffer ( vertexBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, verticesSize, vertices.data () );
		createBuffer ( indexBuffer,  VK_BUFFER_USAGE_INDEX_BUFFER_BIT,  indicesSize,  indices.data  () );
				
		sampler.create ( device );		// use default optiona
		
		createTexture   ();
		createPipelines ();
	}

	void createTexture ()
	{
		int				texWidth, texHeight, texChannels;
		stbi_uc       * pixels    = stbi_load ( "textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha );
		VkDeviceSize	imageSize = texWidth * texHeight * 4;
		uint32_t		mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(texWidth, texHeight)))) + 1;

		if ( !pixels )
			fatal () << "failed to load texture image!";

		Buffer	stagingBuffer;
		
		stagingBuffer.create ( device, imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		stagingBuffer.copy   ( pixels, imageSize );

		stbi_image_free ( pixels );

			// TRANSFER_SRC for mipmap calculations via vkCmdBlitImage
		texture.create ( device, texWidth, texHeight, 1, mipLevels, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );

		{
			SingleTimeCommand	cmd ( device );
			
			texture.getImage ().transitionLayout  ( cmd, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
			texture.getImage ().copyFromBuffer    ( cmd, stagingBuffer, texWidth, texHeight, 1 );
//			texture.getImage ().transitionLayout  ( cmd, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL );
			texture.            generateMipmaps   ( cmd, VK_FORMAT_R8G8B8A8_UNORM, texWidth, texHeight, mipLevels );
		}
	}

			// create buffer and fill using staging buffer
	void	createBuffer ( Buffer& buffer, uint32_t usage, size_t size, const void * data )
	{
		Buffer				stagingBuffer;
		SingleTimeCommand	cmd ( device );

					// use staging buffer to copy data to GPU-local memory
        stagingBuffer.create ( device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		stagingBuffer.copy   ( data, size );
		buffer.create        ( device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
        //copyBuffer           ( stagingBuffer, buffer, size );
		buffer.copyBuffer ( cmd, stagingBuffer, size );

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
				// create depth texture
		depthTexture.create ( device, getWidth (), getHeight (), 1, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_TILING_OPTIMAL, 
							  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		
		{
			SingleTimeCommand	cmd ( device );
			
			depthTexture.getImage ().transitionLayout ( cmd, depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL ); 
		}
		
		createUniformBuffers ();

//		descriptorPool.create ( device, swapChain.imageCount (), swapChain.imageCount (), swapChain.imageCount () );		
		descriptorPool
			.setMaxSets            ( swapChain.imageCount () )
			.setUniformBufferCount ( swapChain.imageCount () )
			.setImageCount         ( swapChain.imageCount () )
			.create                ( device );		

			// current app code
		renderPass.addAttachment   ( swapChain.getFormat (),                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
				  .addAttachment   ( depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
				  .addSubpass      ( 0 )
				  .addDepthSubpass ( 1 )
		          .create          ( device );
		
		pipeline.setDevice ( device )
				.setVertexShader   ( "shaders/shader.4.vert.spv" )
				.setFragmentShader ( "shaders/shader.4.frag.spv" )
				.setSize           ( swapChain.getExtent ().width, swapChain.getExtent ().height )
				.addVertexBinding  ( sizeof ( Vertex ), 0, VK_VERTEX_INPUT_RATE_VERTEX )
				.addVertexAttr     ( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, pos) )
				.addVertexAttr     ( 0, 1, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color) )
				.addVertexAttr     ( 0, 2, VK_FORMAT_R32G32_SFLOAT,    offsetof(Vertex, texCoord) )
//				.addDescriptor     ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT )
//				.addDescriptor     ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )
				.addDescLayout     ( 0, DescSetLayout ()
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
		depthTexture.clean ();
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

			VkBuffer		vertexBuffers [] = { vertexBuffer.getHandle () };
			VkDeviceSize	offsets       [] = { 0 };
			VkDescriptorSet	descSet          = descriptorSets[i].getHandle ();

			vkCmdBindVertexBuffers ( commandBuffers[i], 0, 1, vertexBuffers, offsets );
			vkCmdBindIndexBuffer   ( commandBuffers[i], indexBuffer.getHandle (), 0, VK_INDEX_TYPE_UINT16 );

			vkCmdBindDescriptorSets ( commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, this->pipeline.getLayout (), 0, 1, &descSet, 0, nullptr );

			vkCmdDrawIndexed       ( commandBuffers[i], static_cast<uint32_t>(indices.size()), 1, 0, 0, 0 );

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
		ubo.view        = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
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
	TestWindow	win ( 800, 600, "Test window" );

	return win.run ();
}
