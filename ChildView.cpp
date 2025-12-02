
#include "pch.h"
#include "framework.h"
#include "Bitmaps.h" 
#include "ChildView.h"
#include "resource.h" 
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	// 배경 비트맵
	m_bgImage.LoadBitmap(IDB_BITMAP1);

	// 선택된 점 초기화 (-1은 선택 안됨을 의미)
	m_nSelNode = -1;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_LBUTTONDOWN()
END_MESSAGE_MAP()


// CChildView 메시지 처리기

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS,
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1), NULL);

	return TRUE;
}

void CChildView::OnPaint()
{
	CPaintDC dc(this);

	//  비트맵 출력
	CDC memDC;
	memDC.CreateCompatibleDC(&dc);

	CBitmap* pOldBitmap = memDC.SelectObject(&m_bgImage);

	BITMAP bmpInfo;
	m_bgImage.GetBitmap(&bmpInfo);
		dc.BitBlt(0, 0, bmpInfo.bmWidth, bmpInfo.bmHeight, &memDC, 0, 0, SRCCOPY);
	memDC.SelectObject(pOldBitmap);


		// 최단 경로 그리기
		if (m_shortestPath.GetSize() > 0)
		{

			CPen penRed(PS_SOLID, 5, RGB(255, 0, 0));
			CPen* pOldPen2 = dc.SelectObject(&penRed);

			for (int i = 0; i < m_shortestPath.GetSize(); i++)
			{
				int edgeIdx = m_shortestPath[i];
				EDGE e = m_edgeList[edgeIdx];

				CPoint ptStart = m_nodeList[e.startNode].pt;
				CPoint ptEnd = m_nodeList[e.endNode].pt;

				dc.MoveTo(ptStart);
				dc.LineTo(ptEnd);
			}
			dc.SelectObject(pOldPen2);
		}


	CPen pen(PS_SOLID, 2, RGB(0, 0, 255));
	CPen* pOldPen = dc.SelectObject(&pen);
		int nOldBkMode = dc.SetBkMode(TRANSPARENT);

	for (int i = 0; i < m_edgeList.GetSize(); i++)
	{
		EDGE e = m_edgeList[i];
		CPoint ptStart = m_nodeList[e.startNode].pt;
		CPoint ptEnd = m_nodeList[e.endNode].pt;
			dc.MoveTo(ptStart);
		dc.LineTo(ptEnd);

			CString strDist;
		strDist.Format(_T("%.1f"), e.dist);

		// 선의 중간 위치에 거리 표시
		int midX = (ptStart.x + ptEnd.x) / 2;
		int midY = (ptStart.y + ptEnd.y) / 2;
		dc.TextOut(midX, midY, strDist);
	}

	dc.SetBkMode(nOldBkMode);
	dc.SelectObject(pOldPen);


		// 점 그리기
		CBrush brush(RGB(0, 0, 255)); // 파란색 점 사용
	CBrush* pOldBrush = dc.SelectObject(&brush);

	for (int i = 0; i < m_nodeList.GetSize(); i++)
	{
		CPoint pt = m_nodeList[i].pt;
			dc.Ellipse(pt.x - 5, pt.y - 5, pt.x + 5, pt.y + 5);
	}

	dc.SelectObject(pOldBrush);
}

