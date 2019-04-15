//***************************************************************************************
// Filename: Rubix.cpp 
// Author: Frank Luna, Jamie Neill
// Copyright: Sections: Frank Luna, 2015, All Rights Reserved.
//                      Ulster University.
// Description: Driver class based on Frank Luna's Class driven demo framework
//              from introduction to 3d game programming with directx 12.
// Date: 14/04/2019
//***************************************************************************************

#include "Common/d3dApp.h"
#include "Common/MathHelper.h"
#include "Common/UploadBuffer.h"
#include "Common/GeometryGenerator.h"
#include "FrameResource.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;
using namespace DirectX::PackedVector;

#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "D3D12.lib")

const int gNumFrameResources = 3;

//Global variable for the amount to rotate the entire cube by.
float rotated{ 0.3f };

// Lightweight structure stores parameters to draw a shape.  This will
// vary from app-to-app.
struct RenderItem
{
	RenderItem() = default;

	// World matrix of the shape that describes the object's local space
	// relative to the world space, which defines the position, orientation,
	// and scale of the object in the world.
	XMFLOAT4X4 World = MathHelper::Identity4x4();

	XMFLOAT4X4 TexTransform = MathHelper::Identity4x4();

	// Dirty flag indicating the object data has changed and we need to update the constant buffer.
	// Because we have an object cbuffer for each FrameResource, we have to apply the
	// update to each FrameResource.  Thus, when we modify obect data we should set 
	// NumFramesDirty = gNumFrameResources so that each frame resource gets the update.
	int NumFramesDirty = gNumFrameResources;

	// Index into GPU constant buffer corresponding to the ObjectCB for this render item.
	UINT ObjCBIndex = -1;

	Material* Mat = nullptr;
	MeshGeometry* Geo = nullptr;

	// Primitive topology.
	D3D12_PRIMITIVE_TOPOLOGY PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;

	// DrawIndexedInstanced parameters.
	UINT IndexCount = 0;
	UINT StartIndexLocation = 0;
	int BaseVertexLocation = 0;
};

class Rubix : public D3DApp
{
public:
	Rubix(HINSTANCE hInstance);
	Rubix(const Rubix& rhs) = delete;
	Rubix& operator=(const Rubix& rhs) = delete;
	~Rubix();

	virtual bool Initialize()override;

private:
	virtual void OnResize()override;
	virtual void Update(const GameTimer& gt)override;
	virtual void Draw(const GameTimer& gt)override;

	virtual void OnMouseDown(WPARAM btnState, int x, int y)override;
	virtual void OnMouseUp(WPARAM btnState, int x, int y)override;
	virtual void OnMouseMove(WPARAM btnState, int x, int y)override;

	void OnKeyboardInput(const GameTimer& gt);
	void UpdateCamera(const GameTimer& gt);
	void UpdateObjects(const GameTimer & gt);
	void RotateThird(const GameTimer & gt);
	void UpdateLastFace(char face);
	void UpdateObjectCBs(const GameTimer& gt);
	void UpdateMaterialCBs(const GameTimer& gt);
	void UpdateMainPassCB(const GameTimer& gt);

	void LoadTextures();
	void BuildRootSignature();
	void BuildDescriptorHeaps();
	void BuildShadersAndInputLayout();
	void BuildShapeGeometry();
	void BuildPSOs();
	void BuildFrameResources();
	void BuildMaterials();
	void BuildRenderItems();
	void DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems);

	std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers();

private:

	std::vector<std::unique_ptr<FrameResource>> mFrameResources;
	FrameResource* mCurrFrameResource = nullptr;
	int mCurrFrameResourceIndex = 0;

	UINT mCbvSrvDescriptorSize = 0;

	ComPtr<ID3D12RootSignature> mRootSignature = nullptr;

	ComPtr<ID3D12DescriptorHeap> mSrvDescriptorHeap = nullptr;

	std::unordered_map<std::string, std::unique_ptr<MeshGeometry>> mGeometries;
	std::unordered_map<std::string, std::unique_ptr<Material>> mMaterials;
	std::unordered_map<std::string, std::unique_ptr<Texture>> mTextures;
	std::unordered_map<std::string, ComPtr<ID3DBlob>> mShaders;

	std::vector<D3D12_INPUT_ELEMENT_DESC> mInputLayout;

	ComPtr<ID3D12PipelineState> mOpaquePSO = nullptr;
	ComPtr<ID3D12PipelineState> mwireframePSO = nullptr;
	ComPtr<ID3D12PipelineState> mfrontFacePSO = nullptr;
	ComPtr<ID3D12PipelineState> mbackFacePSO = nullptr;

	// List of all the render items.
	std::vector<std::unique_ptr<RenderItem>> mAllRitems;

	// Render items divided by PSO.
	std::vector<RenderItem*> mOpaqueRitems;

	PassConstants mMainPassCB;

	XMFLOAT3 mEyePos = { 0.0f, 0.0f, 0.0f };
	XMFLOAT4X4 mView = MathHelper::Identity4x4();
	XMFLOAT4X4 mProj = MathHelper::Identity4x4();

	float mTheta = 1.3f*XM_PI;
	float mPhi = 0.4f*XM_PI;
	float mRadius = 2.5f;

	float topRotation = 90.0f;
	float bottomRotation = 90.0f;
	float leftRotation = 90.0f;
	float rightRotation = 90.0f;
	float backRotation = 90.0f;
	float frontRotation = 90.0f;

	POINT mLastMousePos;
};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance,
	PSTR cmdLine, int showCmd)
{
	// Enable run-time memory check for debug builds.
#if defined(DEBUG) | defined(_DEBUG)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif

	try
	{
		Rubix theApp(hInstance);
		if (!theApp.Initialize())
			return 0;

		return theApp.Run();
	}
	catch (DxException& e)
	{
		MessageBox(nullptr, e.ToString().c_str(), L"HR Failed", MB_OK);
		return 0;
	}
}

