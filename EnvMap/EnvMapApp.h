#if !defined(__ENV_MAP_APP_H__)
#define __ENV_MAP_APP_H__

#include "izSampleKit.h"

static const IZ_UINT SCREEN_WIDTH = 1280;
static const IZ_UINT SCREEN_HEIGHT = 800;

class CEnvMapApp: public izanagi::sample::CSampleApp {
public:
    CEnvMapApp();
    virtual ~CEnvMapApp();

protected:
    // 初期化.
    virtual IZ_BOOL InitInternal(
        izanagi::IMemoryAllocator* allocator,
        izanagi::graph::CGraphicsDevice* device,
        izanagi::sample::CSampleCamera& camera);

    // 解放.
    virtual void ReleaseInternal();

    // 更新.
    virtual void UpdateInternal(izanagi::graph::CGraphicsDevice* device);

    // 描画.
    virtual void RenderInternal(izanagi::graph::CGraphicsDevice* device);

    virtual IZ_BOOL OnKeyDown(izanagi::sys::E_KEYBOARD_BUTTON key);

private:
    izanagi::sample::CSampleEnvBox* m_Cube;

    izanagi::CImage* m_Img;
    
    izanagi::graph::CVertexShader* m_vs{ nullptr };
    izanagi::graph::CPixelShader* m_fs{ nullptr };
    izanagi::graph::CShaderProgram* m_shd{ nullptr };

    izanagi::math::SMatrix44 m_L2W;

    static const IZ_INT m_Idx = 1;
};

#endif    // #if !defined(__ENV_MAP_APP_H__)