#include "stdafx.h"
#include "pqueue.h"
#include "CT_Morph.h"

bool 
lessThen( const void *l, const void *r)
{
	ZEdge *pE1, *pE2;
	pE1 = (ZEdge*)l;
	pE2 = (ZEdge*)r;


	return (pE1->error < pE2->error); // l�� r���� ������ ���ĵǾ� ����.
}

bool 
lessThen2( const void *l, const void *r)
{
	ZEdge *pE1, *pE2;
	pE1 = (ZEdge*)l;
	pE2 = (ZEdge*)r;


	return (pE1->waitforswap < pE2->waitforswap); // l�� r���� ������ ���ĵǾ� ����.
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


static int __downObject( int offset, pq_pState pState) // offset ~ MAX ���� �����Ѵ�.
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


static int __adjustObject( int n, pq_pState pState) // n ��ġ�� �߽����� ������ġ�� �����Ѵ�.
{
	int shift;

	if (!(shift = (n >> 1)))
		;
	else {
		if (pState->lessThen( pState->heap[n], pState->heap[shift]))
			return __upObject( n, pState);
	} return __downObject( n, pState);
}

// �����͸� �����Ѵ�. ���⿡�� ������ ���۰� ���ٸ� ť�� �����ϴ� �������� ���İ��� ���� ���� 
// �����͸� ��ȯ�Ѵ�.
void * pq_push( const pq_pState pState, void *obj )
{
	if (pState->nobject < pState->size)
	{
		++pState->nobject;
		pState->heap[pState->nobject] = obj;
	}
	else { // ť�� ���� ������ ���ٸ�...
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

// ���� �����͸� �д´�. (����, ���ĵ� ���� ��ȯ�Ѵ�)
void *pq_top( pq_pState pState)
{
	return pState->nobject ? pState->heap[1]: NULL;
}

// �ֻ��� �����͸� �����ϰ�, ������ �����͸� ������� ���� ���� �����Ѵ�.
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

// ���� �˻����� ť�� ����� �����͸� ã�´�. ���⿡�� n�� ������ 0 ~ N ���� �̴�.
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

// n ��ġ�� �����Ͱ��� �� �����Ѵ�.
int pq_resort( pq_pState pState, int n)
{
	if ((n <= 0) || (n > pState->nobject))
		errno = ERANGE;
	else {
		return __adjustObject( n, pState);
	} return -1;
}
// n ��ġ�� ���� �����ϰ�, ������ �����͸� ������� �������� �����Ѵ�.
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

// n��ġ�� �����͸� ���ο� �����ͷ� �����ϰ� ť�� �������Ѵ�.
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