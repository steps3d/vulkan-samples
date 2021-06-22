//
// Using textures with VulkanWindow
//

#include	"VulkanWindow.h"
#include	"Buffer.h"
#include	"DescriptorSet.h"
#include	"Texture.h"
#include	"stb_image_aug.h"
#include	"BasicMesh.h"
#include	"TgaImage.h"
#include	"Framebuffer.h"
#include	"Semaphore.h"
#include	"ScreenQuad.h"
#include	"CameraController.h"

struct UniformBufferObject 
{
	glm::mat4 mv;
	glm::mat4 proj;
	glm::mat4 nm;
	glm::vec4 light;
};

class	DeferredWindow : public VulkanWindow
{
	std::vector<VkCommandBuffer>	commandBuffers;
	GraphicsPipeline				pipeline;
	Renderpass						renderPass;
	std::vector<Buffer>				uniformBuffers;
	DescriptorPool					descriptorPool;
	std::vector<DescriptorSet> 		descriptorSets;
	Image							image;
	Sampler							sampler;
	Framebuffer						fb;					// G-buffer 
	VkCommandBuffer					offscreenCmd = VK_NULL_HANDLE;
	DescriptorSet					offscreenDescriptorSet;
	Semaphore						offscreenSemaphore;
	GraphicsPipeline				offscreenPipeline;
	ScreenQuad						screen;				// class used to do screen processing
	float							zMin = 0.1f;
	float							zMax = 100.0f;	
	double							time = 0;

	DescriptorSet					offscreenDescriptorSet1, offscreenDescriptorSet2, offscreenDescriptorSet3;

	BasicMesh * box1 = nullptr;		// decalMap, bump1 -> DS1
	BasicMesh * box2 = nullptr;		// stoneMap. bump2 -> DS2
	BasicMesh * box3 = nullptr;
	BasicMesh * box4 = nullptr;
	BasicMesh * box5 = nullptr;
	BasicMesh * knot = nullptr;		// knotMap, bump2 -> DS3

