// AABB_OBB_collision.cpp : 응용 프로그램에 대한 진입점을 정의합니다.
//

#include "stdafx.h"
#include "AABB_OBB_collision.h"

#include <d3dx9.h>
#include <vector>

LPDIRECT3D9 g_pD3D = nullptr;
LPDIRECT3DDEVICE9 g_pD3DDevice = nullptr;

BOOL g_CheckFlag = false;
BOOL g_Method = false;

struct BOX_PROPERTY
{
	D3DXVECTOR3 CenterPos;

	D3DXVECTOR3 AxisDir[3];
	float AxisLen[3];

	D3DXVECTOR3 MinPoint, MaxPoint;
	
	float BoxRotateZ;
	float BoxScaling;
};


BOX_PROPERTY g_Box[2];
LPD3DXMESH g_pMesh = nullptr;
D3DXVECTOR3 g_MinPoint = { 0.f, 0.f, 0.f };
D3DXVECTOR3 g_MaxPoint = { 0.f, 0.f, 0.f };


LPD3DXFONT g_pFont = nullptr;
DWORD g_ElapsedTime = 0;


HRESULT InitD3D( HWND hWnd );
HRESULT InitGeometry();
VOID SetupCamera();
VOID SetupLights();
VOID TextSetting();
VOID Render();
VOID Update();
VOID Cleanup();

//void SetBox( BOX_PROPERTY* Box, D3DXVECTOR3 CenterPos = {0.f,0.f,0.f}, float BoxScaling = 1.5, float BoxRotateZ = 0 );
BOOL CheckAABBIntersection( D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1, D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2 );
BOOL CheckOBBIntersection( BOX_PROPERTY* Box1, BOX_PROPERTY* Box2 );


LRESULT WINAPI MsgProc( HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam )
{
	switch ( msg )
	{
		case WM_DESTROY:
			Cleanup( );
			PostQuitMessage( 0 );
			return 0;
	}

	return DefWindowProc( hWnd, msg, wParam, lParam );
}

//main 함수
INT WINAPI wWinMain( HINSTANCE hInst, HINSTANCE, LPWSTR, INT )
{
	hInst;

	WNDCLASSEX wc =
	{
		sizeof( WNDCLASSEX ), CS_CLASSDC, MsgProc, 0L, 0L,
		GetModuleHandle( NULL ), NULL, LoadCursor( NULL, IDC_ARROW ), NULL, NULL,
		L"D3D Tutorial", NULL
	};
	RegisterClassEx( &wc );

	HWND hWnd = CreateWindow( L"D3D Tutorial", L"collision Check", WS_OVERLAPPEDWINDOW, 100, 100, 600, 600, NULL, NULL, wc.hInstance, NULL );

	if ( SUCCEEDED( InitD3D( hWnd ) ) )
	{
		if ( SUCCEEDED( InitGeometry( ) ) )
		{
			ShowWindow( hWnd, SW_SHOWDEFAULT );
			UpdateWindow( hWnd );

			MSG msg;
			ZeroMemory( &msg, sizeof( msg ) );

			while ( msg.message != WM_QUIT )
			{
				if ( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) )
				{
					TranslateMessage( &msg );
					DispatchMessage( &msg );
				}
				else
				{
					Update();
					Render();
				}
			}
		}
	}

	UnregisterClass( L"D3D Tutorial", wc.hInstance );
	return 0;
}

HRESULT InitD3D( HWND hWnd )
{
	if ( nullptr == ( g_pD3D = Direct3DCreate9( D3D_SDK_VERSION ) ) )
	{
		return E_FAIL;
	}

	D3DPRESENT_PARAMETERS d3dpp;
	ZeroMemory( &d3dpp, sizeof( d3dpp ) );
	d3dpp.Windowed = TRUE;
	d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

	if ( FAILED( g_pD3D->CreateDevice( D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &g_pD3DDevice ) ) )
	{
		return E_FAIL;
	}

	D3DXCreateFont( g_pD3DDevice, 15, 0, FW_NORMAL, 1, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"돋음체", &g_pFont );

	g_pD3DDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_CCW );
	g_pD3DDevice->SetRenderState( D3DRS_ZENABLE, TRUE );

	return S_OK;
}

