#include "Node.h"
#include "Writer.h"

Node::Node(
    Writer* writer,
    const Potree::AABB& aabb, int index, int level, float spacing)
{
    init(writer, aabb, index, level, spacing);
}

void Node::init(
    Writer* writer,
    const Potree::AABB& aabb, int index, int level, float spacing)
{
    m_writer = writer;

    m_aabb = aabb;
    m_index = index;
    m_level = level;

    m_spacing = spacing / powf(2.0f, (float)level);
}

bool Node::add(Potree::Point& point)
{
    // �o�^�\���ǂ����`�F�b�N.
    // �_�̊Ԋu���󂢂Ă��邩�ǂ���.
    auto isAccepted = m_grid.add(point.position);

    auto depth = m_writer->getMaxDepth();

    // TODO
    // ��ԍ������x���̎��͋����I�ɒǉ�.
    if (m_level == depth - 1) {
        isAccepted = true;
    }

    if (isAccepted) {
        m_accepted.push_back(point);
        m_acceptedAABB.update(point.position);
    }
    
    if (m_level >= depth) {
        return isAccepted;
    }

    auto childIndex = Potree::nodeIndex(m_aabb, point);

    if (childIndex >= 0) {
        if (m_children.size() == 0) {
            m_children.resize(8, nullptr);
        }

        auto child = m_children[childIndex];

        if (!child) {
            // create child if not exist.
            child = createChild(childIndex);
        }

        return child->add(point);
    }

    return isAccepted;
}

Node* Node::createChild(int index)
{
    float spacing = m_writer->getSpacing();

    // TODO
    // �q�m�[�h��AABB���v�Z.
    Potree::AABB aabb;

    Node* child = new Node(
        m_writer,
        aabb,
        index,
        m_level + 1,
        spacing);

    return child;
}

bool Node::flush(const std::string& base)
{
    // �O�̂���.
    waitForFlush();

    std::string basepath(base);

    if (!m_exporter.isOpen()) {
        // �o�̓t�@�C���p�X���쐬.
        if (m_level == 0) {
            basepath += "r";
        }
        else {
            char tmp[4] = { 0 };
            sprintf(tmp, "%d\0", m_index);
            basepath += tmp;
        }
        std::string filepath = basepath + ".pcd";  // TODO

        m_exporter.open(filepath);
    }

    JobQueue::instance().enqueue([&] {
        for (auto point : m_accepted) {
            m_exporter.write(point);
        }
        m_evExport.set();
    });

    for (auto child : m_children) {
        if (child) {
            child->flush(basepath);
        }
    }
}

void Node::waitForFlush()
{
    for (auto child : m_children) {
        if (child) {
            child->waitForFlush();
        }
    }

    m_evExport.wait();
    m_evExport.reset();
}

void Node::close()
{
    waitForFlush();

    m_exporter.close();
}
