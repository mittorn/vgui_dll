//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "VGUI.h"
#include "VGUI_ScrollPanel.h"
#include "VGUI_ScrollBar.h"
#include "VGUI_IntChangeSignal.h"

using namespace vgui;

class ChangeHandler : public IntChangeSignal
{
public:
	ChangeHandler(ScrollPanel* scrollPanel)
	{
		_scrollPanel=scrollPanel;
	}
	void intChanged(int value,Panel* panel)
	{
		_scrollPanel->recomputeScroll();
	}
private:
	ScrollPanel* _scrollPanel;
};
#ifdef VGUI_TOUCH_SCROLL
#include "VGUI_InputSignal.h"
class TouchScrollSignal: public InputSignal
{
	ScrollPanel *_panel;
	bool _dragging;
	int _lx, _ly, _ax, _ay;
	bool _skip;
public:
	TouchScrollSignal(ScrollPanel *panel)
	{
		_panel = panel;
	}
	void cursorMoved(int x,int y,Panel* panel)
	{
		int dx = x - _lx, dy = y - _ly;
		_lx = x, _ly = y;
		if(_skip || dx > 50 || dy > 50 || dx < -50 || dy < -50) dx = dy = 0;
		if(_dragging)
		{
			_ax -= dx, _ay -= dy;
			_panel->setScrollValue(_ax,_ay);
		}
		_skip = false;
	}
	void cursorEntered(Panel* panel)
	{
	}
	void cursorExited(Panel* panel)
	{
	}
	void mousePressed(MouseCode code,Panel* panel)
	{
		if(code==MOUSE_LEFT)
		{
			_dragging=true;
			_skip = true;
			_panel->getScrollValue(_ax,_ay);
		}
	}
	void mouseDoublePressed(MouseCode code,Panel* panel)
	{
	}
	void mouseReleased(MouseCode code,Panel* panel)
	{
	}
	void mouseWheeled(int delta,Panel* panel)
	{
	}
	void keyPressed(KeyCode code,Panel* panel)
	{
	}
	void keyTyped(KeyCode code,Panel* panel)
	{

	}
	void keyReleased(KeyCode code,Panel* panel)
	{
	}
	void keyFocusTicked(Panel* panel)
	{
	}
	virtual void moved(int dx,int dy,bool internal,Panel* panel,Panel* parent)
	{
	}
};
#endif
ScrollPanel::ScrollPanel(int x,int y,int wide,int tall) : Panel(x,y,wide,tall)
{
	setPaintBorderEnabled(true);
	setPaintBackgroundEnabled(false);
	setPaintEnabled(false);

	_clientClip=new Panel(0,0,wide-16,tall-16);
	_clientClip->setParent(this);
	_clientClip->setBgColor(Color(0,128,0,0));
	_clientClip->setPaintBorderEnabled(true);
	_clientClip->setPaintBackgroundEnabled(false);
	_clientClip->setPaintEnabled(true);

	_client=new Panel(0,0,wide*2,tall*2);
	_client->setParent(this);
	_client->setPaintBorderEnabled(true);
	_client->setPaintBackgroundEnabled(false);
	_client->setPaintEnabled(true);

	_horizontalScrollBar=new ScrollBar(0,tall-16,wide-16,16,false);
	_horizontalScrollBar->setParent(this);
	_horizontalScrollBar->addIntChangeSignal(new ChangeHandler(this));
	_horizontalScrollBar->setVisible(false);

	_verticalScrollBar=new ScrollBar(wide-16,0,16,tall-16,true);
	_verticalScrollBar->setParent(this);
	_verticalScrollBar->addIntChangeSignal(new ChangeHandler(this));
	_verticalScrollBar->setVisible(false);
#ifdef VGUI_TOUCH_SCROLL
	Panel *touchScrollPanel = new Panel(0,0,wide-16,tall-16);
	touchScrollPanel->setParent(this);
	touchScrollPanel->setPaintBorderEnabled(true);
	touchScrollPanel->setPaintBackgroundEnabled(false);
	touchScrollPanel->setPaintEnabled(true);
	touchScrollPanel->addInputSignal(new TouchScrollSignal(this));
#endif
	_autoVisible[0]=true;
	_autoVisible[1]=true;
	validate();
}