Rubix::Rubix(HINSTANCE hInstance)
	: D3DApp(hInstance)
{
	//Set the window caption
	mMainWndCaption = L"COM428 Assignment";
}

Rubix::~Rubix()
{
	if (md3dDevice != nullptr)
		FlushCommandQueue();
}

bool Rubix::Initialize()
{
	if (!D3DApp::Initialize())
		return false;

	// Reset the command list to prep for initialization commands.
	ThrowIfFailed(mCommandList->Reset(mDirectCmdListAlloc.Get(), nullptr));

	// Get the increment size of a descriptor in this heap type.  This is hardware specific, 
	// so we have to query this information.
	mCbvSrvDescriptorSize = md3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);


	LoadTextures();
	BuildRootSignature();
	BuildDescriptorHeaps();
	BuildShadersAndInputLayout();
	BuildShapeGeometry();
	BuildMaterials();
	BuildRenderItems();
	BuildFrameResources();
	BuildPSOs();

	//Send a mouse click to force camera to clamp to the limit.
	OnMouseMove(MK_RBUTTON, 0, 0);

	// Execute the initialization commands.
	ThrowIfFailed(mCommandList->Close());
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Wait until initialization is complete.
	FlushCommandQueue();

	return true;
}

void Rubix::OnResize()
{
	D3DApp::OnResize();

	// The window resized, so update the aspect ratio and recompute the projection matrix.
	XMMATRIX P = XMMatrixPerspectiveFovLH(0.25f*MathHelper::Pi, AspectRatio(), 1.0f, 1000.0f);
	XMStoreFloat4x4(&mProj, P);
}

void Rubix::Update(const GameTimer& gt)
{
	/*Check if the cube needs reset*/
	if (appInfo.needsReset()) {
		//Create a new app info object and override the current one with it
		RubixCubeAppInfo resetInfo{};
		appInfo = resetInfo;
		//Remove the render items
		mOpaqueRitems.clear();
		//Build them again
		BuildRenderItems();
		//Draw them in their new positions
		Draw(gt);
		//Reset the initial values mEyePos is calculated from
		mTheta = 1.3f*XM_PI;
		mPhi = 0.4f*XM_PI;
		mRadius = 2.5f;
		//Send mouse click to force camera to clamp limit
		OnMouseMove(MK_RBUTTON, 0, 0);
	}
	OnKeyboardInput(gt);
	UpdateCamera(gt);

	// Cycle through the circular frame resource array.
	mCurrFrameResourceIndex = (mCurrFrameResourceIndex + 1) % gNumFrameResources;
	mCurrFrameResource = mFrameResources[mCurrFrameResourceIndex].get();

	// Has the GPU finished processing the commands of the current frame resource?
	// If not, wait until the GPU has completed commands up to this fence point.
	if (mCurrFrameResource->Fence != 0 && mFence->GetCompletedValue() < mCurrFrameResource->Fence)
	{
		HANDLE eventHandle = CreateEventEx(nullptr, false, false, EVENT_ALL_ACCESS);
		ThrowIfFailed(mFence->SetEventOnCompletion(mCurrFrameResource->Fence, eventHandle));
		WaitForSingleObject(eventHandle, INFINITE);
		CloseHandle(eventHandle);
	}

	UpdateObjects(gt);
	RotateThird(gt);
	UpdateObjectCBs(gt);
	UpdateMaterialCBs(gt);
	UpdateMainPassCB(gt);
}

