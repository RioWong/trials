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

        for (uint32_t i = 1; i < depth; i++) {
           // TODO 
        }

        fopen_s(&m_fp, path.c_str(), "wb");
        IZ_ASSERT(m_fp);
    }

    // TODO
}

void Node::close()
{
    IZ_ASSERT(m_fp);

    // TODO

    fclose(m_fp);
}
