#include "stdafx.h"
#include "pqueue.h"
#include "CT_Morph.h"

bool 
lessThen( const void *l, const void *r)
{
	ZEdge *pE1, *pE2;
	pE1 = (ZEdge*)l;
	pE2 = (ZEdge*)r;


	return (pE1->error < pE2->error); // l이 r보다 앞으로 정렬되어 진다.
}

bool 
lessThen2( const void *l, const void *r)
{
	ZEdge *pE1, *pE2;
	pE1 = (ZEdge*)l;
	pE2 = (ZEdge*)r;


	return (pE1->waitforswap < pE2->waitforswap); // l이 r보다 앞으로 정렬되어 진다.
}

int __upObject( int offset, const pq_pState pState )
{
	void *node = pState->heap[offset]; {
		register int shift = (offset >> 1);
		while ((shift > 0) && 
			pState->lessThen( node, pState->heap[shift]))
		{
			pState->heap[offset] = pState->heap[shift]; {
				offset = shift;
			} shift >>= 1;
		} pState->heap[offset] = node;
	} return offset;
}


static int __downObject( int offset, pq_pState pState) // offset ~ MAX 까지 정렬한다.
{
	void *node = pState->heap[offset]; {
		register int delta, 
			down;

_gWhile:down = (delta = offset << 1) + 1;
		if ((down > pState->nobject) ||
			!pState->lessThen( pState->heap[down], pState->heap[delta]))
			;
		else delta = down;

		if ((delta > pState->nobject) ||
			!pState->lessThen( pState->heap[delta], node))
			;
		else {
			pState->heap[offset] = pState->heap[delta]; {
				offset = delta;
			} goto _gWhile;
		} pState->heap[offset] = node;
	} return offset;
}


static int __adjustObject( int n, pq_pState pState) // n 위치를 중심으로 정렬위치를 결정한다.
{
	int shift;

	if (!(shift = (n >> 1)))
		;
	else {
		if (pState->lessThen( pState->heap[n], pState->heap[shift]))
			return __upObject( n, pState);
	} return __downObject( n, pState);
}

// 데이터를 저장한다. 여기에서 저장할 버퍼가 없다면 큐에 존재하는 데이터중 정렬값이 가장 낮은 
// 데이터를 반환한다.
void * pq_push( const pq_pState pState, void *obj )
{
	if (pState->nobject < pState->size)
	{
		++pState->nobject;
		pState->heap[pState->nobject] = obj;
	}
	else { // 큐에 저장 공간이 없다면...
		if ((pState->nobject <= 0) ||
			pState->lessThen( obj, pState->heap[1]))
			;
		else {
			void *top = pState->heap[1];
			pState->heap[1] = obj; { 
				__downObject( 1, pState);
			} obj = top;
		} return obj;
	} __upObject( pState->nobject, pState); return NULL;
}

// 상위 데이터를 읽는다. (물론, 정렬된 값을 반환한다)
void *pq_top( pq_pState pState)
{
	return pState->nobject ? pState->heap[1]: NULL;
}

// 최상위 데이터를 제거하고, 나머지 데이터를 대상으로 상위 값을 추출한다.
void *pq_pop( pq_pState pState)
{
	void *obj;

	if (pState->nobject <= 0)
		return NULL;
	else {
		obj = pState->heap[1]; 

		pState->heap[1] = pState->heap[pState->nobject]; 
		pState->heap[pState->nobject--] = NULL;
		__downObject( 1, pState); 
	} return obj;
}

// 순차 검색으로 큐에 저장된 데이터를 찾는다. 여기에서 n의 범위는 0 ~ N 까지 이다.
void *pq_each( pq_pState pState, int n) 
{
	if ((n <= 0) || (n > pState->nobject))
		errno = ERANGE;
	else {
		return pState->heap[n];
	} return NULL;
}

int pq_find( pq_pState pState, void *obj )
{
	for( int i=0; i < pState->size; i++ )
	{
		if( pState->heap[i] == obj ) 
		{
			return i;
		}
	}
	return -1;
}

// n 위치의 데이터값을 재 정력한다.
int pq_resort( pq_pState pState, int n)
{
	if ((n <= 0) || (n > pState->nobject))
		errno = ERANGE;
	else {
		return __adjustObject( n, pState);
	} return -1;
}
// n 위치의 값을 제거하고, 나머지 데이터를 대상으로 재정렬을 수행한다.
void *pq_remove( pq_pState pState, int n)
{
	if ((n <= 0) || (n > pState->nobject))
		errno = ERANGE;
	else {
		void *obj = pState->heap[n];

		if (n >= pState->nobject)
			pState->heap[pState->nobject--] = NULL;
		else {
			pState->heap[n] = pState->heap[pState->nobject]; {
				pState->heap[pState->nobject--] = NULL;
			} if (pState->nobject > 0) __adjustObject( n, pState);
		} return obj;
	} return NULL;
}

void pq_removebypointer( pq_pState pState, void* obj )
{
	pq_remove( pState, pq_find( pState, obj ) );
}

// n위치의 데이터를 새로운 데이터로 변경하고 큐를 재정렬한다.
void *pq_update( pq_pState pState, int n, void *obj)
{
	if ((n <= 0) || (n > pState->nobject))
		errno = ERANGE;
	else {
		void *now = pState->heap[n];

		pState->heap[n] = obj; {
			__adjustObject( n, pState);
		} return now;
	} return NULL;
}