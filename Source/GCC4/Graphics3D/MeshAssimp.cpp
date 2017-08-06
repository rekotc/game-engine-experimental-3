//========================================================================
//========================================================================

#include "GameCodeStd.h"

#include <SDKmisc.h>

#include "../GameCode4/GameCode.h"
#include "../ResourceCache/ResCache.h"
#include "D3DRenderer.h"
#include "Lights.h"
#include "MeshAssimp.h"
#include "Raycast.h"
#include "SceneNodes.h"


shared_ptr<IResourceLoader> CreateSdkMeshResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW SdkMeshResourceLoader());
}

unsigned int SdkMeshResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	// The raw data of the SDK Mesh file is needed by the CDXUTMesh class, so we're going to keep it around.
	return rawSize;
}

//
// SdkMeshResourceLoader::VLoadResource						- Chapter 16, page 561
//
bool SdkMeshResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	GameCodeApp::Renderer renderer = GameCodeApp::GetRendererImpl();
	if (renderer == GameCodeApp::Renderer_D3D9)
	{
		GCC_ASSERT(0 && "This is not supported in D3D9");
	}
	else if (renderer == GameCodeApp::Renderer_D3D11)
	{
		shared_ptr<D3DSdkMeshResourceExtraData11> extra = shared_ptr<D3DSdkMeshResourceExtraData11>(GCC_NEW D3DSdkMeshResourceExtraData11());

		// Load the Mesh
		if (SUCCEEDED(extra->m_Mesh11.Create(DXUTGetD3D11Device(), (BYTE *)rawBuffer, (UINT)rawSize, true)))
		{
			handle->SetExtra(shared_ptr<D3DSdkMeshResourceExtraData11>(extra));
		}

		return true;
	}

	GCC_ASSERT(0 && "Unsupported Renderer in SdkMeshResourceLoader::VLoadResource");
	return false;
}

//ASSIMP RESOURCE LOADER 
shared_ptr<IResourceLoader> CreateAssimpMeshResourceLoader()
{
	return shared_ptr<IResourceLoader>(GCC_NEW AssimpMeshResourceLoader());
}

unsigned int AssimpMeshResourceLoader::VGetLoadedResourceSize(char *rawBuffer, unsigned int rawSize)
{
	// The raw data of the SDK Mesh file is needed by the CDXUTMesh class, so we're going to keep it around.
	return rawSize;
}

//
// SdkMeshResourceLoader::VLoadResource						- Chapter 16, page 561
//
bool AssimpMeshResourceLoader::VLoadResource(char *rawBuffer, unsigned int rawSize, shared_ptr<ResHandle> handle)
{
	GameCodeApp::Renderer renderer = GameCodeApp::GetRendererImpl();
	if (renderer == GameCodeApp::Renderer_D3D9)
	{
		GCC_ASSERT(0 && "This is not supported in D3D9");
	}
	else if (renderer == GameCodeApp::Renderer_D3D11)
	{
		//shared_ptr<D3DAssimpMeshResourceExtraData11> extra = shared_ptr<D3DAssimpMeshResourceExtraData11>(GCC_NEW D3DAssimpMeshResourceExtraData11());

		// Load the Mesh
	/*	if (SUCCEEDED(extra->m_Mesh11.Create(DXUTGetD3D11Device(), (BYTE *)rawBuffer, (UINT)rawSize, true)))
		{
			handle->SetExtra(shared_ptr<D3DAssimpMeshResourceExtraData11>(extra));
		}*/
		ID3D11Device* device = DXUTGetD3D11Device();
		//ID3D11DeviceContext* deviceContext = DXUTGetD3D11DeviceContext();

		shared_ptr<D3DAssimpMeshResourceExtraData11> extra = LoadModelUsingAssimp(handle->GetName());
		LoadTextureUsingAssimp(device, "../Assets/Art/untitled.obj", &extra);

		if (extra != NULL) {
			handle->SetExtra(shared_ptr<D3DAssimpMeshResourceExtraData11>(extra));
		}
		else
			GCC_ASSERT(0 && "Errore nel caricamento della mesh!");

		//extra->m_assimpMesh11 

		

		return true;
	}

	GCC_ASSERT(0 && "Unsupported Renderer in AssimpMeshResourceLoader::VLoadResource");
	return false;
}




