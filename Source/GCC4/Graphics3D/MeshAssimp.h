#pragma once
//========================================================================
//========================================================================
#include <SDKMesh.h>
#include "Geometry.h"
#include "../ResourceCache/ResCache.h"
#include "ModelType.h"
#include "VertexType.h"

#include <assimp/Importer.hpp>

/* assimp include files. These three are usually needed. */
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

//
// class D3DSdkMeshResourceExtraData11					- Chapter 16, page 561
//
class D3DSdkMeshResourceExtraData11 : public IResourceExtraData
{
	friend class SdkMeshResourceLoader;

public:
	D3DSdkMeshResourceExtraData11() { };
	virtual ~D3DSdkMeshResourceExtraData11() { }
	virtual std::string VToString() { return "D3DSdkMeshResourceExtraData11"; }

	CDXUTSDKMesh                m_Mesh11;



};

//
// class SdkMeshResourceLoader							- Chapter 16, page 561
//
class SdkMeshResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.sdkmesh"; }
};



//ASSIMP aggiunte
//
class D3DAssimpMeshResourceExtraData11 : public IResourceExtraData
{
	friend class SdkMeshResourceLoader;

public:
	D3DAssimpMeshResourceExtraData11() { m_NumIndicesAssimp = 0, m_NumFacesAssimp = 0, m_NumVerteciesAssimp = 0; };
	virtual ~D3DAssimpMeshResourceExtraData11() { }
	virtual std::string VToString() { return "D3DAssimpMeshResourceExtraData11"; }

	CDXUTSDKMesh                m_Mesh11;
	//shared_ptr<ModelType> m_assimpMesh11;
	std::vector<ModelType> m_assimpMesh11;
	TextureClass* m_Texture;
	int m_NumIndicesAssimp, m_NumFacesAssimp, m_NumVerteciesAssimp;
	ID3D11Buffer *m_vertexBuffer, *m_indexBuffer;

};

class AssimpMeshResourceLoader : public IResourceLoader
{
public:
	virtual bool VUseRawFile() { return false; }
	virtual bool VDiscardRawBufferAfterLoad() { return false; }
	virtual unsigned int VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize);
	virtual bool VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle);
	virtual std::string VGetPattern() { return "*.obj"; }

	std::shared_ptr<D3DAssimpMeshResourceExtraData11> LoadModelUsingAssimp(const std::string& Filename);
	bool LoadTextureUsingAssimp(ID3D11Device*, const std::string& Filename);
	
	
};

//END ASSIMP


////////////////////////////////////////////////////
//
// D3DMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameCode4 scene graph
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DMeshNode
//
////////////////////////////////////////////////////

class D3DMeshNode9 : public D3DSceneNode9
{
protected:
	ID3DXMesh *m_pMesh;
	std::wstring m_XFileName;

public:
	D3DMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh,
		RenderPass renderPass,
		const Mat4x4 *t);

	D3DMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::wstring xFileName,
		RenderPass renderPass,
		const Mat4x4 *t);

	virtual ~D3DMeshNode9() { SAFE_RELEASE(m_pMesh); }
	HRESULT VRender(Scene *pScene);
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

	float CalcBoundingSphere();
};


////////////////////////////////////////////////////
//
// D3DShaderMeshNode9 Description
//
//    Attaches a ID3DXMesh Mesh object to the GameCode4 scene graph and renders it with a shader.
//    This is a creature of D3D9, and not described in the 4th edition.
//    In the 3rd edition this was called D3DShaderMeshNode
//
////////////////////////////////////////////////////

class D3DShaderMeshNode9 : public D3DMeshNode9
{
protected:
	ID3DXEffect *m_pEffect;
	std::string m_fxFileName;

public:
	D3DShaderMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		ID3DXMesh *Mesh,
		std::string fxFileName,
		RenderPass renderPass,
		const Mat4x4 *t);

	D3DShaderMeshNode9(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::wstring xFileName,
		std::string fxFileName,
		RenderPass renderPass,
		const Mat4x4 *t);

	virtual ~D3DShaderMeshNode9() { SAFE_RELEASE(m_pEffect); }
	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnLostDevice(Scene *pScene);
	HRESULT VRender(Scene *pScene);
};

////////////////////////////////////////////////////
//
// class TeapotMeshNode9				- Chapter X, page Y
//
////////////////////////////////////////////////////

class D3DTeapotMeshNode9 : public D3DShaderMeshNode9
{
public:
	D3DTeapotMeshNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, std::string fxFileName, RenderPass renderPass, const Mat4x4 *t);
	virtual HRESULT VOnRestore(Scene *pScene);
};


class D3DShaderMeshNode11 : public SceneNode
{
public:
	D3DShaderMeshNode11(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::string sdkMeshFileName,
		RenderPass renderPass,
		const Mat4x4 *t);

	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnLostDevice(Scene *pScene) { return S_OK; }
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

protected:
	std::string					m_sdkMeshFileName;

	GameCode4_Hlsl_VertexShader		m_VertexShader;
	GameCode4_Hlsl_PixelShader		m_PixelShader;

	float CalcBoundingSphere(CDXUTSDKMesh *mesh11);			// this was added post press.
};


class D3DTeapotMeshNode11 : public D3DShaderMeshNode11
{
public:
	D3DTeapotMeshNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Mat4x4 *t)
		//: D3DShaderMeshNode11(actorId, renderComponent, "art\\teapot.sdkmesh", renderPass, t) { }
		: D3DShaderMeshNode11(actorId, renderComponent, "art\\teapot.sdkmesh", renderPass, t) { }

	bool VIsVisible() { return true; }
};

class D3DShaderAssimpMeshNode11 : public SceneNode
{
public:
	D3DShaderAssimpMeshNode11(const ActorId actorId,
		WeakBaseRenderComponentPtr renderComponent,
		std::string sdkMeshFileName,
		RenderPass renderPass,
		const Mat4x4 *t);

	virtual HRESULT VOnRestore(Scene *pScene);
	virtual HRESULT VOnLostDevice(Scene *pScene) { return S_OK; }
	virtual HRESULT VRender(Scene *pScene);
	virtual HRESULT VPick(Scene *pScene, RayCast *pRayCast);

protected:
	std::string					m_sdkMeshFileName;

	std::string					m_assimpMeshFileName;

	GameCode4_Hlsl_VertexShader		m_VertexShader;
	GameCode4_Hlsl_PixelShader		m_PixelShader;

	float CalcBoundingSphere(CDXUTSDKMesh *mesh11);			// this was added post press.
	float CalcBoundingSphere(std::vector<ModelType> m_assimpMesh11);
	bool InitializeBuffers(ID3D11Device* device, std::shared_ptr<D3DAssimpMeshResourceExtraData11>*);
	void RenderBuffers(ID3D11DeviceContext* deviceContext, std::shared_ptr<D3DAssimpMeshResourceExtraData11>*);
	
};

class D3DAssimpTeapotMeshNode11 : public D3DShaderAssimpMeshNode11
{
public:
	D3DAssimpTeapotMeshNode11(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, RenderPass renderPass, const Mat4x4 *t)
		: D3DShaderAssimpMeshNode11(actorId, renderComponent, "art\\untitled.obj", renderPass, t) { }
	bool VIsVisible() { return true; }
};