HRESULT InitGeometry( )
{
	if (FAILED(D3DXCreateBox(g_pD3DDevice, 1.f, 1.f, 1.f, &g_pMesh, NULL)))
	{
		return E_FAIL;
	}

	D3DXVECTOR3 *vertices;
	
	
	g_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**) &vertices );
	D3DXComputeBoundingBox( vertices, g_pMesh->GetNumVertices(), g_pMesh->GetNumBytesPerVertex(), &g_MinPoint, &g_MaxPoint );
	g_pMesh->UnlockVertexBuffer();

	
	D3DXMATRIX matScale, matTrans, matRotateZ, matWorld;

	g_Box[0].BoxScaling = 1.5f;
	g_Box[0].CenterPos = D3DXVECTOR3( 0.f, 0.f, 0.f );
	g_Box[0].BoxRotateZ = 0.f;
	g_Box[0].AxisDir[0] = D3DXVECTOR3( 1, 0, 0 );
	g_Box[0].AxisDir[1] = D3DXVECTOR3( 0, 1, 0 );
	g_Box[0].AxisDir[2] = D3DXVECTOR3( 0, 0, 1 );

	for ( int i = 0; i < 3; ++i )
	{
		g_Box[0].AxisLen[i] = 0.5f;

		D3DXVec3TransformNormal( &(g_Box[0].AxisDir[i]), &(g_Box[0].AxisDir[i]), &matRotateZ );
		D3DXVec3Normalize( &( g_Box[0].AxisDir[i] ), &( g_Box[0].AxisDir[i] ) );
		g_Box[0].AxisLen[i] = g_Box[0].AxisLen[i] * g_Box[0].BoxScaling;
	}
	D3DXMatrixTranslation( &matTrans, g_Box[0].CenterPos.x, g_Box[0].CenterPos.y, g_Box[0].CenterPos.z );
	D3DXMatrixScaling( &matScale, g_Box[0].BoxScaling, g_Box[0].BoxScaling, g_Box[0].BoxScaling );
	D3DXMatrixRotationZ( &matRotateZ, g_Box[0].BoxRotateZ );

	matWorld = matRotateZ* matScale * matTrans;
	D3DXVec3TransformCoord( &g_Box[0].MinPoint, &g_MinPoint, &matWorld );
	D3DXVec3TransformCoord( &g_Box[0].MaxPoint, &g_MaxPoint, &matWorld );

	
	g_Box[1].BoxScaling = 2.f;
	g_Box[1].CenterPos = D3DXVECTOR3( 3.f, 3.f, 0.f );
	g_Box[1].BoxRotateZ = 0.f;
	g_Box[1].AxisDir[0] = D3DXVECTOR3( 1, 0, 0 );
	g_Box[1].AxisDir[1] = D3DXVECTOR3( 0, 1, 0 );
	g_Box[1].AxisDir[2] = D3DXVECTOR3( 0, 0, 1 );

	for ( int i = 0; i < 3; ++i )
	{
		g_Box[1].AxisLen[i] = 0.5f;

		D3DXVec3TransformNormal( &( g_Box[0].AxisDir[i] ), &( g_Box[1].AxisDir[i] ), &matRotateZ );
		D3DXVec3Normalize( &( g_Box[1].AxisDir[i] ), &( g_Box[1].AxisDir[i] ) );
		g_Box[1].AxisLen[i] = g_Box[1].AxisLen[i] * g_Box[1].BoxScaling;
	}

	D3DXMatrixTranslation( &matTrans, g_Box[1].CenterPos.x, g_Box[1].CenterPos.y, g_Box[1].CenterPos.z );
	D3DXMatrixScaling( &matScale, g_Box[1].BoxScaling, g_Box[1].BoxScaling, g_Box[1].BoxScaling );
	D3DXMatrixRotationZ( &matRotateZ, g_Box[1].BoxRotateZ );

	matWorld = matRotateZ* matScale * matTrans;
	D3DXVec3TransformCoord( &g_Box[1].MinPoint, &g_MinPoint, &matWorld );
	D3DXVec3TransformCoord( &g_Box[1].MaxPoint, &g_MaxPoint, &matWorld );


	return S_OK;
}