////////////////////////////////////////////////////
// MeshNode Implementation
////////////////////////////////////////////////////

//
// D3DMeshNode9::D3DMeshNode9				- 3rd Edition, Chapter 14, page 504
//
D3DMeshNode9::D3DMeshNode9(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	ID3DXMesh *Mesh,
	RenderPass renderPass,
	const Mat4x4 *t)
	: D3DSceneNode9(actorId, renderComponent, renderPass, t)
{
	m_pMesh = Mesh;
	if (m_pMesh)
	{
		// Added post press - not all Mesh modes have Meshes. Some are just effects!
		m_pMesh->AddRef();
	}
}

//
// MeshNode::MeshNode					-  3rd Edition, Chapter 14, page 505
//
D3DMeshNode9::D3DMeshNode9(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	std::wstring xFileName,
	RenderPass renderPass,
	const Mat4x4 *t)
	: D3DSceneNode9(actorId, renderComponent, renderPass, t)
{
	m_pMesh = NULL;
	m_XFileName = xFileName;
}

//
// MeshNode::CalcBoundingSphere				-  3rd Edition, Chapter 14, page 507
//
float D3DMeshNode9::CalcBoundingSphere()
{
	D3DXVECTOR3* pData;
	D3DXVECTOR3 vCenter;
	FLOAT fObjectRadius;
	HRESULT hr;
	V(m_pMesh->LockVertexBuffer(0, (LPVOID*)&pData));
	V(D3DXComputeBoundingSphere(pData, m_pMesh->GetNumVertices(),
		D3DXGetFVFVertexSize(m_pMesh->GetFVF()), &vCenter, &fObjectRadius));
	V(m_pMesh->UnlockVertexBuffer());

	return fObjectRadius;
}


//
// MeshNode::VRender				-  3rd Edition, Chapter 14, page 505
//
HRESULT D3DMeshNode9::VRender(Scene *pScene)
{
	if (S_OK != D3DSceneNode9::VRender(pScene))
		return E_FAIL;

	return m_pMesh->DrawSubset(0);
}

//
// MeshNode::VOnRestore				-  3rd Edition, Chapter 14, page 506
//
// This function loads the Mesh and ensures the Mesh has normals; it also optimizes the 
// Mesh for the graphics card's vertex cache, which improves performance by organizing 
// the internal triangle list for less cache misses.
//
HRESULT D3DMeshNode9::VOnRestore(Scene *pScene)
{
	if (m_XFileName.empty())
	{
		SetRadius(CalcBoundingSphere());
		return D3DSceneNode9::VOnRestore(pScene);
	}

	// Change post press - release the Mesh only if we have a valid Mesh file name to load.
	// Otherwise we likely created it on our own, and needs to be kept.
	SAFE_RELEASE(m_pMesh);

	WCHAR str[MAX_PATH];
	HRESULT hr;

	// Load the Mesh with D3DX and get back a ID3DXMesh*.  For this
	// sample we'll ignore the X file's embedded materials since we know 
	// exactly the model we're loading.  See the Mesh samples such as
	// "OptimizedMesh" for a more generic Mesh loading example.
	V_RETURN(DXUTFindDXSDKMediaFileCch(str, MAX_PATH, m_XFileName.c_str()));

	V_RETURN(D3DXLoadMeshFromX(str, D3DXMESH_MANAGED, DXUTGetD3D9Device(), NULL, NULL, NULL, NULL, &m_pMesh));

	DWORD *rgdwAdjacency = NULL;

	// Make sure there are normals which are required for lighting
	if (!(m_pMesh->GetFVF() & D3DFVF_NORMAL))
	{
		ID3DXMesh* pTempMesh;
		V(m_pMesh->CloneMeshFVF(m_pMesh->GetOptions(),
			m_pMesh->GetFVF() | D3DFVF_NORMAL,
			DXUTGetD3D9Device(), &pTempMesh));
		V(D3DXComputeNormals(pTempMesh, NULL));

		SAFE_RELEASE(m_pMesh);
		m_pMesh = pTempMesh;
	}

	// Optimize the Mesh for this graphics card's vertex cache 
	// so when rendering the Mesh's triangle list the vertices will 
	// cache hit more often so it won't have to re-execute the vertex shader 
	// on those vertices so it will improve perf.     

	rgdwAdjacency = GCC_NEW DWORD[m_pMesh->GetNumFaces() * 3];
	if (rgdwAdjacency == NULL)
		return E_OUTOFMEMORY;
	V(m_pMesh->ConvertPointRepsToAdjacency(NULL, rgdwAdjacency));
	V(m_pMesh->OptimizeInplace(D3DXMESHOPT_VERTEXCACHE, rgdwAdjacency, NULL, NULL, NULL));

	SAFE_DELETE_ARRAY(rgdwAdjacency);

	SetRadius(CalcBoundingSphere());

	return D3DSceneNode9::VOnRestore(pScene);
}

