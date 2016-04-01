#if !defined(__WRITER_H__)
#define __WRITER_H__

#include <stdint.h>
#include <thread>
#include "proxy.h"
#include "Node.h"

class Writer {
public:
    Writer() {}

    Writer(
        const std::string& outDir,
        const Potree::AABB& aabb,
        int maxDepth,
        float spacing);

    ~Writer() {}

public:
    void init(
        const std::string& outDir,
        const Potree::AABB& aabb,
        int maxDepth,
        float spacing);

    void add(Potree::Point& point);

    void storeToNode();

    void waitForStoringToNode();

    void flush();

    void close();

public:
    void setMaxDepth(int d)
    {
        m_maxDepth = d;
    }

    int getMaxDepth() const
    {
        return m_maxDepth;
    }

    float setSpacing(float s)
    {
        m_spacing = s;
    }

    float getSpacing() const
    {
        return m_spacing;
    }

private:
    // ���[�g�m�[�h.
    Node m_root;

    // �ꎞ�ۑ��p�̃o�b�t�@.
    std::vector<Potree::Point> m_temporary;

    // �ǉ����ꂽ�_�̐�.
    uint64_t m_addedNum{ 0 };

    std::thread m_thStoreToNode;

    // �m�[�h�ɒǉ����ꂽ�_�̐�.
    uint64_t m_acceptedNum{ 0 };

    // �m�[�h�ɒǉ����ꂽ�_������AABB
    Potree::AABB m_aabb;

    int m_maxDepth{ 0 };
    float m_spacing{ 0.0f };

    // �o�͗p�f�B���N�g��.
    std::string m_outDir;
};

#endif