BOOL CheckAABBIntersection( D3DXVECTOR3* vMin1, D3DXVECTOR3* vMax1, D3DXVECTOR3* vMin2, D3DXVECTOR3* vMax2 )
{
	if ( vMin1->x <= vMax2->x && vMax1->x >= vMin2->x &&
		 vMin1->y <= vMax2->y && vMax1->y >= vMin2->y &&
		 vMin1->z <= vMax2->z && vMax1->z >= vMin2->z )
		 return TRUE;
	return FALSE;
}

BOOL CheckOBBIntersection( BOX_PROPERTY* Box1, BOX_PROPERTY* Box2 )
{
	double c[3][3];
	double absC[3][3];
	double d[3];
	double r0, r1, r;
	int i;
	const double cutoff = 0.999999;
	bool existsParallelPair = false;

	D3DXVECTOR3 diff = Box1->CenterPos - Box2->CenterPos;

	for ( i = 0; i < 3; ++i )
	{
		c[0][i] = D3DXVec3Dot( &Box1->AxisDir[0], &Box2->AxisDir[i] );
		absC[0][i] = abs( c[0][i] );
		if ( absC[0][i] > cutoff )
			existsParallelPair = true;
	}

	d[0] = D3DXVec3Dot( &diff, &Box1->AxisDir[0] );
	r = abs( d[0] );
	r0 = Box1->AxisLen[0];
	r1 = Box2->AxisLen[0] * absC[0][0] + Box2->AxisLen[1] * absC[0][1] + Box2->AxisLen[2] * absC[0][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( i = 0; i < 3; ++i )
	{
		c[1][i] = D3DXVec3Dot( &Box1->AxisDir[1], &Box2->AxisDir[i] );
		absC[1][i] = abs( c[1][i] );
		if ( absC[1][i] > cutoff )
			existsParallelPair = true;
	}
	d[1] = D3DXVec3Dot( &diff, &Box1->AxisDir[1] );
	r = abs( d[1] );
	r0 = Box1->AxisLen[1];
	r1 = Box2->AxisLen[0] * absC[1][0] + Box2->AxisLen[1] * absC[1][1] + Box2->AxisLen[2] * absC[1][2];
	if ( r > r0 + r1 )
		return FALSE;

	for ( i = 0; i < 3; ++i )
	{
		c[2][i] = D3DXVec3Dot( &Box1->AxisDir[2], &Box2->AxisDir[i] );
		absC[2][i] = abs( c[2][i] );
		if ( absC[2][i] > cutoff )
			existsParallelPair = true;
	}
	d[2] = D3DXVec3Dot( &diff, &Box1->AxisDir[2] );
	r = abs( d[2] );
	r0 = Box1->AxisLen[2];
	r1 = Box2->AxisLen[0] * absC[2][0] + Box2->AxisLen[1] * absC[2][1] + Box2->AxisLen[2] * absC[2][2];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &Box2->AxisDir[0] ) );
	r0 = Box1->AxisLen[0] * absC[0][0] + Box1->AxisLen[1] * absC[1][0] + Box1->AxisLen[2] * absC[2][0];
	r1 = Box2->AxisLen[0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &Box2->AxisDir[1] ) );
	r0 = Box1->AxisLen[0] * absC[0][1] + Box1->AxisLen[1] * absC[1][1] + Box1->AxisLen[2] * absC[2][1];
	r1 = Box2->AxisLen[1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( D3DXVec3Dot( &diff, &Box2->AxisDir[2] ) );
	r0 = Box1->AxisLen[0] * absC[0][2] + Box1->AxisLen[1] * absC[1][2] + Box1->AxisLen[2] * absC[2][2];
	r1 = Box2->AxisLen[2];
	if ( r > r0 + r1 )
		return FALSE;

	if ( existsParallelPair == true )
		return TRUE;

	r = abs( d[2] * c[1][0] - d[1] * c[2][0] );
	r0 = Box1->AxisLen[1] * absC[2][0] + Box1->AxisLen[2] * absC[1][0];
	r1 = Box2->AxisLen[1] * absC[0][2] + Box2->AxisLen[2] * absC[0][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][1] - d[1] * c[2][1] );
	r0 = Box1->AxisLen[1] * absC[2][1] + Box1->AxisLen[2] * absC[1][1];
	r1 = Box2->AxisLen[0] * absC[0][2] + Box2->AxisLen[2] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[2] * c[1][2] - d[1] * c[2][2] );
	r0 = Box1->AxisLen[1] * absC[2][2] + Box1->AxisLen[2] * absC[1][2];
	r1 = Box2->AxisLen[0] * absC[0][1] + Box2->AxisLen[1] * absC[0][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][0] - d[2] * c[0][0] );
	r0 = Box1->AxisLen[0] * absC[2][0] + Box1->AxisLen[2] * absC[0][0];
	r1 = Box2->AxisLen[1] * absC[1][2] + Box2->AxisLen[2] * absC[1][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][1] - d[2] * c[0][1] );
	r0 = Box1->AxisLen[0] * absC[2][1] + Box1->AxisLen[2] * absC[0][1];
	r1 = Box2->AxisLen[0] * absC[1][2] + Box2->AxisLen[2] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[0] * c[2][2] - d[2] * c[0][2] );
	r0 = Box1->AxisLen[0] * absC[2][2] + Box1->AxisLen[2] * absC[0][2];
	r1 = Box2->AxisLen[0] * absC[1][1] + Box2->AxisLen[1] * absC[1][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][0] - d[0] * c[1][0] );
	r0 = Box1->AxisLen[0] * absC[1][0] + Box1->AxisLen[1] * absC[0][0];
	r1 = Box2->AxisLen[1] * absC[2][2] + Box2->AxisLen[2] * absC[2][1];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][1] - d[0] * c[1][1] );
	r0 = Box1->AxisLen[0] * absC[1][1] + Box1->AxisLen[1] * absC[0][1];
	r1 = Box2->AxisLen[0] * absC[2][2] + Box2->AxisLen[2] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	r = abs( d[1] * c[0][2] - d[0] * c[1][2] );
	r0 = Box1->AxisLen[0] * absC[1][2] + Box1->AxisLen[1] * absC[0][2];
	r1 = Box2->AxisLen[0] * absC[2][1] + Box2->AxisLen[1] * absC[2][0];
	if ( r > r0 + r1 )
		return FALSE;

	return TRUE;
}

