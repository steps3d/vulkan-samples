#include	"VulkanWindow.h"
#include	"Buffer.h"
#include	"DescriptorSet.h"
#include	"Texture.h"
#include	"stb_image_aug.h"
#include	"BasicMesh.h"
#include	"TgaImage.h"
#include	"Semaphore.h"
#include	"CommandPool.h"

struct UniformBufferObject 		// for render pipeline
{
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

class	TestWindow : public VulkanWindow
{
	std::vector<VkCommandBuffer>	commandBuffers;
	GraphicsPipeline				graphicsPipeline;
	ComputePipeline					computePipeline;
	Renderpass						renderPass;
	std::vector<Buffer>				uniformBuffers;
	DescriptorPool					descriptorPool;
	std::vector<DescriptorSet> 		descriptorSets;
	Buffer							posBuffer;
	Buffer							velBuffer;
	DescriptorSet					computeDescriptorSet;
	VkCommandBuffer					computeCommandBuffer = VK_NULL_HANDLE;
	Semaphore						computeSemaphore, graphicsSemaphore;
	CommandPool						computeCommandPool;
	size_t							n;
	size_t							numParticles;
	float							t     = 0;				// current time in seconds
	float							zNear = 0.1f;
	float							zFar  = 100.0f;	
//	glm::vec3						eye   = glm::vec3 ( 4.0f, 4.0f, 4.0f );
	glm::vec3						eye   = glm::vec3 ( -0.5, 0.5, 15 );
	std::vector<Fence>				fences;

public:
	TestWindow ( int w, int h, const std::string& t ) : VulkanWindow ( w, h, t, true )
	{
		computeCommandPool.create ( device, false, VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT );
		graphicsSemaphore.create  ( device );
		computeSemaphore.create   ( device );
		computeSemaphore.signal   ( device.getGraphicsQueue () );
		
		initParticles   ( 64 );
		createPipelines ();
	}

	~TestWindow () {}

	virtual	void	drawFrame () override
	{
		VulkanWindow::drawFrame ();

		VkSubmitInfo			submitInfo                 = {};
		VkPipelineStageFlags	computeWaitStages          = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
		VkSemaphore				computeWaitSemaphores   [] = { graphicsSemaphore.getHandle () };
		VkSemaphore				computeSignalSemaphores [] = { computeSemaphore.getHandle  () };

		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = computeWaitSemaphores;
		submitInfo.pWaitDstStageMask    = &computeWaitStages;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &computeCommandBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = computeSignalSemaphores;

		if ( vkQueueSubmit ( device.getComputeQueue (), 1, &submitInfo, fences [currentImage].getHandle () ) != VK_SUCCESS )
			fatal () << "failed to submit draw command buffer!";

		fences [currentImage].wait  ( UINT64_MAX );
		fences [currentImage].reset ();

	}

	void	createDescriptorSets ()
	{
		descriptorSets.resize ( swapChain.imageCount () );

		for ( uint32_t i = 0; i < swapChain.imageCount (); i++ )
		{
			descriptorSets  [i]
				.setLayout ( device, graphicsPipeline.getDescLayout (), descriptorPool )
				.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [i], 0, sizeof ( UniformBufferObject ) )
				.create    ();
		}
		
		computeDescriptorSet
			.setLayout ( device, computePipeline.getDescLayout (), descriptorPool )
			.addBuffer ( 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, posBuffer )
			.addBuffer ( 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, velBuffer )
			.create    ();

	}
	
