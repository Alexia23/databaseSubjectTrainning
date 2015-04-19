#include "VectorItem.h"
#include <algorithm>

using namespace std;

VectorItem::VectorItem()
{
}

VectorItem::~VectorItem()
{
}

void VectorItem::insert(int no)
{
	ivector.push_back(no);
}

vector<int> & VectorItem::getVector()
{
	return ivector;
}

void VectorItem::sort()
{
	std::sort(ivector.begin(), ivector.end());
}

int VectorItem::size() const
{
	return ivector.size();
}
