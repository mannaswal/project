
// ChildView.h : interface of the CChildView class
//
#include "FluidSolver.h"

#pragma once


// CChildView window

class CChildView : public CWnd
{
// Construction
public:
	CChildView();

// Attributes
public:
	int windowSize;			//region for fluid rendering
	int dx;					//grid interval size
	int m_timer;

	//Display options
	bool showDensity;
	bool showVelocity;
	bool showGrid;

	//Interaction states
	bool leftButton;
	bool rightButton;
	CPoint current_point;
	CPoint old_point;

	CFluidSolver fluidSolver;
// Operations
public:
	int Find_Cell_Index(CPoint point);

// Overrides
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

// Implementation
public:
	virtual ~CChildView();

	// Generated message map functions
protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

