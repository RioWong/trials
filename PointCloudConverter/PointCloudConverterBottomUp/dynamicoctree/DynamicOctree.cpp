#include "DynamicOctree.h"
#include "DynamicOctreeNode.h"

uint32_t DynamicOctreeBase::getNewIdx(
    int32_t dirX,
    int32_t dirY,
    int32_t dirZ)
{
    // NOTE
    /*
    * y
    * | +----+----+
    * | | 2  | 3  |
    * | +----+----+
    * | | 0  | 1  |
    * | +----+----+
    * +------------->x
    */
    /*
    * y
    * |     +----+----+
    * |  z  | 6  | 7  |
    * |  /  +----+----+
    * | /   | 4  | 5  |
    * |/    +----+----+
    * +------------->x
    */

    uint32_t ret = (dirX < 0 ? 1 : 0);
    ret += (dirY < 0 ? 2 : 0);
    ret += (dirZ < 0 ? 4 : 0);

    return ret;
}

DynamicOctreeBase::NodeDir DynamicOctreeBase::getNodeDir(uint32_t idx)
{
    // NOTE
    /*
    * y
    * | +----+----+
    * | | 2  | 3  |
    * | +----+----+
    * | | 0  | 1  |
    * | +----+----+
    * +------------->x
    */
    /*
    * y
    * |     +----+----+
    * |  z  | 6  | 7  |
    * |  /  +----+----+
    * | /   | 4  | 5  |
    * |/    +----+----+
    * +------------->x
    */

    // ���S����̕������v�Z.

    auto dirX = ((idx & 0x01) == 0 ? -1 : 1);  // ���i�����jor �E�i��j.
    auto dirZ = (idx >= 4 ? 1 : -1);           // ���i4-7�j or ��O�i0-3).

    // NOTE
    // ��� 2, 3, 6, 7.
    // �Q�i���ɂ���ƁA001, 011, 110, 111 �� �Q�r�b�g�ڂ������Ă���.
    // ���� 0, 1, 4, 5.
    // �Q�i���ɂ���ƁA000, 001, 100, 101 �� �Q�r�b�g�ڂ������Ă��Ȃ�.
    auto dirY = ((idx & 0x02) > 0 ? 1 : -1);   // ��i�Q�r�b�g�ڂ������Ă���jor ���i�Q�r�b�g�ڂ������Ă��Ȃ��j.

    return NodeDir(dirX, dirY, dirZ);
}