void ScrollPanel::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);
	getPaintSize(wide,tall);

	if(_autoVisible[0])
	{
		_horizontalScrollBar->setVisible(!_horizontalScrollBar->hasFullRange());
	}
	if(_autoVisible[1])
	{
		_verticalScrollBar->setVisible(!_verticalScrollBar->hasFullRange());
	}

	if(_verticalScrollBar->isVisible())
		wide-=_verticalScrollBar->getWide();
	if(_horizontalScrollBar->isVisible())
		tall-=_horizontalScrollBar->getTall();

	_verticalScrollBar->setBounds(wide,0,_verticalScrollBar->getWide(),tall);
	_horizontalScrollBar->setBounds(0,tall,wide,_horizontalScrollBar->getTall());
	_clientClip->setSize(wide,tall);
	recomputeClientSize();
	repaint();
}

void ScrollPanel::setScrollBarVisible(bool horizontal,bool vertical)
{
	_horizontalScrollBar->setVisible(horizontal);
	_verticalScrollBar->setVisible(vertical);
	validate();
}

void ScrollPanel::setScrollBarAutoVisible(bool horizontal,bool vertical)
{
	_autoVisible[0]=horizontal;
	_autoVisible[1]=vertical;
	validate();
}

Panel* ScrollPanel::getClient()
{
	return _client;
}

Panel* ScrollPanel::getClientClip()
{
	return _clientClip;
}

void ScrollPanel::setScrollValue(int horizontal,int vertical)
{
	/*_horizontalScrollBar->setRange(0,10);//_client->getWide()-_clientClip->getWide());
	_horizontalScrollBar->setRangeWindow(10000);//_client->getWide());

	_verticalScrollBar->setRange(0,10);//_client->getTall()-_clientClip->getTall());
	_verticalScrollBar->setRangeWindow(10000);//_client->getTall());*/
	_horizontalScrollBar->setValue(horizontal);
	_verticalScrollBar->setValue(vertical);
	recomputeScroll();
}

void ScrollPanel::getScrollValue(int& horizontal,int& vertical)
{
	horizontal=_horizontalScrollBar->getValue();
	vertical=_verticalScrollBar->getValue();
}

void ScrollPanel::recomputeClientSize()
{
	int wide=0;
	int tall=0;
	for(int i=0;i<_client->getChildCount();i++)
	{
		Panel* panel=_client->getChild(i);
		if(panel->isVisible())
		{
			int x,y,w,t;
			panel->getPos(x,y);
			panel->getVirtualSize(w,t);
			x+=w;
			y+=t;
			if(x>wide)
				wide=x;
			if(t>tall)
				tall=t;
		}
	}

	_client->setSize(wide,tall);
	_horizontalScrollBar->setRange(0,_client->getWide()-_clientClip->getWide());
	_horizontalScrollBar->setRangeWindow(_client->getWide());

	_verticalScrollBar->setRange(0,_client->getTall()-_clientClip->getTall());
	_verticalScrollBar->setRangeWindow(_client->getTall());
}

ScrollBar* ScrollPanel::getHorizontalScrollBar()
{
	return _horizontalScrollBar;
}

ScrollBar* ScrollPanel::getVerticalScrollBar()
{
	return _verticalScrollBar;
}

void ScrollPanel::validate()
{
	_horizontalScrollBar->setRangeWindowEnabled(true);
	_verticalScrollBar->setRangeWindowEnabled(true);

	int wide,tall;
	getSize(wide,tall);
	setSize(wide,tall);
	setSize(wide,tall);
	setSize(wide,tall);
	setSize(wide,tall);
	//recomputeScroll();
}

void ScrollPanel::recomputeScroll()
{
	int horizontal,vertical;
	getScrollValue(horizontal,vertical);
	_client->setPos(-horizontal,-vertical);
	_clientClip->repaint();
}
