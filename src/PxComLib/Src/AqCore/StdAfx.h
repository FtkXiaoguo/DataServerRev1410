/******************************************************************************


	No part of this code may be reproduced, stored in
	a retrieval system, or transmitted, in any form or by any means,
	electronic or mechanical, photocopying, recording, or otherwise,
	without the prior written permission of the copyright holder.

******************************************************************************/

#if !defined(AFX_STDAFX_H__23630334_C318_4BFB_86BD_B2A48218B916__INCLUDED_)
#define AFX_STDAFX_H__23630334_C318_4BFB_86BD_B2A48218B916__INCLUDED_

//for CoInitializeEx
#define _WIN32_DCOM

#include <atlbase.h>

#define _NO_WTL

#ifndef _NO_WTL

#define _WTL_NO_AUTOMATIC_NAMESPACE
// this definition trggier to use RTL _vstprintf instead of wvsprintf which has 1024 limit
#define _ATL_USE_CSTRING_FLOAT

#include <atlapp.h>
extern CComModule _Module;

#include <atlmisc.h> //for CString

//#include <atlcom.h>
//#include <atlhost.h>
//#include <atlctl.h>
//#include <atlwin.h>

#endif



//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ 

#endif // !defined(AFX_STDAFX_H__23630334_C318_4BFB_86BD_B2A48218B916__INCLUDED_)
