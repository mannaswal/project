// ChildView.cpp : implementation of the CChildView class
//

#include "stdafx.h"
#include "2DStableFluids.h"
#include "ChildView.h"
#include <string> // Include for std::to_wstring

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CChildView

CChildView::CChildView()
{
	windowSize = 600;
	dx = windowSize/fluidSolver.n;
	showDensity = true;
	showVelocity = false;
	showGrid = false;
	m_timer = 0;
	leftButton = false;
	rightButton = false;
}

CChildView::~CChildView()
{
}


BEGIN_MESSAGE_MAP(CChildView, CWnd)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_RBUTTONDOWN()
	ON_WM_RBUTTONUP()
	ON_WM_MOUSEMOVE()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()



// CChildView message handlers

BOOL CChildView::PreCreateWindow(CREATESTRUCT& cs) 
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;

	cs.dwExStyle |= WS_EX_CLIENTEDGE;
	cs.style &= ~WS_BORDER;
	cs.lpszClass = AfxRegisterWndClass(CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS, 
		::LoadCursor(NULL, IDC_ARROW), reinterpret_cast<HBRUSH>(COLOR_WINDOW+1), NULL);

	return TRUE;
}

void CChildView::OnPaint() 
{
	CPaintDC dc(this); // device context for painting
	
	CRect   rect;
	
	this->GetClientRect(rect);
    // double buffering to remove flickering
	
    CDC MemDC; 
    CBitmap MemBitmap;
    MemDC.CreateCompatibleDC(NULL);
    MemBitmap.CreateCompatibleBitmap(&dc,windowSize+1,windowSize+1);
    CBitmap *pOldBit=MemDC.SelectObject(&MemBitmap);
	MemDC.FillSolidRect(0, 0, windowSize+1, windowSize+1, RGB(255,255,255));
    // clear the screen
    CBrush brush;
    // repaint the window with solid black background
    brush.CreateSolidBrush(RGB(255,255,255));

	COLORREF qCircleColor = RGB(0,0,0);
    CPen qCirclePen(PS_SOLID, 5, qCircleColor);
    CPen* pqOrigPen = MemDC.SelectObject(&qCirclePen);

	int grid_number = fluidSolver.n;
	
	if (showDensity)
	{
		for (int cell_i = 0; cell_i < grid_number; cell_i++)
			for (int cell_j = 0; cell_j < grid_number; cell_j++)
			{
				int c = (int) ( (1 - fluidSolver.density[cell_i+cell_j*grid_number]) * 255);
				if (c < 0)
					c = 0;
				if (c > 255)
					c = 255;
				qCircleColor = RGB(c, c, c);                  		
 				MemDC.FillSolidRect(cell_i * dx, cell_j * dx, dx, dx, qCircleColor);			
			}
	}

	if (showVelocity)
	{
		//draw new velocity
		COLORREF qLineColor = RGB(0,0,255);
		CPen qLinePen(PS_SOLID, 1, qLineColor);
		MemDC.SelectObject(&qLinePen);
		vec2 *v;

		for (int cell_i = 0; cell_i < grid_number; cell_i++)
			for (int cell_j = 0; cell_j < grid_number; cell_j++)
			{
				v = fluidSolver.v(cell_i, cell_j);
				MemDC.MoveTo((cell_i) * dx, (cell_j) * dx);
				MemDC.LineTo((int) ((cell_i) * dx + v->x * dx) , (int) ( (cell_j) * dx + v->y * dx));
			}
	}

	//Draw Grid lines
	if (showGrid)
	{
		// Primal edges
		COLORREF qLineColor = RGB(125,125,125);
		CPen qLinePen(PS_SOLID, 1, qLineColor);
		MemDC.SelectObject(&qLinePen);
		for (int cell = 0; cell < grid_number; cell++)
		{
			// hrizental grid lines
			MemDC.MoveTo(0, (cell + 1) * dx);
			MemDC.LineTo(windowSize, (cell + 1) * dx);
			//vertical grid lines
			MemDC.MoveTo((cell + 1) * dx, 0);
			MemDC.LineTo((cell + 1) * dx, windowSize);
		}
	}

	dc.BitBlt(0,0,windowSize+1,windowSize+1,&MemDC,0,0,SRCCOPY);
    MemBitmap.DeleteObject();
    MemDC.DeleteDC();


	int TextWidth = 250;
	int TextHeight = 300;
	CDC MemDC1; 
    CBitmap MemBitmap1;
    MemDC1.CreateCompatibleDC(NULL);
    MemBitmap1.CreateCompatibleBitmap(&dc,TextWidth,TextHeight);
    CBitmap *pOldBit1=MemDC1.SelectObject(&MemBitmap1);
	//Background color for text
	MemDC1.FillSolidRect(0, 0, TextWidth, TextHeight, RGB(0,20,20));
  
	MemDC1.SetTextColor(RGB(0,255,255));
	CString s1,s2;
	s1 = "n = ";
	s2.Format(_T("%d"),grid_number);
	s2 = s1+s2;
	MemDC1.TextOutW(3,10,s2);

	s1 = "Viscosity = ";
	s2.Format(_T("%.3f"), fluidSolver.viscosity);
	s2 = s1 + s2;
	MemDC1.TextOutW(3, 30, s2);

	MemDC1.SetTextColor(RGB(255,255,255));
	int row = 55;
	MemDC1.TextOutW(3,row,_T("User Interface Guide:"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("Z : Start/Stop Animation"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("Left button: inject smoke"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("Right button: stir fluid"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("R : Reset"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("V : Show/Hide Velocity"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("D : Show/Hide Density"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("G : Show/Hide Gridlines"));
	row += 20;
	MemDC1.TextOutW(8,row,_T("A : One time step"));
	row += 20;
	MemDC1.TextOutW(8, row, _T("+/= : Increase Viscosity"));
	row += 20;
	MemDC1.TextOutW(8, row, _T("-   : Decrease Viscosity"));

	dc.BitBlt(windowSize+1,0,TextWidth,TextHeight,&MemDC1,0,0,SRCCOPY);
    MemBitmap1.DeleteObject();
    MemDC1.DeleteDC();

}

void CChildView::OnTimer(UINT_PTR nIDEvent)
{
	if (leftButton) {
		int index = Find_Cell_Index(current_point);	 
		// Inject density
		fluidSolver.density_source[index] = 50.* fluidSolver.h;
	}

	fluidSolver.update();

	Invalidate(false);
	CWnd::OnTimer(nIDEvent);
}


void CChildView::OnLButtonDown(UINT nFlags, CPoint point)
{
	leftButton = true;
	current_point = old_point = point;
	CWnd::OnLButtonDown(nFlags, point);
}


void CChildView::OnLButtonUp(UINT nFlags, CPoint point)
{
	leftButton = false;
	CWnd::OnLButtonUp(nFlags, point);
}


void CChildView::OnRButtonDown(UINT nFlags, CPoint point)
{
	rightButton = true;
	current_point = old_point = point;
	CWnd::OnRButtonDown(nFlags, point);
}


void CChildView::OnRButtonUp(UINT nFlags, CPoint point)
{
	rightButton = false;

	CWnd::OnRButtonUp(nFlags, point);
}


void CChildView::OnMouseMove(UINT nFlags, CPoint point)
{
	if (leftButton || rightButton) {
		old_point = current_point;
		current_point = point;
	}

	if (leftButton) {
		int index = Find_Cell_Index(current_point);
		fluidSolver.density_source[index] = 50. * fluidSolver.h;
	}

	if (rightButton) {
		int index = Find_Cell_Index(current_point);
		fluidSolver.velocity_source[index] = vec2(current_point.x-old_point.x,current_point.y-old_point.y)* 2.0;
	}

	CWnd::OnMouseMove(nFlags, point);
}


void CChildView::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	double viscosity_change = 0.01;

	switch( nChar)
	{
	case 'a':
	case 'A':
		fluidSolver.update();
		Invalidate(false);
		break;
	case 'Z':
	case 'z':
		if (m_timer) {
			KillTimer(m_timer);
			m_timer = 0;
		}
		else
		{
		   m_timer = SetTimer(1, (int) (25), NULL);
		}
		break;
	case 'r':
	case 'R':
		fluidSolver.reset();
		Invalidate(false);
		break;
	case 'D':
	case 'd':
		showDensity = !showDensity;
		Invalidate(false);
		break;
	case 'v':
	case 'V':
		showVelocity = !showVelocity;
		Invalidate(false);
		break;
	case 'G':
	case 'g':
		showGrid = !showGrid;
		InvalidateRect(NULL,FALSE);
		break;
	case VK_OEM_PLUS:
	case VK_ADD:
	case '=':
		fluidSolver.viscosity += viscosity_change;
		fluidSolver.update_velocity_diffusion_matrix();
		Invalidate(false);
		break;
	case VK_OEM_MINUS:
	case VK_SUBTRACT:
	case '-':
		fluidSolver.viscosity -= viscosity_change;
		if (fluidSolver.viscosity < 0) {
			fluidSolver.viscosity = 0;
		}
		fluidSolver.update_velocity_diffusion_matrix();
		Invalidate(false);
		break;
	}

	CWnd::OnKeyDown(nChar, nRepCnt, nFlags);
}

int CChildView::Find_Cell_Index(CPoint point)
{
	int x = point.x;
	int y = point.y;
	// set boundaries for mouse input to be inside the rectangular
	if (x >= windowSize - 1)
		x = windowSize - 2;
	if (x < 0)
		x = 0;
	if (y >= windowSize - 1)
		y = windowSize - 2;
	if (y < 0)
		y = 0;
	// find the cell index of mouse input
	int cell_i = x / dx;
	int cell_j = y / dx;

	// Clamp indices just in case
	if (cell_i >= fluidSolver.n) cell_i = fluidSolver.n - 1;
	if (cell_i < 0) cell_i = 0;
	if (cell_j >= fluidSolver.n) cell_j = fluidSolver.n - 1;
	if (cell_j < 0) cell_j = 0;

	return cell_i + fluidSolver.n * cell_j;
}
