#include "Writer.h"

Writer::Writer(
    const std::string& outDir,
    const Potree::AABB& aabb,
    int maxDepth,
    float spacing)
{
    init(outDir, aabb, maxDepth, spacing);
}

void Writer::init(
    const std::string& outDir,
    const Potree::AABB& aabb,
    int maxDepth,
    float spacing)
{
    m_outDir = outDir;
    m_aabb = aabb;
    m_maxDepth = maxDepth;
    m_spacing = spacing;
}

void Writer::add(Potree::Point& point)
{
    // �m�[�h�ւ̒ǉ������͎��Ԃ�������̂ňꎞ�I�ɕۑ�.
    m_temporary.push_back(point);
    m_addedNum++;

    if (m_temporary.size() > 10000) {
        // ��萔�𒴂����̂ŁA�m�[�h�ɒǉ�����.
        storeToNode();
    }
}

void Writer::storeToNode()
{
    // �ꎞ�ۑ��p�̃o�b�t�@����m�[�h�Ɉڂ�.

    auto store = std::move(m_temporary);
    m_temporary.clear();

    // �O��ɍs�����X���b�h�̏������I���̂�҂�.
    waitForStoringToNode();

    m_thStoreToNode = std::thread([&] {
        for (auto p : store) {
            auto isAccepted = m_root.add(p);
            if (isAccepted) {
                // AABB update.
                m_aabb.update(p.position);

                m_acceptedNum++;
            }
        }
    });
}

void Writer::waitForStoringToNode()
{
    if (m_thStoreToNode.joinable()) {
        m_thStoreToNode.join();
    }
}

void Writer::flush()
{
    storeToNode();
    waitForStoringToNode();

    m_root.flush(m_outDir);
}

void Writer::close()
{
    flush();
    m_root.close();
}
