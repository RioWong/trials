#include "Node.h"

std::string Node::BasePath("./");

void Node::flush()
{
    // TODO
    // ���̂��Ƃ�merge�����ƁAdepth���ς���Ă��܂�.
    // flush ���P�x�ł��Ă΂ꂽ��merge����Ȃ��悤�ɂ���H.

    if (!m_fp) {
        std::string path(BasePath);

        auto depth = getDepth();
        auto mortonNumber = getMortonNumber();

        path += "r";
        uint32_t mask = 0x07 << (3 * (depth - 1));

        for (uint32_t i = 1; i < depth; i++) {
            auto n = mortonNumber & mask;

            path += n;

            mask >>= 3;
        }

        fopen_s(&m_fp, path.c_str(), "wb");
        IZ_ASSERT(m_fp);

        // �w�b�_�[���󂯂�.
        fseek(m_fp, sizeof(m_header), SEEK_SET);

        m_header.magic_number = FOUR_CC('S', 'P', 'C', 'D');
        m_header.version = 0;

        m_header.depth = depth;
        m_header.mortonNumber = mortonNumber;
    }

    DynamicOctreeNode<Point>::flush(
        [&](Point* src, uint32_t size) {
        fwrite(src, size, 1, m_fp);
    });
}

void Node::close()
{
    IZ_ASSERT(m_fp);

    m_header.vtxNum = getTotalObjNum();

    // TODO
    m_header.vtxFormat = VtxFormat::Position
        | VtxFormat::Color;

    // �擪�ɖ߂��āA�w�b�_�[��������.
    fseek(m_fp, 0, SEEK_SET);
    fwrite(&m_header, sizeof(m_header), 1, m_fp);

    fclose(m_fp);
    m_fp = nullptr;
}
