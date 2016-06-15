#include "Node.h"
#include "izSystem.h"

/////////////////////////////////////////////////////

uint16_t HalfFloat::basetable[512] = { 0 };
uint8_t HalfFloat::shifttable[512] = { 0 };

void HalfFloat::genTable()
{
    // NOTE
    // ftp://www.fox-toolkit.org/pub/fasthalffloatconversion.pdf

    int32_t e;

    for (uint32_t i = 0; i < 256; ++i){
        e = i - 127;

        if (e < -24) { // Very small numbers map to zero
            basetable[i | 0x000] = 0x0000;
            basetable[i | 0x100] = 0x8000;
            shifttable[i | 0x000] = 24;
            shifttable[i | 0x100] = 24;
        }
        else if (e < -14) { // Small numbers map to denorms
            basetable[i | 0x000] = (0x0400 >> (-e - 14));
            basetable[i | 0x100] = (0x0400 >> (-e - 14)) | 0x8000;
            shifttable[i | 0x000] = -e - 1;
            shifttable[i | 0x100] = -e - 1;
        }
        else if (e <= 15) { // Normal numbers just lose precision
            basetable[i | 0x000] = ((e + 15) << 10);
            basetable[i | 0x100] = ((e + 15) << 10) | 0x8000;
            shifttable[i | 0x000] = 13;
            shifttable[i | 0x100] = 13;
        }
        else if (e < 128) { // Large numbers map to Infinity
            basetable[i | 0x000] = 0x7C00;
            basetable[i | 0x100] = 0xFC00;
            shifttable[i | 0x000] = 24;
            shifttable[i | 0x100] = 24;
        }
        else { // Infinity and NaN's stay Infinity and NaN's
            basetable[i | 0x000] = 0x7C00;
            basetable[i | 0x100] = 0xFC00;
            shifttable[i | 0x000] = 13;
            shifttable[i | 0x100] = 13;
        }
    }
}

/////////////////////////////////////////////////////

std::string Node::BasePath("./");

std::atomic<uint32_t> Node::FlushedNum = 0;

izanagi::sys::CSpinLock g_locker;
std::atomic<uint32_t> Node::s_ID = 0;

float Node::Scale = 1.0f;

izanagi::sys::CSpinLock Locker;
float Node::AddTime = 0.0f;

std::atomic<uint32_t> Node::CurIdx = 0;

std::vector<bool> IsOpened(1000);

Node::Node()
{
    //izanagi::sys::Lock lock(g_locker);
    m_id = s_ID;
    s_ID++;

#ifndef USE_STL_VECTOR
    m_pos[0] = m_pos[1] = 0;
#endif
}

bool Node::add(const Point& vtx)
{
#ifdef USE_STL_VECTOR
    m_vtx[m_curIdx].push_back(vtx);
#else
    IZ_ASSERT(m_pos[Node::CurIdx] < FLUSH_LIMIT);
    
    auto pos = m_pos[Node::CurIdx];

#ifdef ENABLE_HALF_FLOAT
    auto& pt = m_vtx[Node::CurIdx][pos];

    // NOTE
    // float -> half
    // ftp://www.fox-toolkit.org/pub/fasthalffloatconversion.pdf

#if 0
    uint32_t* pf = (uint32_t*)vtx.pos;

    auto i = _mm_setr_epi32(pf[0], pf[1], pf[2], 0);
    auto f0 = _mm_srli_epi32(i, 16);
    auto f2 = _mm_srli_epi32(i, 13);

    i = _mm_setr_epi32(
        (pf[0] & 0x7f800000) - 0x38000000,
        (pf[1] & 0x7f800000) - 0x38000000,
        (pf[2] & 0x7f800000) - 0x38000000,
        0);
    auto f1 = _mm_srli_epi32(i, 13);

    pt.pos[0] = (f0.m128i_u32[0] & 0x8000) | (f1.m128i_u32[0] & 0x7c00) | (f2.m128i_u32[0] & 0x03ff);
    pt.pos[1] = (f0.m128i_u32[1] & 0x8000) | (f1.m128i_u32[1] & 0x7c00) | (f2.m128i_u32[1] & 0x03ff);
    pt.pos[2] = (f0.m128i_u32[2] & 0x8000) | (f1.m128i_u32[2] & 0x7c00) | (f2.m128i_u32[2] & 0x03ff);
#elif 0
    //uint32_t* pf = (uint32_t*)vtx.pos;

    //uint32_t f = pf[0];
    uint32_t f = vtx.posi[0];
    pt.pos[0] = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);

    //f = pf[1];
    f = vtx.posi[1];
    pt.pos[1] = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);

    //f = pf[2];
    f = vtx.posi[2];
    pt.pos[2] = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);
#else
    // TODO
    // �e�[�u���̌v�Z���ʂƒʏ�v�Z�̌��ʂ���v���Ȃ�...

    uint32_t f = vtx.posi[0];
    pt.pos[0] = HalfFloat::basetable[(f >> 23) & 0x1ff] + ((f & 0x007fffff) >> HalfFloat::shifttable[(f >> 23) & 0x1ff]);

    //uint16_t h = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);
    //IZ_ASSERT(pt.pos[0] == h);
    
    f = vtx.posi[1];
    pt.pos[1] = HalfFloat::basetable[(f >> 23) & 0x1ff] + ((f & 0x007fffff) >> HalfFloat::shifttable[(f >> 23) & 0x1ff]);

    //h = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);
    //IZ_ASSERT(pt.pos[1] == h);

    f = vtx.posi[2];
    pt.pos[2] = HalfFloat::basetable[(f >> 23) & 0x1ff] + ((f & 0x007fffff) >> HalfFloat::shifttable[(f >> 23) & 0x1ff]);

    //h = ((f >> 16) & 0x8000) | ((((f & 0x7f800000) - 0x38000000) >> 13) & 0x7c00) | ((f >> 13) & 0x03ff);
    //IZ_ASSERT(pt.pos[2] == h);