HRESULT D3DMeshNode9::VPick(Scene *pScene, RayCast *pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast) == E_FAIL)
		return E_FAIL;

	pScene->PushAndSetMatrix(m_Props.ToWorld());
	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), m_pMesh);
	pScene->PopMatrix();

	return hr;
}


////////////////////////////////////////////////////
// D3DShaderMeshNode9 Implementation
////////////////////////////////////////////////////

//
// ShaderMeshNode::ShaderMeshNode				- 3rd Edition, Chapter 14, page 517
//
D3DShaderMeshNode9::D3DShaderMeshNode9(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	ID3DXMesh *Mesh,
	std::string fxFileName,			// used to be effect pointer - changed to fix a VOnRestore crash.
	RenderPass renderPass,
	const Mat4x4 *t)
	: D3DMeshNode9(actorId, renderComponent, Mesh, renderPass, t)
{
	m_pEffect = NULL;
	m_fxFileName = fxFileName;
}

//
// ShaderMeshNode::ShaderMeshNode				- 3rd Edition, Chapter 14, page 517
//
D3DShaderMeshNode9::D3DShaderMeshNode9(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	std::wstring xFileName,
	std::string fxFileName,
	RenderPass renderPass,
	const Mat4x4 *t)
	: D3DMeshNode9(actorId, renderComponent, xFileName, renderPass, t)
{
	m_pEffect = NULL;
	m_fxFileName = fxFileName;
}

//
// ShaderMeshNode::VOnRestore					- very similar to MeshNode::VOnRestore
//
HRESULT D3DShaderMeshNode9::VOnRestore(Scene *pScene)
{
	SAFE_RELEASE(m_pEffect);

	DWORD dwShaderFlags = D3DXFX_NOT_CLONEABLE | D3DXSHADER_DEBUG | D3DXSHADER_NO_PRESHADER;
	HRESULT hr;

	Resource resource(m_fxFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);  // this actually loads the XML file from the zip file
	V(D3DXCreateEffect(DXUTGetD3D9Device(), pResourceHandle->Buffer(), pResourceHandle->Size(), NULL, NULL, dwShaderFlags, NULL, &m_pEffect, NULL));
	return D3DMeshNode9::VOnRestore(pScene);
}