	virtual	void	createPipelines () override 
	{
		VkDeviceSize bufferSize = sizeof ( UniformBufferObject );

		uniformBuffers.resize ( swapChain.imageCount() );

		for ( size_t i = 0; i < swapChain.imageCount (); i++ )
			uniformBuffers [i].create ( device, bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );


		descriptorPool
			.setMaxSets            ( 3+swapChain.imageCount () )
			.setUniformBufferCount ( 3+swapChain.imageCount () )
			.setImageCount         ( 3+swapChain.imageCount () )
			.setStorageBufferCount ( 2 )
			.create                ( device );		
		
			// current app code
		renderPass.addAttachment   ( swapChain.getFormat (),                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
				  .addAttachment   ( depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
				  .addSubpass      ( 0 )
				  .addDepthSubpass ( 1 )
				  .create          ( device );
		
		graphicsPipeline
				.setDevice         ( device )
				.addVertexAttr     ( 0, 0, VK_FORMAT_R32G32B32_SFLOAT, 0 )
				.setVertexShader   ( "shaders/particles-render.vert.spv" )
				.setFragmentShader ( "shaders/particles-render.frag.spv" )
				.setSize           ( swapChain.getExtent ().width, swapChain.getExtent ().height )
				.addVertexBinding  ( sizeof ( BasicVertex ), 0, VK_VERTEX_INPUT_RATE_VERTEX )
//				.addDescriptor     ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT )
				.addDescLayout     ( 0, DescSetLayout ()
							.add ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT ) )
				.setTopology       ( VK_PRIMITIVE_TOPOLOGY_POINT_LIST )
				.setCullMode       ( VK_CULL_MODE_NONE                )
				.setFrontFace      ( VK_FRONT_FACE_COUNTER_CLOCKWISE  )
				.setDepthTest      ( true )
				.setDepthWrite     ( true )
				.create            ( renderPass );
			
		computePipeline
			.setDevice     ( device )
			.setShader     ( "shaders/particles.comp.spv" )
			.addDescriptor ( 0, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT )
			.addDescriptor ( 1, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT )
			.create        ();
		
				// create before command buffers
		swapChain.createFramebuffers ( renderPass.getHandle (), depthTexture.getImageView () );

		createDescriptorSets       ();
		createCommandBuffers       ( renderPass.getHandle (), graphicsPipeline.getHandle () );
		createComputeCommandBuffer ();
		
		fences.resize ( swapChain.imageCount () );

		for ( auto& f : fences )
			f.create ( device );
	}

	virtual	void	freePipelines () override
	{
		vkFreeCommandBuffers ( device.getDevice (), device.getCommandPool (), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data () );

		for ( size_t i = 0; i < swapChain.imageCount (); i++ )
			uniformBuffers [i].clean ();

		commandBuffers.clear   ();
		graphicsPipeline.clean ();
		computePipeline.clean  ();
		renderPass.clean       ();
		descriptorSets.clear   ();
		descriptorPool.clean   ();
	}
	
	virtual	void	submit ( uint32_t imageIndex ) override 
	{
		updateUniformBuffer ( imageIndex );

		VkSubmitInfo			submitInfo                  = {};
		VkSemaphore				waitSemaphores           [] = { swapChain.currentAvailableSemaphore (), computeSemaphore.getHandle () };
		VkPipelineStageFlags	graphicsWaitStages       [] = { VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore				graphicsSignalSemaphores [] = { swapChain.currentRenderFinishedSemaphore (), graphicsSemaphore.getHandle () };
		VkFence					currentFence                = swapChain.currentInFlightFence ();

		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount   = 2;
		submitInfo.pWaitSemaphores      = waitSemaphores;
		submitInfo.pWaitDstStageMask    = graphicsWaitStages;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &commandBuffers [imageIndex];
		submitInfo.signalSemaphoreCount = 2;
		submitInfo.pSignalSemaphores    = graphicsSignalSemaphores;

		vkResetFences ( device.getDevice (), 1, &currentFence );

		if ( vkQueueSubmit ( device.getGraphicsQueue (), 1, &submitInfo, currentFence ) != VK_SUCCESS )
			fatal () << "failed to submit draw command buffer!";
	}

	void	createCommandBuffers ( VkRenderPass renderPass, VkPipeline pipeline )
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

			VkBuffer		vertexBuffers [] = { posBuffer.getHandle () };
			VkDeviceSize	offsets       [] = { 0 };
			VkDescriptorSet	descSet          = descriptorSets [i].getHandle ();

			vkCmdBindDescriptorSets ( commandBuffers [i], VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline.getLayout (), 0, 1, &descSet, 0, nullptr );
			vkCmdBindVertexBuffers  ( commandBuffers [i], 0, 1, vertexBuffers, offsets );
			vkCmdDraw               ( commandBuffers [i], (uint32_t)numParticles, 1, 0, 0 );
			vkCmdEndRenderPass      ( commandBuffers [i] );

			if ( vkEndCommandBuffer ( commandBuffers [i] ) != VK_SUCCESS )
				fatal () << "VulkanWindow: failed to record command buffer!";
		}
	}