#endif

    // NOTE
    // Set W value (1.0f) in the vertex shader.
#if 0
    static const uint16_t HalfFloatOne = 15360;
    pt.pos[3] = HalfFloatOne;
#endif

    pt.color = vtx.color;
#else
#if 0
    float* dst = m_vtx[Node::CurIdx][pos].pos;

    __m128 v = _mm_load_ps(vtx.pos);

    izanagi::sys::CTimer timer;
    timer.Begin();

    _mm_store_ps(dst, v);

    auto t = timer.End();

    {
        izanagi::sys::Lock lock(Locker);
        Node::AddTime += t;
    }
#else
    //izanagi::sys::CTimer timer;
    //timer.Begin();

    m_vtx[Node::CurIdx][pos] = vtx;
    //memcpy(&m_vtx[Node::CurIdx][pos], &vtx, sizeof(vtx));
    //m_vtx[Node::CurIdx][pos].m = vtx.m;

    //auto t = timer.End();

    {   
        //izanagi::sys::Lock lock(Locker);
        //Node::AddTime += t;
    }
#endif
#endif

    ++m_pos[Node::CurIdx];
#endif
    return true;
}

void Node::flush()
{
#ifdef MAIN_THREAD_WRITER
    uint32_t idx = 1 - Node::CurIdx;
#else
    uint32_t idx = 0;
#endif

#ifndef USE_STL_VECTOR
    uint32_t num = m_pos[idx];
    m_pos[idx] = 0;
#endif

#ifdef USE_STL_VECTOR
    if (m_vtx[idx].size() == 0) {
#else
    if (num == 0) {
#endif
        return;
    }

    if (!m_fp) {
        std::string path(BasePath);

        auto id = m_id;

        char tmp[10];
        sprintf(tmp, "%d\0", id);

        path += tmp;

        path += ".spcd";

        auto err = fopen_s(&m_fp, path.c_str(), "wb");

        if (!m_fp) {
            IZ_PRINTF("err[%d](%s)(%s)\n", err, path.c_str(), IsOpened[id] ? "open" : "not open");
        }

        IZ_ASSERT(m_fp);
        IsOpened[id] = true;

        // NOTE
        // http://www.cc.u-tokyo.ac.jp/support/press/news/VOL8/No5/data_no1_0609.pdf
        // �T�C�Y���傫���ƁAfclose���Ƀt���b�V���Ɏ��Ԃ�������A�t�ɒx���Ȃ�.
        // ���̂��߁A���ʂ͂قƂ�ǂȂ�����...
        //setvbuf(m_fp, NULL, _IOFBF, 512 * 1024);

        // �w�b�_�[���󂯂�.
        fseek(m_fp, sizeof(SPCDHeader), SEEK_SET);
    }

    auto& vtx = m_vtx[idx];

    auto src = &vtx[0];
#ifdef USE_STL_VECTOR
    auto num = vtx.size();
#endif
    auto size = num * sizeof(ExportPoint);

    m_totalNum += num;

    FlushedNum += num;

    izanagi::sys::CTimer timer;

    fwrite(src, size, 1, m_fp);

#ifdef USE_STL_VECTOR
    vtx.clear();
#endif
}

void Node::close()
{
    if (m_fp) {
#ifdef USE_STL_VECTOR
        IZ_ASSERT(m_vtx[0].size() == 0);
        IZ_ASSERT(m_vtx[1].size() == 0);
#else
        IZ_ASSERT(m_pos[0] == 0);
        IZ_ASSERT(m_pos[1] == 0);
#endif

        SPCDHeader header;

        header.magic_number = FOUR_CC('S', 'P', 'C', 'D');
        header.version = 0;

        // TODO
        header.vtxFormat = VtxFormat::Position
            | VtxFormat::Color;

        auto depth = m_level;
        auto mortonNumber = m_mortonNumber;
        auto objNum = m_totalNum;

        //FlushedNum += objNum;

        header.vtxNum = objNum;

        header.depth = depth;
        header.mortonNumber = mortonNumber;

        auto fileSize = ftell(m_fp);

        header.fileSize = fileSize;
        header.fileSizeWithoutHeader = fileSize - sizeof(header);

        auto& aabbMin = m_aabb.getMin();
        auto& aabbMax = m_aabb.getMax();

        header.aabbMin[0] = aabbMin.x * Node::Scale;
        header.aabbMin[1] = aabbMin.y * Node::Scale;
        header.aabbMin[2] = aabbMin.z * Node::Scale;

        header.aabbMax[0] = aabbMax.x * Node::Scale;
        header.aabbMax[1] = aabbMax.y * Node::Scale;
        header.aabbMax[2] = aabbMax.z * Node::Scale;

        // TODO
        header.spacing = izanagi::math::SVector4::Length2(aabbMin, aabbMax) / 250.0f;

        header.scale = Node::Scale;

        // �擪�ɖ߂��āA�w�b�_�[��������.
        fseek(m_fp, 0, SEEK_SET);
        fwrite(&header, sizeof(header), 1, m_fp);

        fclose(m_fp);
        m_fp = nullptr;
    }
}