void Rubix::Draw(const GameTimer& gt)
{
	auto cmdListAlloc = mCurrFrameResource->CmdListAlloc;

	// Reuse the memory associated with command recording.
	// We can only reset when the associated command lists have finished execution on the GPU.
	ThrowIfFailed(cmdListAlloc->Reset());

	// A command list can be reset after it has been added to the command queue via ExecuteCommandList.
	// Reusing the command list reuses memory.
	ThrowIfFailed(mCommandList->Reset(cmdListAlloc.Get(), mOpaquePSO.Get()));

	mCommandList->RSSetViewports(1, &mScreenViewport);
	mCommandList->RSSetScissorRects(1, &mScissorRect);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	// Clear the back buffer and depth buffer.
	mCommandList->ClearRenderTargetView(CurrentBackBufferView(), Colors::LightSteelBlue, 0, nullptr);
	mCommandList->ClearDepthStencilView(DepthStencilView(), D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

	// Specify the buffers we are going to render to.
	mCommandList->OMSetRenderTargets(1, &CurrentBackBufferView(), true, &DepthStencilView());

	ID3D12DescriptorHeap* descriptorHeaps[] = { mSrvDescriptorHeap.Get() };
	mCommandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

	mCommandList->SetGraphicsRootSignature(mRootSignature.Get());

	auto passCB = mCurrFrameResource->PassCB->Resource();
	mCommandList->SetGraphicsRootConstantBufferView(2, passCB->GetGPUVirtualAddress());

	/*Check if any of the keys predefined in the brief to change the Pipeline
	state have been depressed and switch the Pipeline State Object to the
	relevant one.*/
	if (appInfo.getFill() == 'w') {
		mCommandList->SetPipelineState(mwireframePSO.Get());
	}
	if (appInfo.getFill() == 's') {
		mCommandList->SetPipelineState(mOpaquePSO.Get());
	}
	if (appInfo.getCull() == 'b') {
		mCommandList->SetPipelineState(mbackFacePSO.Get());
	}
	if (appInfo.getCull() == 'f') {
		mCommandList->SetPipelineState(mfrontFacePSO.Get());
	}
	if (appInfo.getCull() == 'n') {
		mCommandList->SetPipelineState(mOpaquePSO.Get());
	}

	/*Draw the render items in the opaque item list regardless of pipeline state
	as even though their Fill and cull modes can be changed, the objects are still
	opaque*/
	DrawRenderItems(mCommandList.Get(), mOpaqueRitems);

	// Indicate a state transition on the resource usage.
	mCommandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(CurrentBackBuffer(),
		D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// Done recording commands.
	ThrowIfFailed(mCommandList->Close());

	// Add the command list to the queue for execution.
	ID3D12CommandList* cmdsLists[] = { mCommandList.Get() };
	mCommandQueue->ExecuteCommandLists(_countof(cmdsLists), cmdsLists);

	// Swap the back and front buffers
	ThrowIfFailed(mSwapChain->Present(0, 0));
	mCurrBackBuffer = (mCurrBackBuffer + 1) % SwapChainBufferCount;

	// Advance the fence value to mark commands up to this fence point.
	mCurrFrameResource->Fence = ++mCurrentFence;

	// Add an instruction to the command queue to set a new fence point. 
	// Because we are on the GPU timeline, the new fence point won't be 
	// set until the GPU finishes processing all the commands prior to this Signal().
	mCommandQueue->Signal(mFence.Get(), mCurrentFence);
}

void Rubix::OnMouseDown(WPARAM btnState, int x, int y)
{
	mLastMousePos.x = x;
	mLastMousePos.y = y;

	SetCapture(mhMainWnd);
}

void Rubix::OnMouseUp(WPARAM btnState, int x, int y)
{
	ReleaseCapture();
}

void Rubix::OnMouseMove(WPARAM btnState, int x, int y)
{
	if ((btnState & MK_LBUTTON) != 0)
	{
		// Make each pixel correspond to a quarter of a degree.
		float dx = XMConvertToRadians(0.25f*static_cast<float>(x - mLastMousePos.x));
		float dy = XMConvertToRadians(0.25f*static_cast<float>(y - mLastMousePos.y));

		// Update angles based on input to orbit camera around box.
		mTheta += dx;
		mPhi += dy;

		// Restrict the angle mPhi.
		mPhi = MathHelper::Clamp(mPhi, 0.1f, MathHelper::Pi - 0.1f);
	}
	else if ((btnState & MK_RBUTTON) != 0)
	{
		// Make each pixel correspond to 0.2 unit in the scene.
		float dx = 0.05f*static_cast<float>(x - mLastMousePos.x);
		float dy = 0.05f*static_cast<float>(y - mLastMousePos.y);

		// Update the camera radius based on input.
		mRadius += dx - dy;

		// Restrict the radius.
		mRadius = MathHelper::Clamp(mRadius, 9.0f, 1500.0f);
	}

	mLastMousePos.x = x;
	mLastMousePos.y = y;
}

void Rubix::OnKeyboardInput(const GameTimer& gt)
{
	/*Check if any relevant keys return a high bit*/

	//Move forward
	if (GetAsyncKeyState(VK_UP) & 0x8000) {
		mEyePos.z += (10.0f*gt.DeltaTime());
		appInfo.setCameraPosition(6);
		UpdateCamera(gt);
	}
	//Move Back
	if (GetAsyncKeyState(VK_DOWN) & 0x8000) {
		mEyePos.z += (-10.0f*gt.DeltaTime());
		appInfo.setCameraPosition(6);
		UpdateCamera(gt);
	}
	//Move Left
	if (GetAsyncKeyState(VK_LEFT) & 0x8000) {
		mEyePos.x += (-10.0f*gt.DeltaTime());
		appInfo.setCameraPosition(6);
		UpdateCamera(gt);
	}
	//Move Right
	if (GetAsyncKeyState(VK_RIGHT) & 0x8000) {
		mEyePos.x += (10.0f*gt.DeltaTime());
		appInfo.setCameraPosition(6);
		UpdateCamera(gt);
	}

	//Enable or disable rotation selection across an axis
	if (GetAsyncKeyState('R') & 0x8000)
		(appInfo.getRotatable()) ? appInfo.setRotatable(false) : appInfo.setRotatable(true);

	//Select an axis to rotate across
	if (GetAsyncKeyState('X') & 0x8000 && appInfo.getRotatable())
		appInfo.setRotationAxis('x');

	if (GetAsyncKeyState('Y') & 0x8000 && appInfo.getRotatable())
		appInfo.setRotationAxis('y');

	if (GetAsyncKeyState('Z') & 0x8000 && appInfo.getRotatable())
		appInfo.setRotationAxis('z');

	//Raise the reset flag
	if (GetAsyncKeyState('I') & 0x8000)
		appInfo.needsReset(true);
	//Change the render mode to Solid
	if (GetAsyncKeyState('S') & 0x8000)
		appInfo.setRenderMode('s');
	//Change the render mode to wireframe
	if (GetAsyncKeyState('W') & 0x8000)
		appInfo.setRenderMode('w');
	//Set the cull mode to none
	if (GetAsyncKeyState('N') & 0x8000)
		appInfo.setCullMode('n');
	//Set the cull mode to front face
	if (GetAsyncKeyState('F') & 0x8000)
		appInfo.setCullMode('f');
	//Set the cull mode to back face
	if (GetAsyncKeyState('B') & 0x8000)
		appInfo.setCullMode('b');
	//View the front of the cube
	if (GetAsyncKeyState('1') & 0x8000)
		appInfo.setCameraPosition(1);
	//View the top of the cube
	if (GetAsyncKeyState('2') & 0x8000)
		appInfo.setCameraPosition(2);
	//View the right of the cube
	if (GetAsyncKeyState('3') & 0x8000)
		appInfo.setCameraPosition(3);
	//Rotate the front face 90 degrees
	if (GetAsyncKeyState('4') & 0x8000)
		appInfo.setSelectedThird('f');
	//Rotate the left face 90 degrees
	if (GetAsyncKeyState('5') & 0x8000)
		appInfo.setSelectedThird('l');
	//Rotate the right face 90 degrees
	if (GetAsyncKeyState('6') & 0x8000)
		appInfo.setSelectedThird('r');
	//Rotate the back face 90 degrees
	if (GetAsyncKeyState('7') & 0x8000)
		appInfo.setSelectedThird('b');
	//Rotate the top face 90 degrees
	if (GetAsyncKeyState('8') & 0x8000)
		appInfo.setSelectedThird('t');
	//Rotate the bottom face 90 degrees
	if (GetAsyncKeyState('9') & 0x8000)
		appInfo.setSelectedThird('d');
	//Exit orthographic view
	if (GetAsyncKeyState('0') & 0x8000)
		appInfo.setCameraPosition(0);

}

void Rubix::UpdateCamera(const GameTimer& gt)
{
	// Convert Spherical to Cartesian coordinates.
	XMVECTOR up;
	//Set the eye position based on the camera position variable
	switch (appInfo.getCameraPosition()) {
	case 1:
		mEyePos.x = 0.0f;
		mEyePos.y = 0.0f;
		mEyePos.z = -6.0f;
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	case 2:
		mEyePos.x = 0.0f;
		mEyePos.y = 6.0f;
		mEyePos.z = 0.0f;
		up = XMVectorSet(0.0f, 0.0f, 1.0f, 1.0f);
		break;
	case 3:
		mEyePos.x = 6.0f;
		mEyePos.y = 0.0f;
		mEyePos.z = 0.0f;
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	case 0:
		mEyePos.x = mRadius * sinf(mPhi)*cosf(mTheta);
		mEyePos.z = mRadius * sinf(mPhi)*sinf(mTheta);
		mEyePos.y = mRadius * cosf(mPhi);
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	default:
		up = XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f);
		break;
	}

	// Build the view matrix.

	XMVECTOR pos = XMVectorSet(mEyePos.x, mEyePos.y, mEyePos.z, 1.0f);
	XMVECTOR target = XMVectorZero();


	XMMATRIX view = XMMatrixLookAtLH(pos, target, up);
	XMStoreFloat4x4(&mView, view);


}

void Rubix::UpdateObjects(const GameTimer& gt) {
	//Check the case
	XMMATRIX world;
	XMMATRIX rot;
	ObjectConstants objConstants;
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	//Rotates each cube based on the axis for the whole cube to rotate across
	switch (appInfo.rotationAxis()) {
	case 'x':
		for (auto &e : mAllRitems) {
			world = XMLoadFloat4x4(&e->World);
			rot = XMMatrixRotationX(XMConvertToRadians(rotated));
			rotated += 0.0003f;
			world *= rot;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
		}
		break;
	case 'y':
		for (auto &e : mAllRitems) {
			world = XMLoadFloat4x4(&e->World);
			rot = XMMatrixRotationY(XMConvertToRadians(rotated));
			rotated += 0.0003f;
			world *= rot;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
		}
		break;
	case 'z':
		for (auto &e : mAllRitems) {
			world = XMLoadFloat4x4(&e->World);
			rot = XMMatrixRotationZ(XMConvertToRadians(rotated));
			rotated += 0.0003f;
			world *= rot;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			currObjectCB->CopyData(e->ObjCBIndex, objConstants);
		}
		break;
	default:
		rot = XMMatrixRotationX(0);
	}

	//Update the world matrix for the RIs
}

void Rubix::RotateThird(const GameTimer&gt) {
	//Set up to manipulate the cubes
	XMMATRIX world;
	XMMATRIX rot;
	ObjectConstants objConstants;
	//Track which face last rotated
	char lastFace = ' ';
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	//Check which third has been selected
	for (auto& e : mAllRitems) {
		switch (appInfo.getSelectedThird()) {
		case 'f':
			if (e->ObjCBIndex == appInfo.frontIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.frontIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationZ(XMConvertToRadians(topRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		case 'l':
			if (e->ObjCBIndex == appInfo.leftIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.leftIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationX(XMConvertToRadians(bottomRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		case 'r':
			if (e->ObjCBIndex == appInfo.rightIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.rightIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationX(XMConvertToRadians(bottomRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		case 'b':
			if (e->ObjCBIndex == appInfo.backIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.backIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationZ(XMConvertToRadians(bottomRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		case 't':
			if (e->ObjCBIndex == appInfo.topIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.topIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationY(XMConvertToRadians(bottomRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		case 'd':
			if (e->ObjCBIndex == appInfo.bottomIndicies_.at(0) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(1) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(2) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(3) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(4) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(5) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(6) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(7) ||
				e->ObjCBIndex == appInfo.bottomIndicies_.at(8)) {
				world = XMLoadFloat4x4(&e->World);
				rot = XMMatrixRotationY(XMConvertToRadians(bottomRotation));
				world *= rot;
				XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
				currObjectCB->CopyData(e->ObjCBIndex, objConstants);
			}
			break;
		}
	}
	appInfo.setSelectedThird(' ');
}
void Rubix::UpdateLastFace(char face) {
	//Update the CB Indicies that compose each face after a rotation
	switch (face) {
	case 'd':
		bottomRotation += 90.0f;
		switch ((int)bottomRotation) {
		case 180:
			appInfo.bottomIndicies_.at(0) = 2;
			appInfo.bottomIndicies_.at(1) = 1;
			appInfo.bottomIndicies_.at(2) = 0;
			appInfo.bottomIndicies_.at(3) = 11;
			appInfo.bottomIndicies_.at(5) = 9;
			appInfo.bottomIndicies_.at(6) = 20;
			appInfo.bottomIndicies_.at(7) = 19;
			appInfo.bottomIndicies_.at(8) = 18;
			break;
		case 270:
			appInfo.bottomIndicies_.at(0);
			appInfo.bottomIndicies_.at(1);
			appInfo.bottomIndicies_.at(2);
			appInfo.bottomIndicies_.at(3);
			appInfo.bottomIndicies_.at(5);
			appInfo.bottomIndicies_.at(6);
			appInfo.bottomIndicies_.at(7);
			appInfo.bottomIndicies_.at(8);
			break;
		default:
			appInfo.bottomIndicies_.at(0);
			appInfo.bottomIndicies_.at(1);
			appInfo.bottomIndicies_.at(2);
			appInfo.bottomIndicies_.at(3);
			appInfo.bottomIndicies_.at(5);
			appInfo.bottomIndicies_.at(6);
			appInfo.bottomIndicies_.at(7);
			appInfo.bottomIndicies_.at(8);
			break;
		}

		break;
	}
}
void Rubix::UpdateObjectCBs(const GameTimer& gt)
{
	auto currObjectCB = mCurrFrameResource->ObjectCB.get();
	for (auto& e : mAllRitems)
	{
		// Only update the cbuffer data if the constants have changed.  
		// This needs to be tracked per frame resource.
		if (e->NumFramesDirty > 0)
		{
			XMMATRIX world = XMLoadFloat4x4(&e->World);
			XMMATRIX texTransform = XMLoadFloat4x4(&e->TexTransform);

			ObjectConstants objConstants;
			XMStoreFloat4x4(&objConstants.World, XMMatrixTranspose(world));
			XMStoreFloat4x4(&objConstants.TexTransform, XMMatrixTranspose(texTransform));

			currObjectCB->CopyData(e->ObjCBIndex, objConstants);

			// Next FrameResource need to be updated too.
			e->NumFramesDirty--;
		}
	}
}

void Rubix::UpdateMaterialCBs(const GameTimer& gt)
{
	auto currMaterialCB = mCurrFrameResource->MaterialCB.get();
	for (auto& e : mMaterials)
	{
		// Only update the cbuffer data if the constants have changed.  If the cbuffer
		// data changes, it needs to be updated for each FrameResource.
		Material* mat = e.second.get();
		if (mat->NumFramesDirty > 0)
		{
			XMMATRIX matTransform = XMLoadFloat4x4(&mat->MatTransform);

			MaterialConstants matConstants;
			matConstants.DiffuseAlbedo = mat->DiffuseAlbedo;
			matConstants.FresnelR0 = mat->FresnelR0;
			matConstants.Roughness = mat->Roughness;
			XMStoreFloat4x4(&matConstants.MatTransform, XMMatrixTranspose(matTransform));

			currMaterialCB->CopyData(mat->MatCBIndex, matConstants);

			// Next FrameResource need to be updated too.
			mat->NumFramesDirty--;
		}
	}
}

void Rubix::UpdateMainPassCB(const GameTimer& gt)
{
	XMMATRIX view = XMLoadFloat4x4(&mView);
	XMMATRIX proj = XMLoadFloat4x4(&mProj);

	XMMATRIX viewProj = XMMatrixMultiply(view, proj);
	XMMATRIX invView = XMMatrixInverse(&XMMatrixDeterminant(view), view);
	XMMATRIX invProj = XMMatrixInverse(&XMMatrixDeterminant(proj), proj);
	XMMATRIX invViewProj = XMMatrixInverse(&XMMatrixDeterminant(viewProj), viewProj);


	XMStoreFloat4x4(&mMainPassCB.View, XMMatrixTranspose(view));
	XMStoreFloat4x4(&mMainPassCB.InvView, XMMatrixTranspose(invView));
	XMStoreFloat4x4(&mMainPassCB.Proj, XMMatrixTranspose(proj));
	XMStoreFloat4x4(&mMainPassCB.InvProj, XMMatrixTranspose(invProj));
	XMStoreFloat4x4(&mMainPassCB.ViewProj, XMMatrixTranspose(viewProj));
	XMStoreFloat4x4(&mMainPassCB.InvViewProj, XMMatrixTranspose(invViewProj));
	mMainPassCB.EyePosW = mEyePos;
	mMainPassCB.RenderTargetSize = XMFLOAT2((float)mClientWidth, (float)mClientHeight);
	mMainPassCB.InvRenderTargetSize = XMFLOAT2(1.0f / mClientWidth, 1.0f / mClientHeight);
	mMainPassCB.NearZ = 1.0f;
	mMainPassCB.FarZ = 1000.0f;
	mMainPassCB.TotalTime = gt.TotalTime();
	mMainPassCB.DeltaTime = gt.DeltaTime();
	//mMainPassCB.AmbientLight = { 0.25f, 0.25f, 0.35f, 1.0f };

	//Directional Lights
	mMainPassCB.Lights[0].Direction = {-3.0f, 0.0f, -3.0f };
	mMainPassCB.Lights[0].Strength = { 0.3f, 0.3f, 0.3f };

	mMainPassCB.Lights[1].Direction = { 3.0f, 0.0f, 3.0f };
	mMainPassCB.Lights[1].Strength = { 0.3f, 0.3f, 0.3f };

	//Point Lights
	mMainPassCB.Lights[2].Position = { 0.0f, -6.0f, 0.0f };
	mMainPassCB.Lights[2].Strength = { 2.0f,2.0f,2.0f };

	//Spotlight
	mMainPassCB.Lights[3].Position = { 0.0f,6.0f,0.0f };
	mMainPassCB.Lights[3].Direction = { 0.0f, -1.0f, 0.0f };
	mMainPassCB.Lights[3].Strength = { 1.0f,1.0f,1.0f };
	mMainPassCB.Lights[3].SpotPower = 0.001f;


	


	auto currPassCB = mCurrFrameResource->PassCB.get();
	currPassCB->CopyData(0, mMainPassCB);
}

void Rubix::LoadTextures()
{
	//Load the rubiks cube texture atlas
	auto cubeTextureAtlas = std::make_unique<Texture>();
	cubeTextureAtlas->Name = "cubeTextureAtlas";
	cubeTextureAtlas->Filename = L"Textures/atlas.dds";
	ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(md3dDevice.Get(),
		mCommandList.Get(), cubeTextureAtlas->Filename.c_str(),
		cubeTextureAtlas->Resource, cubeTextureAtlas->UploadHeap));

	mTextures[cubeTextureAtlas->Name] = std::move(cubeTextureAtlas);

}

void Rubix::BuildRootSignature()
{
	CD3DX12_DESCRIPTOR_RANGE texTable;
	texTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0);

	// Root parameter can be a table, root descriptor or root constants.
	CD3DX12_ROOT_PARAMETER slotRootParameter[4];

	// Perfomance TIP: Order from most frequent to least frequent.
	slotRootParameter[0].InitAsDescriptorTable(1, &texTable, D3D12_SHADER_VISIBILITY_PIXEL);
	slotRootParameter[1].InitAsConstantBufferView(0);
	slotRootParameter[2].InitAsConstantBufferView(1);
	slotRootParameter[3].InitAsConstantBufferView(2);

	auto staticSamplers = GetStaticSamplers();

	// A root signature is an array of root parameters.
	CD3DX12_ROOT_SIGNATURE_DESC rootSigDesc(4, slotRootParameter,
		(UINT)staticSamplers.size(), staticSamplers.data(),
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	// create a root signature with a single slot which points to a descriptor range consisting of a single constant buffer
	ComPtr<ID3DBlob> serializedRootSig = nullptr;
	ComPtr<ID3DBlob> errorBlob = nullptr;
	HRESULT hr = D3D12SerializeRootSignature(&rootSigDesc, D3D_ROOT_SIGNATURE_VERSION_1,
		serializedRootSig.GetAddressOf(), errorBlob.GetAddressOf());

	if (errorBlob != nullptr)
	{
		::OutputDebugStringA((char*)errorBlob->GetBufferPointer());
	}
	ThrowIfFailed(hr);

	ThrowIfFailed(md3dDevice->CreateRootSignature(
		0,
		serializedRootSig->GetBufferPointer(),
		serializedRootSig->GetBufferSize(),
		IID_PPV_ARGS(mRootSignature.GetAddressOf())));
}

void Rubix::BuildDescriptorHeaps()
{
	//
	// Create the SRV heap.
	//
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = 1;
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	ThrowIfFailed(md3dDevice->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&mSrvDescriptorHeap)));

	//
	// Fill out the heap with actual descriptors.
	//
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDescriptor(mSrvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	auto cubeTextureAtlas = mTextures["cubeTextureAtlas"]->Resource;

	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Format = cubeTextureAtlas->GetDesc().Format;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = cubeTextureAtlas->GetDesc().MipLevels;
	srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

	md3dDevice->CreateShaderResourceView(cubeTextureAtlas.Get(), &srvDesc, hDescriptor);

}

void Rubix::BuildShadersAndInputLayout()
{
	mShaders["standardVS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "VS", "vs_5_0");
	mShaders["opaquePS"] = d3dUtil::CompileShader(L"Shaders\\Default.hlsl", nullptr, "PS", "ps_5_0");

	mInputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
	};
}

void Rubix::BuildShapeGeometry()
{
	GeometryGenerator geoGen;
	GeometryGenerator::MeshData box = geoGen.CreateBox(1.0f, 1.0f, 1.0f, 3);

	SubmeshGeometry boxSubmesh;
	boxSubmesh.IndexCount = (UINT)box.Indices32.size();
	boxSubmesh.StartIndexLocation = 0;
	boxSubmesh.BaseVertexLocation = 0;

	std::vector<Vertex> vertices(box.Vertices.size());


	for (size_t i = 0; i < box.Vertices.size(); ++i)
	{
		vertices[i].Pos = box.Vertices[i].Position;
		vertices[i].Normal = box.Vertices[i].Normal;
		vertices[i].TexC = box.Vertices[i].TexC;
	}

	std::vector<std::uint16_t> indices = box.GetIndices16();


	const UINT vbByteSize = (UINT)vertices.size() * sizeof(Vertex);
	const UINT ibByteSize = (UINT)indices.size() * sizeof(std::uint16_t);

	auto geo = std::make_unique<MeshGeometry>();
	geo->Name = "boxGeo";

	ThrowIfFailed(D3DCreateBlob(vbByteSize, &geo->VertexBufferCPU));
	CopyMemory(geo->VertexBufferCPU->GetBufferPointer(), vertices.data(), vbByteSize);

	ThrowIfFailed(D3DCreateBlob(ibByteSize, &geo->IndexBufferCPU));
	CopyMemory(geo->IndexBufferCPU->GetBufferPointer(), indices.data(), ibByteSize);

	geo->VertexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), vertices.data(), vbByteSize, geo->VertexBufferUploader);

	geo->IndexBufferGPU = d3dUtil::CreateDefaultBuffer(md3dDevice.Get(),
		mCommandList.Get(), indices.data(), ibByteSize, geo->IndexBufferUploader);

	geo->VertexByteStride = sizeof(Vertex);
	geo->VertexBufferByteSize = vbByteSize;
	geo->IndexFormat = DXGI_FORMAT_R16_UINT;
	geo->IndexBufferByteSize = ibByteSize;

	geo->DrawArgs["box"] = boxSubmesh;

	mGeometries[geo->Name] = std::move(geo);
}

void Rubix::BuildPSOs()
{
	D3D12_GRAPHICS_PIPELINE_STATE_DESC opaquePsoDesc;

	//
	// PSO for opaque objects. Has none cull mode integrated
	//
	ZeroMemory(&opaquePsoDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	opaquePsoDesc.InputLayout = { mInputLayout.data(), (UINT)mInputLayout.size() };
	opaquePsoDesc.pRootSignature = mRootSignature.Get();
	opaquePsoDesc.VS =
	{
		reinterpret_cast<BYTE*>(mShaders["standardVS"]->GetBufferPointer()),
		mShaders["standardVS"]->GetBufferSize()
	};
	opaquePsoDesc.PS =
	{
		reinterpret_cast<BYTE*>(mShaders["opaquePS"]->GetBufferPointer()),
		mShaders["opaquePS"]->GetBufferSize()
	};
	opaquePsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	opaquePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	opaquePsoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	opaquePsoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	opaquePsoDesc.SampleMask = UINT_MAX;
	opaquePsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	opaquePsoDesc.NumRenderTargets = 1;
	opaquePsoDesc.RTVFormats[0] = mBackBufferFormat;
	opaquePsoDesc.SampleDesc.Count = m4xMsaaState ? 4 : 1;
	opaquePsoDesc.SampleDesc.Quality = m4xMsaaState ? (m4xMsaaQuality - 1) : 0;
	opaquePsoDesc.DSVFormat = mDepthStencilFormat;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&opaquePsoDesc, IID_PPV_ARGS(&mOpaquePSO)));

	//
   // PSO for wireframe objects.
   //
	D3D12_GRAPHICS_PIPELINE_STATE_DESC wireframePsoDesc = opaquePsoDesc;
	wireframePsoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&wireframePsoDesc, IID_PPV_ARGS(&mwireframePSO)));

	//
	// PSO for front face culling.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC frontFacePsoDesc = opaquePsoDesc;
	frontFacePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&frontFacePsoDesc, IID_PPV_ARGS(&mfrontFacePSO)));
	//
	// PSO for backface culling.
	//
	D3D12_GRAPHICS_PIPELINE_STATE_DESC backFacePsoDesc = opaquePsoDesc;
	backFacePsoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	ThrowIfFailed(md3dDevice->CreateGraphicsPipelineState(&backFacePsoDesc, IID_PPV_ARGS(&mbackFacePSO)));
}

void Rubix::BuildFrameResources()
{
	for (int i = 0; i < gNumFrameResources; ++i)
	{
		mFrameResources.push_back(std::make_unique<FrameResource>(md3dDevice.Get(),
			1, (UINT)mAllRitems.size(), (UINT)mMaterials.size()));
	}
}

void Rubix::BuildMaterials()
{
	//Create a material for the lights to interact with
	auto rubixCube = std::make_unique<Material>();
	rubixCube->Name = "rubixCube";
	rubixCube->MatCBIndex = 0;
	rubixCube->DiffuseSrvHeapIndex = 0;
	rubixCube->DiffuseAlbedo = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	rubixCube->FresnelR0 = XMFLOAT3(0.05f, 0.05f, 0.05f);
	rubixCube->Roughness = 0.2f;

	mMaterials["rubixCube"] = std::move(rubixCube);

}

void Rubix::BuildRenderItems()
{
	/*Create an int to assign as the Constant Buffer index for each new object
	and set it to 0 to allow the first object to have CB Index 0*/
	int object{ 0 };
	/*Use three nested for loops to create 3^3 individual cubes*/
	for (float x = -1.0f; x <= 1.0f; x += 1.0f) {
		for (float y = -1.0f; y <= 1.0f; y += 1.0f) {
			for (float z = -1.0f; z <= 1.0f; z += 1.0f) {
				auto boxRitem = std::make_unique<RenderItem>();
				XMStoreFloat4x4(&boxRitem->World, XMMatrixScaling(1.0f, 1.0f, 1.0f)*XMMatrixTranslation(x, y, z));
				boxRitem->ObjCBIndex = object;
				boxRitem->Mat = mMaterials["rubixCube"].get();
				boxRitem->Geo = mGeometries["boxGeo"].get();
				boxRitem->PrimitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
				boxRitem->IndexCount = boxRitem->Geo->DrawArgs["box"].IndexCount;
				boxRitem->StartIndexLocation = boxRitem->Geo->DrawArgs["box"].StartIndexLocation;
				boxRitem->BaseVertexLocation = boxRitem->Geo->DrawArgs["box"].BaseVertexLocation;
				mAllRitems.push_back(std::move(boxRitem));
				object++;//Increment the object variable to assign a unique CB Index to the next cube.
			}
		}
	}

	// All the render items are opaque.
	for (auto& e : mAllRitems)
		mOpaqueRitems.push_back(e.get());
}

void Rubix::DrawRenderItems(ID3D12GraphicsCommandList* cmdList, const std::vector<RenderItem*>& ritems)
{
	UINT objCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(ObjectConstants));
	UINT matCBByteSize = d3dUtil::CalcConstantBufferByteSize(sizeof(MaterialConstants));

	auto objectCB = mCurrFrameResource->ObjectCB->Resource();
	auto matCB = mCurrFrameResource->MaterialCB->Resource();

	// For each render item...
	for (size_t i = 0; i < ritems.size(); ++i)
	{
		auto ri = ritems[i];

		cmdList->IASetVertexBuffers(0, 1, &ri->Geo->VertexBufferView());
		cmdList->IASetIndexBuffer(&ri->Geo->IndexBufferView());
		cmdList->IASetPrimitiveTopology(ri->PrimitiveType);

		CD3DX12_GPU_DESCRIPTOR_HANDLE tex(mSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
		tex.Offset(ri->Mat->DiffuseSrvHeapIndex, mCbvSrvDescriptorSize);

		D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex*objCBByteSize;
		D3D12_GPU_VIRTUAL_ADDRESS matCBAddress = matCB->GetGPUVirtualAddress() + ri->Mat->MatCBIndex*matCBByteSize;

		cmdList->SetGraphicsRootDescriptorTable(0, tex);
		cmdList->SetGraphicsRootConstantBufferView(1, objCBAddress);
		cmdList->SetGraphicsRootConstantBufferView(3, matCBAddress);

		cmdList->DrawIndexedInstanced(ri->IndexCount, 1, ri->StartIndexLocation, ri->BaseVertexLocation, 0);
	}
}


std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> Rubix::GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	//TODO --- Define the static samplers 
	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1,
		D3D12_FILTER_MIN_MAG_MIP_POINT,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP);

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3,
		D3D12_FILTER_MIN_MAG_MIP_LINEAR,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,
		0.0f,
		8);

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5,
		D3D12_FILTER_ANISOTROPIC,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
		0.0f,
		8);

	return { pointWrap, pointClamp,
	linearWrap, linearClamp,
	anisotropicWrap, anisotropicClamp };
}