	void	createComputeCommandBuffer ()
	{
		computeCommandBuffer = computeCommandPool.alloc ();
		
		VkCommandBufferBeginInfo beginInfo = {};

		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

		if ( vkBeginCommandBuffer ( computeCommandBuffer, &beginInfo ) != VK_SUCCESS )
			fatal () << "Cannot begin compute command buffer" << Log::endl;

			// Add memory barrier to ensure that the (graphics) vertex shader has fetched 
			// attributes before compute starts to write to the buffer
		if ( device.getGraphicsQueue () != device.getComputeQueue () )
		{
			transitionBuffer ( posBuffer, 0, VK_ACCESS_SHADER_WRITE_BIT, device.getGraphicsFamilyIndex (), device.getComputeFamilyIndex (), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT );
			transitionBuffer ( velBuffer, 0, VK_ACCESS_SHADER_WRITE_BIT, device.getGraphicsFamilyIndex (), device.getComputeFamilyIndex (), VK_PIPELINE_STAGE_VERTEX_INPUT_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT );
		}

			// Dispatch the compute job
		VkDescriptorSet	descSet = computeDescriptorSet.getHandle ();

		vkCmdBindPipeline       ( computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.getHandle () );
		vkCmdBindDescriptorSets ( computeCommandBuffer, VK_PIPELINE_BIND_POINT_COMPUTE, computePipeline.getLayout (), 0, 1, &descSet, 0, nullptr );
		vkCmdDispatch           ( computeCommandBuffer, (uint32_t) (numParticles + 1023) / 1024, 1, 1 );

			// Add barrier to ensure that compute shader has finished writing to the buffer
			// Without this the (rendering) vertex shader may display incomplete results (partial data from last frame) 
		if ( device.getGraphicsQueue () != device.getComputeQueue () )
		{
			transitionBuffer ( posBuffer, VK_ACCESS_SHADER_WRITE_BIT, 0, device.getComputeFamilyIndex (), device.getGraphicsFamilyIndex (), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT );
			transitionBuffer ( velBuffer, VK_ACCESS_SHADER_WRITE_BIT, 0, device.getComputeFamilyIndex (), device.getGraphicsFamilyIndex (), VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_VERTEX_INPUT_BIT );
		}

		vkEndCommandBuffer ( computeCommandBuffer );
	}

				// transition buffer from one queue family to another
	void	transitionBuffer ( Buffer& buf, VkAccessFlags srcAccessMask, VkAccessFlags dstAccessMask, uint32_t srcFamily, uint32_t dstFamily, 
							   VkPipelineStageFlags srcStageFlags, VkPipelineStageFlags dstStageFlags )
	{
		VkBufferMemoryBarrier bufferBarrier =
		{
			VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER,
			nullptr,
			srcAccessMask,
			dstAccessMask,
			srcFamily,
			dstFamily,
			posBuffer.getHandle (),
			0,
			VK_WHOLE_SIZE
		};

		vkCmdPipelineBarrier (
			computeCommandBuffer,
			srcStageFlags,
			dstStageFlags,
			0,
			0, nullptr,
			1, &bufferBarrier,
			0, nullptr );
	}
	
	void updateUniformBuffer ( uint32_t currentImage )
	{
		float				time = (float)getTime ();
		UniformBufferObject ubo  = {};

		ubo.model       = glm::rotate      ( glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view        = glm::lookAt      ( eye, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f) );
		ubo.proj        = glm::perspective ( glm::radians ( 60.0f ), getAspect (), zNear, zFar );
		ubo.proj[1][1] *= -1;

		uniformBuffers [currentImage].copy ( &ubo, sizeof ( ubo ) );
	}
	