VOID SetupMatrices( )
{
	D3DXMATRIXA16 matWorld;
	D3DXMatrixIdentity( &matWorld );
	g_pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );


	D3DXVECTOR3 vEyePt( 0.f, 5.f, -15.f );
	D3DXVECTOR3 vLookatPt( 0.f, 0.f, 0.f );
	D3DXVECTOR3 vUpVec( 0.f, 1.f, 0.f );
	D3DXMATRIXA16 matView;
	D3DXMatrixLookAtLH( &matView, &vEyePt, &vLookatPt, &vUpVec );
	g_pD3DDevice->SetTransform( D3DTS_VIEW, &matView );

	D3DXMATRIXA16 matProj;
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI / 4, 1.0f, 1.0f, 100.0f );
	g_pD3DDevice->SetTransform( D3DTS_PROJECTION, &matProj );

}



VOID Render( )
{
	RECT rt1, rt2;
	D3DXMATRIX matWorld, matScale, matRotateZ, matTrans;

	g_pD3DDevice->Clear( 0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB( 0, 0, 255 ), 1.0f, 0 );

	if ( SUCCEEDED( g_pD3DDevice->BeginScene( ) ) )
	{

		SetupMatrices( );
		SetRect( &rt1, 10, 10, 0, 0 );
		if (!g_Method)
		{
			g_pFont->DrawTextW( NULL, L"AABB 충돌(left click으로 방법 변경)", -1, &rt1, DT_NOCLIP, D3DXCOLOR( 1.f, 1.f, 0.f, 1.f ) );
		}
		else
		{
			g_pFont->DrawTextW( NULL, L"OBB 충돌(left click으로 방법 변경)", -1, &rt1, DT_NOCLIP, D3DXCOLOR( 1.f, 1.f, 0.f, 1.f ) );
		}
		


		g_pD3DDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );


		for ( DWORD i = 0; i < sizeof( g_Box ) / sizeof( g_Box[1] ); ++i )
		{
			D3DXMatrixTranslation( &matTrans, g_Box[i].CenterPos.x, g_Box[i].CenterPos.y, g_Box[i].CenterPos.z );
			D3DXMatrixScaling( &matScale, g_Box[i].BoxScaling, g_Box[i].BoxScaling, g_Box[i].BoxScaling );
			D3DXMatrixRotationZ( &matRotateZ, g_Box[i].BoxRotateZ );
			matWorld = matRotateZ * matScale * matTrans;

			g_pD3DDevice->SetTransform( D3DTS_WORLD, &matWorld );

			g_pMesh->DrawSubset( 0 );
		}
		SetRect( &rt2, 10, 30, 0, 0 );

		if ( g_CheckFlag )
		{
			g_pFont->DrawTextW( NULL, L"박았음!!", -1, &rt2, DT_NOCLIP, D3DXCOLOR( 1.f, 1.f, 0.f, 1.f ) );
		}
		else
		{
			g_pFont->DrawTextW( NULL, L"아직 멀었음!!", -1, &rt2, DT_NOCLIP, D3DXCOLOR( 1.f, 1.f, 0.f, 1.f ) );
		}

		g_pD3DDevice->EndScene( );
	}

	g_pD3DDevice->Present( NULL, NULL, NULL, NULL );
}