//
// ShaderMeshNode::VRender						- 3rd Edition, Chapter 14, page 517				
//
HRESULT D3DShaderMeshNode9::VRender(Scene *pScene)
{
	if (S_OK != D3DSceneNode9::VRender(pScene))
		return E_FAIL;

	HRESULT hr;

	// Update the effect's variables.  Instead of using strings, it would 
	// be more efficient to cache a handle to the parameter by calling 
	// ID3DXEffect::GetParameterByName

	Mat4x4 worldViewProj = pScene->GetCamera()->GetWorldViewProjection(pScene);
	Mat4x4 world = pScene->GetTopMatrix();

	D3DXCOLOR ambient = m_Props.GetMaterial().GetAmbient();
	V_RETURN(m_pEffect->SetValue("g_MaterialAmbientColor", &ambient, sizeof(D3DXCOLOR)));
	D3DXCOLOR diffuse = m_Props.GetMaterial().GetDiffuse();
	V_RETURN(m_pEffect->SetValue("g_MaterialDiffuseColor", &diffuse, sizeof(D3DXCOLOR)));

	V(m_pEffect->SetMatrix("g_mWorldViewProjection", &worldViewProj));
	V(m_pEffect->SetMatrix("g_mWorld", &world));
	V(m_pEffect->SetFloat("g_fTime", (float)1.0f));

	int count = pScene->GetLightManager()->GetLightCount(this);
	if (count)
	{
		// Light 0 is the only one we use for ambient lighting. The rest are ignored in this simple shader.
		V(m_pEffect->SetValue("g_LightAmbient", pScene->GetLightManager()->GetLightAmbient(this), sizeof(D3DXVECTOR4) * 1));
		V(m_pEffect->SetValue("g_LightDir", pScene->GetLightManager()->GetLightDirection(this), sizeof(D3DXVECTOR4) * MAXIMUM_LIGHTS_SUPPORTED));
		V(m_pEffect->SetValue("g_LightDiffuse", pScene->GetLightManager()->GetLightDiffuse(this), sizeof(D3DXVECTOR4) * MAXIMUM_LIGHTS_SUPPORTED));
		V(m_pEffect->SetInt("g_nNumLights", count));
	}

	float alpha = m_Props.GetMaterial().GetAlpha();
	V(m_pEffect->SetFloat("g_fAlpha", alpha));
	V(m_pEffect->SetTechnique((alpha < 1.0f) ? "RenderSceneWithAlpha" : "RenderScene"));

	// Apply the technique contained in the effect 
	UINT iPass, cPasses;

	V(m_pEffect->Begin(&cPasses, 0));

	for (iPass = 0; iPass < cPasses; iPass++)
	{
		V(m_pEffect->BeginPass(iPass));

		// The effect interface queues up the changes and performs them 
		// with the CommitChanges call. You do not need to call CommitChanges if 
		// you are not setting any parameters between the BeginPass and EndPass.
		// V( g_pEffect->CommitChanges() );

		// Render the Mesh with the applied technique
		V(m_pMesh->DrawSubset(0));

		V(m_pEffect->EndPass());
	}
	V(m_pEffect->End());

	return S_OK;
}

HRESULT D3DShaderMeshNode9::VOnLostDevice(Scene *pScene)
{
	SAFE_RELEASE(m_pEffect);

	HRESULT hr;
	V_RETURN(D3DMeshNode9::VOnLostDevice(pScene));
	return S_OK;
}


//
// TeapotMeshNode::TeapotMeshNode					- Chapter X, page Y 
//
D3DTeapotMeshNode9::D3DTeapotMeshNode9(const ActorId actorId, WeakBaseRenderComponentPtr renderComponent, std::string fxFileName, RenderPass renderPass, const Mat4x4 *t)
	: D3DShaderMeshNode9(actorId, renderComponent, NULL, fxFileName, renderPass, t)
{
	// there's nothing else needed here...
}

//
// TeapotMeshNode::VOnRestore						- Chapter X, page Y
//
HRESULT D3DTeapotMeshNode9::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	IDirect3DDevice9 * pDevice = DXUTGetD3D9Device();

	SAFE_RELEASE(m_pMesh);
	V(D3DXCreateTeapot(pDevice, &m_pMesh, NULL));

	//Rotate the teapot 90 degrees from default so that the spout faces forward
	Mat4x4 rotateY90;
	rotateY90.BuildRotationY(-GCC_PI / 2.0f);
	IDirect3DVertexBuffer9* pVB = NULL;
	m_pMesh->GetVertexBuffer(&pVB);
	Vec3* pVertices = NULL;
	pVB->Lock(0, 0, (void**)&pVertices, 0);
	for (unsigned int i = 0; i<m_pMesh->GetNumVertices(); ++i)
	{
		*pVertices = rotateY90.Xform(*pVertices);
		++pVertices;
		//The structs depicted in this vertex buffer actually store
		//information for normals in addition to xyz, thereby
		//making the vertices in pVB twice the size of the one described
		//by *pVertices.  So we address that here.
		*pVertices = rotateY90.Xform(*pVertices);	//rotate the normals, too
		++pVertices;
	}
	pVB->Unlock();
	SAFE_RELEASE(pVB);
	//...end rotation



	// Note - the Mesh is needed BEFORE calling the base class VOnRestore.
	V(D3DShaderMeshNode9::VOnRestore(pScene));
	return S_OK;
}




