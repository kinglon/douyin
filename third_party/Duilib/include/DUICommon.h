#pragma once

#include "UIlib.h"

#define UIBEGIN_MSG_MAP virtual void Notify(TNotifyUI& msg) { 
#define SUPER_HANDLER_MSG __super::Notify(msg);

#define EVENT_HANDLER(event, func) if( (event == NULL || msg.sType == event) && msg.pSender) \
{ \
	OutputDebugString(msg.sType + _T("\t\t") + msg.pSender->GetName() + _T("\r\n"));\
	func(msg);\
}

#define EVENT_ID_HANDLER(event, id, func) if(msg.sType == event && msg.pSender && _tcsicmp(msg.pSender->GetName(), id) == 0 ) \
{ \
	OutputDebugString(msg.sType + _T("\t\t") + msg.pSender->GetName() + _T("\r\n"));\
	func(msg);\
}

#define EVENT_BUTTON_CLICK(id,	func)		EVENT_ID_HANDLER(DUI_MSGTYPE_CLICK,			id, func)
#define EVENT_ITEM_CLICK(id,	func)		EVENT_ID_HANDLER(DUI_MSGTYPE_ITEMCLICK,		id, func)
#define EVENT_ITEM_RCLICK(id,	func)		EVENT_ID_HANDLER(DUI_MSGTYPE_ITEMRCLICK,	id, func)
#define EVENT_ITEM_LDBCLICK(id, func)		EVENT_ID_HANDLER(DUI_MSGTYPE_ITEMACTIVATE,	id, func)
#define EVENT_ITEM_SELECT(id,	func)		EVENT_ID_HANDLER(DUI_MSGTYPE_ITEMSELECT,	id, func)
#define EVENT_BUTTON_MOUSEENTER(id, func)			EVENT_ID_HANDLER(DUI_MSGTYPE_MOUSEENTER,	id,	func)
#define EVENT_BUTTON_MOUSELEAVE(id, func)			EVENT_ID_HANDLER(DUI_MSGTYPE_MOUSELEAVE,	id,	func)

#define UIEND_MSG_MAP return;} 


#define VERIFY_LPSZ(lpszVal, ret)  do { if ( (NULL == (lpszVal)) || (NULL == lpszVal[0]) ) return ret; } while ( 0 );
#define VERIFY_LPSZ_NOT_RET(lpszVal)  do { if ( (NULL == (lpszVal)) || (NULL == lpszVal[0]) ) return; } while ( 0 );
#define VERIFY_PTR(lpszVal, ret)  do { if ( (NULL == (lpszVal)) ) return ret; } while ( 0 );
#define VERIFY_PTR_NOT_RET(lpszVal)  do { if ( (NULL == (lpszVal)) ) return; } while ( 0 );


typedef enum
{
	UIFONT_GDI = 0,    /**< GDI */
	UIFONT_GDIPLUS,    /**< GDI+ */
	UIFONT_FREETYPE,   /**< FreeType */ 
	UIFONT_DEFAULT,    
} UITYPE_FONT;

#define DT_CALC_SINGLELINE		 DT_SINGLELINE| DT_LEFT_EX|DT_NOPREFIX  //计算单行
#define DT_CALC_MULTILINE		 DT_WORDBREAK | DT_EDITCONTROL | DT_LEFT_EX | DT_NOPREFIX // 计算多行

#define DUI_MSGTYPE_ITEMRCLICK	(_T("itemrclick"))

enum
{
	RES_TYPE_COURSE = 50001,
	RES_TYPE_VIDEO,
	RES_TYPE_PIC,
	RES_TYPE_FLASH,
	RES_TYPE_VOLUME,	
	RES_TYP_TOTAL
};

#define	TOAST_TIMER_ID		10001