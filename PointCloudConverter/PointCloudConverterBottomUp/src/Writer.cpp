#include "Writer.h"

Writer::Writer()
{
    // TODO
    m_octree.init(
        10.0f,
        //izanagi::math::CVector4(5.0f, 5.0f, 5.0f),
        izanagi::math::CVector4(0.0f, 0.0f, 0.0f),
        1.0f,
        4);

    m_runThread = true;

    m_objects.reserve(10000);

    m_waitMain.Set();

    m_thStore = std::thread([&] {
        while (m_runThread) {
            m_waitWorker.Wait();
            m_waitWorker.Reset();

            procStore();

            m_waitMain.Set();
        }
    });
}

Writer::~Writer()
{
    terminate();
}

uint32_t Writer::add(const Point& obj)
{
    m_objects.push_back(obj);
    return m_objects.size();
}

void Writer::store()
{
    waitForFlushing();
    
    m_waitMain.Wait();
    m_waitMain.Reset();

    m_temporary.resize(m_objects.size());

    auto num = m_objects.size();

    memcpy(
        &m_temporary[0],
        &m_objects[0],
        sizeof(Point) * num);

    m_objects.clear();

    m_waitWorker.Set();
}

void Writer::terminate()
{
    waitForFlushing();
    
    m_waitMain.Wait();

    m_runThread = false;

    m_waitWorker.Set();

    if (m_thStore.joinable()) {
        m_thStore.join();
    }
}

void Writer::procStore()
{
    for (auto obj : m_temporary) {
        m_octree.add(obj);

        m_acceptedNum++;
    }

    m_temporary.clear();
}

void Writer::flush(izanagi::threadmodel::CThreadPool& theadPool)
{
    waitForFlushing();
    
    m_waitMain.Wait();

    // 削除リストにあるノードを処理する.
    m_octree.cleanDeleteList();

    auto& list = m_octree.getNodeList();

    for (auto it = list.begin(); it != list.end(); it++) {
        Node* node = (Node*)*it;
        node->enableProcessing();
    }

    auto num = list.size();

    izanagi::threadmodel::CParallel::For(
        theadPool,
        0, num,
        [&](IZ_INT idx) {
        auto it = list.begin();
        std::advance(it, idx);

        Node* node = (Node*)*it;
        node->flush();
    });
}

void Writer::close(izanagi::threadmodel::CThreadPool& theadPool)
{
    waitForFlushing();
    
    m_waitMain.Wait();

    // 削除リストにあるノードを処理する.
    m_octree.cleanDeleteList();

    auto& list = m_octree.getNodeList();

    auto num = list.size();

    izanagi::threadmodel::CParallel::For(
        theadPool,
        0, num,
        [&](IZ_INT idx) {
        auto it = list.begin();
        std::advance(it, idx);

        Node* node = (Node*)*it;
        node->close();
    });

    izanagi::threadmodel::CParallel::waitFor();
}

void Writer::waitForFlushing()
{
    izanagi::threadmodel::CParallel::waitFor();
}
