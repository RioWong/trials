#if !defined(__VERTEX_STREAM_MANAGER_H__)
#define __VERTEX_STREAM_MANAGER_H__

#include "izStd.h"
#include "izSystem.h"
#include "izGraph.h"

struct DrawArraysIndirectCommand
{
    GLuint count;
    GLuint primCount;
    GLuint first;
    GLuint baseInstance;
};

// ���_�f�[�^���̓C���^�[�t�F�[�X.
class IVertexStreamInput {
protected:
    IVertexStreamInput() {}
    virtual ~IVertexStreamInput() {}

public:
    // �X�V���K�v���ǂ���.
    virtual IZ_BOOL needUpdate() = 0;

    // �������݉\���ǂ���.
    virtual IZ_BOOL canWrite(IZ_UINT size) = 0;

    // �w��o�b�t�@�ɏ�������.
    virtual IZ_UINT writeToBuffer(void* buffer, IZ_UINT offset) = 0;

    // �������݃T�C�Y���擾.
    virtual IZ_UINT getSize() const = 0;

    // �������݃f�[�^�����擾.
    virtual IZ_UINT getCount() const = 0;

    // �������݃I�t�Z�b�g�ʒu���擾.
    IZ_UINT getOffset() const
    {
        return m_offset;
    }

protected:
    IZ_UINT m_offset{ 0 };
};

// ���_�f�[�^�X�g���[���}�l�[�W���[.
class VertexStreamManager {
public:
    VertexStreamManager() {}
    ~VertexStreamManager() {}

public:
    void init(
        izanagi::IMemoryAllocator* allocator,
        izanagi::graph::CGraphicsDevice* device,
        IZ_UINT vtxStride,
        IZ_UINT vtxNum);

    void terminate();

    // �`��R�}���h���擾.
    std::tuple<void*, IZ_UINT> getCommands();

    // �X���b�h�Z�[�t�œ��̓f�[�^��ǉ�.
    IZ_INT addInputSafely(IVertexStreamInput* input);

    // ���̓f�[�^��ǉ����J�n.
    void beginAddInput();

    // ���̓f�[�^��ǉ����I��.
    void endAddInput();

    // ���̓f�[�^��ǉ�.
    IZ_INT addInput(IVertexStreamInput* input);

    // ���̓f�[�^���폜.
    IZ_BOOL removeInput(IVertexStreamInput* input);

    // �w��C���f�b�N�X�̓��̓f�[�^���폜.
    IZ_BOOL removeInputByIdx(IZ_UINT idx);

    // For debug.
    void notifyUpdateForcibly();

    izanagi::graph::CVertexBuffer* getVB()
    {
        return m_vbDynamicStream;
    }

private:
    void procThread();

    // �o�b�t�@�̋󂫈ʒu���.
    struct EmptyInfo {
        IZ_UINT offset{ 0 };
        IZ_UINT size{ 0 };

        EmptyInfo(IZ_UINT _offset, IZ_UINT _size) : offset(_offset), size(_size) {}

        bool operator==(const EmptyInfo& rhs)
        {
            return (offset == rhs.offset && size == rhs.size);
        }
    };

    // �������̓��̓f�[�^���擾.
    IVertexStreamInput* getUnprocessedInput();

    // �󂫏����X�V.
    void updateEmptyInfo(EmptyInfo& info, IZ_UINT wroteSize);

    // �󂫏���ǉ�.
    void addEmptyInfo(IZ_UINT offset, IZ_UINT size);

    // �`��R�}���h���X�V.
    void updateCommand(
        IZ_UINT count, 
        IZ_UINT offsetByte);

    // �`��R�}���h���폜.
    void removeCommand(IZ_UINT offsetByte);

    // ���X�g�ɗv�f��ǉ�.
    template <typename _T>
    static IZ_INT add(_T& value, std::vector<_T>& list, std::function<void()> func);

    // ���X�g����v�f���폜.
    template <typename _T>
    static IZ_BOOL remove(_T& value, std::vector<_T>& list, std::function<void()> func);

    // ���X�g����w��C���f�b�N�X�̗v�f���폜.
    template <typename _T>
    static _T removeByIdx(IZ_UINT idx, std::vector<_T>& list, std::function<void()> func);

private:
    static const IZ_UINT NUM = 4;

    // �������݃��X�g�C���f�b�N�X.
    std::atomic<IZ_UINT> m_writingListIdx{ 0 };

    // �`��Q�ƃ��X�g�C���f�b�N�X.
    std::atomic<IZ_UINT> m_drawingListIdx{ 0 };

    // �`��R�}���h���X�g�̎Q�ƈʒu�ύX�t���O.
    std::atomic<bool> m_needChangeCmd{ false };

    // �`��R�}���h.
    izanagi::sys::CSpinLock m_lockerCmd;
    std::vector<DrawArraysIndirectCommand> m_comands[NUM];
    
    GLuint m_glVB{ 0 };

    GLuint m_bufferSize{ 0 };
    IZ_UINT m_vtxStride{ 0 };
    IZ_UINT m_vtxNum{ 0 };

    void* m_mappedDataPtr{ nullptr };
    izanagi::graph::CVertexBuffer* m_vbDynamicStream{ nullptr };

    // ���̓f�[�^.
    izanagi::sys::CSpinLock m_lockerInputs;
    std::vector<IVertexStreamInput*> m_inputs;

    // ���̓f�[�^���X�g�̍X�V�ʒm�p.
    izanagi::sys::CEvent m_notifyUpdate;

    // �X���b�h�I���t���O.
    std::atomic<IZ_BOOL> m_willTerminate{ IZ_FALSE };

    std::thread m_thread;

    // �󂫏��.
    izanagi::sys::CSpinLock m_lockerEmptyInfo;
    std::vector<EmptyInfo> m_emptyInfo;
};

#endif    // #if !defined(__VERTEX_STREAM_MANAGER_H__)