//
// D3DShaderMeshNode11::D3DShaderMeshNode11					- Chapter 16, page 562 
//
D3DShaderMeshNode11::D3DShaderMeshNode11(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	std::string sdkMeshFileName,
	RenderPass renderPass,
	const Mat4x4 *t)
	: SceneNode(actorId, renderComponent, renderPass, t)
{
	m_sdkMeshFileName = sdkMeshFileName;
}


//
// D3DShaderMeshNode11::VOnRestore							- Chapter 16, page 563
//
HRESULT D3DShaderMeshNode11::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene));

	V_RETURN(m_VertexShader.OnRestore(pScene));
	V_RETURN(m_PixelShader.OnRestore(pScene));

	// Force the Mesh to reload
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	SetRadius(CalcBoundingSphere(&extra->m_Mesh11));

	return S_OK;
}

//
// D3DShaderMeshNode11::VRender								- Chapter 16, page 564
//
HRESULT D3DShaderMeshNode11::VRender(Scene *pScene)
{
	HRESULT hr;

	V_RETURN(m_VertexShader.SetupRender(pScene, this));
	V_RETURN(m_PixelShader.SetupRender(pScene, this));

	//Get the Mesh
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	// FUTURE WORK - this code WON'T be able to find texture resources referred to by the sdkmesh file 
	// in the Resource cache.

	//IA setup
	UINT Strides[1];
	UINT Offsets[1];
	ID3D11Buffer* pVB[1];
	pVB[0] = extra->m_Mesh11.GetVB11(0, 0);
	Strides[0] = (UINT)extra->m_Mesh11.GetVertexStride(0, 0);
	Offsets[0] = 0;
	DXUTGetD3D11DeviceContext()->IASetVertexBuffers(0, 1, pVB, Strides, Offsets);
	DXUTGetD3D11DeviceContext()->IASetIndexBuffer(extra->m_Mesh11.GetIB11(0), extra->m_Mesh11.GetIBFormat11(0), 0);

	//Render
	D3D11_PRIMITIVE_TOPOLOGY PrimType;
	for (UINT subset = 0; subset < extra->m_Mesh11.GetNumSubsets(0); ++subset)
	{
		// Get the subset
		SDKMESH_SUBSET *pSubset = extra->m_Mesh11.GetSubset(0, subset);

		PrimType = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		DXUTGetD3D11DeviceContext()->IASetPrimitiveTopology(PrimType);

		ID3D11ShaderResourceView* pDiffuseRV = extra->m_Mesh11.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		DXUTGetD3D11DeviceContext()->PSSetShaderResources(0, 1, &pDiffuseRV);

		DXUTGetD3D11DeviceContext()->DrawIndexed((UINT)pSubset->IndexCount, 0, (UINT)pSubset->VertexStart);
	}
	return S_OK;
}

HRESULT D3DShaderMeshNode11::VPick(Scene *pScene, RayCast *pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast) == E_FAIL)
		return E_FAIL;

	pScene->PushAndSetMatrix(m_Props.ToWorld());

	//Get the Mesh
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DSdkMeshResourceExtraData11> extra = static_pointer_cast<D3DSdkMeshResourceExtraData11>(pResourceHandle->GetExtra());

	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), &extra->m_Mesh11);
	pScene->PopMatrix();

	return hr;
}




