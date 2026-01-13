#ifndef MOUSE_EVT_PROC_H
#define MOUSE_EVT_PROC_H

#include "qpoint.h"
#include "QEvent.h"

#include "map"
//
// mouse event proc interface
//
class CMouseEvtProcOwner
{
public:
	 
	virtual void chgZoom(QPoint delta ) = 0;
 	virtual void chgContrast(QPoint delta) = 0;
 	virtual void chgPan(QPoint delta) = 0;
	virtual void onCursorMove(QPoint point) = 0;
	virtual void onWheelEvent(int d) = 0;
	virtual void onChgCursorStatus() = 0;
};
 
class CMouseEvtProc 
{
	 
public:
	CMouseEvtProc() 
	{
		m_pOwner = 0;
		m_working = false;
	};

	virtual bool mousePressEvent(QMouseEvent *) = 0;
    virtual bool mouseReleaseEvent(QMouseEvent *) = 0;
    virtual bool mouseDoubleClickEvent(QMouseEvent *) = 0;
    virtual bool mouseMoveEvent(QMouseEvent *) = 0;
 
    virtual bool wheelEvent(QWheelEvent *)  ;

	void setupOwner(CMouseEvtProcOwner *owner) { m_pOwner = owner;}; 
protected:
	CMouseEvtProcOwner *m_pOwner;
	QPoint m_curPos;
	bool	m_working;
};



class CMouseEvtProcZoom : public CMouseEvtProc
{
	 
public:
	CMouseEvtProcZoom() ;
	virtual ~CMouseEvtProcZoom() ;

	virtual bool mousePressEvent(QMouseEvent *) ;
    virtual bool mouseReleaseEvent(QMouseEvent *)  ;
    virtual bool mouseDoubleClickEvent(QMouseEvent *)  ;
    virtual bool mouseMoveEvent(QMouseEvent *) ;
 
 //   virtual bool wheelEvent(QWheelEvent *) ;

};

class CMouseEvtProcPan : public CMouseEvtProc
{
	 
public:
	CMouseEvtProcPan() ;
	virtual ~CMouseEvtProcPan() ;

	virtual bool mousePressEvent(QMouseEvent *) ;
    virtual bool mouseReleaseEvent(QMouseEvent *)  ;
    virtual bool mouseDoubleClickEvent(QMouseEvent *)  ;
    virtual bool mouseMoveEvent(QMouseEvent *) ;
 
//    virtual bool wheelEvent(QWheelEvent *) ;

};

class CMouseEvtProcContrast : public CMouseEvtProc
{
	 
public:
	CMouseEvtProcContrast() ;
	virtual ~CMouseEvtProcContrast() ;

	virtual bool mousePressEvent(QMouseEvent *) ;
    virtual bool mouseReleaseEvent(QMouseEvent *)  ;
    virtual bool mouseDoubleClickEvent(QMouseEvent *)  ;
    virtual bool mouseMoveEvent(QMouseEvent *) ;
 
 //   virtual bool wheelEvent(QWheelEvent *) ;

};

class CMouseEvtProcCursor : public CMouseEvtProc
{
	 
public:
	CMouseEvtProcCursor() ;
	virtual ~CMouseEvtProcCursor() ;

	virtual bool mousePressEvent(QMouseEvent *) ;
    virtual bool mouseReleaseEvent(QMouseEvent *)  ;
    virtual bool mouseDoubleClickEvent(QMouseEvent *)  ;
    virtual bool mouseMoveEvent(QMouseEvent *) ;
 
//    virtual bool wheelEvent(QWheelEvent *) ;

};


class CMouseEvtHander
{
public:
	enum MouseEvtHand_Mode {
		MouseEvtHand_Zoom,
		MouseEvtHand_Pan,
		MouseEvtHand_Contrast,
		MouseEvtHand_Cursor,
	};
	virtual bool mousePressEvent(QMouseEvent *) ;
    virtual bool mouseReleaseEvent(QMouseEvent *)  ;
    virtual bool mouseDoubleClickEvent(QMouseEvent *)  ;
    virtual bool mouseMoveEvent(QMouseEvent *) ;
 
    virtual bool wheelEvent(QWheelEvent *) ;

	void	addMouseEvtProc(MouseEvtHand_Mode mode,CMouseEvtProc *evtProc);
	void setCurrentMouseEvtMode(MouseEvtHand_Mode mode) {m_mode = mode;};
protected:
	CMouseEvtProc *getCurrentEvtProc();
	MouseEvtHand_Mode		m_mode;
	std::map<int ,CMouseEvtProc *> m_mouseProcMap;

};
#endif //MOUSE_EVT_PROC_H


