//========= Copyright ?1996-2002, Valve LLC, All rights reserved. ============
//
// Purpose: 
//
// $NoKeywords: $
//=============================================================================

#include "VGUI.h"
#include "VGUI_ScrollBar.h"
#include "VGUI_Button.h"
#include "VGUI_Slider.h"
#include "VGUI_IntChangeSignal.h"
#include "VGUI_ActionSignal.h"
using namespace vgui;

namespace{
class FooDefaultScrollBarIntChangeSignal: public IntChangeSignal
{
	ScrollBar *_scrollBar;
public:

	FooDefaultScrollBarIntChangeSignal( ScrollBar *scrollBar)
	{
		_scrollBar = scrollBar;
	}

	void intChanged(int value, Panel *panel)
	{
		_scrollBar->setValue( value );
	}
};
class FooDefaultButtonSignal: public ActionSignal
{
	ScrollBar *_scrollBar;
	int _buttonIndex;
public:
	FooDefaultButtonSignal( ScrollBar *scrollBar, int index)
	{
		_scrollBar = scrollBar;
		_buttonIndex = index;
	}

	void actionPerformed(Panel *panel)
	{
		_scrollBar->doButtonPressed(_buttonIndex);
	}
};
}
ScrollBar::ScrollBar(int x,int y,int wide,int tall,bool vertical) : Panel(x,y,wide,tall)
{
	_slider=null;
	_button[0]=null;
	_button[1]=null;

	if(vertical)
	{
		setSlider(new Slider(0,wide-1,wide,tall-wide*2+2,true));
		setButton(new Button("",0,0,wide,wide),0);
		setButton(new Button("",0,tall-wide,wide,wide),1);
	}
	else
	{
		setSlider(new Slider(tall,0,wide-tall*2,tall,false));
		setButton(new Button("",0,0,tall+1,tall+1),0);
		setButton(new Button("",0,wide-tall,tall+1,tall+1),1);
	}

	setPaintBorderEnabled(true);
	setPaintBackgroundEnabled(true);
	setPaintEnabled(true);
	setButtonPressedScrollValue(15);
	validate();
}

void ScrollBar::setValue(int value)
{
	_slider->setValue( value );
}

int ScrollBar::getValue()
{
	return _slider->getValue();
}

void ScrollBar::addIntChangeSignal(IntChangeSignal* s)
{
	_intChangeSignalDar.addElement(s);
}
 
void ScrollBar::setRange(int min,int max)
{
	_slider->setRange(min, max);
}

void ScrollBar::setRangeWindow(int rangeWindow)
{
	_slider->setRangeWindow( rangeWindow );
}

void ScrollBar::setRangeWindowEnabled(bool state)
{
	_slider->setRangeWindowEnabled( state );
}

void ScrollBar::setSize(int wide,int tall)
{
	Panel::setSize(wide,tall);

	if(!_slider||!_button[0]||!_button[1])
	{
		return;
	}

	getPaintSize(wide,tall);
	if(_slider->isVertical())
	{
		_slider->setBounds(0,wide,wide,tall-wide*2);
		_button[0]->setBounds(0,0,wide,wide);
		_button[1]->setBounds(0,tall-wide,wide,wide);
	}
	{
		_slider->setBounds(0,wide-tall*2,tall,tall);
		_button[0]->setBounds(0,0,tall,tall);
		_button[1]->setBounds(wide-tall,0,tall,tall);
	}
}

bool ScrollBar::isVertical()
{
	return _slider->isVertical();
}

bool ScrollBar::hasFullRange()
{
	return _slider->hasFullRange();
}

void ScrollBar::setButton(Button* button,int index)
{
	if( index >= 0 && index < 2 )
	{
		if( _button[index])
			removeChild( _button[index]);
		_button[index] = button;
		addChild( button);
		button->addActionSignal( new FooDefaultButtonSignal( this, index) );
		validate();
	}
}

Button* ScrollBar::getButton(int index)
{
	if( index >= 0 && index < 2 )
		return _button[index];
	return null;
}

void ScrollBar::setSlider(Slider* slider)
{
	if( _slider )
		removeChild( _slider );
	_slider = slider;
	addChild( slider );
	slider->addIntChangeSignal(new FooDefaultScrollBarIntChangeSignal( this ) );
	validate();
}

Slider* ScrollBar::getSlider()
{
	return _slider;
}

void ScrollBar::doButtonPressed(int buttonIndex)
{
	if( buttonIndex )
		setValue(getValue() + _buttonPressedScrollValue);
	else
		setValue(getValue() - _buttonPressedScrollValue);

}

void ScrollBar::setButtonPressedScrollValue(int value)
{
	_buttonPressedScrollValue = value;
}

void ScrollBar::validate()
{
	int wide, tall;
	int rangeWindow;
	if( _slider )
	{
		if( _button[0] && _button[0]->isVisible() )
		{
			if( _slider->isVertical() )
				rangeWindow = _button[0]->getTall();
			else
				rangeWindow = _button[0]->getWide();
		}
		if( _button[1] && _button[1]->isVisible() )
		{
			if( _slider->isVertical() )
				rangeWindow += _button[1]->getTall();
			else
				rangeWindow += _button[1]->getWide();
		}
		_slider->setRangeWindow( rangeWindow );
	}
	getSize(wide, tall);
	setSize(wide,tall);
}

void ScrollBar::fireIntChangeSignal()
{
	for(int i=0;i<_intChangeSignalDar.getCount();i++)
		_intChangeSignalDar[i]->intChanged(getValue(), this);
}

void ScrollBar::performLayout()
{

}