float D3DShaderMeshNode11::CalcBoundingSphere(CDXUTSDKMesh *mesh11)
{
	float radius = 0.0f;
	for (UINT subset = 0; subset < mesh11->GetNumSubsets(0); ++subset)
	{
		Vec3 extents = mesh11->GetMeshBBoxExtents(subset);
		extents.x = abs(extents.x);
		extents.y = abs(extents.y);
		extents.z = abs(extents.z);
		radius = (radius > extents.x) ? radius : extents.x;
		radius = (radius > extents.y) ? radius : extents.y;
		radius = (radius > extents.z) ? radius : extents.z;
	}
	return radius;
}

//ASSIMP SHADER MESH CUSTOM

//
//
D3DShaderAssimpMeshNode11::D3DShaderAssimpMeshNode11(const ActorId actorId,
	WeakBaseRenderComponentPtr renderComponent,
	std::string sdkMeshFileName,
	RenderPass renderPass,
	const Mat4x4 *t)
	: SceneNode(actorId, renderComponent, renderPass, t)
{
	m_sdkMeshFileName = sdkMeshFileName;
	m_assimpMeshFileName = sdkMeshFileName;

}


//
// D3DShaderMeshNode11::VOnRestore							- Chapter 16, page 563
//
HRESULT D3DShaderAssimpMeshNode11::VOnRestore(Scene *pScene)
{
	HRESULT hr;

	V_RETURN(SceneNode::VOnRestore(pScene));

	V_RETURN(m_VertexShader.OnRestore(pScene));
	V_RETURN(m_PixelShader.OnRestore(pScene));

	// Force the Mesh to reload
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DAssimpMeshResourceExtraData11> extra = static_pointer_cast<D3DAssimpMeshResourceExtraData11>(pResourceHandle->GetExtra());

	SetRadius(CalcBoundingSphere(extra->m_assimpMesh11));

	return S_OK;
}

//
// D3DShaderMeshNode11::VRender								- Chapter 16, page 564
//
HRESULT D3DShaderAssimpMeshNode11::VRender(Scene *pScene)
{
	HRESULT hr;

	ID3D11Device* device = DXUTGetD3D11Device();
	ID3D11DeviceContext* deviceContext = DXUTGetD3D11DeviceContext();


	//setup degli shader TODO
	V_RETURN(m_VertexShader.SetupRender(pScene, this));
	V_RETURN(m_PixelShader.SetupRender(pScene, this));

	//Get the Mesh
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DAssimpMeshResourceExtraData11> extra = static_pointer_cast<D3DAssimpMeshResourceExtraData11>(pResourceHandle->GetExtra());

	// FUTURE WORK - this code WON'T be able to find texture resources referred to by the sdkmesh file 
	// in the Resource cache.

	//IA setup
	//forse va spostata in VOnRestore
	InitializeBuffers(device,&extra);

	//UINT Strides[1];
	//UINT Offsets[1];
	//ID3D11Buffer* pVB[1];
	//pVB[0] = extra->m_Mesh11.GetVB11(0, 0);
	//Strides[0] = (UINT)extra->m_Mesh11.GetVertexStride(0, 0);
	//Offsets[0] = 0;
	//deviceContext->IASetVertexBuffers(0, 1, pVB, Strides, Offsets);
	//deviceContext->IASetIndexBuffer(extra->m_Mesh11.GetIB11(0), extra->m_Mesh11.GetIBFormat11(0), 0);

	//Render
	RenderBuffers(deviceContext,&extra);



	//D3D11_PRIMITIVE_TOPOLOGY PrimType;
	//for (UINT subset = 0; subset < extra->m_Mesh11.GetNumSubsets(0); ++subset)
	//{
	//	// Get the subset
	//	SDKMESH_SUBSET *pSubset = extra->m_Mesh11.GetSubset(0, subset);

		// = CDXUTSDKMesh::GetPrimitiveType11((SDKMESH_PRIMITIVE_TYPE)pSubset->PrimitiveType);
		//deviceContext->IASetPrimitiveTopology(PrimType);

		//equivale forse a setShaderParameters()
		//ID3D11ShaderResourceView* pDiffuseRV = extra->m_Mesh11.GetMaterial(pSubset->MaterialID)->pDiffuseRV11;
		//deviceContext->PSSetShaderResources(0, 1, &pDiffuseRV);

		// Set shader texture resource in the pixel shader.
		ID3D11ShaderResourceView* texture = extra.get()->m_Texture->GetTexture();
		deviceContext->PSSetShaderResources(0, 1, &texture);

		//dovrebbe essere a posto.
		deviceContext->DrawIndexed((UINT)extra->m_NumIndicesAssimp, 0, 0);
	//}

	return S_OK;
}

