#ifndef __PATCHER9X_VERSION_H__INCLUDED__

#define PATCHER9X_STR_(x) #x
#define PATCHER9X_STR(x) PATCHER9X_STR_(x)

#define PATCHER9X_MAJOR 0
#define PATCHER9X_MINOR 6

#ifndef PATCHER9X_PATCH
#define PATCHER9X_PATCH 27
#endif

#define PATCHER9X_TAG "BETA2"

#define PATCHER9X_VERSION_STR_BUILD(_ma, _mi, _pa, _tag) \
	_ma "." _mi "." _pa "-" _tag

#define PATCHER9X_VERSION_STR PATCHER9X_VERSION_STR_BUILD( \
	PATCHER9X_STR(PATCHER9X_MAJOR), \
	PATCHER9X_STR(PATCHER9X_MINOR), \
	PATCHER9X_STR(PATCHER9X_PATCH), \
	PATCHER9X_TAG)

#endif /* __PATCHER9X_VERSION_H__INCLUDED__ */