	void	saveScreenshot ()
	{
		VkImage	srcImage = swapChain.getImages () [currentImage];		
		Image	image;
		
		image.create ( device, ImageCreateInfo ( getWidth (), getHeight () ).setFormat ( VK_FORMAT_R8G8B8A8_UNORM ).setTiling ( VK_IMAGE_TILING_LINEAR ).setUsage ( VK_IMAGE_USAGE_TRANSFER_DST_BIT ), 
					   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
					   
	   {
			SingleTimeCommand	cmd ( device );
			
			image.transitionLayout  ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL );
			Image::transitionLayout ( cmd, srcImage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
									  VK_ACCESS_MEMORY_READ_BIT, VK_ACCESS_TRANSFER_READ_BIT, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL );
			
					// use image copy (requires us to manually flip components)
			VkImageCopy imageCopyRegion               = {};
			imageCopyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.srcSubresource.layerCount = 1;
			imageCopyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			imageCopyRegion.dstSubresource.layerCount = 1;
			imageCopyRegion.extent.width              = getWidth  ();
			imageCopyRegion.extent.height             = getHeight ();
			imageCopyRegion.extent.depth              = 1;

			// Issue the copy command
			vkCmdCopyImage(
				cmd.getHandle (),
				srcImage, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image.getHandle (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &imageCopyRegion );

			image.transitionLayout  ( cmd, image.getFormat (), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_GENERAL );
			Image::transitionLayout ( cmd, srcImage, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 
								      VK_ACCESS_TRANSFER_READ_BIT, VK_ACCESS_MEMORY_READ_BIT, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR );
	   }
	   
		VkImageSubresource	subResource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0 };
		VkSubresourceLayout	subResourceLayout;
		
		vkGetImageSubresourceLayout ( device.getDevice (), image.getHandle (), &subResource, &subResourceLayout );

		const uint8_t * data = (const uint8_t *)image.getMemory ().map ( VK_WHOLE_SIZE );

		TgaImage	tga ( getWidth (), getHeight () );
		
		tga.setRgbaData ( data + subResourceLayout.offset );
		tga.writeToFile ( "screenshot.tga" );
		image.getMemory ().unmap ();
	}
	
	void	createBuffer ( Buffer& buffer, uint32_t usage, size_t size, const void * data )
	{
		Buffer				stagingBuffer;
		SingleTimeCommand	cmd ( device );

			// use staging buffer to copy data to GPU-local memory
		stagingBuffer.create ( device, size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT );
		stagingBuffer.copy   ( data, size );
		buffer.create        ( device, size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT );
		buffer.copyBuffer    ( cmd, stagingBuffer, size );

	}

	void initParticles ( int num )
	{
		n            = num;
		numParticles = n * n * n;

				// init buffers with particle data
		std::vector<glm::vec4> vb;
		std::vector<glm::vec4> pb;
		float 			  h = 2.0f / (n - 1);

		for ( size_t i = 0; i < n; i++ )
			for ( size_t j = 0; j < n; j++ )
				for ( size_t k = 0; k < n; k++ ) 
				{
					glm::vec4	p ( h * i - 1, h * j - 1, h * k - 1, 1 );

					pb.push_back ( p );
					vb.push_back ( glm::vec4 ( 0 ) );
				}

		createBuffer ( posBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,  numParticles * sizeof ( pb [0] ), pb.data () );
		createBuffer ( velBuffer, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,  numParticles * sizeof ( vb [0] ), vb.data () );
	}

	virtual	void	keyTyped ( int key, int scancode, int action, int mods ) override
	{
		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
			glfwSetWindowShouldClose ( window, GLFW_TRUE );		

		if ( key == GLFW_KEY_F1 && action == GLFW_PRESS )
			saveScreenshot ();
	}

};

int main ( int argc, const char * argv [] ) 
{
	TestWindow	win ( 1400, 900, "Compute particles" );

	return win.run ();
}