	Texture							decalMap, stoneMap, knotMap;
	Texture							bump1, bump2;
	CameraController				controller;

public:
	DeferredWindow ( int w, int h, const std::string& t ) : VulkanWindow ( w, h, t, true ), controller ( this )
	{
		sampler.create ( device );		// use default optiona		
		screen.create  ( device );

		decalMap.load2D ( device, "../../Textures/oak.jpg"         );
		stoneMap.load2D ( device, "../../Textures/block.jpg"       );
		knotMap.load2D  ( device, "../../Textures/Oxidated.jpg"    );
		bump1.load2D    ( device, "../../Textures/wood_normal.png" );
		bump2.load2D    ( device, "../../Textures/brick_nm.bmp"    );

			// create G-buffer with 2 RGBA16F color attachments and depth attachment
		fb.init ( device, getWidth (), getHeight () )
		  .addAttachment ( VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
		  .addAttachment ( VK_FORMAT_R16G16B16A16_SNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT )
		  .addAttachment ( VK_FORMAT_D24_UNORM_S8_UINT,  VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT  )
		  .create ();
		  
			// create meshes
		box1 = createBox  ( device, glm::vec3 ( -6, -0.1, -6 ),   glm::vec3 ( 12, 3, 12 ), nullptr, true );
		box2 = createBox  ( device, glm::vec3 ( -1.5, 0, -0.5 ),  glm::vec3 ( 1,  2,  2 ) );
		box3 = createBox  ( device, glm::vec3 ( 1.5, 0, -0.5 ),   glm::vec3 ( 1,  1,  1 ) );
		box4 = createBox  ( device, glm::vec3 ( -4, 0, -0.5 ),    glm::vec3 ( 1,  1,  1 ) );
		box5 = createBox  ( device, glm::vec3 ( -4, 0, -4 ),      glm::vec3 ( 1,  1,  1 ) ); 
		knot = createKnot ( device, 1, 4, 120, 30 );
				
			// create all pipelines
		createPipelines ();
	}

	~DeferredWindow ()
	{
		delete box1;
		delete box2;
		delete box3;
		delete box4;
		delete box5;
		delete knot;
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

			// create descriptors for last pass
		for ( uint32_t i = 0; i < swapChain.imageCount (); i++ )
		{
			descriptorSets  [i]
				.setLayout ( device, pipeline.getDescLayout (), descriptorPool )
				.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [i], 0, sizeof ( UniformBufferObject ) )
				.addImage  ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, fb.getAttachment ( 0 ), sampler )
				.addImage  ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, fb.getAttachment ( 1 ), sampler )
				.create    ();
		}
		
			// create descriptors for rendering boxes 
		offscreenDescriptorSet1
			.setLayout ( device, offscreenPipeline.getDescLayout (), descriptorPool )
			.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [0], 0, sizeof ( UniformBufferObject ) )
			.addImage  ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, decalMap, sampler )
			.addImage  ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, bump1,    sampler )
			.create    ();

		
		offscreenDescriptorSet2
			.setLayout ( device, offscreenPipeline.getDescLayout (), descriptorPool )
			.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [0], 0, sizeof ( UniformBufferObject ) )
			.addImage  ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stoneMap, sampler )
			.addImage  ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, bump2,    sampler )
			.create    ();

		
		offscreenDescriptorSet3
			.setLayout ( device, offscreenPipeline.getDescLayout (), descriptorPool )
			.addBuffer ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uniformBuffers [0], 0, sizeof ( UniformBufferObject ) )
			.addImage  ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, knotMap, sampler )
			.addImage  ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, bump2,   sampler )
			.create    ();

		
	}
	
	virtual	void	createPipelines () override 
	{
		createUniformBuffers ();

		descriptorPool
			.setMaxSets            ( 15+swapChain.imageCount () )
			.setUniformBufferCount ( 15+swapChain.imageCount () )
			.setImageCount         ( 15+swapChain.imageCount () )
			.create                ( device );

			// current app code
		renderPass.addAttachment   ( swapChain.getFormat (),                VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR )
				  .addAttachment   ( depthTexture.getImage ().getFormat (), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL )
				  .addSubpass      ( 0 )
				  .addDepthSubpass ( 1 )
				  .create          ( device );
		
		screen.setVertexAttrs ( pipeline )
				.setDevice         ( device )
				.setVertexShader   ( "shaders/ds-3-2.vert.spv" )
				.setFragmentShader ( "shaders/ds-3-2.frag.spv" )
				.setSize           ( swapChain.getExtent ().width, swapChain.getExtent ().height )
				.addVertexBinding  ( sizeof ( float ) * 4, 0, VK_VERTEX_INPUT_RATE_VERTEX )
				.addDescLayout     ( 0, DescSetLayout ()
							.add ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT )
							.add ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )		// color attachment 0
							.add ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT )	)	// color attachment 1
				.setCullMode       ( VK_CULL_MODE_NONE )
				.setFrontFace      ( VK_FRONT_FACE_COUNTER_CLOCKWISE )
				.setDepthTest      ( true )
				.setDepthWrite     ( true )
				.create            ( renderPass );
			
		box1->setVertexAttrs ( offscreenPipeline )
				.setDevice         ( device )
				.setVertexShader   ( "shaders/ds-3-1.vert.spv" )
				.setFragmentShader ( "shaders/ds-3-1.frag.spv" )
				.setSize           ( swapChain.getExtent ().width, swapChain.getExtent ().height )
				.addVertexBinding  ( sizeof ( BasicVertex ), 0, VK_VERTEX_INPUT_RATE_VERTEX )
				.addDescLayout     ( 0, DescSetLayout ()
							.add ( 0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,         VK_SHADER_STAGE_VERTEX_BIT	)
							.add ( 1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT ) 		// decal
							.add ( 2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT ) )		// bump
				.setCullMode       ( VK_CULL_MODE_NONE )
				.setFrontFace      ( VK_FRONT_FACE_COUNTER_CLOCKWISE )
				.setDepthTest      ( true )
				.setDepthWrite     ( true )
				.create            ( fb.getRenderpass () );
			


				// create before command buffers
		swapChain.createFramebuffers ( renderPass.getHandle (), depthTexture.getImageView () );

		createDescriptorSets         ();
		createCommandBuffers         ( renderPass.getHandle (), pipeline.getHandle () );
		createOffscreenCommandBuffer ();
	}

	virtual	void	freePipelines () override
	{
		vkFreeCommandBuffers ( device.getDevice (), device.getCommandPool (), static_cast<uint32_t>(commandBuffers.size()), commandBuffers.data () );

		commandBuffers.clear ();	
		pipeline.clean       ();
		renderPass.clean     ();
		freeUniformBuffers   ();
		descriptorSets.clear ();
		descriptorPool.clean ();
	}
	
	virtual	void	submit ( uint32_t imageIndex ) override 
	{
		updateUniformBuffer ( imageIndex );

		VkSubmitInfo			submitInfo           = {};
		VkSemaphore				waitSemaphores    [] = { swapChain.currentAvailableSemaphore () };
		VkPipelineStageFlags	waitStages        [] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
		VkSemaphore				signalSemaphores1 [] = { offscreenSemaphore.getHandle () };
		VkSemaphore				signalSemaphores2 [] = { swapChain.currentRenderFinishedSemaphore () };
		VkFence					currentFence         = swapChain.currentInFlightFence ();

		submitInfo.sType                = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.waitSemaphoreCount   = 1;
		submitInfo.pWaitSemaphores      = waitSemaphores;
		submitInfo.pWaitDstStageMask    = waitStages;
		submitInfo.commandBufferCount   = 1;
		submitInfo.pCommandBuffers      = &offscreenCmd;	//&commandBuffers [imageIndex];
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores    = signalSemaphores1;

		vkResetFences ( device.getDevice (), 1, &currentFence );

		if ( vkQueueSubmit ( device.getGraphicsQueue (), 1, &submitInfo, currentFence ) != VK_SUCCESS )
			fatal () << "failed to submit draw command buffer!";

				
		submitInfo.pWaitSemaphores    = signalSemaphores1;	// wait for offscreen semaphore		
		submitInfo.pSignalSemaphores  = signalSemaphores2;	// Signal ready with render complete semaphpre
		submitInfo.pCommandBuffers    = &commandBuffers [imageIndex];
		
		if ( vkQueueSubmit ( device.getGraphicsQueue (), 1, &submitInfo, VK_NULL_HANDLE ) != VK_SUCCESS )
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

			screen.render  ( commandBuffers [i] );

			vkCmdEndRenderPass     ( commandBuffers [i] );

			if ( vkEndCommandBuffer ( commandBuffers [i] ) != VK_SUCCESS )
				fatal () << "VulkanWindow: failed to record command buffer!";
		}
	}

	void	createOffscreenCommandBuffer ()
	{
		offscreenSemaphore.create ( device );
		
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType              = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool        = device.getCommandPool ();
		allocInfo.level              = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = 1;

		if ( vkAllocateCommandBuffers ( device.getDevice (), &allocInfo, &offscreenCmd ) != VK_SUCCESS )
			fatal () << "VulkanWindow: failed to allocate offscreen command buffer!";
		
		VkCommandBufferBeginInfo beginInfo = {};
		
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
// XXX
beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
// XXX

		if ( vkBeginCommandBuffer ( offscreenCmd, &beginInfo ) != VK_SUCCESS )
			fatal () << "VulkanWindow: failed to begin recording command buffer!";

		VkRenderPassBeginInfo	renderPassInfo = {};
		VkClearValue			clearValues [3] = {};

		clearValues[0].color        = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[1].color        = {0.0f, 0.0f, 0.0f, 1.0f};
		clearValues[2].depthStencil = {1.0f, 0};

		renderPassInfo.sType             = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		renderPassInfo.renderPass        = fb.getRenderpass ().getHandle ();
		renderPassInfo.framebuffer       = fb.getHandle ();
		renderPassInfo.renderArea.offset = {0, 0};
		renderPassInfo.renderArea.extent = { fb.getWidth (), fb.getHeight () };
		renderPassInfo.clearValueCount   = 3;
		renderPassInfo.pClearValues      = clearValues;

		vkCmdBeginRenderPass ( offscreenCmd, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE );
		vkCmdBindPipeline    ( offscreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline.getHandle () );

		VkDeviceSize	offsets [] = { 0 };
		VkDescriptorSet	descSet1    = offscreenDescriptorSet1.getHandle ();
		VkDescriptorSet	descSet2    = offscreenDescriptorSet2.getHandle ();
		VkDescriptorSet	descSet3    = offscreenDescriptorSet3.getHandle ();

		vkCmdBindDescriptorSets ( offscreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline.getLayout (), 0, 1, &descSet1, 0, nullptr );

		box1->render  ( offscreenCmd );

		vkCmdBindDescriptorSets ( offscreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline.getLayout (), 0, 1, &descSet2, 0, nullptr );

		box2->render  ( offscreenCmd );
		box3->render  ( offscreenCmd );
		box4->render  ( offscreenCmd );
		box5->render  ( offscreenCmd );

		vkCmdBindDescriptorSets ( offscreenCmd, VK_PIPELINE_BIND_POINT_GRAPHICS, offscreenPipeline.getLayout (), 0, 1, &descSet3, 0, nullptr );

		knot->render  ( offscreenCmd );

		vkCmdEndRenderPass     ( offscreenCmd );

		if ( vkEndCommandBuffer ( offscreenCmd ) != VK_SUCCESS )
			fatal () << "VulkanWindow: failed to record command buffer!";
		
	}
	
	void updateUniformBuffer ( uint32_t currentImage )
	{
		float				time = (float)getTime ();
		auto				mv   = controller.getModelView  ();
		auto				proj = controller.getProjection ();
		UniformBufferObject ubo  = {};

		ubo.mv          = mv;
		ubo.proj        = proj;		//glm::perspective(glm::radians(45.0f), getAspect (), zMin, zMax );
		ubo.proj[1][1] *= -1;
		ubo.nm          = normalMatrix ( ubo.mv );

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
	
	virtual	void	keyTyped ( int key, int scancode, int action, int mods ) override
	{
		if ( key == GLFW_KEY_ESCAPE && action == GLFW_PRESS )
			glfwSetWindowShouldClose ( window, GLFW_TRUE );		

		if ( key == GLFW_KEY_F1 && action == GLFW_PRESS )
			saveScreenshot ();

		controller.keyTyped ( key, scancode, action, mods );
	}

	virtual	void	mouseMotion ( double x, double y ) override
	{
		controller.mouseMotion ( x, y );
	}

	virtual	void	idle () override 
	{
		double	t  = getTime ();
		double	dt = 30.0f*(t - time);

		controller.timeElapsed ( dt );

		//log () << "Dt = " << dt << Log::endl;

		time = t;
	}
};

int main ( int argc, const char * argv [] ) 
{
	DeferredWindow	win ( 1200, 900, "Deferred rendering in Vulkan" );

	return win.run ();
}