void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	// 클릭한 위치에 점이 있는지 확인
	int nHitIndex = HitTestNode(point);

	// Ctrl 키 + 기존 점 클릭 -> 선 연결
	if (nFlags & MK_CONTROL)
	{
		if (nHitIndex != -1)
		{
			if (m_nSelNode == -1)
			{
				// 첫 번째 점 선택
				m_nSelNode = nHitIndex;
				AfxMessageBox(_T("첫 번째 점 선택. 연결할 다음 점을 Ctrl+클릭 하세요."));
			}
			else
			{
				// 두 번째 점 선택
				if (m_nSelNode != nHitIndex)
				{
					EDGE newEdge;
					newEdge.startNode = m_nSelNode;
					newEdge.endNode = nHitIndex;

					// 거리 계산
					double dx = m_nodeList[m_nSelNode].pt.x - m_nodeList[nHitIndex].pt.x;
					double dy = m_nodeList[m_nSelNode].pt.y - m_nodeList[nHitIndex].pt.y;
					newEdge.dist = sqrt(dx * dx + dy * dy);

					m_edgeList.Add(newEdge);
					m_nSelNode = -1; 
					Invalidate();
				}
			}
		}
	}
	// Shift 키 + 점 클릭 -> 최단 경로 찾기
	else if (nFlags & MK_SHIFT)
	{
		if (nHitIndex != -1)
		{
			if (m_nSelNode == -1)
			{
				m_nSelNode = nHitIndex; // 시작점 선택
				AfxMessageBox(_T("출발점 선택! 도착점을 Shift+클릭 하세요."));
			}
			else
			{
				// 도착점 선택
				FindShortestPath(m_nSelNode, nHitIndex);
				m_nSelNode = -1; // 초기화
				Invalidate();
			}
		}
	}
	// 그냥 클릭 -> 점 추가
	else
	{
		// 빈 공간 클릭
		if (nHitIndex == -1)
		{
			NODE newNode;
			newNode.pt = point;
			newNode.id = (int)m_nodeList.GetSize();

			m_nodeList.Add(newNode);
			Invalidate();
		}
	}

	CWnd::OnLButtonDown(nFlags, point);
}

int CChildView::HitTestNode(CPoint point)
{
	for (int i = 0; i < m_nodeList.GetSize(); i++)
	{
		CPoint nodePt = m_nodeList[i].pt;
		if (abs(point.x - nodePt.x) < 10 && abs(point.y - nodePt.y) < 10)
		{
			return i; // 인덱스 반환
		}
	}
	return -1;
}

void CChildView::FindShortestPath(int startNode, int endNode)
{
	int nNodeCount = (int)m_nodeList.GetSize(); //점 개수 세기

	if (nNodeCount == 0 || startNode >= nNodeCount || endNode >= nNodeCount) return; //점의 유무로 함수 종료

	// 메모리 할당
	double* dist = new double[nNodeCount];
	int* parent = new int[nNodeCount];
	bool* visited = new bool[nNodeCount];

	// 초기화
	for (int i = 0; i < nNodeCount; i++) {
		dist[i] = 9999999.0;
		parent[i] = -1;
		visited[i] = false;
	}

	dist[startNode] = 0.0;

	for (int i = 0; i < nNodeCount; i++) //점 개수만큼 박복하여 길 찾기
	{
		int u = -1;
		double minDist = 9999999.0;

		for (int j = 0; j < nNodeCount; j++) {
			if (!visited[j] && dist[j] < minDist) {
				minDist = dist[j];
				u = j;
			}
		}

		if (u == -1 || dist[u] == 9999999.0) break;
		if (u == endNode) break; 

		visited[u] = true;

		for (int k = 0; k < m_edgeList.GetSize(); k++) //지도에 있는 모든 선 검사
		{
			EDGE e = m_edgeList[k];
			int v = -1;

			if (e.startNode == u) v = e.endNode;
			else if (e.endNode == u) v = e.startNode;

			if (v != -1 && !visited[v])
			{
				if (dist[u] + e.dist < dist[v])
				{
					dist[v] = dist[u] + e.dist;
					parent[v] = u;
				}
			}
		}
	}

	// 경로 역추적
	m_shortestPath.RemoveAll();

	int curr = endNode;
	while (curr != -1 && curr != startNode)
	{
		int prev = parent[curr];
		if (prev == -1) break;

		// 연결된 선을 찾아 경로에 추가
		for (int k = 0; k < m_edgeList.GetSize(); k++)
		{
			EDGE e = m_edgeList[k];
			if ((e.startNode == prev && e.endNode == curr) ||
				(e.startNode == curr && e.endNode == prev))
			{
				m_shortestPath.Add(k);
				break;
			}
		}
		curr = prev;
	}

	// 결과 출력
	if (curr != startNode) {
		AfxMessageBox(_T("경로를 찾을 수 없습니다."));
	}
	else {
		CString msg;
		msg.Format(_T("최단 경로 찾기 완료! (총 거리: %.1f)"), dist[endNode]);
		AfxMessageBox(msg);
	}

	// 메모리 해제
	delete[] dist;
	delete[] parent;
	delete[] visited;
}