VOID Update()
{
	DWORD currentTime = GetTickCount();
	static DWORD preTime;
	g_ElapsedTime = currentTime - preTime;
	preTime = currentTime;

	if ( GetAsyncKeyState( VK_LEFT ) )
	{
		g_Box[0].CenterPos.x -= g_ElapsedTime*0.002f;
	}
	if ( GetAsyncKeyState( VK_RIGHT ) )
	{
		g_Box[0].CenterPos.x += g_ElapsedTime*0.002f;
	}
	if ( GetAsyncKeyState( VK_UP ) )
	{
		g_Box[0].CenterPos.y += g_ElapsedTime*0.002f;
	}
	if ( GetAsyncKeyState( VK_DOWN ) )
	{
		g_Box[0].CenterPos.y -= g_ElapsedTime*0.002f;
	}
	if ( GetAsyncKeyState( VK_LBUTTON ) )
	{
		if (g_ElapsedTime > 20)
		{
			g_Method = !g_Method;
		}
		
	}
	if ( GetAsyncKeyState( VK_HOME ) )
	{
		g_Box[1].BoxRotateZ -= 0.2f;
	}
	if ( GetAsyncKeyState( VK_END ) )
	{
		g_Box[1].BoxRotateZ += 0.2f;
	}

	D3DXVECTOR3 *vertices;

	g_pMesh->LockVertexBuffer( D3DLOCK_READONLY, (void**) &vertices );
	D3DXComputeBoundingBox( vertices, g_pMesh->GetNumVertices(), g_pMesh->GetNumBytesPerVertex(), &g_MinPoint, &g_MaxPoint );
	g_pMesh->UnlockVertexBuffer();

	D3DXMATRIX matScale, matTrans, matRotateZ, matWorld;
	D3DXMatrixTranslation( &matTrans, g_Box[0].CenterPos.x, g_Box[0].CenterPos.y, g_Box[0].CenterPos.z );
	D3DXMatrixScaling( &matScale, g_Box[0].BoxScaling, g_Box[0].BoxScaling, g_Box[0].BoxScaling );
	D3DXMatrixRotationZ( &matRotateZ, g_Box[0].BoxRotateZ );

	matWorld = matRotateZ* matScale * matTrans;
	D3DXVec3TransformCoord( &g_Box[0].MinPoint, &g_MinPoint, &matWorld );
	D3DXVec3TransformCoord( &g_Box[0].MaxPoint, &g_MaxPoint, &matWorld );
	
	if (!g_Method)
	{
		g_CheckFlag = CheckAABBIntersection( &g_Box[0].MinPoint, &g_Box[0].MaxPoint, &g_Box[1].MinPoint, &g_Box[1].MaxPoint );
	}
	else
	{
		g_CheckFlag = CheckOBBIntersection( &g_Box[0], &g_Box[1] );
	}
	
}

