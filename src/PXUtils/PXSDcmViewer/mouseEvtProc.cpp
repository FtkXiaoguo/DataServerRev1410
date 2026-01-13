 
#include "mouseEvtProc.h"
  


bool CMouseEvtHander::mousePressEvent(QMouseEvent *e)
{
	CMouseEvtProc *cur_proc =  getCurrentEvtProc();
	if(cur_proc) {
		return cur_proc->mousePressEvent(e);
	}else{
		return false;
	}
}
bool CMouseEvtHander::mouseReleaseEvent(QMouseEvent *e)
{
	CMouseEvtProc *cur_proc =  getCurrentEvtProc();
	if(cur_proc) {
		return cur_proc->mouseReleaseEvent(e);
	}else{
		return false;
	}
}
bool CMouseEvtHander::mouseDoubleClickEvent(QMouseEvent *e)
{
	CMouseEvtProc *cur_proc =  getCurrentEvtProc();
	if(cur_proc){
		return cur_proc->mouseDoubleClickEvent(e);
	}else{
		return false;
	}
}
bool CMouseEvtHander::mouseMoveEvent(QMouseEvent *e)
{
	CMouseEvtProc *cur_proc =  getCurrentEvtProc();
	if(cur_proc) {
		return cur_proc->mouseMoveEvent(e);
	}else{
		return false;
	}
}
 
bool CMouseEvtHander::wheelEvent(QWheelEvent *e)
{
	CMouseEvtProc *cur_proc =  getCurrentEvtProc();
	if(cur_proc) {
		return cur_proc->wheelEvent(e);
	}else{
		return false;
	}
}
CMouseEvtProc *CMouseEvtHander::getCurrentEvtProc()
{
	CMouseEvtProc *ret_p = 0;
	std::map<int ,CMouseEvtProc *>::iterator it = m_mouseProcMap.begin();
	while(it!=m_mouseProcMap.end()){
		if(it->first == m_mode){
			ret_p = it->second;
			break;
		}
		it++;
	}

	return ret_p;
	 
}

void CMouseEvtHander::addMouseEvtProc(MouseEvtHand_Mode mode,CMouseEvtProc *evtProc)
{
	m_mouseProcMap[mode] = evtProc;
}
//////
 
bool CMouseEvtProc::wheelEvent(QWheelEvent *e)
{
	if(m_pOwner){
		m_pOwner->onWheelEvent(e->delta());
		return true;
	}
	return false;
}


////////
CMouseEvtProcZoom::CMouseEvtProcZoom() 
{

};
CMouseEvtProcZoom::~CMouseEvtProcZoom() 
{

};

bool CMouseEvtProcZoom::mousePressEvent(QMouseEvent *e)
{
	m_curPos = e->pos();
	m_working = true;
	return true;
}
bool CMouseEvtProcZoom::mouseReleaseEvent(QMouseEvent *e)
{
	m_working = false;
	return true;
}
bool CMouseEvtProcZoom::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(m_pOwner){
		m_pOwner->onChgCursorStatus();
		return true;
	}

	return false;
}
bool CMouseEvtProcZoom::mouseMoveEvent(QMouseEvent *e)
{ 
	if(m_working){
		 
		QPoint delta  = e->pos() - m_curPos;
		m_curPos = e->pos();
		if(m_pOwner){
			m_pOwner->chgZoom(delta);
			return true;
		}
		
	}else{
		 
	}
	return false;
}
 
 
/////////
//////
 
CMouseEvtProcPan::CMouseEvtProcPan() 
{

};
CMouseEvtProcPan::~CMouseEvtProcPan() 
{

};

bool CMouseEvtProcPan::mousePressEvent(QMouseEvent *e)
{
	m_curPos = e->pos();
	m_working = true;
	return true;
}
bool CMouseEvtProcPan::mouseReleaseEvent(QMouseEvent *e)
{
	m_working = false;
	return true;
}
bool CMouseEvtProcPan::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(m_pOwner){
		m_pOwner->onChgCursorStatus();
		return true;
	}
	return false;
}
bool CMouseEvtProcPan::mouseMoveEvent(QMouseEvent *e)
{ 
	if(m_working){
		 
		QPoint delta  = e->pos() - m_curPos;
		m_curPos = e->pos();
		if(m_pOwner){
			m_pOwner->chgPan(delta);
			return true;
		}
		
	}else{
		 
	}
	return false;
}
 


////////////

CMouseEvtProcContrast::CMouseEvtProcContrast() 
{

};
CMouseEvtProcContrast::~CMouseEvtProcContrast() 
{

};

bool CMouseEvtProcContrast::mousePressEvent(QMouseEvent *e)
{
	m_curPos = e->pos();
	m_working = true;
	return true;
}
bool CMouseEvtProcContrast::mouseReleaseEvent(QMouseEvent *e)
{
	m_working = false;
	return true;
}
bool CMouseEvtProcContrast::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(m_pOwner){
		m_pOwner->onChgCursorStatus();
		return true;
	}
	return false;
}
bool CMouseEvtProcContrast::mouseMoveEvent(QMouseEvent *e)
{ 
	if(m_working){
		 
		QPoint delta  = e->pos() - m_curPos;
		m_curPos = e->pos();
		if(m_pOwner){
			m_pOwner->chgContrast(delta);
			return true;
		}
		
	}else{
		 
	}
	return false;
}
 
 
//////////////

////////////

CMouseEvtProcCursor::CMouseEvtProcCursor() 
{

};
CMouseEvtProcCursor::~CMouseEvtProcCursor() 
{

};

bool CMouseEvtProcCursor::mousePressEvent(QMouseEvent *e)
{
	m_curPos = e->pos();
	m_working = true;
	return true;
}
bool CMouseEvtProcCursor::mouseReleaseEvent(QMouseEvent *e)
{
	m_working = false;
	return true;
}
bool CMouseEvtProcCursor::mouseDoubleClickEvent(QMouseEvent *e)
{
	if(m_pOwner){
		m_pOwner->onChgCursorStatus();
		return true;
	}
	return false;
}
bool CMouseEvtProcCursor::mouseMoveEvent(QMouseEvent *e)
{ 
	if(m_working){
		 
		QPoint delta  = e->pos() - m_curPos;
		m_curPos = e->pos();
		if(m_pOwner){
			m_pOwner->chgContrast(delta);
			return true;
		}
		
	}else{
	 
		if(m_pOwner){
			m_pOwner->onCursorMove(e->pos());
			return true;
		}
		
	}
	return false;
}
 
 
//////////////