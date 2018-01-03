#include "stdafx.h"
#include "WorldTreeNode.h"


WorldTreeNode::WorldTreeNode()
{
}


WorldTreeNode::~WorldTreeNode()
{
}

void WorldTreeNode::SetBBox(glm::vec3 boxMin, glm::vec3 boxMax)
{
	m_BBoxMin = boxMin;
	m_BBoxMax = boxMax;

	m_fCenterX = (boxMax.x + boxMin.x) * 0.5f;
	m_fCenterZ = (boxMax.z + boxMin.z) * 0.5f;

	m_fSmallestDim = LTMIN(boxMax.x - boxMin.x, boxMax.z - boxMin.z);
}

bool WorldTreeNode::Load(FILE * pFile, uint8 & curByte, uint8 & curBit)
{
	uint32 i;
	bool bSubdivide;
	// Read the next bit.
	if (curBit == 8)
	{
		fread(&curByte,sizeof(curByte),1,pFile);
		curBit = 0;
	}
	bSubdivide = !!(curByte & (1 << curBit));
	++curBit;
	if (bSubdivide)
	{
		if (!Subdivide())
			return LTFALSE;

		for (i = 0; i < MAX_WTNODE_CHILDREN; i++)
		{
			if (!m_ChildrenA[i]->Load(pFile, curByte, curBit))
			{
				TermChildren();
				return LTFALSE;
			}
		}
	}

	return LTTRUE;
}

bool WorldTreeNode::Subdivide()
{
	uint32 i;
	glm::vec3 vSubSize, vSubMin, vSubMax;


	// Allocate..
	for (i = 0; i < MAX_WTNODE_CHILDREN; i++)
	{
		m_ChildrenA[i] = std::make_unique<WorldTreeNode>();
		if (!m_ChildrenA[i])
		{
			Term();
			return LTFALSE;
		}

		m_ChildrenA[i]->m_pParent = this;
	}

	vSubSize = (m_BBoxMax - m_BBoxMin) * 0.5f;
	/*
	// -x -z
	m_Children[0][0]->SetBBox(
		LTVector(m_vCenter.x - vSubSize.x, m_BBoxMin.y, m_vCenter.z - vSubSize.z),
		LTVector(m_vCenter.x, m_BBoxMax.y, m_vCenter.z));

	// -x +z
	m_Children[0][1]->SetBBox(
		LTVector(m_vCenter.x - vSubSize.x, m_BBoxMin.y, m_vCenter.z),
		LTVector(m_vCenter.x, m_BBoxMax.y, m_vCenter.z + vSubSize.z));

	// +x -z
	m_Children[1][0]->SetBBox(
		LTVector(m_vCenter.x, m_BBoxMin.y, m_vCenter.z - vSubSize.z),
		LTVector(m_vCenter.x + vSubSize.x, m_BBoxMax.y, m_vCenter.z));

	// +x +z
	m_Children[1][1]->SetBBox(
		LTVector(m_vCenter.x, m_BBoxMin.y, m_vCenter.z),
		LTVector(m_vCenter.x + vSubSize.x, m_BBoxMax.y, m_vCenter.z + vSubSize.z));
		*/
	return LTTRUE;
}

void WorldTreeNode::TermChildren()
{
	uint32 i;

	for (i = 0; i < MAX_WTNODE_CHILDREN; i++)
	{
		if (m_ChildrenA[i])
		{
			delete m_ChildrenA[i].release();
		}
	}

}

void WorldTreeNode::Term()
{
	uint32 i;
	LTLink *pCur, *pNext;
	WorldTreeObj *pObj;

	// Free child nodes.
	TermChildren();

	// Remove all the objects, so they don't have bad pointers into us.
	for (i = 0; i < NUM_NODEOBJ_ARRAYS; i++)
	{
		for (pCur = m_Objects[i].m_pNext; pCur != &m_Objects[i]; pCur = pNext)
		{
			pNext = pCur->m_pNext;
			pObj = ((WorldTreeObj*)pCur->m_pData);
			pObj->RemoveFromWorldTree();
		}
	}

	Clear();
}

void WorldTreeNode::Clear()
{
	m_pParent = nullptr;

	for (uint32 i = 0; i < NUM_NODEOBJ_ARRAYS; i++)
	{
		m_Objects[i].TieOff();
	}

	m_nObjectsOnOrBelow = 0;

	m_fCenterX = 0.0f;
	m_fCenterZ = 0.0f;

	m_fSmallestDim = 0.0f;
}

void WorldTreeNode::AddObjectToList(WTObjLink * pLink, NodeObjArray iArray)
{
	WorldTreeNode *pTempNode;


	m_Objects[iArray].AddAfter(&pLink->m_Link);
	pLink->m_pNode = this;

	// Add a reference to all the nodes above here.
	pTempNode = this;
	while (pTempNode)
	{
		pTempNode->m_nObjectsOnOrBelow++;
		pTempNode = pTempNode->m_pParent;
	}
}

WorldTreeObj::WorldTreeObj(WTObjType objType)
{
	uint32 i;
	WTObjLink *pLink;

	for (i = 0; i < MAX_OBJ_NODE_LINKS; i++)
	{
		pLink = &m_Links[i];

		pLink->m_Link.TieOff();
		pLink->m_Link.m_pData = this;
		pLink->m_pNode = NULL;
	}

	m_ObjType = objType;
	m_WTFrameCode = FRAMECODE_NOTINTREE;
}

WorldTreeObj::~WorldTreeObj()
{
}

void WorldTreeObj::RemoveFromWorldTree()
{
}