VOID Cleanup( )
{
	if (NULL != g_pFont)
	{
		g_pFont->Release();
	}

	if (NULL != g_pMesh)
	{
		g_pMesh->Release();
	}

	if ( NULL != g_pD3DDevice )
	{
		g_pD3DDevice->Release();
	}

	if ( NULL != g_pD3D )
	{
		g_pD3D->Release();
	}
}

/*
void SetBox( BOX_PROPERTY* Box, D3DXVECTOR3 CenterPos, float BoxScaling, float BoxRotateZ )
{
D3DXMATRIX matScale, matTrans, matRotateZ, matWorld;
D3DXMatrixIdentity( &matScale );
D3DXMatrixIdentity( &matTrans );
D3DXMatrixIdentity( &matRotateZ );
D3DXMatrixIdentity( &matWorld );

Box->CenterPos = CenterPos;
Box->BoxRotateZ = BoxRotateZ;

Box->AxisDir[0] = D3DXVECTOR3( 1, 0, 0 );
Box->AxisDir[1] = D3DXVECTOR3( 0, 1, 0 );
Box->AxisDir[2] = D3DXVECTOR3( 0, 0, 1 );

Box->AxisLen[0] = 0.5f;
Box->AxisLen[1] = 0.5f;
Box->AxisLen[2] = 0.5f;

D3DXMatrixTranslation( &matTrans, Box->CenterPos.x, Box->CenterPos.y, Box->CenterPos.z );
D3DXMatrixScaling( &matScale, Box->BoxScaling, Box->BoxScaling, Box->BoxScaling );
D3DXMatrixRotationZ( &matRotateZ, Box->BoxRotateZ );
matWorld = matRotateZ * matScale * matTrans;

D3DXVec3TransformCoord( &Box->MinPoint, &g_MinPoint, &matWorld );
D3DXVec3TransformCoord( &Box->MaxPoint, &g_MaxPoint, &matWorld );

for ( int i = 0; i < 3; ++i )
{
D3DXVec3TransformNormal( &Box->AxisDir[i], &Box->AxisDir[i], &matRotateZ );
D3DXVec3Normalize( &Box->AxisDir[i], &Box->AxisDir[i] );

Box->AxisLen[i] = Box->AxisLen[i] * BoxScaling;
}
}
*/