HRESULT D3DShaderAssimpMeshNode11::VPick(Scene *pScene, RayCast *pRayCast)
{
	if (SceneNode::VPick(pScene, pRayCast) == E_FAIL)
		return E_FAIL;

	pScene->PushAndSetMatrix(m_Props.ToWorld());

	//Get the Mesh
	Resource resource(m_sdkMeshFileName);
	shared_ptr<ResHandle> pResourceHandle = g_pApp->m_ResCache->GetHandle(&resource);
	shared_ptr<D3DAssimpMeshResourceExtraData11> extra = static_pointer_cast<D3DAssimpMeshResourceExtraData11>(pResourceHandle->GetExtra());

	HRESULT hr = pRayCast->Pick(pScene, m_Props.ActorId(), &extra->m_Mesh11);
	pScene->PopMatrix();

	return hr;
}




float D3DShaderAssimpMeshNode11::CalcBoundingSphere(std::vector<ModelType> m_assimpMesh11)
{
	float radius = 0.0f;
	/*for (UINT subset = 0; subset < mesh11->GetNumSubsets(0); ++subset)
	{
		Vec3 extents = mesh11->GetMeshBBoxExtents(subset);
		extents.x = abs(extents.x);
		extents.y = abs(extents.y);
		extents.z = abs(extents.z);
		radius = (radius > extents.x) ? radius : extents.x;
		radius = (radius > extents.y) ? radius : extents.y;
		radius = (radius > extents.z) ? radius : extents.z;
	}*/
	return radius;
}


std::shared_ptr<D3DAssimpMeshResourceExtraData11> AssimpMeshResourceLoader::LoadModelUsingAssimp(const std::string& filename)
{
	
	std::string Filename = filename;
	Filename = "M:\\github\\visual-studio-2015\\game-engine-experimental-3\\Assets\\Art\\untitled.obj";
	Assimp::Importer Importer;
	const aiScene *pScene = NULL;
	const aiMesh *pMesh = NULL;

	std::shared_ptr<D3DAssimpMeshResourceExtraData11> extra(new D3DAssimpMeshResourceExtraData11());

	pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_FindInvalidData);

	if (!pScene)
	{
		printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
		return false;
	}

	pMesh = pScene->mMeshes[0];
	if (!pMesh)
	{
		printf("Error Finding Model In file.  Did you export an empty scene?");
		return false;
	}

	for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
	{
		if (pMesh->mFaces[i].mNumIndices == 3)
		{
			extra.get()[0].m_NumIndicesAssimp = extra.get()[0].m_NumIndicesAssimp + 3;
		}

		else
		{
			printf("Error parsing Faces. Try to Re-Export model from 3d package!");
			return false;
		}
	}

	extra.get()[0].m_NumFacesAssimp = pMesh->mNumFaces;
	extra.get()[0].m_NumVerteciesAssimp = pMesh->mNumVertices;
	
	// Create the model using the vertex count that was read in.
	//std::unique_ptr<ModelType[]> arr(new ModelType[extra.get()[0].m_NumVerteciesAssimp]);
	//std::shared_ptr<ModelType> outputModel(std::move(arr));

	//int numVertex = pMesh->mNumVertices;
	std::vector<ModelType> outputModel(pMesh->mNumVertices);

	/*if (!outputModel)
	{
		return false;
	}*/

	for (int i = 0; i < pMesh->mNumVertices; i++){

		outputModel[i].x = pMesh->mVertices[i].x;
		outputModel[i].y = pMesh->mVertices[i].y;
		outputModel[i].z = pMesh->mVertices[i].z;
		outputModel[i].tu = pMesh->mTextureCoords[0][i].x;
		outputModel[i].tv = pMesh->mTextureCoords[0][i].y;
		outputModel[i].nx = pMesh->mNormals[i].x;
		outputModel[i].ny = pMesh->mNormals[i].y;
		outputModel[i].nz = pMesh->mNormals[i].z;

		//outModel += sizeof(ModelType);
	}

	//outModel = &out;
	extra.get()[0].m_assimpMesh11 = outputModel;
	return extra;
}

