
#include <iostream>
#include <math.h>

#include "SparseGrid.h"
#include "GridIndex.h"

using std::min;

namespace Potree{

    // spacing ���l�������m�[�h.
    // �m�[�h�ɑ΂��� 1:1 �ŕR�Â�.

SparseGrid::SparseGrid(AABB aabb, float spacing){
	this->aabb = aabb;

    // �m�[�h����spacing�ɉ����ăZ���������邪�A���̃T�C�Y.
    // 5.0 �͉��H�␳�H
	this->width =	(int)(aabb.size.x / (spacing * 5.0) );
	this->height =	(int)(aabb.size.y / (spacing * 5.0) );
	this->depth =	(int)(aabb.size.z / (spacing * 5.0) );

	this->squaredSpacing = spacing * spacing;
}

SparseGrid::~SparseGrid(){
	SparseGrid::iterator it;
	for(it = begin(); it != end(); it++){
		delete it->second;
	}
}

void SparseGrid::init(AABB aabb, float spacing)
{
    this->aabb = aabb;
    this->width = (int)(aabb.size.x / (spacing * 5.0));
    this->height = (int)(aabb.size.y / (spacing * 5.0));
    this->depth = (int)(aabb.size.z / (spacing * 5.0));
    this->squaredSpacing = spacing * spacing;
}

bool SparseGrid::isDistant(const Vector3<double> &p, GridCell *cell){
	if(!cell->isDistant(p, squaredSpacing)){
		return false;
	}

	for(const auto &neighbour : cell->neighbours) {
		if(!neighbour->isDistant(p, squaredSpacing)){
			return false;
		}
	}

	return true;
}

bool SparseGrid::willBeAccepted(const Vector3<double> &p){
	int nx = (int)(width*(p.x - aabb.min.x) / aabb.size.x);
	int ny = (int)(height*(p.y - aabb.min.y) / aabb.size.y);
	int nz = (int)(depth*(p.z - aabb.min.z) / aabb.size.z);

	int i = min(nx, width-1);
	int j = min(ny, height-1);
	int k = min(nz, depth-1);

	GridIndex index(i,j,k);
	long long key = ((long long)k << 40) | ((long long)j << 20) | (long long)i;
	SparseGrid::iterator it = find(key);
	if(it == end()){
		it = this->insert(value_type(key, new GridCell(this, index))).first;
	}

	if(isDistant(p, it->second)){
		return true;
	}else{
		return false;
	}
}

bool SparseGrid::add(Vector3<double> &p){
    // �ǂ̃Z���ɏ������邩�����߂�.
	int nx = (int)(width*(p.x - aabb.min.x) / aabb.size.x);
	int ny = (int)(height*(p.y - aabb.min.y) / aabb.size.y);
	int nz = (int)(depth*(p.z - aabb.min.z) / aabb.size.z);

	int i = min(nx, width-1);
	int j = min(ny, height-1);
	int k = min(nz, depth-1);

	GridIndex index(i,j,k);
	long long key = ((long long)k << 40) | ((long long)j << 20) | (long long)i;
	SparseGrid::iterator it = find(key);
	if(it == end()){
		it = this->insert(value_type(key, new GridCell(this, index))).first;
	}

    // �w�肳�ꂽ�Z���Ɋ܂܂��_�Ƃ̋��������ȏ�ispacing�j����Ă��邩�ǂ������`�F�b�N.
	if(isDistant(p, it->second)){
        // ����Ă���ꍇ�́A�Z���ɓo�^.
		this->operator[](key)->add(p);

        // �󂯓��ꂽ�_�̐��𑝂₷.
		numAccepted++;

		return true;
	}else{
		return false;
	}
}

void SparseGrid::addWithoutCheck(Vector3<double> &p){
	int nx = (int)(width*(p.x - aabb.min.x) / aabb.size.x);
	int ny = (int)(height*(p.y - aabb.min.y) / aabb.size.y);
	int nz = (int)(depth*(p.z - aabb.min.z) / aabb.size.z);

	int i = min(nx, width-1);
	int j = min(ny, height-1);
	int k = min(nz, depth-1);

	GridIndex index(i,j,k);
	long long key = ((long long)k << 40) | ((long long)j << 20) | (long long)i;
	SparseGrid::iterator it = find(key);
	if(it == end()){
		it = this->insert(value_type(key, new GridCell(this, index))).first;
	}

	it->second->add(p);
}

}