/**
* @file GM_RSSP1_Mutex.h
* @brief
* @author Hao Liming
* @date 2010-11-26 22:25:31
* @version
* <pre><b>copyright: CASCO</b></pre>
* <pre><b>email: </b>haoliming@casco.com.cn</pre>
* <pre><b>company: </b>http://www.casco.com.cn</pre>
* <pre><b>All rights reserved.</b></pre>
* <pre><b>modification:</b></pre>
* <pre>Write modifications here.</pre>
*/
#ifndef GM_RSSP1_MUTEX_H
#define GM_RSSP1_MUTEX_H

#include "GM_RSSP1_Utils_Base.h"
#include "GM_RSSP1_Memory.h"

#ifndef GM_RSSP1_DISABLE_LOCK

#ifdef VXWORKS
#include <semLib.h>
#endif

#if (defined QNX || defined REWORKS)
#include <pthread.h>
#endif

#ifdef LINUX
#include <pthread.h>
#endif

#ifdef REWORKS_CERTV2
#include <core/mutex.h>
#endif

#endif

#ifdef __cplusplus
extern "C"
{
#endif /**< __cplusplus */

#ifdef GM_RSSP1_DISABLE_LOCK
    typedef GM_RSSP1_UINT8  GM_RSSP1_MUTEX;
#else

#ifdef WIN32
	typedef HANDLE          GM_RSSP1_MUTEX;
#endif
#if (defined QNX || defined REWORKS)
	typedef pthread_mutex_t* GM_RSSP1_MUTEX;
#endif
#ifdef VXWORKS
	typedef SEM_ID          GM_RSSP1_MUTEX;
#endif

#ifdef LINUX
	typedef pthread_mutex_t* GM_RSSP1_MUTEX;
#endif

#ifdef REWORKS_CERTV2
	typedef struct mutex*  GM_RSSP1_MUTEX;
#endif

#endif

    GM_RSSP1_BOOL GM_RSSP1_Mutex_Init(GM_RSSP1_MUTEX* pMutex);

    /**
    * @brief GM_RSSP1_Mutex_Lock
    *
    * 获得互斥器资源.
    * @param[in] mutex
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Mutex_Lock(GM_RSSP1_MUTEX mutex);

    /**
    * @brief GM_RSSP1_Mutex_Unlock
    *
    * 释放互斥器.
    * @param[in] mutex
    * @return GM_RSSP1_BOOL
    */
    GM_RSSP1_BOOL GM_RSSP1_Mutex_Unlock(GM_RSSP1_MUTEX mutex);

#ifdef __cplusplus
}

#endif /**< __cplusplus */

#endif /**< _GM_RSSP1_MUTEX_H */
