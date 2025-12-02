#pragma once
#include <afxtempl.h> 

// 점 정보를 저장할 구조체
struct NODE {
	CPoint pt;      // 좌표
	int id;         // 고유 번호
};

// 선 정보를 저장할 구조체
struct EDGE {
	int startNode;  // 시작점 
	int endNode;    // 끝점 
	double dist;    // 거리
};

// CChildView 창

class CChildView : public CWnd
{
	// 생성
public:
	CChildView();

	// 특성
public:
	CArray<NODE, NODE&> m_nodeList;
	CArray<EDGE, EDGE&> m_edgeList;

	// 최단 경로에 포함된 선의 인덱스를 저장할 배열
	CArray<int, int> m_shortestPath;

	CBitmap m_bgImage; // 배경 
	int m_nSelNode;    // 현재 선택된 점

	// 작업
public:

	int HitTestNode(CPoint pt);
	void FindShortestPath(int startNode, int endNode);

	// 재정의
protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	// 구현
public:
	virtual ~CChildView();

	// 생성된 메시지 맵 함수
protected:
	afx_msg void OnPaint();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};