bool D3DShaderAssimpMeshNode11::InitializeBuffers(ID3D11Device* device, std::shared_ptr<D3DAssimpMeshResourceExtraData11>* extra) {

	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;


	// Create the vertex array.
	vertices = new VertexType[extra->get()[0].m_NumVerteciesAssimp];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.
	indices = new unsigned long[extra->get()[0].m_NumIndicesAssimp];
	if (!indices)
	{
		return false;
	}



	// Load the vertex array and index array with data.
	for (i = 0; i<extra->get()[0].m_NumVerteciesAssimp; i++)
	{
		vertices[i].position = D3DXVECTOR3(extra->get()[0].m_assimpMesh11.at(i).x, extra->get()[0].m_assimpMesh11.at(i).y, extra->get()[0].m_assimpMesh11.at(i).z);
		vertices[i].texture = D3DXVECTOR2(extra->get()[0].m_assimpMesh11.at(i).tu, extra->get()[0].m_assimpMesh11.at(i).tv);
		vertices[i].normal = D3DXVECTOR3(extra->get()[0].m_assimpMesh11.at(i).nx, extra->get()[0].m_assimpMesh11.at(i).ny, extra->get()[0].m_assimpMesh11.at(i).nz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * extra->get()[0].m_NumVerteciesAssimp;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &extra->get()[0].m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * extra->get()[0].m_NumIndicesAssimp;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &extra->get()[0].m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

	return true;
}

void D3DShaderAssimpMeshNode11::RenderBuffers(ID3D11DeviceContext* deviceContext, std::shared_ptr<D3DAssimpMeshResourceExtraData11>* extra)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &extra->get()[0].m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(extra->get()[0].m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}
bool AssimpMeshResourceLoader::LoadTextureUsingAssimp(ID3D11Device* device, const std::string& Filename , std::shared_ptr<D3DAssimpMeshResourceExtraData11>* extra)
{
	bool result;

	Assimp::Importer Importer;
	const aiScene *pScene = NULL;
	const aiMesh *pMesh = NULL;
	TextureClass* texture = NULL;
	pScene = Importer.ReadFile(Filename.c_str(), aiProcess_Triangulate | aiProcess_ConvertToLeftHanded | aiProcess_ValidateDataStructure | aiProcess_FindInvalidData);

	if (!pScene)
	{
		printf("Error parsing '%s': '%s'\n", Filename.c_str(), Importer.GetErrorString());
		return false;
	}


	for (unsigned int i = 0; i < pScene->mNumMaterials; i++) {
		const aiMaterial* pMaterial = pScene->mMaterials[i];

		//al momento ho una sola texture, altrimenti dovrei ciclare su m_Textures[i]
		//m_Texture = NULL;

		if (pMaterial->GetTextureCount(aiTextureType_DIFFUSE) > 0) {

			aiString Path;
			if (pMaterial->GetTexture(aiTextureType_DIFFUSE, 0, &Path, NULL, NULL, NULL, NULL, NULL) == AI_SUCCESS) {
				// Create the texture object.
				texture = new TextureClass;
				if (!texture)
				{
					return false;
				}
				// Initialize the texture object.

				const char* text_char = Path.C_Str();
				size_t length = strlen(text_char);
				std::wstring text_wchar(length, L'#');
				mbstowcs(&text_wchar[0], text_char, length);

				std::wstring folder = L"../Assets/Art/";
				folder += text_wchar;
				result = texture->Initialize(device, folder.data());
				if (!result)
				{
					return false;
				}
				else
					extra->get()[0].m_Texture = texture;

			}
		}
	}

	return true;
}
